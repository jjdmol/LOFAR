#!/usr/bin/python
# this script executes the 3BlockPerf program a number of time with different settings

import os, sys
from time import sleep

def avg(mylist):
  """
  Calculate average of the values in a list
  """
  if len(mylist) == 0:
    return 0
  else:
    total = float(0)
    for x in mylist:
      total = total + x
    return total/len(mylist)

def createFile(size, flop):
  print "Creating file with size " + str(size) + " and flops " + str( flop)
  file = open ("3BlockPerf.in", 'w')
  file.writelines("dataSize = " + str(size) + ",\n")
  file.writelines("flopsPerByte = " + str(flop) + ",\n")
  file.writelines("packetsPerMeasurement = 100,\n")
  file.writelines("numberOfRuns = 1000")
  file.close()

def run3BP(size, flop):
  print("Running 3BlockPerf")
  # sleep for a second
  # if we don't sleep the program might get an old version of the settings file (using NFS)
  sleep(1)
  os.popen("mpirun -hostfile nodelist -np 3 src/3BlockPerf > 3BlockPerfOne.out")

def addResult(size, flop):
  print("Analyzing results")
  ifile = open("3BlockPerfOne.out", 'r')
  lines = ifile.read().split('\n')
  ifile.close()
  speed = []
  flopsPs = []
  flopsPB = flop
  mysize = size
  found = False
  for line in lines:
    if line != "":
      words = line.split()
      if words[0] == "inf":
        found = True
      elif found:
        # print(str(words))
        speed.append(float(words[0]))
        flopsPs.append(float(words[2]))
        mysize = int(words[7])
        flopsPB = int(words[14])
  ofile = open("3BlockPerfAll.out", 'a')
  ofile.write(" " + str(size))
  ofile.write(" " + str(flopsPB))
  ofile.write(" " +str(avg(speed)))
  ofile.write(" " + str(avg(flopsPs)))
  ofile.write('\n')
  ofile.close()

def main(maxSize = 100000000, maxFlop = 100, sizesteps = 10, flopsteps = 10):
  # create a grid for the measurement points

  # we want the number of flop linear and the packet sizes exponential
  flopsPerByte = [(x * maxFlop/(flopsteps-1)) for x in range(0, flopsteps)
  factor = 1.0 * maxSize / (sizesteps * sizesteps)
  packetSizes = [int(factor *x*x) for x in range(1, sizesteps + 1)] # here we do not start at 0!

  # clear file
  file = open("3BlockPerfAll.out", 'w')
  file.close()
  for size in packetSizes:
    for flop in flopsPerByte:
      print "size and flop: " + str(size) + " " + str(flop)
      createFile(size, flop)
      run3BP(size, flop)
      addResult(size, flop)

if __name__ == "__main__":
  if len(sys.argv) == 1:
    main()
  elif len(sys.argv) == 3:
    main(int(sys.argv[1]), int(sys.argv[2]))
  elif len(sys.argv) == 5:
    main(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]))
  else:
    print " Usage :"
    print sys.argv[0] + " [maxPacketSize maxFlopsPerByte [NoPacketSizeSteps NoFlopsPerByteSteps]]"

