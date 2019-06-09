#! /bin/sh

PROLOG='swipl'
SCRIPT='./lambda.pl'

t()
{
  step="$1"
  inp="$2"
  exp="$3"
  got=$(echo "$inp" \
    | $PROLOG $SCRIPT 2>/dev/null \
    | grep "^$step: " \
    | cut -d ' ' -f 2- \
    | sed 's/\\/\\\\/g') # for proper grep and echo
  if echo "$got" | grep -q "^$exp$"; then
    echo -n "GOOD: $inp\t--> "
    if [ -z "$got" ]; then
      echo 'failed as expected'
    else
      echo "$got"
    fi
  else
    echo "BAD!! $inp\n  EXP: $exp\n  GOT: $got"
  fi
}

test_data()
{
  echo '---- phase: parse into data ----'
  t data 'x' 'sym(x)'
  t data 'x y' 'app(sym(x),sym(y))'
  t data '(x y) (a b)' 'app(app(sym(x),sym(y)),app(sym(a),sym(b)))'
  t data '\\x.x' 'fun(sym(x),sym(x))'
  t data '\\x.a b c' 'fun(sym(x),app(app(sym(a),sym(b)),sym(c)))'
  t data '\\x.a (b c)' 'fun(sym(x),app(sym(a),app(sym(b),sym(c))))'
  t data '\\y.(\\x.x)y z' 'fun(sym(y),app(app(fun(sym(x),sym(x)),sym(y)),sym(z)))'
  t data '\\x.a\\b.c(z y)' 'fun(sym(x),app(sym(a),fun(sym(b),app(sym(c),app(sym(z),sym(y))))))'
  t data '(((x)))' 'sym(x)'
  t data '((x))((y))' 'app(sym(x),sym(y))'
  t data 'xx' 'sym(xx)'
  t data 'a    b' 'app(sym(a),sym(b))'
  t data '(a)(b)' 'app(sym(a),sym(b))'
  t data '(' ''
  t data ')' ''
  t data '()' ''
  t data ')(' ''
  t data '\\' ''
  t data '\\x' ''
  t data '\\x.' ''
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
}

test_result()
{
  echo '---- phase: revert to canon ----'
  t result 'x' 'x'
  t result '(\\x.x y)(a b)' '((a b) y)'
}

test_more()
{
  # BEWARE: Equality checking is paren-syntax-sensitive! Checking "redu"
  #         would be less ambiguous (but also less legible).

  # numerals
  plus='\\m.\\n.\\f.\\x.((m f)((n f) x))'
  mult='\\m.\\n.\\f.\\x.(m (n f) x)'
  exp='\\m.\\n.(n m)' # does not work :-(
  one='\\f.\\x.(f x)'
  two='\\f.\\x.(f (f x))'
  three='\\f.\\x.(f (f (f x)))'
  four='\\f.\\x.(f (f (f (f x))))'

  # combinators
  I='\\x.x'
  K='\\x.\\y.x'
  S='\\x.\\y.\\z.(x z (y z))'
  B='\\f.\\g.\\x.(f (g x))'
  C='\\f.\\g.\\x.((f x) g)'

  echo '---- more ----'
  echo '"+ 2 2" = 4'; t result "($plus)($two)($two)" "$four"
  echo '"* 1 2" = 2'; t result "($mult)($one)($two)" "$two"
  echo '"+ (+ 1 1) 1" = 3'; t result "($plus)(($plus)($one)($one))($one)" "$three"
  echo '"S K K x" = x'; t result "($S)($K)($K)x" 'x'
  echo '"C I x y" = y x'; t result "($C)($I)x y" '(y x)'
}

echo 'starting tests'
test_data
test_redu
test_rere
test_result
test_more

