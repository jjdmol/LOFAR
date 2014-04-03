#!/usr/bin/python

import sys
from math import sqrt, cos, pi
import subprocess
import itertools
import re

import threading
import time
import pickle

import matplotlib.pyplot as plt

#*****************************************************************************
# Code needed to do a nvprof run on the command line
def runAndGetCerrCout(cmd):
  """
  System call of cmd
  Throws exception on non zero exitvalue
  returns the sterr and stout
  """
  # start
  process = subprocess.Popen(
                        cmd,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
  # wait till end get the cout, cerr and exitvalue
  (stdoutdata, stderrdata) = process.communicate()
  exit_status = process.returncode

  # exit != 0
  if (exit_status != 0):
    print "Encountered an error running the c++ interface to RingCoordinates:"
    raise Exception()
    
  return stdoutdata, stderrdata
  
#*****************************************************************************
# Code needed to do parse the nvprof output
def getCSVLinesFromNVProfOutput(stderr):
  """
  Look for nvprof csv output seperator and return all the csv lines
  """
  # nvprof csv header:
  pattern = re.compile("==\d+== Profiling result:")

  nv_trace_found = False
  nv_trace_lines = []

  # split on newlines
  for line in stderr.split("\n") :
    # If we have a trace_header add them to the output
    if nv_trace_found:
      nv_trace_lines.append(line)
      continue

    #look for csv header line
    if pattern.match(line):
      nv_trace_found = True

  return nv_trace_lines


class csvData:
  """
  Contains parsed data as retrieved from nvprof
  Units are normalized normalized ( to whole seconds)
  Remove spurious quotes. 
  """
  def __init__(self, CSVLines, normalizeValues=True):
    # First line contains the name of the entries
    self.entrieNames = [ x.strip('"') for x in CSVLines[0].split(",")]

    # second entry the szie
    self.entrieUnit = CSVLines[1].split(",")

    self.entrieLines = {}
    for line in CSVLines[2:]:
      # skip empty lines
      if len(line) == 0:
        continue

      # try parsing as values if fails use string
      values = []
      for idx, entrie in enumerate(line.split(",")):
        try:
          typedValue = float(entrie)
          if normalizeValues:
            # Now do the normalizing
            if self.entrieUnit[idx] == "ns":
              typedValue *= 1e-9
            elif self.entrieUnit[idx] == "us":
              typedValue *= 1e-6
            elif self.entrieUnit[idx] == "ms":
              typedValue *= 1e-3
            elif self.entrieUnit[idx] == "s":
              pass
            elif self.entrieUnit[idx] == "%":
              pass
            else:
              raise ValueError("Could not parset the unit type for entry:" + 
                              str(entry) + " with type: " + self.entrieUnit[idx])

          values.append(typedValue)
        except:
          name = entrie.strip('"')
          # also insert the name in the values collumn
          values.append(name)
      # save the now typed line
      self.entrieLines[name] = values 

  # helper functions to allow pickeling of data
  def __getstate__(self):
    odict = {}
    odict["entrieNames"] = self.entrieNames
    odict["entrieUnit"] = self.entrieUnit
    odict["entrieLines"] = self.entrieLines

    return odict

  def __setstate__(self, dict):
    self.entrieNames = dict["entrieNames"]
    self.entrieUnit =  dict["entrieUnit"]
    self.entrieLines = dict["entrieLines"] 
 

  def getNamedMetric(self, nameLine, metric):
    """
    returns value of the metric at the entry nameline 
    or exception if either is not found
    """
    indexInList =  None
    if nameLine in self.entrieLines:
      try:
        indexInList = self.entrieNames.index(metric)
      except ValueError:
        raise ValueError("Could not find metric > " + str(metric) + " < in nvprof output. ")

    else:
      raise ValueError("Could not find >" + str(nameLine) + " < as an entrie returned by nvprof")

    # return the  value and the unit
    return self.entrieLines[nameLine][indexInList]

  def __str__(self):
    return str(self.entrieNames) + "\n" + str(self.entrieUnit) + "\n" + str(self.entrieLines)

#*****************************************************************************
# Kernel specific interface
def beamFormerGetAvgTimeForKernelRun(nTabs, nChannels, threadIdx):
  """

  """
  # Create the command to run on the command line
  cmd = ["nvprof", "--csv", "./BeamFormerKernelPerformance", 
         "-t", str(nTabs), "-c", str(nChannels), "-i", str(threadIdx)]
  # run
  stdout, stderr = runAndGetCerrCout(cmd)  
  # Clean the output
  CSVLines = getCSVLinesFromNVProfOutput(stderr)
  # parset and normalize to data
  data = csvData(CSVLines)

  # return the value of interest
  return data


def gputhread(threadIdx, nTabs, nChannels, sharedVAlue):

  data = beamFormerGetAvgTimeForKernelRun(nTabs, nChannels, threadIdx)

  print "measuring; tab, channel, gpuIdx: " + str(nTabs) + "' " + str(nChannels) + "' " + str(threadIdx)
                  
    #Might need a lock..
  lock.acquire()
  sharedValue[(nTabs, nChannels)] = data
  lock.release()


class myThread (threading.Thread):
  def __init__(self, threadID, name, sharedValue, nTabs, nChannels, ):
    threading.Thread.__init__(self)
    self.threadID = threadID
    self.name = name
    self.nTabs = nTabs
    self.nChannels = nChannels
    self.sharedValue = sharedValue

  def run(self):
    gputhread(self.threadID, self.nTabs, self.nChannels, self.sharedValue)


# Define a function for the thread
lock = threading.Lock()

if __name__ == "__main__":
  ## test 1
  # If zero rings then return empty array!!

  # Create a array of the executable and the arguments (all string)
  resultLines = []
  tabRange = range(10, 261, 50)
  channelsToTest = [1, 16, 64, 256]
  # Entry point for parralellization of the available 4 cobalt nodes
  # These test should be adapted when running on different systems
  nrParallelTreads = 4
  sharedValue = {}

  for tabIdx in tabRange:
    # not the producer/ consumer model. Bit mheee, it works
    # Increase the maximum number with at least stepsize. Do not start threads that
    # have an index that is to large
    for startChanIdx in range(0, len(channelsToTest) + nrParallelTreads, nrParallelTreads):
      threads = []
      for idx in range(4):
        # skip if this index is equal or larger than available work items
        if ((startChanIdx + idx) >= len(channelsToTest)):
          continue
        thread = myThread(idx, "Thread" + str(idx), sharedValue, tabIdx, channelsToTest[startChanIdx + idx])
        thread.start()
        threads.append(thread)

      for thread in threads:
        thread.join()

  print sharedValue

  output = open('data.pkl', 'wb')


  # Pickle the list using the highest protocol available.
  pickle.dump(sharedValue, output, -1)
  output.close()



  pkl_file = open('data.pkl', 'rb')

  data1 = pickle.load(pkl_file)
  print data1 


  pkl_file.close()

  #import matplotlib.pyplot as plt
  #plt.plot(xrange, resultLines[0])
  #plt.plot(xrange, resultLines[1])
  #plt.plot(xrange, resultLines[2])
  #plt.plot(xrange, resultLines[3])
  #plt.ylim((0,0.0025))

  #plt.legend(['1 channel', '16 channel', '64 channel', '256 channel'], loc='upper left')
  #plt.ylabel('Duration (sec)')
  #plt.xlabel('Number of tabs')
  #plt.show()

