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
from  CsvData import *

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
    print "Encountered an error running the executble:"
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


#*****************************************************************************
# Kernel specific interface
def getDataForRun(threadIdx, testToAnalyze, 
              var1, var1item,
              var2, var2item):
  """

  """
  # Create the command to run on the command line

  cmd = ["nvprof", "--csv", testToAnalyze, 
         var1, str(var1item), var2, str(var2item), "-i", str(threadIdx)]
  # run
  stdout, stderr = runAndGetCerrCout(cmd)  
  # Clean the output
  CSVLines = getCSVLinesFromNVProfOutput(stderr)
  # parset and normalize to data
  data = csvData(CSVLines)

  # return the value of interest
  return data


def gputhread(sharedValue,
              threadID, 
              testToAnalyze,
              var1, var1item,
              var2, var2item):
  lock.acquire()
  print "measuring on gpu" + str(threadID) + ": " + testToAnalyze + " "  + \
     str(var1) + " " +str(var1item) + ", "+ str(var2) + " " +str(var2item)
  lock.release()
  data = getDataForRun(threadID, testToAnalyze, 
              var1, var1item,
              var2, var2item)

  lock.acquire()
  sharedValue[(var1item, var2item)] = data
  lock.release()


  
class MyThread (threading.Thread):
  def __init__(self,sharedValue,
               threadID, 
               testToAnalyze, 
               var1, var1item,
               var2, var2item):
    threading.Thread.__init__(self)
    self.sharedValue = sharedValue
    self.threadID = threadID
    self.testToAnalyze = testToAnalyze   
    self.var1 = var1
    self.var1item = var1item
    self.var2 = var2
    self.var2item = var2item

  def run(self):
    gputhread(self.sharedValue,
              self.threadID, 
              self.testToAnalyze,
              self.var1, self.var1item,
              self.var2, self.var2item)

# Define a function for the thread
lock = threading.Lock()

if __name__ == "__main__":
  if len(sys.argv) < 6:
    print "usage: tKernelPerformance.py <path to test> <par1> <range1> <par1> <range1>"
    print ""
    print "example:  tKernelPerformance.py ./tBeamFormerKernel -t '[1, 2, 3, 4]' -c 'range(1, 40, 4)'"
    print "Ranges are evaluated using eval(). NO SANITAZION IS PERFORMED"
    print "for optimal performance put at least 4 items in the first range"
    exit(1)

  testToAnalyze = sys.argv[1]
  var1           = sys.argv[2]
  var1Range      = eval(sys.argv[3])
  var2           = sys.argv[4]
  var2Range      = eval(sys.argv[5])

  
  # Create a array of the executable and the arguments (all string)
  resultLines = []

  # Entry point for parralellization of the available 4 cobalt nodes
  # These test should be adapted when running on different systems
  nrGPUs = 4
  nrParallel = min(len(var1Range),nrGPUs) # use nrGPUs threads parallel at maximum
  sharedValue = {}

  for var2item in var2Range:
    # not the producer/ consumer model. Bit mheee, it works
    # Increase the maximum number with at least stepsize. Do not start threads that
    # have an index that is to large
    for startIdx in range(0, len(var1Range) + nrParallel, nrParallel):
      threads = []
      for idx in range(nrParallel):
        # skip if this index is equal or larger than available work items
        if ((startIdx + idx) >= len(var1Range)):
          continue

        thread = MyThread(sharedValue, 
                          idx, 
                          testToAnalyze,
                          var1, var1Range[startIdx + idx],
                          var2, var2item)
        thread.start()
        threads.append(thread)

      for thread in threads:
        thread.join()

  header = [testToAnalyze, var1, var1Range, var2, var2Range]

  output = open('testStats.pkl', 'wb')
  pickle.dump(sharedValue, output, -1)
  output.close()

  headerOutput = open('testStatsHeader.pkl', 'wb')
  pickle.dump(header, headerOutput, -1)
  headerOutput.close()
