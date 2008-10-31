"""HBA server access via modem, based on TCL testcase 5.43

  - Specific arguments:
     . client_rcu    : RCU to use for the HBA client I2C access, x or y
     . server        : first server number and last server number: first,last
     . server_access : bc = broadcast to all servers, uc = unicast to first server
     . server_func   : server function: gb, gw, sb, sw
     . server_reg    : server register: delay_x, delay_y, version, address
     . data          : data byte(s) to write or verify for each server
     . count         : use counter data for each write request (instead of data)
     . rand          : use random data for each write request (instead of data)
"""

################################################################################
# - Verify options
rspId = tc.rspId
blpId = tc.blpId
repeat = tc.repeat    # When > 1 then the register access will be repeated

# - Rename testcase specific options
arg_rcu    = arg_hba_client_rcu
arg_server = arg_hba_server
arg_access = arg_hba_server_access
arg_func   = arg_hba_server_function
arg_reg    = arg_hba_server_reg

# Get rcuId
rcuId = [arg_rcu]

# Adapt access format
if arg_access == 'bc':
  str_access = 'broadcast'
else:
  str_access = 'unicast'
  arg_access = 'uc'

# Get server addresses
bc_server = rsp.c_hba_bc_server
if arg_access == 'bc':
  first_server = arg_server[0]
  last_server  = arg_server[1]
else:
  first_server = arg_server[0]
  last_server  = first_server
if first_server > last_server:
  tc.appendLog(11, 'First server %d must be <= last server %d!' % (first_server, last_server))
  sys.exit()
elif last_server - first_server + 1 > 16:
  tc.appendLog(11, 'Maximum first server %d to %d range is 16!' % (first_server, last_server))
  sys.exit()
elif first_server < 1 or first_server > 127:
  tc.appendLog(11, 'First server %d must be in range 1 to 127!' % first_server)
  sys.exit()
elif last_server < 1 or last_server > 127:
  tc.appendLog(11, 'Last server %d must be in range 1 to 127!' % last_server)
  sys.exit()
  
# Get function
f_set_byte = rsp.c_hba_f_set_byte
f_get_byte = rsp.c_hba_f_get_byte
f_set_word = rsp.c_hba_f_set_word
f_get_word = rsp.c_hba_f_get_word
if arg_func == 'sb':
  funcId    = f_set_byte
  funcSet   = 1
  funcWidth = 1
elif arg_func == 'gb':
  funcId    = f_get_byte
  funcSet   = 0
  funcWidth = 1
elif arg_func == 'sw':
  funcId    = f_set_word
  funcSet   = 1
  funcWidth = 2
elif arg_func == 'gw':
  funcId    = f_get_word
  funcSet   = 0
  funcWidth = 2
else:
  tc.appendLog(11, 'Function -%s- is unknown or not supported!' % arg_func)
  sys.exit()
  
# Get server register
if arg_reg == 'delay_x':
  sreg = rsp.c_hba_sreg_xdelay           # HBA server frontend X delay register
elif arg_reg == 'delay_y':
  sreg = rsp.c_hba_sreg_ydelay           # HBA server frontend Y delay register
  if funcWidth != 1 and funcSet == 1:
    tc.appendLog(11, 'Combination -%s- and -%s- is not supported!' % (arg_func, arg_reg))
    sys.exit()
elif arg_reg == 'version':
  sreg = rsp.c_hba_sreg_version          # HBA server version register
  if funcWidth != 1 and funcSet == 1:
    tc.appendLog(11, 'Combination -%s- and -%s- is not supported!' % (arg_func, arg_reg))
    sys.exit()
elif arg_reg == 'address':
  sreg = rsp.c_hba_sreg_address          # HBA server address register
  if funcWidth != 1 and funcSet == 1:
    tc.appendLog(11, 'Combination -%s- and -%s- is not supported!' % (arg_func, arg_reg))
    sys.exit()
else:
  tc.appendLog(11, 'Server register -%s- is unknown or not supported!' % arg_reg)
  sys.exit()
  
# Define the server data: count values for bc and arg_data for uc
data    = range(2)
data[0] = arg_data[0]
data[1] = 0
if len(arg_data) == 2:
  data[1] = arg_data[1]

server_data = range(256)
for i in range(256):
  server_data[i]    = range(2)
  server_data[i][0] = 0
  server_data[i][1] = 0
  
if arg_access == 'bc':
  for si in range(first_server, last_server+1):
    offset = 1
    for di in range(funcWidth):
      server_data[si][di] = funcWidth*(si-first_server)+offset
      offset += 1
else:
  for di in range(funcWidth):
    server_data[first_server][di] = data[di]
  
# Init random seed to allow reproduceble results for arg_rand
random.seed(0)
    
tc.setResult('PASSED')
  
tc.appendLog(11, '')
tc.appendLog(11, '>>> HBA server access of RSP-%s, BLP-%s via I2C access and control modem of RCU-%s' % (rspId, blpId, rcuId))
tc.appendLog(11, '   Server access   : str_access')
if arg_access == 'bc':
  tc.appendLog(11, '   First server    : %d' % first_server)
  tc.appendLog(11, '   Last server     : %d' % last_server)
else:
  tc.appendLog(11, '   Server address  : %d' % first_server)
tc.appendLog(11, '   Function        : %s' % arg_func)
tc.appendLog(11, '   Register        : %s' % arg_reg)
if funcSet == 1:
  if arg_count==1:
    tc.appendLog(11, '   Server data     : Counter')
  elif arg_rand==1:
    tc.appendLog(11, '   Server data     : Random')
  else:
    tc.appendLog(11, '   Server data     :')
    for si in range(first_server, last_server+1):
      if funcWidth == 1:
        tc.appendLog(11, '   %-3d: d(0) = %3d'             % (si, server_data[si][0]))
      else:
        tc.appendLog(11, '   %-3d: d(0) = %3d, d(1) = %3d' % (si, server_data[si][0], server_data[si][1]))
tc.appendLog(11, '')


################################################################################
# - Determine data request message
  
reg_response_size = rsp.c_hba_reg_response_sz    # HBA client RESPONSE register size in bytes
  
data_request_hdr = []
if arg_access == 'bc':
  # Broadcast request
  data_request_hdr.append(bc_server)
  if funcSet == 1:
    # Set byte or word
    data_request_hdr.append(5 + funcWidth*(last_server - first_server + 1))
    data_request_hdr.append(funcId)
    data_request_hdr.append(sreg)
    data_request_hdr.append(first_server)
    data_request_hdr.append(last_server)
  else:
    # Get byte or word
    tc.appendLog(11, '')
    tc.appendLog(11, '>>> Broadcast get function is allowed but void, because the HBA servers will not respond!')
    tc.appendLog(11, '')
    data_request_hdr.append(5)
    data_request_hdr.append(funcId)
    data_request_hdr.append(sreg)
    data_request_hdr.append(first_server)
    data_request_hdr.append(last_server)
else:
  # Unicast request
  data_request_hdr.append(first_server)
  if funcSet == 1:
    # Set byte or word
    data_request_hdr.append(3 + funcWidth)
    data_request_hdr.append(funcId)
    data_request_hdr.append(sreg)
  else:
    # Get byte or word
    data_request_hdr.append(3)
    data_request_hdr.append(funcId)
    data_request_hdr.append(sreg)
  
################################################################################
# - Testcase initializations

# - Disable external sync
rsp.write_cr_syncoff(tc, msg, blpId, rspId)

# - HBA modem timing
msec     = rsp.c_msec
bc_wait  = rsp.hba_bc_wait
uc_wait  = rsp.hba_uc_wait
gap_wait = rsp.hba_gap_wait
bc_i2c   = rsp.hba_bc_i2c
uc_i2c   = rsp.hba_uc_i2c
reg_i2c  = rsp.hba_reg_i2c
  
  
###############################################################################
# - HBA client I2C access
  
i2c_addr     = rsp.c_rcuh_i2c_addr_hba         # HBA client I2C address (7 bit)
  
cmd_request  = rsp.c_hba_cmd_request           # HBA client REQUEST register
cmd_response = rsp.c_hba_cmd_response          # HBA client RESPONSE register
  
for rep in range(1,1+repeat):

  if arg_count==1:
    # - Use counter write data for each new data request
    count = rep-1
    for si in range(first_server, last_server+1):
      server_data[si][0] = (count >> 8) & 0xFF   # byte data repeats after 2^8 count
      if funcWidth==2:
        server_data[si][1] = count & 0xFF        # word data repeats after 2^16 count
                                                 # use same count for all servers
  elif arg_rand==1:
    # - Use random write data for each new data request
    for si in range(first_server, last_server+1):
      for di in range(funcWidth):
        server_data[si][di] = int(256*random.random())
  # else: use default data
    
  # - Append the write data to the fixed data request header 
  data_request = data_request_hdr
  if arg_access == 'bc':
    # Broadcast request
    if funcSet == 1:
      # Set byte or word
      for si in range(first_server, last_server+1):
        for di in range(funcWidth):
          data_request.append(server_data[si][di])
    # else: Get byte or word
  else:
    # Unicast request
    if funcSet == 1:
      # Set byte or word
      for di in range(funcWidth):
        data_request.append(server_data[first_server][di])
    # else: Get byte or word
    
  # - Define random data for response overwrite
  repsonse_overwrite = []
  for i in range(reg_response_size):
    repsonse_overwrite.append(int(256*random.random()))
    
  if arg_access == 'bc':
    # Broadcast request
    # Define arbitrary response register data that will written first and should not be affected by the broadcast request
    data_response = repsonse_overwrite
  else:
    # Unicast request
    # Determine expected response register data
    data_response = [first_server + 128]
    if funcSet == 1:
      data_response.append(1)
    else:
      data_response.append(funcWidth + 1)
      for di in range(funcWidth):
        data_response.append(server_data[first_server][di])

  # Log for debugging
  #tc.appendLog(11, 'Expected REQUEST  register: %s' % data_request)
  #tc.appendLog(11, 'Expected RESPONSE register: %s' % data_response)
    
  if repeat > 1:
    tc.appendLog(11, '>>> rep')
    
  protocol_list = []
  exp_result    = []
  tc.appendLog(21, 'Overwrite RESPONSE register: %s' % repsonse_overwrite)
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', len(repsonse_overwrite), i2c_addr, repsonse_overwrite, cmd_response))
  exp_result.append(0)
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', gap_wait * msec))
  exp_result.append(0)
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', len(data_request),       i2c_addr, data_request,       cmd_request))
  exp_result.append(0)
  if arg_access == 'bc':
    protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', bc_wait * msec))
  else:
    protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_WAIT', uc_wait * msec))
  exp_result.append(0)
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT', len(data_response),       i2c_addr, None,               cmd_response))
  exp_result.extend(data_response)
  exp_result.append(0)
  protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_END'))
  exp_result.append(0)
  
  # - Overwrite and read the protocol results from the RCUH
  rsp.overwrite_rd_smbh_protocol_results(tc, msg, 'rcuh', rcuId, blpId, rspId)
   
  # - Write (and readback) the protocol list to the RCUH
  rsp.write_rd_smbh_protocol_list(tc, msg, 'rcuh', protocol_list, rcuId, blpId, rspId)

  # - Apply altsync to start the RCUH SMBus protocols
  rsp.write_rsu_altsync(tc, msg, rspId)
  if arg_access == 'bc':
    tc.sleep(reg_i2c + gap_wait + bc_i2c + bc_wait)
  else:
    tc.sleep(reg_i2c + gap_wait + uc_i2c + uc_wait)
  
  # Read the protocol results from the RCUH
  for ri in rspId:
    for bi in blpId:
      for pi in rcuId:
        rd_result = smbus.read_results(tc, msg, 'rcuh', len(exp_result), pi, [bi], [ri])
        # Equal?
        if rd_result == exp_result:
          tc.appendLog(21, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client I2C access result buffer contents is OK' % (ri, bi, pi))
          #tc.appendLog(11, 'Expected protocol result: %s' % exp_result)     # useful for debugging
        else:
          tc.appendLog(11, '>>> RSP-%s, BLP-%s, RCU-%s, HBA client I2C access result buffer contents is wrong:' % (ri, bi, pi))
          tc.appendLog(11, 'Expected protocol result: %s' % exp_result)
          tc.appendLog(11, 'Read     protocol result: %s' % rd_result)
          tc.setResult('FAILED')
    
  if tc.getResult()=='FAILED':
    None
    #break
      
# - Enable external sync
rsp.write_cr_syncon(tc, msg, blpId, rspId)
