-- for : nprg005
-- date: 17 apr 2019

------------------------------------------------------------------------
-- 1. sequences

-- (a) all sequences

all_sequences :: [a] -> [[a]]

all_sequences [] = [[]]

all_sequences s@(x:xs) = [] : [y:ys | y <- s, ys <- all_sequences xs]

-- (b) Post's correspondence problem

------------------------------------------------------------------------
-- 2. rod cutting

------------------------------------------------------------------------
-- 3. extended rod cutting

------------------------------------------------------------------------
-- fin

