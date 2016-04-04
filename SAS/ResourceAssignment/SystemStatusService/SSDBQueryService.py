#!/usr/bin/python

from lofar.messaging import Service, MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.sas.systemstatus.database.ssdb import SSDB
from optparse import OptionParser
from lofar.common import dbcredentials
from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_BUSNAME
from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_SERVICENAME

import logging
import sys
logger = logging.getLogger(__name__)

class DataMonitorQueryService(MessageHandlerInterface):

    def __init__(self,**kwargs):
        super(DataMonitorQueryService,self).__init__(**kwargs)

        self.dbcreds = kwargs.pop("dbcreds", None)

        self.service2MethodMap = {
            'GetStateNames':self.getstatenames,
            'GetActiveGroupNames':self.getactivegroupnames,
            'GetHostForGID':self.gethostsforgid,
            'CountHostsForGroups': self.counthostsforgroups,
            'ListAll': self.listall,
            'CountActiveHosts': self.countactivehosts,
            'GetArchivingStatus': self.getArchivingStatus }


    def prepare_loop(self):
        self.ssdb = SSDB(dbcreds=self.dbcreds)

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

def createService(busname=DEFAULT_SSDB_BUSNAME,servicename=DEFAULT_SSDB_SERVICENAME, dbcreds=None, broker=None):
    return Service(servicename,
                   DataMonitorQueryService,
                   busname=busname,
                   numthreads=1,
                   broker=broker,
                   handler_args={'dbcreds': dbcreds},
                   use_service_methods=True)

def main():
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the systemstatus database service')
    parser.add_option('-q', '--broker', dest='broker', type='string',
                      default=None,
                      help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string",
                      default=DEFAULT_SSDB_BUSNAME,
                      help="Name of the bus exchange on the qpid broker. [default: %default]")
    parser.add_option("-s", "--servicename", dest="servicename", type="string",
                      default=DEFAULT_SSDB_SERVICENAME,
                      help="Name for this service. [default: %default]")
    parser.add_option_group(dbcredentials.options_group(parser))
    parser.set_defaults(dbcredentials="SSDB")
    (options, args) = parser.parse_args()

    dbcreds = dbcredentials.parse_options(options)

    logger.info("Using dbcreds: %s" % dbcreds.stringWithHiddenPassword())

    with createService(busname=options.busname, servicename=options.servicename, dbcreds=dbcreds, broker=options.broker):
        waitForInterrupt()
