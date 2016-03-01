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
from .messagebus import ToBus, FromBus
from .messages import RequestMessage, ReplyMessage
import uuid

def _analyze_args(args,kwargs):
    HasKwArgs=(len(kwargs)>0)
    # more than one argument given?
    HasMultipleArgs=(len(args)> 1 ) or (( len(kwargs)>0 ) and (len(args)>0))
    return (HasMultipleArgs,HasKwArgs)

def _args_as_content(*args,**kwargs):
    """
    Convert positional args and named args into a message body.
    :param msg: Message to be converted into a Qpid message.
    :return: Qpid message
    :raise InvalidMessage if `msg` cannot be converted into a Qpid message.
    """
    if len(args) == 0 and len(kwargs) == 0:
        return None

    HasMultipleArgs,HasKwArgs = _analyze_args(args, kwargs)
    if HasMultipleArgs:
        # convert arguments to list
        Content = list(args)
        if HasKwArgs:
            # if both positional and named arguments then
            # we add the kwargs dictionary as the last item in the list
            Content.append(kwargs)
        return Content
    if HasKwArgs:
        # we have only one named argument
        return kwargs
    # we have only one positional argument
    return list(args)[0]

class RPCException(Exception):
    "Exception occured in the RPC code itself, like time-out, invalid message received, etc."
    pass

class RPCTimeoutException(RPCException):
    "Exception occured when the RPC call times out."
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
    def __init__(self, service, broker=None, **kwargs ):
        """
        Initialize an Remote procedure call using:
            service= <str>    Service Name
            busname= <str>    Bus Name
            broker= <str>     qpid broker, default None which is localhost
            timeout= <float>  Time to wait in seconds before the call is considered a failure.
            Verbose= <bool>   If True output extra logging to stdout.

        Use with extra care: ForwardExceptions= <bool>
            This enables forwarding exceptions from the server side tobe raised at the client side durting RPC invocation.
        """
        self.timeout           = kwargs.pop("timeout", None)
        self.ForwardExceptions = kwargs.pop("ForwardExceptions", False)
        self.Verbose           = kwargs.pop("Verbose", False)
        self.BusName           = kwargs.pop("busname", None)
        self.ServiceName       = service
        self.broker            = broker
        if self.BusName is None:
            self.Request = ToBus(self.ServiceName, broker=self.broker)
        else:
            self.Request = ToBus("%s/%s" % (self.BusName, self.ServiceName), broker=self.broker)
        if len(kwargs):
            raise AttributeError("Unexpected argument passed to RPC class: %s" %( kwargs ))

    def open(self):
        """
        Start accepting requests.
        """

        self.Request.open()

    def close(self):
        """
        Stop accepting requests.
        """

        self.Request.close()

    def __enter__(self):
        """
        Internal use only. (handles scope 'with')
        """
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        Internal use only. (handles scope 'with')
        """
        self.close()

    def __call__(self, *args, **kwargs):
        """
        Enable the use of the object to directly invoke the RPC.

        example:

            with RPC(bus,service) as myrpc:
                result=myrpc(request)

        """
        timeout = kwargs.pop("timeout", self.timeout)
        Content = _args_as_content(*args, **kwargs)
        HasArgs, HasKwArgs = _analyze_args(args, kwargs)
        # create unique reply address for this rpc call
        options = {'create':'always','delete':'receiver'}
        ReplyAddress = "reply.%s" % (str(uuid.uuid4()))
        if self.BusName is None:
            Reply = FromBus("%s ; %s" %(ReplyAddress,str(options)), broker=self.broker)
        else:
            Reply = FromBus("%s/%s" % (self.BusName, ReplyAddress), broker=self.broker)
        with Reply:
            MyMsg = RequestMessage(content=Content, reply_to=ReplyAddress, has_args=HasArgs, has_kwargs=HasKwArgs)
            MyMsg.ttl = timeout
            self.Request.send(MyMsg)
            answer = Reply.receive(timeout)

        status = {}
        # Check for Time-Out
        if answer is None:
            status["state"] = "TIMEOUT"
            status["errmsg"] = "RPC Timed out"
            status["backtrace"] = ""
            raise RPCTimeoutException(status)

        # Check for illegal message type
        if isinstance(answer, ReplyMessage) is False:
            # if we come here we had a Time-Out
            status["state"] = "ERROR"
            status["errmsg"] = "Incorrect messagetype (%s) received." % (str(type(answer)))
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
