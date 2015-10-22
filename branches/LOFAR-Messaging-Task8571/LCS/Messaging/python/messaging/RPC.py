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


class RPC():
    def __init__(self, bus, service, timeout=None, ForwardExceptions=None, Verbose=None):
        self.timeout = timeout
        self.ForwardExceptions = False
        self.Verbose = False
        if ForwardExceptions is True:
            self.ForwardExceptions = True
        if Verbose is True:
            self.Verbose = True
        self.BusName = bus
        self.ServiceName = service
        self.Request = ToBus(self.BusName + "/" + self.ServiceName)
        self.ReplyAddress = "reply." + str(uuid.uuid4())
        self.Reply = FromBus(self.BusName + "/" + self.ReplyAddress)

    def __enter__(self):
        self.Request.open()
        self.Reply.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.Request.close()
        self.Reply.close()

    def __call__(self, msg, timeout=None):
        if timeout is None:
            timeout = self.timeout

        MyMsg = ServiceMessage(msg, self.ReplyAddress)
        self.Request.send(MyMsg)
        answer = self.Reply.receive(timeout)

        # Check for Time-Out
        if answer is None:
            status = []
            status["state"] = "TIMEOUT"
            status["errmsg"] = "RPC Timed out"
            status["backtrace"] = ""
            return (None, status)

        # Check for illegal message type
        if isinstance(answer, ReplyMessage) is False:
            # if we come here we had a Time-Out
            status = []
            status["state"] = "ERROR"
            status["errmsg"] = "Incorrect messagetype (" + str(type(answer)) + ") received."
            status["backtrace"] = ""
            return (None, status)

        # return content and status if status is 'OK'
        if (answer.status == "OK"):
            return (answer.content, answer.status)

        # Compile error handling from status
        status = {}
        try:
            status["state"] = answer.status
            status["errmsg"] = answer.errmsg
            status["backtrace"] = answer.backtrace
        except Exception as e:
            status["state"] = "ERROR"
            status["errmsg"] = "Return state in message not found"
            status["backtrace"] = ""
            return (Null, status)

        # Does the client expect us to throw the exception?
        if self.ForwardExceptions is True:
            excep_mod = __import__("exceptions")
            excep_class_ = getattr(excep_mod, answer.errmsg.split(':')[0], None)
            if (excep_class_ != None):
                instance = excep_class_(answer.backtrace)
                raise (instance)
            else:
                raise (Exception(answer.errmsg))
        return (None,status)
