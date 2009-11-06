import socket

from nanostamp import now2ns;

class IncomingByteList(list):

    def peel_number(self, nbytes):
        """Peel off a little-endian encoded number from the start."""
        N = self[:nbytes]
        del self[:nbytes]
        multiplier = 1
        y = 0
        for x in N:
            y = y + multiplier * x
            multiplier = multiplier * 256
        return y

    def peel_timeval2ns(self):
        tv_sec  = self.peel_number(8)
        tv_nsec = self.peel_number(4)
        return tv_sec * 1000000000 + tv_nsec

    def peel_string(self):
        nbytes = self.peel_number(2) # strings are 64k max
        S = self[:nbytes]
        del self[:nbytes]
        return list2str(S)

    def peel_number_array(self, bytes_per_element, as_string = False):
        number_of_elements = self.peel_number(4)
        #MAXMOD
        #print "MAXMOD: peel-number-array will peel %d numbers:\n" % number_of_elements
        if as_string:
            nbytes = number_of_elements * bytes_per_element
            data = list2str(self[:nbytes])
            del self[:nbytes]
        else:
            data = []
            for i in range(number_of_elements):
                data.append(self.peel_number(bytes_per_element))
        return data

class OutgoingByteList(list):

    def append_number(self, value, nbytes):
        """Append a little-endian encoded number to the end."""
        while nbytes>0:
            self.append(value % 256)
            value = value // 256
            nbytes = nbytes - 1
        assert value==0
    
    def append_time_ns(self, ns):
        (ns_sec, ns_nsec)  = (ns // 1000000000, ns % 1000000000)
        self.append_number(ns_sec, 8)
        self.append_number(ns_nsec, 4)

    def append_string(self, S):
        assert len(S)<=65535
        self.append_number(len(S), 2) # strings are 64k max
        self.extend(str2list(S))

    def append_bitset(self, n_bits, bits):
        assert n_bits % 8 == 0
        bits = set(bits)
        nb = 0
        bytelist = []
        byteval = 0
        for i in range(n_bits):
            if i in bits:
                byteval = 2 * byteval + 1
            else:
                byteval = 2 * byteval
            nb = nb + 1
            if nb == 8:
                bytelist.append(byteval)
                byteval = 0
                nb = 0
        self.extend(bytelist)

def str2list(S):
    return map(ord, S)

def list2str(L):
    return str.join("", map(chr, L))

class GCFConnection:

    def __init__(self, host, port):
        self._host = host
        self._port = port
        self._socket = None

    def connect(self):
        assert self._socket is None
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.connect((self._host, self._port))

    def disconnect(self):
        self._socket.close()
        self._socket = None

    def _xrecv(self, nbytes):
        result = []
        #MAXMOD
        #print "in _xrecv, going to get ", nbytes, " =nbytes\n"
        while nbytes > 0:
            data = self._socket.recv(nbytes)
            if len(data)==0:
                raise EOFError
            result.append(data)
            nbytes = nbytes - len(data)
        return str2list(str.join('', result))

    # Note: definition of 'protocol' field can be found here:
    #     LOFAR/MAC/GCF/TM/include/GCF/TM/GCF_Event.h

    def send_packet(self, protocol, payload):
        packet = OutgoingByteList()
        packet.append_number(protocol, 2)
        packet.append_number(len(payload), 4)
        packet.extend(payload)
        packet = list2str(packet)
        self._socket.sendall(packet)

    def recv_packet(self):
        # get protocol spec (2 bytes) and size (4 bytes)
        data     = IncomingByteList(self._xrecv(6))
        protocol = data.peel_number(2)
        nbytes   = data.peel_number(4)
        # get payload
        payload = IncomingByteList(self._xrecv(nbytes))
        return (protocol, payload)
