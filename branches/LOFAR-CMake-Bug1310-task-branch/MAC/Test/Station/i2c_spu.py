"""Testcase for RSP - SPU I2C interface, read SPU sensor information
"""

################################################################################
# User imports
import cli

print ''
print 'Read SPU sensor information to verify the RSP - SPU I2C interface'
print ''

################################################################################
# Command line

appLev = False
cli.command('rm -f spustat.log',appLev)
cli.command('rspctl --spustat > spustat.log',appLev)

################################################################################
# Verify result

f=open('spustat.log','r')
f.readline()                # skip title line
data_str = f.readline()     # keep measured data line
f.close()

data_str = data_str.replace(' ','')    # remove spaces
data_str = data_str.strip()            # remove \n
data = data_str.split('|')             # make a list of strings

volt_5v  = float(data[1])
volt_8v  = float(data[2])
volt_48v = float(data[3])
volt_3v3 = float(data[4])
temp_pcb = int(data[5])

print '5   V = %5.2f V' % volt_5v
print '8   V = %5.2f V' % volt_8v
print '48  V = %5.2f V' % volt_48v
print '3.3 V = %5.2f V' % volt_3v3
print 'Temp  = %d degrees C' % temp_pcb
print ''

tc_result = 1
if volt_5v  < 4.5 or volt_5v  > 5.5:
  print 'Value %f for 5 V is wrong' % volt_5v
  tc_result = 0
if volt_8v  < 7.5 or volt_8v  > 8.5:
  print 'Value %f for 8 V is wrong' % volt_8v
  tc_result = 0
#if volt_48v < 47  or volt_48v > 49:
#  print 'Value %f for 48 V is wrong' % volt_48v
#  tc_result = 0
if volt_3v3 < 3.0 or volt_3v3  > 4.0:
  print 'Value %f for 3.3 V is wrong' % volt_3v3
  tc_result = 0
if temp_pcb < 10  or temp_pcb > 40:
  print 'Value %f for PCB temperature is wrong' % temp_pcb
  tc_result = 0
  
if tc_result == 0:
  print 'RSP --> SPU I2C interface test went wrong'
else:
  print 'RSP --> SPU I2C interface test went OK'
