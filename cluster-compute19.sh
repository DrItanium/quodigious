#!/bin/bash

for ((j=$1; j<=$2; j++)); do
    ./compute19_work_unit.sh $j > /srv/repo/data/quodigious/n19_wu$j
done
