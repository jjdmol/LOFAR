#!/usr/bin/python

import sys
from math import sqrt, cos, pi
import subprocess
import itertools


def getCPPValue():
  """
  Interface with the cpp implementation
  """
  # Create a array of the executable and the arguments (all string)
  nTabs = 20
  nChannels = 20
  #["nvprof", "./BeamFormerKernelPerformance", "-t", str(nTabs), "-c", str(nChannel)]

  cmd = ["nvprof", "./BeamFormerKernelPerformance", 
         "-t", str(nTabs), "-c", str(nChannel)]
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
    print stderrdata
    sys.exit(1)


  print stdoutdata
  # try to cast to an array (floats are created from the numeric values)
  #try:
  #  outputAsArray = eval(stdoutdata)
  #except:
  #  print "encountered a problem during parsing of the c++ output data."
  #  print "Not a valid python array: "
  #  print stdoutdata
  #  sys.exit(1)

  return outputAsArray


def isClose(a,b, rtol=1e-05,atol=1e-08):
  """
  Simple helper function to compare floats
  """
  # Helper function to test float equality
  # Does not support nan, included in numpy from version 1.9
  return abs(a - b) <= rtol * (abs(a) + abs(b)) + atol


def compareCoordArray(array1, array2):
  """
  Helper function to compare two arrays of pairs
  """
  # compare the two array
  # on error print the offending value and exit(1)
  if (len(array1) != len(array2)):
    print "Returned arrays not of same size: comparison failed"
    print array1
    print "!="
    print array2
    exit(1)

  for idx, (entry1, entry2) in enumerate(zip(array1, array2)):
    if(not (isClose(entry1[0], entry2[0]) and
       isClose(entry1[1], entry2[1]))):
      print "encounter incorrect entry index: " + str(idx)
      print str(entry1) + "!=" + str(entry2)
      exit(1)

if __name__ == "__main__":
  ## test 1
  # If zero rings then return empty array!!
  print "Test 1: zero rings return empthy coord list. "
  cppOutput = getCPPValue()  


    