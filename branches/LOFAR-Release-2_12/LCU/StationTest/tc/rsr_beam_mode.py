"""Write or read the RSR Beam Mode register, based on TCL testcase 11.9

  - Arguments: --fp [rsp 0 1 2 3] --bm # --read
 
   
   Write RSR beam mode:
   > python verify.py --brd rsp0 -v 21 --te tc/rsr_beam_mode.py --bm 0
   Read RSR beam mode:
   > python verify.py --brd rsp0 -v 21 --te tc/rsr_beam_mode.py --read

"""

################################################################################
# - Verify options
rspId = tc.rspId
fpgaId = tc.bpId
fpgaId.extend(tc.blpId)
beamMode = tc.beamMode

if arg_read:
    rsp.read_rsr_beam_mode(tc, msg, fpgaId, rspId, 11)
else:
    rsp.write_rsr_beam_mode(tc, msg, beamMode, fpgaId, rspId, 11)
  