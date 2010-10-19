"""Test the SERDES ring between RSP boards, based on TCL testcase 3.8

   - Specific arguments:
     . diag_mode:
         off    = rsp.c_diag_mode_no_tst   --> enable sync to stop continuous tx test
         tx     = rsp.c_diag_mode_loop_tx  --> keep ext sync disabled to continue tx
         rx     = rsp.c_diag_mode_loop_rx
         tx_rx  = rsp.c_diag_mode_loop_tx_rx
         local  = rsp.c_diag_mode_loop_local --> use local loopback (idem as testcase 3.6)
     . diag_sync: When 0 use external sync, else use altsync with sync interval time
                  given by 'sync' sec.
     . diag_data:
         cntr   = use incremental data
         lfsr   = use LFSR data

   - Note:
     . All four lanes are verified in parallel
"""
################################################################################

# NOTE:
#   When using arg_sync = 0 to select the external sync the --brd order of RSP
#   boards should match the fysical SERDES tx order. Due to the slow script
#   (few MEP messages/sec) the test migth otherwise fail when too many boards
#   are in the test.

################################################################################
# - Verify options
rspId = tc.rspId
repeat = tc.repeat

# - Rename testcase specific options
arg_mode = arg_diag_mode
arg_sync = arg_diag_sync
arg_data = arg_diag_data

################################################################################
# - Test selections
tst_interface = rsp.c_diag_dev_serdes
tst_lane      = 0xFF            # not used
  
if   arg_mode == 'off':   tst_mode = rsp.c_diag_mode_no_tst
elif arg_mode == 'tx':    tst_mode = rsp.c_diag_mode_tx
elif arg_mode == 'rx':    tst_mode = rsp.c_diag_mode_rx
elif arg_mode == 'tx_rx': tst_mode = rsp.c_diag_mode_tx_rx
elif arg_mode == 'local': tst_mode = rsp.c_diag_mode_loop_local
else:
  tst_mode = rsp.c_diag_mode_loop_local
  arg_mode = 'local'
  tc.appendLog(11, 'ILLEGAL test mode, default to test mode %s.' % arg_mode)
  
if arg_mode == 'tx':
  # Use mode 'tx' as continuous tx, so no repeat necessary. Mode 'off' will stop the tx.
  repeat = 1

# Incremental data is generated for test duration debug, else LFSR data
if   arg_data == 'cntr': tst_duration = rsp.c_diag_duration_debug
elif arg_data == 'lfsr': tst_duration = rsp.c_diag_duration_quick
else:
  tst_duration = rsp.c_diag_duration_quick
  arg_data = 'lfsr'

selftest = [tst_interface, tst_mode, tst_duration, tst_lane]

#############################################################################
# Constants
  
if arg_mode == 'off':
  tc.appendLog(11, '')
  tc.appendLog(11, '>>> RSP-%s, stop any previous SERDES tx selftest using mode %s.' % (rspId, arg_mode))
  tc.appendLog(11, '')
  # - Stop any previous tx selftest
  rsp.write_rsu_altsync(tc, msg, rspId)         # start of tst sync interval
  rsp.write_rsu_altsync(tc, msg, rspId)         # end of tst sync interval
  rsp.write_rsu_altsync(tc, msg, rspId)         # end tx
  rsp.write_cr_syncon(tc, msg, ['rsp'], rspId)  # restore CR ext sync enabled default
else:
  # - Run selftest
    
  tc.appendLog(11, '')
  if arg_sync == 0:
    tc.appendLog(11, '>>> RSP-%s, run SERDES test: mode %s, data %s, using external sync from TD clock board.' % (rspId, arg_mode, arg_data))
  else:
    tc.appendLog(11, '>>> RSP-%s, run SERDES test: mode %s, data %s, using RSP on board alternative sync.' % (rspId, arg_mode, arg_data))
  tc.appendLog(11, '')
  
  for rep in range(1,1+repeat):
    # Temporarily disable external sync
    rsp.write_cr_syncoff(tc, msg, ['rsp'], rspId)
  
    value = 0xAB
    tc.appendLog(31, '')
    tc.appendLog(31, '>>> Overwrite RSR diag bytes with %#x to ensure fresh status.' % value)
    tc.appendLog(31, '')
    rsp.overwrite_rsr(tc, msg, 'diag', value, rspId)
    for ri in rspId:
      rsp.read_rsr(tc, msg, 'diag', [ri], 31)

    # Configure DIAG tst
    rsp.write_diag_selftest(tc, msg, selftest, ['rsp'], rspId, 99)

    tc.sleep(100)        # wait DIAG ack_period_done

    # Sync control
    if arg_mode != 'tx':
      if arg_sync == 0:
        # Enable external sync, first ext sync will start test, next ext sync will end test
        rsp.write_cr_syncon(tc, msg, ['rsp'], rspId)

        # Wait for serdes_diag_lane tx, rx test to finish (1 until start sync, 1 measurement sync interval, 1 sync interval
        # for tx to finish, so worst case 3 sec)
        tc.sleep(3000)
      else:
        # Run serdes_diag_lane tx, rx test for 1 sync interval (arbitrary long, typically 1 sec)
        rsp.write_rsu_altsync(tc, msg, rspId)        # start of sync interval
        tc.sleep(arg_sync*1000)
        rsp.write_rsu_altsync(tc, msg, rspId)        # end of sync interval
        rsp.write_rsu_altsync(tc, msg, rspId)        # end tx
        tc.sleep(1000)                               # allow time for DIAG tst result event to reach RSR
        rsp.write_cr_syncon(tc, msg, ['rsp'], rspId)  # restore CR ext sync enabled default
    # else: For mode 'tx' keep ext sync off to run tx forever, i.e. until tc is rerun with mode 'off'.

    for ri in rspId:
      tc.appendLog(31, '')
      tc.appendLog(31, '>>> %d: RSP-%s, read RSR diag status:' % (rep, ri))
      tc.appendLog(31, '')

      status = rsp.read_rsr(tc, msg, 'diag', [ri], 31)
      status = status[0][0][:]
      diag_interface     = status[0]
      diag_mode          = status[1]
      diag_ri_errors     = status[2]
      diag_rcux_errors   = status[3]
      diag_rcuy_errors   = status[4]
      diag_lcu_errors    = status[5]
      diag_cep_errors    = status[6]
      diag_serdes_errors = status[7]
      diag_ap0_ri_errors = status[8]
      diag_ap1_ri_errors = status[9]
      diag_ap2_ri_errors = status[10]
      diag_ap3_ri_errors = status[11]

      if arg_mode == 'tx':
        if diag_interface == value:
          tc.appendLog(11, '>>> RSP-%s, SERDES tx only is active, use mode off to stop it.' % ri)
          tc.appendLog(11, '')
        else:
          tc.appendLog(11, '>>> RSP-%s, SERDES tx only went wrong.' % ri)
          tc.appendLog(11, '    Unexpected test interface %d.' % diag_interface)
          tc.setResult('FAILED')
      elif diag_interface     == tst_interface and \
           diag_mode          == tst_mode      and \
           diag_serdes_errors == 0:
        tc.appendLog(11, '>>> %d: RSP-%s, SERDES test went OK.' % (rep, ri))
        tc.setResult('PASSED')
      else:
        tc.appendLog(11, '>>> %d: RSP-%s, SERDES test went wrong.' % (rep, ri))
        if diag_interface != tst_interface:
          tc.appendLog(11, '    Unexpected test interface %d.' % diag_interface)
        elif diag_mode != tst_mode:
          tc.appendLog(11, '    Unexpected test mode %d.' % diag_mode)
        else:
          lane_mask = 2**rsp.c_nof_lanes - 1
          lane_ref  = 2**rsp.c_nof_lanes
          for i in range(rsp.c_nof_lanes):
            lane_errors = (diag_serdes_errors >> (i*rsp.c_nof_lanes)) & lane_mask
            if lane_errors == rsp.c_diag_res_ok:
              tc.appendLog(11, '    Lane %d went OK.' % i)
            elif lane_errors == rsp.c_diag_res_none:
              tc.appendLog(11, '    Lane %d went wrong, nothing happened.' % i)
            elif lane_errors == rsp.c_diag_res_sync_timeout:
              tc.appendLog(11, '    Lane %d went wrong, sync timeout.' % i)
            elif lane_errors == rsp.c_diag_res_data_timeout:
              tc.appendLog(11, '    Lane %d went wrong, data timeout.' % i)
            elif lane_errors == rsp.c_diag_res_word_err:
              tc.appendLog(11, '    Lane %d went wrong, word errors occured.' % i)
            elif lane_errors == rsp.c_diag_res_illegal:
              tc.appendLog(11, '    Lane %d illegal status %d.' % (i, lane_errors))
            else:
              tc.appendLog(11, '    Lane %d unknown status %d.' % (i, lane_errors))
        tc.setResult('FAILED')
    # Break repeat loop in case test FAILED
    if tc.getResult() == 'FAILED':
      break
  # Restore external sync enabled
  rsp.write_cr_syncon(tc, msg, ['rsp'], rspId)
