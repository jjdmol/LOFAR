#!/bin/sh
# 1.0 check the GPS reception
# 3-4-09 M.J. Norden

echo "check GPS reception"
tail -f /var/log/ntpstats/clockstats

