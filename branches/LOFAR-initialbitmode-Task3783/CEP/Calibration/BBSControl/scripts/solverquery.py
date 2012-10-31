# Solver class handles reading statistics from the "solver" table logged by
# ParmDBLog into a CASA table
#
# All parameters, unless they are whole tables, are returned in the form of 
# a Python dictionary
#
#
# File:			SolverQuery.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:          	2010/07/16
# Last change:   	2011/12/08
#


# import pyrap as pr
import pyrap.tables as pt
import numpy as np            # numpy needed for machine epsilon value
import time                   # used for timing query functions


class SolverQuery:
    # Empty constructor (does not open the table immediately)
    def __init__(self):
        print "Empty constructor called"
        # do nothing

    # Default constructor, opens the table of name (default: "solver")
    #
    def __init__(self, tablename=""):
        try:
            self.TIMING=True           # use this to activate timing of query functions
            self.convertTime=False      # conver STARTTIME and ENDTIME to human readable

            # Reset frequencies and time vectors that are used for parameter retrieval
            # initialize SolverQuery attributes with defaults that can be recognized
            self.frequencies=[]
            self.timeSlots=[]

            self.startFreqs=[]
            self.endFreqs=[]
            #self.startTimes=[]
            #self.endTimes=[]
            self.midTimes=[]
            self.parameterNames=""   # names of solver parameters (= columns - non-solver column names)
            self.maxIter=0

            self.type=""             # Type of table: PERSOLUTION, PERSOLUTION_CORRMATRIX, PERITERATION, PERITERATION_CORRMATRIX

            if tablename is not "":
                # Set object attributes, read values from MS
                self.tablename=tablename               # keep tablename in object
                self.solverTable=pt.table(tablename)
                self.setParameterNames()
                #self.maxIter==pt.tablecolumn(self.solverTable, "MAXITER")[0]
                self.setMaxIter()

                self.setType()
                self.setTimeSlots()
                self.getMidTimes()
                self.setFreqs()

                #return self   # return also the object for calls from the outside?
        except ValueError:
            traceback.print_exc()


    # Open the solver table
    #
    def open(self, tablename):
        try:
            self.tablename=tablename
            self.solverTable=pt.table(tablename)
            self.setParameterNames()
            self.setMaxIter()
            
            self.setType()
            self.setTimeSlots()
            self.setFreqs()

            return self   # return also the object for calls from the outside?
        except ValueError:
            traceback.print_exc()


    # Flush and close the table of the Solver object
    #
    def close(self):
        self.frequencies=[]
        self.timeSlots=[]

        self.startFreqs=[]
        self.endFreqs=[]
        self.startTimes=[]
        self.endTimes=[]
        self.midTimes=[]
        self.tablename=""
        self.parameterNames=""
        self.maxIter=0
        self.type=""

        self.solverTable.close()


    # Return info about Solver table object (as written in the casa table)
    def info(self):
        return pt.tableinfo(self.tablename)


    # Read a column "colname" from the SolverTable
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned
    #
    def readColumn( self, colname ):
        column=tablecolumn(self, colname)
        return column


    # Read a parameter of a cell (including all iterations if present)
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned (default sorting by FREQ first and then TIME)
    #
    def readParameter(self, parameter_name, start_time, end_time, start_freq, end_freq, iteration="last"):
        #print "solverQuery::readParameter() ", "start_time: ", start_time, "  end_time: ", end_time         # DEBUG

        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyFreq(start_freq, end_freq) 

        #print "fuzzy start_time: ", start_time, "  end_time:  ", end_time   # DEBUG

        if self.TIMING == True:
            t1=time.time()

        if self.parameterExists(parameter_name):
            # Distinguish between specific iteration, last iteration or all iterations
            #
            parmsDict={}     # create empty dictionary
            
            if iteration is "last":
                #print "readParameter(): last"   # DEBUG
                # Fetch requested parameter for time and freq where LASTITER=TRUE
                taqlcmd = "SELECT * FROM " + self.tablename + " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= "+ str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE ORDER BY STARTTIME"
                result=pt.taql(taqlcmd)  
                parameter=result.getcol(parameter_name)

                if self.convertTime == True:
                  taqlcmd= "SELECT STARTTIME FROM " + self.tablename + " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= "+ str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE"              
                  resultTime=pt.taql(taqlcmd)
                  starttimes = resultTime.getcol('STARTTIME')
                else:
                  starttimes = result.getcol('STARTTIME')

                #print "sq::starttimes =", starttimes                 # DEBUG
                #print "sq::starttimes.sort() =", starttimes.sort()   # DEBUG

                #print "readParameter(): len(parameter): ", len(parameter)  # DEBUG
                #print "readParameter() result.nrows() = ", result.nrows()

                parmsDict["result"]="last"   # write type of result into dictionary that it can be handled by caller
                parmsDict["last"]=parameter

            elif type(iteration).__name__ is "int":
                #print "readParameter(): specific iteration number"   # DEBUG

                parmsDict["result"]="iteration"   # write type of result into dictionary that it can be handled by caller

                # Fetch requested parameter for time and freq where ITER=x
                taqlcmd = "SELECT * FROM " + self.tablename + " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= "+ str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND ITER=" + str(iteration)
                result=pt.taql(taqlcmd)  
                parameter=result.getcol(parameter_name)

                if self.convertTime == True:
                  taqlcmd= "SELECT STARTTIME FROM " + self.tablename + " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= "+ str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE ORDER BY STARTTIME"              
                  resultTime=pt.taql(taqlcmd)
                  starttimes = resultTime.getcol('STARTTIME')
                else:
                  starttimes = result.getcol('STARTTIME')

                parmsDict[iteration]=parameter

            elif iteration is "all":
                #print "readParameter() all iterations"     # DEBUG
                # Fetch requested parameter for all iterations with that time and freq
                # and sort them into a dictionary

                parmsDict["result"]="all"   # write type of result into dictionary that it can be handled by caller
                taqlcmd="SELECT * FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time-0.2) + " AND ENDTIME <= " + str(end_time+0.2) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) #+ " AND ITER=" + str(1)
             
                result=pt.taql(taqlcmd)
                parameter=result.getcol(parameter_name)

                if self.convertTime == False:
                  starttimes = result.getcol('STARTTIME')
                else:
                  taqlcmd="SELECT CTOD(STARTTIME s) FROM " + self.tablename + " ORDER BY STARTTIME"
                  resultTime=pt.taql(timecmd)
                  starttimes = resultTime.getcol('STARTTIME')

                #print "readParameter(): len(selection)", len(selection)  # DEBUG
                #print "type(result).__name__: ", type(result).__name__
                #print "result.nrows(): ", result.nrows()
                #print "type(selection).__name__: ", type(selection).__name__
              
                for iter in range(1, len(parameter)+1):  # +1 see arange doc
                    parmsDict[iter]=parameter[iter-1]

            else:
                print "readParameter() unknown iteration keyword"
                return False           

            # Do timing
            if self.TIMING == True:
                t2=time.time()
                print "solverQuery::readParameter(): query took %6.2f ms" % ((t2-t1)*1000)


            return parmsDict, starttimes    # return type is Dictionary

        else:         # If column_name is ""
            print "readParameter: wrong parameter name"
            return False



    # Check if a cell for a particular STARTTIME/ENDTIME,STARTFREQ/ENDFREQ-Cell
    # exists in the solver table
    # It returns the number of nrows it found for that STARTTIME/ENDTIME,STARTFREQ/ENDFREQ
    #
    def cellExists(self, start_time, end_time, start_freq, end_freq):
 
        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)

        query="STARTTIME >= "+ start_time + "AND ENDTIME <= " + end_time + " AND STARTFREQ >= " + start_freq + " AND ENDFREQ <= " + end_freq
       
        result=self.solverTable.query(query)
        return result.nrows()



    # Find the last iteration in an subtable, numpy.ndarray
    # for a particular cell with
    # start_time, start_freq, end_time, end_freq
    #
    #
    def findLastIterationCell(self, start_time, end_time, start_freq, end_freq):
         print "findLastIteration(self, start_time, end_time, start_freq, end_freq)"

         start_time, end_time=self.fuzzyTime(start_time, end_time)
         start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)

         parmsDict={}

         # Get first all iterations for that start_time/end_time, start_freq/end_freq
         query="STARTTIME >= "+ str(start_time) + "AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + "sortlist='ITER'"                     # sort by ITER

         selection=self.solverTable.query(query)             # execute query
         result=selection[selection.nrows()-1]   # set result to last element of selection

         parmsDict["result"]="last"   # write type of result into dictionary that it can be handled by caller
         parmsDict["last"]=result

         return parmsDict


    # Find the last iteration in a mixed result table for all
    # unique frequencies/timeslots
    #
    # intermediate parameter might be a pyrap table or
    # a numpy.ndarray
    #
    def findLastIteration(self, data):
        print "findLastIteration(self, data)"   # DEBUG
        print "type(data): ", type(data)        # DEBUG

        # Determine type of paramter intermediate
        if type(data).__name__ == "numpy.ndarray":
            print "numpy.ndarray"       # DEBUG
            length=len(data)
            return data[length-1]

        elif type(data).__name__ == "pyrap.table.table":
            print "pyrap.table.table"   # DEBUG
            nrows=data.nrows()
            result=data[nrows-1]
            return result

        elif type(data).__name__ == "dict":
            print "dictionary"     # DEBUG

            # Get the last iteration for each dictionary entry
            # and rearrage them in a new dictionary
            result={}      # create empty new dictionary

            for (key, entry) in data.items():
                print "key: ", key
                # Get last entry of this array

            return result

        else:    # unknown type
            print "findLastIteration(): unknown type"
            return False


    # Find solution cells that weren't solved by the solver
    # EXPERIMENTAL
    #
    def findUnsolvedSolutions(self, start_time, end_time, start_freq, end_freq):
        print "solverQuery::findUnsolvedSolutions()"
        
        solutionsDict={}

        if self.TIMING == True:
            t1=time.time()
        
        # Criteria to determine unsolved solutions:
        # final value == initial value (where are these stored?)
        # final value 0
        # chiSqr 0?
        taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) 
        + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) 
        + " AND LASTITER=TRUE AND CHISQR=0"
        
        selection=pt.taql(taqlcmd)
        solutionsDict["last"]=selection.getcol("SOLUTION")        
        
        return solutionsDict
        

    # Get the solution vector from the solver table
    # for a particular cell
    #
    def getSolution(self, start_time, end_time, start_freq, end_freq, iteration="all"):
        print "solverQuery::getSolution() ", "start_time = ", start_time, " end_time = ", end_time

        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyFreq(start_freq, end_freq)

        #print "fuzzy Times: ", start_time, "  ", end_time   # DEBUG
        #print "fuzzy Freqs: ", start_freq, "  ", end_freq   # DEBUG

        solutionsDict={}

        if self.TIMING == True:
            t1=time.time()

        if iteration=="all":
            print "getSolution: get all iterations"    # DEBUG

            solutionsDict["result"]="all"

            taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq)
            selection=pt.taql(taqlcmd)

            #print "getSolution() nrows() = ", selection.nrows()  # DEBUG

            solution=selection.getcol("SOLUTION")
            for iter in range(1, len(solution)+1):
                solutionsDict[iter]=solution[iter-1]

        elif iteration=="last":
            print "getSolution: get last iteration"     # DEBUG

            solutionsDict["result"]="last"

            taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE" 
 
            #taqlcmd="SELECT * FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE"  
      
            selection=pt.taql(taqlcmd)       
            solutionsDict["last"]=selection.getcol("SOLUTION")
            
            print "getSolution() selection.nrows(): ", selection.nrows()   # DEBUG
            
        elif type(iteration).__name__ == "int":
            print "getSolution: iteration ", iteration   # DEBUG

            solutionsDict["result"]="iteration"

            taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND ITER=", str(iteration)        
            selection=pt.taql(taqlcmd)

            #print "type(selection).__name__ : ", type(selection).__name__
            if(type(selection).__name__ == 'NoneType'):
                solutionsDict["result"]="False"
      
            #print "selection.nrows(): ", selection.nrows()   # DEBUG
            #print "selection: ", selection.getcol("ITER")    # DEBUG
            solutionsDict[str(iteration)]=selection.getcol("SOLUTION")

            #return solutionsDict

        else:
            solutionsDict["result"]="False"
            print "getSolution: unknown iteration keyword"
            return False

        # Do timing
        if self.TIMING == True:
            t2=time.time()
            print "solverQuery::getSolution(): query took %6.2f ms" % ((t2-t1)*1000)
        
        #print "solverQuery::getSolution() len(solutionsDict['last']) = ", len(solutionsDict['last'])   # DEBUG

        return solutionsDict


    # Get the solution of a particular parameter (not physical parameter)
    #
    # index             - index in solution vector to return
    #
    def getSolutionParameter(self, start_time, end_time, start_freq, end_freq, index, iteration="all"):
        print "getSolutionParameter()"      # DEBUG

        parameterDict={}

        # Call function class to get parameters and then select index
        solutions=self.getSolutionParameter(start_time, end_time, start_freq, end_freq, iteration="all")    

        # Check if index is within range
        if index <= 0:
            print "solverQuery::getSolutionParameter() index out of range"
            return False
        if index > len(solutions[1]):
            print "solverQuery::getSolutionParameter() index out of range"
            return False
        else:
            parameterDict[0]=iteration
            parameterDict[1]=solutions[index]

        return parameterDict


    # Get the correlation matrix from the solver table for a (list of) cell(s)
    # the correlation matrices are then returned as a list
    # optionally also the correspoding StartTimes and Ranks are returned
    #
    # getStartTimes    - get the corresponding start times (default=False)
    # getRank          - get rank(s) of Correlation matrices too (default=False)
    #
    def getCorrMatrix(self, start_time, end_time, start_freq, end_freq, getStartTimes=True, getRank=False):
        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyFreq(start_freq, end_freq)

        corrMatrix=[]    # list to contain returned corrMatrices

        # LASTITER=TRUE (Correlation matrix is only recorded by the solver for the last iteration)
        taqlcmd="SELECT STARTTIME, CORRMATRIX FROM " + self.tablename + " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE"

        result=pt.taql(taqlcmd)
        rank=self.getRank(start_time, end_time, start_freq, end_freq)          # get the RANK from this cell
        #rankDef=self.getRankDef(start_time, end_time, start_freq, end_freq)    # rank deficiency

        #print "solverQuery::getCorrMatrix() result.nrows() = ", result.nrows()   # DEBUG

        # This is needed if we get back more than one result (but is buggy now)
        #if self.type=="PERITERATION_CORRMATRIX":
        #    result=result[1]['CORRMATRIX']
        #else:
        #    result=result[0]['CORRMATRIX']
  
        # The Corrmatrix will only be for (N+1)th iteration
        if result.nrows()==1:
          corrMatrix=result[0]['CORRMATRIX']  # select CORRMATRIX and write to numpy 1D-array
        else:
          corrMatrix=result[1]['CORRMATRIX']  # select CORRMATRIX and write to numpy 1D-array

        if getStartTimes==True and getRank==True:
            starttimes=result[1].getcol('STARTTIME')  # also select starttimes
            return corrMatrix, starttimes, getRank
        elif getStartTimes==False and getRank==True: 
            return corrMatrix, getRank
        else:
            return corrMatrix


    # Select a particular solution from a dictionary  with arrays
    # of solutions (default is "last")
    #
    # solutions - dictionary containing the solutions read from the solver table
    # selected  - the index which is selected for
    # result    - type of result expected in dictionary: last, iteration (default=last)
    #
    def selectSolution(self, solutions, selected, result="last"):
        #solutionsSelect={}   # dictionary to hold result of selected solution

        solutionsSelect=[]    # array to hold selected solution

        #print "selectSolution(): type(solutions) = ", type(solutions)  # DEBUG
        #print "selectSolution(): len(solutions) = ", len(solutions)    # DEBUG
        #print "selectSolution(): result = ", result                    # DEBUG
        #print "selectSolution(): solutions = ", solutions             # DEBUG

        if isinstance(solutions, dict) == False:
            print "selectSolution() solutions have wrong type"
        else:
            if result == "all":
                # TODO: does not work if we have solutions from an interval....
                if len(solutions)<1:
                    print "selectSolution() no iterations found"

                print "solutions = ", solutions
                for iter in range(1, len(solutions)):
                    print "solutions[iter][selected]", solutions[iter][selected] # DEBUG
                    #print "solutions[", iter, "][0][selected] = ", solutions[iter][0][selected]
                    solutionsSelect.append(solutions[iter][selected])
            elif result == "last":   
                for iter in range(0, len(solutions[result])):
                    solutionsSelect.append(solutions[result][iter][selected])
                    #print "selectSolution: ", solutions[result][iter]  #[selected]
            elif isinstance(result, int):
                # TODO!
                #print "solutionsSelect(): result = ", result
                solutionsSelect.append(solutions[result][0][selected])
            else:
                raise "selectSolution(): unknown result keyword"

            #print "selectSolution(): solutionsSelect", solutionsSelect  # DEBUG

        return solutionsSelect


    # Select a particular solution form a dictionary
    # but pick all iterations
    # TODO
    #
    def selectSolutionPeriteration(self, solutions, selected):
        print "selectSolutionPeriteration()"
        solutionsSelect={}



    #**************************************************
    #
    # Functions to read Parameters and Cells by index
    #
    #**************************************************

    # Read a parameter from the table at index
    #
    def readParameterIdx(self, parameter, index):
        #print "readParameter(self, parameter, index):"
        
        tr=self.solverTable.row()
        selection=tr[index]
        result=selection[parameter]
        return result


    # Read a parameter from the table for all indices between
    # lower_index and upper_index
    #
    def readParameterNIdx(self, parameter, lower_index, upper_index):
        #print "readParameterNIdx(self, lower_index, upper_index)"

        if lower_index > upper_index:
            print "readParameterNIdx: lower_index > upper_index"
            return False

        list=[]                       # list of table rows between lower and upper index
        idx=lower_index
        while idx <= upper_index:
            list.append(self.readParameterIdx(parameter, idx))
            idx+=1
        return list


    # Read a complete Cell (row) from the table by index
    #
    def readCellIdx(self, index):
        tr=self.solverTable.row()
        selection=tr[index]
        return selection


    # Read all Cells (rows) from table that lie between
    # lower_index and upper_index
    #
    def readCellNIdx(self, lower_index, upper_index):
        #print "readCellNIdx(self, lower_index, upper_index):"

        if lower_index > upper_index:
            print "readCellNIdx: lower_index > upper_index"
            return False

        list=[]                       # list of table rows between lower and upper index
        idx=lower_index
        while idx <= upper_index:
            list.append(self.readCellIdx(idx))
            idx+=1
        return list


    #**************************************************
    #
    # Functions to read Parameters and Cells 
    # by time interval
    #
    #**************************************************

    
    # Build a tableindex based on STARTFREQ, ENDFREQ, STARTTIME and ENDTIME
    #
    # THIS DOES NOT WORK YET
    #
    def buildIndex(self):        
        # Build dictionary containing STARTFREQ, ENDFREQ, STARTTIME, ENDTIME and ITER
        self.solverTableIndex=self.solverTable.index('STARTFREQ','STARTTIME')

        # Check if all keys in the index are unique
        if self.solverTableIndex.isunique() == True:
            return True
        else:
            print "buildIndex(): index is not unique"
            return False


    # Read a complete cell, but only for a particular iteration
    # the given times and frequencies specify an inclusive search interval    
    #
    # iteration=all (default) - a dictionary with entries 0,1,2,...,maxIter
    # iteration=x             - a numpy.ndarray for that iteration 
    # iteration=Last          - a numpy.ndarray with the last iterations entries
    #
    def readCell(self, start_time, end_time, start_freq, end_freq, iteration="Last"):
        print "readCell(self, start_time, end_time, start_freq, end_freq, iteration=Last)"  # DEBUG

        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyFreq(start_freq, end_freq)

        cellDict={}

        # return all iterations (default behaviour)
        if iteration == "all":
           cellDict["result"]="all"                   # give type of result

           # Loop over all iterations
           for iter in range(1, self.getMaxIter()):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ >=" + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND ITER = " + str(iter)
                 cell[iter]=pt.taql(taqlcmd)           # execute TaQL command
           return cell

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
           cellDict["result"]="last"                   # give type of result

           # Loop over all iterations
           taqlcmd="SELECT * FROM " + self.tablename + " WHERE LASTITER=", str(iter)
           selection=pt.taql(taqlcmd)           # execute TaQL command
           cellDict["last"]=selection

           return cellDict

        # return only a particular iteration
        elif isinstance(iteration, int):
            cellDict["result"]="iteration"                   # give type of result

            taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ=" + str(start_freq) + " AND ENDFREQ=" + str(end_freq) + " AND ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            selection=pt.taql(taqlcmd)           # execute TaQL command      

            cellDict[iteration]=selection

            return cellDict

        else:
            print "readCell(): unknown iteration"
            cellDict["result"]="False"


            return cellDict


    # Read a parameter for all frequency cells (of one particular time slot)
    # Its return value depends on the query parameter iteration
    #
    def readFreqColumn(self, parameter, iteration="all"):
        #print "readFreqColumn(self, parameter, iteration=", iteration, ":"    # DEBUG

        # Get first all unique frequencies
        if len(self.frequencies)==0:
            self.frequencies=getFreqs()

        # Get MAXITER first
        #maxIter=pt.tablecolumn(self.solverTable, "MAXITER")[0]

        parmsDict={}       # create an empty dictionary

        # return all iterations (default behaviour)
        if iteration == "all":
           parmsDict["result"]="all"

           # Loop over all iterations
           for iter in range(1, self.getMaxIter()+1):
                 taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iter)
                 selection=pt.taql(taqlcmd)              # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 parmsDict[iter]=parmIter                    # write into dictionary__
           return parmsDict

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
            parmsDict["result"]="last"

            taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, " + parameter + " FROM " + self.tablename + " WHERE LASTITER=TRUE"
            selection=pt.taql(taqlcmd)           # execute TaQL command
            parmsDict["last"]=selection.getcol(parameter)    # select column with wanted parameter
            return parmsDict

        # return only a particular iteration
        elif type(iteration).__name__ == "int":
            parmsDict["result"]="iteration"

            taqlcmd="SELECT " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            #print "taqlcmd: ", taqlcmd           # DEBUG
            selection=pt.taql(taqlcmd)           # execute TaQL command      
            parmsDict[iteration]=selection.getcol(parameter)    # select column with wanted parameter

            return parmsDict
        else:
            parmsDict["result"]=False
            return parmsDict



    # Read a parameter for all frequency cells of a particular time slot
    #
    # parameter        - name of parameter to read
    # start_time       - start of time range
    # end_time         - end of time range
    #
    def readFreqColumnTimeSlot(self, parameter, start_time, end_time, iteration="last"):
        # Get first all unique time slots
        if self.timeSlots.nrows()==0:
            self.timeSlots=getTimeSlots()

        # Create fuzzy times
        start_time, end_time = self.fuzzyTime(start_time, end_time)

        if iteration == "last":
           parmsDict["result"]="last"

           taqlcmd="SELECT STARTFREQ, ENDFREQ, " + parameter + " FROM " + self.tablename + " WHERE STARTTIME=" + start_time + " AND ENDTIME=" + end_time + " WHERE ITER=MAXITER ORDER BY ITER"
           selection=pt.taql(taqlcmd)           # execute TaQL command
           parmsDict["last"]=selection.getcol(parameter)    # select column with wanted parameter

           return parmsDict

        elif iteration == "all":
            parmsDict["result"]="all"

            for iter in range(1, self.getMaxIter()+1):
                taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, " + parameter + ", ITER FROM " + self.tablename + " WHERE STARTTIME=" + start_time + " AND ENDTIME=" + end_time
                selection=pt.taql(taqlcmd)           # execute TaQL command
                print selection                      # DEBUG
                parmsDict[str(iter)]=selection.getcol(parameter)    # select column with wanted parameter
            return parmsDict

        elif type(iteration).__name__ == "int":
            parmsDict["result"]="iteration"

            taqlcmd="SELECT STARTFREQ, ENDFREQ, " + parameter + " FROM " + self.tablename + " WHERE STARTTIME=" + start_time + " AND ENDTIME=" + end_time + " WHERE ITER=", str(iteration)
            selection=pt.taql(taqlcmd)
            parmsDict[str(iteration)]=selection.getcol(parameter)
            
            return parmsDict

        else:
            parmsDict["result"]="False"
            return parmsDict


    # Read a parameter for all time cells, default return all entries
    # for all time slots and iterations
    # Its return value depends on the query parameter iteration
    #
    # iteration=all (default) - a dictionary with entries 0,1,2,...,maxIter
    # iteration=x             - a numpy.ndarray for that iteration 
    # iteration=Last          - a numpy.ndarray with the last iterations entries
    #
    def readTimeColumn(self, parameter, iteration="all"):
        print "readTimeColumn(self, parameter, iteration=", iteration ,"):"   # DEBUG
        
        # Get first all unique time slots
        if self.timeSlots.nrows()==0:
            self.timeSlots=self.getTimeSlots()

        # Get MAXITER first
        maxIter=self.getMaxIter()
        print "maxIter: ", maxIter

        parmsDict={}

        # return all iterations (default behaviour)
        if iteration == "all":
           parmsDict["result"]="all"
   
           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iter)
                 selection=pt.taql(taqlcmd)              # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 print "readTimeColumn-type(parmIter): ", type(parmIter)
                 parmsDict[iter]=parmIter
           return parmsDict

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
           parmsDict["result"]="last"

           taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE LASTITER=TRUE"
           selection=pt.taql(taqlcmd)           # execute TaQL command
           parmDict["last"]=selection.getcol(parameter)    # select column with wanted parameter

           return parmsDict

        # return only a particular iteration
        elif type(iteration).__name__==int:
            parmsDict["result"]="iteration"

            taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTTIME"
            selection=pt.taql(taqlcmd)           # execute TaQL command      
            parmsDict[iteration]=selection.getcol(parameter)    # select column with wanted parameter

            return parmsDict

        else:
            parmsDict["result"]=False
            return parmsDict


    # Read a series of cells between start_freq/end_freq and start_time/end_time
    # Default is iteration="last", returning only the last solution
    #
    def readCells(self, start_time, end_time, start_freq, end_freq, iteration="last"):
        print "readCells(self, start_time, end_time, start_freq, end_freq)"     # DEBUG

        cellsDict={}       # create an empty dictionary

        # return all iterations (default behaviour)
        if iteration == "all":
           cellsDict["result"]="all"

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_freq) + " AND ENDTIME<=" + str(end_freq) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND ITER=" + str(iter)
                 cell[iter]=pt.taql(taqlcmd)           # execute TaQL command
           return cell

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
           print "readCells(): last"        # DEBUG

           cellsDict["result"]="last"

           taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_freq) + " AND ENDTIME<=" + str(end_freq) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND LASTITER=TRUE"
           cellsDict["last"]=pt.taql(taqlcmd)           # execute TaQL command
   
           return cellsDict

        # return only a particular iteration
        elif type(iteration).__name__ == "int":
            print "iteration: ", iteration    # DEBUG

            cellsDict["result"]="iteration"

            taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ=" + str(start_freq) + " AND ENDFREQ=" + str(end_freq) + " AND ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            selection=pt.taql(taqlcmd)        # execute TaQL command      

            cellsDict[iteration]=selection

            return cellsDict
        

    # Return a histogram of converged solutions, i.e. distribution
    # of iterations where solver converged or max iter
    #
    # TODO: This is better done in the plotting class
    def histogramConvergedIteration(self):
        print "histogramConvergedIteration():"

        # Get all converged solutions

        # make a histogram of the data


    # Get the message from the solver for a (series of cells)
    #
    def getMessages(self, start_time, end_time, start_freq, end_freq, iteration="last"):
        messagesDict={}       # create an empty dictionary
        # return all iterations (default behaviour)
        if iteration == "all":
           messagesDict["result"]="all"

           # Loop over all iterations
           for iter in range(1, self.getMaxIter()+1):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_time) + " AND ENDTIME<=" + str(end_time) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND ITER=" + str(iter)
                 result=pt.taql(taqlcmd)           # execute TaQL command
                 messagesDict[iter]=result.getcol("MESSAGE")

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
           #print "readCells(): last"        # DEBUG
           messagesDict["result"]="last"

           taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_time) + " AND ENDTIME<=" + str(end_time) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND LASTITER=TRUE"
           result=pt.taql(taqlcmd)           # execute TaQL command
           
           print "result.nrows() = ", result.nrows()
           
           messagesDict["last"]=result.getcol("MESSAGE")

        # return only a particular iteration
        elif type(iteration).__name__ == "int":
            #print "iteration: ", iteration    # DEBUG
            messagesDict["result"]="iteration"
            taqlcmd="SELECT * FROM " + self.tablename + + " WHERE STARTTIME>=" + str(start_time) + " AND ENDTIME<=" + str(end_time) + " AND STARTFREQ=" + str(start_freq) + " AND ENDFREQ=" + str(end_freq) + " AND ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            result=pt.taql(taqlcmd)        # execute TaQL command      
            
            messagesDict[iteration]=result.getcol("MESSAGE")

        return messagesDict



    # This function returns teh RANK of the Solutions, it returns the first one
    # found in the table, unless a specific start_time, end_time, start_freq,
    # end_freq is provided
    #
    def getRank(self, start_time=None, end_time=None, start_freq=None, end_freq=None):
        if start_time == None or end_time == None or start_freq == None or end_freq == None:
            rank=self.readParameterIdx("RANK", 0)
        else:
            taqlcmd="SELECT RANK FROM " + self.tablename + " WHERE STARTTIME >= " + str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE"

            result=pt.taql(taqlcmd)
            rank=result.getcol("RANK")

        return rank


    # Get the rank deficancy for a particular cell
    #
    def getRankDef(self, start_time=None, end_time=None, start_freq=None, end_freq=None):
        if start_time == None or end_time == None or start_freq == None or end_freq == None:
            rank=self.readParameterIdx("RANK", 0)
        else:
            taqlcmd="SELECT RANKDEF FROM " + self.tablename + " WHERE STARTTIME >= " + str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND LASTITER=TRUE"
            result=pt.taql(taqlcmd)
            rankdef=result.getcol("RANKDEF")

        return rankdef


    #*******************************************************
    #
    # Helper functions
    #
    #*******************************************************

    # Set TIMING of (most important) query functions
    # activate=True activates the timing
    # activate=False turns it off
    #
    def setTIMING(self, activate=True):
        self.TIMING=activate


    # Get bool attribute if TIMING is activated
    #
    def getTIMING(self):
        return self.TIMING


    # Get a table of unique STARTTIME, ENDTIME pairs
    # which give all the time slots of the Measurementset
    #
    def setTimeSlots(self):
        print "SolverQuery::setTimeSlots()"       # DEBUG
        taqlcmd="SELECT UNIQUE STARTTIME, ENDTIME FROM " + self.tablename       
        self.timeSlots=pt.taql(taqlcmd)

    def getTimeSlots(self):
        return self.timeSlots

    # Return the unique STARTTIMES present in the
    # Measurementset
    #
    def getStartTimes(self):
        taqlcmd="SELECT UNIQUE STARTTIME FROM " + self.tablename + " ORDER BY STARTTIME"
        self.startTimes=pt.taql(taqlcmd)


    # Return the unique ENDTIMES present in the
    # Measurementset
    #
    def getEndTimes(self):
        taqlcmd="SELECT UNIQUE ENDTIME FROM " + self.tablename
        self.endTimes=pt.taql(taqlcmd)


    # Return (a slice of) the midtimes of each timeslot
    # The slice is defined by a startIdx and an endIdx
    # default return value is the whole list
    #
    # startIdx         - start index to take slice (default=0), this can also be a time stamp of type flaot
    # endIdx           - end index to take slice (default='end'), this can also be a time stamp of type float
    #
    def getMidTimes(self, startIdx=0, endIdx="end"):
        sliceMidTimes=[]

        if len(self.timeSlots) == 0:
            raise "getMidTimes(): timeSlots are 0"

        if len(self.midTimes) != 0:     # If midTimes have already been determined for this table instance
            # If no limit indices are given, return all midTimes
            if startIdx==0 and endIdx=="end":            
                return self.midTimes
        else:                           # otherwise compute them first from the TimeSlots start and end times
            self.midTimes=[]            # clear the list first
            for i in range(0, len(self.timeSlots)):
                self.midTimes.append(self.timeSlots[i]['STARTTIME'] + 0.5*(self.timeSlots[i]['ENDTIME']-self.timeSlots[i]['STARTTIME']))

        # If all midTimes are requested (default):
        if startIdx==0 and endIdx=="end": 
            return self.midTimes
        # If the user specified floats, i.e. a start and end time, look for them in the interval
        elif isinstance(startIdx, float) and isinstance(endIdx, float):

            startIdx, endIdx = self.fuzzyTime(startIdx, endIdx)

            for i in range(0, len(self.midTimes)):
                if self.midTimes[i] >= startIdx and self.midTimes[i] <= endIdx:
                    sliceMidTimes.append(self.midTimes[i])

            #print "len(sliceMidTimes): ", len(sliceMidTimes)  # DEBUG

            return sliceMidTimes
        else:
            return self.midTimes[startIdx:endIdx]


    # Set a table of unique STARTTIME, ENDTIME pairs
    # which give all the time slots of the Measurementset
    # only compute them once
    #
    def setFreqs(self):
        if len(self.frequencies) == 0:
            taqlcmd="SELECT UNIQUE STARTFREQ, ENDFREQ FROM " + self.tablename
            self.frequencies=pt.taql(taqlcmd)
            
            self.startFreqs=self.frequencies.getcol("STARTFREQ")
            self.endFreqs=self.frequencies.getcol("ENDFREQ")


    # Get table of frequencies with STARTFREQ, ENDFREQ column
    #
    def getFreqs(self):
        return self.frequencies


    # Return the number of distinct time slots
    #
    def getNumTimeSlots(self):
        return self.getTimeSlots().nrows()


    # Return the number of distinct frequencies
    #
    def getNumFreqs(self):
        return self.getFreqs().nrows()


    # Read the unique STARTTIMES present in the
    # Measurementset
    #
    def setStartFreqs(self):
        taqlcmd="SELECT UNIQUE STARTFREQ FROM " + self.tablename + " ORDER BY STARTTIME"
        self.startFreqs=pt.taql(taqlcmd)


    # Return the unique STARTTIMES present in the
    # Measurementset
    #
    def getStartFreqs(self):
        return self.startFreqs


    # Read the unique ENDTIMES present in the
    # Measurementset
    #
    def setEndFreqs(self):
        taqlcmd="SELECT UNIQUE ENDFREQ FROM " + self.tablename
        self.endFreqs=pt.taql(taqlcmd)


    # Return the unique ENDTIMES present in the
    # Measurementset
    #
    def getEndFreqs(self):
        return self.endFreqs


    # Read the MAXITER value from the solver table Measurementset,
    #
    def setMaxIter(self):
        self.maxIter=self.solverTable.getkeywords()['MaxIter']


    # Get the maximum number of iterations
    #
    def getMaxIter(self):
        return self.maxIter


    # Use machine epsilon value and time request to make a 
    # fuzzy time interval
    #
    def fuzzyTime(self, start_time, end_time):
        machineEps=np.finfo(np.double).eps

        fuzzy_start_time=start_time - machineEps - 0.005
        fuzzy_end_time=end_time + machineEps + 0.005    # <- value is a dirty hack

        return fuzzy_start_time, fuzzy_end_time

    
    # Use reasonable LOFAR frequency selection accuracy to make
    # fuzzy frequency interval
    #
    def fuzzyFreq(self, start_freq, end_freq):
        freqEpsilon=0.8    # (with 0.767kHZ channel width a 1Hz "epsilon" should be fine)

        fuzzy_start_freq=start_freq-freqEpsilon       
        fuzzy_end_freq=end_freq+freqEpsilon

        return fuzzy_start_freq, fuzzy_end_freq


    # Read the table columns from the table 
    #
    def readColumnNames(self):
        #print "type(self.solverTable): ", type(self.solverTable) 
        columnNames=self.solverTable.colnames()
        return columnNames


    # Check if a parameter exists in the table
    #
    # parameter       - name of parameter to check for
    #
    def parameterExists(self, parameter):
        columnNames=self.readColumnNames()
        for name in columnNames:
            if name == parameter:
                exists=True
                return exists
            else:
                exists=False

        return exists


    # Read the type of the table from the MS: 
    # PERITERATION, PERSOLUTION or (TODO: PERSOLUTION_CORRMATRIX, PERITERATION_CORRMATRIX)
    #
    def setType(self):
        print "setType()"

        tablekeywords=self.solverTable.getkeywords()   # get all the table keywords
        keys=tablekeywords.keys()

        if "Logginglevel" in keys:
            loglevel=self.solverTable.getkeyword("Logginglevel")
        else:
            loglevel="Unknown"

        self.type=loglevel


    # Return the type of the table: 
    # PERITERATION, PERSOLUTION or PERSOLUTION_CORRMATRIX
    #
    def getType(self):            
        return self.type


    # Read the available solver parameter names from the table
    # this excludes the cell parameters STARTTIME, ENDTIME,
    # STARTFREQ, ENDFREQ and ITER
    #
    def setParameterNames(self):
        # First get all column names
        columnNames=self.readColumnNames()

        # Then remove STARTTIME, ENDTIME, STARTFREQ and ENDFREQ from the list
        #print "type(columnNames): ", type(columnNames)
        # Do this manually it is faster:
        del columnNames[0:4]    
        # Delete  (which is a bool value only)
        del columnNames[2]

        self.parameterNames=columnNames


    # Return the parameter names stored in the SolverQuery object
    #
    def getParameterNames(self):
        return self.parameterNames


    # Return iteration number at which solution has converged
    # or has hit MAXIMUM number of iterations reached
    # The return value is an integer
    #
    def getConvergedIteration(self, start_time, end_time, start_freq, end_freq):
        # Create fuzzy times and frequencies
        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)

        taqlcmd="SELECT STARTTIME, ENDTIME, ITER FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_time) + " AND ENDTIME<=" + str(end_time) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND LASTITER=TRUE"
        
        result=pt.taql(taqlcmd)              # execute TaQL command
        iteration=result.getcol("ITER")      # get ITER parameter

        return iteration


    # Return the value of a parameter on convergence
    # The selection is done on LASTITER=TRUE and a single value (if one cell is found)
    # or a numpy.ndarray is returned
    #
    def getConvergedParameter(self, parameter, start_time, end_time, start_freq, end_freq):
        # Create fuzzy times and frequencies
        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyFreq(start_freq, end_freq)

        taqlcmd="SELECT STARTTIME, ENDTIME, LASTITER, " + parameter + " FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_time) + " AND ENDTIME<=" + str(end_time) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND LASTITER=TRUE"

        print "taqlcmd = ", taqlcmd          # DEBUG
        
        result=pt.taql(taqlcmd)              # execute TaQL command
        selection=result.getcol(parameter)   # get parameter column

        return selection 
