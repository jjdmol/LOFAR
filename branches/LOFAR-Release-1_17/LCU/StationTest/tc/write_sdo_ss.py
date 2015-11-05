"""Write both pages of SDO SS"""

################################################################################
# - Verify options
blpId = tc.blpId
rspId = tc.rspId

banks  = arg_data
ss_map = arg_hexdata

tc.appendLog(11, '')
tc.appendLog(11, '>>> RSP-%s, write SDO SS banks-%s with ss_map %s.' % (rspId, banks, ss_map))
tc.appendLog(11, '')
  

################################################################################
# - Run RSR overwrite test

bypass_dc                       = 1
bypass_page_swap_on_system_sync = (1<<12)

#for ri in rspId:
#    read_diag_bypass(tc, msg, blpId, ri):

# Use RSU alt_sync to cause SDO SS page swap
bypass = bypass_dc + bypass_page_swap_on_system_sync
write_diag_bypass(tc, msg, bypass, blpId, rspId)
write_cr_syncoff(tc, msg, blpId, rspId)

# . first page
rsp.write_sdo_ss(tc, msg, ss_map, blpId, rspId, banks)
write_rsu_altsync(tc, msg, rspId)

# . other page
rsp.write_sdo_ss(tc, msg, ss_map, blpId, rspId, banks)
write_rsu_altsync(tc, msg, rspId)

# Restore default sync for dual page swap
bypass = bypass_dc
write_diag_bypass(tc, msg, bypass, blpId, rspId)
write_cr_syncon(tc, msg, blpId, rspId)
