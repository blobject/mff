-- for : nprg005
-- date: 04 apr 2019

import Data.List  -- sort

------------------------------------------------------------------------
-- 1. collatz sequence

-- collatz: Entry point of solution.
--          Calls `collatz'` with a default counter of 1.

collatz n = collatz' n 1

-- collatz': Return the smallest head `i` of collatz sequence `C` such
--           that `length C` is at least `n`.

collatz' n i =
  if n <= length (collatzen i)
    then i
    else collatz' n (i + 1)

-- collatzen: Generate the collatz sequence that begins with `n`.

collatzen 1 = [1]

collatzen n =
  if mod n 2 == 0
    then (n : collatzen (div n 2))
    else (n : collatzen (n * 3 + 1))

------------------------------------------------------------------------
-- 2. n queens

{-
NOTE1: (time complexity)
       `sort`    : ? O(nlogn)
       `distinct`: O(nlogn) = O(sort + n)
       `pinup`   : O(n)
       `pindown` : O(n)
       `pin`     : O(n^2) = O(n * (pinup + pindown))
       `queens`  : O(n^2) = O(distinct + pin)

NOTE2: Each elem in input (list of n ints) assumed to be in [1..n].
-}

-- queens: Entry point of solution.
--         Applies horizontal constraint, then the diagonal by calling
--         `not pin`. Vertical constraint is implicit in the encoding
--         of the input.

queens [] = True

queens [_] = True

queens l =
  distinct l && not (pin (reverse l))

-- pin: Diagonal negative constraint.
--      Recursively determine coincidence of a queen and the line of
--      attack of another queen.

pin [] = False

pin [_] = False

pin (q:qs) =
  pinup q qs || pindown q qs || pin qs

-- pinup   &   pindown:
--
-- | |x| | |   | | | | |
-- | | |x| |   | | | |Q|
-- | | | |Q|   | | |x| |
-- | | | | |   | |x| | |

pinup 0 _ = False

pinup _ [] = False

pinup q (x:xs) =
  (q - 1) == x || pinup (q - 1) xs

pindown _ [] = False

pindown q (x:xs) =
  (q + 1) == x || pindown (q + 1) xs

-- distinct: Horizontal positive constraint.
--           Determine whether all elements in list `l` is unique.

distinct l = distinct' (sort l)

distinct' [_] = True

distinct' (x:xs) =
  x /= (head xs) && distinct' xs

------------------------------------------------------------------------
-- fin

