#!/usr/bin/env python
#
# Python class for End-to-end tests of BBS
#
# This class combines a set of file handling and result comparison
# functions into a BBS test class. This class can then be used to
# quickly write individual tests, providing a reference MS, parset
# and skymodel
#
#
#
# File:             testsip.py
# Date:             2011-07-27
# Last change:      2012-02-20
# Author:           Sven Duscha (duscha@astron.nl)


import os
import sys
import shutil
import subprocess
from socket import gethostname
import numpy


# LOFAR python classes for tests
import pyrap.tables as pt
import re                       # regular expressions
import lofar.parmdb as parmdb


class testsip:

    # Create a testBBS class with MS, parset, skymodel and optional working directory
    #
    def __init__(self, MS, parset, wd='.', verbose=True, taql=True, key="tests", clusterdesc=""):
        self.wd = wd    
        self.passed = False
        self.MS = MS
        self.parset = parset
        #self.skymodel = skymodel     # this is now part of the testbbs class
        self.test_MS = os.path.split(self.MS)[0] + "/test_" + os.path.split(self.MS)[1] 
        self.gds = ""
        self.host = gethostname()
        self.dbserver = "ldb001"
        if clusterdesc=="":
          self.clusterdesc = self.getClusterDescription()
        else:
          self.clusterdesc = clusterdesc
        self.dbuser = "postgres"
        self.key = key              # database key to be used during test run
        
        self.parms = []
        self.columns = []
        self.acceptancelimit = 1e-3   # modify this to how accurate you expect your tests to mimic the reference
        self.results = {}                
        self.verbose = verbose
        self.debug = True             # currently have to set this manually, not visible to the outside
        self.taql = taql              # use TaQL to compare columns
        
        return self
    
    # Show current Test settings
    #
    def showCommon(self):
        print "Current test settings"
        print "MS           = ", self.MS
        print "Parset       = ", self.parset
        print "test_MS      = ", self.test_MS
        print "gds          = ", self.gds
        print "wd           = ", self.wd
        print "host         = ", self.host
        print "clusterdesc  = ", self.clusterdesc
        print "columns      = ", self.columns
        print "acceptancelimit = ", self.acceptancelimit
        
    
    # Get the corresponding clusterdesc file for this host (CEP1 or CEP2)
    #
    def getClusterDescription(self):
        if self.host.find('lce') != -1:
            clusterdesc = "/globaldata/bbs/Config/full.clusterdesc"
        elif self.host.find('locus') != -1:
            clusterdesc = "/globaldata/bbs/Config/cep2.clusterdesc"            
        elif self.host.find('Sven-Duschas-Macbook-Pro') != -1:
            clusterdesc = "/Users/duscha/Cluster/Config/mbp.clusterdesc"
            self.dbserver = "localhost"         # on MBP we also have a different database server
        else:
            print "test_bbs: unknown host ", self.host, ". No corresponding clusterdesc file found."
            exit(0)
        
        return clusterdesc 
        
    
    #############################################
    #
    # File handling procedures
    #
    #############################################
        
    # Check test files
    #
    def checkFiles(self):
        if self.verbose:
            print bcolors.BLUE + "Checking test files " + bcolors.ENDC + self.MS + ", " + self.parset + ", " + self.skymodel      # DEBUG
        
        if os.path.isfile(self.parset) == False:                     # parset
            print "Fatal: parset ", self.parset, "not found."
            self.end()
        if os.path.isfile(self.skymodel) == False:                   # skymodel
            print bcolor.FAIL + "Fatal: Skymodel " + self.skymodel + "not found." + bcolor.ENDC
            self.end()
        if self.MS.find('.gds') == True:     # If MS was given as a gds
            files = self.parseGDS()
            
            for file in files:
                if os.path.isdir(file) == False:
                    print bcolor.FAIL + "Fatal: MS " + file + " not found." + bcolor.ENDC
                    self.end()
        else:
            if os.path.isdir(MS) == False:                          # MS
                print "Fatal: MS ", MS, "not found."
                exit(0)
        if os.path.isfile(self.clusterdesc) == False:
            print "Fatal: clusterdesc ", self.clusterdesc, "not found."
            self.end()


    # Copy the original test files to test_<file>, so that they remain
    # untouched; depending on the type of self.MS, it will copy a single
    # MS or loop through a list of files and copy those
    #
    def copyOriginalFiles(self):
        if self.verbose:
            print bcolors.OKBLUE + "Copying orignal files." + bcolors.ENDC       
            print "self.MS = ", self.MS                   # DEBUG
            print "self.test_MS = ", self.test_MS         # DEBUG
        
        # Depending on a single MS or given a list of MS
        # copy the/or each MS file (these are directories, so use shutil.copytree)
        #
        if isinstance(self.test_MS, str):
            shutil.copytree(self.MS, self.test_MS)
        elif isinstance(self.test_MS, list):
            for file in self.test_MS:
                destname = 'test_' + file 
                shutil.copytree(file, destname)
        else:
            print bcolor.FAIL + "Fatal: No MS or gds provided." + bcolor.ENDC
            self.end()
                
     
    # Test if instrument table is present
    #
    def testForInstrumentTable(self):
        if isinstance(self.test_MS, str):
            if os.path.isdir(self.test_MS + "/instrument") == False:
                return False
            else:
                return True
        else:
            for file in self.test_MS:
                if os.path.isdir(self.test_MS + "/instrument") == False:
                    return False
                else:
                    return True
                
    
    # Create a GDS, this implies creating the vds
    #
    def makeGDS(self):
        if self.verbose:
            print bcolors.OKBLUE + "Creating GDS file " + self.gds + bcolors.ENDC               # DEBUG
        
        ret = 0
        vdslist = []        # list of vds files created from self.test_MS
        
        # First makevds, either individual or loop through list
        if isinstance(self.test_MS, str):
            arguments = self.clusterdesc + ' ' + self.test_MS
                
            os.popen('makevds' + ' ' + arguments)
            #ret=subprocess.call(['makevds', arguments])            
            if ret!=0:
                print bcolors.WARNING + "Warning: makevds failed for " + self.test_MS + bcolors.ENDC
            else:
                vdslist.append(self.test_MS + '.vds')
        elif insinstance(self.test_MS, list):
            for file in self.test_MS:
                arguments = self.clusterdesc + ' ' + file
                
                os.popen('makevds ' + arguments) 
                #ret=subprocess.call(['makevds', arguments])
                if ret!=0:
                    print bcolors.WARNING + "Warning: makevds failed for " + self.test_M + bcolors.ENDC
                else:                
                    vdslist.append(file + '.vds')
        else:
            print bcolors.FATAL + "Fatal: MS filename is invalid." + bcolors.ENDC

        # Now create the gds
        self.gds=self.test_MS + ".gds"
        arguments = self.gds
        for vds in vdslist:
            arguments = arguments + ' ' + vds
         
        os.popen('combinevds ' + arguments)
        #subprocess.call(['combinevds', arguments])               # Combinevds
    
    
    
    # Parse a GDS file to get a list of the constituent files
    # TODO: test, and handling of gds as test input
    #
    def parseGDS(self):
        if self.verbose:
            print "parseGDS()"                  # DEBUG
    
        gds_fh = open(self.gds , "r")
        lines = gds_fd.readlines()   
    
        parts=[]                            # Parts of MS
    
        for line in lines:
            if line.find('Filename'):       # we are looking for the Filename of the MSs
                splits = line.split('=')
                parts.append(splits[1])
        return parts
    
    
    #calibrate -v -f --cluster-desc $CLUSTERDESC --db $DBSERVER --db-user postgres L24380_SB030_uv.MS.dppp.dppp.gds uv-plane-cal.parset 3C196-bbs_2sources.skymodel /data/scratch/duscha/
    #
    # Execute BBS calibration through the calibrate script
    #
    def runBBS(self):    
        print bcolors.OKBLUE + "Running BBS through calibrate script." + bcolors.ENDC
        arguments = '-v -f -n --clean --key bbstest --cluster-desc ' + self.clusterdesc + ' --db ' + self.dbserver + ' --db-user ' + self.dbuser + ' ' + self.gds + ' ' + self.parset + ' ' + self.skymodel + ' ' + self.wd
        command = ['calibrate', arguments] # '-v', '-f', '--clean', '--key bbstest', '--cluster-desc ' + self.clusterdesc, 
#        '--db ' + self.dbserver, '--db-user ' + self.dbuser, self.gds, self.parset, self.skymodel,  self.wd]
    
        #print "arguments = ", arguments
        #os.popen('calibrate' + ' ' + arguments)
        
        proc = subprocess.Popen('calibrate ' + arguments, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in proc.stdout.readlines():
            print line
        ret = proc.wait()
 
        #ret=subprocess.call(command)       
        if ret==0:
            print bcolors.OKBLUE + "BBS calibration exited successfully." + bcolors.ENDC
        else:
            print bcolors.FAIL + "Fatal: BBS terminated with an error." + bcolors.ENDC
            self.passed=False
            self.end()
    
    
    # Delete the temporary test_<filename> test files
    # (practically only the test_<MS>
    #
    def deleteTestFiles(self):
        if self.verbose:
            print bcolors.OKBLUE + "Deleting test files." + bcolors.ENDC 

        # Depending on a single MS or given a list of MS
        # copy the/or each MS file (these are directories, so use shutil.copytree)
        #
        if isinstance(self.test_MS, str):
            shutil.rmtree(self.test_MS)
            os.remove(self.test_MS + ".vds")
        elif isinstance(self.test_MS, list):
            for file in self.test_MS:
                shutil.rmtree(file)
                os.remove(file + ".vds")
        else:
            print bcolor.FAIL + "Fatal: Error MS or gds provided." + bcolors.ENDC
            self.end()

        os.remove(self.test_MS + ".gds")        # Delete test_<>_.gds file


    #################################################
    #
    # Result presentation functions
    #
    #################################################


    # Display summary of results dictionary
    #
    def printResults(self, results):
        print bcolors.OKBLUE + "Detailed test results:" + bcolors.ENDC           
        
        keys=results.keys()                 # get keys of dictionary
        for key in keys:
            if results[key]==True:
                print bcolors.OKGREEN + "Test " + bcolors.WARNING + key + bcolors.OKGREEN + " passed." + bcolors.ENDC 
            else:
                print bcolors.FAIL + "Test " + bcolors.WARNING + key + bcolors.FAIL + " failed." + bcolors.ENDC            

        
    # Display the result of the overall test
    #
    def printResult(self):
        if self.passed:
            print bcolors.OKGREEN + "Test " + sys.argv[0] + " passed." + bcolors.ENDC 
        else:
            print bcolors.FAIL + "Test " + sys.argv[0] + " failed." + bcolors.ENDC

    
    # Check individual results and set overall self.passed to True or False accordingly
    #
    def checkResults(self, results):
        keys=results.keys()                 # get keys of dictionary
        for key in keys:
            if results[key]==True:            
                self.passed=True
            else:
                self.passed=False
                return

    # End test
    #
    def end(self):
        self.printResult()
        print bcolors.OKBLUE + sys.argv[0] + " exiting." + bcolors.ENDC
        exit(0)


    #############################################
    #
    # Example Tests
    #
    #############################################

    #################################################
    #
    # Column test functions
    #
    #################################################
    
    # Compare a list of columns individually with the reference MS
    # If taql=True then use TaQL to compare the columns, otherwise use plain numpy
    #
    def compareColumns(self, columnnames="all", taql=False):
        #print "compareColumns()"                    # DEBUG
        
        if columnnames=="all":
            columnnames=self.columns
    
        for column in columnnames:
            ret = self.compareColumn(column, taql) # need, taql           
            self.results[column] = ret
            
    
    # Compare a particular MS column with the reference
    # If taql=True then use TaQL to compare the columns, otherwise use plain numpy
    #
    def compareColumn(self, columnname, taql=False):
        if self.verbose:
            print "Comparing "+  bcolors.OKBLUE + columnname + bcolors.ENDC + " columns." # DEBUG

        passed=False
        errorcount=0                                # counter that counts rows with differying columns

        if taql==False:                             # If taql is not to be used for comparison, use numpy difference
          if self.debug:
            print "compareColumn() using numpy" 

          reftab=pt.table(self.MS)                # Open reference MS in readonly mode
          testtab=pt.table(self.test_MS)          # Open test MS in readonly mode     
     
          tc_ref=reftab.col(columnname)           # get column in reference table as numpy array
          tc_test=testtab.col(columnname)         # get column in test table as numpy array
  
          nrows=testtab.nrows()                  
          for i in progressbar( range(0, nrows-1), "comparing " + columnname + " ", 60):
              difference = numpy.max(abs(tc_test[i] - tc_ref[i]))    # Use numpy's ability to substract arrays from each other
              #sum=numpy.sum(difference)
              
              #if sum > (self.acceptancelimit/len(difference)):     # determine if this failed the test
              if difference > self.acceptancelimit:                 # determine if this failed the test
                  passed=False
              else:
                  passed=True
  
          reftab.close()
          testtab.close()
        else:
            if self.debug:
              print "compareColumn() using TaQL"          # DEBUG
  
            self.addRefColumnToTesttab(columnname)      # add reference table column as forward column
        
            testcolumnname = "test_" + columnname       # create name which is used in test_MS if refcolum was added
        
            # Loop over columns, compute and check difference (must be within self.acceptancelimit)            
            # use TaQL for this? How to select from two tables? TODO: check this!
            
#            taqlcmd = "SELECT * FROM '" + self.test_MS + "' WHERE !all(NEAR(Real("+columnname+"), Real("+testcolumnname+")) AND NEAR(Imag("+columnname+"), Imag("+testcolumnname+")))"
#            errorcount = result.nrows()
            taqlcmd = "SELECT * FROM '" + self.test_MS + "' WHERE !all(NEARABS(Real("+columnname+"), Real("+testcolumnname+")," + str(self.acceptancelimit) + ") AND NEARABS(Imag("+columnname+"), Imag("+testcolumnname+"),"+ str(self.acceptancelimit) +"))"
#            print "taqlcmd = ", taqlcmd     # DEBUG
            errorcount=pt.taql(taqlcmd).nrows()            
            
            if self.verbose or self.debug:
              print "errorcount = ", errorcount         # display number of errors=No. of rows

            # If test_MS COLUMN and reference COLUMN have any discrepancy...            
            if errorcount > 0:
                passed=False      # ... the test is failed
            else:
                passed=True
        return passed

 
    # Add the column from the referencetable to the testtable (needed for the TaQL comparison)
    #
    def addRefColumnToTesttab(self, columnname):
        if self.verbose:
            print bcolors.OKBLUE + "Forwarding reference column " + bcolors.WARNING + columnname + bcolors.OKBLUE + " to " + self.test_MS + bcolors.ENDC           # DEBUG
           
        testtab=pt.table(self.test_MS, readonly=False)          # Open test_MS in readonly mode
        reftab=pt.table(self.MS)                  # Open reference MS in readonly mode
  
        # Get column description from testtab
        testcolumnname = "test_" + columnname
        
        testtab.renamecol(columnname, testcolumnname)           # rename the existing column in the test table
        refcol_desc=reftab.getcoldesc(columnname)  

        # Use ForwardColumnEngine to refer column in testtab to reftab columnname
        testtab.addcols(pt.maketabdesc([pt.makecoldesc(columnname, refcol_desc)]), dminfo={'1':{'TYPE':'ForwardColumnEngine', 'NAME':'ForwardData', 'COLUMNS':[columnname], 'SPEC':{'FORWARDTABLE':reftab.name()}}})

        testtab.flush()
 
    """
    #################################################
    #
    # ParmDB test functions
    #
    #################################################
 
    # Test all parameters in parmdb that match wildcard "parameter"
    #
    def compareParms(self, parameter=""):
        if self.verbose:
            print "Comparing " + bcolors.OKBLUE + "parmDB parameters " + bcolors.ENDC + "in test MS " + bcolors.WARNING + self.test_MS + bcolors.ENDC + " and reference MS " + bcolors.WARNING + self.MS + bcolors.ENDC         # DEBUG       

        if isinstance(self.test_MS, str):
            parmDB_test=parmdb.parmdb(self.test_MS + '/instrument')       # test_MS parmdb
            parmDB_ref=parmdb.parmdb(self.MS + '/instrument')             # (ref) MS parmdb

        if self.testForInstrumentTable() == False:              # check for correct instrument tables in MS and test_MS
            self.passed=False
            self.end()
            
        if parameter=="":
            parameters=parmDB_ref.getNames()
            test_parameters=parmDB_test.getNames()
        else:
            parameters=parmDB_ref.getNames(parameter)
        
        if len(self.parms)==0 or self.parms==None:      # if we don't have any parms, e.g only PREDICT in parset
            self.passed=True
            self.results["ParmDB"] = self.passed
            return                                      # just return

        # Test if all parameters have been solved for            
        for parm in parameters:
            if parm not in parameters:
                print "compareParms() test MS is missing solved parameters"
                self.end()

        for parm in progressbar(parameters, "Comparing parameters ", 40):
            # Only check for parameters that were declared in the parset           

            for parsetparm in self.parms:
                if re.match(parsetparm, parm) != None:
                    testparms = parmDB_test.getValues(parm)[parm]
                    refparms = parmDB_ref.getValues(parm)[parm]
                    
                    # Compare the values and store the difference residual
                    difference=[]
                    
                    if isinstance(testparms['values'], list) or isinstance(testparms['values'], numpy.ndarray):
#                        print "len(testparms['values']) = ", len(testparms['values'])   # DEBUG
#                        print "len(refparms['values']) = ", len(refparms['values'])   # DEBUG
                    
                        for i in range(0, len(testparms['values'])):
                            difference.append(abs(testparms['values'][i] - refparms['values'][i]))
                            
                            if sum(difference) > self.acceptancelimit/len(difference):
                                print bcolors.FAIL + "Parameter " + parm + " differs more than " + str(difference) + bcolors.ENDC
                                self.passed = False
                                self.end()
                            else:
                                self.passed = True      # set to true after successful test
                    elif isinstance(testparms['values'], int):
                        difference = abs(testparms - refparms)
                        
                        if difference > self.acceptancelimit:
                            print bcolors.FAIL + "Parameter " + parm + " differes more than " + difference 
                            self.passed = False
                            self.end()                    
                        else:
                            self.passed = True      # set to true after successful test

        self.results["ParmDB"] = self.passed


    # Get the parameters that were solved for from the parset
    #
    def getParmsFromParset(self):
        print bcolors.OKBLUE + "Reading parms from parset" + bcolors.ENDC

        parset_fh=open(self.parset, "r")
        lines=parset_fh.readlines()
        parms=[]
        for line in lines:
            if line.find("Parms")!=-1:
                parts=line.split()
                key=parts[2]
                key=key.replace('[', '').replace(']','').replace('"','')
                parms=key.split(",")

        return parms
    #"""
   
    # Read the output data columns, e.g CORRECTED_DATA etc. from the parset
    #
    def getColumnsFromParset(self):
        if self.verbose:
            print bcolors.OKBLUE + "Reading columns from parset" + bcolors.ENDC
        
        parset_fh=open(self.parset, "r")
        lines=parset_fh.readlines()
        columns=[]
    
        for line in lines:
            if line.find("Output.Column") and line.find("msout.datacolumn") !=-1:         # DPPP and BBS regular expression
            #if line.find("Output.Column")!=-1:         # BBS regular expression
                parts=line.split("=")              
                parts=parts[1].split('#')                     # need to remove eventual comments
                column=parts[0]          
                column=column.strip()
                if column != '':
                  columns.append(column)
  
        return columns

    # Cleanup logfiles generated by calibrate script
    #
    def cleanUpLogs(self):
      if self.debug:
        print "cleanUpLogs()"      # DEBUG

      logfiles=self.key + "*log*"
      os.remove(logfiles)

    """
  
    ##################################################################
    #
    # High-level function that executes the whole test procedure
    #
    ##################################################################
  
    def executeTest(self, test="all", verbose=False, taql=False):
        if self.verbose:
            print bcolors.WARNING + "Execute test " + bcolors.ENDC + sys.argv[0] 

        self.copyOriginalFiles()
        self.makeGDS()
        self.parms=self.getParmsFromParset()
        self.columns=self.getColumnsFromParset()

        if self.verbose:
            self.show()

        self.runBBS()
        taql=True
        if test=="parms" or test=="all":
            self.compareParms()
        if test=="columns" or test=="all":
            self.compareColumns(self.columns, taql)

        if self.verbose:
            self.printResults(self.results)
    
        self.checkResults(self.results)
        self.printResult()
        self.deleteTestFiles()              # Clean up       
    """    

#############################################
#
# Helper class for color terminal printing
#
#############################################

        
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''


#**************************************************************
#    
# Textinterface progress bar
#
#**************************************************************
#
def progressbar(it, prefix = "", size = 60):
    if len(it)!=0:
        count = len(it)
    else:
        count = 1
    def _show(_i):
        x = int(size*_i/count)
        sys.stdout.write("%s[%s%s] %i/%i\r" % (prefix, "#"*x, "."*(size-x), _i, count))
        sys.stdout.flush()
    
    _show(0)
    for i, item in enumerate(it):
        yield item
        _show(i+1)
    sys.stdout.write("\n")
    sys.stdout.flush()
        
        
#####################################################
#
# Main function for test purposes of this class
#
#####################################################        

"""
def main():
    test=testbbs('L24380_SB030_uv.MS.dppp.dppp.cut', 'uv-plane-cal.parset', '3C196-bbs.skymodel')    
    test.getColumns()
    test.executeTest()   

    

# Entry point
#
if __name__ == "__main__":
    main()
""" 
