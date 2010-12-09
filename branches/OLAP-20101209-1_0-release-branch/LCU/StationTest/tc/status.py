""" Read the RSP status for one or all pid, based on TCL testcase 11.1"""

################################################################################
# - Verify options
rspId = tc.rspId
repeat = tc.repeat

# Testcase specific options

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
tc.appendLog(11, '>>> Selected RSR read %s for RSP-%s.' % (arg_procid, rspId))
tc.appendLog(11, '')


#############################################################################
# Read RSR for each RSP board

if repeat==1:
  status = rsp.read_rsr(tc, msg, arg_procid, rspId, 11)
else:
  for rep in range(1,1+repeat):
    tc.appendLog(11, '>>> %d' % rep)
    if arg_procid == 'rad':
      for ri in rspId:
        status = rsp.read_rsr(tc, msg, arg_procid, [ri], 99)
        rad_status = status[0][0]  # based on bit 20 (= align, 0 is ok),
                                   #          bit 19 (= sync, 1 is ok),
                                   #          bit 18 (= brc, 0 is ok)
        rad_raw =  status[0][1]    # rad_status info with count (bits 17:0)
        rad_cnt =  status[0][1][0] & 0x3FFFF # use RI count bits 17:0
        
        rsr_str   = '%d' % rad_status
        for rd in rad_raw:
          rsr_str += '%8d' % rd
        if rad_status == rsp.c_rsr_undefined:
          tc.appendLog(11, '%s, %d: RAD status is OFF    (%s)' % (ri, rep, rsr_str))
        elif rad_status == rsp.c_rsr_ok:
          tc.appendLog(21, '%s, %d: RAD status is OK    (%s), RI count = %d' % (ri, rep, rsr_str, rad_cnt))
        else:
          tc.appendLog(11, '%s, %d: RAD status is wrong : %s' % (ri, rep, rsr_str))
    else:
      for ri in rspId:
        status = rsp.read_rsr(tc, msg, arg_procid, [ri], 11)
    #tc.sleep(100)   # less than 1 sec so we do not miss an interval status
