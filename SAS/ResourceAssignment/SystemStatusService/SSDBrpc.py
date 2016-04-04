#!/usr/bin/python

from lofar.messaging.RPC import RPC, RPCException, RPCWrapper

import logging
import sys
logger = logging.getLogger(__name__)

from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_BUSNAME
from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_SERVICENAME

class SSDBRPC(RPCWrapper):
    def __init__(self,
                 busname=DEFAULT_SSDB_BUSNAME,
                 servicename=DEFAULT_SSDB_SERVICENAME,
                 broker=None):
        super(SSDBRPC, self).__init__(busname, servicename, broker)

    def getResourceClaimStatuses(self):
        return self.rpc('GetResourceClaimStatuses')

    def getstatenames(self):
        return self.rpc('GetStateNames')

    def getactivegroupnames(self):
        return self.rpc('GetActiveGroupNames')

    def gethostsforgid(self,gid):
        return self.rpc('GetHostForGID', gid=gid)

    def counthostsforgroups(self,groups,states):
        return self.rpc('CountHostsForGroups')

    def listall(self):
        return self.rpc('ListAll')

    def countactivehosts(self):
        return self.rpc('CountActiveHosts')

    def getArchivingStatus(self,*args,**kwargs):
        return self.rpc('GetArchivingStatus')

