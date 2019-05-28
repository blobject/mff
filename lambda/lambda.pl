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
max(A, B, M) :- A #>= B, M = A; M = B.

% Get 1-indexed position of element in a list, 0 if absent.
index([], _, 0).
index([E|_], E, 0) :- !.
index([X|L], E, N) :- dif(X, E), index(L, E, N1), N #= N1 + 1.

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
chars(C, C, [])    :- not(char(C)).
chars(C, R, [C|S]) :- char(C), get_char(N), chars(N, R, S).

string(C, V, R) :- get_char(N), chars(N, R, S), atom_chars(V, [C|S]).

% Build a list of tokens delimited by space.
token(S)        :- get_char(C), token(C, S).
token(C, [])    :- eol(C).
token(C, S)     :- space(C), get_char(N), token(N, S).
token(C, [C|S]) :- special(C), get_char(N), token(N, S).
token(C, [V|S]) :- char(C), string(C, V, N), token(N, S).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% parse & grammar

% Parse a list of tokens into a parenthetic tree.
tree(L, T) :- tree(L, T, []).
tree([], [], []).
tree([P|L], [P|T], R)  :- not(paren(P)), tree(L, T, R).
tree([P|L], [], [P|L]) :- rparen(P).
tree([P|L], [M|T], R)  :- lparen(P), rparen(Q),
                          tree(L, M, [Q|S]), tree(S, T, R).

% Convert a parenthetic tree into a compound data structure.
%   sym = variable    --.
%   app = application --|-- term
%   fun = abstraction --'
data([T], R)      :- is_list(T), data(T, R).
data([S], R)      :- not(is_list(S)), R = sym(S).
data([A|B], R)    :- is_list(A), data(A, X), data(B, Y), R = app(X, Y).
data([A|B], R)    :- not(is_list(A)), not(spacial(A)),
                     data(B, X), R = app(sym(A), X).
data([C,S,D|T],R) :- caret(C), dot(D), not(spacial(S)),
                     data(T, X), R = fun(sym(S), X).

% Convert a compound data structure into a de Bruijn index tree.
d2b(D, R) :- d2b(D, [], R).
d2b(sym(S), V, R)         :- index(V, S, X), length(V, N),
                             (X #>= N, R = 0, !; R #= X + 1).
d2b(app(A, B), V, [C, D]) :- d2b(A, V, C), d2b(B, V, D).
d2b(fun(sym(S), T), V, R) :- d2b(T, [S|V], X), R = f(X).

% Convert a de Bruijn index tree into a compound data structure.
b2d(T, R) :- b2d(T, _, R).
b2d(N, 0, sym(N))   :- integer(N).
b2d([N], 0, sym(N)) :- integer(N).
b2d([N|L], D, R) :- dif(L, []), b2d(N, A, X), b2d(L, B, Y),
                    max(A, B, D), R = app(X, Y).
b2d(f(T), D, R) :- D #= E + 1, b2d(T, E, X), R = fun(sym(D), X).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% reduce (beta reduction on de Bruijn index trees)
%%
%% NOTE: alpha conversion made unnecessary by use of de Bruijn indexing.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% run

loop :-
  % Build token list.
  token(L), write('toks: '), write(L), nl,

  % Build parenthetic tree.
  tree(L, M), write('tree: '), write(M), nl,

  % Build compound data structure.
  data(M, T), write('data: '), write(T), nl,

  % De/Construct de Bruijn index tree.
  d2b(T, B), write('d->B: '), write(B), nl,
  b2d(B, D), write('B->d: '), write(D), nl,

  % TODO: Beta-reduce.

  loop.

preamble :-
  write('.-------------------------------------------.'), nl,
  write('| Welcome to a lambda-calculus interpreter! |'), nl,
  write('|                                           |'), nl,
  write('| INPUT examples:                           |'), nl,
  write('| - x; x y; (x y) z; \\x.x y; \\a (\\b.b c) d  |'), nl,
  write('| OUTPUT:                                   |'), nl,
  write('| - tok(ens); (parse)tree; (compound) data; |'), nl,
  write('|   de Bruijn indexing; reductions;         |'), nl,
  write('| - Any absence of output means failure.    |'), nl,
  write('| START:                                    |'), nl,
  write('| - `swipl lambda.pl` OR                    |'), nl,
  write('| - `echo \'x\' | swipl lambda.pl`            |'), nl,
  write('| QUIT:                                     |'), nl,
  write('| - C-d on empty OR double C-c              |'), nl,
  write("'-------------------------------------------'"), nl.

run :-
  %preamble,
  loop,
  halt.
run :- halt(1).

