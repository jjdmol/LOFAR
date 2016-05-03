""" Serdes control clock delay, based on TCL testcase 8.5

   One idelay step ~= 78 ps @ 200MHz and 98 ps @160 MHz. However the rx_clk and
   tx_clk run at 125 MHz so the serdes PHY - BP interface delay setting must be
   selected such that it suits both RSP system clock speeds.
   
   Usage:
   . Reset     rx_clk input delay                   : python verify.py --brd rsp0 -v 21 --te tc/serdes_delay.py --interface rx --data 0
   . Increment rx_clk input delay by 4 idelay steps : python verify.py --brd rsp0 -v 21 --te tc/serdes_delay.py --interface rx --data 4
   . Read      rx_clk input delay status            : python verify.py --brd rsp0 -v 21 --te tc/serdes_delay.py --interface rx --read
   
   Idem for tx interface, but the firmware does not support variable tx_clk
   output delay control. Hence effectively the tx interface timing can not be 
   changed by the LCU. The tx_clk output delay can only be set at synthesis.
   In practise this is no problem because the default tx interface timing is 
   accurately aligned by using a DDIO element for both clk and d,c,h data
   lines without any need to offset delay the tx_clk.
"""

################################################################################
# - Verify options
rspId = tc.rspId

# Testcase specific options
clk_interface = arg_interface
clk_delay     = arg_data[0]

tc.appendLog(11, '')
if arg_read:
    # Read
    if clk_interface=='rx':
        tc.appendLog(11, '>>> Read RX_CLK input delay status for RSP-%s.' % rspId)
        for ri in rspId:
            rsp.read_serdes_rx_delay(tc, msg, [ri])
    else:
        tc.appendLog(11, '>>> Read TX_CLK output delay status for RSP-%s.' % rspId)
        for ri in rspId:
            rsp.read_serdes_tx_delay(tc, msg, [ri])
else:
    # Write
    if clk_delay==0:
        # Reset delay
        if clk_interface=='rx':
            tc.appendLog(11, '>>> RSP-%s: Reset RX_CLK input delay to default.' % rspId)
            rsp.write_serdes_rx_delay(tc, msg, 0, rspId)
        else:
            tc.appendLog(11, '>>> RSP-%s: Reset TX_CLK output delay to default.' % rspId)
            rsp.write_serdes_tx_delay(tc, msg, 0, rspId)
    else:
        # Increment delay
        if clk_interface=='rx':
            tc.appendLog(11, '>>> RSP-%s: Increment RX_CLK input delay %d times.' % (rspId, clk_delay))
            for ri in range(clk_delay):
                rsp.write_serdes_rx_delay(tc, msg, 1, rspId)
        else:
            tc.appendLog(11, '>>> RSP-%s: Increment TX_CLK output delay %d times.' % (rspId, clk_delay))
            for ri in range(clk_delay):
                rsp.write_serdes_tx_delay(tc, msg, 1, rspId)
tc.appendLog(11, '')
