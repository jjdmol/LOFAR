# RCU modem led check version V12
# This version disable the DC from the HBA connector
# led on VDC=3.3V, led off VDC=0.04V
# note communication with the TILES only with LED ON !!! 
# M.J. Norden, 31-08-2011

for ((run = 1; run < 200; run++)) do
  rspctl --hbadelays=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
  sleep 2
done







