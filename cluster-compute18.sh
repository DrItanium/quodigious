#!/bin/bash

for ((j=$1; j<=$2; j++)); do
    ./compute18_work_unit.sh $j > /srv/repo/data/quodigious/n18_wu$j
done
