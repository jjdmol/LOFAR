#!/usr/bin/python

from lofar.messaging import RPC

import logging
import sys
logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

SERVICENAME = "GetServerState"
BUSNAME     = "simpletest"

class SSDBrpc:

    def __init__(self,**kwargs):
        self.servicename = kwargs.pop("servicename", SERVICENAME)
        self.busname = kwargs.pop("busname",BUSNAME)
        self.timeout = kwargs.pop("timeout",None)

    def getstatenames(self):
        servicename="%s.GetStateNames" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb()

    def getactivegroupnames(self):
        servicename="%s.GetActiveGroupNames" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb()

    def gethostsforgid(self,gid):
        servicename="%s.GetHostForGID" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb(gid)

    def counthostsforgroups(self,groups,states):
        servicename="%s.CountHostsForGroups" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb(groups,states)

    def listall(self):
        servicename="%s.ListAll" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb()

    def countactivehosts(self):
        servicename="%s.CountActiveHosts" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb()

    def getArchivingStatus(self,*args,**kwargs):
        servicename="%s.GetArchivingStatus" %(self.servicename)
        with RPC(servicename,busname=self.busname,timeout=self.timeout) as ssdb:
            return ssdb(*args,**kwargs)
