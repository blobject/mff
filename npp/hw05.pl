% for : nprg005
% date: 27 mar 2019

% use_module(library(clpfd)).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 1. jack o' lanterns
%%
%% NOTE: Does not work.
%%       Tracing loop/2 shows that prolog always chooses indexes 2, 4,
%%       3, then 2, 3 repeatedly. I cannot figure out how to break out
%%       of this determinism.

%% diff(?L, ?M, ?N)
%% Find the index N whereat L and M differ.

diff(L, M, N) :-
  same_length(L, M),
  dif(L, M),
  nth1(N, L, X),
  nth1(N, M, Y),
  select(X, L, Y, M).

%% toggling

toggle(l, o).
toggle(o, l).

% Toggle one pumpkin, returning the index.

toggle_one(L, M, N) :-
  toggle(X, Y),
  select(X, L, Y, M),
  diff(L, M, N).

% Toggle one pumpkin, at an index.

toggle_at(L, N, M) :-
  toggle(X, Y),
  select(X, L, Y, M),
  diff(L, M, N),
  nth1(N, M, Y).

%% boop(+L, -M)
%% Make a move in the puzzle, ie. toggle 3 adjacent pumpkins.

boop(L, M) :-
  toggle_one(L, T, N),
  length(L, A),
  X is (N - 1) mod A,
  Y is (N + 1) mod A,
  toggle_at(T, X, U),
  toggle_at(U, Y, M).

%% loop(+S, +T)
%% Make a path of moves, ie. a loop of boops.

loop(S, S).
loop(S, T) :-
  boop(S, S1),
  loop(S1, T).

%% solve

% Convenience predicate for creating a circle of pumpkins.

rep(X, N, L) :-
  length(L, N),
  maplist(=(X), L).

% The puzzle.

pumpkin(Initial) :-
  length(Initial, L),
  L > 3,
  rep(o, 7, O),
  loop(Initial, O).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 2. which bottle
%%
%% NOTE: Very incomplete.

quaff(X) :-
  Drinks = [Poison, Wine, Water, Hinderer, Helper],
  Hinderer in 2..8,
  Helper in 1..9,
  all_distinct(Drinks),
  Large  in 1 \/ 3 \/ 6 \/ 8,
  Medium in 4 \/ 7,
  Small  in 2 \/ 5 \/ 9,
  Poison #\= Medium,
  Wine #\= Large,
  Poison #= Poison \/ (Helper + 1),
  ((Wine #= Hinderer - 1, Water #= Hinderer + 1);
   (Wine #= Hinderer + 1, Water #= Hinderer - 1)),
  X = Helper.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% fin

