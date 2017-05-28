#!/bin/bash

echo "$1 2" | ./quodigious_single_top > tmp/qnums$1_2 &
echo "$1 3" | ./quodigious_single_top > tmp/qnums$1_3 &
echo "$1 4" | ./quodigious_single_top > tmp/qnums$1_4 &
echo "$1 6" | ./quodigious_single_top > tmp/qnums$1_6 &
echo "$1 7" | ./quodigious_single_top > tmp/qnums$1_7 &
echo "$1 8" | ./quodigious_single_top > tmp/qnums$1_8 &
echo "$1 9" | ./quodigious_single_top > tmp/qnums$1_9 &
wait
