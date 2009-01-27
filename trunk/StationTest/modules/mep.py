"""MEP interface for RSP board access via rspctl --readblock/--writeblock

  See also MepMessage.tcl.

  class MepMessage:
    def swap2(self, h)
    def swap3(self, h)
    def swap4(self, h)
    def packAddr(self, blpId, pid, regId)
    def clearPayload(self)
    def appendPayload(self, hexString)
    def lenPayload(self)
    def getPayload(self, offset, nof)
    def packPayload(self, data, width)
    def extractPayload(self, readData)
    def unpackPayload(self, width, sign='+')
    def setOffset(self, offset=0)
    def incrOffset(self, offset=1)
    def readUnsigned(self, width):
    def readSigned(self, width):
"""

class MepMessage:
  c_blpid = {'blp0' : 1,
             'blp1' : 2,
             'blp2' : 4,
             'blp3' : 8}
             
  c_pid = {'rsr'   :1,
           'rsu'   :2,
           'diag'  :3,
           'ss'    :4,
           'bf'    :5,
           'bst'   :6,
           'sst'   :7,
           'rcuh'  :8,
           'cr'    :9,
           'xst'   :10,
           'cdo'   :11,
           'bs'    :12,
           'serdes':13,
           'tdsh'  :14,
           'tbbi'  :15,
           'cep'   :16,
           'lcu'   :17,
           'rad'   :18}
   
  c_regid = {('rsr','status') :     0,
             ('rsr','version') :    1,
             ('rsr','timestamp') :  2,
             ('rsu','flash') :      1,
             ('rsu','erase') :      2,
             ('rsu','reconfig') :   3,
             ('rsu','sysctrl') :    4,
             ('rsu','unprotect') :  5,
             ('tdsh','protocol') :  0,
             ('tdsh','result') :    1,
             ('diag','setx') :      0,
             ('diag','sety') :      1,
             ('diag','wavex') :     2,
             ('diag','wavey') :     3,
             ('diag','bypass') :    4,
             ('diag','result') :    5,
             ('diag','selftest') :  6,
             ('bs','psync') :       0,
             ('ss','settings') :    0,
             ('bf','coefxr') :      0,
             ('bf','coefxi') :      1,
             ('bf','coefyr') :      2,
             ('bf','coefyi') :      3,
             ('sst','power') :      0,
             ('bst','power0') :     0,
             ('bst','power1') :     1,
             ('bst','power2') :     2,
             ('bst','power3') :     3,
             ('xst','r0') :         0,
             ('xst','r1') :         1,
             ('xst','r2') :         2,
             ('xst','r3') :         3,
             ('xst','r4') :         4,
             ('xst','r5') :         5,
             ('xst','r6') :         6,
             ('xst','r7') :         7,
             ('xst','r8') :         8,
             ('xst','r9') :         9,
             ('xst','r10') :        10,
             ('xst','r11') :        11,
             ('xst','r12') :        12,
             ('xst','r13') :        13,
             ('xst','r14') :        14,
             ('xst','r15') :        15,
             ('xst','r16') :        16,
             ('xst','r17') :        17,
             ('xst','r18') :        18,
             ('xst','r19') :        19,
             ('xst','r20') :        20,
             ('xst','r21') :        21,
             ('xst','r22') :        22,
             ('xst','r23') :        23,
             ('xst','r24') :        24,
             ('xst','r25') :        25,
             ('xst','r26') :        26,
             ('xst','r27') :        27,
             ('xst','r28') :        28,
             ('xst','r29') :        29,
             ('xst','r30') :        30,
             ('xst','r31') :        31,
             ('rcuh','settings') :  0,
             ('rcuh','protocolx') : 1,
             ('rcuh','resultx') :   2,
             ('rcuh','protocoly') : 3,
             ('rcuh','resulty') :   4,
             ('cr','softrst') :     0,
             ('cr','softsync') :    1,
             ('cr','syncoff') :     2,
             ('cr','syncdelay') :   3,
             ('cdo','settings') :   0,
             ('cdo','iphdr') :      1,
             ('serdes','hdr') :     0,
             ('serdes','data') :    1,
             ('cep','hdr') :        0,
             ('cep','data') :       1,
             ('lcu','hdr') :        0,
             ('lcu','data') :       1,
             ('rad','settings') :   0,
             ('rad','latency') :    1,
             ('tbbi','settingsx') : 0,
             ('tbbi','settingsy') : 1,
             ('tbbi','selmemx') :   2,
             ('tbbi','selmemy') :   3}
     
  c_hw = 2         # 2 hex digits per byte

  hexAddr = ''     # rspctl --readblock/writeblock ADDR field as string of hex digits
  hexPayload = ''  # payload string of hex digits
  offset = 0       # offset for read hexPayload in nof bytes
     
  def swap2(self, h):
    """Byte swap hex string of two bytes
    """
    return h[2:4] + h[0:2]
    
  def swap3(self, h):
    """Byte swap hex string of three bytes
    """
    return h[4:6] + h[2:4] + h[0:2]
    
  def swap4(self, h):
    """Byte swap hex string of four bytes
    """
    return h[6:8] + h[4:6] + h[2:4] + h[0:2]
    
  def packAddr(self, blpId, pid, regId):
    """Construct string of hex digits for rspctl --readblock/writeblock ADDR field
    
    Input:
    - blpId = list of 'rsp' for BP and/or one or more 'blp#' for AP# to address
    - pid   = process ID
    - regId = register ID of the process
    Output:
    - self.hexAddr
    Return: void
    Example:
      p=mep.MepMessage()
      p.packAddr(['rsp', 'blp0', 'blp1'], 'diag', 'bypass')
    """
    ap = 0
    bp = 0
    for i in blpId:
      if i=='rsp':
        bp = 1
      else:
        ap += self.c_blpid[i]
    self.hexAddr = '%02x%02x%02x%02x' % (ap, bp, self.c_pid[pid], self.c_regid[(pid,regId)])
    
  def clearPayload(self):
    """Clear hex payload string
    """
    self.hexPayload = ''
  
  def appendPayload(self, hexString):
    """Append hex string to the hex payload string
    """
    self.hexPayload += hexString
    
  def lenPayload(self):
    """Return length of the hex payload string in nof bytes
    """
    return len(self.hexPayload)/self.c_hw
    
  def getPayload(self, offset, nof):
    """Return hex payload string
    
    Input:
    - offset = Offset in nof bytes
    - nof    = Nof bytes to get
    Return:
    - hex string
    """
    return self.hexPayload[offset * self.c_hw : (offset + nof) * self.c_hw]
    
  def packPayload(self, data, width, append=False):
    """Pack data list into hex payload string
    
    Input:
    - data   = list of numbers
    - width  = nof bytes per number
    - append = when false clear hexPayload first, when true append the data
    Output:
    - self.hexPayload
    Return: void
    Example:
      p=mep.MepMessage()
      p.packPayload([3,-400,50],2)
    """
    if append == False:
      self.clearPayload()
    if width==1:
      for i in data:
        self.appendPayload(('%02x' % i)[-2:])   # pad 0 for single hex digit, strip sign extension of negative long
    elif width==2:
      for i in data:
        self.appendPayload(self.swap2(('%04x' % i)[-4:]))
    else: # width==4:
      for i in data:
        self.appendPayload(self.swap4(('%08x' % i)))
    
  def extractPayload(self, readData, append=False):
    """Extract hex payload string from rspctl read data
    Input:
    - readData = echo from rspctl command
    - append   = when false clear hexPayload first, when true append the read data
    Output:
    - self.hexPayload
    Return: void
    """
    if append == False:
      self.clearPayload()
    j = 1
    for i in range(len(readData)):
      if readData[i] == '\n':
        j = 0                               # found new line
      elif readData[i] != ':' and j == 5:
        j = -1                              # check data start of ':' at pos 5
      elif j > 5 and j < 55:
        if readData[i] != ' ':              # read the data
          self.appendPayload(readData[i])
      if j >= 0:
        j += 1

  def unpackPayload(self, width, sign='+'):
    """Unpack data list from hex payload string
    
    Input:
    - self.hexPayload
    - width = nof bytes per number
    - sign = '+' for unsigned word list, '-' for signed word list
    Return:
    - list of numbers
    Example:
      p=mep.MepMessage()
      p.packPayload([3,-400,50],2)
      p.unpackPayload(2, '-'):
    """
    umax = [0, 256, 65536, 0, 4294967296L]  # sign convert constant
    data = []
    if sign=='+':  # unsigned
      for i in range(0, self.lenPayload(), width):
        d = self.getPayload(i, width)
        if width==2:
          d = self.swap2(d)
        elif width==4:
          d = self.swap4(d)
        d = int(d,16)
        data.append(d)
    else:          # signed
      for i in range(0, self.lenPayload(), width):
        d = self.getPayload(i, width)
        if width==2:
          d = self.swap2(d)
        elif width==4:
          d = self.swap4(d)
        if int(d[0],16)<8:
          d = int(d,16)
        else:
          d = int(d,16)-umax[width]
        data.append(d)
    return data

  def setOffset(self, offset=0):
    """Set offset for read access in payload octets
    
    Input:
    - offset      = offset as nof bytes
    Output:
    - self.offset = offset as nof bytes
    Return: void
    """ 
    self.offset = offset
    
  def incrOffset(self, offset=1):
    """Increment offset for read access in payload octets
    
    Input:
    - offset      = offset increment in nof bytes
    Output:
    - self.offset = offset incremented by offset nof bytes
    Return: void
    """ 
    self.offset += offset
    
  def readUnsigned(self, width):
    """Read unsigned word from the hex payload string and increment offset
    
    Input:
    - self.hexPayload
    - width = nof bytes per number
    - self.offset
    Return:
    - unsigned
    Example:
      p=mep.MepMessage()
      p.packPayload([3,-400,50],2)
      p.setOffset(2)
      p.readUnsigned(payload, 2)
    """
    d = self.getPayload(self.offset, width)
    self.incrOffset(width)
    if width==2:
      d = self.swap2(d)
    elif width==3:
      d = self.swap3(d)
    elif width==4:
      d = self.swap4(d)
    return int(d,16)

  def readSigned(self, width):
    """Read signed from the hex payload string and increment offset
    
    Input:
    - self.hexPayload
    - width = nof bytes per number
    - self.offset
    Return:
    - signed
    Example:
      p=mep.MepMessage()
      p.packPayload([3,-400,50],2)
      p.setOffset(1)
      p.readSigned(payload, 2)
    """
    umax = [0, 256, 65536, 0, 4294967296L]  # sign convert constant
    d = self.getPayload(self.offset, width)
    self.incrOffset(width)
    if width==2:
      d = self.swap2(d)
    elif width==3:
      d = self.swap3(d)
    elif width==4:
      d = self.swap4(d)
    if int(d[0],16)<8:
      d = int(d,16)
    else:
      d = int(d,16)-umax[width]
    return d

