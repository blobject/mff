-- for : nprg005
-- date: 30 apr 2019

import Data.Set (toList, fromList)

------------------------------------------------------------------------
-- 1. ranked-choice voting

-- count occurrence of a string in a list of strings
count :: String -> [String] -> Int
count x =
  let f y sum | y == x = sum + 1
              | otherwise = sum
  in foldr f 0

-- fill with dummy candidates up to largest ballot size
-- note: assumes no candidate will have name "" (empty string)
bloat :: [[String]] -> [[String]]
bloat bs =
  let n = maximum (map length bs)
  in [take n (b ++ repeat "") | b <- bs]

-- return unique elements of a list of lists
uniq :: [[String]] -> [String]
uniq =
  concat . toList . fromList

-- recursively eliminate and prepend worst candidates
till :: [String] -> [[String]] -> [String]
till [] _ = []
till cs bs =
  let heads = map head bs
      loser = snd (minimum [(count c heads, c) | c <- cs])
      nbs = [filter (/= loser) b | b <- bs]
      ncs = filter (/= loser) cs
  in loser : till ncs (bloat nbs)

-- problem solution
elect :: [[String]] -> [String]
elect ballots =
  let candidates = uniq (ballots)
  in till candidates (bloat ballots)

------------------------------------------------------------------------
-- fin

