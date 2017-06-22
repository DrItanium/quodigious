#!/bin/bash
width=$1
next=$(( width + 1))
begin=$((width * 49))
end=$((next * 49))
./compute-range19 $begin $end
