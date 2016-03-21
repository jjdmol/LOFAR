"""
   GSSAPI kerberos token manipulation.

   Two classes: Client and Server

   Usage:
  
      Client creates instance of client_token("service")


            TokenManager = client_token("qpidd")

            Token = TokenManager.get_token()

            ... Send token to service ...

            ... Incoming token form service can be verified using: ...

            TokenManager.validate_token(IncomingToken)


      service creates instance of service_token("service")


            TokenManager = service_token("service")
           
            ... On receiving a token from a client ...

"""

import gssapi
import base64

class client_token:
    def __init__(self,servicename):
        self.service_name = gssapi.Name(servicename, gssapi.C_NT_HOSTBASED_SERVICE)
        # Create an InitContext targeting the demo service
        self.ctx = gssapi.InitContext(self.service_name)

    def proc_token(self,in_token):
        self.in_token=None
        if in_token:
           self.in_token=base64.b64decode(in_token)
        return base64.b64encode(self.ctx.step(self.in_token))

    def get_token(self):
        return self.proc_token(None)

    def validate_token(self,token):
        ret=self.proc_token(token)
        if self.ctx.established:
           print "Token Accepted"
        else:
           print "Token Rejected"
        return ret

class service_token:
    def __init__(self,servicename):
        self.service_name = gssapi.Name(servicename, gssapi.C_NT_HOSTBASED_SERVICE)
        self.server_cred = gssapi.Credential(self.service_name, usage=gssapi.C_ACCEPT)
        self.ctx = gssapi.AcceptContext(self.server_cred)
        

    def validate_token(self,token):
        self.ctx = gssapi.AcceptContext(self.server_cred)
        in_token=base64.b64decode(token)
        ret=self.ctx.step(in_token)
        if self.ctx.established:
           print "Token Accepted"
        else:
           print "Token Rejected"
        if ret:
           ret=base64.b64encode(ret)
        return ret

