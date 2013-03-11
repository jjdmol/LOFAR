#!/usr/bin/python
# This is a simple port-forward / proxy, written using only the default python
# library. If you want to make a suggestion or fix something you can contact-me
# at voorloop_at_gmail.com
# Distributed over IDC(I Don't Care) license
import socket
import select
import time
import sys


buffer_size = 4096
delay = 0.0001 #polling frequency
forward_to = ('lce072', '9090')

class Forward:
    def __init__(self):
        self.forward = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def start(self, host, port):
        try:
            self.forward.connect((host, int(port)))
            return self.forward
        except Exception, e:
            print e
            return False

class NodeDeamon:
    input_list = []
    socked_from_to_ = {}

    def __init__(self, host, port):
        self.socked_listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socked_listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socked_listener.bind((host, port))
        self.socked_listener.listen(200)

    def main_loop(self):
        self.input_list.append(self.socked_listener)
        continue_loop = True
        while continue_loop:
            time.sleep(delay)
            ss = select.select
            inputready, outputready, exceptready = ss(self.input_list, [], [])
            for self.s in inputready:
                if self.s == self.socked_listener:
                    self.on_accept()
                    break
                """
                
Traceback (most recent call last):
  File "headnode.py", line 173, in <module>
    nodeDeamon.main_loop()
  File "headnode.py", line 50, in main_loop
    self.data = self.s.recv(buffer_size)
socket.error: [Errno 104] Connection reset by peer

                """
                self.data = self.s.recv(buffer_size)
                if len(self.data) == 0:
                    self.on_close()
                else:
                    self.on_recv()

    def on_accept(self):
        clientsock, clientaddr = self.socked_listener.accept()
        print clientaddr, "has connected"
        clientsock.send("test")
        self.input_list.append(clientsock)
        if self.stand_alone:
            return

        forward = Forward().start(forward_to[0], forward_to[1])
        if forward:

            self.input_list.append(forward)
            self.socked_from_to_[clientsock] = forward
            self.socked_from_to_[forward] = clientsock

        else:
            print "Can't establish connection with remote socked_listener.",
            print "running in standalone mode"
            #print "Closing connection with client side", clientaddr
            #clientsock.close()

    def on_close(self):
        print self.s.getpeername(), "has disconnected"
        #remove objects from input_list
        self.input_list.remove(self.s)
        if not self.stand_alone:
            self.input_list.remove(self.socked_from_to_[self.s])
            out = self.socked_from_to_[self.s]
            # close the connection with client
            self.socked_from_to_[out].close()  # equivalent to do self.s.close()
            # close the connection with remote socked_listener
            self.socked_from_to_[self.s].close()
            # delete both objects from socked_from_to_ dict
            del self.socked_from_to_[out]
            del self.socked_from_to_[self.s]

    def on_recv(self):
        data = self.data
        if data == "ping":
            self.s.send("pong")
        # here we can parse and/or modify the data before send forward
        return_data = """
<?xml version="1.0" encoding="utf-8"?>
<testsuite errors="0" failures="0" name="copier_test.copierTest" tests="1" time="0.000">
        <testcase classname="copier_test.copierTest" name="test_validate_source_target_mapfile" time="0.000"/>
        <system-out>
<![CDATA[Muck parameterset, parameter retrieved:
/tmp/tmpCTj4nW/parset
Muck parameterset, parameter retrieved:
/tmp/tmpCTj4nW/parset
Muck parameterset, parameter retrieved:
/tmp/tmpCTj4nW/parset
Muck parameterset, parameter retrieved:
/tmp/tmpCTj4nW/parset
Muck parameterset, parameter retrieved:
/tmp/tmpyTGmAl/parset
Muck parameterset, parameter retrieved:
/tmp/tmpyTGmAl/parset
Muck parameterset, parameter retrieved:
/tmp/tmpyTGmAl/parset
Muck parameterset, parameter retrieved:
/tmp/tmpyTGmAl/parset
]]>     </system-out>
        <system-err>
<![CDATA[]]>    </system-err>
</testsuite>

        """

#        print ">{0}<".format(data[0:3])
#        if data[0:3] == "GET":
#            self.s.send(return_data)
#            self.s.close()
        print data
        if data == "Stop":
            return False
        if not self.stand_alone:
            self.socked_from_to_[self.s].send(data)
        return True

def read_parset(file_path):
    fp = open(file_path)
    head_port = None
    nodes = None
    nodes_port = None
    for line in fp.readlines():
        head, tail = line.split('=')

        if head == "headnode_port":
            data = eval(tail.strip())
            head_port = data

        if head == "nodes":
            data = eval(tail.strip())
            nodes = data

        if head == "nodes_port":
            data = eval(tail.strip())
            nodes_port = data

    if head_port == None or nodes == None or nodes_port == None:
        print "Error parsing parset file:"
        for line in fp.readlines():
            print line
        raise Exception("Incorrect parset")

    return head_port, nodes, nodes_port


if __name__ == '__main__':
    file_path = "config.par"
    head_port, nodes, nodes_port = read_parset(file_path)


    nodeDeamon = NodeDeamon('', 9090)
    nodeDeamon.stand_alone = True
    try:
        nodeDeamon.main_loop()
        print "Received exit command"
        sys.exit(0)
    except KeyboardInterrupt:
        print "Ctrl C - Stopping NodeDeamon"
        sys.exit(1)
