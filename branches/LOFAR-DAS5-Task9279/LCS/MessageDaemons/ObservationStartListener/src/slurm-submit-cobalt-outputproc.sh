#!/bin/sh
# Submit COBALT outputProc as SLURM job from args
#
# Usage: slurm-submit-cobalt-outputproc.sh obs_start_settings.qpid.msg storage_host01 ...
#
# $Id$

SRUN=/usr/local/bin/srun
PROGRAM=/opt/lofar/bin/outputProc

progname=`basename -- $0`
progdir=`dirname -- $0`
settings_file=$1
shift
hostnames=$*

. "$progdir/lofarlogger.sh"

log INFO "[$progname] Submitting slurm job to run COBALT outputProc with settings $settings_file on hosts: $hostnames"
#exec "$SRUN" --nodelist=$hostnames "$PROGRAM" $settings_file
