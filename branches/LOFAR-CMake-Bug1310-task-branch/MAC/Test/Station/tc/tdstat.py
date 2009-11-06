"""TDS monitor voltages and temperature via I2C, based on TCL testcase 9.1"""

################################################################################
# - Verify options
rspId = tc.rspId    # allow multiple rsp, e.g. rsp0,rsp4 to access TDS in two subracks
repeat = tc.repeat  # use >1 for continuous T,V monitoring or I2C stress test

tc.setResult('PASSED')

tc.appendLog(11,'')
tc.appendLog(11,'>>> Monitor T and V of the TDS board using an I2C access via RSP-%s.' % rspId)
tc.appendLog(11,'')

# Handle older versions of the TDS (Timing Distribution Subrack) clock board.
td_version = 2
#td_version = 3
if td_version == 2:
  v5 = 0
if td_version == 3:
  v5 = 5

################################################################################
# - Testcase initializations

# - Disable external sync to avoid asynchronous trigger of TDSH protocol list
rsp.write_cr_syncoff(tc, msg, ['rsp'], rspId)


################################################################################
# Read TD sensor status via I2C

addr   = smbus.c_max6652_addr_gnd
config = smbus.c_max6652_config_start + smbus.c_max6652_config_line_freq_sel
protocol_list = []
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_WRITE_BYTE', None, addr, [config], smbus.c_max6652_cmd_config))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_config))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_2v5))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_vcc))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_temp))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_END'))
exp_result = '0 %d 0 ? 0 ? 0 ? 0 0' % config
len_result = len(exp_result.split())


for rep in range(1,1+repeat):
  for ri in rspId:
    # - Overwrite and read the protocol results from the TDSH
    rsp.overwrite_rd_smbh_protocol_results(tc, msg, 'tdsh', None, None, [ri])
    
    # - Write (and readback) the protocol list to the TDSH
    rsp.write_rd_smbh_protocol_list(tc, msg, 'tdsh', protocol_list, None, None, [ri])

    # Apply altsync to start the TDSH SMBus protocols
    rsp.write_rsu_altsync(tc, msg, [ri])
    tc.sleep(100)

    # Read the protocol results from the TDSH
    rd_result = smbus.read_results(tc, msg, 'tdsh', len_result, None, None, [ri])

    # Sensor values  
    volt_2v5 = rd_result[3]
    volt_vcc = rd_result[5]
    temp     = rd_result[7]

    # TDS voltages
    tds_5v  = volt_2v5 * smbus.c_max6652_unit_2v5 * (10+10)/10
    tds_3v3 = volt_vcc * smbus.c_max6652_unit_vcc               # note vcc unit is based on 5v, independent of true VCC

    tc.appendLog(11, '')
    tc.appendLog(11, '>>> TDS voltage and temperature measurements:')
    tc.appendLog(11, '')
    tc.appendLog(11, '    5V via 2v5 input = %6.3f V  (= %4d * %7.4f mV)' % (tds_5v,  volt_2v5, 1000*smbus.c_max6652_unit_2v5 * (10+10)/10))
    tc.appendLog(11, '  3.3V via vcc input = %6.3f V  (= %4d * %7.4f mV)' % (tds_3v3, volt_vcc, 1000*smbus.c_max6652_unit_vcc))
    tc.appendLog(11, '  Temp               = %4d degrees C' % temp)
    tc.appendLog(11, '')

    if rd_result[0] == 0        and \
       rd_result[1] == config   and \
       rd_result[2] == 0        and \
       tds_5v        > v5-1.0 and tds_5v  < v5+0.5 and \
       rd_result[4] == 0      and \
       tds_3v3       > 3.0    and tds_3v3 < 4.0    and \
       rd_result[6] == 0      and \
       temp            >  0   and temp    < 70     and \
       rd_result[8] == 0      and \
       rd_result[9] == 0:
      tc.appendLog(11, '>>> Rep-%d, RSP-%s, I2C access to the MAX6652 T,V sensor on the TDS board went OK' % (rep, ri))
    else:
      tc.appendLog(11, '>>> Rep-%d, RSP-%s, I2C access to the MAX6652 T,V sensor on the TDS board went wrong:' % (rep, ri))
      tc.appendLog(11, 'Expected protocol result: %s' % exp_result)
      tc.appendLog(11, 'Read     protocol result: %s   (slave address 0x%x)' % (rd_result, addr))
      tc.setResult('FAILED')

# - Enable external sync
rsp.write_cr_syncon(tc, msg, ['rsp'], rspId)
