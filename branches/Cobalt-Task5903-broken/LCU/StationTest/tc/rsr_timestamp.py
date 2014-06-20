"""Write or read the RSR timestamp register, based on TCL testcase 11.3

  - Arguments: --fp [rsp 0 1 2 3] --data timestamp,mode --read
 
   
   Write RSR timestamp:
   > python verify.py --brd rsp0 -v 21 --te tc/rsr_timestamp.py --data 10,1
   Read RSR timestamp:
   > python verify.py --brd rsp0 -v 21 --te tc/rsr_timestamp.py --read

"""

################################################################################
# - Verify options
rspId = tc.rspId
fpgaId = tc.bpId
fpgaId.extend(tc.blpId)

if arg_read:
    rsp.read_rsr_timestamp(tc, msg, fpgaId, rspId, 11)
else:
    timestamp = arg_data[0]
    timemode  = 1
    if len(arg_data)>1:
        timemode  = arg_data[1]
    rsp.write_rsr_timestamp(tc, msg, timestamp, timemode, fpgaId, rspId, 11)
  