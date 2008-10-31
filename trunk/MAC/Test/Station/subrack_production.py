#
# Run the production tests for a LOFAR station subrack
#

import sys
from optparse import OptionParser

import cli
import testlog

################################################################################
# Parse command line for subrack ID
#
# -v  0 : only PASSED or FAILED
# -v  1 : overall title
# -v 11 : result per test
# -v 21 : title per test
  
op = OptionParser(usage='usage: python %prog [options]', version='%prog 0.1')
  
op.add_option('-v', type='int', dest='verbosity',
  help='Verbosity level',default=11)
op.add_option('-b', type='int', dest='batch_nr',
  help='Provide subrack batch number that will be used for the log file name',default=None)
op.add_option('-s', type='int', dest='serial_nr',
  help='Provide subrack serial number that will be used for the log file name',default=None)

opts, args = op.parse_args()

# - Option checks and/or reformatting
if opts.batch_nr==None:
  op.error('Option -b must specify a subrack batch number')
if opts.serial_nr==None:
  op.error('Option -s must specify a subrack serial number')


################################################################################
# Define subrack testlog class for pass/fail and logging
vlev = opts.verbosity
testId = ''
logName = 'SUBR-%05d-%05d.dat' % (opts.batch_nr, opts.serial_nr)

sr = testlog.Testlog(vlev, testId, logName)

sr.setResult('PASSED')

sr.setId('Subrack - ')
sr.appendLog(11,'')
sr.appendLog(1,'Subrack production test %s' % logName)
sr.appendLog(11,'')


################################################################################
sr.setId('RSP version - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify LCU - RSP ethernet link by getting the RSP version info')
sr.appendLog(21,'')
res = cli.command('./rsp_version.sh')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> RSP version test went OK')
else:
  sr.appendLog(11,'>>> RSP version test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'rsp_version.log')
  sr.appendLog(11,'Expected:')
  sr.appendFile(11,'gold/rsp_version.gold')  
  sr.setResult('FAILED')


################################################################################
sr.setId('TBB version - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify LCU - TBB ethernet link by getting the TBB version info')
sr.appendLog(21,'')
res = cli.command('./tbb_version.sh')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> TBB version test went OK')
else:
  sr.appendLog(11,'>>> TBB version test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'tbb_version.log')
  sr.appendLog(11,'Expected:')
  sr.appendFile(11,'gold/tbb_version.gold')
  sr.setResult('FAILED')
  

################################################################################
sr.setId('SPU status - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the RSP - SPU I2C interface by reading the SPU sensor data')
sr.appendLog(21,'')

res = cli.command('python i2c_spu.py')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> RSP - SPU I2c interface test went OK')
else:
  sr.appendLog(11,'>>> RSP - SPU I2c interface test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'spustat.log')
  sr.setResult('FAILED')


################################################################################
sr.setId('TD status - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the RSP - TD I2C interface by reading the TD sensor data')
sr.appendLog(21,'')

res = cli.command('python i2c_td.py')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> RSP - TD I2c interface test went OK')
else:
  sr.appendLog(11,'>>> RSP - TD I2c interface test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'tdstat.log')
  sr.setResult('FAILED')


################################################################################
sr.setId('RCU-RSP - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the RCU -> RSP LVDS interfaces by capturing pseudo random data on RSP')
sr.appendLog(21,'')

res = cli.command('python verify.py --brd rsp0,rsp1,rsp2,rsp3 --fpga blp0,blp1,blp2,blp3 --pol x,y --rep 1 -v 11 --te tc/prsg.py')
if res.find('FAILED')==-1:
  sr.appendLog(11,'>>> RCU-RSP interface test went OK')
  sr.appendFile(21,'tc/prsg.log')
else:
  sr.appendLog(11,'>>> RCU-RSP interface test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendFile(11,'tc/prsg.log')
  sr.setResult('FAILED')


################################################################################
sr.setId('RCU-RSP-TBB - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the RCU - RSP - TBB LVDS interfaces by capturing pseudo random data on TBB')
sr.appendLog(21,'')

#res = cli.command('./tbb_prbs_tester.sh')
#if res.find('wrong')==-1:
#  sr.appendLog(11,'>>> RCU - RSP - TBB LVDS interfaces test went OK')
#else:
#  sr.appendLog(11,'>>> RCU - RSP - TBB LVDS interfaces went wrong')
#  sr.appendLog(11,'CLI:')
#  sr.appendLog(11,res,1,1,1)
#  sr.setResult('FAILED')


################################################################################
# End of the subrack test

sr.setId('Subrack - ')
dt = sr.getRunTime()
sr.appendLog(2,'Duration: %02dm:%02ds' % (dt/60 % 60, dt % 60))
sr.appendLog(0,sr.getResult())
sr.closeLog()
