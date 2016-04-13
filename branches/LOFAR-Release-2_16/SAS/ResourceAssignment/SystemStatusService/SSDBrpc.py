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

    def getstatenames(self):
        return self.rpc('GetStateNames')

    def getactivegroupnames(self):
        return self.rpc('GetActiveGroupNames')

    def gethostsforgid(self,gid):
        return self.rpc('GetHostForGID', gid=gid)

    def counthostsforgroups(self,groups,states):
        return self.rpc('CountHostsForGroups', groups=groups, states=states)

    def listall(self):
        return self.rpc('ListAll')

    def countactivehosts(self):
        return self.rpc('CountActiveHosts')

    def getArchivingStatus(self,*args,**kwargs):
        return self.rpc('GetArchivingStatus')


# test code for all methods
if __name__ == '__main__':
    import pprint

    with SSDBRPC(broker='10.149.96.22') as ssdb:
        print '\n------------------'
        print 'getstatenames'
        states = ssdb.getstatenames()
        pprint.pprint(states)


        print '\n------------------'
        print 'getactivegroupnames'
        groups = ssdb.getactivegroupnames()
        pprint.pprint(ssdb.getactivegroupnames())

        for gid, groupname in groups.items():
            print '\n------------------'
            print 'gethostsforgid'
            pprint.pprint(ssdb.gethostsforgid(gid))

        for gid, groupname in groups.items():
            for sid, statename in states.items():
                print '\n------------------'
                print 'counthostsforgroups'
                pprint.pprint(ssdb.counthostsforgroups({gid:groupname}, {sid:statename}))

        print '\n------------------'
        print 'listall'
        pprint.pprint(ssdb.listall())

        print '\n------------------'
        print 'countactivehosts'
        pprint.pprint(ssdb.countactivehosts())

        print '\n------------------'
        print 'getArchivingStatus'
        pprint.pprint(ssdb.getArchivingStatus())

