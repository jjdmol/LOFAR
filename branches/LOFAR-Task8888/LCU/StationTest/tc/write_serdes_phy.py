"""Serdes read or write of one PHY register in the PMC 8358 10G Serdes PHY chip on RSP

   - Specific arguments:
     . arg_read       : if present then read PHY register, else write to PHY register
     . arg_hexdata[0] : register address for read or write
     . arg_hexdata[1] : register data in case of write
     
   - Example:
  
     The PHY register D002 defines the clock-data timing of the interface between BP and Serdes chip
       reg = D002 --> Master configuration 2 register address
       dat = 0000 --> bit 3 = REDGE: '1' RX_CLK from PHY is center       aligned with data D,C,H
                                     '0' RX_CLK from PHY is simultaneous aligned with data D,C,H
                      bit 2 = TEDGE: '1' TX_CLK to   PHY is center       aligned with data D,C,H
                                     '0' TX_CLK to   PHY is simultaneous aligned with data D,C,H

     Write PHY register D002:
       python verify.py --brd rsp0,rsp1,rsp2,rsp3 -v 21 --te tc/write_serdes_phy.py --args hexdata d002,0008
    
     Read PHY register D002:
    
       python verify.py --brd rsp0,rsp1,rsp2,rsp3 -v 21 --te tc/write_serdes_phy.py --args hexdata d002 --read
"""

################################################################################
# User imports
import mmd_serdes
           
################################################################################
# Verify options
rspId = tc.rspId
reg = arg_hexdata[0]
dat = arg_hexdata[1]

################################################################################
# Execution
if arg_read:
    for ri in rspId:
        dat = mmd_serdes.readReg(tc, msg, reg, [ri])
        tc.appendLog(11, 'RSP-%s : Read SERDES PHY register 0x%04X = 0x%04X' % (ri, reg, dat))
else:
    mmd_serdes.writeReg (tc, msg, reg, dat, rspId)
    tc.appendLog(11, '>>> RSP-%s : Write SERDES PHY register 0x%04X := 0x%04X' % (rspId, reg, dat))

