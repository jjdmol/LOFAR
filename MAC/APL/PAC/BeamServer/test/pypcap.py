#!/usr/bin/env python2

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

  def send(self, iface, data):
    sdto = (iface, ETH_P_ALL)
    self.sock.bind(sdto)
    self.sock.sendto(str(data), stdto)

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

if __name__=='__main__':

  print dir(socket)
  s = rawsocket("eth0", "ether dst aa:bb:cc:dd:ee:ff")

  while 1:
    pkt=s.recv(100)
    if not len(pkt):
      break
    print_packet(len(pkt), pkt, 0)
    

  if len(sys.argv) < 3:
    print 'usage: sniff.py <interface> <expr>'
    sys.exit(0)
  p = pcap.pcapObject()
  #dev = pcap.lookupdev()
  dev = sys.argv[1]
  net, mask = pcap.lookupnet(dev)
  # note:  to_ms does nothing on linux
  p.open_live(dev, 1600, 0, 100)
  #p.dump_open('dumpfile')
  p.setfilter(string.join(sys.argv[2:],' '), 0, 0)

  # try-except block to catch keyboard interrupt.  Failure to shut
  # down cleanly can result in the interface not being taken out of promisc.
  # mode
  #p.setnonblock(1)
  #try:
  while 1:
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
  



