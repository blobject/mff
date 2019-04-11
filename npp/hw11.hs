-- for : nprg005
-- date: 8 may 2019

import Data.Set (toList, fromList)

------------------------------------------------------------------------
-- 1. tautologies

data Prop = Const Bool
          | Var Char
          | Not Prop
          | And Prop Prop
          | Or Prop Prop
          deriving Show

type Assignment = [(Char, Bool)]

-- vars: Recursively extract Vars from a Prop, including repeats.

vars :: Prop -> [Char]
vars (Var c) = [c]
vars (Not p) = vars p
vars (And p q) = vars p ++ vars q
vars (Or p q) = vars p ++ vars q
vars _ = ""

-- eval: Recursively evaluate Prop.

eval :: Assignment -> Prop -> Bool
eval _ (Const b) = b
eval t (Var c) = (snd . head) (filter (\x -> c == fst x) t)
eval t (Not p) = not (eval t p)
eval t (And p q) = eval t p && eval t q
eval t (Or p q) = eval t p || eval t q

-- uniq: Find unique elements from list.

uniq :: [Char] -> [Char]
uniq =
  toList . fromList

-- truths: Given a list of Chars, generate its truth table, ie. the
--         list of their assignments.

truths :: String -> [Assignment]
truths [] = [[]]
truths (c:cs) = [a:as
                | a <- [(c, b) | b <- [True, False]],
                  as <- truths cs]

-- isTaut: Apply conjunction on all evaluations of assignments against
--         the given Prop.

isTaut :: Prop -> Bool
isTaut p =
  and [eval a p | a <- truths (uniq (vars p))]

------------------------------------------------------------------------
-- 2. all balanced binary trees

data Tree = Nil | Node Tree Int Tree
  deriving (Eq, Ord, Show)

--anyBalanced :: Int -> Int -> [Tree]
anyBalanced m n f
  | m == n = Node Nil n Nil
  | otherwise =
      let mid = f (fromIntegral (m + n) / 2)
          l | mid == m = Nil
            | otherwise = anyBalanced m (mid - 1) f
          r | mid == n = Nil
            | otherwise = anyBalanced (mid + 1) n f
      in Node l mid r

--allBalanced :: Int -> [Tree]
allBalanced n =
  [anyBalanced 1 n f | f <- [floor, ceiling]]

------------------------------------------------------------------------
-- fin

