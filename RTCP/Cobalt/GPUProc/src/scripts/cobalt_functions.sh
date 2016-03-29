
function addlogprefix {
  ME="`basename -- "$0" .sh`@`hostname`"
  while read LINE
  do
    echo "$ME" "`date "+%F %T.%3N"`" "$LINE"
  done
}

#
# The following functions assume that $PARSET is set.
#

function getkey {
  KEY=$1
  DEFAULT=$2

  # grab the last key matching "^$KEY=", ignoring spaces.
  VALUE=`<$PARSET perl -ne '/^'$KEY'\s*=\s*"?(.*?)"?\s*$/ || next; print "$1\n";' | tail -n 1`

  if [ "$VALUE" == "" ]
  then
    echo "$DEFAULT"
  else
    echo "$VALUE"
  fi
}

function setkey {
  KEY=$1
  VAL=$2

  # In case already there, comment all out to avoid stale warnings. Then append.
  KEYESC=`echo "$KEY" | sed -r -e "s/([\.[])/\\\\\\\\\1/g"`  # escape '.' '[' chars in keys with enough '\'
  sed -i --follow-symlinks -r -e "s/^([[:blank:]]*$KEYESC[[:blank:]]*=)/#\1/g" "$PARSET"
  echo "$KEY = $VAL" >> "$PARSET"
}

function read_cluster_model {
  CLUSTER_NAME=$(getkey Observation.Cluster.ProcessingCluster.clusterName "")

  # Hack to derive required properties (cluster model) from cluster name.
  case "${CLUSTER_NAME}" in
    CEP4)
      HEADNODE=head01.cep4.control.lofar
      #COMPUTENODES="`seq -f "cpu%02.0f.cep4" 1 50`"
      COMPUTENODES="`seq -f "cpu%02.0f.cep4" 1 26` `seq -f "cpu%02.0f.cep4" 30 35` `seq -f "cpu%02.0f.cep4" 37 39`"
      NRCOMPUTENODES=50

      GLOBALFS_DIR=/data

      #SLURM=true
      SLURM=false # Don't use SLURM for now, let's get it working without it first
      GLOBALFS=true
      DOCKER=true
      ;;
    *)
      HEADNODE=lhn001.cep2.lofar
      COMPUTENODES="`seq -f "locus%02.0f" 1 94`"
      NRCOMPUTENODES=94

      SLURM=false
      GLOBALFS=false
      DOCKER=false
      ;;
  esac
}
