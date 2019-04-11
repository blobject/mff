% for : nprg005
% date: 03 apr 2019

% use_module(library(clpfd)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 1. binary trees

%% maptree(+P, ?T)

maptree(_, nil).

maptree(P, t(L, X, R)) :-
  call(P, X),
  maptree(P, L),
  maptree(P, R).

%% maptree(+P, ?T1, ?T2)

maptree(_, nil, nil).

maptree(P, t(L, X, R), t(M, Y, S)) :-
  call(P, X, Y),
  maptree(P, L, M),
  maptree(P, R, S).

%% size(?T, ?N, ?H)

size(nil, 0, -1).

size(t(L, _, R), N, H) :-
  N #>= 0,
  H #>= 0,
  Nl #>= 0,
  Nr #>= 0,
  Hl #>= -1,
  Hr #>= -1,
  N #= 1 + Nl + Nr,
  H #= 1 + max(Hl, Hr),
  size(L, Nl, Hl),
  size(R, Nr, Hr).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 2. regular expressions

%% match(+E, ?L)

match([], []).

match([one(E) | R], L) :-
  append(E, M, L),
  match(R, M).

match([star(E) | R], L) :-
  append(E, M, L),
  match([star(E) | R], M).

match([star(_) | R], L) :-
  match(R, L).

match([plus(E) | R], L) :-
  append(E, M, L),
  match([star(E) | R], M).

match([opt(E) | R], L) :-
  append(E, M, L),
  match(R, M).

match([opt(_) | R], L) :-
  match(R, L).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 3. jealous husbands

% problem not attempted :-(

