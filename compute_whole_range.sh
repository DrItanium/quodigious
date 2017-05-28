#!/bin/bash

time echo "$1 2" | ./quodigious_single_top > qnums$1_2
time echo "$1 3" | ./quodigious_single_top > qnums$1_3
time echo "$1 4" | ./quodigious_single_top > qnums$1_4
time echo "$1 6" | ./quodigious_single_top > qnums$1_6
time echo "$1 7" | ./quodigious_single_top > qnums$1_7
time echo "$1 8" | ./quodigious_single_top > qnums$1_8
time echo "$1 9" | ./quodigious_single_top > qnums$1_9
