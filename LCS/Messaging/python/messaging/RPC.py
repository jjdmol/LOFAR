# RPC.py: RPC client side used by the lofar.messaging module.
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

#  RPC invocation with possible timeout
from lofar.messaging.messagebus import ToBus, FromBus
from lofar.messaging.messages import ServiceMessage, ReplyMessage
import uuid

class RPCException(Exception):
    "Exception occured in the RPC code itself, like time-out, invalid message received, etc."
    pass

class RPC():
    """
    This class provides an easy way to invoke a Remote Rrocedure Call to a
    Services on the message bus.

    Note that most methods require that the RPC object is used *inside* a
    context. When entering the context, the connection with the broker is
    opened, and a session and a sender are created. For each rpc invocation
    a new unique reply context is created as to avoid cross talk.
    When exiting the context the connection to the broker is closed.
    As a side-effect the sender and session are destroyed.

    """
    def __init__(self, service, **kwargs ): #busname=None, timeout=None, ForwardExceptions=None, Verbose=None):
	"""
	Initialize an Remote procedure call using:
	    service= <str>    Service Name
	        bus=     <str>    Bus Name
            timeout= <float>  Time to wait in seconds before the call is considered a failure.
            Verbose= <bool>   If True output extra logging to stdout.

        Use with extra care: ForwardExceptions= <bool>
	    This enables forwarding exceptions from the server side tobe raised at the client side durting RPC invocation.
	"""
        self.timeout           = kwargs.pop("timeout",None)
        self.ForwardExceptions = kwargs.pop("ForwardExceptions",False)
        self.Verbose           = kwargs.pop("Verbose",False)
        self.BusName           = kwargs.pop("busname",None)
        self.ServiceName = service
        if self.BusName is None:
            self.Request = ToBus(self.ServiceName)
        else:
            self.Request = ToBus(self.BusName + "/" + self.ServiceName)
        if len(kwargs):
            raise AttributeError("Unexpected argument passed to RPC class: %s", kwargs)

    def __enter__(self):
	"""
	Internal use only. (handles scope 'with')
	"""
        self.Request.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
	"""
	Internal use only. (handles scope 'with')
	"""
        self.Request.close()

    def __call__(self, *msg, **kwargs):
	"""
	Enable the use of the object to directly invoke the RPC.

        example:

	    with RPC(bus,service) as myrpc:
                result=myrpc(request)
 
	"""
        timeout= kwargs.pop("timeout",self.timeout)
        
        
        Content=list(msg)
        HasKwArgs=(len(kwargs)>0)
        # more than one argument given? 
        HasArgs=(len(msg)> 1 ) or (( len(kwargs)>0 ) and (len(msg)>0))
        if HasArgs:
            # convert arguments to list
            Content = list(msg)
            if HasKwArgs:
                # if both positional and named arguments then
                # we add the kwargs dictionary as the last item in the list
                Content.append(kwargs)
        else:
            if HasKwArgs:
                # we have only one named argument
                Content=kwargs
            else:
                # we have only one positional argument
                Content=Content[0]
        # create unique reply address for this rpc call
        options={'create':'always','delete':'receiver'}
        ReplyAddress= "reply." + str(uuid.uuid4())
        if self.BusName is None:
            Reply = FromBus(ReplyAddress+" ; "+str(options))
        else:
            Reply = FromBus(self.BusName + "/" + ReplyAddress)
        with Reply:
            MyMsg = ServiceMessage(Content, ReplyAddress , has_args=HasArgs, has_kwargs=HasKwArgs)
            MyMsg.ttl = timeout
            self.Request.send(MyMsg)
            answer = Reply.receive(timeout)

        status = {}
        # Check for Time-Out
        if answer is None:
            status["state"] = "TIMEOUT"
            status["errmsg"] = "RPC Timed out"
            status["backtrace"] = ""
            raise RPCException(status)

        # Check for illegal message type
        if isinstance(answer, ReplyMessage) is False:
            # if we come here we had a Time-Out
            status["state"] = "ERROR"
            status["errmsg"] = "Incorrect messagetype (" + str(type(answer)) + ") received."
            status["backtrace"] = ""
            raise RPCException(status)

        # return content and status if status is 'OK'
        if (answer.status == "OK"):
            return (answer.content, answer.status)

        # Compile error handling from status
        try:
            status["state"] = answer.status
            status["errmsg"] = answer.errmsg
            status["backtrace"] = answer.backtrace
        except Exception as e:
            status["state"] = "ERROR"
            status["errmsg"] = "Return state in message not found"
            status["backtrace"] = ""
            raise RPCException(status)

        # Does the client expect us to throw the exception?
        if self.ForwardExceptions is True:
            excep_mod = __import__("exceptions")
            excep_class_ = getattr(excep_mod, answer.errmsg.split(':')[0], None)
            if (excep_class_ != None):
                instance = excep_class_(answer.backtrace)
                raise (instance)
            else:
                raise RPCException(answer.errmsg)
        return (None, status)

__all__ = ["RPC", "RPCException"]
