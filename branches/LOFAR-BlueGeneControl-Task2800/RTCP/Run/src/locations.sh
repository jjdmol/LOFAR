function isproduction() {
  [ "lofarsys" == "$USER" ]
}

TIMESTAMP=`date +%Y-%m-%d_%H%M%S`

if isproduction
then
  ISPRODUCTION=1

  CNPROC=$HOME/production/lofar/bgp_cn/bin/CN_Processing
  IONPROC=$HOME/production/lofar/bgp_ion/bin/ION_Processing

  LOGDIR=$HOME/log/L$TIMESTAMP
  LOGSYMLINK=$HOME/log/latest
else
  ISPRODUCTION=0

  CNPROC=$HOME/projects/LOFAR/installed/bgp_cn/bin/CN_Processing
  IONPROC=$HOME/projects/LOFAR/installed/bgp_ion/bin/ION_Processing

  LOGDIR=$HOME/projects/LOFAR/L$TIMESTAMP
  LOGSYMLINK=$HOME/projects/LOFAR/log
fi
