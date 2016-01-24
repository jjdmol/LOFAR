#!/usr/bin/env python
#
# Script that generates ASCII output from solver statistics table
#
#


import sys
import pyrap.tables as pt
import lofar.bbs.solverquery as sq

# Variables
MSfilename=""
filename="solverstat.txt"

# TODO


# parse command arguments
if len(sys.argv)==1:
  print "No MS filename given"
  sys.exit(0)
elif len(sys.argv)==2:
  MSfilename=sys.argv[1]
elif len(sys.argv)==3:
  MSfilename=sys.argv[1]
  filename=sys.argv[2]


# open MS through solverquery

# solverquery object
print "MSfilename = ", MSfilename
solverstat=sq.SolverQuery()
solverstat=solverstat.open(MSfilename)   # open the solver statistics table

print "tableType = ", solverstat.getType()

# get unique timeslots
print "numTimeslots = ", solverstat.getNumTimeSlots()
timeslots=[]
timeslots=solverstat.getTimeSlots()   #.getcol("STARTTIME")

# Open output file for writing
outfile=open(filename, "w")

# get STARTFREQ and ENDFREQ
startfreq=solverstat.getStartFreqs()[0]
endfreq=solverstat.getEndFreqs()[0]

print "startfreq = ", startfreq     # DEBUG
print "endfreq = ", endfreq         # DEBUG
#print "timeslots.nrows() = ", timeslots.nrows()

for i in range(0, timeslots.nrows()):   # loop over time slots
  #print "i = ", i
  
  # get solution vector
  solutions=solverstat.getSolution(timeslots[i]["STARTTIME"], timeslots[i]["ENDTIME"], startfreq, endfreq, iteration="all")
  # get solver statistics parameter: ChiSqr
  chiSqr, times=solverstat.readParameter("CHISQR", timeslots[i]["STARTTIME"], timeslots[i]["ENDTIME"], startfreq, endfreq, iteration="all")

 # print "type(solutions) = ", type(solutions)
#  print "length = ", length

  length=len(solutions)  
  for iter in range(1, length):     # Loop over iterations
    line=str(i)               # timeslot at first place of line to write out to file
    line += "\t" + str(iter)        # second column is iteration number

    # get Real and imaginary part for all antennas from solution
    # put values to together
    for j in range(0, len(solutions[iter]), 2):
      #print "len(solutions[iter]) = ", len(solutions[iter])
      print "iter = ", iter    # DEBUG
      print "j = ", j          # DEBUG
      line += "\t" + str(solutions[iter][j]) + "\t" + str(solutions[iter][j+1])

    #print "len(chiSqr) = ", len(chiSqr)    
    line = line + "\t" + str(chiSqr[iter]) + "\n"
    #print "line = ", line         # DEBUG
    outfile.write(line)           # write line to file
    line=""
    

print "Closing ASCII file ", filename
outfile.close()
print "Closing MS solver statistics file ", MSfilename
#solverstat.close()
