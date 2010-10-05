"""Measure the HBA client line level, based on TCL testcase 5.47

  The client uses the PIC16F87. The modem DC line level is measured by varying
  the comparator VREF reference level. The comparator has 2 ranges for VREF:
  
  . range 0 with 32 steps: VDD * (8 + (0..15))/32, so max 3.3V * 0.719 = 2.37 V
  . range 1 with 24 steps: VDD *      (0..15) /24, so max 3.3V * 0.625 = 2.06 V
  
  The modem DC line level should be close to VDD = 3.3V, therefore any
  measurement whereby the comparator measures a level less than VREF causes
  a too low alarm message. This test can only reveal whether the line level
  is above 2.37V, but not how close it is actually to 3.3V.
  
  The default VREF setting is c_hba_vref_default = 0xEC yielding a reference
  level of VDD * (0xC)/24 = 0.5 VDD.
  
  This test case puts the RCUs in HBA mode, to ensure that they are powered on.
  
  - Usage for single measurement per client modem line:
   
    python verify.py --brd rsp0 --fp blp0  -v 11 --data 0,1 --te tc/hba_line_level.py 
     
  - Specific arguments
    . client_rcu    : RCU to use for the HBA client I2C access, x or y
    . data          : [0] Specify what VREF range to use:
                          . 0    = range 32
                          . 1    = range 24
                          . else = both
                      [1] Specify how many steps to use:
                          . 1..16
                          . else default to only the last 1 step
"""

################################################################################
# - Verify options
rspId = tc.rspId
blpId = tc.blpId
repeat = tc.repeat    # When > 1 then the register access will be repeated

# - Rename testcase specific options
arg_rcu = arg_hba_client_rcu

# Get rcuId
rcuId = [arg_rcu]

# Get VREF range and number of steps
vref_use_range = []
if len(arg_data)==1 or len(arg_data)==2:
  if arg_data[0]!=0 and arg_data[0]!=1:
    vref_use_range.append(0)            # use both range 32
    vref_use_range.append(1)            #      and range 24
  else:
    vref_use_range.append(arg_data[0])  # use specified range
else:
  vref_use_range.append(0)              # default use only range 32

vref_init_step = 16
if len(arg_data)==2:
  if arg_data[1]>=1 and arg_data[1]<=16:
    vref_init_step = 16-arg_data[1]     # use specified number of steps from vref_init_step to vref_nof_steps-1
  else:
    vref_init_step = 16-1               # default use only last step
else:
  vref_init_step = 16-1                 # default use only last step
  

tc.setResult('PASSED')

tc.appendLog(11, '')
tc.appendLog(11, '>>> Measure HBA line level for client at RSP-%s, BLP-%s, RCU-%s' % (rspId, blpId, arg_rcu))
tc.appendLog(11, '    . Using VREF range %s' % vref_use_range)
if vref_init_step==16-1:
  tc.appendLog(11, '    . Using VREF last step 15')
else:
  tc.appendLog(11, '    . Using VREF steps %s to last step 15' % vref_init_step)
tc.appendLog(11, '')


################################################################################
# - Testcase initializations

# - Set RCU in HBA mode to apply the 48V power and enable the data path
rsp.rspctl(tc, '--rcumode=5')
rsp.rspctl(tc, '--rcuenable=1')
tc.sleep(2010)

# - Use external sync to trigger the RCUH protocol list

# - Prepare the protocol list for RCU control register
addr          = rsp.c_rcuh_i2c_addr_hba       # HBA client I2C address (7 bit)

cmd_vref      = rsp.c_hba_cmd_vref

f_set_byte    = rsp.c_hba_f_set_byte
f_get_byte    = rsp.c_hba_f_get_byte

# VREF register
vref_vdd        =  3.3         # Reference supply voltage

vref_on         =  0xC0        # Reference on and output pin enable
#vref_on         =  0x80        # Reference on and output pin disable

vref_default    =  rsp.c_hba_vref_default
vref_nof_ranges =  2
vref_nof_steps  = 16
vref_range      = range(2)
vref_offset     = range(2)
vref_range[0]   = 32
vref_offset[0]  =  8
vref_range[1]   = 24
vref_offset[1]  =  0

msec            = rsp.c_msec

# - Declare Vline result matrix (using natural indices instead of named indices like with TCL)
#   . In TCL one can use names like rsp0, rsp1 and x, y as array indices, I do
#     not know how to do this in Python e.g. using dictionary. Therefore use
#     this declared multi-dimensional matrix with natural range (integers >= 0)
#     for the array indices.
rd_vline = range(len(rspId))
for ri in rspId:
  rn = rspId.index(ri)
  rd_vline[rn] = range(len(blpId))
  for bi in blpId:
    bn = blpId.index(bi)
    rd_vline[rn][bn] = range(len(rcuId))
    for pi in rcuId:
      pn = rcuId.index(pi)
      rd_vline[rn][bn][pn] = range(vref_init_step,vref_nof_steps)

for rep in range(1,1+repeat):
  if repeat > 1:
    tc.appendLog(11, '>>> %s' % rep)

  for sel in vref_use_range:

    # - Measure Vline result matrix
    for ra in range(vref_init_step,vref_nof_steps):

      vref_dat = vref_on | (sel << 5) | ra

      # - Set VREF and read comparator output (0: Vline > VREF, 1: Vline < VREF)
      protocol_list = []
      exp_result    = []
      # set VREF register
      protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', 1, addr, [vref_dat], cmd_vref))
      exp_result.append(0)
      # wait a little
      protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', 10 * msec))
      exp_result.append(0)
      exp_result2 = exp_result[:]  # must use [:] to copy the list into a new list
      # read VREF register word to get comparator status from bit 0 of the second byte
      protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT', 2, addr, None, cmd_vref))
      exp_result.append(vref_dat)
      exp_result2.append(vref_dat)
      exp_result.append(0)         # HBA comparator level via bit 0 can be 0, or
      exp_result2.append(1)        # HBA comparator level via bit 0 can be 1
      exp_result.append(0)
      exp_result2.append(0)
      protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_END'))
      exp_result.append(0)
      exp_result2.append(0)

      # - Overwrite and read the protocol results from the RCUH
      rsp.overwrite_rd_smbh_protocol_results(tc, msg, 'rcuh', rcuId, blpId, rspId)

      # - Write (and readback) the protocol list to the RCUH
      rsp.write_rd_smbh_protocol_list(tc, msg, 'rcuh', protocol_list, rcuId, blpId, rspId)

      # External sync will start the RCUH SMBus protocols
      tc.sleep(2010)

      # Read the protocol results from the RCUH
      for ri in rspId:
        rn = rspId.index(ri)
        for bi in blpId:
          bn = blpId.index(bi)
          for pi in rcuId:
            pn = rcuId.index(pi)
            rd_result = smbus.read_results(tc, msg, 'rcuh', len(exp_result), pi, [bi], [ri])

            rd_vline[rn][bn][pn][ra-vref_init_step] = rd_result[3]

            # Equal?
            if rd_result == exp_result or rd_result == exp_result2:
              tc.appendLog(21, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client VREF range %d, step %d measures : %d' % (ri, bi, pi, vref_range[sel], ra, rd_vline[rn][bn][pn][ra-vref_init_step]))
            else:
              tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client I2C access result buffer contents (%d, %d) is wrong:' % (ri, bi, pi, sel, ra))
              tc.appendLog(11, 'Expected protocol result: %s' % exp_result)
              tc.appendLog(11, 'Expected protocol result: %s' % exp_result2)
              tc.appendLog(11, 'Read     protocol result: %s' % rd_result)
              tc.setResult('FAILED')

    # - Evaluate measurement result for Vline
    for ri in rspId:
      rn = rspId.index(ri)
      for bi in blpId:
        bn = blpId.index(bi)
        for pi in rcuId:
          pn = rcuId.index(pi)
          # - Initial measurement
          vline = rd_vline[rn][bn][pn][0]
          # Comparator output 0: Vline > VREF, 1: Vline < VREF
          if vline==0:
            monotone = 0  # Vline is above lowest, next measurements may yield more accurate range
            vline_lo = vref_vdd*(vref_nof_steps-1+vref_offset[sel])/vref_range[sel]
            vline_hi = vref_vdd
          else:
            monotone = 1  # Vline is lowest, next measurements should not indicate otherwise
            vline_lo = 0
            vline_hi = vref_vdd*(                 vref_offset[sel])/vref_range[sel]
          prev_vline = vline
          # - Next measurements
          for ra in range(vref_init_step+1,vref_nof_steps):
            vline = rd_vline[rn][bn][pn][ra-vref_init_step]
            if vline!=prev_vline:
              monotone = monotone + 1
              if vline==1:
                vline_lo = vref_vdd*(ra-1+vref_offset[sel])/vref_range[sel]
                vline_hi = vref_vdd*(ra  +vref_offset[sel])/vref_range[sel]
            prev_vline = vline
          vline_lo = round(100*vline_lo)/100.0
          vline_hi = round(100*vline_hi)/100.0
          if monotone <= 1:
            tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA line voltage using VREF range %d is between %4.2f - %4.2f Volt' % (ri, bi, pi, vref_range[sel], vline_lo, vline_hi))
            if vline_hi < vref_vdd:
              tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA line voltage is too low ALARM !!!' % (ri, bi, pi))
              tc.setResult('FAILED')
          else:
            tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA line voltage using VREF range %d is not monotone.' % (ri, bi, pi, vref_range[sel]))
            tc.setResult('FAILED')
            
    # - Print Vline result matrix
    tc.appendLog(11, 'HBA client VREF range %d:' % vref_range[sel])
    tc.appendLog(11, '  RSP  AP  RCU  : VREF step comparator measurements')
    for ri in rspId:
      rn = rspId.index(ri)
      for bi in blpId:
        bn = blpId.index(bi)
        for pi in rcuId:
          pn = rcuId.index(pi)
          tc.appendLog(11, '  %s %s %s   : %s' % (ri, bi, pi, rd_vline[rn][bn][pn][0:vref_nof_steps-vref_init_step]))


# - Restore default VREF
tc.appendLog(11, '')
tc.appendLog(11, '>>> Restore default VREF = 0x%X for RSP-%s, BLP-%s, RCU-%s' % (vref_default, rspId, blpId, rcuId))
tc.appendLog(11, '')
protocol_list = []
exp_result    = []
# set VREF register
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', 1, addr, [vref_default], cmd_vref))
exp_result.append(0)
# wait a little
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', 10 * msec))
exp_result.append(0)
# read back VREF register
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT', 1, addr, None, cmd_vref))
exp_result.append(vref_default)
exp_result.append(0)
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_END'))
exp_result.append(0)

# - Overwrite and read the protocol results from the RCUH
rsp.overwrite_rd_smbh_protocol_results(tc, msg, 'rcuh', rcuId, blpId, rspId)

# - Write (and readback) the protocol list to the RCUH
rsp.write_rd_smbh_protocol_list(tc, msg, 'rcuh', protocol_list, rcuId, blpId, rspId)

# External sync will start the RCUH SMBus protocols
tc.sleep(2010)

# Read the protocol results from the RCUH
for ri in rspId:
  for bi in blpId:
    for pi in rcuId:
      rd_result = smbus.read_results(tc, msg, 'rcuh', len(exp_result), pi, [bi], [ri])

      # Equal?
      if rd_result == exp_result:
        tc.appendLog(21, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client I2C access result buffer contents is OK' % (ri, bi, pi))
      else:
        tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client I2C access result buffer contents is wrong:' % (ri, bi, pi))
        tc.appendLog(11, 'Expected protocol result: %s' % exp_result)
        tc.appendLog(11, 'Read     protocol result: %s' % rd_result)
        tc.setResult('FAILED')
