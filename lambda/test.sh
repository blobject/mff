#! /bin/sh

# file: lambda/test.sh
# by  : jooh@cuni.cz
# for : nprg005

PROLOG='swipl'
SCRIPT='./lambda.pl'

t()
{
  step="$1"
  input="$2"
  expect="$3"
  got=$(echo "$input" \
    | $PROLOG $SCRIPT 2>/dev/null \
    | grep "^$step: " \
    | cut -d ' ' -f 2- \
    | sed 's/\\/\\\\/g') # for proper grep and echo
  if echo "$got" | grep -q "^$expect$"; then
    echo -n "GOOD: $input\t--> "
    if [ -z "$got" ]; then
      echo 'failed as expected'
    else
      echo "$got"
    fi
  else
    echo "BAD!! $input\n  EXP: $expect\n  GOT: $got"
  fi
}

test_tree()
{
  echo '---- phase: string-to-tree ----'
  t tree 'xx' '\[xx\]'
  t tree 'a    b' '\[a,b\]'
  t tree '(' ''
  t tree ')' ''
}

test_data()
{
  echo '---- phase: string-to-tree-to-data ----'
  t data 'x' 'sym(x)'
  t data '_A"str@nge123/var' 'sym(_A"str@nge123/var)'
  t data '(((x)))' 'sym(x)'
  t data '((x))((y))' 'app(sym(x),sym(y))'
  t data 'x y' 'app(sym(x),sym(y))'
  t data '(x)(y)' 'app(sym(x),sym(y))'
  t data '(x)(y)(z)' 'app(app(sym(x),sym(y)),sym(z))'
  t data '(x y)(a b)(u v)' 'app(app(app(sym(x),sym(y)),app(sym(a),sym(b))),app(sym(u),sym(v)))'
  t data '(\x.x)(\y.y)(\z.z)' 'app(app(fun(sym(x),sym(x)),fun(sym(y),sym(y))),fun(sym(z),sym(z)))'
  t data '\\x.x' 'fun(sym(x),sym(x))'
  t data '\\x.a b c' 'fun(sym(x),app(app(sym(a),sym(b)),sym(c)))'
  t data '\\x.a (b c)' 'fun(sym(x),app(sym(a),app(sym(b),sym(c))))'
  t data '\\y.(\\x.x)y z' 'fun(sym(y),app(app(fun(sym(x),sym(x)),sym(y)),sym(z)))'
  t data '\\x.a\\b.c(z y)' 'fun(sym(x),app(sym(a),fun(sym(b),app(sym(c),app(sym(z),sym(y))))))'
}

test_fail()
{
  echo '---- phase: to-data fail ----'
  t data '()' ''
  t data ')(' ''
  t data '\\' ''
  t data '\\x' ''
  t data '\\x.' ''
  t data '\(.)' ''
  t data '\x(.)y' ''
  t data '(\x.)y' ''
  #t data '\(x).y' '' # TODO: this should fail
}

test_redu()
{
  echo '---- phase: reduce (without rename) ----'
  t redu '(\\x.x)2' 'sym(2)'
  t redu '(\\a.x)b' 'sym(x)'
  t redu '(\\x.x y)2' 'app(sym(2),sym(y))'
  t redu '(\\x.x)(a b)' 'app(sym(a),sym(b))'
  t redu '(\\x.x y)(a b)' 'app(app(sym(a),sym(b)),sym(y))'
  t redu '(\\x.x)(\\y.y)' 'fun(sym(y),sym(y))'
  t redu '(\\x.x x)(\\a.b c)' 'app(sym(b),sym(c))'
}

test_rere()
{
  echo '---- phase: rename and reduce ----'
  t redu '(\\x.\\y.x x)(\\x.y)' 'fun(sym(y0),sym(y))'
  t redu '(\\x.\\y.\\z.x y z)(\\x.x x)(\\x.x)x' 'sym(x)'
  t redu '(\\x.((\\y.(+ 2 y))(- x y)))z' 'app(app(sym(+),sym(2)),app(app(sym(-),sym(z)),sym(y)))'
  t redu '((\x.x)(\y.y))((\z.z)(\w.w))' 'fun(sym(w),sym(w))'
}

test_result()
{
  echo '---- phase: revert to canon ----'
  t result 'x' 'x'
  t result '(\\x.x y)(a b)' '((a b) y)'
}

test_more()
{
  # BEWARE: Equality checking is paren-sensitive! Checking "redu" would
  #         be less ambiguous, but also less legible.

  # numerals
  plus='\\m.\\n.\\f.\\x.((m f)((n f) x))'
  mult='\\m.\\n.\\f.\\x.(m (n f) x)'
  exp='\\m.\\n.(n m)'
  one='\\f.\\x.(f x)'
  two='\\f.\\x.(f (f x))'
  three='\\f.\\x.(f (f (f x)))'
  four='\\f.\\x.(f (f (f (f x))))'
  one_alias='\\x.\\x0.(x x0)'
  four_alias='\\x.\\x0.(x (x (x (x x0))))'

  # combinators
  I='\\x.x'
  K='\\x.\\y.x'
  S='\\x.\\y.\\z.(x z (y z))'
  B='\\f.\\g.\\x.(f (g x))'
  C='\\f.\\g.\\x.((f x) g)'

  echo '---- more ----'
  echo '"+ 2 2" = 4'; t result "($plus)($two)($two)" "$four"
  echo '"+ (+ 1 1) 1" = 3'; t result "($plus)(($plus)($one)($one))($one)" "$three"
  echo '"* 1 2" = 2'; t result "($mult)($one)($two)" "$two"
  echo '"^ 2 2" = 4'; t result "($exp)($two)($two)" "$four_alias"
  echo '"^ 1 3" = 1'; t result "($exp)($one)($three)" "$one_alias"
  echo '"S K K x" = x'; t result "($S)($K)($K)x" 'x'
  echo '"C I x y" = y x'; t result "($C)($I)x y" '(y x)'
}

echo 'starting tests'
test_tree
test_data
test_fail
test_redu
test_rere
test_result
test_more

