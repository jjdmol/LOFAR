#!/opt/local/bin/python

# Python script for a quick analysis of solver parameters recorded in a BBS solution
# Measurement Table
#
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.


#*******************************
#
# Import modules
#
#*******************************

import os
import sys
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
import pyrap.tables as pt

from PyQt4 import QtCore
from PyQt4 import QtGui         # we want to have a GUI


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


# Solver class handles reading statistics from the "solver" table logged by
# ParmDBLog into a CASA table
#
# File:          SolverQuery.py
# Author:        Sven Duscha (duscha@astron.nl)
# Date:          2010/07/16
# Last change:   2010/07/20
#
class SolverQuery:
    # Empty constructor (does not open the table immediately)
    def __init__(self):
        print "Empty constructor called"
        # do nothing

    # Default constructor, opens the table of name (default: "solver")
    #
    def __init__(self, tablename="solver"):
        try:
            self=pt.table(tablename)
        except ValueError:
            traceback.print_exc()


    # Flush and close the table of the Solver object
    def close(self):
        self.close()


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
        return tableinfo(self)


    # Read a column "colname" from the SolverTable
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned
    #
    def readColumn( self, colname ):
        column=tablecolumn(self, colname)
        return column
   

    # Build a "standard" query string by searching for a parameter
    def buildQuery( self, colname, start_time, start_freq, end_time, end_freq ):
        print "foo"        # DEBUG
        query = "Starttime >= " + start_time + " AND Endtime <= " + end_time
                + " AND StartFreq >= " + start_freq + " AND EndFreq <= " + end_freq
                + ", sortlist='FREQ, TIME'"

        print "Query = ", query       # DEBUG


    # Perform a query on the solver table
    def performQuery( self, query, colname=None):
        results = self.query(query)              # perform query on table

        if(colname != None):                     # If a colname to be selected was given...
            results = results.getcol(colname)    # select column with parameter name

        return results


    # Read a parameter of a cell (including all iterations if present)
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned (default sorting by FREQ first and then TIME)
    #
    def readParameter(self, parameter_name, start_time, start_freq, end_time, end_freq, sort_list="STARTFREQ, STARTTIME"):
        print "readParameter(StartTime, StartFreq, EndTime, EndFreq )"   # DEBUG
  
        desc=self.tabledesc()             # Get table description (dictionary)
        column_name=desc[parameter_name]  # lookup column name for the parameter asked for

        query = "StartTime >= "+ start_time + "AND EndTime <= " + end_time " AND StartFreq >= "
                + start_freq " AND EndFreq <= " + end_freq + ", sortlist=sort_list"
        selection=self.query()
        parameter=selection.getcol(parameter_name)


    # Read a parameter of a cell of a particular iteration
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned (default sorting by FREQ first and then TIME)
    #
    def readParameter(self, parameter_name, start_time, start_freq, end_time, end_freq, iteration=Last):
        print "readParameter(StartTime, StartFreq, EndTime, EndFreq, Iteration=", Iteration, " )"   # DEBUG

        query = "StartTime >= "+ start_time + "AND EndTime <= " + end_time " AND StartFreq >= "
                + start_freq " AND EndFreq <= " + end_freq 

        if Iteration==Last:    # default: return last iteration only
            query += " AND " + "ITER == " + lastIter
            query += ", sortlist=sort_list"             # apply sorting
            selection = self.query(query) 
        else:                                           # return all iterations
            query += ", sortlist=sort_list"             # apply sorting
            selection = self.query(query)               # query selection

            result = selection.getcol(parameter_name)   # select column from selection

        return result


    # Read a complete cell (including all iterations if present)
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned
    #
    def readCell(start_time, start_freq, end_time, end_freq, sort_list="STARTFREQ, STARTTIME"):
        
        # Build query string
        query = "StartTime >= "+ start_time + "AND EndTime <= " + end_time " AND StartFreq >= "
                + start_freq " AND EndFreq <= " + end_freq + ", sortlist=sort_list"

        # Use Pyrap function to return a complete row of the table
        rows = self.query(query) 


    # Read a complete cell, but only for a particular iteration
    # If the cell with these start and end values is not found,
    # the interval between start_time, start_freq and end_time
    # and end_freq is returned
    #
    def readCell(start_time, start_freq, end_time, end_freq, iter=Last):
        print "readCell(StartTime, StartFreq, EndTime, EndFreq, Iteration=", Iteration, " )"   # DEBUG
        query = "StartTime >= "+ start_time + "AND EndTime <= " + end_time " AND StartFreq >= "
                + start_freq " AND EndFreq <= " + end_freq + ", sortlist=sort_list"

        # Use Pyrap function to return a complete row of the table
        if iter==Last:                       # default: return last iteration only
            currIteration=0                  # initialize current iteration for loop 
            prevIteration=currIteration      # keep previous iteration
            while prevIteration!=Iteration:  # as long as we step on in iterations              
                print "foo"
        else:
            print "foo"



class Plot():
    def __init__(self, parent):
        QtGui.QMainWindow.__init__(self, parent)

        self.setGeometry(300, 300, 350, 300)
        self.setWindowTitle('OpenFile')

        self.textEdit = QtGui.QTextEdit()
        self.setCentralWidget(self.textEdit)
        self.statusBar()
        self.setFocus()
    
    # Create a histogram of "data"
    def createHisto( data ):
        n, bins, patches = pl.hist(data, 50, normed=1, histtype='stepfilled')
        pl.setp(patches, 'facecolor', 'g', 'alpha', 0.75) 
        return

    def save_plot(self):
        file_choices = "PNG (*.png)|*.png"
        
        path = unicode(QFileDialog.getSaveFileName(self, 
                        'Save file', '', 
                        file_choices))
        if path:
            self.canvas.print_figure(path, dpi=self.dpi)
            self.statusBar().showMessage('Saved to %s' % path, 2000)

    def createSubplot(plot):

        return

    # Create Plot on canvas
    def plot():
        ax.plot(list, "r+")    
        ax.set_title(colname)
        plt.draw()
        return

    # Plotting function, update on change of plotting parameters
    def replot():
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

    sol=Solver(tableName)  # create new solver object

    solverTable=sol.openSolverTable( tableName );

#    print "ncols = ", solverTable.ncols()   # DEBUG
#    print "nrows = ", solverTable.nrows()   # DEBUG

    colname="CHISQR"

#    column=pt.tablecolumn(solverTable, colname)
#
#   list=column[0:5000:1];
#    fig = plt.figure()
#    ax = fig.add_subplot(111)    
#    ax.plot(list, "ro")    
#    ax.set_title(colname)
#    plt.draw()

    column=pt.tablecolumn(solverTable, colname)

    size=len(column)
    list=column[0:size:1]
    fig = plt.figure()
    ax = fig.add_subplot(111)    
    ax.plot(list, "ro")    
    ax.set_title(colname)
    plt.show()

    #app = QtGui.QApplication(sys.argv)

 #   input("Press any key to exit")
    sys.exit(0)



if __name__ == "__main__":
    main()

