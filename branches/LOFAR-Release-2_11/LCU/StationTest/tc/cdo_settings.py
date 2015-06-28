"""Read the CDO settings"""

################################################################################
# - Verify options
rspId = tc.rspId

if arg_read:
  for ri in rspId:
    rsp.read_cdo_settings(tc, msg, [ri], 11)
else:
  tc.appendLog(11, 'Must use --read argument')

