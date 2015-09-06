#!/usr/bin/env python
from multiprocessing import Process, Queue, Manager, Value
from multiprocessing.managers import SyncManager
from Queue import Empty
import SimpleXMLRPCServer, sys, os

class MkdirServer(Process):
  def makeDirectory(self, srmpath=""):
    if srmpath:
      targetpath = 'srm://srm.target.rug.nl:8444/lofar/user/disk/ingest'
      if targetpath in srmpath:
        newdir = '/' + srmpath.split('/')[-1]
        try:
          uploaddir = '/target/gpfs2/lofar/home/ingestmantest/test'
          os.makedirs(uploaddir + newdir)
        except Exception as e:
          print e
          return ('error', -30, 'Could not create directory')
        if os.path.isdir(uploaddir + newdir):
          print "New Ingest job request recieved: %s created" % (uploaddir + newdir)
          return  ('ok', 0, 'Directory created')
        else:
          return ('error', -30, 'Could not create directory')
      else:
        return ('error', -20, 'Illegal LTA location')
    return  ('error', -10, 'No srmpath given')

  def run(self):
    mkdir_server= SimpleXMLRPCServer.SimpleXMLRPCServer(('192.168.210.188', 2013))
    mkdir_server.register_introspection_functions()
    mkdir_server.register_function(self.makeDirectory, 'makeDirectory')
    mkdir_server.serve_forever()

class NewJobsServer(Process):
  def newJobs(self, ingestId="", contactAuthor="", projectId="", observationIds=[], locations=[], dataProductIds=[], jobIds=[]):
    if ingestId and contactAuthor and projectId and observationIds and locations and dataProductIds and jobIds:
      try:
        print "recieved new Ingest job %s for project %s with contact %s" % (ingestId, projectId, contactAuthor)
        size = len(observationIds)
        for i in range(size):
          print i
          print observationIds[i]
          print locations[i]
          print dataProductIds[i]
          print jobIds[i]
        return ('ok', 0, 'Valid Ingest recieved')
      except Exception as e:
        print e
    return ('error', -10, 'Pie in the sky')
 
  def run(self):
    mkdir_server= SimpleXMLRPCServer.SimpleXMLRPCServer(('192.168.210.188', 2015))
    mkdir_server.register_introspection_functions()
    mkdir_server.register_function(self.newJobs, 'newJobs')
    mkdir_server.serve_forever()

class UserIngest():
  def __init__(self, config):
    print 'Initializing'
    self.config = config

  def serve(self):
    print 'Starting'
    self.mkdir = MkdirServer()
    self.mkdir.start()
   
    self.newJobs = NewJobsServer()
    self.newJobs.start()
    print 'Running'


## Stand alone execution code ------------------------------------------
if __name__ == '__main__':
  usage = """Usage:
  master.py <config>
  config   Something like 'ingest_config' (without the .py)"""

  if len(sys.argv) < 2:
    print usage
    exit(1)
  config     = sys.argv[1]
  standalone = UserIngest(config)
  standalone.serve()

