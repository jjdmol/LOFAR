#!/bin/bash

for tnode in head{01..02}.cep4.lofar
do
for fnode in cpu{01..50}.cep4.lofar
do
    ./addtoQPIDDB.py -b $fnode -q lofar.task.feedback.dataproducts -f $tnode -e lofar.default.bus
done
./addtoQPIDDB.py -b$tnode -q lofar.task.feedback.dataproducts -f ccu001.control.lofar -e lofar.default.bus
done


