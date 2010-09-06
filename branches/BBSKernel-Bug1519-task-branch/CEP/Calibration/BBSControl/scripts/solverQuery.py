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
# Last change:   	2010/09/03
#


# import pyrap as pr
import pyrap.tables as pt
import numpy as np            # numpy needed for machine epsilon value


class SolverQuery:
    # Empty constructor (does not open the table immediately)
    def __init__(self):
        print "Empty constructor called"
        # do nothing

    # Default constructor, opens the table of name (default: "solver")
    #
    def __init__(self, tablename=""):
        try:
            # Reset frequencies and time vectors that are used for parameter retrieval
            self.frequencies=[]
            self.timeSlots=[]

            self.startFreqs=[]
            self.endFreqs=[]
            self.startTimes=[]
            self.endTimes=[]
            self.parameterNames=""


            if tablename is not "":
                self.tablename=tablename               # keep tablename in object
                self.solverTable=pt.table(tablename)
                self.parameterNames=self.readParameterNames()

                #return self   # return also the object for calls from the outside?
        except ValueError:
            traceback.print_exc()


    # Open the solver table
    #
    def open(self, tablename):
        try:
            self.tablename=tablename
            self.solverTable=pt.table(tablename)
            self.parameterNames=self.readParameterNames()

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
        self.tablename=""
        self.parameterNames=""

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
    def readParameter(self, parameter_name, start_time, end_time, start_freq, end_freq, iteration="last", sort_list="STARTFREQ, STARTTIME"):
        print "readParameter(STARTTIME, STARTFREQ, ENDTIME, ENDFREQ )"   # DEBUG

        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)
  
        colnames=self.solverTable.colnames()
        column_name=colnames[parameter_name]  # lookup column name for the parameter asked for

        if column_name is not "":

            query = "STARTTIME >= "+ start_time + "AND ENDTIME <= " + end_time + " AND STARTFREQ >= "+ start_freq + " AND ENDFREQ <= " + end_freq

            # Distinguish between specific iteration, last iteration or all iterations
            #
            parmsDict={}     # create empty dictionary

            if iteration is "last":
                print "readParameter(): last"
                # Fetch requested parameter for time and freq where LASTITER=1
                query = query + " AND LASTITER=1"
                selection=self.solverTable.query()
                parameter=selection.getcol(parameter_name)

                parmsDict["last"]=parameter
                return paramsDict  # return type depends on type of parameter itself?

            elif type(iteration).__name__ is "int":
                print "readParameter(): specific iteration number"
                # Fetch requested parameter for time and freq where ITER=x
                query = query + " AND ITER=" + str(iteration)
                selection=self.solverTable.query()
                parameter=selection.getcol(parameter_name)

                parmsDict[iteration]=parameter
                return parmsDict   # return dictionary with one entry

            elif iteration is "all":
                print "readParameter() all iterations"
                # Fetch requested parameter for all iterations with that time and freq
                # and sort them into a dictionary

                # Loop over all iterations
                for iter in range(1, maxIter):
                    taqlcmd="SELECT * FROM " + self.tablename + " WHERE ITER=", str(iter)
                    selection=pt.taql(taqlcmd)              # execute TaQL command
                    parmIter=selection.getcol(parameter)    # select column with wanted parameter
                    parmsDict[iter]=parmIter
                
                return parmsDict    # return type is Dictionary
            else:
                print "readParameter() unknown iteration keyword"
                return False
            

        else:         # If column_name is ""
            print "readParameter: wrong parameter name"
            return False


    # Read a parameter of a cell of a particular iteration
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned (default sorting by FREQ first and then TIME)
    #
#    def readParameter(self, parameter_name, start_time, end_time, start_freq, end_freq, iteration="Last"):
#        print "readParameter(parameter_name, STARTTIME, ENDTIME, STARTFREQ, ENDFREQ, Iteration=", iteration, " )"   # DEBUG
#        start_time, end_time=self.fuzzyTime(start_time, end_time)
#        start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)
#
#        query = "STARTTIME >= " + str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) 
#
#        if iteration=="Last":    # default: return last iteration only
#            selection = self.solverTable.query(query)
#            print "type(selection): ", type(selection)        # DEBUG
#            print "selection.nrows(): ", selection.nrows()    # DEBUG
#        else:                                           # return all iterations
#            query += " AND ITER==" + str(iteration)    # apply sorting by iteration
#
#            selection = self.solverTable.query(query)   # query selection
#
#        result = selection.getcol(parameter_name)   # select column from selection
#        print "readParameter::type(result): ", type(result)   # DEBUG
#        return result
#

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

         # Get first all iterations for that start_time/end_time, start_freq/end_freq
         query="STARTTIME >= "+ str(start_time) + "AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + "sortlist='ITER'"                     # sort by ITER

         selection=self.solverTable.query(query)             # execute query
         result=selection[selection.nrows()-1]   # set result to last element of selection

         return result


    # Find the last iteration in a mixed result table for all
    # unique frequencies/timeslots
    #
    # intermediate parameter might be a pyrap table or
    # a numpy.ndarray
    #
    def findLastIteration(self, data):
        print "findLastIteration(self, data)"

        print "type(data): ", type(data)

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
            print "findLastIteration: unknown type"
            return False



    # Get the solution vector from the solver table
    # for a particular cell
    #
    def getSolution(self, start_time, end_time, start_freq, end_freq, iteration="all"):
        #print "getSolution(self, pt.table)"      # DEBUG

        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)

        print "start_time = ", start_time
        print "end_time = ", end_time
        print "start_freq = ", start_freq
        print "end_freq = ", end_freq

        solutionsDict={}

        if iteration=="all":
            print "getSolution: get all iterations"
            taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq)
            selection=pt.taql(taqlcmd)

            for iter in range(1, selection.nrows()):
                solution=selection.getcol("SOLUTION")
                solutionsDict[iter]=solution[iter-1]

            return solutionsDict

        elif iteration=="last":
            print "getSolution: get last iteration"
            taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + "AND LASTITER=1"        
            selection=pt.taql(taqlcmd)       
            solutionDict["last"]=selection.getcol("SOLUTION")
            return solutionDict

        elif type(iteration).__name__ == "int":
            print "getSolution: iteration ", iteration
            taqlcmd="SELECT STARTTIME, ENDTIME, ITER, SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND ITER=", str(iteration)        
            selection=pt.taql(taqlcmd)       
            solutionsDict[iteration]=selection.getcol("SOLUTION")

            return solutionsDict

        else:
            print "getSolution: unknown iteration keyword"
            return False



    # Get the correlation matrix for a particular cell
    # The correlation matrix has the dimensions n,n where n=RANK
    #
    # TODO: Test this with corrlation matrix output from solver solution
    #
    def getCorrMatrix(self, start_time, end_time, start_freq, end_freq, iteration="all"):
        print "getCorrMatrix(self, start_time, end_time, start_freq, end_freq, iteration=", iteration ,")"   # DEBUG

        start_time, end_time=self.fuzzyTime(start_time, end_time)
        start_freq, end_freq=self.fuzzyTime(start_freq, end_freq)        

        taqlcmd="SELECT CORRMATRIX FROM " + self.tablename + " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq)

        print "getCorrMatrix: taqlcmd: ", taqlcmd                             # DEBUG
        selection=pt.taql(taqlcmd)
        print "getCorrMatrix: ", selection.nrows(), " ", selection.ncols()    # DEBUG

        print "getCorrMatrix: type(selection): ", type(selection)             # DEBUG

        # Rearrange entries in a numpy,ndarray?

        return selection


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


    # Read a complete cell (including all iterations if present)
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned
    #
    #def readCell(self, start_time, end_time, start_freq, end_freq, sort_list="STARTFREQ, STARTTIME"):
     #   # Check if that STARTTIME/ENDTIME,STARTFREQ/ENDFREQ-Cell exists in the table
     #   if self.cellExists(start_time, start_freq, end_time, end_freq)==False:
     #       # Build query string
     #       query = "STARTTIME >= "+ str(start_time) + "AND ENDTIME <= " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + ", sortlist=sort_list"

      #      # Use Pyrap function to return a complete row of the table
      #      selection= self.solverTable.query(query)

       #     return selection
       # else:
       #      return False


    # Read a complete cell, but only for a particular iteration
    # the given times and frequencies specify an inclusive search interval
    #
   # def readCell(self, start_time, end_time, start_freq, end_freq, iteration="Last"):
        #print "readCell(STARTTIME, ENDTIME, STARTFREQ, ENDFREQ, Iteration=", iteration, " )"   # DEBUG
   #     query = "STARTTIME = "+ str(start_time) + " AND ENDTIME = " + str(end_time) + " AND STARTFREQ >= " + str(start_freq) + " AND ENDFREQ = " + str(end_freq) + " , sortlist=ITER"

        # Use Pyrap function to return a complete row of the table
        #print "query = ", query        # DEBUG
   #     result=self.solverTable.query(query)
        #print "result.nrows() = ", result.nrows()

   #     if iter=="last":                      # default: return last iteration only
   #         result=result[result.len()-1]
   #         return result
   #     elif iter=="all":
   #         return result
   #     elif type(iter).__name__=="int":      # If a particular iteration was requested
   #         if iteration > result.nrows():    # if the iteration asked for is greater than the number of rows
   #             return False
   #         else:
   #             return result[iteration-1]     # iteration counting starts with 1, but index with 0
   #     else:                                  # Unknown iteration keyword given
   #        return False


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

        # return all iterations (default behaviour)
        if iteration == "all":
           cell={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ >=" + str(start_freq) + " AND ENDFREQ <= " + str(end_freq) + " AND ITER = " + str(iter)
                 cell[iter]=pt.taql(taqlcmd)           # execute TaQL command
           return cell

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
           cell={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE ITER=", str(iter)
                 selection=pt.taql(taqlcmd)           # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 parmsDict[iter]=parmIter

           # Collect last iteration from dictionary entries
           parms={}
           for i in range(1, len(parmsDict)):
               parms[i]=parmsDict[i][len(parmsDict[i])-1]
           return parms

        # return only a particular iteration
        else:
            #print "iteration: ", iteration    # DEBUG
            taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ=" + str(start_freq) + " AND ENDFREQ=" + str(end_freq) + " AND ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            print "taqlcmd: ", taqlcmd
            selection=pt.taql(taqlcmd)           # execute TaQL command      

            return selection



    # Read a parameter for all frequency cells (of one particular time slot)
    # Its return value depends on the query parameter iteration
    #
    # iteration=all (default) - a dictionary with entries 0,1,2,...,maxIter
    # iteration=x             - a numpy.ndarray for that iteration 
    # iteration=Last          - a numpy.ndarray with the last iterations entries
    #
    def readFreqColumn(self, parameter, iteration="all"):
        #print "readFreqColumn(self, parameter, iteration=", iteration, ":"    # DEBUG

        # Get first all unique frequencies
        if len(self.frequencies)==0:
            self.frequencies=getFreqs()

        # Get MAXITER first
        maxIter=pt.tablecolumn(self.solverTable, "MAXITER")[0]

        # return all iterations (default behaviour)
        if iteration == "all":
           parms={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iter)
                 selection=pt.taql(taqlcmd)              # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 parms[iter]=parmIter                    # write into dictionary__
           return parms

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
            taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, " + parameter + " FROM " + self.tablename + " WHERE LASTITER=TRUE"
            selection=pt.taql(taqlcmd)           # execute TaQL command
            parms=selection.getcol(parameter)    # select column with wanted parameter
            return parms

        # return only a particular iteration
        else:
            #print "iteration: ", iteration    # DEBUG
#            taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTFREQ"

            taqlcmd="SELECT " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            print "taqlcmd: ", taqlcmd           # DEBUG
            selection=pt.taql(taqlcmd)           # execute TaQL command      
            parms=selection.getcol(parameter)    # select column with wanted parameter

            return parms


    # Read a parameter for all frequency cells of a particular time slot
    #
    def readFreqColumnTimeSlot(self, parameter, start_time, end_time, iteration="last"):
        # Get first all unique time slots
        if self.timeSlots.nrows()==0:
            self.timeSlots=getTimeSlots()

        if iteration == "last":
            taqlcmd="SELECT STARTFREQ, ENDFREQ, " + parameter + " FROM " + self.tablename + " WHERE STARTTIME=" + start_time + " AND ENDTIME=" + end_time + " WHERE ITER=MAXITER ORDER BY ITER"
            selection=pt.taql(taqlcmd)           # execute TaQL command
            parms=selection.getcol(parameter)    # select column with wanted parameter
            return parms[parms.size-1]
        else:
            taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, " + parameter + ", ITER FROM " + self.tablename + " WHERE STARTTIME=" + start_time + " AND ENDTIME=" + end_time
            selection=pt.taql(taqlcmd)           # execute TaQL command
            print selection                      # DEBUG
            parms=selection.getcol(parameter)    # select column with wanted parameter
            return parms



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

        # return all iterations (default behaviour)
        if iteration == "all":
   
           parms={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iter)
                 selection=pt.taql(taqlcmd)           # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 print "readTimeColumn-type(parmIter): ", type(parmIter)
                 parms[iter]=parmIter
           return parms

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":

           parmsDict={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter):
                 #taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=", str(iter)
                 taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=", str(iter)
                 selection=pt.taql(taqlcmd)           # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 parmsDict[iter]=parmIter
                 print "parmIter: ", parmIter

           # Collect last iteration from dictionary entries
           parms={}
           for i in range(1, len(parmsDict)):
               parms[i]=parmsDict[i][len(parmsDict[i])-1]
           return parms

        # return only a particular iteration
        elif type(iteration).__name__==int:
            taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTTIME"
            print "taqlcmd: ", taqlcmd
            selection=pt.taql(taqlcmd)           # execute TaQL command      
            parms=selection.getcol(parameter)    # select column with wanted parameter

            return parms


    # Read a series of cells between start_freq/end_freq and start_time/end_time
    #
    def readCells(self, start_time, end_time, start_freq, end_freq):
        print "readCells(self, start_time, end_time, start_freq, end_freq)"     # DEBUG

        # return all iterations (default behaviour)
        if iteration == "all":
           cell={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_freq) + " AND ENDTIME<=" + str(end_freq) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND ITER=" + str(iter)
                 cell[iter]=pt.taql(taqlcmd)           # execute TaQL command
           return cell

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":
           cell={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE ITER=", str(iter)
                 selection=pt.taql(taqlcmd)           # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 parmsDict[iter]=parmIter

           # Collect last iteration from dictionary entries
           parms={}
           for i in range(1, len(parmsDict)):
               parms[i]=parmsDict[i][len(parmsDict[i])-1]
           return parms

        # return only a particular iteration
        else:
            #print "iteration: ", iteration    # DEBUG
            taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ=" + str(start_freq) + " AND ENDFREQ=" + str(end_freq) + " AND ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            print "taqlcmd: ", taqlcmd
            selection=pt.taql(taqlcmd)           # execute TaQL command      

            return selection
        

    #*******************************************************
    #
    # Helper functions
    #
    #*******************************************************

    # Get a table of unique STARTTIME, ENDTIME pairs
    # which give all the time slots of the Measurementset
    #
    def getTimeSlots(self):
        taqlcmd="SELECT UNIQUE STARTTIME, ENDTIME FROM " + self.tablename
        timeslots=pt.taql(taqlcmd)
        
        self.timeSlots=timeslots     # set class variable
       
        return timeslots


    # Return the unique STARTTIMES present in the
    # Measurementset
    #
    def getStartTimes(self):
        taqlcmd="SELECT UNIQUE STARTTIME FROM " + self.tablename
        starttimes=pt.taql(taqlcmd)

        self.startTimes=starttimes
        return starttimes


    # Return the unique ENDTIMES present in the
    # Measurementset
    #
    def getEndTimes(self):
        taqlcmd="SELECT UNIQUE ENDTIME FROM " + self.tablename
        endtimes=pt.taql(taqlcmd)

        self.endTimes=endtimes

        return endtimes        
 

    # Get a table of unique STARTTIME, ENDTIME pairs
    # which give all the time slots of the Measurementset
    # only compute them once
    #
    def getFreqs(self):
        if len(self.frequencies) == 0:
            taqlcmd="SELECT UNIQUE STARTFREQ, ENDFREQ FROM " + self.tablename
            self.frequencies=pt.taql(taqlcmd)

            return self.frequencies
        else:
            return self.frequencies


    # Return the number of distinct time slots
    #
    def getNumTimeSlots(self):
        #print "getNumTimeSlots(self): ", type(self.getTimeSlots())    # DEBUG
        return self.getTimeSlots().nrows()


    # Return the number of distinct frequencies
    #
    def getNumFreqs(self):
        print "getNumFreqs(self): ", type(self.getFreqs())             # DEBUG
        return self.getFreqs().nrows()

    # Return the unique STARTTIMES present in the
    # Measurementset
    #
    def getStartFreqs(self):
        taqlcmd="SELECT UNIQUE STARTFREQ FROM " + self.tablename
        startfreqs=pt.taql(taqlcmd)

        return startfreqs


    # Return the unique ENDTIMES present in the
    # Measurementset
    #
    def getEndFreqs(self):
        taqlcmd="SELECT UNIQUE ENDFREQ FROM " + self.tablename
        endfreqs=pt.taql(taqlcmd)

        return endfreqs        


    # Get the MAXITER value from the solver table Measurementset
    #
    def getMaxIter(self):
        maxIter=0
        maxIter=pt.tablecolumn(self.solverTable, "MAXITER")[0]
        return maxIter


    # Use machine epsilon value and time request to make a 
    # fuzzy time interval
    #
    def fuzzyTime(self, start_time, end_time):
        machineEps=np.finfo(np.double).eps

        fuzzy_start_time=start_time - machineEps
        fuzzy_end_time=end_time + machineEps

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
    def parameterExists(self, parameter):
        columnNames=self.readColumnNames()
        for name in columnNames:
            if name == parameter:
                exists=True
                return exists
            else:
                exists=False

        return exists


    # Read the available solver parameter names from the table
    # this excludes the cell parameters STARTTIME, ENDTIME,
    # STARTFREQ, ENDFREQ and ITER
    #
    def readParameterNames(self):
        print "readParameterNames(self)"
        
        # First get all column names
        columnNames=self.readColumnNames()

        # Then remove STARTTIME, ENDTIME, STARTFREQ and ENDFREQ from the list
        #print "type(columnNames): ", type(columnNames)
        # Do this manually it is faster:
        del columnNames[0:4]    
        # Delete LASTITER (which is a bool value only)
        del columnNames[2]
        
        return columnNames

    # Return the parameter names stored in the SolverQuery object
    #
    def getParameterNames(self):
        if self.parameterNames=="":
            self.parameterNames=self.readParameterNames()
            return self.parameterNames
        else:
            return self.parameterNames


    # Return iteration number at which solution has converged
    # or has hit MAXIMUM number of iterations reached
    def getConvergedIteration(self, start_time, end_time, start_freq, end_freq):
        taqlcmd="SELECT ITER FROM " + self.tablename + " WHERE STARTTIME>=" + str(start_freq) + " AND ENDTIME<=" + str(end_freq) + " AND STARTFREQ>=" + str(start_freq) + " AND ENDFREQ<=" + str(end_freq) + " AND LASTITER=1"
        
        result=pt.taql(taqlcmd)           # execute TaQL command        
        iteration=result.getcol("ITER")   # get ITER parameter

        return iteration
