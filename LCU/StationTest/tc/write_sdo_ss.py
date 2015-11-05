"""Write both pages of SDO SS

   >>> See also tc 5.54 and 5.56
   
   WG: 
     The WG can be set with rspctl --wg. A WG amplitude of 0.5 yields a CW
     amplitude of 255 and a subband amplitude of 32582 so that just fits in
     16 bit SDO data. With fsys = 160 M or 200 M and c_rsp_slice_size=124 the
     WG frequency determines the PFB bin as:
     
       cw_bin = round(freq/fsys * c_rsp_slice_size).
   
   SDO_SS:
     AP             SP   Subband
                         0 1 ...8  9   ...17 18  ...26 27  ...35
      0 sdo_ss_map: 0,1  XYXY...XY XYXY...XY XYXY...XY XYXY...XY
      1 sdo_ss_map: 2,3  XYXY...XY XYXY...XY XYXY...XY XYXY...XY
      2 sdo_ss_map: 4,5  XYXY...XY XYXY...XY XYXY...XY XYXY...XY
      3 sdo_ss_map: 6,7  XYXY...XY XYXY...XY XYXY...XY XYXY...XY
                         <--LF0--> <--LF1--> <--LF2--> <--LF3-->
     
     Fill the sdo_ss_map with zeros. Use the same sdo_ss_map:
     . in both pages
     . for all AP
     . and for all banks
     
     The subband frequency bins from the PFB arrive alternately in series so
     in total 512 XY bins per slice. The X bins have bin_x = 2 * bin and the
     Y bins have index bin_y = bin * 2 + 1.
     The sdo_ss_map has 2 pol * 36 subbands = 72 words. The SDO_SS outputs 72
     subbands. To select the CW bin at arg_sdo_subband (0..35) for both SP-X
     and SP-Y as in the table above use:
  
       cw_bin_x = cw_bin*c_pol
       cw_bin_y = cw_bin*c_pol + 1
       sdo_ss_x = sdo_subband * c_pol
       sdo_ss_y = sdo_subband * c_pol + 1
  
     In the table above the subbands are selected alternately from both
     polarizations by filling in a bin_x and a bin_y in pairs, but this is
     not mandatory.
  
     The subbands 0:8 from SP0:7 get output via lane 0 of the BP serdes
     interface, similar subbands 9:17 via lane 1, subbands 18:26 via lane 2
     and subbands 27:35 via lane 3. The subband order in an lane frame (LF)
     is for LF0:
     
          SP hhh 01234567 01234567 ... 01234567 t
     subband --- 00000000 11111111 ... 88888888 -
     
     Similar for LF1, LF2 and LF3. The 'hhh' mark three DP packet header words
     and the 't' marks the DP packet tail word.
  
"""

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
