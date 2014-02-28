#!/usr/bin/python

import sys
from math import sqrt, cos, pi
import subprocess
import itertools

class RingCoordinates:
    def __init__(self, numrings, width, center, dirtype):
        self.numrings = numrings
        self.width    = width
        self.center   = center
        self.dirtype  = dirtype

    def cos_adjust(self, offset):
        if self.dirtype != "J2000" and self.dirtype != "B1950":
          return offset

        # warp coordinates closer to the NCP

        cos_dec = cos(self.center[1] + offset[1])
        epsilon = 0.0001

        if cos_dec > epsilon:
            return (offset[0]/cos_dec, offset[1])
        else:
            return offset


    def len_edge(self):
        """
          _
         / \ 
         \_/
         |.|
        """
        return self.width / sqrt(3)

    def len_width(self):
        """
          _
         / \ 
         \_/
        |...|
        """
        return 2 * self.len_edge()

    def len_height(self):
        """
         _  _
        / \ :
        \_/ _
             
        """
        return self.len_width()

    def delta_width(self):
        """
         _ 
        / \_
        \_/ \ 
          \_/
         |.|
        """
        return 1.5 * self.len_edge()

    def delta_height(self):
        """
         _
        / \_  -
        \_/ \ -
          \_/  
        """
        return 0.5 * self.len_width()

    def coordinates(self):
        if self.numrings == 0:
          return []

        coordinates = [(0,0)] # start with central beam

        # stride for each side, starting from the top, clock-wise
        dl = [0] * 6
        dm = [0] * 6

        #  _    
        # / \_  
        # \_/ \ 
        #   \_/ 
        dl[0] = self.delta_width()
        dm[0] = -self.delta_height()

        #  _  
        # / \ 
        # \_/ 
        # / \ 
        # \_/ 
        dl[1] = 0
        dm[1] = -self.len_height()

        #    _  
        #  _/ \ 
        # / \_/ 
        # \_/   
        dl[2] = -self.delta_width()
        dm[2] = -self.delta_height()

        #  _    
        # / \_  
        # \_/ \ 
        #   \_/ 
        dl[3] = -self.delta_width()
        dm[3] = self.delta_height()

        #  _  
        # / \ 
        # \_/ 
        # / \ 
        # \_/ 
        dl[4] = 0
        dm[4] = self.len_height()

        #    _  
        #  _/ \ 
        # / \_/ 
        # \_/   
        dl[5] = self.delta_width()
        dm[5] = self.delta_height()

        # ring 1-n: create the pencil beams from the inner ring outwards
        for r in xrange(1,self.numrings+1):
          # start from the top
          l = 0.0
          m = self.len_height() * r

          for side in xrange(6):
            # every side has length r
            for b in xrange(r):
              coordinates.append( (l,m) )
              l += dl[side]
              m += dm[side]

        return map(self.cos_adjust, coordinates)

def getCPPValue(nrings, width, center, type="J2000"):
  # Create a array of the executable and the arguments (all string)
  cmd = ["tRingCoordinates", str(nrings),str(width),
           str(center[0]),str(center[1]),"J2000"]
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

  # try to cast to an array (floats are created from the numeric values)
  try:
    outputAsArray = eval(stdoutdata)
  except:
    print "encountered a problem during parsing of the c++ output data."
    print "Not a valid python array: "
    print stdoutdata
    sys.exit(1)

  return outputAsArray

def isClose(a,b, rtol=1e-05,atol=1e-08):
  # Helper function to test float equality
  # Does not support nan, included in numpy from version 1.9
  return abs(a - b) <= rtol * (abs(a) + abs(b)) + atol


def compareCoordArray(array1, array2):
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
  # test 1
  # If zero rings then return empty array!!
  print "Test 1: zero rings return empthy coord list. "
  cppOutput = getCPPValue(0, 2, (3,4), "J2000")  
  referenceOutput = RingCoordinates(0, 2, (3, 4), "J2000" ).coordinates()
  
  compareCoordArray(cppOutput, referenceOutput)
  print "succes"

  # test 2
  # Take some values and get the correct results
  print "Test 2: input values: 1, 2, (3,4), J2000"
  cppOutput = getCPPValue(1, 2, (3,4), "J2000")  
  referenceOutput = RingCoordinates(1, 2, (3, 4), "J2000" ).coordinates()
  print cppOutput
  print referenceOutput
  compareCoordArray(cppOutput, referenceOutput)
  
  