"""SERDES PHY MMD access constants and functions (translated from RSP mmd.tcl)

   Procedures for accessing the SERDES PHY chip via MMD:
   
   - writeReg
   - readReg
   - incReadReg
   - logRange
"""

################################################################################
# System imports

# User imports
import rsp


###############################################################################
# - Serdes MMD functions

def writeReg (tc, msg, reg, dat, rspId=['rsp0'], prtad=0, devad=5):
  """Write word to register in SERDES PHY chip

  Input:
  - tc     = Testcase
  - msg    = MepMessage
  - reg    = Register address
  - dat    = Data word to write
  - rspId  = RSP ID: 'rsp#'
  - prtad  = Port address
  - devad  = Device address

  Return: void
  """
  st =         0
  op = {'ad' : 0,
        'wr' : 1,
        'rd' : 3}
  ta = {'wr' : 2}

  # Register address.
  for ri in rspId:
    msg.packAddr(['rsp'], 'serdes', 'hdr')
    hdr = round(st*pow(2,14) + op['ad']*pow(2,12) + prtad*pow(2,7) + devad*pow(2,2) + ta['wr'])
    msg.packPayload([hdr],2)
    rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (ri[3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()
  
  for ri in rspId:
    msg.packAddr(['rsp'], 'serdes', 'data')
    msg.packPayload([reg],2)
    rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (ri[3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()
  
  # Data.
  for ri in rspId:
    msg.packAddr(['rsp'], 'serdes', 'hdr')
    hdr = round(st*pow(2,14) + op['wr']*pow(2,12) + prtad*pow(2,7) + devad*pow(2,2) + ta['wr'])
    msg.packPayload([hdr],2)
    rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (ri[3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()
  
  for ri in rspId:
    msg.packAddr(['rsp'], 'serdes', 'data')
    msg.packPayload([dat],2)
    rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (ri[3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()


def readReg (tc, msg, reg, rspId=['rsp0'], prtad=0, devad=5):
  """Read word from register in SERDES PHY chip

  Input:
  - tc     = Testcase
  - msg    = MepMessage
  - reg    = Register address
  - rspId  = RSP ID: 'rsp#' - only one
  - prtad  = Port address
  - devad  = Device address

  Return:
  - dat    = Read word
  """
  st =         0
  op = {'ad' : 0,
        'wr' : 1,
        'rd' : 3}
  ta = {'wr' : 2,
        'rd' : 0}

  # Register address.
  msg.packAddr(['rsp'], 'serdes', 'hdr')
  hdr = round(st*pow(2,14) + op['ad']*pow(2,12) + prtad*pow(2,7) + devad*pow(2,2) + ta['wr'])
  msg.packPayload([hdr],2)
  rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (rspId[0][3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()
    
  msg.packAddr(['rsp'], 'serdes', 'data')
  msg.packPayload([reg],2)
  rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (rspId[0][3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()

  # Data.
  msg.packAddr(['rsp'], 'serdes', 'hdr')
  hdr = round(st*pow(2,14) + op['rd']*pow(2,12) + prtad*pow(2,7) + devad*pow(2,2) + ta['rd'])
  msg.packPayload([hdr],2)
  rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (rspId[0][3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()

  msg.packAddr(['rsp'], 'serdes', 'data')  
  readData = rsp.rspctl(tc, '--readblock=%s,%s,0,2' % (rspId[0][3:], msg.hexAddr))
  msg.extractPayload(readData)
  dat = msg.unpackPayload(2, '+')
  return dat[0]

  
def incReadReg (tc, msg, rspId=['rsp0'], prtad=0, devad=5):
  """Read word from current register in PHY chip and post increment register address

  Input:
  - tc     = Testcase
  - msg    = MepMessage
  - rspId  = RSP ID: 'rsp#' - only one
  - prtad  = Port address
  - devad  = Device address

  Return:
  - dat    = Read word
  """
  st =         0
  op = {'ad' : 0,
        'wr' : 1,
        'rd' : 3,
        'ird': 2}
  ta = {'rd' : 0}
  
  # Data.
  msg.packAddr(['rsp'], 'serdes', 'hdr')
  hdr = round(st*pow(2,14) + op['ird']*pow(2,12) + prtad*pow(2,7) + devad*pow(2,2) + ta['rd'])
  msg.packPayload([hdr],2)
  rsp.rspctl(tc, '--writeblock=%s,%s,0,%s' % (rspId[0][3:], msg.hexAddr, msg.hexPayload))
  rsp.rspctl_write_sleep()

  msg.packAddr(['rsp'], 'serdes', 'data')  
  readData = rsp.rspctl(tc, '--readblock=%s,%s,0,2' % (rspId[0][3:], msg.hexAddr))
  msg.extractPayload(readData)
  dat = msg.unpackPayload(2, '+')
  return dat[0]

  
def logRange(tc, msg, reg_begin, reg_end, rspId=['rsp0'], repeat=1):
  """Read range of registers from SERDES PHY chip

  Input:
  - tc        = Testcase
  - msg       = MepMessage
  - reg_begin = Register range begin address
  - reg_end   = Register range end address
  - rspId     = RSP ID: 'rsp#'

  Return: void.
    tc appendlog messages are reported.
  """
  for reg in range(reg_begin, reg_end+1, 1):
    for ri in rspId:
      for rep in range(1,1+repeat):
        dat = readReg(tc, msg, reg, [ri])
        tc.appendLog(11, '%s : Register 0x%04X = 0x%04X' % (ri, reg, dat))
