""" Read the RSP status for one or all pid, based on TCL testcase 11.1"""

################################################################################
# - Verify options
rspId = tc.rspId
blpId = tc.bpId
blpId.extend(tc.blpId)

# Testcase specific options
arg_edge  = arg_pps_edge
arg_delay = arg_pps_delay

tc.appendLog(11, '')
if arg_read:
  tc.appendLog(11, '>>> Read PPS input delay status for RSP-%s, BLP-%s.' % (rspId, blpId))
  tc.appendLog(11, '')
  for ri in rspId:
    for bi in blpId:
      rsp.read_cr_sync_delay(tc, msg, [bi], [ri])
else:
  if arg_delay==0:
    if arg_edge=='r':
      tc.appendLog(11, '>>> RSP-%s, BLP-%s: Reset PPS input delay to default and capture on rising edge.' % (rspId, blpId))
      rsp.write_cr_sync_delay(tc, msg, 0, 0, blpId, rspId)
    else:
      tc.appendLog(11, '>>> RSP-%s, BLP-%s: Reset PPS input delay to default and capture on falling edge.' % (rspId, blpId))
      rsp.write_cr_sync_delay(tc, msg, 0, 1, blpId, rspId)
  else:
    if arg_edge=='r':
      tc.appendLog(11, '>>> RSP-%s, BLP-%s: Increment PPS input delay %d times and capture on rising edge.' % (rspId, blpId, arg_delay))
      for ri in range(arg_delay):
        rsp.write_cr_sync_delay(tc, msg, 1, 0, blpId, rspId)
    else:
      tc.appendLog(11, '>>> RSP-%s, BLP-%s: Increment PPS input delay %d times and capture on falling edge.' % (rspId, blpId, arg_delay))
      for ri in range(arg_delay):
        rsp.write_cr_sync_delay(tc, msg, 1, 1, blpId, rspId)
tc.appendLog(11, '')
