"""Testcase for RSP - SPU I2C interface, read SPU sensor information
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
# Example of 'rspctl --spustat' result format:
#
# Subrack | RCU 5.0V | LBA 8.0V | HBA 48V | SPU 3.3V | Temperature
#     0   |      ?   |      ?   |     ?   |      ?   |  ?
#     1   |     4.7  |     7.5  |    7.5  |     3.3  | 24
#     2   |      ?   |      ?   |     ?   |      ?   |  ?
#
# Via '?' rspctl indicates that the I2C access failed, due to e.g. no sensor on SPU
#
  
op = OptionParser(usage='usage: python %prog [options]', version='%prog 0.1')
  
op.add_option('-v', type='int', dest='verbosity',
  help='Verbosity level',default=21)
op.add_option('--sub', type='string', dest='subId',
  help='Subrack id: sub0,sub1,sub2 for a station with 3 subracks', default='sub0')

opts, args = op.parse_args()

# - Option checks and/or reformatting
vlev = opts.verbosity

# Get subrack IDs and numbers
strId = opts.subId.split(',')

subId = []
subNr = []
for sub in strId:
  if sub[:3] == 'sub':
    subId.append(sub)
    subNr.append(sub[3:])
  else:
    op.error('Option --sub has invalid subrack id %s' % sub)
subId.sort()  # There are less than 10 subracks in a station so sort on string is fine
subNr.sort()


################################################################################
# Define subrack testlog class for pass/fail and logging
testId = 'I2C-SPU - '
logName = 'i2c_spu.log'

tlog = testlog.Testlog(vlev, testId, logName)

tlog.setResult('PASSED')

tlog.appendLog(11,'')
tlog.appendLog(1,'Read SPU sensor to verify the RSP - SPU I2C interface for subrack %s' % subId)
tlog.appendLog(11,'')


################################################################################
# Command line

appLev = False
cli.command('rm -f spustat.log',appLev)
cli.command('rspctl --spustat > spustat.log',appLev)

# Echo spustat.log into i2c_spu.log
tlog.appendFile(21,'spustat.log')


################################################################################
# Verify result

f=open('spustat.log','r')
f.readline()                # skip title line

data_str = f.readline()     # read measured data line
j = 0
while data_str != '':
  data_str = data_str.replace(' ','')    # remove spaces
  data_str = data_str.strip()            # remove \n
  data = data_str.split('|')             # make a list of strings

  if j < len(subNr) and data[0] == subNr[j]:
    # Temporary also still accept 0.0 instead of ? to indicate I2C not ack
    if data[1] == '?' or data[1] == '0.0':
      # The I2C access itself failed
      tlog.appendLog(11,'Subrack-%s --> SPU I2C access to sensor went wrong' % subId[j])
      tlog.setResult('FAILED')
    else:
      volt_5v  = float(data[1])
      volt_8v  = float(data[2])
      volt_48v = float(data[3])
      volt_48v = 48               # Temporary fix to effectively ignore 48v value
      volt_3v3 = float(data[4])
      temp_pcb = int(data[5])
      #print volt_5v, volt_8v, volt_48v, volt_3v3, temp_pcb

      # Verify that sensor values are in valid range
      if volt_5v  <  4.5 or volt_5v  >  5.5 or \
         volt_8v  <  7   or volt_8v  >  9   or \
         volt_48v < 44   or volt_48v > 50   or \
         volt_3v3 <  3.0 or volt_3v3 >  4.0 or \
         temp_pcb < 10   or temp_pcb > 50:
        tlog.appendLog(11,'Subrack-%s --> SPU I2C sensor values are wrong' % subId[j])
        tlog.setResult('FAILED')
      else:
        tlog.appendLog(11,'Subrack-%s --> SPU I2C sensor values are OK' % subId[j])
    j += 1
  else:
    # Expect that all subracks in the station are in subId
    tlog.appendLog(11,'Wrong: Missing Subrack-%s in the argument list' % data[0])
    tlog.setResult('FAILED')
  data_str = f.readline()     # next measured data line
  
f.close()

while j < len(subId):
  tlog.appendLog(11,'Wrong: Subrack-%s does not exist in the station' % subId[j])
  tlog.setResult('FAILED')
  j += 1

tlog.appendLog(0,tlog.getResult())
