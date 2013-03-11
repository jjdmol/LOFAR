#!/usr/bin/python
# This is a simple port-forward / proxy, written using only the default python
# library. If you want to make a suggestion or fix something you can contact-me
# at voorloop_at_gmail.com
# Distributed over IDC(I Don't Care) license
import socket
import select
import time
import sys
import client
from threading import Thread


buffer_size = 4096
delay = 0.0001 #polling frequency
forward_to = ('smtp.das.ufsc.br', '25')

class SockedConnection:
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
            try:
                socked_connection_headnode.send("ping")
            except:
                print "lost connection with headnode. Aborting"
                sys.exit(1)
            data = socked_connection_headnode.recv(4096)
            if not data:
                print "lost connection with headnode. Aborting"
                sys.exit(1)
            ss = select.select
            inputready, outputready, exceptready = ss(self.input_list, [], [], 1)

            for self.s in inputready:
                if self.s == self.socked_listener:
                    self.on_accept()
                    break

                self.data = self.s.recv(buffer_size)
                if len(self.data) == 0:
                    self.on_close()
                else:
                    self.on_recv()

    def on_accept(self):
        print "debug1"
        clientsock, clientaddr = self.socked_listener.accept()
        self.input_list.append(clientsock)
        self.stand_alone = True
        forward = SockedConnection().start(forward_to[0], forward_to[1])
        if forward:
            print clientaddr, "has connected"
            self.input_list.append(forward)
            self.socked_from_to_[clientsock] = forward
            self.socked_from_to_[forward] = clientsock
            self.stand_alone = False
        else:
            self.stand_alone = True
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
        # here we can parse and/or modify the data before send forward
        print data
        if data == "Stop":
            return False
        if not self.stand_alone:
            self.socked_from_to_[self.s].send(data)
        return True


if __name__ == '__main__':
    # test if a connection can be established with the headnode
    socked_connection_headnode = SockedConnection().start('lce072', 9090)


    nodeDeamon = NodeDeamon('', 9090)
    nodeDeamon.head_node_connection = socked_connection_headnode
    try:
        nodeDeamon.main_loop()
        print "Received exit command"
        sys.exit(0)
    except KeyboardInterrupt:
        nodeDeamon.head_node_connection.close()
        print "Ctrl C - Stopping NodeDeamon"
        sys.exit(1)
