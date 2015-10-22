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
from lofar.messaging.messages import ReplyMessage,ServiceMessage
import threading
import time
import uuid
import sys
import traceback
import pickle


# create service:
class Service:
    """
    Service class for registering python functions with a Service name on a messgage bus.
    """
    def __init__(self, busname, servicename, servicehandler, options=None, exclusive=True, numthreads=1, verbose=False):
        self.BusName = busname
        self.ServiceName = servicename
        self.ServiceHandler = servicehandler
        self.connected = False
        self.running = False
        self.exclusive = exclusive
        self.link_uuid = str(uuid.uuid4())
        self._numthreads = numthreads
        self.Verbose = verbose
        self.options = {"capacity": numthreads*20}
        # Set appropriate flags for exclusive binding
        if self.exclusive is True:
            self.options["link"] = '{name:"' + self.link_uuid + '", x-bindings:[{key:' + self.ServiceName + ', arguments: {"qpid.exclusive-binding":True}}]}'
        # only add options if it is given as a dictionary
        if isinstance(options,dict):
            for key,val in options.iteritems():
                self.options[key] = val
        # Usually a service will be listening on a 'bus' implemented by a topic exchange
        if self.BusName is not None:
            self.Listen = FromBus(self.BusName+"/"+self.ServiceName,options=self.options)
            self.Reply = ToBus(self.BusName)
        else:
            # assume that we are listening on a queue and therefore we cannot use a generic ToBus() for replies.
            self.Listen = FromBus(self.ServiceName,options=self.options)
            self.Reply=self.replyto

    def _debug(self, txt):
        if self.Verbose is True:
            print(txt)

    def StartListening(self, numthreads=None):
        if numthreads is not None:
            self._numthreads = numthreads
        self.connected = True
        self.running = True
        self._tr = []
        self.reccounter = []
        self.okcounter =[]
        for i in range(self._numthreads):
            self._tr.append(threading.Thread(target=self.loop, args=[i]))
            self.reccounter.append(0)
            self.okcounter.append(0)
            self._tr[i].start()

    def __enter__(self):
        if isinstance(self.Listen,FromBus):
            self.Listen.open()
        if isinstance(self.Reply,ToBus):
            self.Reply.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.StopListening()

    def send_reply(self, replymessage, status, reply_to, errtxt="",backtrace=""):
        # Compose Reply message from reply and status.
        ToSend = ReplyMessage(replymessage, reply_to)
        ToSend.status = status
        ToSend.errmsg = errtxt
        ToSend.backtrace = backtrace

        # show the message content if required by the Verbose flag.
        if self.Verbose is True:
            msg.show()
            ToSend.show()

        # send the result to the RPC client
        if isinstance(self.Reply,ToBus):
            self.Reply.send(ToSend)
        else:
            with ToBus(self.Reply) as dest:
                dest.send(ToSend)


    def loop(self, index):
        print( "Thread %d START Listening for messages on Bus %s and service name %s." %(index, self.BusName, self.ServiceName))
        while self.running:
            try:
                # get the next message
                msg = self.Listen.receive(1)
                # loop until we get a valid message.
                if msg is None:
                    continue

                # report if messages are not Service Messages
                if isinstance(msg, ServiceMessage) is not True:
                    print "Received wrong messagetype %s, ServiceMessage expected." %(str(type(msg)))
                    self.Listen.ack(msg)
                    continue

                # Keep track of number of received messages
                self.reccounter[index] += 1

                # Execute the service handler function and send reply back to client
                try:
                    self._debug("Running handler")
                    replymessage = self.ServiceHandler(msg.content)
                    self._debug("finished handler")
                    self.send_reply(replymessage,"OK",msg.reply_to)
                    self.okcounter[index] += 1
                    self.Listen.ack(msg)
                    continue

                except Exception as e:
                    # Any thrown exceptions either Service exception or unhandled exception
                    # during the execution of the service handler is caught here.
                    self._debug("handling exception")
                    exc_info = sys.exc_info()
                    status="ERROR"
                    rawbacktrace = traceback.format_exception(*exc_info)
                    errtxt = rawbacktrace[-1]
                    self._debug(rawbacktrace)
                    # cleanup the backtrace print by removing the first two lines and the last
                    del rawbacktrace[1]
                    del rawbacktrace[0]
                    del rawbacktrace[-1]
                    backtrace= ''.join(rawbacktrace).encode('latin-1').decode('unicode_escape')
                    self._debug(backtrace)
                    if self.Verbose is True:
                        print status
                        print errtxt
                        print backtrace
                    self.send_reply(None,status,msg.reply_to,errtxt=errtxt,backtrace=backtrace)


            except Exception as e:
                # Unknown problem in the library. Report this and continue.
                excinfo = sys.exc_info()
                print "ERROR during processing of incoming message."
                traceback.print_exception(*excinfo)
                print "Thread %d: Resuming listening on bus %s for service %s" %(index,self.BusName,self.ServiceName)

        print("Thread %2d: STOPPED Listening for messages on Bus %s and service name %s." %(index,self.BusName,self.ServiceName))
        print("           %d messages received and %d processed OK." %(self.reccounter[index],self.okcounter[index]))

    def StopListening(self):
        # stop all running threads
        if self.running is True:
            self.running = False
            for i in range(self._numthreads):
                self._tr[i].join()

        # possibly doubly defined..
        if self.connected is True:
            self.connected = False
            if isinstance(self.Listen, FromBus):
                self.Listen.close()
            if isinstance(self.Reply, ToBus):
                self.Reply.close()

    def WaitForInterrupt(self):
        looping = True
        while looping:
            try:
                time.sleep(100)
            except KeyboardInterrupt:
                looping = False
                print("Keyboard interrupt received.")
