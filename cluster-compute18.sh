#!/bin/bash

for ((i=$1; i<=$2; i++)); do
    ./compute18_work_unit.sh $1 > /srv/repo/data/quodigious/n18_wu$i
done
