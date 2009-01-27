"""SPU monitor voltages and temperature via I2C, based on TCL testcase 9.6"""

################################################################################
# - Verify options
rspId = tc.rspId    # allow multiple rsp, e.g. rsp0,rsp4 to access SPU in two subracks
repeat = tc.repeat  # use >1 for continuous T,V monitoring or I2C stress test

tc.setResult('PASSED')


tc.appendLog(11,'')
tc.appendLog(11,'>>> Monitor T and V of the SPU board using an I2C access via RSP-%s.' % rspId)
tc.appendLog(11,'')

# Handle older versions of the SPU (Subrack Power Unit).
spu_version = 2
#spu_version = 3
if spu_version == 2:
  v48 = 8
if spu_version == 3:
  v48 = 48

################################################################################
# - Testcase initializations

# - Disable external sync to avoid asynchronous trigger of TDSH protocol list
rsp.write_cr_syncoff(tc, msg, ['rsp'], rspId)


################################################################################
# Read SPU sensor status via I2C

addr   = smbus.c_max6652_addr_vcc
config = smbus.c_max6652_config_start + smbus.c_max6652_config_line_freq_sel
protocol_list = []
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_WRITE_BYTE', None, addr, [config], smbus.c_max6652_cmd_config))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_2v5))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_3v3))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_12v))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_vcc))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_READ_BYTE',  None, addr, None,     smbus.c_max6652_cmd_read_temp))
protocol_list.extend(smbus.set_protocol(tc, 'PROTOCOL_C_END'))
exp_result = '0 ? 0 ? 0 ? 0 ? 0 ? 0 0'
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
    spu_volt_2v5 = rd_result[1]
    spu_volt_3v3 = rd_result[3]
    spu_volt_12v = rd_result[5]
    spu_volt_vcc = rd_result[7]
    spu_temp     = rd_result[9]
    
    # SPU voltages
    v_rcu     = spu_volt_2v5 * smbus.c_max6652_unit_2v5 * (10+10)/10
    v_lba_rcu = spu_volt_3v3 * smbus.c_max6652_unit_3v3 * (10+20)/10
    v_hba     = spu_volt_12v * smbus.c_max6652_unit_12v * (10+30.1)/10
    v_monitor = spu_volt_vcc * smbus.c_max6652_unit_vcc
  
    tc.appendLog(11, '')
    tc.appendLog(11, 'SPU voltage and temperature measurements:')
    tc.appendLog(11, '')
    tc.appendLog(11, '  V_RCU     = %6.3f V   (= %4d * %7.1f mV)' % (v_rcu,     spu_volt_2v5, 1000 * smbus.c_max6652_unit_2v5 * (10+10)/10))
    tc.appendLog(11, '  V_LBA_RCU = %6.3f V   (= %4d * %7.1f mV)' % (v_lba_rcu, spu_volt_3v3, 1000 * smbus.c_max6652_unit_3v3 * (10+20)/10))
    tc.appendLog(11, '  V_HBA     = %6.3f V   (= %4d * %7.1f mV)' % (v_hba,     spu_volt_12v, 1000 * smbus.c_max6652_unit_12v * (10+30.1)/10))
    tc.appendLog(11, '')
    tc.appendLog(11, '  V_monitor = %6.3f V   (= %4d * %7.1f mV)    (This is the supply voltage of the sensor itself)' \
                                                                  % (v_monitor, spu_volt_vcc, 1000 * smbus.c_max6652_unit_vcc))
    tc.appendLog(11, '')
    tc.appendLog(11, '  Temp      = %4d degrees C' % spu_temp)
    tc.appendLog(11, '')
  
    if rd_result[ 0] == 0 and v_rcu     >  4.0  and v_rcu     < 5.5   and \
       rd_result[ 2] == 0 and v_lba_rcu >  7    and v_lba_rcu < 9     and \
       rd_result[ 4] == 0 and v_hba     > v48-5 and v_hba     < v48+5 and \
       rd_result[ 6] == 0 and v_monitor >  3    and v_monitor < 4     and \
       rd_result[ 8] == 0 and spu_temp  >  0    and spu_temp  < 70    and \
       rd_result[10] == 0:
      tc.appendLog(11, '>>> Rep-%d, RSP-%s, I2C access to the MAX6652 T,V sensor on the SPU board went OK' % (rep, ri))
    else:
      tc.appendLog(11, '>>> Rep-%d, RSP-%s, I2C access to the MAX6652 T,V sensor on the SPU board went wrong:' % (rep, ri))
      tc.appendLog(11, 'Expected protocol result: %s' % exp_result)
      tc.appendLog(11, 'Read     protocol result: %s   (slave address 0x%x)' % (rd_result, addr))
      tc.setResult('FAILED')

# - Enable external sync
rsp.write_cr_syncon(tc, msg, ['rsp'], rspId)
