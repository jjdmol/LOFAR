#!/usr/bin/env python
from multiprocessing import Process, Queue, Manager, Value
from multiprocessing.managers import SyncManager
from Queue import Empty
from job_group import job_group
import job_parser as parser
from job_parser import JobRetry, JobError, JobHold, JobScheduled, JobProducing, JobProduced
from job_parser import jobState2String
import os, time, sys, shutil


##------------------ Listener for incomming jobs --------------------------
class jobListener(Process):
  """This listens for messages with new jobs using the (SOAP) server from the config.
  It writes the file to disk but does not parse it, as it would take too long and keep
  it from receiving the next message."""
  def __init__(self, logger, queue, jobsdir, server):
    logger.info('Initializing Incomming job Listener')
    self.logger  = logger
    self.jobs    = queue
    self.jobsdir = jobsdir
    self.server  = server
    self.server.registerFunction(self.new_job, 'urn:pipeline.export')
    super(jobListener, self).__init__()
    logger.info('Incomming job Listener initialized')

  def new_job(self, fileName, fileContent):
    self.logger.debug("Received a new job: %s" % fileName)
    try:
      if fileContent:
        f = open(self.jobsdir + fileName, 'w')
        f.write(fileContent) ## Save the job so we remember it is in the queue
        f.close()
    except:
      self.logger.exception('Problem writing job: %s' % fileName)
    if fileContent:
      self.jobs.put(fileName)
    elif os.path.exists(self.jobsdir + fileName): ## We've put it back into the queue.
      self.jobs.put(fileName)
    else: ## MoM can send alive messages with no fileContent to check if the ingest is running
      self.logger("The received job was empty: %s" % fileName)

  def run(self):
    self.server.socket.settimeout(60)
    self.logger.info('Incomming job Listener started')
    while True:
      self.server.handle_request()

##--------------------- MoM talker -----------------------------

class momTalker(Process):
  """This sends messages of status updates to MoM. Currently has a maxTalkQueue to prevent
  MoM from getting confused and messages being sent out of order. Needs to be improved."""
  def __init__(self, logger, client, count, maxTalkQueue):
    self.logger       = logger
    self.jobs         = Queue(maxTalkQueue)
    self.exportClient = client
    self.retryCount   = count
    super(momTalker, self).__init__()
    logger.info('momTalker initialzed')

  def getQueue(self):
    """Other worker processes only talk to the queue."""
    return self.jobs

  ## This function also exists in the slave, should be refactored at some point
  def communicateJob(self, job):
    """function to write to log and communicate with GUI"""
    if   job['Status'] == JobRetry:     self.logger.info('Job:' + str(job['ExportID']) + ' will be retried')
    elif job['Status'] == JobError:     self.logger.info('Job:' + str(job['ExportID']) + ' Failed')
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
      if status: ## we retry, because the exportClient does not do an internal retry, but only reports the problem
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
        self.communicateJob(job)
      except Empty: pass

##--------------------- Job handler --------------------------
class jobHandler(Process):
  def __init__(self, logger, incomming_jobs, scheduled_jobs, job_done_msg, update_job_msg,
               jobsdir, faileddir, donedir, 
               talker, masterAddress, masterPort, masterAuth, mailCommand, parallelJobs):
    logger.info('Initializing jobHandler')
    self.masterAddress  = masterAddress
    self.masterPort     = masterPort
    self.masterAuth     = masterAuth
    self.mailCommand    = mailCommand
    self.scheduled      = scheduled_jobs
    self.job_done_msg   = job_done_msg
    self.update_job_msg = update_job_msg
    self.active         = dict()
    self.logger         = logger
    self.jobsdir        = jobsdir
    self.faileddir      = faileddir
    self.donedir        = donedir
    self.parser         = parser.parser(logger)
    self.talker         = talker
    self.parallelJobs   = parallelJobs
    super(jobHandler, self).__init__()
    self.logger.info('jobHandler initialzed')

  def update_file(self, job):
    """Ugly function to interact with the file system.
    The suffix is used to remember how often we tried to process this file.
    We use one directory per job_group or otherwise the lists get too long.
    Finished files go to the donedir, files that failed to much to the faileddir."""
    jobname = job['filename']
    if job['Type'] == 'MoM':
      Dir = 'A_%s/' % job['job_group']  
    else: ## tier0-ingest
      Dir = 'B_%s/' % job['job_group']
    if job['Status'] == JobProduced:
      old = self.jobsdir + jobname
      new = self.donedir + Dir + jobname
      if not os.path.isdir(self.donedir + Dir):
        os.mkdir(self.donedir + Dir)
    else:
      suffix = jobname.split('.')[-1]
      if suffix.isdigit():
        new_jobname = jobname[:-len(suffix)] + str(int(suffix) + 1)
      else:
        new_jobname = jobname + '.1'
      old = self.jobsdir + jobname 
      if job['Status'] == JobRetry:
        new = self.jobsdir + new_jobname
      else:
        new = self.faileddir + Dir + new_jobname
        if not os.path.isdir(self.faileddir + Dir):
          os.mkdir(self.faileddir + Dir)
      job['filename'] = new_jobname
    self.logger.debug('Moving %s to %s' % (old, new))
    shutil.move(old, new)

  def job_done(self, job):
    """Remove the job from the active list, put it back in the queue if it needs to be
    tried again, and update the location/filename to reflect it's state."""
    self.active.pop(job['ExportID'])
    self.update_file(job)
    if job['Status'] == JobRetry:
      self.scheduled.put(job) ## We need to try it again
    self.logger.debug("Job %s no longer active because of state %s" % (job['ExportID'], jobState2String(job['Status'])))

  def run(self):
    ## ====== Waiting for slaves ======
    self.logger.info('Waiting for slaves to connect')
    class manager(SyncManager): pass
    manager.register('number')
    manager.register('get')
    self.manager = manager(address=(self.masterAddress, self.masterPort), authkey=self.masterAuth)
    self.manager.connect()
    nr_of_slaves = int(str(self.manager.number()))
    while nr_of_slaves < 1: # There are no slaves
      time.sleep(10) 
      nr_of_slaves = int(str(self.manager.number()))
    time.sleep(10) #Let's wait a few seconds for any more slaves. Currently all slaves need to connect in 10 seconds.
    nr_of_slaves = int(str(self.manager.number()))
    self.logger.info('Slaves found: %d' % nr_of_slaves)
    os.system('echo "The LTA Ingest has been restarted."|mailx -s "LTA Ingest restarted" ' + self.mailCommand)
    
    ## ======= Main loop ======
    first = True
    while True:
      sleep = True
      try: ## See if any jobs have finished
        job   = self.job_done_msg.get_nowait()
        sleep = False ## found a job
        self.logger.debug("Job's done: %s (%i)" % (job['ExportID'], len(self.active)))
        self.job_done(job)
        self.logger.debug("Job done handled: %s (%i)" % (job['ExportID'], len(self.active)))
        first = True
      except Empty: pass
      if len(self.active) < (self.parallelJobs * nr_of_slaves):
        try: ## See if there is anything scheduled that we can start doing
          job   = self.scheduled.get_nowait()
          sleep = False ## found a job
          self.update_job_msg.put((job, job['Status'], JobProducing, None))
          job['Status'] = JobProducing
          self.active[job['ExportID']] = job
          self.manager.get(None, job['destination']).put(job) ## sends it to the slave with the shortest queue of the possible destinations
          self.logger.debug("Job's started: %s (%i)" % (job['ExportID'], len(self.active)))
          first = True
        except Empty: pass
      if sleep: ##nothing to do, time for a nap.
        #self.emptyQueue.value = False
        #self.logger.debug("sleeping, queue: %s" % len(self.active))
        if first:
           self.logger.debug("sleeping, queue: %s" % len(self.active))
           self.logger.debug("Active exports: %s" % str(self.active.keys()))
           first = False
        time.sleep(10)


## Queue Handler ----------------------------------------------------------
class queueHandler(Process):
  """This schedules jobs in the queue if the can be parsed and makes sure that the job_group
  administration is up-to-date so we know when to send a mail to the user."""
  def __init__(self, logger, talker, incomming_jobs, scheduled_jobs, update_job_msg, jobsdir, faileddir, donedir, mailCommand):
    self.logger         = logger
    self.talker         = talker
    self.incomming      = incomming_jobs ## FIFO queue of filenames
    self.scheduled      = scheduled_jobs ## FIFO queue of jobs
    self.update_job_msg = update_job_msg ## FIFO queue with job status updates
    self.jobsdir        = jobsdir ## where to find the jobs
    self.job_groups     = dict()
    self.faileddir      = faileddir
    self.donedir        = donedir
    self.mailCommand    = mailCommand
    self.parser         = parser.parser(logger)
    super(queueHandler, self).__init__()
    self.logger.info('queueHandler initialzed')

  def update_job(self, job, old_status, new_status, fileType): 
    """This function does the job_group management. Please note that
    old_status and new_status should be used, not job['Status'] to avoid race conditions."""
    jg = job['job_group']
    if not self.job_groups.has_key(jg): ## should only happen on JobScheduled, but let's be safe.
      self.job_groups[jg] = job_group(self.logger, jg, job['Type'], self.mailCommand)
      self.job_groups[jg].read_old_jobs(self.faileddir, self.donedir)
    if new_status == JobScheduled:
      self.job_groups[jg].add_job(job)
    else:
      self.job_groups[jg].update_job(job, old_status, new_status, fileType)
      if self.job_groups[jg].check_finished():
        self.job_groups[jg].send_mail()
        self.job_groups.pop(jg)

  def newJob(self, fileName):
    """Read filename and add to the queue of scheduled jobs if it is a valid file."""
    self.logger.info("Processing job: %s" % fileName)
    job = self.parser.parse(self.jobsdir + fileName)
    job['filename'] = fileName
    if job['Status'] == JobScheduled:
      self.update_job(job, None, JobScheduled, None)
      job['destination'] = self.job_groups[job['job_group']].get_destination()
      self.scheduled.put(job)
#      self.talker.put(job) ## Tell MoM we've done something
    else:
      self.logger.warning('Parsing ' + self.jobsdir + fileName + ' failed')
  
  def run(self):
    while True: 
      try:
        sleep = True
        try:
          msg   = self.update_job_msg.get_nowait()
          sleep = False
          self.update_job(msg[0], msg[1], msg[2], msg[3])
        except Empty: pass
        try:
          fileName = self.incomming.get_nowait()
          sleep    = False
          self.newJob(fileName)
        except Empty: pass
        if sleep: ## nothing to do, time for a nap.
          time.sleep(10)
      except Exception as e:
        self.logger.error("Exception in queueHandler main loop: %s" % e)
        time.sleep(250)



## Startup ----------------------------------------------------------
## This class reads the existing queues from disk in parallel to the main threads.
## The goal is to start processing as soon as some jobs have been read.
class startup(Process):
  def __init__(self, logger, incomming_jobs, jobsdir, mailCommand):
    logger.info('Initializing Master Startup')
    self.jobs          = incomming_jobs
    self.logger        = logger
    self.jobsdir       = jobsdir
    self.mailCommand   = mailCommand
    super(startup, self).__init__()
    logger.info('Master Startup initialzed')

  def run(self):
    existingJobs = os.listdir(self.jobsdir)
    #sort jobs by creation time to keep the order of the queue more or less intact
    existingJobs.sort(key=lambda s: os.path.getmtime(os.path.join(self.jobsdir, s)))
    self.logger.info('Found %d existing jobs' % len(existingJobs))
    for e in existingJobs:
      self.jobs.put(e)
    self.logger.info('Master Startup finished')
    self.logger.info('Currently %s jobs in input queue' % self.jobs.qsize())
    os.system('echo "The LTA Ingest has been restarted. %d existing jobs still found in queue."|mailx -s "LTA Ingest existing jobs in queue" ' % len(existingJobs) + self.mailCommand)


## LTA Master ----------------------------------------------------------
class ltaMaster():
  """Reads the config, starts the threads and talks to the slaves"""
  def __init__(self, config):
    self.incomming_jobs = Queue() ##FIFO queue of filenames
    self.scheduled_jobs = Queue() ##FIFO queue of jobs
    self.update_job_msg = Queue()
    self.job_done_msg   = Queue()
    self.slaves         = {}
    configFile = config
    try:
      self.readConfig(configFile)
    except Exception as e:
      print ('\n%s' % e)
      print('The Configuration is incomplete, exiting')
      exit(2)
    self.logger.info('Master initialized')

  def readConfig(self, configFile):
    exec(eval("'from %s import *' % configFile"))
    self.host          = host
    self.jobsdir       = jobsdir
    self.faileddir     = faileddir
    self.logdir        = logdir
    self.donedir       = donedir
    self.logger        = logger
    self.ltaClient     = ltaClient
    self.pipelineRetry = pipelineRetry
    self.exportClient  = exportClient
    self.momRetry      = momRetry
    self.momServer     = momServer
    self.masterAddress = masterAddress
    self.masterPort    = masterPort
    self.masterAuth    = masterAuth
    self.maxTalkQueue  = maxMasterTalkerQueue
    self.mailCommand   = mailCommand
    self.parallelJobs  = parallelJobs
    if momServer == None: #specific check on the master, this is no problem on the slave
      raise Exception('No MoM to listen to!')

  def add_slave(self, slave):
    self.slaves[slave] = Queue()
    return self.slaves[slave]

  def slave_size(self):
    return len(self.slaves)

  ##Gives you the shortest slave queue unless you ask for a specific one.
  def get_slave(self, source, destination):
    if source: ## this code was developed for use on lse nodes/staging area, not really used.
      return self.slaves[source]
    else:
      result = None
      length = sys.maxint
      for k in self.slaves.keys():
        if destination in k:# subselection of slaves based on destination, bit of a hack right now: choice between: lexar,lotar
          size = self.slaves[k].qsize()
          if length > size:
            result = self.slaves[k]
            length = size
            self.logger.debug('found slave %s' % k)
      return result

  def remove_slave(self, slave):
    q = self.slaves.pop(slave, None)
    if q and not q.empty():
      self.logger.warning('Lingering items were left by %s' % slave)

  def slave_done(self, job, result, fileType):
    if result:
      job['errors'].append(result)
    self.update_job_msg.put((job, JobProducing, job['Status'], fileType))
    self.job_done_msg.put(job)
    self.logger.debug('Slave reported done with %s, status %s' % (job['ExportID'], jobState2String(job['Status'])))

  def serve(self):
    class manager(SyncManager): pass
    manager.register('add_slave', self.add_slave)
    manager.register('number', self.slave_size)
    manager.register('get', self.get_slave)
    manager.register('remove_slave', self.remove_slave)
    manager.register('slave_done', self.slave_done)
    self.manager = manager(address=(self.masterAddress, self.masterPort), authkey=self.masterAuth)

    self.momTalker = momTalker(self.logger, self.exportClient, self.momRetry, self.maxTalkQueue)
    self.momTalker.start()
    talker = self.momTalker.getQueue()

    self.startup = startup(self.logger, self.incomming_jobs, self.jobsdir, self.mailCommand)
    self.startup.start()

    self.queueHandler = queueHandler(self.logger, talker, self.incomming_jobs, self.scheduled_jobs, 
                                     self.update_job_msg,
                                     self.jobsdir, self.faileddir, self.donedir, self.mailCommand)
    self.queueHandler.start()

    self.jobHandler = jobHandler(self.logger, self.incomming_jobs, self.scheduled_jobs, self.job_done_msg, self.update_job_msg,
                                 self.jobsdir, self.faileddir, self.donedir, talker,
                                 self.masterAddress, self.masterPort, self.masterAuth,
                                 self.mailCommand, self.parallelJobs)
    self.jobHandler.start()

    self.jobListener = jobListener(self.logger, self.incomming_jobs, self.jobsdir, self.momServer)
    self.jobListener.start()

    #This doesn't work??: self.manager.start(), we use serve_forever() instead
    self.logger.info('Manager has been started')
    self.manager.get_server().serve_forever() ## We would need a custom serve_forever to be able to stop.

## Stand alone execution code ------------------------------------------
if __name__ == '__main__':
  usage = """Usage:
  master.py <config>
  config:   Something like 'ingest_config' (without the .py)"""

  if len(sys.argv) < 2:
    print usage
    exit(1)
  config     = sys.argv[1]
  standalone = ltaMaster(config)
  standalone.serve()
