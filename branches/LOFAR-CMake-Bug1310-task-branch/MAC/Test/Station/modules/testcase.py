"""Testcase utilities
"""

################################################################################
# System imports
import time

################################################################################
# Functions

class Testcase:

  def __init__(self, verbosity=11, testName='empty.py', repeat=1,
                     rspId=['rsp0'], bpId='rsp', blpId='blp0',
                     tbbId=None, tpId=None, mpId=None,
                     polId=['x','y']):
    self.startTime = time.time()
    self.verbosity = verbosity
    self.testName = testName
    self.repeat = repeat
    self.rspId = rspId
    self.bpId = bpId
    self.blpId = blpId
    self.tbbId = tbbId
    self.tpId = tpId
    self.mpId = mpId
    self.polId = polId
    if testName != None:
      self.logName = testName[0:-3] + '.log'
      self.logFile = open(self.logName,'w')
    self.result = 'RUNONLY'
    
  def setResult(self, res):                     # Use this method rather than direct access to result
    if res in ['RUNONLY', 'PASSED', 'FAILED']:  # Ignore illegal res
      if self.result != 'FAILED':               # Once FAILED the result can not be changed
        self.result=res

  def getResult(self):    # Method alterative to than direct access to result
    return self.result
    
  def appendLog(self, level, string, nolevel=0, notime=0):
    txt = 'Tc %s - ' % self.testName[0:-3]
    if notime == 0:
      t = time.localtime()
      txt = txt + '[%d:%02d:%02d %02d:%02d:%02d]' % (t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)
    if nolevel == 0:
      txt = txt + ' - (%02d) ' % level
    txt = txt + string
    
    if level <= self.verbosity:
      print txt
      self.logFile.write(txt + '\n')

  def appendFile(self, level, fileName):
    try:
      appFile = open(fileName,'r')
      self.appendLog(level,appFile.read(),1,1,1)
      appFile.close()
    except IOError:
      self.appendLog(level,'ERROR : Can not open file %s' % fileName)

  def sleep(self, ms):
    time.sleep(ms/1000.0)
  
  def setStartTime(self):
    self.startTime = time.time()
    
  def getRunTime(self):
    return int(time.time() - self.startTime)
  
  def closeLog(self):
    self.logFile.close()
