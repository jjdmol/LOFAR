#!/bin/bash

grep ', lost' $1 | awk '{ ( $10 == "subbands," && $13 == "subbands.") ?  sum += $12/($9 + $12) : missed += 1}  END { print "Output loss: " 100*sum/(NR - missed) " %" }'
