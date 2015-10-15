#!/usr/bin/python

from lofar.messaging.messagebus import ToBus,FromBus
from lofar.messaging.messages import EventMessage
import threading
import time
import uuid

# create service:
class Service():

   def __init__(self,BusName,ServiceName,ServiceHandler,exclusive=True,numthreads=1,Verbose=False):
      self.BusName=BusName
      self.ServiceName=ServiceName
      self.ServiceHandler=ServiceHandler
      self.connected=False
      self.running=False
      self.exclusive=exclusive
      self.link_uuid=str(uuid.uuid4())
      self._numthreads=numthreads
      self.Verbose=Verbose
      if (self.BusName!=None):
         self.Listen=FromBus(self.BusName+"/"+self.ServiceName,options={"link":"{name:two, x-bindings:[{key:" + self.ServiceName + ", arguments: {\"qpid.exclusive-binding\":True}}]}","capacity":numthreads*20})
         self.Reply=ToBus(self.BusName)
      else:
         self.Listen=FromBus(self.ServiceName)
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
      self.Reply.open()
      return self

   def __exit__(self, exc_type, exc_val, exc_tb):
      self.Listen.close()
      self.Reply.close()

   def loop(self,index):
      print( "Listening for messages on Bus %s and service name %s." %(self.BusName,self.ServiceName)) 
      #with self.Listen,self.Reply:
      while self.running:
         msg=None
         try:
           # get the next message
           msg=self.Listen.receive(1)
         except Exception as e:
           print e

         if (msg!=None):
             status="unknown"
             self.counter[index]+=1
             #print "got a message"
             # create a reply message using the ToUpper conversion
             replymessage=""
             try:
                replymessage=self.ServiceHandler(msg.content)
                status="OK"
             except Exception as e:
                print e
                replymessage=""
                status=str(e)
             # ensure to deliver at the destination in the reply_to field
             # send the result to the RPC client
             ToSend=EventMessage(replymessage)
             ToSend.status=status
             ToSend.subject=msg.reply_to
             if (self.Verbose):
               msg.show()
               ToSend.show()
             self.Reply.send(ToSend)
             self.Listen.ack(msg)

      print("STOPPED Listening for messages on Bus %s and service name %s and %d processed." %(self.BusName,self.ServiceName,self.counter[index]))

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
