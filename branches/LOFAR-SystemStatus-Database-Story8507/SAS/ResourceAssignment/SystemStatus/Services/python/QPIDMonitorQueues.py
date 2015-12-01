#!/usr/bin/python -u
import time
import os
from qmf.console import Session as QMFSession
from threading import Timer,Thread,Event
import platform
import datetime

from lofar.messaging.messagebus import ToBus
from lofar.messaging.messages import MonitoringMessage


MyHostname=platform.node()

brokerlist= []

if MyHostname.lower() != 'ccu099' :
   brokerlist = [ "amqp://CCU001","amqp://sas001","amqp://lcs023","amqp://cbm001","amqp://mcu001","amqp://lhn001.cep2.lofar","amqp://lhn002.cep2.lofar"]
   for i in range(1,95):
      nodename = 'amqp://locus%(#)03d.cep2.lofar' % {"#": i}
      brokerlist.append(nodename)
else:
    brokerlist = [ "amqp://ccu099","amqp://SAS099","amqp://lcs028","amqp://cbt009","amqp://locus098","amqp://locus099","amqp://locus102","amqp://mcu099"]

last_status="NOTCONNECTED"

def log_message(self, format, *args):
    return


broker = {}
sess = {}

def connect_to_brokers():
    global sess
    global broker
    global brokerlist
    print " Connecting to brokers .."
    for brokername in brokerlist:
        print "   %s ... " %(brokername)
        sess[brokername]=QMFSession(manageConnections=True)
        broker[brokername]=sess[brokername].addBroker(brokername)
        print " connected "

def retrieve_status():
    global brokerlist
    global sess

    queues = {}
    queuestat = {}

    # get current local time
    ts = time.time()
    datestamp = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
    queuestat['datestamp']=datestamp
    for brokername in brokerlist:
        try:
          queues[brokername] = sess[brokername].getObjects(_class="queue", _package="org.apache.qpid.broker")
          queuestat[brokername]={}
        except Exception ,e:
          print("Failed to retrieve from Broker at %s." %(brokername))
          print e

    for brokername,qlist in queues.iteritems():
      for q in qlist:
        queuestat[brokername][q.name]=(q.msgDepth,q.msgTotalDequeues,q.consumerCount)
    stest=str(queuestat).replace("'",'"',1000000).replace('(','[',10000000).replace(')',']',1000000).replace('L','',1000000)
    return stest 

#retrievebusy=False
state={'last':'NOTCONNECTED','retrievebusy':False,'connected':False,'connectbusy':False}

def periodic_retrieve( ):
    global state
    if ((state['connected']==False) and (state['connectbusy']==False)):
       try:
         state['last']="TRYCONNECT"
         state['connectbusy']=True
         connect_to_brokers()
         state['connectbusy']=False
         state['connected']=True
         state['last']="CONNECTION setup"
       except Exception , e:
         print("failed to connect. retrying in 30 seconds")
         print e
         state['connectbusy']=False
         state['last']="RETRYCONNECT"
       return
    if (state['retrievebusy']):
       return
    state['retrievebusy']=True
    try:
        state['last']=retrieve_status()
    except Exception , e:
        state['last']=str(e)
    state['retrievebusy']=False

class perpetualTimer:

   def __init__(self,t,hFunction):
      self.t=t
      self.hFunction = hFunction
      self.thread = Timer(self.t,self.handle_function)

   def handle_function(self):
      self.hFunction()
      self.thread = Timer(self.t,self.handle_function)
      self.thread.start()

   def start(self):
      self.thread.start()

   def cancel(self):
      self.thread.cancel()


if __name__ == '__main__':
    # once for setup
    periodic_retrieve()
    # once to retrieve first dataset
    periodic_retrieve()
    bgproc=perpetualTimer(60,periodic_retrieve)
    bgproc.start()
    with ToBus("notifications") as queuemonitor:
        try:
            while True:
                msg=MonitoringMessage(unicode(state['last'], "utf-8"))
                msg.domain="QueueMonitorPRD"
                queuemonitor.send(msg)
                time.sleep(1)
        except KeyboardInterrupt:
            pass
    bgproc.cancel()

