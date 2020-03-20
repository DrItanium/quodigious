#!/bin/bash

cat outputs/qnums*_last$1 | sort | uniq > UniqueLast$1Digits
gcc -P -E -DWIDTH=$1 unique_digits_generator.c > unique$1.clp
maya -f2 AnalyzedNumber.clp -f2 unique$1.clp -f2 reset-run-exit.clp > Specialization$1Digits.cc
