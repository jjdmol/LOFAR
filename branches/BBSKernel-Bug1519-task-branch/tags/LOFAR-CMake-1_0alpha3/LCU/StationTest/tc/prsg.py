"""Testcase for RCU - RSP data interface using PRSG, based on TCL testcase 5.10

  Note: No specific arguments
"""

################################################################################
# Constants

nof_reflets_ap  = rsp.c_nof_reflets_ap
nof_beamlets    = rsp.c_nof_beamlets      # maximum capable by RSP gateware
#nof_beamlets    = 216                     # sufficient nof beamlets for 32 MHz BW
nof_beamlets_ap = rsp.c_nof_beamlets_ap           # including reflets
nof_beamlets_ap = nof_reflets_ap + nof_beamlets   # including reflets

# - SS output size
c_ss_reflets_size = rsp.c_pol * nof_reflets_ap
c_ss_size         = rsp.c_pol * nof_beamlets_ap

c_ss_gap          = rsp.c_slice_size - rsp.c_cpx * rsp.c_pol * nof_beamlets_ap

# - Datapath result buffer
c_res_word_width        = 4
c_res_nof_words         = rsp.c_cpx * rsp.c_pol * nof_beamlets_ap    # include read reflets
c_res_nof_words_per_pol = rsp.c_cpx *             nof_beamlets       # skipped reflets

################################################################################
# - Verify options
rspId = tc.rspId
blpId = tc.blpId
polId = tc.polId
repeat = tc.repeat
tc.setResult('PASSED')   # self checking test, so start assuming it will run PASSED

tc.appendLog(11,'')
tc.appendLog(11,'>>> Capture PRSG data for RSP-%s, BLP-%s, RCU-%s' % (rspId, blpId, polId))
tc.appendLog(11,'')
  
################################################################################
# - Testcase initializations

# - Set RCU in PRSG mode
rsp.rspctl(tc, '--rcuprsg')
rsp.rspctl(tc, '--rcuenable=1')

bypass = 0x8F  # Bypass data path to have direct access to RCU data via SS, use resync
               # version of pps to preserve X and Y order in captured data

# - Set SS mapping
# Default use incrementing SS mapping
ss_map = []
for i in range(0, rsp.c_pol * nof_beamlets_ap):
  ss_map.append(i)

# Use rspctl to write the SS mapping
rsp.rspctl(tc, '--subbands=0:%d' % (nof_beamlets-1))

# Write the SS mapping, repeat twice to ensure both pages are written
#r = 1
#for i in range(0,r):
#  rsp.write_ss(tc, msg, ss_map, blpId, rspId)
#  tc.sleep(1000)
#  rsp.write_ss(tc, msg, ss_map, blpId, rspId)
#  tc.sleep(1000)

# Apparently rspctl updates the SS every pps, so overwriting it does not work.
# Disabling SS update in RSPDriver.conf may be an option. However instead adapt
# this test case to the actual SS by reordering the captured data accordingly.
# Therefore read the actual SS into ss_map.

# Read actual SS mapping, to use it to reorder the read DIAG result
# . assume both pages and all BLP use same mapping
# . strip the reflets
# . assume that the beamlets mapping uses all subbands
bi = [blpId[0]]
ri = [rspId[0]]
ss_map = rsp.read_ss(tc, msg, c_ss_size, bi, ri)
ss_map = ss_map[c_ss_reflets_size:]
tc.appendLog(21,'Active SS map (length %d):' % len(ss_map))
tc.appendLog(21,'%s' % ss_map)

rsp.write_cr_syncoff(tc, msg, blpId, rspId)
rsp.write_diag_bypass(tc, msg, bypass, blpId, rspId)


################################################################################
# Run the test
for k in range(0, repeat):
  rsp.write_rsu_altsync(tc, msg, rspId)      # Apply altsync to capture a new result buffer
  tc.sleep(100)

  for ri in rspId:
    for bi in blpId:
      res_buffer = rsp.read_diag_result_buffer(tc, msg, c_res_nof_words, c_res_word_width, [bi], [ri])
    
      res ={'x':[], 'y':[]}
      for x in range(0, len(res_buffer), 2):
        res['x'].append(res_buffer[x] & rsp.c_rcu_dat_mask)
      for y in range(1, len(res_buffer), 2):
        res['y'].append(res_buffer[y] & rsp.c_rcu_dat_mask)
      res['x'] = res['x'][c_ss_reflets_size:]    # strip the reflets
      res['y'] = res['y'][c_ss_reflets_size:]
      res['x'] = rsp.reorder(res['x'], ss_map)   # reorder according to SS map
      res['y'] = rsp.reorder(res['y'], ss_map)
      
      for pi in polId:
        first = 1     # First result sample is used as starting seed for the expected samples
        ok    = 0     # 0 = OK
        if len(res[pi]) == c_res_nof_words_per_pol:
          for rs in res[pi]:
            if first == 0:
              if ok == 0:
                if rs != rsp.calculate_next_sequence_value(rs_prev):
                  # Mismatch, so bridge the potential SS gap in the sample stream to verify whether this is the cause
                  rs_gap = rs_prev
                  for i in range(0, c_ss_gap):
                    rs_gap = rsp.calculate_next_sequence_value(rs_gap)
                  if rs != rsp.calculate_next_sequence_value(rs_prev):
                    # Mismatch again, so assume the potential SS gap was not the cause of the initial mismatch
                    nxt_rs_prev = rsp.calculate_next_sequence_value(rs_prev)
                    exp.append(nxt_rs_prev)
                    ok = 1   # 1 = sample mismatch
                  else:
                    # OK, so bridge the SS gap in the expected results
                    nxt_rs_prev = rsp.calculate_next_sequence_value(rs_gap)
                    exp.append(nxt_rs_prev)
                else:
                  # OK, no SS gap to bridge    
                  nxt_rs_prev = rsp.calculate_next_sequence_value(rs_prev)
                  exp.append(nxt_rs_prev)
              else:
                # A mismatch has aleready occured, no need to check for more mismatches
                nxt_rs_prev = rsp.calculate_next_sequence_value(rs_prev)
                exp.append(nxt_rs_prev)
            else:
              first       = 0
              nxt_rs_prev = res[pi][0]
              exp         = [nxt_rs_prev]
            rs_prev = nxt_rs_prev
        else:
          ok = 2    # 2 = length error

        # Report results      
        if ok == 0:
          tc.appendLog(11,'>>> %d : RSP-%s, BLP-%s, RCU-%s PRSG data is OK.' % (k, ri, bi, pi))
        elif ok == 1:
          tc.appendLog(11,'>>> %d : RSP-%s, BLP-%s, RCU-%s PRSG data mismatch.' % (k, ri, bi, pi))
          tc.appendLog(11,'- Expected data:')
          tc.appendLog(11,'%s' % exp)
          tc.appendLog(11,'- Captured data:')
          tc.appendLog(11,'%s' % res[pi])
          tc.setResult('FAILED')
        else:
          tc.appendLog(11,'>>> %d : RSP-%s, BLP-%s, RCU-%s PRSG data length mismatch.' % (k, ri, bi, pi))
          tc.appendLog(11,'Captured length %d != expected length %d' % (len(res[pi]), c_res_nof_words_per_pol))
          tc.setResult('FAILED')

# Restore defaults
bypass = 1
rsp.write_diag_bypass(tc, msg, bypass, blpId, rspId, 99)
rsp.write_cr_syncon(tc, msg, blpId, rspId)
