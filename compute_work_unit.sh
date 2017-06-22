#!/bin/bash

echo $1 $2 | ./quodigious | sort > rnums_$1_$2
