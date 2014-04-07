#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (C) 2007
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$


import os
import socket
import sys
import time
import numpy

from threading import Thread

class Server( Thread ):

   def __init__ ( self ):
      Thread.__init__( self )
      self.socket = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
      self.port = 2727
      while True:
         try:
            self.socket.bind ( ( '', self.port ) )
            break
         except socket.error:
            self.port += 1
            
      self.socket.listen ( 3 )
      self.socket.settimeout(1)

   def run(self):
      self.connectionserverlist = []
      self.stop_flag = False
      while not self.stop_flag:
         try:
            connection, details = self.socket.accept()
         except socket.timeout:
            pass
         else:
            connection.settimeout( None )
            connectionserver = ConnectionServer(connection)
            connectionserver.start()
            self.connectionserverlist.append( connectionserver )
   
   def get_results( self ):
      
      self.stop_flag = True
      self.join()
      results = []
      for connectionserver in self.connectionserverlist:
         connectionserver.join()
         results.append( connectionserver.result )
      return results
      
   
class ConnectionServer( Thread ):
   
   def __init__( self, connection ):
      Thread.__init__( self )
      self.connection = connection
      
   def run( self ):
      self.result = []
      while True:
         data = self.connection.recv(1024)
         if not data:
            break
         #Always explicitely send an aknowledgement, this speeds up communication
         self.connection.send('OK')
         self.result.append( data )
      self.connection.close()


def readms(gdsfile, clusterdesc):

   host = socket.gethostname()

   server = Server()
   server.start()
   os.system("startdistproc -useenv -wait -cdn %s -dsn %s -mode %i -masterhost %s -nostartmaster `which readms-part.sh` $PWD" % (clusterdesc, gdsfile, server.port, host))

   results = server.get_results()
   antennas = eval(results[0][1], {})
   positions = eval(results[0][2], {'array': numpy.array})
   pointing = eval(results[0][3], {'array': numpy.array})
   
   return (antennas, positions, pointing)
   
