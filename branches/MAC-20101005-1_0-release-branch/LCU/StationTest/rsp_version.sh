#!/bin/bash

#
# Get version info from the RSP boards and compare this with the expected golden result.
#

rm -f *.log
rm -f *.diff
rspctl --version > rsp_version.log
diff rsp_version.log gold/rsp_version.gold > rsp_version.diff
if [ -e rsp_version.log ] && [ -e gold/rsp_version.gold ] && [ -e rsp_version.diff ] && ! [ -s rsp_version.diff ]; then
  # The files exists AND the diff has size 0
  echo "RSP version test went OK"
else
  echo "RSP version test went wrong"
fi
