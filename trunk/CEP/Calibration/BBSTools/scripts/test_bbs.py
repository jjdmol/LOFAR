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
# File:             test_bbs.py
# Date:             2011-07-05
# Last change:      2011-07-11
# Author:           Sven Duscha (duscha@astron.nl)


import os
import sys
import shutil
import subprocess
from socket import gethostname


# LOFAR python classes for tests
import pyrap.tables as pt
import re                       # regular expressions
import lofar.parmdb as parmdb


class testBBS:

    # Create a testBBS class with MS, parset, skymodel and optional working directory
    #
    def __init__(self, MS, parset, skymodel, wd='.'):
        self.passed = False
        self.MS = MS
        self.parset = parset
        self.skymodel = skymodel
        self.test_MS = "test_" + MS 
        self.gds = ""
        self.wd = wd
        self.host = gethostname()
        self.dbserver = "ldb001"
        self.clusterdesc = self.getClusterDescription()
        self.dbuser = "postgres"
        
        self.parms = []
        self.columns = []
        self.acceptancelimit=10e-4
                
        
    
    # Show current Test settings
    #
    def show(self):
        print "Current BBS test settings"
        print "MS           = ", self.MS
        print "Parset       = ", self.parset
        print "Skymodel     = ", self.skymodel
        print "test_MS      = ", self.test_MS
        print "gds          = ", self.gds
        print "wd           = ", self.wd
        print "host         = ", self.host
        print "clusterdesc  = ", self.clusterdesc
        print "dbserver     = ", self.dbserver
        print "parms        = ", self.parms
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
        print "checkFiles()"            # DEBUG
        
        if os.path.isfile(self.parset) == False:                     # parset
            print "Fatal: parset ", self.parset, "not found."
            self.end()
        if os.path.isfile(selfskymodel) == False:                   # skymodel
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
        print bcolors.OKBLUE + "Copying orignal files." + bcolors.ENDC
        
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
            if os.path.isdir(self.test_MS/instrument) == False:
                return False
            else:
                return True
        else:
            for file in self.test_MS:
                if os.path.isdir(self.test_MS/instrument) == False:
                    return False
                else:
                    return True
                
    
    # Create a GDS, this implies creating the vds
    #
    def makeGDS(self):
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
        print bcolors.OKBLUE + "Deleting test files." + bcolors.ENDC 

        # Depending on a single MS or given a list of MS
        # copy the/or each MS file (these are directories, so use shutil.copytree)
        #
        if isinstance(self.test_MS, str):
            shutil.rmtree(self.test_MS)
        elif isinstance(self.test_MS, list):
            for file in self.test_MS:
                destname = 'test_' + file 
                shutil.rmtree(file)
        else:
            print bcolor.FAIL + "Fatal: Error MS or gds provided." + bcolors.ENDC
            self.end()
        
        
    # Display the result of the test
    #
    def printResult(self):
        if self.passed:
            print bcolors.OKGREEN + "Test " + sys.argv[0] + " passed." + bcolors.ENDC 
        else:
            print bcolors.FAIL + "Test " + sys.argv[0] + " failed." + bcolors.ENDC
            

    # End test
    #
    def end(self):
        self.printResult()
        print bcolors.OKBLUE + sys.argv[0] + " exiting." + bcolors.ENDC
        exit(0)


    # High-level function that executes the whole test procedure
    #
    def executeTest():
        print bcolors.OKBLUE + "Execute test " + bcolor.ENDC + sys.argv[0] 

        test.copyOriginalFiles()
        test.makeGDS()
     
        test.show()
        test.printResult()
        test.deleteTestFiles()        
        
            
    #############################################
    #
    # Example Tests
    #
    #############################################
    
    
    # Compare a particular MS column with the reference
    #
    def compareColumn(self, columnname):
        print "Comparing "+  bcolors.OKBLUE + columnname + bcolors.ENDC + " columns."             # DEBUG
        
        # Loop over columns, compute and check difference (must be within self.acceptancelimit)            
        # use TaQL for this? How to select from two tables?
 
 
    # Add the column from the referencetable to the testtable (needed for the TaQL comparison)
    #
    def addRefColumnToTesttab():
      print "addRefColumnToTesttab()"           # DEBUG
 
      errorcount=0                              # counter that counts rows with differying columns
      
      testtab=pt.table(self.test_MS, readonly=False)          # Open test_MS in readonly mode
      reftab=pt.table(self.MS)                  # Open reference MS in readonly mode
      
      #tc_test=testtab.col(columnname)          # get column in test table
      tc_ref=reftab.col(columnname)             # get column in reference table

      # Get column description from testtab
      testcolumnname = "test_" + columnname
      
      refcol_desc = reftab.getcoldesc(columnname)             # get the column description from the ref table
      refdmi = reftab.getdminfo(columnname)                   # also get its datamanager info
      testtab.renamecol(columnname, testcolumnname)           # rename the existing column in the test table
      testcol_desc=testtab.getcoldesc(testcolumnname)         # column description of renamed column 
#        refcol_desc=pt.makecoldesc(columnname, reftab.getcoldesc(columnname))

      testcol_desc['name']=testcolumnname
      #refcol_desc['dataManagerGroup']=testcol_desc['dataManagerGroup']
      #refcol_desc['shape']=testcol_desc['shape']
   
      testtab.addcols(pt.maketabdes(pt.makearrcoldesc(outcol, 0, valuetype='complex', 
      shape=numpy.array(t.getcell(column, 0), dminfo=refdmi)))

      testtab.putcol(tc_ref)

 
    
    # Test all parameters in parmdb that match wildcard
    # "parameter"
    #
    def compareParms(self, parameter=""):
        print "Comparing parmDB parameters in test and reference MS."         # DEBUG       

        if isinstance(self.test_MS, str):
            parmDB_test=parmdb.parmdb(self.test_MS + '/instrument')       # test_MS parmdb
            parmDB_ref=parmdb.parmdb(self.MS + '/instrument')             # (ref) MS parmdb

        if parameter=="":
            parameters=parmDB_ref.getNames()
            test_parameters=parmDB_test.getNames()
        else:
            parameters=parmDB_ref.getNames(parameter)
        
        # Test if all parameters have been solved for
        for parm in parameters:
            if parm not in parameters:
                print "compareParms() test MS is missing solved parameters"
                self.end()

        for parm in parameters:
            # Only check for parameters that were declared in the parset           
            for parsetparm in self.parms:
                if re.match(parsetparm, parm) != None:
                    testparms = parmDB_test.getValues(parm)[parm]
                    refparms = parmDB_ref.getValues(parm)[parm]
                    
                    # Compare the values and store the difference residual
                    difference=0
                    if isinstance(testparms, list):
                        for i in len(testparms):
                            difference.append(abs(testparms[i] - refparms[i]))
                            if difference > self.acceptancelimit:
                                print bcolors.FAIL + "Parameter " + parm + " differes more than " + difference
                                self.passed = False
                                self.end()
                    elif isinstance(testparms, int):
                        difference = abs(testparms - refparms)
                        if difference > self.acceptancelimit:
                            print bcolors.FAIL + "Parameter " + parm + " differes more than " + difference 
                            self.passed = False
                            self.end()                    

            self.passed = True      # set to true after successful test  
 

    # Get the parameters that were solved for from the parset
    #
    def getParmsFromParset(self):
        print bcolors.OKBLUE + "Reading parms from parset" + bcolors.ENDC

        parset_fh=open(self.parset, "r")
        lines=parset_fh.readlines()

        for line in lines:
            if line.find("Parms")!=-1:
                parts=line.split()
                key=parts[2]
                key=key.replace('[', '').replace(']','').replace('"','')
                parms=key.split(",")

        #print "parms = ", parms                 # DEBUG
        return parms

    
    # Read the output data columns, e.g CORRECTED_DATA etc. from the parset
    #
    def getColumnsFromParset(self):
        print bcolors.OKBLUE + "Reading columns from parset" + bcolors.ENDC
        
        parset_fh=open(self.parset, "r")
        lines=parset_fh.readlines()
        columns=[]

        for line in lines:
            if line.find("Output.Column")!=-1:        
                parts=line.split('=')
                column=parts[1]
                column=column.strip()               
                columns.append(column)
        return columns


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
        
        
#############################################
#
# Main function for test purposes of this class
#
#############################################        
    
def main():
    test=testBBS('L24380_SB030_uv.MS.dppp.dppp.cut', 'uv-plane-cal.parset', '3C196-bbs.skymodel')    
    
    #test.copyOriginalFiles()
    #test.makeGDS()
    test.parms=test.getParmsFromParset()
    test.columns=test.getColumnsFromParset()

    test.show()
    #test.runBBS()
    #test.compareParms()
    test.compareColumn("CORRECTED_DATA")
    
    test.printResult()
    #test.deleteTestFiles()
    

# Entry point
#
if __name__ == "__main__":
    main()
    
        