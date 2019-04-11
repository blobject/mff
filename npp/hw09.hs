-- by  : bin joo
-- for : nprg005
-- date: 25 apr 2019

import Data.Char

------------------------------------------------------------------------
-- 1. codebreaking

-- count occurrence of a char in a (curried) string
count :: Char -> String -> Int
count c =
  let f x sum | x == c = sum + 1
              | otherwise = sum
  in foldr f 0

-- given a ciphertext, calculate frequency-per-letter
observe :: String -> [Float]
observe text =
  [let c = fromIntegral (count (toLower x) text)
       l = fromIntegral (length text)
   in c * 100.0 / l | x <- ['a'..'z']]

-- rotate a list leftwards
rotate :: Int -> [a] -> [a]
rotate n xs =
  let m = mod n (length xs)
  in drop m xs ++ take m xs

-- "rotate" a string "upwards" (eg. "Xyz" -> "Yza")
slur :: Int -> String -> String
slur n cs =
  let shift c offset = chr ((mod (ord c - offset + n) 26) + offset)
      f c | c >= 'A' && c <= 'Z' = shift c 65
          | c >= 'a' && c <= 'z' = shift c 97
          | otherwise = c
  in map f cs

-- the chi-squared statistic
statistic :: [Float] -> [Float] -> Float
statistic os es =
  sum [(o - e)^2 / e | (o, e) <- zip os es]

-- given a ciphertext, calculate statistic for each rotation of list of
-- expected frequencies (which is equivalent to calculating statistic of
-- the expected frequencies against each "slur")
scores :: String -> [Float] -> [(Float, Int)]
scores text es =
  map (\n -> (statistic (observe text) (rotate n es), n)) [0..25]

-- expected letter frequencies for English
en :: [Float]
en = [8.2, 1.5, 2.8, 4.3, 12.7, 2.2, 2.0, 6.1, 7.0, 0.2, 0.8, 4.0, 2.4,
      6.7, 7.5, 1.9, 0.1, 6.0, 6.3, 9.1, 2.8, 1.0, 2.4, 0.2, 2.0, 0.1]

-- problem solution
crack :: String -> String
crack text =
  slur (snd (minimum (scores text en))) text

------------------------------------------------------------------------
-- fin

