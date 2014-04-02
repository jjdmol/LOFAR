#!/usr/bin/python

import sys
from math import sqrt, cos, pi
import subprocess
import itertools
import re

import matplotlib.pyplot as plt

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
    

def beamFormerGetAvgTimeForKernelRun(nTabs, nChannels):

  # Create the command to run on the command line
  cmd = ["nvprof", "--csv", "./BeamFormerKernelPerformance", 
         "-t", str(nTabs), "-c", str(nChannels)]
  # run
  stdout, stderr = runAndGetCerrCout(cmd)  
  # Clean the output
  CSVLines = getCSVLinesFromNVProfOutput(stderr)
  # parset and normalize to data
  data = csvData(CSVLines)

  # return the value of interest
  return data.getNamedMetric("beamFormer", 'Avg')


if __name__ == "__main__":
  ## test 1
  # If zero rings then return empty array!!

  # Create a array of the executable and the arguments (all string)
  resultLines = []
  xrange = range(10, 261, 50)
  channelsToTest = [1, 16, 64, 256]
  for nChan in channelsToTest:
    runningTimes = []
    for nTab in xrange:
      runningTimes.append(beamFormerGetAvgTimeForKernelRun(nTab, nChan))
    resultLines.append(runningTimes)



  import matplotlib.pyplot as plt
  plt.plot(xrange, resultLines[0])
  plt.plot(xrange, resultLines[1])
  plt.plot(xrange, resultLines[2])
  plt.plot(xrange, resultLines[3])
  plt.ylim((0,0.0025))

  plt.legend(['1 channel', '16 channel', '64 channel', '256 channel'], loc='upper left')
  plt.ylabel('Duration (sec)')
  plt.xlabel('Number of tabs')
  plt.show()

