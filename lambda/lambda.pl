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

% Enumerate natural numbers.
nat(0).
nat(N) :- nat(X), N #= X + 1.

% Get 1-indexed position of element in a list, 0 if absent.
index([], _, 0).
index([E|_], E, 0) :- !.
index([X|L], E, N) :- dif(X, E), index(L, E, N1), N #= N1 + 1.

% Flatten list of lists.
flatten([], []) :- !.
flatten([E|L], R) :- !, flatten(E, X), flatten(L, Y), append(X, Y, R).
flatten(L, [L]).

% Is sym V free?
free(V, V).
free(V, app(T, U)) :- free(V, T); free(V, U).
free(V, fun(W, T)) :- dif(V, W), free(V, T).

% Get all free syms.
frees(sym(V), [sym(V)]).
frees(app(T, U), R) :- frees(T, X), frees(U, Y), union(X, Y, R).
frees(fun(V, T), R) :- frees(T, X), subtract(X, [V], R).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% read & tokenise

% Is character an ascii grapheme?
% Excluded:
%   0..31  = control (incl. nl) --.
%   32     = space                |
%   40..41 = parens               |-- "spacial"
%   46     = dot                  |
%   92     = caret              --'
%   127    = del
char(C) :- not(char_type(C, end_of_file)), char_code(C, A),
           (A in 33..39; A in 42..45; A in 47..91; A in 93..126).

% Read (characters) into string S.
%
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
%% input canon -> tree -> data -> reduce --.
%%          ^                              |
%%          '------------ atad <-----------'

%% tree %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Parse a list of tokens into a parenthetic tree.
tree(L, T) :- tree(L, T, []).
tree([],    [],    []).
tree([P|L], [P|T], R)     :- not(paren(P)), tree(L, T, R).
tree([P|L], [],    [P|L]) :- rparen(P).
tree([P|L], [M|T], R)     :- lparen(P), rparen(Q),
                             tree(L, M, [Q|S]), tree(S, T, R).

%% data & atad %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%        .-- sym = variable
% term --|-- app = application
%        '-- fun = abstraction
%
% Parsing into a compound data structure is done in two steps in order
% to achieve left-associative application.
% - The first step (dat) ensures that the "\ x . _" forms are properly
%   recognised as funs.
% - The second step (data) parses out the apps left-associatively via
%   append. All syms are also settled in this step.

% Convert a parenthetic tree into a data structure. First iteration.
dat([S],       [S])              :- not(is_list(S)).
dat([S],       [R])              :- is_list(S), dat(S, R).
dat([C,S,D|T], [fun(sym(S), X)]) :- caret(C), dot(D), not(spacial(S)),
                                    dat(T, X), !.
dat([S|T],     [X|Y])            :- (is_list(S), dat(S, X), !;
                                    X = S), dat(T, Y).

% Second iteration: parse all apps and syms.
data(fun(S, T), fun(S, X)) :- data(T, X), !.
data([S], R)       :- data(S, R), !.
data(S, sym(S))    :- not(is_list(S)), not(spacial(S)).
data(T, app(X, Y)) :- append(A, sym(S), T), data(A, X), Y = sym(S).
data(T, app(X, Y)) :- append(A, fun(V, U), T), data(A, X),
                      data(U, W), Y = fun(V, W).
data(T, app(X, Y)) :- append(A, [B], T), data(A, X), data(B, Y).

% Convert a compound data structure into a canonical string.
% NOTE: The result is a list of characters, to be stringified by caller.
%
chars([], []).
chars([A|R], [C|S]) :- char_code(C, A), chars(R, S).

atad(sym(S),    R) :- atom_codes(S, A), chars(A, R).
atad(app(A, B), R) :- atad(A, X), atad(B, Y),
                      flatten(['(', X, ' ', Y, ')'], R).
atad(fun(S, T), R) :- atad(S, X), atad(T, Y),
                      flatten(['\\', X, '.', Y], R).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% reduction

% Create new name N, from original O, for term T.
% Done by appending a number after O such that N is not free in T.
alias(T, O, N) :- nat(A), atom_concat(O, A, N), not(free(sym(N), T)), !.

% Substitute given old sym V with given new sym W.
rename(sym(V),         V, W, sym(W)).
rename(sym(S),         V, _, sym(S))         :- dif(S, V).
rename(fun(sym(V), M), V, W, fun(sym(W), N)) :- rename(M, V, W, N).
rename(fun(sym(S), M), V, W, fun(sym(S), N)) :- dif(S, V),
                                                rename(M, V, W, N).
rename(app(A, B),      V, W, app(X, Y))      :- rename(A, V, W, X),
                                                rename(B, V, W, Y).

% Handle contractum. (alpha happens @ "sub(fun(sym..")
sub(V,         V, N, N). % V is always sym(_)
sub(sym(S),    V, _, sym(S))    :- dif(sym(S), V).
sub(app(A, B), V, N, app(X, Y)) :- sub(A, V, N, X), sub(B, V, N, Y).
% (\x.P)[x:=N] => \x.P
sub(fun(V, M), V, _, fun(V, M)).
% (\y.P)[x:=N] => \y.P[x:=N]       <= y NOT_IN frees(N)
% (\y.P)[x:=N] => \z.P[y:=z][x:=N] <= y IN frees(N), z NOT_IN frees(NP)
sub(fun(sym(S), M), sym(V), N, fun(sym(X), Y)) :-
  dif(S, V), (free(sym(S), N), alias(M, S, X), alias(N, S, X),
  rename(M, S, X, P), !; P = M, X = S), sub(P, sym(V), N, Y).

% Find redex and call sub on it. (beta happens @ "reduce(app(fun..")
red(sym(A),    sym(A)).
red(fun(V, M), fun(V, N)) :- red(M, N).
red(app(fun(V, M), N), R) :- sub(M, V, N, R), !.
red(app(A, B), app(X, B)) :- red(A, X).

% Wrapper for the red, sub, rename family of functions.
reduce(T, R) :- red(T, P), red(P, Q), (dif(P, Q), reduce(Q, R), !; R = Q).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% run

loop :-
  % Build token list.
  token(L), nl, write('toks: '), write(L), nl,

  % Convert to parenthetic tree.
  tree(L, T), write('tree: '), write(T), nl,

  % Convert to compound data structure.
  dat(T, C), data(C, D), write('data: '), write(D), nl,

  % Reduce.
  reduce(D, B), write('redu: '), write(B), nl,

  % Convert back to canonical syntax.
  atad(B, S), string_chars(R, S), write('result: '), write(R), nl,

  loop.

preamble :-
  write('.-------------------------------------------.'), nl,
  write('| Welcome to a lambda-calculus interpreter! |'), nl,
  write('|                                           |'), nl,
  write('| INPUT examples:                           |'), nl,
  write('| - x; x y; x (y z); \\x.x y; \\a (\\b.b c) d  |'), nl,
  write('| OUTPUT:                                   |'), nl,
  write('| - [tok]ens; parse [tree]; compound [data];|'), nl,
  write('|   alpha/beta [redu]ction; conversion back |'), nl,
  write('|   to input syntax.                        |'), nl,
  write('| - Absent output step means failure.       |'), nl,
  write('| START:                                    |'), nl,
  write('| - `swipl lambda.pl` OR                    |'), nl,
  write("| - `echo 'INPUT' | swipl lambda.pl` OR     |"), nl,
  write('| - `sh test.sh`                            |'), nl,
  write('| QUIT:                                     |'), nl,
  write('| - C-d on empty OR double C-c              |'), nl,
  write("'-------------------------------------------'"), nl.

run :-
  %preamble,
  loop,
  halt.
run :- halt(1).

