"""Read the RAD latency, based on TCL testcase 5.49"""

################################################################################
# - Verify options
rspId = tc.rspId
repeat = tc.repeat

tc.appendLog(11, '')
tc.appendLog(11, '>>> Read RAD_BP latency for RSP-%s.' % rspId)
tc.appendLog(11, '')


#############################################################################
# Read RAD latency for each RSP board

if repeat==1:
  latency = rsp.read_rad_latency(tc, msg, rspId, 11)
else:
  for rep in range(1,1+repeat):
    tc.appendLog(11, '>>> Rep %d' % rep)
    latency = rsp.read_rad_latency(tc, msg, rspId, 11)
