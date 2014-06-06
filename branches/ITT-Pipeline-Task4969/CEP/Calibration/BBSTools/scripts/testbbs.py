#!/usr/bin/env python
#
# Python class for End-to-end tests of BBS
#
#
#
#
# File:             testbbs.py
# Date:             2011-07-27
# Last change:      2012-02-21
# Author:           Sven Duscha (duscha@astron.nl)


import os
import sys
from lofar.bbs.testsip import *
#import lofar.bbs.testsip


# LOFAR python class for BBS tests
#
class testbbs(testsip):

    # Constructor of testbbs class which inherits from testsip baseclass
    #
    def __init__(self, MS, parset, skymodel, wd='.', verbose=True, taql=True, key='tests'):
        self.sip=testsip.__init__(self, MS, parset, wd, verbose, taql)   # call baseclass constructor
        self.skymodel = skymodel                              # BBS has in addition a skymodel
        self.key=key

    def show(self):
        self.sip.showCommon()                                 # call baseclass show() method first
        print "skymodel     = ", self.skymodel                # Then print BBS specific information
        print "dbserver     = ", self.dbserver
        print "parms        = ", self.parms

    # Read the output data columns, e.g CORRECTED_DATA etc. from the parset
    #
    def getColumnsFromParset(self):
        if self.verbose:
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
                            max=numpy.max(difference)
                            if max > self.acceptancelimit:
                                print bcolors.FAIL + "Parameter " + parm + " differs more than " + str(max) + bcolors.ENDC
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


    #calibrate -v -f --cluster-desc $CLUSTERDESC --db $DBSERVER --db-user postgres L24380_SB030_uv.MS.dppp.dppp.gds uv-plane-cal.parset 3C196-bbs_2sources.skymodel /data/scratch/duscha/
    #
    # Execute BBS calibration through the calibrate script
    #
    def runBBS(self):    
        print bcolors.OKBLUE + "Running BBS through calibrate script." + bcolors.ENDC
        arguments = '-v -f -n --clean --key ' + self.key  + ' --cluster-desc ' + self.clusterdesc + ' --db ' + self.dbserver + ' --db-user ' + self.dbuser + ' ' + self.gds + ' ' + self.parset + ' ' + self.skymodel + ' ' + self.wd
        command = ['calibrate', arguments] # '-v', '-f', '--clean', '--key bbstest', '--cluster-desc ' + self.clusterdesc, 
#        '--db ' + self.dbserver, '--db-user ' + self.dbuser, self.gds, self.parset, self.skymodel,  self.wd]
    
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
            

    # Execute the test sequence
    #
    def executeTest(self, test="all", verbose=False, taql=False):    
        if self.verbose:
            print bcolors.WARNING + "Execute test " + bcolors.ENDC + sys.argv[0] 

        self.sip.copyOriginalFiles()
        self.sip.makeGDS()
        self.parms=self.getParmsFromParset()
        self.columns=self.getColumnsFromParset()

        # How to call baseclass method?
        if self.verbose:
            self.sip.show()

        self.runBBS()
        if test=="parms" or test=="all":
            self.sip.compareParms()
        if test=="columns" or test=="all":
            print "executeTest() self.sip.taql = ", self.sip.taql     # DEBUG
            self.sip.compareColumns(self.columns, self.sip.taql)

        if self.verbose:
            self.sip.printResults(self.results)
    
        self.sip.checkResults(self.results)
        self.sip.printResult()
        self.sip.deleteTestFiles()              # Clean up 


"""
#####################################################
#
# Main function for test purposes of this class
#
#####################################################

def main():
    test=testbbs('L24380_SB030_uv.MS.dppp.dppp.cut', 'uv-plane-cal.parset', '3C196-bbs.skymodel')    
    test.executeTest()   

    

# Entry point
#
if __name__ == "__main__":
    main()
"""
