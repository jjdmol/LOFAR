#!/usr/bin/python
# This is a simple port-forward / proxy, written using only the default python
# library. If you want to make a suggestion or fix something you can contact-me
# at voorloop_at_gmail.com
# Distributed over IDC(I Don't Care) license
import socket
import select
import time
import sys

class SockedConnection:
    def __init__(self):
        self.forward = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def start(self, host, port):
        try:
            self.forward.connect((host, port))
            self.forward.send("VBLALAKLJHS")
            return self.forward
        except Exception, e:
            print e
            return False


if __name__ == '__main__':
        client = SockedConnection().start('localhost', 9090)

