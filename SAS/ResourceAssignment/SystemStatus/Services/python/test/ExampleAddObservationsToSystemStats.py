#!/usr/bin/env python

from lofar.messaging.RPC import RPC

# Used settings
NumRequests=10
request=1
Test={}
Test["Version Major"]=0
Test["Version Minor"]=1

RPCService=RPC("SystemStateAddObservation",busname="SystemStats",timeout=10)

# loop forever
with RPCService:
 for request in range(NumRequests):
   # send to the RPC Service
   Test["Id"]="LC_%04d" %(request)
   Test["JobID"]=100+request
   Test["NumFiles"]=10


   replymessage=RPCService(Test)

   # show the result from the RPC client
   print replymessage

