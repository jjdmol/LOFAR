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
forward_to = ('smtp.das.ufsc.br', '25')

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

                self.data = self.s.recv(buffer_size)
                if len(self.data) == 0:
                    self.on_close()
                else:
                    self.on_recv()

    def on_accept(self):
        forward = Forward().start(forward_to[0], forward_to[1])
        clientsock, clientaddr = self.socked_listener.accept()

        if forward:
            print clientaddr, "has connected"
            self.input_list.append(clientsock)
            self.input_list.append(forward)
            self.socked_from_to_[clientsock] = forward
            self.socked_from_to_[forward] = clientsock
        else:
            print "Can't establish connection with remote socked_listener.",
            print "Closing connection with client side", clientaddr
            clientsock.close()

    def on_close(self):
        print self.s.getpeername(), "has disconnected"
        #remove objects from input_list
        self.input_list.remove(self.s)
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
        self.socked_from_to_[self.s].send(data)
        return True


if __name__ == '__main__':
        nodeDeamon = NodeDeamon('', 9090)
        try:
            nodeDeamon.main_loop()
            print "Received exit command"
            sys.exit(0)
        except KeyboardInterrupt:
            print "Ctrl C - Stopping NodeDeamon"
            sys.exit(1)
