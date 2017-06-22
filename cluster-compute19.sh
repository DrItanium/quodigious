#!/bin/bash

for ((i=$1; i<=$2; i++)); do
    ./compute19_work_unit.sh $1 > /srv/repo/data/quodigious/n19_wu$i
done
