#!/usr/bin/env python

import sys
print >>sys.stderr, "QPID support NOT enabled! Will NOT connect to any broker, and messages will be lost!"

def uuid4():
  return "<uuid>"


"""
  Exceptions.
"""

class MessagingError(Exception):
  pass

class SessionError(Exception):
  pass

class exceptions:
  class Timeout(Exception):
    pass

  class Empty(Exception):
    pass

"""
  Messages.
"""

class Message(object):
  def __init__(self, content_type, durable):
    self.content = ""

"""
  Communication.
"""

class Sender(object):
  def __init__(self, dest):
    self.dest = dest

  def send(self, msg):
    pass

class Receiver(object):
  def __init__(self, source):
    self.capacity = 0
    self.source = source

  def fetch(self):
    return None

class Session(object):
  def sender(self, address):
    return Sender(address)

  def receiver(self, address):
    return Receiver(address)

  def next_receiver(self, timeout=0):
    return Receiver("unknown")

  def acknowledge(self, msg):
    pass

class Connection(object):
  def __init__(self, broker):
    self.reconnect = False

  def open(self):
    pass

  def close(self, timeout=0):
    pass

  def session(self):
    return Session()

