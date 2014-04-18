#!/bin/bash
multitail \
  --config $LOFARROOT/etc/watchlogs-multitail.conf \
  -cS cobalt -fr cobalt -iw $LOFARROOT/var/log/'rtcp-*.log' 10 \
  -cS startBGL -fr startBGL -wh 6 -f --retry $LOFARROOT/var/log/startBGL.log
