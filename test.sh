#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}


assert 21 "5+20-4;"
assert 41 "12 + 34 - 5;"
assert 42 "6*7;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 2 "6/3;"
assert 4 "(3+5)/2;"
assert 3 "1 + +2;"
assert 7 "10 + -3;"
assert 13 "10 - -3;"
assert 14 "(10 % (7-3))*7;"
assert 0 "35+65==50*2+(1+1);"
assert 1 "35+65==100;"
assert 1 "35+65!=50*2+(1+1);"
assert 0 "35+65!=100;"
assert 1 "100<101;"
assert 0 "100<100;"
assert 1 "99<=100;"
assert 1 "100<=100;"
assert 1 "100<=101;"
assert 1 "201>200;"
assert 0 "200>200;"
assert 1 "200>=200;"
assert 0 "199>=200;"
assert 42 "10+32;"
assert 3 "a=3;"
assert 15 "b=3*4+3;"
#assert 10 "b=a=10;"
#assert 20 "a=18;a+2;"
#assert 1 "a=b=2;"


echo OK
