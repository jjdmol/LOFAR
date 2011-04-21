"""Testcase for Station RSP - RSP RAD interface, read RAD status information
"""

################################################################################
# User imports
import sys
from optparse import OptionParser

import cli
import testlog

################################################################################
# Parse command line for verbosity and subrack ID
#
# -v  0 : only PASSED or FAILED
# -v  1 : overall title
# -v 11 : test results
# -v 21 : detailed results
#
  
op = OptionParser(usage='usage: python %prog [options]', version='%prog 0.1')
  
op.add_option('-v', type='int', dest='verbosity',
  help='Verbosity level',default=21)
op.add_option('--rep', type='int', dest='repeat',
  help='Repeat the test', default=1)

opts, args = op.parse_args()

# - Option checks and/or reformatting
vlev = opts.verbosity
repeat = opts.repeat

################################################################################
# Define subrack testlog class for pass/fail and logging
testId = 'RSP-RAD - '
logName = 'rad_status.log'

tlog = testlog.Testlog(vlev, testId, logName)

tlog.setResult('PASSED')

tlog.appendLog(11,'')
tlog.appendLog(1,'Read RSP status information for the station RAD interface')
tlog.appendLog(11,'')


################################################################################
# Command line repeat loop

appLev = False
for rep in range(repeat):
  tlog.appendLog(11, 'Rep-%d' % rep)
  # Do rspctl --status and grep for RAD lane or ri error (any case) into one.log
  cli.command('rspctl --status | egrep \'(lane|   ri)\' | egrep [Ee][Rr][Rr][Oo][Rr] > one.log', appLev)
  # Verify result, this one.log file should be empty
  f=open('one.log','r')
  if f.readline() != '':
    # Preserve the RAD error(s) into the test log
    tlog.appendFile(21, 'one.log')
    tlog.setResult('FAILED')
    # Stop repeat loop after first error
    #f.close()
    #break
  f.close()
  tlog.sleep(950)

tlog.appendLog(0,tlog.getResult())
tlog.closeLog()
