"""Serdes read all PHY registers, based on TCL testcase 8.3

   - Specific arguments:
     . rep     : Repeat each register read access rep times
     . data[0] : Select a group of registers or use an out of range select
                 number to print help.
"""

################################################################################
# User imports
import mmd_serdes
           
################################################################################
# Verify options
rspId = tc.rspId
repeat = tc.repeat

################################################################################
# Execution

# c_data_str[sel]:
# . 0    : Arbitrary register range
# . 1-14 : Predefined register ranges
# . >15  : Single registers that seem particularly useful for link diagnoses
# . else : Out of range selection will print help

c_data_str = {0 : 'Specify register address range via --hexdata',
              1 : 'DTE XS MDIO Manageable Device primary register space',
              2 : 'DTE XS MDIO Manageable Device redundant register space',
              3 : 'QuadPHY 10 GX Master register space',
              4 : 'Analogue Transmit and Receive register space',
              5 : 'Crossbar register space',
              6 : 'Analogue Clock synthesis unit register space',
              7 : 'REFX primary register space',
              8 : 'REFX redundant register space',
              9 : 'REFX primary lane control register space',
             10 : 'REFX redundant lane control register space',
             11 : 'TEFX primary register space',
             12 : 'TEFX redundant register space',
             13 : 'TEFX primary lane control register space',
             14 : 'TEFX redundant lane control register space',
             15 : 'Register: Primary DTE XS Status 1',
             16 : 'Register: Redundant DTE XS Status 1',
             17 : 'Register: Primary DTE XS Status 2',
             18 : 'Register: Redundant DTE XS Status 2',
             19 : 'Register: Primary DTE XS Lane Status Register',
             20 : 'Register: Redundant DTE XS Lane Status Register',
             21 : 'Register: Master Register Port Status',
             22 : 'Register: Master Primary Receive Fault Status',
             23 : 'Register: Master Redundant Receive Fault Status'}

c_rb = {0 : 0x0000,
        1 : 0x0000,
        2 : 0xC000,
        3 : 0xD000,
        4 : 0xD020,
        5 : 0xD080,
        6 : 0xD0B0,
        7 : 0xD100,
        8 : 0xD200,
        9 : 0xD140,
       10 : 0xD240,
       11 : 0xD300,
       12 : 0xD400,
       13 : 0xD320,
       14 : 0xD420,
       15 : 0x0001,
       16 : 0xC001,
       17 : 0x0008,
       18 : 0xC008,
       19 : 0x0018,
       20 : 0xC018,
       21 : 0xD006,
       22 : 0xD007,
       23 : 0xD008}

c_re = {0 : 0x0000,
        1 : 0x0019,
        2 : 0xC019,
        3 : 0xD00C,
        4 : 0xD05F,
        5 : 0xD0A7,
        6 : 0xD0B8,
        7 : 0xD119,
        8 : 0xD219,
        9 : 0xD177,
       10 : 0xD277,
       11 : 0xD305,
       12 : 0xD405,
       13 : 0xD33C,
       14 : 0xD43C,
       15 : 0x0001,
       16 : 0xC001,
       17 : 0x0008,
       18 : 0xC008,
       19 : 0x0018,
       20 : 0xC018,
       21 : 0xD006,
       22 : 0xD007,
       23 : 0xD008}
       
c_nof_data_str = 24

tc.appendLog(11, '')
tc.appendLog(11, '>>> RSP-%s, read SERDES PHY registers %d times.' % (rspId, repeat))
tc.appendLog(11, '')

for sel in arg_data:
  nof = arg_hexdata[0] 
  if sel<0 or sel >= c_nof_data_str:
    tc.appendLog(11, '')
    tc.appendLog(11, 'Usage:')
    tc.appendLog(11, '')
    tc.appendLog(11, 'Typically use this read PHY test when serdes.py fails, include in this test')
    tc.appendLog(11, 'some RSP boards that did not fail, and the RSP boards around the RSP that')
    tc.appendLog(11, 'failed. Differences between the register values for the RSP may identify the')
    tc.appendLog(11, 'problem. Please check the PMC8358 QuadPHY 10GX datasheet for the definitions.')
    tc.appendLog(11, 'The available arguments for option --data are (one or more):')
    tc.appendLog(11, '')
    for i in range(c_nof_data_str):
      tc.appendLog(11, '%2d = 0x%04X : 0x%04X  %s' % (i, c_rb[i], c_re[i], c_data_str[i]))
    tc.appendLog(11, '')
    tc.appendLog(11, 'Or type an out of range select number to print this help.')
    tc.appendLog(11, '')
  else:
    tc.appendLog(11, '%2d = %s' % (sel, c_data_str[sel]))
    rb = c_rb[sel]
    re = c_re[sel]
    if sel == 0:
      rb = arg_hexdata[0]
      re = arg_hexdata[1]
    mmd_serdes.logRange(tc, msg, rb, re, rspId, repeat)
  tc.appendLog(11, '')
