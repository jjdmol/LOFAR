"""Overwrite and readback RSR"""

################################################################################
# - Verify options
rspId = tc.rspId

value = 0xBC
value = arg_data[0] & 0xFF        # Use overwrite byte value from arg_data list

if not (arg_procid in ['rsp',
                       'eth',
                       'mep',
                       'diag',
                       'bs',
                       'rcuh',
                       'rsu',
                       'ado',
                       'rad',
                       'all']):
  arg_procid = 'all'
  tc.appendLog(11, 'Forced unknown procid to \'all\'.')

tc.appendLog(11, '')
tc.appendLog(11, '>>> RSP-%s, overwrite RSR %s with value %#x and read back.' % (rspId, arg_procid, value))
tc.appendLog(11, '')
  

################################################################################
# - Run RSR overwrite test
    
rsp.overwrite_rsr(tc, msg, arg_procid, value, rspId)
for ri in rspId:
  rsp.read_rsr(tc, msg, arg_procid, [ri], 11)
  