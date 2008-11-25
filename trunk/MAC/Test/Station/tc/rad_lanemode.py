"""Write or read the RAD lane mode, based on TCL testcase 5.24"""

################################################################################
# - Verify options
rspId = tc.rspId
repeat = tc.repeat

# - Construct settings word from arg_rad_lane_mode
#
#   settings: one byte for each lane, byte [3:0] for lane [3:0]
#     format: XXXXAABB
#       where XX = don't care
#             AA = xlet mode
#             BB = blet mode
#   
#        mode 00 = ignore remote data (only local)  DEFAULT
#        mode 01 = disable
#        mode 10 = combine local and remote data
#        mode 11 = ignore local data (only remote)
#
#   arg_rad_lane_mode = [X3,X2,X1,X0,B3,B2,B1,B0]

settings = 0
for i in range(rsp.c_nof_lanes):
  settings += ((arg_rad_lane_mode[i] << 2) + arg_rad_lane_mode[i+rsp.c_nof_lanes]) << (8*i)  # X,B[3:0]


#############################################################################
# Read RAD latency for each RSP board

if arg_read:
  for ri in rspId:
    rsp.read_rad_settings(tc, msg, [ri], 11)
else:
  rsp.write_rad_settings(tc, msg, settings, rspId, 11)
