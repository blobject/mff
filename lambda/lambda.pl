% file: lambda/lambda.pl
% for: nprg005

:- use_module(library(clpfd)).
:- set_prolog_flag(verbose, silent).
:- initialization(run). % commandline exec, with pipeable input

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% special characters

space(' '). eol('\n').
lparen('('). rparen(')').
caret('\\'). dot('.').
paren(P) :- lparen(P); rparen(P).
special(X) :- paren(X); caret(X); dot(X).
spacial(X) :- special(X); space(X); eol(X). % special incl. whitespace

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% helpers

% Let M be the greater of A and B.
max(A, B, M) :- A #>= B, M = A, !; M = B.

% Get 1-indexed position of element in a list, 0 if absent.
index([], _, 0).
index([E|_], E, 0) :- !.
index([X|L], E, N) :- dif(X, E), index(L, E, N1), N #= N1 + 1.

% Flatten list of lists.
flatten([], []) :- !.
flatten([E|L], R) :- !, flatten(E, X), flatten(L, Y), append(X, Y, R).
flatten(L, [L]).

% (unused) Is sym V free?
free(V, V).
free(V, app(T, U)) :- free(V, T); free(V, U).
free(V, fun(W, T)) :- dif(V, W), free(V, T).

% (unused) Get all free syms.
frees(sym(V), [sym(V)]).
frees(app(T, U), R) :- frees(T, X), frees(U, Y), union(X, Y, R).
frees(fun(V, T), R) :- frees(T, X), select(V, X, R).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% read & tokenise

% Is character an ascii grapheme?
char(C) :- not(char_type(C, end_of_file)), char_code(C, A),
           (A #>= 65, A #=< 90;
            A #>= 33, A #=< 64,
            dif(A, 40), dif(A, 41), dif(A, 46);
            A #>= 91, A #=< 126, dif(A, 92)).

% Read (characters) into string S.
read_chars(C, C, [])    :- not(char(C)).
read_chars(C, R, [C|S]) :- char(C), get_char(N), read_chars(N, R, S).

read_string(C, V, R) :- get_char(N), read_chars(N, R, S),
                        atom_chars(V, [C|S]).

% Build a list of tokens delimited by space.
token(S)        :- get_char(C), token(C, S).
token(C, [])    :- eol(C).
token(C, S)     :- space(C), get_char(N), token(N, S).
token(C, [C|S]) :- special(C), get_char(N), token(N, S).
token(C, [V|S]) :- char(C), read_string(C, V, N), token(N, S).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% representations
%%
%% Sketch of process:
%%
%% input canon -> tree -> data -> deb -> reduce -.
%%          ^                                    |
%%          '------------ atad -> bed <----------'

%% tree %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Parse a list of tokens into a parenthetic tree.
tree(L, T) :- tree(L, T, []).
tree([],    [],    []).
tree([P|L], [P|T], R)     :- not(paren(P)), tree(L, T, R).
tree([P|L], [],    [P|L]) :- rparen(P).
tree([P|L], [M|T], R)     :- lparen(P), rparen(Q),
                             tree(L, M, [Q|S]), tree(S, T, R).

%% data & atad %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Convert a parenthetic tree into a compound data structure.
%   sym = variable    --.
%   app = application --|-- term
%   fun = abstraction --'
data([T],   R)    :- is_list(T), data(T, R).
data([S],   R)    :- not(is_list(S)), R = sym(S).
data([A|B], R)    :- is_list(A), data(A, X), data(B, Y), R = app(X, Y).
data([A|B], R)    :- not(is_list(A)), not(spacial(A)),
                     data(B, X), R = app(sym(A), X).
data([C,S,D|T],R) :- caret(C), dot(D), not(spacial(S)),
                     data(T, X), R = fun(sym(S), X).

% Convert a compound data structure into a canonical string.
% NOTE: The result is a list of characters, to be stringified by caller.
chars([], []).
chars([A|R], [C|S]) :- char_code(C, A), chars(R, S).

atad(sym(S),    R) :- atom_codes(S, A), chars(A, R).
atad(app(A, B), R) :- atad(A, X), atad(B, Y),
                      flatten(['(', X, ' ', Y, ')'], R).
atad(fun(S, T), R) :- atad(S, X), atad(T, Y),
                      flatten(['\\', X, '.', Y], R).

%% deb & bed %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Convert a compound data structure into a de Bruijn index tree.
deb(D, R) :- deb(D, [], R).
deb(sym(S),         V, R)      :- index(V, S, X), length(V, N),
                                  (X #>= N, R = 0, !; R #= X + 1).
deb(app(A, B),      V, [C, D]) :- deb(A, V, C), deb(B, V, D).
deb(fun(sym(S), T), V, R)      :- deb(T, [S|V], X), R = f(X).

% Convert a de Bruijn index tree into a compound data structure.
bed(T, R) :- bed(T, _, R).
bed(N,      0, sym(N)) :- integer(N).
bed([N],    0, sym(N)) :- integer(N).
bed([A, B], D, R)      :- bed(A, M, X), bed(B, N, Y),
                          max(M, N, D), R = app(X, Y).
bed(f(T),   D, R)      :- D #= E + 1, bed(T, E, X), R = fun(sym(D), X).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% reduce (beta reduction on de Bruijn index trees)

% github.com/ptarau/play/blob/master/play.pro

beta(f(A), T, R) :- sub(A, 1, T, R).

sub([A, B], I, T, [X, Y]) :- I #>= 0, sub(A, I, T, X), sub(B, I, T, Y).
sub(f(A),   I, T, f(R))   :- I #>= 0, J #= I + 1, sub(A, J, T, R).
sub(N,      I, _, R)      :- integer(N), I #>= 0, N #> I, R #= N - 1.
sub(N,      I, _, N)      :- integer(N), I #>= 0, N #< I.
sub(N,      I, T, R)      :- integer(N), I #>= 0, N #= I,
                             shift(I, 1, T, R).

shift(I, K, [A, B], [X, Y]) :- K #>= 0, I #>= 0,
                               shift(I, K, A, X), shift(I, K, B, Y).
shift(I, K, f(A),   f(R))   :- K #>= 0, I #>= 0, J #= K + 1,
                               shift(I, J, A, R).
shift(I, K, N,      M)      :- integer(N), K #>= 0, I #>= 0, N #>= K,
                               M #= N + I.
shift(I, K, N,      N)      :- integer(N), K #>= 0, I #>= 0, N #< K.

redux1(N,      N) :- integer(N).
redux1(f(T),   f(T)).
redux1([A, B], R) :- redux1(A, X), reduce1(X, B, R).

reduce1(N,      X, [N, X]) :- integer(N).
reduce1(f(T),   X, R)      :- beta(f(T), X, A), redux1(A, R).
reduce1([A, B], X, [[A, B], X]).

redux(N,      N)    :- integer(N).
redux(f(T),   f(U)) :- redux(T, U).
redux([A, B], R)    :- redux1(A, X), reduce(X, B, R).

reduce(N,      X, [N, Y]) :- integer(N), redux(X, Y).
reduce(f(T),   X, R)      :- beta(f(T), X, A), redux(A, R).
reduce([A, B], X, [C, D]) :- redux([A, B], C), redux(X, D).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% run

loop :-
  % Build token list.
  token(L), write('toks: '), write(L), nl,

  % Convert to parenthetic tree.
  tree(L, M), write('tree: '), write(M), nl,

  % Convert to compound data structure.
  data(M, T), write('data: '), write(T), nl,

  % Convert to de Bruijn index tree.
  deb(T, B), write('d->B: '), write(B), nl,

  % Beta-reduce.
  redux(B, R), write('red.: '), write(R), nl,

  % Convert back to compound data structure.
  bed(R, D), write('B->d: '), write(D), nl,

  % Convert back to canonical syntax.
  atad(D, S), string_chars(O, S), write('RESULT: '), write(O), nl,

  loop.

preamble :-
  write('.-------------------------------------------.'), nl,
  write('| Welcome to a lambda-calculus interpreter! |'), nl,
  write('|                                           |'), nl,
  write('| INPUT examples:                           |'), nl,
  write('| - x; x y; (x y) z; \\x.x y; \\a (\\b.b c) d  |'), nl,
  write('| OUTPUT:                                   |'), nl,
  write('| - tok(ens); (parse) tree; (compound) data;|'), nl,
  write('|   de Bruijn indexing; reductions;         |'), nl,
  write('| - Absence of any output means failure.    |'), nl,
  write('| START:                                    |'), nl,
  write('| - `swipl lambda.pl` OR                    |'), nl,
  write('| - `echo \'x.x\' | swipl lambda.pl`          |'), nl,
  write('| QUIT:                                     |'), nl,
  write('| - C-d on empty OR double C-c              |'), nl,
  write("'-------------------------------------------'"), nl.

run :-
  %preamble,
  loop,
  halt.
run :- halt(1).

