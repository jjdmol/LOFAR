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

from .messagebus import ToBus,FromBus
from .messages import ReplyMessage,RequestMessage
import threading
import time
import uuid
import sys
import traceback
import logging

logger = logging.getLogger(__name__)

class MessageHandlerInterface(object):
    """
    Interface class for tuning the handling of a message by the Service class.
    The class defines some (placeholders for) functions that the Service class calls
    during the handling of messages. It can for instance be used to maintain a database connection.

    The pseudocode of the Service class is:
    Service(busname, function or from_MessageHandlerInterface_derived_class, ..., HandlerArguments={})

        handler = <from_MessageHandlerInterface_derived_class>(HandlerArguments)
        handler.prepare_loop()
        while alive:
            handler.prepare_receive()
            msg = wait for messages()
            handler.handle_message(msg)
            handler.finalize_handling(handling_result)
        handler.finalize_loop()
    """
    def __init__(self, **kwargs):
        # if you want your subclass to handle multiple services
        # then you can specify for each service which method has to be called
        # In case this map is empty, or the called service is not in this map,
        # then the default handle_message is called
        self.service2MethodMap = {}

    def prepare_loop(self):
        "Called before main processing loop is entered."
        pass

    def prepare_receive(self):
        "Called in main processing loop just before a blocking wait for messages is done."
        pass

    def handle_message(self, msg):
        "Function the should handle the received message and return a result."
        raise NotImplementedError("OOPS! YOU ENDED UP IN THE MESSAGE HANDLER OF THE ABSTRACT BASE CLASS!")

    def finalize_handling(self, successful):
        "Called in the main loop after the result was send back to the requester."
        "@successful@ reflects the state of the handling: true/false"
        pass

    def finalize_loop(self):
        "Called after main processing loop is finished."
        pass


# create service:
class Service(object):
    """
    Service class for registering python functions with a Service name on a message bus.
    create new service with Service(busname, servicename, servicehandler)
    busname <string>       The name of the messagebus (queue or exchange) the service whould listen on.
    servicename <string>   The name that the user should use the invocate the servicehandler.
    servicehandler <...>   May be a function of an class that is derived from the MessageHandlerInterface.
                           The service uses this function or class for the handling of the messages.
    Optional arguments:
       options <dict>          For the QPID connection
       exclusive <bool>        To create eclusive access to this messagebus. Default:True
       numthreads <int>        Amount of threads processing messages. Default:1
       parsefullmessage <bool> Pass full message of only message content to the service handler. Default:False.
       verbose <bool>          Show debug text. Default:False
       handler_args <dict>     Arguments that are passed to the constructor of the servicehandler is case the servicehandler
                               is a class in stead of a function.
    """

    def __init__(self, servicename, servicehandler, **kwargs):
        """
        Initialize Service object with servicename (str) and servicehandler function.
        additional parameters:
            busname= <string>  Name of the bus in case exchanges are used in stead of queues
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: True)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        self.service_name     = servicename
        self.service_handler  = servicehandler
        self.connected        = False
        self.running          = [False]
        self.link_uuid        = str(uuid.uuid4())
        self.busname          = kwargs.pop("busname", None)
        self.exclusive        = kwargs.pop("exclusive", True)
        self._numthreads      = kwargs.pop("numthreads", 1)
        self.verbose          = kwargs.pop("verbose", False)
        self.options          = {"capacity": self._numthreads*20}
        options               = kwargs.pop("options", None)
        self.parsefullmessage = kwargs.pop("parsefullmessage", False)
        self.handler_args     = kwargs.pop("handler_args", {})
        self.listening        = False
        if len(kwargs):
            raise AttributeError("Unexpected argument passed to Service class: %s", kwargs)

        # Set appropriate flags for exclusive binding
        if self.exclusive == True:
            self.options["link"] = '{name:"' + self.link_uuid + \
                                   '", x-bindings:[{key:' + self.service_name + \
                                   ', arguments: {"qpid.exclusive-binding":True}}]}'

        # only add options if it is given as a dictionary
        if isinstance(options,dict):
            for key,val in options.iteritems():
                self.options[key] = val

    def _debug(self, txt):
        """
        Internal use only.
        """
        if self.verbose == True:
            logger.debug("[Service: %s]", txt)

    def start_listening(self, numthreads=None):
        """
        Start the background threads and process incoming messages.
        """ 
        if self.listening == True:
            return

        # Usually a service will be listening on a 'bus' implemented by a topic exchange
        if self.busname != None:
            self.listener  = FromBus(self.busname+"/"+self.service_name, options=self.options)
            self.reply_bus = ToBus(self.busname)
            self.listener.open()
            self.reply_bus.open()
        # Handle case when queues are used
        else:
            # assume that we are listening on a queue and therefore we cannot use a generic ToBus() for replies.
            self.listener = FromBus(self.service_name, options=self.options)
            self.listener.open()
            self.reply_bus=None

        self.connected = True

        if numthreads != None:
            self._numthreads = numthreads

        # use a list to ensure that threads always 'see' changes in the running state.
        self.running = [ True ]
        self._tr = []
        self.reccounter = []
        self.okcounter =[]
        for i in range(self._numthreads):
            # set up service_handler
            if str(type(self.service_handler)) == "<type 'instancemethod'>" or \
               str(type(self.service_handler)) == "<type 'function'>":
                thread_service_handler = MessageHandlerInterface()
                thread_service_handler.handle_message = self.service_handler
            else:
                thread_service_handler = self.service_handler(**self.handler_args)
            if not isinstance(thread_service_handler, MessageHandlerInterface):
                raise TypeError("Servicehandler argument must by a function or a derived class from MessageHandlerInterface.")

            self._tr.append(threading.Thread(target=self._loop, 
                                             kwargs={"index":i, "service_handler":thread_service_handler}))
            self.reccounter.append(0)
            self.okcounter.append(0)
            self._tr[i].start()
        self.listening = True

    def stop_listening(self):
        """
        Stop the background threads that listen to incoming messages.
        """
        # stop all running threads
        if self.running[0] == True:
            self.running[0] = False
            for i in range(self._numthreads):
                self._tr[i].join()
                logger.info("Thread %2d: STOPPED Listening for messages on Bus %s and service name %s." % (i, self.busname, self.service_name))
                logger.info("           %d messages received and %d processed OK." % (self.reccounter[i], self.okcounter[i]))
        self.listening = False
        # close the listeners
        if self.connected == True:
            if isinstance(self.listener, FromBus):
                self.listener.close()
            if isinstance(self.reply_bus, ToBus):
                self.reply_bus.close()
            self.connected = False


    def __enter__(self):
        """
        Internal use only. Handles scope with keyword 'with'
        """
        self.start_listening()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
	"""
	Internal use only. Handles scope with keyword 'with'
	"""
        self.stop_listening()

    def _send_reply(self, replymessage, status, reply_to, errtxt="",backtrace=""):
	"""
	Internal use only. Send a reply message to the RPC client including exception info.
	"""
        # Compose Reply message from reply and status.
        if isinstance(replymessage,ReplyMessage):
            reply_msg = replymessage
        else:
            reply_msg = ReplyMessage(replymessage, reply_to)
        reply_msg.status = status
        reply_msg.errmsg = errtxt
        reply_msg.backtrace = backtrace

        # show the message content if required by the verbose flag.
        if self.verbose == True:
            reply_msg.show()

        # send the result to the RPC client
        if '/' in reply_to:
            # sometimes clients (JAVA) setup the reply_to field as "exchange/key; {options}"
            # make sure we can deal with that.
            reply_address=reply_to.split('/')
            num_parts=len(reply_address)
            reply_busname=reply_address[num_parts-2]
            subject=reply_address[num_parts-1]
            try:
                with ToBus(reply_busname) as dest:
                    # remove any extra field if present
                    if ';' in subject:
                        subject = subject.split(';')[0]
                    reply_msg.subject=subject
                    dest.send(reply_msg)
            except  MessageBusError as e:
                logger.error("Failed to send reply message to reply address %s on messagebus %s." %(subject,reply_busname))
            return

        if isinstance(self.reply_bus,ToBus):
            reply_msg.subject = reply_to
            try:
                self.reply_bus.send(reply_msg)
            except MessageBusError as e:
                logger.error("Failed to send reply message to reply address %s on messagebus %s." %(reply_to,self.busname))
            return
        else:
            # the reply address is not in a default known format
            # and we do not have a default bus destination
            # we will try to deliver the message anyway.
            try:
                with ToBus(reply_to) as dest:
                    dest.send(reply_msg)
            except MessageBusError as e:
                logger.error("Failed to send reply messgage to reply address %s" %(reply_to))


    def _loop(self, **kwargs):
	"""
	Internal use only. Message listener loop that receives messages and starts the attached function with the message content as argument.
	"""
        thread_idx = kwargs.pop("index")
        service_handler = kwargs.pop("service_handler")
        logger.info( "Thread %d START Listening for messages on Bus %s and service name %s." %(thread_idx, self.busname, self.service_name))
        try:
            service_handler.prepare_loop()
        except Exception as e:
            logger.error("prepare_loop() failed with %s", e)

        while self.running[0]:
            try:
                service_handler.prepare_receive()
            except Exception as e:
                logger.error("prepare_receive() failed with %s", e)
                continue

            try:
                # get the next message
                lofar_msg = self.listener.receive(1)
                # retry if timed-out
                if lofar_msg is None:
                    continue

                # report if messages are not Service Messages
                if not isinstance(lofar_msg, RequestMessage):
                    logger.error( "Received wrong messagetype %s, RequestMessage expected." %(str(type(lofar_msg))))
                    self.listener.ack(lofar_msg)
                    continue

                # Keep track of number of received messages
                self.reccounter[thread_idx] += 1

                # Execute the service handler function and send reply back to client
                try:
                    self._debug("Running handler")

                    # determine which handler method has to be called
                    if hasattr(service_handler, 'service2MethodMap') and lofar_msg.subject in service_handler.service2MethodMap:
                        # pass the handling of this message on to the specific method for this service
                        serviceHandlerMethod = service_handler.service2MethodMap[lofar_msg.subject]
                    else:
                        serviceHandlerMethod = service_handler.handle_message

                    if self.parsefullmessage is True:
                        replymessage = serviceHandlerMethod(lofar_msg)
                    else:
                        # check for positional arguments and named arguments
                        # depending on presence of args and kwargs,
                        # the signature of the handler method should vary as well
                        if lofar_msg.has_args and lofar_msg.has_kwargs:
                            # both positional and named arguments
                            # rpcargs and rpckwargs are packed in the content
                            rpcargs = lofar_msg.content

                            # rpckwargs is the last argument in the content
                            # rpcargs is the rest in front
                            rpckwargs = rpcargs[-1]
                            del rpcargs[-1]
                            rpcargs = tuple(rpcargs)
                            replymessage = serviceHandlerMethod(*rpcargs, **rpckwargs)
                        elif lofar_msg.has_args:
                            # only positional arguments
                            # msg.content should be a list
                            rpcargs = tuple(lofar_msg.content)
                            replymessage = serviceHandlerMethod(*rpcargs)
                        elif lofar_msg.has_kwargs:
                            # only named arguments
                            # msg.content should be a dict
                            rpckwargs = lofar_msg.content
                            replymessage = serviceHandlerMethod(**rpckwargs)
                        elif lofar_msg.content:
                            rpccontent = lofar_msg.content
                            replymessage = serviceHandlerMethod(rpccontent)
                        else:
                            replymessage = serviceHandlerMethod()

                    self._debug("finished handler")
                    self._send_reply(replymessage,"OK",lofar_msg.reply_to)
                    self.okcounter[thread_idx] += 1
                    self.listener.ack(lofar_msg)
                    try:
                        service_handler.finalize_handling(True)
                    except Exception as e:
                        logger.error("finalize_handling() failed with %s", e)
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
                    backtrace = ''.join(rawbacktrace).encode('latin-1').decode('unicode_escape')
                    self._debug(backtrace)
                    if self.verbose is True:
                        logger.info("[Service:] Status: %s", str(status))
                        logger.info("[Service:] ERRTXT: %s", str(errtxt))
                        logger.info("[Service:] BackTrace: %s", str( backtrace ))
                    self._send_reply(None, status, lofar_msg.reply_to, errtxt=errtxt, backtrace=backtrace)
                    try:
                        service_handler.finalize_handling(False)
                    except Exception as e:
                        logger.error("finalize_handling() failed with %s", e)
                    continue

            except Exception as e:
                # Unknown problem in the library. Report this and continue.
                excinfo = sys.exc_info()
                logger.error("[Service:] ERROR during processing of incoming message.")
                traceback.print_exception(*excinfo)
                logger.info("Thread %d: Resuming listening on bus %s for service %s" % 
                            (thread_idx, self.busname, self.service_name))

        try:
            service_handler.finalize_loop()
        except Exception as e:
            logger.error("finalize_loop() failed with %s", e)

__all__ = ["Service", "MessageHandlerInterface"]
