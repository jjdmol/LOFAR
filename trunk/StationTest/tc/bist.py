"""Run the BIST messages manually, based on TCL testcase 11.5.
"""

# See bist/src/data/bist_msgrom.txt

################################################################################
# - Verify options
rspId = tc.rspId
bpId = ['rsp']
fpgaId = ['rsp', 'blp0', 'blp1', 'blp2', 'blp3']

tc.appendLog(11, '')
tc.appendLog(11, '>>> Run the BIST messages manually on RSP-%s' % rspId)
tc.appendLog(11, '')
  
################################################################################
# - Test selections
tst_duration = rsp.c_diag_duration_quick     # 1 = quick
tst_duration = rsp.c_diag_duration_normal    # 2 = normal
tst_lane = 0

# Ensure CR ext sync is enabled 
#rsp.write_cr_syncon(tc, msg, fpgaId, rspId)
  
# - Overwrite 20 (= 0x10+0x04) RSR selftest result fields with FE starting at offset 30 (= 0x1E).
#
#   ty st frlen blp rsp  pi rg  offset payld  seqnr  rsvd   Data ...
#   02 00 20 00 00  01   01 00  1E 00  10 00  01 00  00 00  FE FE FE FE  FE FE FE FE  FE FE FE FE  FE FE FE FE
#   02 00 14 00 00  01   01 00  2E 00  04 00  02 00  00 00  FE FE FE FE  00 00 00 00  00 00 00 00  00 00 00 00
#
value = 0xFE
rsp.overwrite_rsr(tc, msg, 'mep', value, rspId)
rsp.overwrite_rsr(tc, msg, 'diag', value, rspId)
  
# Check overwriten RSR DIAG
for ri in rspId:
  rsp.read_rsr(tc, msg, 'diag', [ri], 31)
  
# - Write DIAG selftest for LCU
#
#   ty st frlen blp rsp  pi rg  offset payld  seqnr  rsvd   Data ...
#   02 00 14 00 00  01   03 06  00 00  04 00  03 00  00 00  03 01 02 00  00 00 00 00  00 00 00 00  00 00 00 00
#
tst_interface = rsp.c_diag_dev_lcu          # 3 = LCU
tst_mode      = rsp.c_diag_mode_loop_local  # 1 = loop local
selftest = [tst_interface, tst_mode, tst_duration, tst_lane]
for ri in rspId:
  rsp.write_diag_selftest(tc, msg, selftest, bpId, [ri], 99)
tc.sleep(7000)
  
# - Write DIAG selftest for CEP
#
#   ty st frlen blp rsp  pi rg  offset payld  seqnr  rsvd   Data ...
#   02 00 14 00 00  01   03 06  00 00  04 00  04 00  00 00  04 01 02 00  00 00 00 00  00 00 00 00  00 00 00 00
#
tst_interface = rsp.c_diag_dev_cep          # 4 = CEP
tst_mode      = rsp.c_diag_mode_loop_local  # 1 = loop local
selftest = [tst_interface, tst_mode, tst_duration, tst_lane]
for ri in rspId:
  rsp.write_diag_selftest(tc, msg, selftest, bpId, [ri], 99)
tc.sleep(5000)
  
# - Write DIAG selftest for SERDES
#
#   ty st frlen blp rsp  pi rg  offset payld  seqnr  rsvd   Data ...
#   02 00 14 00 00  01   03 06  00 00  04 00  05 00  00 00  05 01 02 00  00 00 00 00  00 00 00 00  00 00 00 00
#
tst_interface = rsp.c_diag_dev_serdes       # 5 = SERDES
tst_mode      = rsp.c_diag_mode_loop_local  # 1 = loop local
selftest = [tst_interface, tst_mode, tst_duration, tst_lane]
for ri in rspId:
  rsp.write_diag_selftest(tc, msg, selftest, bpId, [ri], 99)
tc.sleep(1000)
  
# - Write DIAG selftest for RI
#
#   ty st frlen blp rsp  pi rg  offset payld  seqnr  rsvd   Data ...
#   02 00 14 00 0F  01   03 06  00 00  04 00  06 00  00 00  00 06 02 00  00 00 00 00  00 00 00 00  00 00 00 00
#
tst_interface = rsp.c_diag_dev_ri           # 0 = RI
tst_mode      = rsp.c_diag_mode_bus         # 6 = mode bus is mode tx,rx
selftest = [tst_interface, tst_mode, tst_duration, tst_lane]
for ri in rspId:
  rsp.write_diag_selftest(tc, msg, selftest, fpgaId, [ri], 99)
tc.sleep(1000)
  
# - Read RSR to get BIST result, starting with MEP status error field (offset 0x1A) and length 0x18 bytes
#
#   ty st frlen blp rsp  pi rg  offset payld  seqnr  rsvd   Data ...
#   01 00 10 00 00  01   01 00  1A 00  18 00  07 00  00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00
#
for ri in rspId:
  rsp.read_mem(tc, msg, 'rsr', 'status', rsp.c_ei_status_rsr_size, bpId, [ri], 'h', 1, rsp.c_ei_status_rsr_offset)
  
  msg.setOffset(rsp.c_ei_status_mep_offset)
  dummy          = msg.readUnsigned(2)
  mep_prev_msg   = msg.readUnsigned(1)
  dummy          = msg.readUnsigned(1)

  msg.setOffset(rsp.c_ei_status_diag_offset)
  # DIAG status
  diag_interface = msg.readUnsigned(1)
  diag_mode      = msg.readUnsigned(1)
  diag_ri_bp     = msg.readUnsigned(2)
  diag_rcux      = msg.readUnsigned(2)
  diag_rcuy      = msg.readUnsigned(2)
  diag_eth_LCU   = msg.readUnsigned(2)
  diag_eth_CEP   = msg.readUnsigned(2)
  diag_serdes    = msg.readUnsigned(2)
  diag_ri_ap0    = msg.readUnsigned(2)
  diag_ri_ap1    = msg.readUnsigned(2)
  diag_ri_ap2    = msg.readUnsigned(2)
  diag_ri_ap3    = msg.readUnsigned(2)

  # Verify
  if mep_prev_msg == 0                 and \
     diag_ri_bp   == rsp.c_diag_res_ok and \
     diag_rcux    != rsp.c_diag_res_ok and \
     diag_rcuy    != rsp.c_diag_res_ok and \
     diag_eth_LCU == rsp.c_diag_res_ok and \
     diag_eth_CEP == rsp.c_diag_res_ok and \
     diag_serdes  == rsp.c_diag_res_ok and \
     diag_ri_ap0  == rsp.c_diag_res_ok and \
     diag_ri_ap1  == rsp.c_diag_res_ok and \
     diag_ri_ap2  == rsp.c_diag_res_ok and \
     diag_ri_ap3  == rsp.c_diag_res_ok:
    tc.appendLog(11, 'RSP-%s : BIST sequence went OK' % ri)
    tc.setResult('PASSED')
  else:
    tc.appendLog(11, 'RSP-%s : BIST sequence went wrong:' % ri)
    tc.appendLog(11, '  mep msg = %d' % mep_prev_msg)
    tc.appendLog(11, '  ri-bp   = %d' % diag_ri_bp)
    tc.appendLog(11, '  rcux    = %d' % diag_rcux)
    tc.appendLog(11, '  rcuy    = %d' % diag_rcuy)
    tc.appendLog(11, '  eth-LCU = %d' % diag_eth_LCU)
    tc.appendLog(11, '  eth-CEP = %d' % diag_eth_CEP)
    tc.appendLog(11, '  serdes  = %d' % diag_serdes)
    tc.appendLog(11, '  ri-ap0  = %d' % diag_ri_ap0)
    tc.appendLog(11, '  ri-ap1  = %d' % diag_ri_ap1)
    tc.appendLog(11, '  ri-ap2  = %d' % diag_ri_ap2)
    tc.appendLog(11, '  ri-ap3  = %d' % diag_ri_ap3)
    tc.setResult('FAILED')
  
# Read RSR DIAG again to also display result in text instead of numbers
for ri in rspId:
  rsp.read_rsr(tc, msg, 'diag', [ri], 21)
