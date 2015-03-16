"""Write or read the RSR SDO register, based on TCL testcase 11.10

  - Arguments: --fp [rsp 0 1 2 3] --sm # --read
 
   
   Write RSR SDO mode:
   > python verify.py --brd rsp0 -v 21 --te tc/rsr_sdo_mode.py --sm 0
   Read RSR SDO mode:
   > python verify.py --brd rsp0 -v 21 --te tc/rsr_sdo_mode.py --read

"""

################################################################################
# - Verify options
rspId = tc.rspId
fpgaId = tc.bpId
fpgaId.extend(tc.blpId)
sdoMode = tc.sdoMode

if arg_read:
    rsp.read_rsr_sdo_mode(tc, msg, fpgaId, rspId, 11)
else:
    rsp.write_rsr_sdo_mode(tc, msg, sdoMode, fpgaId, rspId, 11)
  