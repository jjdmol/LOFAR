#!/usr/bin/env python
"""
This module contains the data structures for the test data
and some methods for printing
"""
import sys
import time

class TestInfo:
   """
   This class holds some information about the run environment of the tests
   """
   def __init__(self):
      defaultTags = ['hostname', 'CPUs', 'memory', 'runDate']
      self.__infoDict = dict()
      for t in defaultTags:
         self.__infoDict[t] = ['']
   def __setitem__(self, name, info):
      if isinstance(info, list):
         self.__infoDict[name] = info
      else:
         self.__infoDict[name] = [info]
   def __getitem__(self, name):
      return self.__infoDict[name]
   def getTags(self):
      return self.__infoDict.keys()
   def addInfo2Tag(self, name, info):
      if not self.__infoDict.has_key(name):
         self.__infoDict[name] = [info]
      else:
         self.__infoDict[name].append(info)

class Timer:
   """
   This class represents a timer output from LCS/Common/NSTimer
   """
   def __init__(self, name, avg = 0, total = 0, count = 0):
      self.__name = name
      self.__avg = avg
      self.__total = total
      self.__count = count
   def __str__(self):
      return 'Timer ' + self.__name + '[avg=' + str(self.__avg) + ';total=' + str(self.__total) + ';count=' + str(self.__count) + ']'
   def __add__(self, other):
      total = self.__total + other.__total
      count = self.__count + other.__count
      if (count==0):
         avg = 0
      else:
         avg = total/count
      return Timer(self.__name + ' + ' + other.__name, avg, total, count)
   def getTotal(self):
      return self.__total
   def getName(self):
      return self.__name
      
class TimerUSR:
   """
   This class represents a user/system/real timer
   """
   def __init__(self, name, user = 0, system = 0, real = 0):
      self.__name = name
      self.__user = user
      self.__system = system
      self.__real = real
   def __add__(self, other):
      return TimerUSR(self.__name + ' + ' + other.__name, self.__user + other.__user, self.__system + other.__system, self.__real + other.__real)
   def __str__(self):
      return 'Timer ' + self.__name + '[' + self.getShortStr() + ']'
   def getShortStr(self):
      return 'usr=' + str(self.__user) + ', sys=' + str(self.__system) + ', real=' + str(self.__real)
   def getName(self):
      return self.__name

def addDicts(dictlist):
   """
   This method takes a list of dictonaries and adds all the values by key
   """
   ret = dict()
   for d in dictlist:
      for key in d:
         value = d[key]
         if ret.has_key(key):
            ret[key] = ret[key] + value
            ret[key].name = key
         else:
            ret[key] = value
   return ret

class Parameter:
   def __init__(self, name, value):
      self.__name = name
      self.__value = list()
      if value != None:
         if value.__class__ == type(list()):
            self.__value = value      
         else:
            self.__value.append(value)
   def getName(self):
      return self.__name
   def getValue(self):
      return self.__value
   def setValue(value):
      self.__value = list()
      if value.__class__ == type(list()):
         self.__value = value      
      else:
         self.__value.append(value)
   def __str__(self):
      return 'Parm[' + self.__name + '=[' + ', '.join(map(str, self.__value)) + '];]'

class RealParameter(Parameter):
   def __str__(self):
      return 'RealParm[' + self.__name + '=[' + ', '.join(map(str, self.__value)) + '];]'

def getQImpr(newError, oldError):
   if (newError == oldError):
      qualityImpr = 'no change'       
   elif (newError == 0):
      qualityImpr = 'correct'
   else:
      qualityImpr = str(oldError/newError)
   return qualityImpr         

class ParameterTrace:
   """
   This class is like a parameter with several values
   It can be used to trace the value of a parameter through time
   """
   def __init__(self, name = None, realValue = None):
      self.name = name
      if realValue != None:
         self._realValue = realValue.parm.getValue()
      else:
         self._realValue = None
      if self.name == None:
         self.name = realValue.name
      self.valueList = list()
   def append(self, parm):
      if self.name == None:
         self.name = parm.getName()
      self.valueList.append(parm.getValue())
   def getQuality(self):
      if len(self.valueList) == 0:
         qualityImpr = 'no parms'
      else:
         oldError = map(lambda x,y: float(x) - float(y), self.valueList[0], self._realValue) 
         newError = map(lambda x,y: float(x) - float(y), self.valueList[-1], self._realValue)
         qualityImpr = ', '.join(map(getQImpr, newError, oldError))
      return qualityImpr      
   def getRealValue(self):
      return self._realValue
   def getValues(self):
      return self.valueList
   def setRealValue(self, parm):
      self._realValue = parm.getValue()

class ParameterTraceDict(dict):
   """
   This class hold a number of ParameterTraces.
   They can be looked up by name
   """
   def __init__(self):
      dict.__init__(self)
   def addParm(self, parm):
      if not self.has_key(parm.getName()):
         self[parm.getName()]=ParameterTrace(parm.getName())
      self[parm.getName()].append(parm)
   def addParms(self, parms):
      for parm in parms:
         self.addParm(parms[parm])
   def addRealValue(self, parm):
      if not self.has_key(parm.getName()):
         self[parm.getName()]=ParameterTrace(parm.getName())
      self[parm.getName()].setRealValue(parm)
   def addRealValues(self, parms):
      for parm in parms:
         self.addRealValue(parms[parm])

class TestPart:
   def __init__(self):
      self.parms = dict()
      self.timers = dict()
      self.children = list()
   def add (self, object):
      # print ('adding object ' + str(object) + ' to ' + self.__class__.__name__)
      if isinstance(object, Parameter): 
         self.parms[object.getName()] = object
      elif isinstance(object, Timer):
         self.timers[object.getName()] = object
      elif isinstance(object, TimerUSR):
         self.timers[object.getName()] = object
      elif isinstance(object, TestPart):
         self.children.append(object)
      else:
         raise Exception('cannot add object ' + str(object) + ' to ' + self.__class__.__name__)
      
   def getParm (self, name):
      return self.parms[name]
   def getTimer (self, name):
      return self.timer[name]
   def doPrint(self, prefix):
      #print 'TestPart::doPrint'
      prefix = prefix + self.__class__.__name__
      #print str(len(self.parms)) + ' parms'
      for p in self.parms:
         print prefix + ': ' + p + '=' + str(self.parms[p])
      #print str(len(self.timers)) + ' timers'
      for t in self.timers:
         print prefix + ': ' + t + '=' + str(self.timers[t])
      #print str(len(self.children)) + ' children'
      for c in self.children:
         c.doPrint(prefix + '-')

class Iteration(TestPart):
   def __init__(self):
      TestPart.__init__(self)

class Interval(TestPart):
   def __init__(self):
      TestPart.__init__(self)
      # the real values of the parameters
      self.realParms = dict()
   def add (self, object):
      if isinstance(object, RealParameter): 
         self.realParms[object.getName()] = object
      else:
         TestPart.add(self, object)
   def getRealParm (self, name):
      return self.realParms[name]
   def getTotalTimes(self):
      totalTimes = addDicts((dict(), self.timers))
      for it in self.children:
         totalTimes = addDicts((totalTimes, it.timers))
      return totalTimes
   def getTraceDict(self):
      ptracedict = ParameterTraceDict()
      ptracedict.addParms(self.parms)
      ptracedict.addRealValues(self.realParms)
      for it in self.children:
        ptracedict.addParms(it.parms)
      return ptracedict
   def doPrint(self, prefix):
      newprefix = prefix + self.__class__.__name__
      for r in self.realParms:
         print newprefix + ': ' + r + '=' + str(self.realParms[r])
      TestPart.doPrint(self, prefix)
      
class TestRun(TestPart):
   def __init__(self):
      TestPart.__init__(self)
      self.settings = []
   def getTotalTimes(self):
      totalTimes = addDicts((dict(), self.timers))
      for int in self.children:
         totalTimes = addDicts((totalTimes, int.getTotalTimes()))
      return totalTimes
   def getTraceDicts(self):
      tddict = dict()
      for int in range(0, len(self.children)):
         tddict[int] = self.children[int].getTraceDict()
      return tddict
   def doPrint(self, prefix):
      for s in self.settings:
         print (prefix + self.__class__.__name__ + '-Settings: ' + s)
      TestPart.doPrint(self, prefix)

if __name__ == '__main__':
   tr=TestRun()
   tr.add(Interval())
   tr.children[0].add(Iteration())
   tr.children[0].children[0].add(Parameter('testparm', list()))
   tr.doPrint('')
   print('exitting')
