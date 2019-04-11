% for : nprg005
% date: 09 mar 2019

% 1. group(+L, ?M)
% repeated elements

group([], []).

group([X], [[1, X]]).

group([X, Y | L], [[1 | [X]] | M]) :-
  dif(X, Y),
  group([Y | L], M).

group([X, X | L], [[N | [X]] | M]) :-
  group([X | L], [[N1 | [X]] | M]),
  N is N1 + 1.

% 2. better(?H1, ?H2)
% poker hand inequality
% note: five of a kind is legal

val(2, 2). val(3, 3). val(4, 4). val(5, 5).
val(6, 6). val(6, 6). val(7, 7).
val(8, 8). val(9, 9). val(10, 10).
val(j, 11). val(q, 12). val(k, 13). val(a, 14).

arrange(L, M) :-
  maplist(val, L, X),
  sort(0, @>=, X, Y),
  group(Y, Z),
  sort(0, @>=, Z, M).

better(H1, H2) :-
  length(H1, 5),
  length(H2, 5),
  arrange(H1, A),
  arrange(H2, B),
  arrangedly_better(A, B).

arrangedly_better([], []) :-
  false.

arrangedly_better([[X | _] | _], [[Y | _] | _]) :-
  X > Y.

arrangedly_better([[X | P] | _], [[Y | Q] | _]) :-
  X = Y,
  P > Q.

arrangedly_better([[X | P] | A], [[Y | Q] | B]) :-
  X = Y,
  P = Q,
  arrangedly_better(A, B).

