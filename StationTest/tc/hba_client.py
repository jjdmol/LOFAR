"""Write and readback HBA client registers, based on TCL testcase 5.42

   - Specific arguments
     . client_rcu    : RCU to use for the HBA client I2C access, x or y
     . client_access : r = read only, w = write only, wr = first write then readback
     . client_reg    : client register to access: request, response, led, vref, version, speed
     . data          : data byte(s) to write or verify read
"""

################################################################################
# - Verify options
rspId = tc.rspId
blpId = tc.blpId
repeat = tc.repeat    # When > 1 then the register access will be repeated

# - Rename testcase specific options
arg_rcu    = arg_hba_client_rcu
arg_access = arg_hba_client_access
arg_reg    = arg_hba_client_reg

# Get rcuId
rcuId = [arg_rcu]

# Adapt access format
if arg_access == 'w':
  str_access = 'Write'
elif arg_access == 'r':
  str_access = 'Read'
else:
  str_access = 'Write and read'
  arg_access = 'wr'

tc.setResult('PASSED')
  
tc.appendLog(11, '')
tc.appendLog(11, '>>> %s access of RSP-%s, BLP-%s, HBA client register %s via I2C access of RCU-%s' % (str_access, rspId, blpId, arg_reg, arg_rcu))
tc.appendLog(11, '')

################################################################################
# - Testcase initializations

# - Disable external sync
rsp.write_cr_syncoff(tc, msg, blpId, rspId)

# - Prepare the protocol list for RCU control register
addr          = rsp.c_rcuh_i2c_addr_hba       # HBA client I2C address (7 bit)
  
cmd_request   = rsp.c_hba_cmd_request
cmd_response  = rsp.c_hba_cmd_response
cmd_led       = rsp.c_hba_cmd_led
cmd_vref      = rsp.c_hba_cmd_vref
cmd_speed     = rsp.c_hba_cmd_speed
cmd_version   = rsp.c_hba_cmd_version
  
f_set_byte    = rsp.c_hba_f_set_byte
f_get_byte    = rsp.c_hba_f_get_byte
f_set_word    = rsp.c_hba_f_set_word
f_get_word    = rsp.c_hba_f_get_word
  
bc_server     = rsp.c_hba_bc_server
first_server  = 1
last_server   = 16
  
reg_xdelay    = rsp.c_hba_sreg_xdelay

# Select register
if arg_reg == 'request':
  cmd      = cmd_request
  data_len = 2*(last_server - first_server + 1)
  data     = []                 # primary data values
  data.append(bc_server)
  data.append(5 + data_len)
  data.append(f_set_word)
  data.append(reg_xdelay)
  data.append(first_server)
  data.append(last_server)
  data2    = data               # alternate data values
  for i in range(data_len):
    data.append(i)              # count up data
    data2.append(data_len - i)  # count down data
elif arg_reg == 'response':
  cmd   = cmd_response
  data  = [17, 37, 59, 214]
  data2 = [97, 231, 1, 14]
elif arg_reg == 'led':
  cmd   = cmd_led
  data  = arg_data     # HBA client LED on  > 0
  data2 = 0            # HBA client LED off = 0
elif arg_reg == 'vref':
  cmd   = cmd_vref
  data  = arg_data     # HBA client VREF setting, see pic16f87 manual
  data2 = 236          # HBA client default VREF "11101100", [5]=1 for 24 steps, [3:0]=12 for mid level
elif arg_reg == 'speed':
  cmd   = cmd_speed
  data  = arg_data     # HBA modem default speed setting is 40
  data2 = 41
elif arg_reg == 'version':
  cmd   = cmd_version
  data  = arg_data     # HBA client version write access should be ignored
  data2 = 29
else:
  tc.appendLog(11, 'Register -%s- is unknown or not supported!' % arg_reg)
  sys.exit()

# - HBA modem timing
msec     = rsp.c_msec
bc_wait  = rsp.hba_bc_wait
gap_wait = rsp.hba_gap_wait
bc_i2c   = rsp.hba_bc_i2c
reg_i2c  = rsp.hba_reg_i2c

for rep in range(1,1+repeat):
  reg_dat = data
  if repeat > 1:
    tc.appendLog(11, '>>> %s' % rep)
    if (rep % 2)==0 and arg_access != 'r':
      reg_dat = data2           # alternate between write data and write data2
      
  protocol_list = []
  exp_result    = []
  if arg_access == 'wr' or arg_access == 'w':
    protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', len(reg_dat), addr, reg_dat, cmd))
    exp_result.append(0)
  if arg_access == 'wr' and arg_reg == 'request':
    # Broadcast request takes modem time, 38 msec seems minimum for broadcast 16 servers and modem speed = 40,
    # other I2C only register accesses do not need wait.
    protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', bc_wait * msec))
    exp_result.append(0)
  else:
    # Default I2C client register access wait time
    protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', gap_wait * msec))
    exp_result.append(0)
  if arg_access == 'wr' or arg_access == 'r':
    protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT', len(reg_dat), addr, None, cmd))
    exp_result.extend(reg_dat)
    exp_result.append(0)
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_END'))
  exp_result.append(0)

  # - Overwrite and read the protocol results from the RCUH
  rsp.overwrite_rd_smbh_protocol_results(tc, msg, 'rcuh', rcuId, blpId, rspId)
   
  # - Write (and readback) the protocol list to the RCUH
  rsp.write_rd_smbh_protocol_list(tc, msg, 'rcuh', protocol_list, rcuId, blpId, rspId)

  # - Apply altsync to start the RCUH SMBus protocols
  rsp.write_rsu_altsync(tc, msg, rspId)
  tc.sleep(bc_i2c + bc_wait + reg_i2c)
  
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
    
  #tc.sleep(1000)
      
# - Enable external sync
rsp.write_cr_syncon(tc, msg, blpId, rspId)
