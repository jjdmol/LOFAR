#!/opt/local/bin/python

# This is a test python script for the SolverQuery class
#
# File:			tSolverQuery.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:			2010/07/21
# Last change		2010/10/06


import os
import sys
import SolverQuery as sq
import pylab as P        # needed for histogram test


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
    parms=solver.readFreqColumn("CHISQR", iteration="last")

    print "type(parms).__name__: ", type(parms).__name__
    print "type(solver.frequencies[i])", type(solver.frequencies)
    print "len(parms): ", len(parms)

    # Decide on return type how to display it
    if parms["result"] != "False":
        for j in range(0, len(parms["last"])-1):
            print solver.timeSlots[i], ":", parms["last"][j]
    else:
        print "readFreqColumn() failed"


    # Read Solution for a particular cell
    iter=1
    #solution=solver.getSolution(solver.solverTable[100]['STARTTIME'], solver.solverTable[100]['ENDTIME'],solver.solverTable[100]['STARTFREQ'], solver.solverTable[100]['ENDFREQ'], iteration=iter)
    #print "type(solution): ", type(solution)
    #print "Solution: ", solution
    #print "len(solution): ", len(solution)
    #print "solution:", solution[str(iter)]

    #if solution["result"] != "False":
    #    for j in range(0, len(solution[str(iter)])-1):
    #        print solver.timeSlots[i], ":", solution["last"][str(j)]
    #else:
    #    print "getSolution() failed"
    


    # Read Starttimes from table
    starttimes=solver.getStartTimes()
    #for i in range(0, starttimes.nrows()):
    #    print starttimes[i]

    # Read Endtimes from table
    endtimes=solver.getEndTimes()
    #for i in range(0, endtimes.nrows()):
        #print endtimes[i]


    # Read parameter names from table
    #columnNames=solver.readColumnNames()
    #print columnNames

    # Check if a prameter exists
    exists=solver.parameterExists("CHISQR")
    print "CHISQR exists: ", exists


    # Get parameter names
    parameters=solver.readParameterNames()
    print "Parameters: ", parameters

    # Get converged iteration for a cell
    convergedIter=solver.getConvergedIteration(solver.solverTable[0]['STARTTIME'],
solver.solverTable[0]['ENDTIME'],solver.solverTable[0]['STARTFREQ'],
solver.solverTable[0]['ENDFREQ'])
    print "Converged iteration: ", convergedIter

    # Read a parameter along frequency column
    parms=solver.readFreqColumn("CHISQR", iteration="last")
    if parms["result"] != "False":
        print "parms: ", parms
        # loop through keys in dictionary that are not "result"
        for keys in parms.keys():
            if keys != "result":    # we do not want to print the result type
                print parms[keys]
    else:
        print "readFreqColumn failed"


    # get all converged iterations
    convergedIter=solver.getConvergedIteration(solver.solverTable[0]['STARTTIME'],
solver.solverTable[0]['ENDTIME'],solver.solverTable[0]['STARTFREQ'],
solver.solverTable[0]['ENDFREQ'])

    # create a histogram of the converged iterations
    print "convergedIter = ", convergedIter

    #convergedParameter=solver.getConvergedParameter("CHISQR", solver.solverTable[0]['STARTTIME'],solver.solverTable[10000]['ENDTIME'],solver.solverTable[0]['STARTFREQ'], solver.solverTable[0]['ENDFREQ'])

    #print "convergedParameter = ", convergedParameter

    # Create a histogram of the parameter
    #n, bins, patches = P.hist(convergedParameter, 80, histtype='stepfilled')
    #raw_input("Press any Key")


    # Get the correlation Matrix for a particular cell
    corrMatrix=solver.getCorrMatrix( solver.solverTable[0]['STARTTIME'],
solver.solverTable[0]['ENDTIME'],solver.solverTable[0]['STARTFREQ'],
solver.solverTable[0]['ENDFREQ'])


# Define main function entry
if __name__ == "__main__":
    main()

