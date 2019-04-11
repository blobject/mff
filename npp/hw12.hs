-- for : nprg005
-- date: 12 may 2019

------------------------------------------------------------------------
-- 1. arithmetic expressions

data Op = Plus | Minus | Times
  deriving (Eq, Ord)

instance Show Op where
  show Plus = "+"
  show Minus = "-"
  show Times = "*"

data Exp = Const Int | Oper Op Exp Exp
  deriving (Eq, Ord)

instance Show Exp where
  show (Const n) = show n
  show (Oper op l r) =
    brackets l ++ " " ++ show op ++ " " ++ brackets r
    where brackets (Const n) = show n
          brackets e = "(" ++ show e ++ ")"

ops :: [Op]
ops = [Plus, Minus, Times]

arit' :: [Exp] -> [Exp]
arit' e@[Const _] = e
arit' es =
  [Oper o x y | o <- ops
              , index <- [1 .. length es - 1]
              , x <- arit' (take index es)
              , y <- arit' (drop index es)]

eval :: Exp -> Int
eval (Const x) = x
eval (Oper Plus e f) = (eval e) + (eval f)
eval (Oper Minus e f) = (eval e) - (eval f)
eval (Oper Times e f) = (eval e) * (eval f)

arit :: [Int] -> Int -> [Exp]
arit xs sol
  | length xs == 0 = []
  | length xs == 1 && head xs /= sol = []
  | otherwise = filter (\a -> (eval a) == sol) (arit' (map Const xs))

------------------------------------------------------------------------
-- 2. dividing polynomials

type Poly = [Double]

-- degree
d :: Poly -> Int
d p = length p - 1

-- remove leading zeroes
mislead :: [Double] -> Poly
mislead p@(x:xs) | x /= 0 = p
                 | xs == [] = []
                 | otherwise = mislead xs

divide' :: Poly -> Poly -> Poly -> Int -> (Poly, Poly)
divide' a divisor q depth
  | d a < d divisor = (q, a)
  | otherwise =
    let coefficient = head a / head divisor
        quotient = take depth q ++
                   coefficient : take ((d a) - (d divisor)) [0,0..]
        product = map (* coefficient) divisor ++
                  take (d quotient - depth) [0,0..]
        dividend = mislead [x - y | (x, y) <- zip a product]
    in divide' dividend divisor quotient (depth + 1)

divide :: Poly -> Poly -> (Poly, Poly)
divide a b =
  divide' a b [] 0

------------------------------------------------------------------------
-- fin

