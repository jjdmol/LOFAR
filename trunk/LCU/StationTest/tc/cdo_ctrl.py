"""Write or read the CDO control"""

################################################################################
# - Verify options
rspId = tc.rspId

# - Construct settings word from arg_data list
cdo_ctrl = arg_data[0]


#############################################################################
# Read RAD latency for each RSP board

if arg_read:
  for ri in rspId:
    rsp.read_cdo_ctrl(tc, msg, [ri], 11)
else:
  rsp.write_cdo_ctrl(tc, msg, cdo_ctrl, rspId, 11)
