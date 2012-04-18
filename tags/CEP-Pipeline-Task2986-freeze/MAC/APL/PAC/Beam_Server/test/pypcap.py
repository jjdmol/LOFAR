#!/usr/bin/env python

from __future__ import generators

import pcap
import sys
import string
import time
import socket
import struct
from fcntl import ioctl

# From bits/ioctls.h
SIOCGIFHWADDR  = 0x8927          # Get hardware address    
SIOCGIFADDR    = 0x8915          # get PA address          
SIOCGIFNETMASK = 0x891b          # get network PA mask     
SIOCGIFNAME    = 0x8910          # get iface name          
SIOCSIFLINK    = 0x8911          # set iface channel       
SIOCGIFCONF    = 0x8912          # get iface list          
SIOCGIFFLAGS   = 0x8913          # get flags               
SIOCSIFFLAGS   = 0x8914          # set flags               
SIOCGIFINDEX   = 0x8933          # name -> if_index mapping
SIOCGIFCOUNT   = 0x8938          # get number of devices

# From linux/if_ether.h
ETH_P_ALL = 3

# From netpacket/packet.h
PACKET_ADD_MEMBERSHIP  = 1
PACKET_DROP_MEMBERSHIP = 2
PACKET_RECV_OUTPUT     = 3
PACKET_RX_RING         = 5
PACKET_STATISTICS      = 6
PACKET_MR_MULTICAST    = 0
PACKET_MR_PROMISC      = 1
PACKET_MR_ALLMULTI     = 2


# From bits/socket.h
SOL_PACKET = 263
# From asm/socket.h
SO_ATTACH_FILTER = 26
SOL_SOCKET = 1

def mac2str(mac):
    return "".join(map(lambda x: chr(int(x,16)), mac.split(":")))

def str2mac(s):
    return ("%02x:"*6)[:-1] % tuple(map(ord, s)) 

###############
## BPF stuff ##
###############

def attach_filter(s, iface, filter):
    import os
    # XXX We generate the filter on the interface conf.iface 
    # because tcpdump open the "any" interface and ppp interfaces
    # in cooked mode. As we use them in raw mode, the filter will not
    # work... one solution could be to use "any" interface and translate
    # the filter from cooked mode to raw mode
    # mode
    f = os.popen("/usr/sbin/tcpdump -i %s -ddd -s 1600 '%s'" % (iface,filter))
    lines = f.readlines()
    if f.close():
        raise Exception("Filter parse error")
    nb = int(lines[0])
    bpf = ""
    for l in lines[1:]:
        bpf += struct.pack("HBBI",*map(long,l.split()))

    # XXX. Argl! We need to give the kernel a pointer on the BPF,
    # python object header seems to be 20 bytes
    bpfh = struct.pack("HI", nb, id(bpf)+20)  
    s.setsockopt(SOL_SOCKET, SO_ATTACH_FILTER, bpfh)

def get_if(iff,cmd):
    s=socket.socket()
    ifreq = ioctl(s, cmd, struct.pack("16s16x",iff))
    s.close()
    return ifreq

def get_if_hwaddr(iff):
    addrfamily, mac = struct.unpack("16xh6s8x",get_if(iff,SIOCGIFHWADDR))
    if addrfamily in [ARPHDR_ETHER,ARPHDR_LOOPBACK]:
        return str2mac(mac)
    else:
        raise Exception("Unsupported address family (%i)"%addrfamily)


def get_if_index(iff):
    return int(struct.unpack("I",get_if(iff, SIOCGIFINDEX)[16:20])[0])

def set_promisc(s,iff,val=1):
    mreq = struct.pack("IHH8s", get_if_index(iff), PACKET_MR_PROMISC, 0, "")
    if val:
        cmd = PACKET_ADD_MEMBERSHIP
    else:
        cmd = PACKET_DROP_MEMBERSHIP
    s.setsockopt(SOL_PACKET, cmd, mreq)

class rawsocket:
  '''demonstration class only 
  - coded for clarity, not efficiency'''
  def __init__(self, iface, filter=None):
    self.sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(ETH_P_ALL))
    self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 2**30)
    if filter is not None:
      attach_filter(self.sock, iface, filter)
    set_promisc(self.sock, iface)

  def close(self):
    close(self.sock)
    set_promisc(self.sock, i, 0)

  def connect(self, host, port):
    self.sock.connect((host, port))

  def send(self, data, iface='eth0'):
    sdto = (iface, ETH_P_ALL)
    self.sock.bind(sdto)
    self.sock.sendto(str(data), sdto)

  def recv(self, count):
    pkt, sa_ll = self.sock.recvfrom(count)
    return pkt

def decode_epa_packet(s):
  d={}
  d['command']=ord(s[0])
  d['seqnr']=ord(s[1])
  d['pktsize']=socket.ntohs(struct.unpack('H',s[2:4])[0])
  d['data']=s[4:]
  return d

def dumphex(s):
  bytes = map(lambda x: '%.2x' % x, map(ord, s))
  for i in xrange(0,len(bytes)/16):
    print '    %s' % string.join(bytes[i*16:(i+1)*16],' ')
#  for i in xrange(len(s)/16):
#    print s[i*16:(i+1)*16]

def print_packet(pktlen, data, timestamp):
  if not data:
    return
  decoded=decode_epa_packet(data[14:])
  print '[ command=%s\n  seqnr=%d\n  pktsize=%d\n  data=' % (
    decoded['command'],
    decoded['seqnr'],
    decoded['pktsize'])
  if len(decoded['data']):
    dumphex(decoded['data'])
  print ']'

################
## Generators ##
################

class Gen:
    def __iter__(self):
        return iter([])
    

class SetGen(Gen):
    def __init__(self, set):
        if type(set) is list:
            self.set = set
        else:
            self.set = [set]
    def transf(self, element):
        return element
    def __iter__(self):
        for i in self.set:
            if (type(i) is tuple) and (len(i) == 2):
                if  (i[0] <= i[1]):
                    j=i[0]
                    while j <= i[1]:
                        yield j
                        j += 1
            elif isinstance(i, Gen):
                for j in i:
                    yield j
            else:
                yield i
    def __repr__(self):
        return "<SetGen %s>" % self.set.__repr__()

############
## Fields ##
############

class Field:
    islist=0
    def __init__(self, name, default, fmt="H"):
        self.name = name
        self.fmt = "!"+fmt
        self.default = self.any2i(None,default)
        self.sz = struct.calcsize(self.fmt)

    def h2i(self, pkt, x):
        return x
    def i2h(self, pkt, x):
        return x
    def m2i(self, pkt, x):
        return x
    def i2m(self, pkt, x):
        if x is None:
            x = 0
        return x
    def any2i(self, pkt, x):
        return x
    def i2repr(self, pkt, x):
	if x is None:
	    x = 0
        return repr(self.i2h(pkt,x))
    def addfield(self, pkt, s, val):
        return s+struct.pack(self.fmt, self.i2m(pkt,val))
    def getfield(self, pkt, s):
        return  s[self.sz:], self.m2i(pkt, struct.unpack(self.fmt, s[:self.sz])[0])
    def copy(self, x):
        if hasattr(x, "copy"):
            return x.copy()
        elif type(x) is list:
            return x[:]
        else:
            return x
    def __eq__(self, other):
        return self.name == other
    def __hash__(self):
        return hash(self.name)
    def __repr__(self):
        return self.name

class MACField(Field):
    def __init__(self, name, default):
        Field.__init__(self, name, default, "6s")
    def i2m(self, pkt, x):
        return mac2str(x)
    def m2i(self, pkt, x):
        return str2mac(x)
    def any2i(self, pkt, x):
        if type(x) is str and len(x) is 6:
            x = self.m2i(pkt, x)
        return x
    def i2repr(self, pkt, x):
        return self.i2h(pkt, x)

class DestMACField(MACField):
    def __init__(self, name):
        MACField.__init__(self, name, None)
    def i2h(self, pkt, x):
        if x is None:
            dstip = None
            #dstip = pkt.payload.dst
            if isinstance(dstip, Gen):
                warning("Dest mac not calculated if more than 1 dest IP (%s)"%repr(dstip))
                return None
            x = "ff:ff:ff:ff:ff:ff"
            if dstip is not None:
                m=getmacbyip(dstip)
                if m:
                    x = m
                else:
                    warning("Mac address for %s not found\n"%dstip)
        return MACField.i2h(self, pkt, x)
    def i2m(self, pkt, x):
        return MACField.i2m(self, pkt, self.i2h(pkt, x))
        
class SourceMACField(MACField):
    def __init__(self, name):
        MACField.__init__(self, name, None)
    def i2h(self, pkt, x):
        if x is None:
            dstip = None
            #dstip = pkt.payload.dst
            if isinstance(dstip, Gen):
                warning("Source mac not calculated if more than 1 dest IP (%s)"%repr(dstip))
                return None
            x = "00:00:00:00:00:00"
            if dstip is not None:
                iff,a,gw = choose_route(dstip)
                m = get_if_hwaddr(iff)
                if m:
                    x = m
        return MACField.i2h(self, pkt, x)
    def i2m(self, pkt, x):
        return MACField.i2m(self, pkt, self.i2h(pkt, x))

class ShortField(Field):
    def __init__(self, name, default):
        Field.__init__(self, name, default, "H")

class XShortField(ShortField):
    def i2repr(self, pkt, x):
	if x is None:
	    x = 0
        return hex(self.i2h(pkt, x))

class BitField(Field):
    def __init__(self, name, default, size):
        Field.__init__(self, name, default)
        self.size = size
    def addfield(self, pkt, s, val):
        if val is None:
            val = 0
        if type(s) is tuple:
            s,bitsdone,v = s
        else:
            bitsdone = 0
            v = 0
        v <<= self.size
        v |= val & ((1<<self.size) - 1)
        bitsdone += self.size
        while bitsdone >= 8:
            bitsdone -= 8
            s = s+struct.pack("!B", v >> bitsdone)
            v &= (1<<bitsdone)-1
        if bitsdone:
            return s,bitsdone,v
        else:
            return s
    def getfield(self, pkt, s):
        if type(s) is tuple:
            s,bn = s
        else:
            bn = 0
        fmt,sz=[("!B",1),("!H",2),("!I",4),("!I",4)][self.size/8]
        b = struct.unpack(fmt, s[:sz])[0] << bn
        b >>= (sz*8-self.size)
        b &= (1 << self.size)-1
        bn += self.size
        s = s[bn/8:]
        bn = bn%8
        if bn:
            return (s,bn),b
        else:
            return s,b

class ByteField(Field):
    def __init__(self, name, default):
        Field.__init__(self, name, default, "B")

class ShortField(Field):
    def __init__(self, name, default):
        Field.__init__(self, name, default, "H")

class XShortField(ShortField):
    def i2repr(self, pkt, x):
	if x is None:
	    x = 0
        return hex(self.i2h(pkt, x))

###############
# Packets
###############

class Packet(Gen):
    name="abstract packet"

    fields_desc = []

    aliastypes = []
    overload_fields = {}

    underlayer = None

    payload_guess = []
    initialized = 0

    def __init__(self, pkt="", **fields):
        self.time  = time.time()
        self.aliastypes = [ self.__class__ ] + self.aliastypes
        self.default_fields = {}
        self.overloaded_fields = {}
        self.fields={}
        self.fieldtype={}
        self.__dict__["payload"] = NoPayload()
        for f in self.fields_desc:
            self.default_fields[f] = f.default
            self.fieldtype[f] = f
        self.initialized = 1
        if pkt:
            self.dissect(pkt)
        for f in fields.keys():
            self.fields[f] = self.fieldtype[f].any2i(self,fields[f])

    def add_payload(self, payload):
        if payload is None:
            return
        elif not isinstance(self.payload, NoPayload):
            self.payload.add_payload(payload)
        else:
            if isinstance(payload, Packet):
                self.__dict__["payload"] = payload
                payload.add_underlayer(self)
                for t in self.aliastypes:
                    if payload.overload_fields.has_key(t):
                        self.overloaded_fields = payload.overload_fields[t]
                        break
            elif type(payload) is str:
                self.__dict__["payload"] = Raw(load=payload)
            else:
                raise TypeError("payload must be either 'Packet' or 'str', not [%s]" % repr(payload))
    def remove_payload(self):
        self.payload.remove_underlayer(self)
        self.__dict__["payload"] = NoPayload()
        self.overloaded_fields = {}
    def add_underlayer(self, underlayer):
        self.underlayer = underlayer
    def remove_underlayer(self, underlayer):
        self.underlayer = None
    def copy(self):
        clone = self.__class__()
        clone.fields = self.fields.copy()
        for k in clone.fields:
            clone.fields[k]=self.fieldtype[k].copy(clone.fields[k])
        clone.default_fields = self.default_fields.copy()
        clone.overloaded_fields = self.overloaded_fields.copy()
        clone.overload_fields = self.overload_fields.copy()
        clone.underlayer=self.underlayer
        clone.__dict__["payload"] = self.payload.copy()
        clone.payload.add_underlayer(clone)
        return clone
    def __getattr__(self, attr):
        if self.initialized:
            fld = self.fieldtype.get(attr)
            if fld is None:
                i2h = lambda x,y: y
            else:
                i2h = fld.i2h
            for f in ["fields", "overloaded_fields", "default_fields"]:
                fields = self.__dict__[f]
                if fields.has_key(attr):
                    return i2h(self, fields[attr] )
            return getattr(self.payload, attr)
        raise AttributeError(attr)

    def __setattr__(self, attr, val):
        if self.initialized:
            if self.default_fields.has_key(attr):
                fld = self.fieldtype.get(attr)
                if fld is None:
                    any2i = lambda x,y: y
                else:
                    any2i = fld.any2i
                self.fields[attr] = any2i(self, val)
            elif attr == "payload":
                self.remove_payload()
                self.add_payload(val)
            else:
                self.__dict__[attr] = val
        else:
            self.__dict__[attr] = val
    def __delattr__(self, attr):
        if self.initialized:
            if self.fields.has_key(attr):
                del(self.fields[attr])
                return
            elif self.default_fields.has_key(attr):
                return
            elif attr == "payload":
                self.remove_payload()
                return
        if self.__dict__.has_key(attr):
            del(self.__dict__[attr])
        else:
            raise AttributeError(attr)
            
    def __repr__(self):
        s = ""
        for f in self.fields_desc:
            if f in self.fields:
                s += " %s=%s" % (f, f.i2repr(self, self.fields[f]))
            elif f in self.overloaded_fields:
                s += " %s=%s" % (f, f.i2repr(self, self.overloaded_fields[f]))
        return "<%s%s |%s>"% (self.__class__.__name__,
                              s, repr(self.payload))
    def __str__(self):
        return self.__iter__().next().build()
    def __div__(self, other):
        if isinstance(other, Packet):
            cloneA = self.copy()
            cloneB = other.copy()
            cloneA.add_payload(cloneB)
            return cloneA
        elif type(other) is str:
            return self/Raw(load=other)
        else:
            return other.__rdiv__(self)
    def __rdiv__(self, other):
        if type(other) is str:
            return Raw(load=other)/self
        else:
            raise TypeError
    def __len__(self):
        return len(self.__str__())
    def do_build(self):
        p=""
        for f in self.fields_desc:
            p = f.addfield(self, p, self.__getattr__(f))
        pkt = p+str(self.payload)
        return pkt
    
    def post_build(self, pkt):
        return pkt

    def build(self):
        return self.post_build(self.do_build())

    def extract_padding(self, s):
        return s,None

    def do_dissect(self, s):
        flist = self.fields_desc[:]
        flist.reverse()
        while s and flist:
            f = flist.pop()
            s,fval = f.getfield(self, s)
            self.fields[f] = fval
        payl,pad = self.extract_padding(s)
        self.do_dissect_payload(payl)
        if pad and conf.padding:
            self.add_payload(Padding(pad))
    def do_dissect_payload(self, s):
        if s:
            cls = self.guess_payload_class()
            try:
                p = cls(s)
            except:
                p = Raw(s)
            self.add_payload(p)

    def dissect(self, s):
        return self.do_dissect(s)

    def guess_payload_class(self):
        for t in self.aliastypes:
            for fval, cls in t.payload_guess:
                ok = 1
                for k in fval.keys():
                    if fval[k] != getattr(self,k):
                        ok = 0
                        break
                if ok:
                    return cls
        return None

    def hide_defaults(self):
        for k in self.fields.keys():
            if self.default_fields.has_key(k):
                if self.default_fields[k] == self.fields[k]:
                    del(self.fields[k])
        self.payload.hide_defaults()
            

    def __iter__(self):
        def loop(todo, done, self=self):
            if todo:
                eltname = todo.pop()
                elt = self.__getattr__(eltname)
                if not isinstance(elt, Gen):
                    if self.fieldtype[eltname].islist:
                        elt = SetGen([elt])
                    else:
                        elt = SetGen(elt)
                for e in elt:
                    done[eltname]=e
                    for x in loop(todo[:], done):
                        yield x
            else:
                if isinstance(self.payload,NoPayload):
                    payloads = [None]
                else:
                    payloads = self.payload
                for payl in payloads:
                    done2=done.copy()
#                    for k in done2:
#                        if isinstance(done2[k], RandNum):
#                            done2[k] = int(done2[k])
                    pkt = self.__class__(**done2)
                    pkt.underlayer = self.underlayer
                    pkt.overload_fields = self.overload_fields.copy()
                    if payl is None:
                        yield pkt
                    else:
                        yield pkt/payl
        return loop(map(lambda x:str(x), self.fields.keys()), {})

    def send(self, s, slp=0):
        for p in self:
            s.send(str(p))
            if slp:
                time.sleep(slp)

    def __gt__(self, other):
        if isinstance(other, Packet):
            return other < self
        elif type(other) is str:
            return 1
        else:
            raise TypeError((self, other))
    def __lt__(self, other):
        if isinstance(other, Packet):
            return self.answers(other)
        elif type(other) is str:
            return 1
        else:
            raise TypeError((self, other))
        
    def hashret(self):
        return self.payload.hashret()
    def answers(self, other):
        return 0

    def haslayer(self, cls):
        if self.__class__ == cls:
            return 1
        return self.payload.haslayer(cls)
    def getlayer(self, cls):
        if self.__class__ == cls:
            return self
        return self.payload.getlayer(cls)
    

    def display(self, lvl=0):
        print "---[ %s ]---" % self.name
        for f in self.fields_desc:
            print "%s%-10s= %s" % ("   "*lvl, f.name, f.i2repr(self,self.__getattr__(f)))
        self.payload.display(lvl+1)

    def sprintf(self, fmt, relax=1):
        """sprintf(format, [relax=1]) -> str
where format is a string that can include directives. A directive begins and
ends by % and has the following format %[fmt[r],][cls[:nb].]field%.

fmt is a classic printf directive, "r" can be appended for raw substitution
(ex: IP.flags=0x18 instead of SA), nb is the number of the layer we want
(ex: for IP/IP packets, IP:2.src is the src of the upper IP layer).
Special case : "%.time%" is the creation time.
Ex : p.sprintf("%.time% %-15s,IP.src% -> %-15s,IP.dst% %IP.chksum% "
               "%03xr,IP.proto% %r,TCP.flags%")
"""
        s = ""
        while "%" in fmt:
            i = fmt.index("%")
            s += fmt[:i]
            fmt = fmt[i+1:]
            if fmt[0] == "%":
                fmt = fmt[1:]
                s += "%"
                continue
            else:
                try:
                    i = fmt.index("%")
                    sfclsfld = fmt[:i]
                    fclsfld = sfclsfld.split(",")
                    if len(fclsfld) == 1:
                        f = "s"
                        clsfld = fclsfld[0]
                    elif len(fclsfld) == 2:
                        f,clsfld = fclsfld
                    else:
                        raise Exception
                    cls,fld = clsfld.split(".")
                    num = 1
                    if ":" in cls:
                        cls,num = cls.split(":")
                        num = int(num)
                    fmt = fmt[i+1:]
                except:
                    raise Exception("Bad format string [%%%s%s]" % (fmt[:25], fmt[25:] and "..."))
                else:
                    if fld == "time":
                        val = time.strftime("%H:%M:%S.%%06i", time.localtime(self.time)) % int((self.time-int(self.time))*1000000)
                    elif cls == self.__class__.__name__ and hasattr(self, fld):
                        if num > 1:
                            val = self.payload.sprintf("%%%s,%s:%s.%s%%" % (f,cls,num-1,fld), relax)
                            f = "s"
                        elif f[-1] == "r":  # Raw field value
                            val = getattr(self,fld)
                            f = f[:-1]
                            if not f:
                                f = "s"
                        else:
                            val = getattr(self,fld)
                            if fld in self.fieldtype:
                                val = self.fieldtype[fld].i2repr(self,val)
                    else:
                        val = self.payload.sprintf("%%%s%%" % sfclsfld, relax)
                        f = "s"
                    s += ("%"+f) % val
            
        s += fmt
        return s

    def mysummary(self):
        return ""
    def summaryback(self, smallname=0):
        ret = ""
        if not smallname:
            ret = self.mysummary()
        if ret:
            smallname = 1
        else:
            ret = self.__class__.__name__
        if self.underlayer is not None:
            ret = "%s / %s" % (self.underlayer.summaryback(smallname),ret)
        return ret
    def summary(self, onlyname=0):
        if isinstance(self.payload, NoPayload):
            return self.summaryback()
        else:
            return self.payload.summary()

class NoPayload(Packet,object):
    def __new__(cls, *args, **kargs):
        singl = cls.__dict__.get("__singl__")
        if singl is None:
            cls.__singl__ = singl = object.__new__(cls)
            Packet.__init__(singl, *args, **kargs)
        return singl
    def __init__(self, *args, **kargs):
        pass
    def add_payload(self, payload):
        raise Exception("Can't add payload to NoPayload instance")
    def remove_payload(self):
        pass
    def add_underlayer(self,underlayer):
        pass
    def remove_underlayer(self):
        pass
    def copy(self):
        return self
    def __repr__(self):
        return ""
    def __str__(self):
        return ""
    def __getattr__(self, attr):
        print self.__dict__
        print self.__class__.__dict__
        print attr
        if attr in self.__dict__:
            return self.__dict__[attr]
        elif attr in self.__class__.__dict__:
            return self.__class__.__dict__[attr]
        else:
            raise AttributeError, attr
    def hide_defaults(self):
        pass
    def __iter__(self):
        return iter([])
    def hashret(self):
        return ""
    def answers(self, other):
        return isinstance(other, NoPayload) or isinstance(other, Padding)
    def haslayer(self, cls):
        return 0
    def getlayer(self, cls):
        return None
    def display(self, lvl=0):
        pass
    def sprintf(self, fmt, relax):
        if relax:
            return "??"
        else:
            raise Exception("Format not found [%s]"%fmt)
    def summary(self):
        return self.summaryback()

class Ethernet(Packet):
  name = "Ethernet"
  fields_desc = [ DestMACField("dst"),
                  SourceMACField("src"),
                  XShortField("length", 0x0000) ]

class EPACommand(Ethernet):
  name = "EPACommand"
  fields_desc = [ ByteField("command", 0),
                  ByteField("seqnr", 0),
                  ShortField("pktsize", 12) ]
  
if __name__=='__main__':

  f = Field("test", 0)

  print f

  p = Ethernet(dst="aa:bb:cc:dd:ee:ff")/EPACommand(command=1,
                 seqnr=1,
                 pktsize=12)

  print struct.unpack('!BBH', p)

  s = rawsocket("eth0", "ether dst aa:bb:cc:dd:ee:ff")

  p.send(s)

  sys.exit(0)

#   while 1:
#     pkt=s.recv(1500)
#     if not len(pkt):
#       break
#     print_packet(len(pkt), pkt, 0)
    

  if len(sys.argv) < 2:
    print 'usage: sniff.py <interface> [<expr>]'
    sys.exit(0)
  p = pcap.pcapObject()
  #dev = pcap.lookupdev()
  dev = sys.argv[1]
  net, mask = pcap.lookupnet(dev)
  # note:  to_ms does nothing on linux
  p.open_live(dev, 1600, 0, 0)
  #p.dump_open('dumpfile')
  if len(sys.argv) > 2:
      p.setfilter(string.join(sys.argv[2:],' '), 0, 0)

  # try-except block to catch keyboard interrupt.  Failure to shut
  # down cleanly can result in the interface not being taken out of promisc.
  # mode
  #p.setnonblock(1)
  #try:
  while 1:
    #s.send('eth0', '112345678911234567891123456789')
    p.dispatch(1, print_packet)

    # specify 'None' to dump to dumpfile, assuming you have called
    # the dump_open method
    #  p.dispatch(0, None)

    # the loop method is another way of doing things
    #  p.loop(1, print_packet)

    # as is the next() method
    # p.next() returns a (pktlen, data, timestamp) tuple 
    #  apply(print_packet,p.next())
  #except KeyboardInterrupt:
  #  print '%s' % sys.exc_type
  #  print 'shutting down'
  #  print '%d packets received, %d packets dropped, %d packets dropped by interface' % p.stats()
  



