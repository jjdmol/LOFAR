#!/usr/bin/env python
#
# Copyright (C) 2016
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
"""
Daemon that starts/stops pipelines based on their status in OTDB.

The execution chains are as follows:

-----------------------------
  Starting a pipeline
-----------------------------

[SCHEDULED]          -> PipelineControl schedules

                           runPipeline.sh <obsid> || pipelineAborted.sh <obsid>

                        using two SLURM jobs, guaranteeing that pipelineAborted.sh is
                        called in the following circumstances:

                          - runPipeline.sh exits with failure
                          - runPipeline.sh is killed by SLURM
                          - runPipeline.sh job is cancelled in the queue

                        State is set to [QUEUED].

(runPipeline.sh)     -> Calls
                          - state <- [ACTIVE]
                          - getParset
                          - (run pipeline)
                          - state <- [COMPLETING]
                          - (wrap up)
                          - state <- [FINISHED]

(pipelineAborted.sh) -> Calls
                          - state <- [ABORTED]

-----------------------------
  Stopping a pipeline
-----------------------------

[ABORTED]            -> Cancels SLURM job associated with pipeline, causing
                        a cascade of job terminations of successor pipelines.
"""

from lofar.messaging import FromBus, ToBus, RPC, EventMessage
from lofar.parameterset import PyParameterValue
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.sas.otdb.config import DEFAULT_OTDB_NOTIFICATION_BUSNAME, DEFAULT_OTDB_SERVICE_BUSNAME
from lofar.common.util import waitForInterrupt
from lofar.messaging.RPC import RPC, RPCTimeoutException

import subprocess
import datetime
import os
import re

import logging
logger = logging.getLogger(__name__)

def runCommand(cmdline, input=None):
  logger.info("Running '%s'", cmdline)

  # Start command
  proc = subprocess.Popen(
    cmdline,
    stdin=subprocess.PIPE if input else file("/dev/null"),
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    shell=True,
    universal_newlines=True
    )

  # Feed input and wait for termination
  stdout, _ = proc.communicate(input)
  logger.debug(stdout)

  # Check exit status, bail on error
  if proc.returncode != 0:
    raise subprocess.CalledProcessError(proc.returncode, cmdline)

  # Return output
  return stdout.strip()

""" Prefix that is common to all parset keys, depending on the exact source. """
PARSET_PREFIX="ObsSW."

class ProcessingClusterTask(object):
  def __init__(self):
    self.name = ""
    self.partition = ""

    self.maxDuration = 0 # seconds

    self.nrCoresPerTask = 1
    self.nrTasks = 1
    self.minMemPerTask = 1024 # MB

class CEP2Task(ProcessingClusterTask):
  def __init__(self):
    super(CEP2Task).__init__(self)

    self.name = "CEP2"

class CEP4Task(ProcessingClusterTask):
  def __init__(self):
    super(CEP4Task).__init__(self)

    self.name = "CEP4"
    self.partition = "cpu"

    self.nrCoresPerTask = 8
    self.minMemPerTask = self.nrCoresPerTask * 10 * 1024 # ~240 GB free for 24 cores -> ~10GB/core

clusterTaskFactory = {
  "":     CEP2Task,
  "CEP2": CEP2Task,
  "CEP4": CEP4Task,
}

class Parset(dict):
  def predecessors(self):
    """ Extract the list of predecessor obs IDs from the given parset. """

    key = PARSET_PREFIX + "Observation.Scheduler.predecessors"
    strlist = PyParameterValue(str(self[key]), True).getStringVector()

    # Key contains "Lxxxxx" values, we want to have "xxxxx" only
    result = [int(filter(str.isdigit,x)) for x in strlist]

    return result

  def isObservation(self):
    return self[PARSET_PREFIX + "Observation.processType"] == "Observation"

  def isPipeline(self):
    return not self.isObservation()

  def processingCluster(self):
    clusterName = self[PARSET_PREFIX + "Observation.Cluster.ProcessingCluster.clusterName"] or "CEP2"
    task = clusterTaskFactory[clusterName]()

    # override defaults if provided by parset
    task.partition = self[PARSET_PREFIX + "Observation.Cluster.ProcessingCluster.clusterPartition"] or task.partition
    try:
      task.nrCoresPerTask = int(self[PARSET_PREFIX + "Observation.Cluster.ProcessingCluster.numberOfCoresPerTask"] or task.nrCoresPerTask
    except ValueError:
      pass

  def dockerTag(self):
    # Return the version set in the parset, and fall back to our own version.
    return (self[PARSET_PREFIX + "Observation.ObservationControl.PythonControl.softwareVersion"] or
            runCommand("docker-template", "${LOFAR_TAG}"))

  def slurmJobName(self):
    return str(self.otdbId())

  def otdbId(self):
    return int(self[PARSET_PREFIX + "Observation.otdbID"])

class Slurm(object):
  def __init__(self, headnode="head01.cep4"):
    self.headnode = headnode

    # TODO: Derive SLURM partition name
    self.partition = "cpu"

  def _runCommand(self, cmdline):
    cmdline = "ssh %s %s" % (self.headnode, cmdline)
    runCommand(cmdline)

  def submit(self, jobName, cmdline, sbatch_params=None):
    if sbatch_params is None:
      sbatch_params = []

    stdout = self._runCommand("sbatch --partition=%s --job-name=%s %s bash -c '%s'" % (self.partition, jobName, " ".join(sbatch_params), cmdline))

    # Returns "Submitted batch job 3" -- extract ID
    match = re.search("Submitted batch job (\d+)", stdout)
    if not match:
      return None

    return match.group(1)

  def cancel(self, jobName):
    stdout = self._runCommand("scancel --jobname %s" % (jobName,))

    logger.debug("scancel output: %s" % (output,))

  def jobs(self, maxage=datetime.timedelta(365)):
    starttime = (datetime.datetime.utcnow() - maxage).strftime("%FT%T")

    stdout = self._runCommand("sacct --starttime=%s --noheader --parsable2 --format=jobid,jobname" % (starttime,))

    jobs = {}
    for l in stdout.split("\n"):
      # One line is one job
      jobid, jobname = l.split("|")
      jobs_properties = { "JobId": jobid, "JobName": jobname }

      # warn of duplicate names
      if jobname in jobs:
        logger.warning("Duplicate job name: %s" % (jobname,))
      jobs[jobname] = job_properties

    return jobs

  def jobid(self, jobname):
    stdout = self._runCommand("sacct --starttime=2016-01-01 --noheader --parsable2 --format=jobid --name=%s" % (jobname,))

    if stdout == "":
      return None

    lines = stdout.split("\n")

    if len(lines) > 1:
      logger.warning("Duplicate job name: %s" % (jobname,))

    # Use last occurance if there are multiple
    return lines[-1]

class PipelineControl(OTDBBusListener):
  def __init__(self, otdb_notification_busname=DEFAULT_OTDB_NOTIFICATION_BUSNAME, otdb_service_busname=DEFAULT_OTDB_SERVICE_BUSNAME, **kwargs):
    super(PipelineControl, self).__init__(busname=otdb_notification_busname, **kwargs)

    self.parset_rpc = RPC(service="TaskGetSpecification", busname=otdb_service_busname, ForwardExceptions=True)
    self.otdb_service_busname = otdb_service_busname

    self.slurm = Slurm()

  def _setStatus(self, obsid, status):
    try:
        with RPC("TaskSetStatus", busname=self.otdb_service_busname, timeout=10, ForwardExceptions=True) as status_rpc:
            result, _ = status_rpc(OtdbID=obsid, NewStatus=status)
    except RPCTimeoutException, e:
        # We use a queue, so delivery is guaranteed. We don't care about the answer.
        pass

  def start_listening(self, **kwargs):
    self.parset_rpc.open()

    super(PipelineControl, self).start_listening(**kwargs)

  def stop_listening(self, **kwargs):
    super(PipelineControl, self).stop_listening(**kwargs)

    self.parset_rpc.close()

  @classmethod
  def _shouldHandle(self, parset):
    if not parset.isPipeline():
      logger.info("Not processing tree: is not a pipeline")
      return False

    if parset.processingCluster() == "CEP2":
      logger.info("Not processing tree: is a CEP2 pipeline")
      return False

    return True

  def _slurmJobIds(self, parsets):
    return [self.slurm.jobid(p.slurmJobName()) for p in parsets]

  def _getParset(self, otdbId):
    return Parset(self.parset_rpc( OtdbID=otdbId, timeout=10 )[0])

  def _getPredecessorParsets(self, parset):
    otdbIds = parset.predecessors()

    logger.info("Obtaining predecessor parsets %s", otdbIds)

    return [self._getParset(otdbId) for otdbId in otdbIds]

  def onObservationAborted(self, otdbId, modificationTime):
    logger.info("***** STOP Otdb ID %s *****", otdbId)

    # Request the parset
    parset = self._getParset(otdbId)

    if not self._shouldHandle(parset):
      return

    # Cancel corresponding SLURM job, causing any successors
    # to be cancelled as well.
    jobName = parset.slurmJobName()
    logger.info("Cancelling job %s", jobName)
    self.slurm.cancel(jobName)

  """
    More statusses we want to abort on.
  """
  onObservationConflict = onObservationAborted
  onObservationHold     = onObservationAborted

  @classmethod
  def _minStartTime(self, preparsets, margin=datetime.timedelta(0, 60, 0)):
    result = None

    for p in preparsets:
      processType = p[PARSET_PREFIX + "Observation.processType"]

      # If we depend on an observation, start 1 minute after it
      obs_endtime = datetime.datetime.strptime(p[PARSET_PREFIX + "Observation.stopTime"], "%Y-%m-%d %H:%M:%S")
      min_starttime = obs_endtime + margin

      result = max(result, min_starttime) if result else min_starttime

    return result

  def onObservationScheduled(self, otdbId, modificationTime):
    logger.info("***** QUEUE Otdb ID %s *****", otdbId)

    # Request the parset
    parset = self._getParset(otdbId)

    if not self._shouldHandle(parset):
      return

    """
      Collect predecessor information.
    """

    # Collect the parsets of predecessors
    logger.info("Obtaining predecessor parsets")
    preparsets = self._getPredecessorParsets(parset)

    """
      Schedule "docker-runPipeline.sh", which will fetch the parset and run the pipeline within
      a SLURM job.
    """

    # Determine SLURM parameters
    sbatch_params = [
                     # Only run job if all nodes are ready
                     "--wait-all-nodes=1",

                     # Enforce the dependencies, instead of creating lingering jobs
                     "--kill-on-invalid-dep=yes",

                     # Restart job if a node fails
                     "--requeue",

                     # Maximum run time for job (31 days)
                     "--time=31-0",
                    
                     # TODO: Compute nr nodes
                     "--nodes=50",
                    
                     # Define better places to write the output
                     os.path.expandvars("--error=$LOFARROOT/var/log/docker-startPython-%s.stderr" % (otdbId,)),
                     os.path.expandvars("--output=$LOFARROOT/var/log/docker-startPython-%s.log" % (otdbId,)),
                     ]

    min_starttime = self._minStartTime([x for x in preparsets if x.isObservation()])
    if min_starttime:
      sbatch_params.append("--begin=%s" % (min_starttime.strftime("%FT%T"),))

    predecessor_jobs = self._slurmJobIds([x for x in preparsets if x.isPipeline()])
    if predecessor_jobs:
      sbatch_params.append("--dependency=%s" % (",".join(("afterok:%s" % x for x in predecessor_jobs)),))

    # Schedule runPipeline.sh
    logger.info("Scheduling SLURM job for runPipeline.sh")
    slurm_job_id = self.slurm.submit(parset.slurmJobName(),

      "docker run --rm"
        " --net=host"
        " -v /data:/data"
        " -u $UID"
        " -e LOFARENV={lofarenv}"
        " -v $HOME/.ssh:/home/lofar/.ssh:ro"
        " -e SLURM_JOB_ID=$SLURM_JOB_ID"
        " lofar-pipeline:{tag}"
        " runPipeline.sh -o {obsid} -c /opt/lofar/share/pipeline/pipeline.cfg.{cluster} -B {status_bus}"
      .format(
        lofarenv = os.environ.get("LOFARENV", ""),
        obsid = otdbId,
        tag = parset.dockerTag(),
        cluster = parset.processingCluster(),
        status_bus = self.otdb_service_busname,
      ),

      sbatch_params=sbatch_params
    )
    logger.info("Scheduled SLURM job %s" % (slurm_job_id,))

    # Schedule pipelineAborted.sh
    logger.info("Scheduling SLURM job for pipelineAborted.sh")
    slurm_cancel_job_id = self.slurm.submit("%s-aborted" % parset.slurmJobName(),

      "docker run --rm"
        " --net=host"
        " -u $UID"
        " -e LOFARENV={lofarenv}"
        " lofar-pipeline:{tag}"
        " pipelineAborted.sh -o {obsid} -B {status_bus}"
      .format(
        lofarenv = os.environ.get("LOFARENV", ""),
        obsid = otdbId,
        tag = parset.dockerTag(),
        status_bus = self.otdb_service_busname,
      ),

      sbatch_params=[
        "--cpus-per=task=1",
        "--ntasks=1"
        "--dependency=afternotok:%s" % slurm_job_id,
        "--kill-on-invalid-dep=yes",
        "--requeue",
      ]
    )
    logger.info("Scheduled SLURM job %s" % (slurm_cancel_job_id,))

    """
      Update OTDB status. Note the possible race condition
      as the SLURM jobs will set the status too.
    """

    # Set OTDB status to QUEUED
    # TODO: How to avoid race condition with runPipeline.sh setting the status to STARTED
    #       when the SLURM job starts running?
    logger.info("Setting status to QUEUED")
    self._setStatus(otdbId, "queued")

    logger.info("Pipeline processed.")

