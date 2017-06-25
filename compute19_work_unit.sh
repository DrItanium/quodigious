#!/bin/bash
width=$1
next=$(( width + 1))
begin=$((width * 2401))
end=$((next * 2401))
./compute-range19 $begin $end
