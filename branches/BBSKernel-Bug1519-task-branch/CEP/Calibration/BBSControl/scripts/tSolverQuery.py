#!/opt/local/bin/python

# This is a test python script for the SolverQuery class
#
# File:			tSolverQuery.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:			2010/07/21
# Last change		2010/07/26


import os
import sys
import SolverQuery as sq


#*******************************
#
# Function definitions
#
#*******************************

# Usage function
def usage():
    print "Usage: ", sys.argv[0],"<MS>/<solver>"
    print "<MS>       Measurement Set file containing solutions"
    print "<solver>   Name of table containing solver parameters (default: 'solver')"
    return


#*******************************
#
# Main function
#
#*******************************

def main():
    # If no command line argument is given, display error message
    if len(sys.argv) < 2:
        usage()
        sys.exit(0)
    else:
        tableName=sys.argv[1]

    # Test SolverQuery class
    solver=sq.SolverQuery(tableName)    # create a SolverQuery object


    # Test readCell functions
    cell=solver.readCell(solver.solverTable[100]['STARTTIME'],
solver.solverTable[100]['ENDTIME'],solver.solverTable[100]['STARTFREQ'],
solver.solverTable[100]['ENDFREQ'], iteration=1)
    print "Cell: ", cell

 
    #parameters=solver.readParameterNIdx("LMFACTOR", 1010, 1020)
    #print parameters

    # getFreqChannels
    chans=solver.getFreqs()
    print chans[0]

    # getTimeSlots
    timeslots=solver.getTimeSlots()
  #  for i in range(0,timeslots.nrows()):
  #      print i, timeslots[i]

    print "No. of timeslots: ", timeslots.nrows()

    # getStartFreqs
    startFreqs=solver.getStartFreqs()
    for i in range(0, startFreqs.nrows()):
        print i, startFreqs[i]


    # Read a parameter for all frequency cells
    parms=solver.readFreqColumn("CHISQR", iteration="all")

    #print "type(parms): ", type(parms)
    #print "type(solver.frequencies[i])", type(solver.frequencies)
    #print "len(parms): ", len(parms)

    # Decide on return type how to display it
#    if type(parms).__name__=="dict":
#        for i in range(0, len(parms)):
#            print solver.frequencies[i], ":", parms[i+1]
#            print parms[i+1]
#    elif type(parms).__name__=='ndarray':
#        for i in range(0, parms.size):
#            print solver.timeSlots[i], ":", parms[i+1]
#    else:
#        print "Unknown type parms ", type(parms)


    # Read a parameter for all TimeSlots (iteration=last/all/x)
    parms=solver.readFreqColumn("RANK", iteration=1)

    print "type(parms).__name__: ", type(parms).__name__
    print "type(solver.frequencies[i])", type(solver.frequencies)
    print "len(parms): ", len(parms)

    # Decide on return type how to display it
    if type(parms).__name__=="dict":
        for i in range(0, solver.timeSlots.nrows()-1):
            for j in range(0, len(parms)-1):
                print solver.timeSlots[i], ":", parms[j+1]
    elif type(parms).__name__=="ndarray":
        for i in range(0, parms.size-1):
            print solver.timeSlots[i], ":", parms[i+1]
    elif type(parms).__name__=="list":
        for i in range(0, len(parms)-1):
            #print type(parms[i+1])
            print parms[i]
    else:
        print "Unknown type parms ", type(parms)


    # Read Solution for a particular cell
    parms=solver.getSolution(solver.solverTable[40]['STARTTIME'],
solver.solverTable[40]['ENDTIME'],solver.solverTable[40]['STARTFREQ'],
solver.solverTable[40]['ENDFREQ'])
    print "type(parms): ", type(parms)
    print "Parms: ", parms
    print "len(parms): ", len(parms)
    
    # Read Starttimes from table
    starttimes=solver.getStartTimes()
    #for i in range(0, starttimes.nrows()):
    #    print starttimes[i]

    # Read Endtimes from table
    endtimes=solver.getEndTimes()
    #for i in range(0, endtimes.nrows()):
    #    print endtimes[i]


    # Read parameter names from table
    solver.readColumnNames()

# Define main function entry
if __name__ == "__main__":
    main()

