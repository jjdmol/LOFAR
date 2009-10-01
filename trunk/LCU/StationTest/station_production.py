#
# Run the production tests for a LOFAR station
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
op.add_option('-r', type='int', dest='rsp_nr',
  help='Provide number of rsp boards that will be used in this test',default=None)
op.add_option('-t', type='int', dest='tbb_nr',
  help='Provide number of tbb boards that will be used in this test',default=None)

opts, args = op.parse_args()

# - Option checks and/or reformatting
if opts.rsp_nr==None:
  op.error('Option -r must specify the number of rsp boards')
if opts.tbb_nr==None:
  op.error('Option -t must specify the number of tbb boards')

if opts.rsp_nr == 4:
        RspBrd = 'rsp0,rsp1,rsp2,rsp3'
        SubBrd = 'rsp0'
        SubRck = 'sub0'      
if opts.rsp_nr == 12:
        RspBrd = 'rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11'
        SubBrd = 'rsp0,rsp4,rsp8'
        SubRck = 'sub0,sub1,sub2'  
if opts.rsp_nr == 24:
        RspBrd = 'rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11,rsp12,rsp13,rsp14,rsp15,rsp16,rsp17,rsp18,rsp19,rsp20,rsp21,rsp22,rsp23'
        SubBrd = 'rsp0,rsp4,rsp8,rsp12,rsp16,rsp20'
        SubRck = 'sub0,sub1,sub2,sub3,sub4,sub5'

###############################################################################
# Define subrack testlog class for pass/fail and logging
vlev = opts.verbosity
testId = ''
appLev = False
logName = '/opt/stationtest/data/SUBR-%05d-%05d.dat' % (opts.rsp_nr, opts.tbb_nr)
cli.command('rm -f /opt/stationtest/data/SUBR-%05d-%05d.dat', appLev) 
sr = testlog.Testlog(vlev, testId, logName)

sr.setResult('PASSED')

sr.setId('Station - ')
sr.appendLog(11,'')
sr.appendLog(1,'Station production test %s' % logName)
sr.appendLog(11,'')

################################################################################
sr.setId('RSP version - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify LCU - RSP ethernet link by getting the RSP version info')
sr.appendLog(21,'')
res = cli.command('./rsp_version.sh')
if res.find('OK')==-1:
  sr.appendLog(11,'>>> RSP version test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'rsp_version.log')
  sr.appendLog(11,'Expected:')
  sr.appendFile(11,'gold/rsp_version.gold')  
  sr.setResult('FAILED')
else:
  sr.appendLog(11,'>>> RSP version test went OK')
  
################################################################################
sr.setId('TBB version - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify LCU - TBB ethernet link by getting the TBB version info')
sr.appendLog(21,'')
res = cli.command('./tbb_version.sh')
if res.find('OK')==-1:
  sr.appendLog(11,'>>> TBB version test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'tbb_version.log')
  sr.appendLog(11,'Expected:')
  sr.appendFile(11,'gold/tbb_version.gold')
  sr.setResult('FAILED')
else:
  sr.appendLog(11,'>>> TBB version test went OK')
  
################################################################################
sr.setId('TBB size check - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the size of the TBB memory modules')
sr.appendLog(21,'')
res = cli.command('./tbb_size.sh')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> TBB size test went OK')
else:
  sr.appendLog(11,'>>> TBB size test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'tbb_size.log')
  sr.appendLog(11,'Expected:')
  sr.appendFile(11,'gold/tbb_size.gold')
  sr.setResult('FAILED')
  
################################################################################
sr.setId('TBB memory check - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify TBB memory modules on the TBB')
sr.appendLog(21,'')
res = cli.command('./tbb_memory.sh')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> TBB memory test went OK')
else:
  sr.appendLog(11,'>>> TBB memory test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'Result:')
  sr.appendFile(11,'tbb_memory.log')
  sr.appendLog(11,'Expected:')
  sr.appendFile(11,'gold/tbb_memory.gold')
  sr.setResult('FAILED')

################################################################################
sr.setId('RCU-RSP-TBB - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the RCU - RSP - TBB LVDS interfaces by capturing pseudo random data on TBB')
sr.appendLog(21,'')
res = cli.command('./tbb_prbs_tester.sh')
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> RCU - RSP - TBB LVDS interfaces test went OK')
else:
  sr.appendLog(11,'>>> RCU - RSP - TBB LVDS interfaces went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.setResult('FAILED')
   
################################################################################
sr.setId('SPU status - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the RSP - SPU I2C interface by reading the SPU sensor data')
sr.appendLog(21,'')
#res = cli.command('python i2c_spu.py --sub %s --rep 1 -v 11' %(SubRck,))
res = cli.command('python i2c_spu.py --sub sub0,sub1,sub2')
if res.find('FAILED')==-1:
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
res = cli.command('python i2c_td.py --brd %s' %(SubBrd,))
if res.find('FAILED')==-1:
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
res = cli.command('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --pol x,y --rep 1 -v 11 --te tc/prsg.py' %(RspBrd,)) 
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> RCU-RSP interface test went OK')
  sr.appendFile(21,'tc/prsg.log')
else:
  sr.appendLog(11,'>>> RCU-RSP interface test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendFile(11,'tc/prsg.log')
  sr.setResult('FAILED')

################################################################################
sr.setId('RCU-HBA modem - ')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the control modem on the RCU')
sr.appendLog(21,'')
res = cli.command('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --rep 1 -v 11 --te tc/hba_client.py --client_access r --client_reg version --data 10' %(RspBrd,)) 
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> RCU-HBA modem test went OK')
  sr.appendFile(21,'tc/hba_client.log')
else:
  sr.appendLog(11,'>>> RCU-HBA modem went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendFile(11,'tc/hba_client.log')
  sr.setResult('FAILED')

################################################################################
sr.setId('Serdes ring -')
sr.appendLog(21,'')
sr.appendLog(21,'### Verify the Serdes ring connection between the RSP boards')
sr.appendLog(21,'')
res = cli.command('python verify.py --brd %s --rep 1 -v 21 --te tc/serdes.py --diag_sync 0' %(RspBrd,))
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> Serdes ring test went OK')
  sr.appendLog(21,'tc/serdes.log')
else:
  sr.appendLog(11,'>>> Serdes ring test went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'tc/serdes.log')
  sr.appendLog('FAILED')

################################################################################
sr.setId('Build In Self Test -')
sr.appendLog(21,'')
sr.appendLog(21,'### Build In Self Test (BIST)')
sr.appendLog(21,'')
res = cli.command('python verify.py --brd %s --rep 1 -v 21 --te tc/bist.py' %(RspBrd,))
if res.find('wrong')==-1:
  sr.appendLog(11,'>>> BIST went OK')
  sr.appendLog(21,'tc/bist.log')
else:
  sr.appendLog(11,'>>> BIST went wrong')
  sr.appendLog(11,'CLI:')
  sr.appendLog(11,res,1,1,1)
  sr.appendLog(11,'tc/bist.log')
  sr.appendLog('FAILED')  

################################################################################
# End of the subrack test

sr.setId('Subrack - ')
dt = sr.getRunTime()
sr.appendLog(2,'Duration: %02dm:%02ds' % (dt/60 % 60, dt % 60))
sr.appendLog(0,sr.getResult())
sr.closeLog()
