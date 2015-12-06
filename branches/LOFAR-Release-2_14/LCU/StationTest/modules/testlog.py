"""Test logging utilities
"""

################################################################################
# System imports
import time

################################################################################
# Functions

class Testlog:

  def __init__(self, verbosity=11, testId=None, logName='empty.dat'):
    self.startTime = time.time()
    self.verbosity = verbosity
    self.testId = testId
    self.logName = logName
    if logName != None:
      try:
        self.logFile = open(self.logName,'w')
      except IOError:
        print 'ERROR : Can not open log file %s' % logName
    self.result = 'RUNONLY'
    
  def setId(self, txt):  # Use this method rather than direct access to testId
    self.testId=txt
  
  def setResult(self, res):                     # Use this method rather than direct access to result
    if res in ['RUNONLY', 'PASSED', 'FAILED']:  # Ignore illegal res
      if self.result != 'FAILED':               # Once FAILED the result can not be changed
        self.result=res

  def getResult(self):   # Method alternative for direct access to result
    return self.result
    
  def appendLog(self, level, string, notime=0, nolevel=0, noId=0):
    txt = ''
    if notime == 0:
      t = time.localtime()
      txt = txt + '[%d:%02d:%02d %02d:%02d:%02d]' % (t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)
    if nolevel == 0:
      txt = txt + ' - (%02d) ' % level
    if noId == 0:
      txt = txt + self.testId
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
