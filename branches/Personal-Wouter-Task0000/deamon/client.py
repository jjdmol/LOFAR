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
delay = 0.0001
forward_to = ('smtp.das.ufsc.br', '25')

class Forward:
    def __init__(self):
        self.forward = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def start(self):
        try:

            self.forward.connect(("localhost", 9090))
            self.forward.send("")
            return self.forward
        except Exception, e:
            print e
            return False


if __name__ == '__main__':
        client = Forward().start()

