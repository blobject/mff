% for : nprg005
% date: 01 mar 2019

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 1. distinct_factors(+N, ?F)

distinct_factors(N, F) :-
  % TODO: distinct_factors(252, X) does not work
  all_factors(N, L),
  count(L, F).

all_factors(1, _).

all_factors(N, [H | L]) :-
  N1 is N div H,
  all_factors(N1, L).

count([], 0).

count([H, H | L], C) :-
  count([H | L], C).

count([H1, H2 | L], C) :-
  C1 is C - 1,
  count(L, C1).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 2. split(?L, ?M, ?N)

split(L, M, N) :-
  append(M, N, X),
  permutation(X, L),
  orderly(L, M),
  orderly(L, N).

orderly(_, []).

orderly([Lh | L], [Mh | M]) :-
  Lh = Mh,
  orderly(L, M).

orderly([Lh | L], [Mh | M]) :-
  dif(Lh, Mh),
  orderly(L, [Mh | M]).

% TODO: `split([a,b,c,d], X, [b,d])` does not end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 3. merge(+L1, +L2, ?M)

merge(L1, L2, M) :-
  % TODO: still accepts duplicates
  m(L1, L2, X),
  u(X, M).

% Simple merge, base cases.
m([], L2, L2).
m(L1, [], L1).

% Head of L1 matches M's and is less than L2's.
m([H1 | L1], [H2 | L2], [H1 | M]) :-
  H1 < H2,
  m(L1, [H2 | L2], M).

% Head of L2 matches M's and is less than L1's.
m([H1 | L1], [H2 | L2], [H2 | M]) :-
  H1 > H2,
  m([H1 | L1], L2, M).

% All heads match.
m([H | L1], [H | L2], [H | M]) :-
  m(L1, L2, M).

% Uniquify list, base case.
u([], []).

% Remove duplicates.
u([H, H | L], U) :-
  u([H | L], U).

% Match heads.
u([H | L], [H | U]) :-
  u(L, U).

