#!/usr/bin/python
# Service.py: Service definition for the lofar.messaging module.
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#

from lofar.messaging.messagebus import ToBus,FromBus
from lofar.messaging.messages import EventMessage,ServiceMessage
import threading
import time
import uuid
import sys
import traceback

# create service:
class Service():
   """
   Service class for registering python functions with a Service name on a messgage bus.
   """
   def __init__(self,BusName,ServiceName,ServiceHandler,options=None,exclusive=True,numthreads=1,Verbose=False):
      self.BusName=BusName
      self.ServiceName=ServiceName
      self.ServiceHandler=ServiceHandler
      self.connected=False
      self.running=False
      self.exclusive=exclusive
      self.link_uuid=str(uuid.uuid4())
      self._numthreads=numthreads
      self.Verbose=Verbose
      self.options={}
      self.options["capacity"]=numthreads*20
      if (self.exclusive==True):
         self.options["link"]="{name:\""+self.link_uuid+"\", x-bindings:[{key:" + self.ServiceName + ", arguments: {\"qpid.exclusive-binding\":True}}]}"
      if (isinstance(options,dict)):
         for key,val in options.iter_items():
            self.options[key]=val
      if (self.BusName!=None):
         self.Listen=FromBus(self.BusName+"/"+self.ServiceName,options=self.options)
         self.Reply=ToBus(self.BusName)
      else:
         self.Listen=FromBus(self.ServiceName,options=self.options)
         self.Reply=self.replyto
 
   def StartListening(self,numthreads=None):
      if (numthreads!=None):
         self._numthreads=numthreads
      numthreads=self._numthreads
      self.connected=True
      self.running=True
      self._tr=[]
      self.counter=[]
      for i in range(self._numthreads):
             self._tr.append(threading.Thread(target=self.loop,args=[i]))
             self.counter.append(0)
             self._tr[i].start()
   
   def __enter__(self):
      self.Listen.open()
      if (isinstance(self.Reply,ToBus)):
         self.Reply.open()
      return self

   def __exit__(self, exc_type, exc_val, exc_tb):
      self.Listen.close()
      if (isinstance(self.Reply,ToBus)):
         self.Reply.close()

   def loop(self,index):
      print( "Thread %d START Listening for messages on Bus %s and service name %s." %(index,self.BusName,self.ServiceName)) 
      #with self.Listen,self.Reply:
      while self.running:
         msg=None
         try:
           # get the next message
           msg=self.Listen.receive(1)
         except Exception as e:
           print e

         try:

           if (isinstance(msg,ServiceMessage)):
             status="unknown"
             self.counter[index]+=1
             #print "got a message"
             # create a reply message using the ToUpper conversion
             replymessage=""
             try:
                print status
                replymessage=self.ServiceHandler(msg.content)
                status="OK"
             except Exception as e:
                print status
                exc_type, exc_value, exc_traceback = sys.exc_info()
                errtxt=traceback.format_exception(exc_type, exc_value, exc_traceback)
                #del errtxt[1]
                status="ERROR: "+''.join(errtxt).encode('latin-1').decode('unicode_escape')
                if self.Verbose:
                  print status
                replymessage=None
             # ensure to deliver at the destination in the reply_to field
             # send the result to the RPC client
             print status
             ToSend=EventMessage(replymessage)
             ToSend.status=status
             ToSend.subject=msg.reply_to
             if (self.Verbose):
               msg.show()
               ToSend.show()
             if (isinstance(self.Reply,ToBus)):
               self.Reply.send(ToSend)
             else:
               dest=ToBus(self.Reply)
               with dest:
                  dest.send(ToSend)
             self.Listen.ack(msg)
           else:
             if (msg!=None):
                print "Received wrong messagetype %s" %(str(type(msg)))
         except Exception as e:
            excinfo = sys.exc_info()
            print "ERROR during processing of incoming message."
            traceback.print_exception(*excinfo)
            print "Thread %d: Resuming listening on bus %s for service %s" %(index,self.BusName,self.ServiceName)

      print("Thread %d STOPPED Listening for messages on Bus %s and service name %s and %d processed." %(index,self.BusName,self.ServiceName,self.counter[index]))

   def StopListening(self):
      if (self.running):
           self.running=False
           for i in range(self._numthreads):
             self._tr[i].join()
      if (self.connected):
           self.connected=False
           self.Listen.close()
           self.Reply.close()

   def WaitForInterrupt(self):
      looping=True
      while looping:
         try:
           time.sleep(100)
         except (KeyboardInterrupt): 
           looping=False
           print("Keyboard interrupt received.")
