"""Measure the HBA client line level, based on TCL testcase 5.47

   - Specific arguments
     . client_rcu    : RCU to use for the HBA client I2C access, x or y
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

tc.setResult('PASSED')
  
tc.appendLog(11, '')
tc.appendLog(11, '>>> Measure HBA line level for client at RSP-%s, BLP-%s, RCU-%s' % (rspId, blpId, arg_rcu))
tc.appendLog(11, '')


################################################################################
# - Testcase initializations

# - Use external sync to trigger the RCUH protocol list

# - Prepare the protocol list for RCU control register
addr          = rsp.c_rcuh_i2c_addr_hba       # HBA client I2C address (7 bit)

cmd_vref      = rsp.c_hba_cmd_vref
  
f_set_byte    = rsp.c_hba_f_set_byte
f_get_byte    = rsp.c_hba_f_get_byte
  
# VREF register
vref_vdd        =  3.3         # Reference supply voltage
vref_on         =  0xC0        # Reference on and output pin enable
vref_default    =  rsp.c_hba_vref_default
vref_nof_ranges =  2
vref_nof_steps  = 16
vref_range[0]   = 32
vref_offset[0]  =  8
vref_range[1]   = 24
vref_offset[1]  =  0
  
msec            = rsp.c_msec

for rep in range(1,1+repeat):

  if repeat > 1:
    tc.appendLog(11, '>>> %s' % rep)

    for sel in range(0,vref_nof_ranges):
      
      for ra in range(0,vref_nof_steps):
        
        vref_dat = vref_on | (sel << 5) | ra
      
        # - Set VREF and read comparator output (0: Vline > VREF, 1: Vline < VREF)
        protocol_list = []
        exp_result    = []
        # set VREF register
        protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', 1, addr, vref_dat, cmd_vref))
        exp_result.append(0)
        # wait a little
        protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', 10 * msec))
        exp_result.append(0)
        exp_result2 = exp_result
        # read VREF register word to get comparator status from bit 0 of the second byte 
        protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT', 2, addr, None, cmd_vref))
        exp_result.append( [vref_dat, 0, 0])  # HBA comparator level via bit 0 can be 0, or
        exp_result2.append([vref_dat, 1, 0])  # HBA comparator level via bit 0 can be 1
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
          for bi in blpId:
            for pi in rcuId:
              rd_result = smbus.read_results(tc, msg, 'rcuh', len(exp_result), pi, [bi], [ri])
        
              rd_vline[ri][bi][pi][ra] = rd_result[3]
        
              # Equal?
              if rd_result == exp_result || rd_result == exp_result2:
                tc.appendLog(21, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client VREF range %d, step %d measures : %d' % (ri, bi, pi, vref_range(sel), ra, rd_vline[ri][bi][pi][ra]))
              else:
                tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client I2C access result buffer contents (sel, ra) is wrong:' % (ri, bi, pi, sel, ra))
                tc.appendLog(11, 'Expected protocol result: %s' % exp_result)
                tc.appendLog(11, 'Expected protocol result: %s' % exp_result2)
                tc.appendLog(11, 'Read     protocol result: %s' % rd_result)
                tc.setResult('FAILED')
      
        # - Evaluate measurement result for Vline
        for ri in rspId:
          for bi in blpId:
            for pi in rcuId:
            # - Initial measurement
            vline = rd_vline[ri][bi][pi][0]
            # Comparator output 0: Vline > VREF, 1: Vline < VREF
            if vline==0:
              monotone = 0  # Vline is above lowest, next measurements may yield more accurate range
              vline_lo = vref_vdd*(vref_nof_steps-1+vref_offset(sel))/vref_range(sel)
              vline_hi = vref_vdd
            else:
              monotone = 1  # Vline is lowest, next measurements should not indicate otherwise
              vline_lo = 0
              vline_hi = $vref_vdd*(                vref_offset(sel))/vref_range(sel)
            prev_vline = vline
            # - Next measurements
            for ra in range(1,vref_nof_steps):
              vline = rd_vline[ri][bi][pi][ra]
              if vline!=prev_vline:
                monotone = monotone + 1
                if vline==1:
                  vline_lo = vref_vdd*(ra-1+vref_offset(sel))/vref_range(sel)
                  vline_hi = vref_vdd*(ra  +vref_offset(sel))/vref_range(sel)
              prev_vline = vline
            vline_lo = round(100*vline_lo)/100.0
            vline_hi = round(100*vline_hi)/100.0
            if monotone <= 1:
              tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA line voltage using VREF range %d is between %4.2 - %4.2f Volt' % (ri, bi, pi, vref_range(sel), vline_lo, vline_hi)
              if vline_hi < vref_vdd:
                tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA line voltage is too low ALARM !!!' % (ri, bi, pi)
                tc.setResult('FAILED')
            else:
              tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA line voltage using VREF range %d is not monotone.' % (ri, bi, pi, vref_range(sel))
              tc.setResult('FAILED')
  
  # - Restore default VREF
  
  protocol_list = []
  exp_result    = []
  # set VREF register
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', 1, addr, vref_default, cmd_vref))
  exp_result.append(0)
  # wait a little
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', 10 * msec))
  exp_result.append(0)
  # read back VREF register
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT', 1, addr, None, cmd_vref))
  exp_result.append( [vref_default, 0])
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
