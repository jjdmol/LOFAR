"""Testcase for no DC offset

  Note: No specific arguments
"""

################################################################################
# Constants


################################################################################
# - Verify options
rspId = tc.rspId
blpId = tc.blpId

repeat = tc.repeat
tc.setResult('PASSED')   # self checking test, so start assuming it will run PASSED

tc.appendLog(11,'')
tc.appendLog(11,'>>> Set DC blocker for RSP-%s, BLP-%s' % (rspId, blpId))
tc.appendLog(11,'')
  
################################################################################
# - Testcase initializations


#bypass = 0x01  # Enable DC blocker

# default bypass is 0x01 Disable DC blocker


for ri in rspId:
  for bi in blpId:
    rsp.write_diag_bypass(tc, msg, 4, blpId, rspId)
    print rsp.read_diag_bypass(tc,msg,blpId,rspId)
    
      
    