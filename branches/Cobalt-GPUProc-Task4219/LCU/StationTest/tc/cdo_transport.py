"""Read the CDO transport header"""

################################################################################
# - Verify options
rspId = tc.rspId

if arg_read:
  for ri in rspId:
    rsp.read_cdo_transport(tc, msg, [ri], 11)
else:
  tc.appendLog(11, 'Must use --read argument')

