"""Write or read the DIAG bypass register, based on TCL testcase 5.23

  - Arguments: --fp [rsp 0 1 2 3] --hexdata # --read
 
     bit
       0   dc     =  Bypass DC
       1   pfs    =  Bypass PFS
       2   pft    =  Bypass PFT
       3   bf     =  Bypass BF
       4   six    =  SI enable X
       5   siy    =  SI enable Y
       6   sync   =  DIAG result buffer use sync
       7   resync =  DIAG result buffer use resync
       8   pftsw  =  PFT switching disable
    10:9   bank   =  DIAG result buffer for BM bank 
      11   bp     =  DIAG result buffer for BP      
      12   swap   =  Page swap on system sync       
   
   Write DIAG bypass:
   > python verify.py --brd rsp0 -v 21 --te tc/diag_bypass.py --hexdata 1001
   Read DIAG bypass:
   > python verify.py --brd rsp0 -v 21 --te tc/diag_bypass.py --read

"""

################################################################################
# - Verify options
rspId = tc.rspId
fpgaId = tc.bpId
fpgaId.extend(tc.blpId)

if arg_read:
  for ri in rspId:
    for fi in fpgaId:
      rsp.read_diag_bypass(tc, msg, [fi], [ri], 11)
else:
  bypass = arg_data[0]
  rsp.write_diag_bypass(tc, msg, bypass, fpgaId, rspId, 11)
  