#!/usr/bin/env python
from multiprocessing import Process, Queue, Value
from Queue import Empty as QueueEmpty
from multiprocessing.managers import SyncManager
from job_parser import JobRetry, JobError, JobHold, JobScheduled, JobProducing, JobProduced
from job_parser import jobState2String
import os, time, sys
from ingestpipeline import IngestPipeline, PipelineError, PipelineJobFailedError
from ingestpipeline import PipelineNoSourceError, PipelineAlreadyInLTAError, PipelineAlreadyInLTAError, PipelineNoProjectInLTAError

##--------------------- MoM talker -----------------------------

class momTalker(Process):
  def __init__(self, logger, client, count, maxTalkQueue):
    self.logger       = logger
    self.jobs         = Queue(maxTalkQueue)
    self.exportClient = client
    self.retryCount   = count
    super(momTalker, self).__init__()
    logger.info('momTalker initialzed')

  def getQueue(self):
    return self.jobs

  ## This function also exists in the master, should be refactored at some point
  def communicateJob(self, job):
    """function to write to log and communicate with GUI"""
    if   job['Status'] == JobError:     self.logger.info('Job:' + str(job['ExportID']) + ' Failed')
    elif job['Status'] == JobHold:      self.logger.info('Job:' + str(job['ExportID']) + ' is on Hold')
    elif job['Status'] == JobScheduled: self.logger.info('Job:' + str(job['ExportID']) + ' Scheduled')
    elif job['Status'] == JobProducing: self.logger.info('Job:' + str(job['ExportID']) + ' Started')
    elif job['Status'] == JobProduced:  self.logger.info('Job:' + str(job['ExportID']) + ' Produced')
    try:
      if job['Status'] == JobRetry:
        self.logger.info('Job:' + str(job['ExportID']) + ' retry state not communicated to MoM')
        return
      if not job['Type'] == 'MoM':
        self.logger.info('Job:' + str(job['ExportID']) + ' not communicated to MoM')
        return
      (status, message) = self.exportClient.setStatus(str(job['ExportID']), str(job['Status']))
      if status: ## we retry, because the client does not do an internal retry, but only reports the problem
        self.logger.warning("Problem communicating with MoM, retrying " + str(job['ExportID']) + ": " + message)
        count = 1
        while (status and (count < self.retryCount)):
          time.sleep(60 * count)
          (status, message) = self.exportClient.setStatus(str(job['ExportID']), str(job['Status']))
          count += 1
          if status:
            self.logger.warning(message)
      self.logger.info(message)
    except:
      self.logger.exception('Could not update job %s status to %s.' % (str(job['ExportID']), jobState2String(job['Status'])))

  def run(self):
    self.logger.info('momTalker started')
    while True:
      try:
        job = self.jobs.get(True, 10)
      except QueueEmpty:
        job = None
      if job:
        self.communicateJob(job)

## ----------------- Job executer ----------------------------------------

class executer(Process):
  def __init__(self, logger, logdir, job, talker, jobs, momClient, ltaClient, host, port, mailCommand, manager, pipelineRetry, momRetry, ltaRetry, srmRetry, srmInit):
    self.logger    = logger
    self.logdir    = logdir
    self.job       = job
    self.talker    = talker
    self.jobs      = jobs
    self.momClient = momClient
    self.ltaClient = ltaClient
    self.host      = host
    self.ltacpport = port
    self.mailCommand = mailCommand
    self.manager   = manager
    self.pipelineRetry = pipelineRetry
    self.momRetry  = momRetry
    self.ltaRetry  = ltaRetry
    self.srmRetry  = srmRetry
    self.srmInit   = srmInit
    self.result    = None
    super(executer, self).__init__()
    logger.info('Executer initialzed for %s (pid: %i)' % (job['ExportID'], os.getpid()))

  def run(self):
    start = time.time()
    self.logger.debug("Slave Pipeline executer starting for %s" % (self.job['ExportID']))
    self.job['Status'] = JobProducing
    if not self.talker.full():
      self.talker.put(self.job)
    else:
      self.logger.debug("MoM queue full, skipping JobProducing status update for %s" % (self.job['ExportID']))

    pipeline = IngestPipeline(self.logdir, self.job, self.momClient, self.ltaClient, self.host, self.ltacpport, self.mailCommand, self.momRetry, self.ltaRetry, self.srmRetry, self.srmInit)
    try:
      pipeline.run()
      self.logger.debug('Slave found no Error for %s' % self.job['ExportID'])
      self.job['Status'] = JobProduced
    except PipelineError as e:
      self.logger.info('The Ingest Pipeline failed for %s' % self.job['ExportID'])
      if e.type == PipelineNoSourceError:
        self.logger.debug('Slave found PipelineNoSource Error for %s' % self.job['ExportID'])
        ## It is not useful to mark the job as failed as this dataproduct is simply nonexistent
        ## Todo: handle with a separate status that can be communicated with MoM?
        self.job['Status'] = JobProduced
      elif e.type == PipelineAlreadyInLTAError:
        self.logger.debug('Slave found PipelineAlreadyInLTA Error for %s' % self.job['ExportID'])
        ## It is not useful to mark the job as failed as this dataproduct is simply already done
        ## Todo: handle with a separate status that can be communicated with MoM?
        self.job['Status'] = JobProduced
      elif e.type == PipelineNoProjectInLTAError:
        self.logger.debug('Slave found PipelineNoProjectInLta Error for %s' % self.job['ExportID'])
        ## We do want this status to be marked as failed
        ## Todo: handle with a separate status that can be communicated with MoM?
        self.job['Status'] = JobError
        self.result = ("Project not in LTA","ingest")
      else:
        self.logger.debug('Slave found PipelineFailedError for %s' % self.job['ExportID'])
        self.job['Status'] = JobError
        self.result = (e.args[0], e.source)
    except Exception as e:
      self.logger.debug('Slave found unexpected Error for %s' % self.job['ExportID'])
      self.logger.warning('The Ingest Pipeline failed for %s' % self.job['ExportID'])
      self.job['Status'] = JobError
      self.result = (str(e), 'ingestpipeline')
    ## Only communicate failure with MoM if we have given up retrying
    ## Todo: More elegant to handle all decisions about failed/successful jobs in the manager?
    if self.job['Status'] == JobError:
      self.job['retry'] += 1
      if self.job['retry'] < self.pipelineRetry:
        self.job['Status'] = JobRetry
    if (self.job['Status'] == JobProduced) or (self.job['Status'] == JobError):
        self.talker.put(self.job)
    self.manager.slave_done(self.job, self.result, pipeline.FileType)
    #python 'with' does not work for some reason, so just use acquire/release
    self.jobs.get_lock().acquire()
    self.jobs.value -= 1
    self.jobs.get_lock().release()
    self.logger.debug("Slave Pipeline executer finished for %s in %d sec" % (self.job['ExportID'], time.time() - start))

## ---------------- LTA Slave --------------------------------------------
class ltaSlave():
  def __init__(self, config):
    configFile   = config
    try:
      self.readConfig(configFile)
    except Exception as e:
      print ('\n%s' % e)
      print('The Configuration is incomplete, exiting')
      exit(2)

    self.jobs   = Value('i', 0)
    self.logger.info('Slave %s initialized' % self.host)

  def readConfig(self, configFile):
    exec(eval("'from %s import *' % configFile"))
    self.host          = host
    self.ltacpport     = ltacpport
    self.mailSlCommand = mailSlCommand
    self.jobsdir       = jobsdir
    self.logger        = logger
    self.logdir        = logdir
    self.ltaClient     = ltaClient
    self.exportClient  = exportClient
    self.momClient     = momClient
    self.pipelineRetry = pipelineRetry
    self.momRetry      = momRetry
    self.ltaRetry      = ltaRetry
    self.srmRetry      = srmRetry
    self.srmInit       = srmInit
    self.momServer     = momServer
    self.masterAddress = masterAddress
    self.masterPort    = masterPort
    self.masterAuth    = masterAuth
    self.maxTalkQueue  = maxSlaveTalkerQueue
    self.parallelJobs  = parallelJobs

  def serve(self):
    class Manager(SyncManager): pass
    Manager.register('add_slave')
    Manager.register('remove_slave')
    Manager.register('slave_done')
    self.manager = Manager(address=(self.masterAddress, self.masterPort), authkey=self.masterAuth)
    self.manager.connect()
    self.logger.debug('Master found')
    self.queue = self.manager.add_slave(self.host)

    self.momTalker = momTalker(self.logger, self.exportClient, self.momRetry, self.maxTalkQueue)
    self.momTalker.start()
    talker = self.momTalker.getQueue()

    self.logger.info('Slave %s started' % self.host)
    while True:
      if self.jobs.value < self.parallelJobs:
        try:
          job = self.queue.get(True, 10)
        except QueueEmpty:
          job = None
        if job:
          #python 'with' does not work for some reason, so just use acquire/release
          self.jobs.get_lock().acquire()
          self.jobs.value += 1
          self.jobs.get_lock().release()
          runner = executer(self.logger, self.logdir, job, talker, self.jobs, self.momClient, self.ltaClient, self.host, self.ltacpport, self.mailSlCommand, self.manager, self.pipelineRetry, self.momRetry, self.ltaRetry, self.srmRetry, self.srmInit)
          runner.start()
      else:
        time.sleep(10)

## Stand alone execution code ------------------------------------------
if __name__ == '__main__':
  usage = """Usage:
  slave.py <config>
  config   Something like ingest_config (without the .py)"""

  if len(sys.argv) < 2:
    print usage
    exit(1)
  config     = sys.argv[1]
  standalone = ltaSlave(config)
  standalone.serve()
