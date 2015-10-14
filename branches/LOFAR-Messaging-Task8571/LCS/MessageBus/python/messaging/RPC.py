
#  RPC invocation with possible timeout
from messaging.messagebus import ToBus,FromBus
from messaging.messages import ServiceMessage

import uuid


class RPC():
  def __init__(self,bus,service,timeout=None):
     self.timeout=timeout
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

  def __call__(self,msg,timeout=-1):
     if (timeout==-1):
       timeout=self.timeout
     MyMsg=ServiceMessage(msg)
     MyMsg.reply_to=self.ReplyAddress
     self.Request.send(MyMsg) #ServiceMessage(MyMsg) #msg,reply_to=self.ReplyAddress))
     answer=self.Reply.receive(timeout)
     try:
        answer=(answer.content,answer.status)
     except Exception as e:
        answer=("","Malformed return message")
     return answer
