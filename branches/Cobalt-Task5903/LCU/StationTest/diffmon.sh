#!/bin/bash
#
# check the DIFF value with rspctl --status.
# only values with 0 and 512 are allowed for 200MHz clock (0 for 160MHz)
# No output when the DIFF values are okay !!
# 
# 12-3-12, M.J.Norden; original line Michiel Brentjes



rspctl --status 2>&1 |grep -A 4 diff |grep -v ERROR|grep -v diff|grep -v '   0   '|grep -v '   512   '





