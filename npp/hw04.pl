% for : nprg005
% date: 16 mar 2019

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 1. matrix transposition

% headhunt(?M, ?H, ?T)
% Scrape off heads of M, dump it into H, and have T left over.

headhunt([], [], []).

headhunt([[H | T] | M], [H | Hs], [T | Ts]) :-
  headhunt(M, Hs, Ts).

% transpose(+M1, ?M2)

transpose(M1, []) :-
  % Each inner list of the nested list M1 is empty.
  maplist(=([]), M1).

transpose(M1, [R | M2]) :-
  headhunt(M1, R, T),
  transpose(T, M2).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 2. back substitution

% drop(+L, +N, ?M)
% Drop N heads from list L, where 0 < N < len(L) - 1.

drop(X, 0, X).

drop([_ | L], N, M) :-
  dif(L, []),
  N1 is N - 1,
  drop(L, N1, M).

% back_sub(+A, +B, ?X)

back_sub(_, [X], [X]).

back_sub([R | A], [S | B], [V | X]) :-
  % Recurse.
  back_sub(A, B, X),
  % Drop zeroes and pivot.
  length(B, Lb),
  length(R, Lr),
  L is Lr - Lb,
  drop(R, L, Y),
  % Calculate the next (ie. previous) x (ie. V)
  dot(X, Y, Z),
  V is S - Z.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 3. matrix multiplication

% mul_row(+M1, +V1, ?V2)

mul_row([], _, []).

mul_row([R | M1], V1, [V | V2]) :-
  dot(R, V1, V),
  mul_row(M1, V1, V2).

% mul_mm(+M1, +M2, ?N)

mul_mm(M1, M2, N) :-
  transpose(M2, T),
  maplist(mul_row(M1), T, X),
  transpose(X, N).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% common

% dot(+L, +M, ?V)
% Dot product of list L and list M.

dot([], [], 0).

dot([I | L], [J | M], V) :-
  dot(L, M, V1),
  V is V1 + I * J.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% fin
