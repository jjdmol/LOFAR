"""Testcase for RSP - TD I2C interface, read TD sensor information
"""

################################################################################
# User imports
import sys
from optparse import OptionParser

import cli
import testlog

################################################################################
# Parse command line for verbosity and RSP ID (typically one RSP per subrack)
#
# -v  0 : only PASSED or FAILED
# -v  1 : overall title
# -v 11 : test results
# -v 21 : detailed results
#
# Example of 'rspctl --tdstat' result format:
#
# RSP |   10MHz input | output clock | PPS input | PLL 160MHZ | PLL 200MHz | V3.3 | V5.0 | Temperature
#   0 |      SMA      |      160     |    SMA    |     LOCKED | not locked |  3.3 |  0.0 | 23
#   1 | Not controlling the TD board
#   2 | Not controlling the TD board
#   3 | Not controlling the TD board
#   4 |      SMA      |      160     |    SMA    |     LOCKED | not locked |  3.3 |  0.0 | 23
#   5 | Not controlling the TD board
#   6 | Not controlling the TD board
#   7 | Not controlling the TD board
#   8 |      SMA      |      160     |    SMA    |     LOCKED | not locked |  3.3 |  0.0 | 23
#   9 | Not controlling the TD board
#  10 | Not controlling the TD board
#  11 | Not controlling the TD board
#
# If the I2C access fails for rsp0 (e.g. due to no sensor on TD), then the rspctl will fill in:
#   0 |       ?       |       ?      |     ?     |        ?   |      ?     |   ?  |   ?  | ?
#

op = OptionParser(usage='usage: python %prog [options]', version='%prog 0.1')
  
op.add_option('-v', type='int', dest='verbosity',
  help='Verbosity level',default=21)
op.add_option('--brd', type='string', dest='brdId',
  help='Board id: rsp0,rsp4 for RSP 0 and 4 that each control a TD in a subrack', default='rsp0')

opts, args = op.parse_args()

# - Option checks and/or reformatting
vlev = opts.verbosity

# Get RSP IDs and numbers
strId = opts.brdId.split(',')

brdNr = []
for brd in strId:
  if brd[:3] == 'rsp':
    brdNr.append(int(brd[3:]))  # convert string to integer to prepare for sort
  else:
    op.error('Option --brd has invalid board id %s' % brd)
brdNr.sort()                    # sort on integers because sort on strings would give e.g. 0,12,4,8
rspId = []
rspNr = []
for brd in brdNr:
  rspId.append('rsp%d' % brd)   # back to string
  rspNr.append('%d' % brd)      # back to string


################################################################################
# Define subrack testlog class for pass/fail and logging
testId = 'I2C-TD- '
logName = 'i2c_td.log'

tlog = testlog.Testlog(vlev, testId, logName)

tlog.setResult('PASSED')

tlog.appendLog(11,'')
tlog.appendLog(1,'Read TD sensor to verify the RSP - TD I2C interface for %s' % rspId)
tlog.appendLog(11,'')


################################################################################
# Command line

appLev = False
cli.command('rm -f tdstat.log',appLev)
cli.command('rspctl --tdstat > tdstat.log',appLev)

# Echo tdstat.log into i2c_td.log
tlog.appendFile(21,'tdstat.log')


################################################################################
# Verify result

f=open('tdstat.log','r')
f.readline()                # skip title line
data_str = f.readline()     # read measured data line

j = 0
while data_str != '':
  data_str = data_str.replace(' ','')    # remove spaces
  data_str = data_str.strip()            # remove \n
  data = data_str.split('|')             # make a list of strings
  
  if j < len(rspNr) and data[0] == rspNr[j]:
    # The RSP-rspId does exist in the station
    if data[1] == 'NotcontrollingtheTDboard':
      # The RSP-rspId is expected to control a TD board
      tlog.appendLog(11,'Wrong: RSP-%s does not control a TD board' % rspId[j])
      tlog.setResult('FAILED')
    elif data[1] == '?':
      # The RSP-rspId does control a TD board but the I2C access itself failed
      tlog.appendLog(11,'RSP-%s --> TD I2C access to sensor went wrong' % rspId[j])
      tlog.setResult('FAILED')
    else:
      # The RSP-rspId does control a TD board,
      # get sensor voltages and temperature
      volt_3v3 = float(data[6])
      volt_5v  = float(data[7])
      volt_5v  = 5.0                     # Temporary fix to effectively ignore 5v value
      temp_pcb = int(data[8])
      #print volt_3v3, volt_5v, temp_pcb
      
      # Verify that sensor values are in valid range
      if volt_3v3 < 3.0 or volt_3v3 > 4.0 or \
         volt_5v  < 4.5 or volt_5v  > 5.5 or \
         temp_pcb < 10  or temp_pcb > 50:
        tlog.appendLog(11,'RSP-%s --> TD I2C sensor values are wrong' % rspId[j])
        tlog.setResult('FAILED')
      else:
        tlog.appendLog(11,'RSP-%s --> TD I2C sensor values are OK' % rspId[j])
    j += 1
  else:
    # Expect that all RSP that control a TD board are in rspId
    if data[1] != 'NotcontrollingtheTDboard':
      tlog.appendLog(11,'Wrong: Missing RSP-%s in the argument list' % data[0])
      tlog.setResult('FAILED')
  data_str = f.readline()     # next measured data line

f.close()

while j < len(rspId):
  tlog.appendLog(11,'Wrong: RSP-%s does not exist in the station' % rspId[j])
  tlog.setResult('FAILED')
  j += 1

tlog.appendLog(0,tlog.getResult())
