#!/usr/bin/python

from lofar.messaging import Service, MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.sas.systemstatus.database.ssdb import SSDB

import logging
import sys
logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

SERVICENAME = "GetServerState"
BUSNAME     = "simpletest"
DATABASE    = "datamonitor"
USER        = "lofarsys"
PASSWORD    = "welkom001"

class DataMonitorQueryService(MessageHandlerInterface):

    def __init__(self,**kwargs):
        super(DataMonitorQueryService,self).__init__(**kwargs)

        self.username = kwargs.pop("username", USER)
        self.password = kwargs.pop("password", PASSWORD)
        self.database = kwargs.pop("database", DATABASE)

        self.service2MethodMap = {
            'GetStateNames':self.getstatenames,
            'GetActiveGroupNames':self.getactivegroupnames,
            'GetHostForGID':self.gethostsforgid,
            'CountHostsForGroups': self.counthostsforgroups,
            'ListAll': self.listall,
            'CountActiveHosts': self.countactivehosts,
            'GetArchivingStatus': self.getArchivingStatus }


    def prepare_loop(self):
        self.ssdb = SSDB(username=self.username,password=self.password,database=self.database)

    def prepare_receive(self):
        self.ssdb.ensure_connected()


    def getstatenames(self):
        qres=self.ssdb.getstatenames()
        names={}
        for i in qres:
            names[str(i['id'])]=i['statename']
        return names

    def getactivegroupnames(self):
        names={}
        qres = self.ssdb.getactivegroupnames()
        for i in qres:
            names[str(i['id'])]=i['groupname']
        return names

    def gethostsforgid(self,gid):
        hosts = self.ssdb.gethostsforgid(gid)
        groups = self.getactivegroupnames()
        ret = { "groupname":groups[str(gid)] , "nodes":hosts }
        return ret

    def counthostsforgroups(self,groups,states):
        qres=self.ssdb.gethostsforgroups()
        ret={}
        for gid,name in groups.iteritems():
            ret[name]={}
            for sid,sname in states.iteritems():
                ret[name][sname]=0
        for row in qres:
            ret[groups[str(row['groupid'])]][states[str(row['statusid'])]]+=1
        return ret

    def listall(self):
        res = self.ssdb.listall()
        nodes={}
        domain={'name':'CEP4','storage':[]}
        ret={'domain':domain,'nodes':nodes}
        try:
            for i in res:
                # new node names need to setup the basic node info first.
                if i['hostname'] not in nodes:
                    nodes[i['hostname']]={'status':i['statusid'],'groupid':i['groupid'],'storage':[]}
                # fill in the node storage info
                nodes[i['hostname']]['storage'].append({'path':i['path'],'totalspace':i['totalspace'],'usedspace':i['usedspace'],'claimedspace':i['claimedspace']})
        except Exception as e:
            # we might not have all the components
            print e
        return ret

    def countactivehosts(self):
        snames=self.getstatenames()
        gnames=self.getactivegroupnames()
        return self.counthostsforgroups(gnames,snames)

    def getArchivingStatus(self,*args,**kwargs):
        maininfo = self.ssdb.getIngestMain()
        jobinfo  = self.ssdb.getIngestJobs()
        return { "main" : maininfo, "jobs" : jobinfo };

def createService(busname=BUSNAME,servicename=SERVICENAME):
    return Service(servicename,DataMonitorQueryService,busname=busname,numthreads=4,use_service_methods=True)

def runservice(busname=BUSNAME,servicename=SERVICENAME):
    with createService(busname,servicename) as GetServerState:
        waitForInterrupt()
