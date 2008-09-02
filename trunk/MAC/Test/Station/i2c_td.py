"""Testcase for RSP - TD I2C interface, read TD sensor information
"""

################################################################################
# User imports
import cli

print ''
print 'Read TD sensor information to verify the RSP - TD I2C interface'
print ''

################################################################################
# Command line

appLev = False
cli.command('rm -f tdstat.log',appLev)
cli.command('rspctl --tdstat > tdstat.log',appLev)

################################################################################
# Verify result

f=open('tdstat.log','r')
f.readline()                # skip title line
data_str = f.readline()     # keep measured data line
f.close()

data_str = data_str.replace(' ','')    # remove spaces
data_str = data_str.strip()            # remove \n
data = data_str.split('|')             # make a list of strings

volt_3v3 = float(data[6])
volt_5v  = float(data[7])
temp_pcb = int(data[8])

print '3.3 V = %5.2f V' % volt_3v3
print '5   V = %5.2f V' % volt_5v
print 'Temp  = %d degrees C' % temp_pcb
print ''

tc_result = 1
if volt_3v3 < 3.0 and volt_5v  > 4.0:
  print 'Value %f for 3.3 V is wrong' % volt_3v3
  tc_result = 0
if volt_5v  < 4.5 and volt_5v  > 5.5:
  print 'Value %f for 5 V is wrong' % volt_5v
  tc_result = 0
if temp_pcb < 10  and temp_pcb > 40:
  print 'Value %f for PCB temperature is wrong' % temp_pcb
  tc_result = 0
  
if tc_result == 0:
  print 'RSP --> TD I2C interface test went wrong'
else:
  print 'RSP --> TD I2C interface test went OK'
