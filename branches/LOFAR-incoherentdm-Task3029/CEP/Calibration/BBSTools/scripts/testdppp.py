#!/usr/bin/env python
#
# Python class for End-to-end tests of BBS
#
#
#
#
# File:             testbbs.py
# Date:             2011-07-27
# Last change:      2011-07-27
# Author:           Sven Duscha (duscha@astron.nl)


import os
import sys
from lofar.bbs.testsip import *


# LOFAR python class for BBS tests
#
class testdppp(testsip):

    # Constructor of testbbs class which inherits from testsip baseclass
    #
    def __init__(self, MS, parset, wd='.', verbose=True):
        testsip.__init__(self, MS, parset, wd, verbose)       # call baseclass constructor


    # Execute DPPP <parset>
    #
    def run(self):    
        print bcolors.OKBLUE + "Running DPPP "+ self.parset + bcolors.ENDC
        arguments = self.parset
        #command = ['DPPP', arguments]
        
        #print "arguments = ", arguments
        #os.popen('calibrate' + ' ' + arguments)
        
        proc = subprocess.Popen('DPPP ' + arguments, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in proc.stdout.readlines():
            print line
        ret = proc.wait()
 
        #ret=subprocess.call(command)       
        if ret==0:
            print bcolors.OKBLUE + "DPPP exited successfully." + bcolors.ENDC
        else:
            print bcolors.FAIL + "Fatal: DPPP terminated with an error." + bcolors.ENDC
            self.passed=False
            self.end()
            

    # Execute the test sequence
    #
    def executeTest(self, test="all", verbose=False, taql=False):
        if self.verbose:
            print bcolors.WARNING + "Execute test " + bcolors.ENDC + sys.argv[0] 

        self.copyOriginalFiles()
#        self.makeGDS()
#        self.parms=self.getParmsFromParset()
        self.columns=self.getColumnsFromParset()

        if self.verbose:
            self.show()

        self.run()
        taql=True
#        if test=="parms" or test=="all":
#            self.compareParms()
        if test=="columns" or test=="all":
            self.compareColumns(self.columns, taql)

        if self.verbose:
            self.printResults(self.results)
    
        self.checkResults(self.results)
        self.printResult()
        self.deleteTestFiles()              # Clean up 


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
