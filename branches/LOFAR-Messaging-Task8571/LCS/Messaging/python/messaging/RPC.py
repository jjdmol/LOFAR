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
from lofar.messaging.messagebus import ToBus,FromBus
from lofar.messaging.messages import ServiceMessage,ReplyMessage
import uuid

class RPC():
  def __init__(self,bus,service,timeout=None,ForwardExceptions=None):
     self.timeout=timeout
     self.ForwardExceptions=False
     if (ForwardExceptions==True):
        self.ForwardExceptions=True
     self.BusName=bus
     self.ServiceName=service
     self.Request = ToBus(self.BusName+"/"+self.ServiceName,
            options={"create":"always", "link":"{x-declare: {arguments:{ \"qpid.default_mandatory_topic\": True}}}"})
     self.ReplyAddress="reply."+str(uuid.uuid4())
     self.Reply = FromBus(self.BusName+"/"+self.ReplyAddress)

  def __enter__(self):
     self.Request.open()
     self.Reply.open()
     return self

  def __exit__(self, exc_type, exc_val, exc_tb):
     self.Request.close()
     self.Reply.close()

  def __call__(self,msg,timeout=None):
     if (timeout==None):
       timeout=self.timeout
     MyMsg=ServiceMessage(msg,self.ReplyAddress)
     #MyMsg.reply_to=self.ReplyAddress
     self.Request.send(MyMsg)
     print("sent")
     answer=self.Reply.receive(timeout)
     print("received or timed out")
     if (answer!=None):
        if (isinstance(answer,ReplyMessage)):
           status={}
           exception=None
           try:
              if (answer.status!="OK"):
                 status["state"]=answer.status
                 status["errmsg"]=answer.errmsg
                 status["backtrace"]=answer.backtrace
                 if (answer.exception!=""):
                    exception=pickle.loads(answer.exception)
              else:
                 status="OK"
           except Exception as e:
              status["state"]="ERROR"
              status["errmsg"]="Return state in message not found"
              status["backtrace"]=""
           else:
              if (self.ForwardExceptions==True):
                if (exception!=None):
                  raise exception[0],exception[1],exception[2]
        try:
           answer=(answer.content,status)
        except Exception as e:
           # we can't properly convert to a result message.
           answer=(None,{"ERROR":"Malformed return message"})
     else:
        # if we come here we had a Time-Out
        answer=(None,"RPC Timed out")
     return answer
