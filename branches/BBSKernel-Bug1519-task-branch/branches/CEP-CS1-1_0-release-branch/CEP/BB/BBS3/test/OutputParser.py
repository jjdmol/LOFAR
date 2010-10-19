#!/usr/bin/env python
from TestData import *

def readTimerValue(line, keyword):
   part = line.split(keyword)[-1]
   value = float(part.split()[0])
   if len(part.split()) > 1:
      unit = part.split(' ')[1][0:2]
      if unit == 'ms':
         value = value / 1000
      elif unit == 'us':
         value = value / 1000000
      elif unit == 'ns':
         value = value / 1000000000
      elif unit == 'ks':
         value = value * 1000
   return value

def readTimer(line):
   t = Timer('Unreadable timer')
   #try:
   avg = readTimerValue(line, 'avg = ')
   total = readTimerValue(line, 'total = ')
   count = readTimerValue(line, 'count = ')

   name = ''.join(line.split(' Timer ')[-1].split('avg')[0].split(':')[0:-1])
   t = Timer(name, avg, total, count)
   #except:
   #   raise Exception('Cannot read Timer from this line:' + line)
   return t

def readTimerUSR(line):
   t = TimerUSR('Unreadable timer')
   try:
      words = line.split()
      t = TimerUSR(words[1], real = float(words[2]), user = float(words[4]), system = float(words[6]))
   except:
      raise Exception('Cannot read TimerUSR from this line:' + line)
   return t

def readParm(line):
   try:
      words = ''.join(line.split('parm')[1:])
      if '[' in line:
         words = words.split('[')
         values = eval('[' + words[-1])
         name = words[0].split()[-1]
      else:
         values = words.split()[-1]
         name = words.split()[-2]
      p = Parameter(name, values)
   except:
      raise Exception('Cannot read Parameter from this line:' + line)
   return p

class GeneralInfoParser:
   # this read a file containing general info
   def __init__(self):
      self.linePrefix = 'BBSTest:'
      self.testinfo = TestInfo()
   def parseFileByName(self, fileName): 
      f = open(fileName, 'r')
      self.readFile(f)
      f.close()
   def getInfo(self):
      return self.testinfo
   def readFile(self, f):
      line = f.readline()
      key = None
      while line:
         if line.startswith(self.linePrefix):
            key = line.split()[-1]
         else:
            if key != None:
               # add this line to the list
               self.testinfo.addInfo2Tag(key, line)
	       if key=='working_directory':
	         # extract build_variant from working_directory
                 # look for directory build and take what follows
                 subdir = line.split('/build/')[-1]
                 # take the first string before a slash
		 build_variant = subdir.split('/')[0]
                 self.testinfo.addInfo2Tag('build variant', build_variant)
         line = f.readline()      

class OutputParser:
   def __init__(self, name):
      self.linePrefix = 'BBSTest '
      self.itsTestRun = TestRun()
      self.name = name

   def getTestRun(self):
      return itsTestRun
   
   def getline(self):
      line = 'dummyline'
      while line:
         line = self.itsFile.readline()
         if line.startswith(self.linePrefix):
            break
         else:
            pass
      return line
      
   def parseOpenFile(self, file): 
      # the second argument should be an open file or file like object
      # you can also give a string :
      #   import StringIO
      #   s = StringIO.StringIO('string contents, blabla \n another line')
      # and then call:
      #   outputParser.parse(s)
      
      # clear last testrun object
      self.itsTestRun = TestRun()
      self.itsFile = file
      self.parseTestRun()
      return self.itsTestRun
   
   def parseFileByName(self, fileName): 
      f = open(fileName, 'r')
      self.parseOpenFile(f)
      f.close()
      return self.itsTestRun
   
   def parseTestRun(self):
      # print 'parsing testrun'
      # read first line containing the keyvaluemap
      line = self.itsFile.readline()
      while not line.startswith('['):
         line = self.itsFile.readline()
      self.itsTestRun.settings = list()
      while line.startswith('['):
         # print ('line: >' + str(line) + '<')
         self.itsTestRun.settings.append(line)
         line = self.itsFile.readline()

      myTestRun = TestRun()
      myInterval = Interval()
      myTestRun.add(myInterval)
      myIteration = Iteration()
      myInterval.add(myIteration)
      myCurrent = myIteration
      self.itsTestRun = myTestRun
      while True:
         line = self.getline()
         # print ('line: >' + str(line) + '<')
         #         myObjects = []

         # parse test run data
         if not line:
            break
         elif 'BBSTest' in line:
            if 'Start of test' in line:
               pass
               #myTestRun = TestRun()
               #myCurrent = myTestRun
            elif 'End of TestRun' in line:
               pass
               #myTestRun = None
               #myCurrent = myTestRun
            elif 'NextInterval' in line:
               myInterval = Interval()
               myTestRun.add(myInterval)               
               myIteration = Iteration
               myInterval.add(myIteration)
               myCurrent = myIteration
            elif 'BeginOfInterval' in line:
               pass
            elif 'EndOfInterval' in line:
               pass
            elif 'NextIteration' in line:
               myIteration = Iteration()
               myInterval.add(myIteration)
               myCurrent = myIteration
            elif 'BeginOfIteration' in line:
               pass
            elif 'EndOfIteration' in line:
               pass
            elif 'avg' in line:
               myCurrent.add(readTimer(line))
            elif 'timerUSR' in line:
               myCurrent.add(readTimerUSR(line))
            elif 'parm' in line:
               myCurrent.add(readParm(line))
            elif 'realParm' in line:
               myCurrent.add(readRealParm(line))
            else:
               raise Exception('unrecognized BBSTest line: ' + line)
   
if __name__ == '__main__':
   argv = sys.argv
   if len(argv) == 1:
       filename = 'testDefault.out'
   else:
       filename = argv[1]
   f = open(filename, 'r')
   op = OutputParser('parser')
   op.parseOpenFile(f)
   op.itsTestRun.doPrint('')
   if False:
      timers = op.itsTestRun.getTotalTimes()
      for timer in timers:
         print('timer: ' + str(timer) + ' = ' + str(timers[timer]))
         tracedict = op.itsTestRun.getTraceDicts()
         for trace in tracedict:
            st = 'Trace of ' + str(trace) + ': '
            for p in tracedict[trace]:
               st = st + str(p.getValue()) + ' '
            print(st)
   f.close()
	
