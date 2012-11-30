import subprocess
import os
import sys
from socket import gethostname
import IPython.parallel
import time
import numpy
import pyrap.tables

dataprocessor_id = 0
def get_dataprocessor_id() :
  global dataprocessor_id
  dataprocessor_id += 1
  return dataprocessor_id-1

def createDataProcessor( datadescriptor ) :
  
  # Check whether the datadescriptor is a list or a string containing a ':'. 
  # In that case the IPython parallel framework will be used to process the data.
  if isinstance(datadescriptor, list)  :
    return DataProcessorParallel( datadescriptor )
  elif isinstance(datadescriptor, str) and (':' in datadescriptor) :
    return DataProcessorParallel( [datadescriptor] )
  else:
    return DataProcessor( datadescriptor )

    
GRIDTYPE_VISIBILITY = 0 
GRIDTYPE_IMAGE = 1
GRIDTYPE_IMAGE_FLATNOISE = 2
GRIDTYPE_IMAGE_FLATGAIN = 3
    
class DataProcessor :
   def __init__(self, msname, parset = {}) :
     self.msname = msname
     self.parset = parset
     self.ms = pyrap.tables.table(msname)
     
   def get_msname(self):
     return self.msname
     
   def get_info(self):
     metainfo = {}
     metainfo['LEN'] = len(self.ms)
     t_field = pyrap.tables.table(self.ms.getkeyword('FIELD'))
     metainfo['FIELD'] = t_field[:]
     t_spw = pyrap.tables.table(self.ms.getkeyword('SPECTRAL_WINDOW'))
     metainfo['SPECTRAL_WINDOW'] = t_field[:]
     return metainfo
     
   def getSumBeamSquared(self, coordinatesystem) :
     sumbeamsquared = 0
     sumweight = 0
     return (sumbeamsquared, sumweight)
     
   def setRootMeanSquareBeam(self, coordinatesystem, grid, gridtype = GRIDTYPE_IMAGE) :
     pass
   
   def getCoverage(self, coordinatesystem, gridtype = GRIDTYPE_IMAGE) :
     grid = 0
     gridtype = GRIDTYPE_IMAGE
     return grid
     
   def getPointSpreadFunction(self, coordinatesystem, gridtype = GRIDTYPE_IMAGE) :
     grid = 0
     return grid, weight
   
   def getDirtyImage(self, coordinatesystem, gridtype = GRIDTYPE_IMAGE):
     grid = numpy.zeros((10,10))
     weight = 0.0
     return grid, weight
   
   def getResidualImage(self, coordinatesystem, gridtype = GRIDTYPE_IMAGE) :
     return grid, weight
   
   def fft(self, grid):
     return numpy.fft.fft2(grid)
   
class DataProcessorParallel :
  def __init__(self, datadescriptor, parset = {}):
    self.profile = "%s-%s-%i-%i" % (sys.argv[0], gethostname(), os.getpid(), get_dataprocessor_id())
    fnull = open(os.devnull, "w")
    # start ipcontroller
    self.ipcontroller = subprocess.Popen(["ipcontroller", "--ip=*", "--profile=" + self.profile], stdin=fnull, stdout=fnull, stderr=fnull)
    print "Waiting for ipcontroller to come online..."
    while True :
      try :
        self.rc = IPython.parallel.Client(profile=self.profile)
      except :
        pass
      else:
        break
      time.sleep(1)
    # start ipengines
    for url in datadescriptor :
      if ':' in url :
        hostname, msname = url.split(':')
      else:
        hostname = 'localhost'
        msname = url
      shell = subprocess.Popen( ["ssh", hostname, "/bin/sh"], stdin=subprocess.PIPE, stdout=fnull, stderr=fnull)
      shell.stdin.write("export PYTHONPATH=" + os.environ['PYTHONPATH'] + "\n")
      shell.stdin.write("export LD_LIBRARY_PATH=" + os.environ['LD_LIBRARY_PATH'] +"\n")
      shell.stdin.write("ipengine --profile=" + self.profile + ' --work-dir=' + os.getcwd() + ' --c="global msname;msname=\'' + msname+ '\'"&\n')
      shell.stdin.write("exit\n")
      #ipengine = subprocess.Popen( ["ssh", hostname, "echo", "--profile=" + self.profile, "--c='global msname;msname=" + repr(msname)+"'"])
    print "Waiting for engines to register..."
    while len(self.rc) < len(datadescriptor) :
      time.sleep(1)
    print self.rc[:]['msname']
    self.rc[:].execute('import dataprocessor').get()
    self.rc[:].execute('localdataprocessor = dataprocessor.DataProcessor(msname)').get()
    self.remotedataprocessor = IPython.parallel.Reference('localdataprocessor')
    self.connected = True
    print self.get_msname()
    
  def get_msname(self) :
    return self.rc[:].apply(lambda f, *a, **b : f.get_msname(*a,**b), self.remotedataprocessor).get()
    
  def get_info(self) :
    return self.rc[:].apply(lambda f, *a, **b : f.get_info(*a,**b), self.remotedataprocessor).get()
    
  def fft(self, grid) :
    return self.rc[:].apply(lambda f, *a, **b : f.fft(*a,**b), self.remotedataprocessor, grid).get()
    
  def close(self) :
    if self.connected :
      self.rc.shutdown(hub=True)
      self.connected = False
    
  def __del__(self) :
    self.close()
    