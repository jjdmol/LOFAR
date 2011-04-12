"""SMBus (I2C) access constants and functions (translated from RSP smbus.tcl)

   Constants for I2C peripherals

   - c_max6652 voltage temperature sensor
   - c_pca9555 I/O expander

   Procedures for accessing the SMBus (I2C) interface

   - set_protocol
   - write_protocol_list
   - readback_protocol_list
   - read_results
   - overwrite_results
"""

################################################################################
# System imports

# User imports
import rsp

###############################################################################
# - MAX6652 voltage temperature sensor constants

c_max6652_addr_gnd             = 0x14   # I2C slave address
c_max6652_addr_vcc             = 0x15
c_max6652_addr_sda             = 0x16
c_max6652_addr_scl             = 0x17

c_max6652_cmd_read_2v5         = 0x20   # Commands
c_max6652_cmd_read_12v         = 0x21
c_max6652_cmd_read_3v3         = 0x22
c_max6652_cmd_read_vcc         = 0x23
c_max6652_cmd_read_temp        = 0x27
c_max6652_cmd_config           = 0x40

c_max6652_config_start         = 0x01   # Configuration bits
c_max6652_config_int_en        = 0x02
c_max6652_config_int_clr       = 0x08
c_max6652_config_line_freq_sel = 0x10
c_max6652_config_short_cycle   = 0x20
c_max6652_config_reset         = 0x80

c_max6652_unit_12v             = 12.0/192   # Unit in V
c_max6652_unit_vcc             =  5.0/192
c_max6652_unit_3v3             =  3.3/192
c_max6652_unit_2v5             =  2.5/192
c_max6652_unit_temp            =  1.0       # Unit in degree C


###############################################################################
# - PCA9555 I/O expander constants

c_pca9555_base_addr            = 0x20
c_pca9555_addr_000             = c_pca9555_base_addr + 0   # A2 A1 A0
c_pca9555_addr_001             = c_pca9555_base_addr + 1
c_pca9555_addr_010             = c_pca9555_base_addr + 2
c_pca9555_addr_011             = c_pca9555_base_addr + 3
c_pca9555_addr_100             = c_pca9555_base_addr + 4
c_pca9555_addr_101             = c_pca9555_base_addr + 5
c_pca9555_addr_110             = c_pca9555_base_addr + 6
c_pca9555_addr_111             = c_pca9555_base_addr + 7

c_pca9555_cmd_input_0          = 0
c_pca9555_cmd_input_1          = 1
c_pca9555_cmd_output_0         = 2
c_pca9555_cmd_output_1         = 3
c_pca9555_cmd_invert_0         = 4
c_pca9555_cmd_invert_1         = 5
c_pca9555_cmd_config_0         = 6
c_pca9555_cmd_config_1         = 7


###############################################################################
# - SMBus functions

def set_protocol(tc, protocol_id, cnt=1, addr=1, data='', cmd='', cmd2='', appLev=1):
  """Returns filled in protocol message

  See i2c_smbus(pkg).vhd for more detailed comments on the SMBus protocol definitions.

  Input:
  - tc             = Testcase
  - protocol_id    = Protocol ID: STRING
  - cnt            = Number of data bytes, or timeout value: INTEGER
  - addr           = I2C slave address: BYTE
  - data           = I2C write data: list of BYTE
  - cmd            = I2C slave command (register)
  - cmd2           = I2C slave command (register) 2

  Return:
  - msg            = Protocol request message
  """

  def error_len(data, le):
    ret = 0
    if len(data)!=le:
      print 'Wrong SMBus protocol data length, must be %d bytes.' % le
      ret = 1
    return ret

  # SMBUS protocol identifiers conform i2c_smbus(pkg).vhd
  PROTOCOL_ARRAY = {'PROTOCOL_WRITE_QUICK'           :2,
                    'PROTOCOL_READ_QUICK'            :3,
                    'PROTOCOL_SEND_BYTE'             :4,
                    'PROTOCOL_RECEIVE_BYTE'          :5,
                    'PROTOCOL_WRITE_BYTE'            :6,
                    'PROTOCOL_READ_BYTE'             :7,
                    'PROTOCOL_WRITE_WORD'            :8,
                    'PROTOCOL_READ_WORD'             :9,
                    'PROTOCOL_WRITE_BLOCK'           :10,
                    'PROTOCOL_READ_BLOCK'            :11,
                    'PROTOCOL_PROCESS_CALL'          :12,
                    'PROTOCOL_C_WRITE_BLOCK_NO_CNT'  :13,
                    'PROTOCOL_C_READ_BLOCK_NO_CNT'   :14,
                    'PROTOCOL_C_SEND_BLOCK'          :15,
                    'PROTOCOL_C_RECEIVE_BLOCK'       :16,
                    'PROTOCOL_C_NOP'                 :17,
                    'PROTOCOL_C_WAIT'                :18,
                    'PROTOCOL_C_END'                 :19,
                    'PROTOCOL_C_UNKNOWN'             :20}    # To test unknown protocol

  # First message field
  msg = []
  msg.append(PROTOCOL_ARRAY[protocol_id])

  # Additional message fields:
  if   protocol_id == 'PROTOCOL_WRITE_QUICK':          msg.append(addr)
  elif protocol_id == 'PROTOCOL_READ_QUICK':           msg.append(addr)
  elif protocol_id == 'PROTOCOL_SEND_BYTE':
                                                       msg.append(addr)
                                                       msg.extend(data)
                                                       if error_len(data,1)!=0: msg = -1
  elif protocol_id == 'PROTOCOL_RECEIVE_BYTE':           msg.append(addr)
  elif protocol_id == 'PROTOCOL_WRITE_BYTE':
                                                       msg.append(addr)
                                                       msg.append(cmd)
						       msg.extend(data)
						       if error_len(data,1)!=0: msg = -1
  elif protocol_id == 'PROTOCOL_READ_BYTE':              msg.append(addr); msg.append(cmd)
  elif protocol_id == 'PROTOCOL_WRITE_WORD':
                                                       msg.append(addr)
                                                       msg.append(cmd)
                                                       msg.extend(data)
						       if error_len(data,2)!=0: msg = -1
  elif protocol_id == 'PROTOCOL_READ_WORD':            msg.append(addr); msg.append(cmd)
  elif protocol_id == 'PROTOCOL_WRITE_BLOCK':          msg.append(addr); msg.append(cmd); msg.append(cnt); msg.extend(data)
  elif protocol_id == 'PROTOCOL_READ_BLOCK':           msg.append(addr); msg.append(cmd); msg.append(cnt)
  elif protocol_id == 'PROTOCOL_PROCESS_CALL':
                                                       msg.append(addr)
						       msg.append(cmd)
						       msg.extend(data)
						       if error_len(data,2)!=0: msg = -1
                                                       msg.append(addr); msg.append(cmd2)
  elif protocol_id == 'PROTOCOL_C_WRITE_BLOCK_NO_CNT': msg.append(addr); msg.append(cmd); msg.append(cnt); msg.extend(data)
  elif protocol_id == 'PROTOCOL_C_READ_BLOCK_NO_CNT':  msg.append(addr); msg.append(cmd); msg.append(cnt)
  elif protocol_id == 'PROTOCOL_C_SEND_BLOCK':         msg.append(addr);                  msg.append(cnt); msg.extend(data)
  elif protocol_id == 'PROTOCOL_C_RECEIVE_BLOCK':      msg.append(addr);                  msg.append(cnt)
  elif protocol_id == 'PROTOCOL_C_NOP':                None
  elif protocol_id == 'PROTOCOL_C_WAIT':               msg.extend(rsp.i2bbbb([cnt]))
  elif protocol_id == 'PROTOCOL_C_END':                None
  else:                                                tc.appendLog(appLev, 'Unknown SMBus protocol.')

  return msg


def test_protocols(tc):
  """Procedure used to verify set_protocol in a Python shell
  """
  print set_protocol(tc, 'PROTOCOL_WRITE_QUICK',          None, 1, None,      None, None)
  print set_protocol(tc, 'PROTOCOL_READ_QUICK',           None, 1, None,      None, None)
  print set_protocol(tc, 'PROTOCOL_SEND_BYTE',            None, 1, [5],       None, None)
  print set_protocol(tc, 'PROTOCOL_RECEIVE_BYTE',         None, 1, None,      None, None)
  print set_protocol(tc, 'PROTOCOL_WRITE_BYTE',           None, 1, [5],       17,   None)
  print set_protocol(tc, 'PROTOCOL_READ_BYTE',            None, 1, None,      17,   None)
  print set_protocol(tc, 'PROTOCOL_WRITE_WORD',           None, 1, [5, 6],    17,   None)
  print set_protocol(tc, 'PROTOCOL_READ_WORD',            None, 1, None,      17,   None)
  print set_protocol(tc, 'PROTOCOL_WRITE_BLOCK',          3,    1, [9, 9, 9], 17,   None)
  print set_protocol(tc, 'PROTOCOL_READ_BLOCK',           3,    1, None,      17,   None)
  print set_protocol(tc, 'PROTOCOL_PROCESS_CALL',         None, 1, [5, 6],    17,   18)
  print set_protocol(tc, 'PROTOCOL_C_WRITE_BLOCK_NO_CNT', 3,    1, [9, 9, 9], 17,   None)
  print set_protocol(tc, 'PROTOCOL_C_READ_BLOCK_NO_CNT',  3,    1, None,      17,   None)
  print set_protocol(tc, 'PROTOCOL_C_SEND_BLOCK',         3,    1, [9, 9, 9], None, None)
  print set_protocol(tc, 'PROTOCOL_C_RECEIVE_BLOCK',      3,    1, None,      None, None)
  print set_protocol(tc, 'PROTOCOL_C_NOP',                None, 1, None,      None, None)
  print set_protocol(tc, 'PROTOCOL_C_WAIT',               1333, 1, None,      None, None)
  print set_protocol(tc, 'PROTOCOL_C_END',                None, 1, None,      None, None)
  set_protocol(      tc, 'PROTOCOL_C_UNKNOWN',            None, 1, None,      None, None)


def write_protocol_list(tc, msg, smbh, protocol_list, polId=['x', 'y'], blpId=['blp0'], rspId=['rsp0']):
  """Write SMBus protocol list to SMBus handler
  
  Input:
  - tc             = Testcase
  - msg            = MepMessage
  - smbh           = I2C device handler: 'rcuh' or 'tdsh'
  - protocol_list  = Protocol list: bytes
  - polId          = Polarization: 'x' or 'y' for RCUH, ignored for TDSH
  - blpId          = BLP ID: 'blp#' for RCUH, destination defaults to 'rsp' for TDSH
  - rspId          = RSP ID: 'rsp#'
 
  Return: void
  """
  width  = 1   # protocol_list is in bytes
  offset = 0   # access from address 0
  bc     = 1   # allow BLP broadcast
  if smbh == 'tdsh':
    rsp.write_mem(tc, msg, smbh, 'protocol', protocol_list, ['rsp'], rspId, width, offset, bc)
  if smbh == 'rcuh':
    for pi in polId:
      rsp.write_mem(tc, msg, smbh, 'protocol'+pi, protocol_list, blpId, rspId, width, offset, bc)


def readback_protocol_list(tc, msg, smbh, le, polId='x', blpId=['blp0'], rspId=['rsp0']):
  """Read back SMBus protocol list from SMBus handler
  
  Input:
  - tc             = Testcase
  - msg            = MepMessage
  - smbh           = I2C device handler: 'rcuh' or 'tdsh'
  - le             = Number of bytes to read from the protocol list buffer, INTEGER
  - polId          = Polarization ID: 'x' or 'y' for RCUH, ignored for TDSH          - only one
  - blpId          = BLP ID: 'blp#' for RCUH, destination defaults to 'rsp' for TDSH - only one
  - rspId          = RSP ID: 'rsp#'                                                  - only one

  Return:            Read protocol list bytes
  """
  width  = 1   # protocol_list is in bytes
  offset = 0   # access from address 0
  if smbh == 'tdsh':
    return rsp.read_mem(tc, msg, smbh, 'protocol',       width * le, ['rsp'], rspId, '+', width, offset)
  if smbh == 'rcuh':
    return rsp.read_mem(tc, msg, smbh, 'protocol'+polId, width * le, blpId,   rspId, '+', width, offset)
  

def read_results (tc, msg, smbh, le, polId='x', blpId=['blp0'], rspId=['rsp0']):
  """Read the results of the SMBus protocol list from the SMBus handler
 
  Input:
  - tc             = Testcase
  - msg            = MepMessage
  - smbh           = I2C device handler: 'rcuh' or 'tdsh'
  - le             = Number of bytes to read from the protocol result buffer, INTEGER
  - polId          = Polarization ID: 'x' or 'y' for RCUH, ignored for TDSH          - only one
  - blpId          = BLP ID: 'blp#' for RCUH, destination defaults to 'rsp' for TDSH - only one
  - rspId          = RSP ID: 'rsp#'                                                  - only one
  
  Return:          = Read result data
  """
  width  = 1   # protocol_list is in bytes
  offset = 0   # access from address 0
  if smbh == 'tdsh':
    return rsp.read_mem(tc, msg, smbh, 'result',       width * le, ['rsp'], rspId, '+', width, offset)
  if smbh == 'rcuh':
    return rsp.read_mem(tc, msg, smbh, 'result'+polId, width * le, blpId,   rspId, '+', width, offset)


def overwrite_results (tc, msg, smbh, wr_result, polId=['x', 'y'], blpId=['blp0'], rspId=['rsp0']):
  """Write SMBus results to SMBus handler
  
  Input:
  - tc             = Testcase
  - msg            = MepMessage
  - smbh           = I2C device handler: 'rcuh' or 'tdsh'
  - wr_result      = Overwrite results: bytes
  - polId          = Polarization: 'x' or 'y' for RCUH, ignored for TDSH
  - blpId          = BLP ID: 'blp#' for RCUH, destination defaults to 'rsp' for TDSH
  - rspId          = RSP ID: 'rsp#'
  
  Return: void
  """
  width  = 1   # result is in bytes
  offset = 0   # access from address 0
  bc     = 1   # allow BLP broadcast
  if smbh == 'tdsh':
    rsp.write_mem(tc, msg, smbh, 'result', wr_result, ['rsp'], rspId, width, offset, bc)
  if smbh == 'rcuh':
    for pi in polId:
      rsp.write_mem(tc, msg, smbh, 'result'+pi, wr_result, blpId, rspId, width, offset, bc)
