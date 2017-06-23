#!/bin/bash
width=$1
next=$(( width + 1))
begin=$((width * 343))
end=$((next * 343))
./compute-range18 $begin $end
