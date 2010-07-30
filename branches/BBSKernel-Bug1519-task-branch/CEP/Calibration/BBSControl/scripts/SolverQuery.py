# Solver class handles reading statistics from the "solver" table logged by
# ParmDBLog into a CASA table
#
# File:			SolverQuery.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:          	2010/07/16
# Last change:   	2010/07/29
#


# import pyrap as pr
import pyrap.tables as pt


class SolverQuery:
    # Empty constructor (does not open the table immediately)
    def __init__(self):
        print "Empty constructor called"
        # do nothing

    # Default constructor, opens the table of name (default: "solver")
    #
    def __init__(self, tablename="solver"):
        try:
            # Reset frequencies and time vectors that are used for parameter retrieval
            self.frequencies=0
            self.timeSlots=0

            self.startFreqs=0
            self.endFreqs=0
            self.startTimes=0
            self.endTimes=0

            self.tablename=tablename               # keep tablename in object
            self.solverTable=pt.table(tablename)
        except ValueError:
            traceback.print_exc()


    # Flush and close the table of the Solver object
    def close(self):
        self.frequencies=0
        self.solverTable.close()


    # Open solverDb (=table)
    # Default name: solver
    #
    def openSolverTable( self, tablename="solver" ):
        try:
            solverTable=pt.table(tablename)
            return solverTable
        except ValueError:
            traceback.print_exc()
 

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
   

    # Build a "standard" query string by searching for a parameter
    def buildQuery( self, colname, start_time, end_time, start_freq, end_freq ):
        query = "Starttime >= " + start_time + " AND Endtime <= " + end_time + " AND STARTFREQ >= " + start_freq + " AND ENDFREQ <= " + end_freq + ", sortlist='FREQ, TIME'"

        print "Query = ", query       # DEBUG


    # Perform a query on the solver table
    def performQuery( self, query, colname=None):
        results = self.solverTable.query(query)              # perform query on table

        if(colname != None):                     # If a colname to be selected was given...
            results = results.getcol(colname)    # select column with parameter name

        return results


    # Read a parameter of a cell (including all iterations if present)
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned (default sorting by FREQ first and then TIME)
    #
    def readParameter(self, parameter_name, start_time, end_time, start_freq, end_freq, sort_list="STARTFREQ, STARTTIME"):
        print "readParameter(STARTTIME, STARTFREQ, ENDTIME, ENDFREQ )"   # DEBUG
  
        desc=self.tabledesc()             # Get table description (dictionary)
        column_name=desc[parameter_name]  # lookup column name for the parameter asked for

        query = "STARTTIME >= "+ start_time + "AND ENDTIME <= " + end_time + " AND STARTFREQ >= "+ start_freq + " AND ENDFREQ <= " + end_freq + ", sortlist=sort_list"
        selection=self.solverTable.query()
        parameter=selection.getcol(parameter_name)


    # Read a parameter of a cell of a particular iteration
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned (default sorting by FREQ first and then TIME)
    #
    def readParameter(self, parameter_name, start_time, end_time, start_freq, end_freq, iteration="Last"):
        print "readParameter(parameter_name, STARTTIME, ENDTIME, STARTFREQ, ENDFREQ, Iteration=", iteration, " )"   # DEBUG

        query = "STARTTIME = " + str(start_time) + " AND ENDTIME = " + str(end_time) + " AND STARTFREQ = " + str(start_freq) + " AND ENDFREQ = " + str(end_freq) 

        if iteration=="Last":    # default: return last iteration only
            #query += ", sortlist=" + sort_list          # apply sorting

            print "Query = ", query                     # DEBUG

            selection = self.solverTable.query(query)
            print "type(selection): ", type(selection)
            print "selection.nrows(): ", selection.nrows()
        else:                                           # return all iterations
            query += " AND ITER==" + str(iteration)    # apply sorting by iteration

            selection = self.solverTable.query(query)   # query selection

        result = selection.getcol(parameter_name)   # select column from selection
        print "readParameter::type(result): ", type(result)
        return result


    # Check if a cell for a particular STARTTIME/ENDTIME,STARTFREQ/ENDFREQ-Cell
    # exists in the solver table
    #
    def cellExists(self, start_time, end_time, start_freq, end_freq):
         query="STARTTIME = "+ start_time + "AND ENDTIME = " + end_time + " AND STARTFREQ = " + start_freq + " AND ENDFREQ = " + end_freq
       
         result=self.solverTable.query(query)

         if result.nrows() >= 0:
             return true
         else:
             return false


    # Find the last iteration in an subtable, numpy.ndarray
    # for a particular cell with
    # start_time, start_freq, end_time, end_freq
    #
    #
    def findLastIterationCell(self, start_time, end_time, start_freq, end_freq):
         print "findLastIteration(self, start_time, end_time, start_freq, end_freq)"

         # Get first all iterations for that start_time/end_time, start_freq/end_freq
         query="STARTTIME == "+ str(start_time) + "AND ENDTIME == " + str(end_time) + " AND STARTFREQ == " + str(start_freq) + " AND ENDFREQ == " + str(end_freq) + "sortlist='ITER'"                     # sort by ITER

         selection=self.solverTable.query(query)             # execute query
         result=selection[selection.nrows()-1]   # set result to last element of selection

         return result


    # Find the last iteration in a mixed result table for all
    # unique frequencies/timeslots
    #
    # intermediate parameter might be a pyrap table or
    # a numpy.ndarray
    #
    def findLastIteration(self, intermediate):
        print "findLastIteration(self, intermediate)"

        print "type(intermediate): ", type(intermediate)

        # Determine type of paramter intermediate
        if type(intermediate) == numpy.ndarray:
            print "numpy.ndarray"       # DEBUG
        elif type(intermediate) == pyrap.table.table:
            print "pyrap.table.table"   # DEBUG


    # Get the solution vector from the solver table
    # for a particular cell
    #
    def getSolution(self, start_time, end_time, start_freq, end_freq, iteration="all"):
        print "getSolution(self, pt.table)"
 
        #query="STARTTIME = "+ str(start_time) + " AND ENDTIME = " + str(end_time) + " AND STARTFREQ = " + str(start_freq) + " AND ENDFREQ = " + str(end_freq) " AND ITER = " + str(iteration) #+ ", sortlist=ITER"

        taqlcmd="SELECT SOLUTION FROM " + self.tablename +  " WHERE STARTTIME >= "+ str(start_time) + " AND ENDTIME <= " + str(end_time) 
        
        print "ENDTIME - STARTTIME = ", end_time - start_time
        print "taqlcmd: ", taqlcmd
        selection=pt.taql(taqlcmd)

        print "selection.nrows(): ", selection.nrows()

        result=selection.getcol("SOLUTION")
        print result

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

        # return all iterations (default behaviour)
        if iteration == "all":
           cell={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter):
                 taqlcmd="SELECT * FROM " + self.tablename + " WHERE STARTFREQ=" + str(start_freq) + " AND ENDFREQ=" + str(end_freq) + " AND ITER=" + str(iter)
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
        if self.frequencies==0:
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
           parmsDict={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter):
                 taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=", str(iter)
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
#            taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTFREQ"

            taqlcmd="SELECT DISTINCT STARTFREQ, ENDFREQ, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iteration) + " ORDERBY STARTFREQ"
            print "taqlcmd: ", taqlcmd           # DEBUG
            selection=pt.taql(taqlcmd)           # execute TaQL command      
            parms=selection.getcol(parameter)    # select column with wanted parameter

            return parms


    # Read a parameter for all frequency cells of a particular time slot
    #
    def readFreqColumnTimeSlot(self, parameter, start_time, end_time, iteration="last"):
        # Get first all unique frequencies
        if self.timeSlots==0:
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
        if self.timeSlots==0:
            self.timeSlots=self.getTimeSlots()

        # Get MAXITER first
        maxIter=self.getMaxIter()
        print "maxIter: ", maxIter

        # return all iterations (default behaviour)
        if iteration == "all":
   
           parms={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER " + parameter + " FROM " + self.tablename + " WHERE ITER=" + str(iter)
                 selection=pt.taql(taqlcmd)           # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 print "readTimeColumn-type(parmIter): ", type(parmIter)
                 parms[iter]=parmIter
           return parms

        # return the last iteration only
        elif iteration == "Last" or iteration == "last":

           parmsDict={}       # create an empty dictionary

           # Loop over all iterations
           for iter in range(1, maxIter+1):
                 #taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=", str(iter)
                 taqlcmd="SELECT DISTINCT STARTTIME, ENDTIME, ITER, " + parameter + " FROM " + self.tablename + " WHERE ITER=", str(iter)
                 selection=pt.taql(taqlcmd)           # execute TaQL command
                 parmIter=selection.getcol(parameter)    # select column with wanted parameter
                 parmsDict[iter]=parmIter

           # Collect last iteration from dictionary entries
           parms={}
           for i in range(1, len(parmsDict)):
               parms[i]=parmsDict[i][len(parmsDict[i])]
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
        
        return timeslots


    # Return the unique STARTTIMES present in the
    # Measurementset
    #
    def getStartTimes(self):
        taqlcmd="SELECT UNIQUE STARTTIME FROM " + self.tablename
        starttimes=pt.taql(taqlcmd)

        #print "No. timeslots: ", starttimes.nrows()     # DEBUG
        return starttimes


    # Return the unique ENDTIMES present in the
    # Measurementset
    #
    def getEndTimes(self):
        taqlcmd="SELECT UNIQUE ENDTIME FROM " + self.tablename
        endtimes=pt.taql(taqlcmd)

        return endtimes        
 

    # Get a table of unique STARTTIME, ENDTIME pairs
    # which give all the time slots of the Measurementset
    # only compute them once
    #
    def getFreqs(self):
        if self.frequencies == 0:
            taqlcmd="SELECT UNIQUE STARTFREQ, ENDFREQ FROM " + self.tablename
            self.frequencies=pt.taql(taqlcmd)

            return self.frequencies
        else:
            return self.frequencies


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
