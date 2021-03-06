#!/usr/bin/python

import logging
import datetime
from lofar.messaging.RPC import RPC, RPCException, RPCWrapper
from lofar.sas.otdb.config import DEFAULT_OTDB_SERVICE_BUSNAME, DEFAULT_OTDB_SERVICENAME

''' Simple RPC client for Service lofarbus.*Z
'''

logger = logging.getLogger(__name__)

class OTDBPRCException(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "OTDBPRCException: " + str(self.message)

class OTDBRPC(RPCWrapper):
    def __init__(self, busname=DEFAULT_OTDB_SERVICE_BUSNAME,
                 servicename=DEFAULT_OTDB_SERVICENAME,
                 broker=None):
        super(OTDBRPC, self).__init__(busname, servicename, broker)

    def taskGetIDs(self, otdb_id=None, mom_id=None):
        if otdb_id:
            answer = self.rpc('TaskGetIDs', OtdbID=otdb_id, return_tuple=False)
        elif mom_id:
            answer = self.rpc('TaskGetIDs', MomID=mom_id, return_tuple=False)
        else:
            raise OTDBPRCException("TaskGetIDs was called without OTDB or Mom ID")
        if not answer:
            raise OTDBPRCException("TaskGetIDs returned an empty dict")
        return {"tree_type": answer[0], "otdb_id": answer[1], "mom_id": answer[2]}


    def taskGetSpecification(self, otdb_id=None, mom_id=None):
        if otdb_id:
            answer = self.rpc('TaskGetSpecification', OtdbID=otdb_id)
        elif mom_id:
            answer = self.rpc('TaskGetSpecification', MomID=mom_id)
        else:
            raise OTDBPRCException("TaskGetSpecification was called without OTDB or Mom ID")
        if not answer["TaskSpecification"]:
            raise OTDBPRCException("TaskGetSpecification returned an empty dict")
        return {"specification": answer["TaskSpecification"]}
    
    def taskCreate(self, otdb_id=None, mom_id=None, template_name="", campaign_name="", specification={}):
        if otdb_id: ##Can this ever be called with a otdb_id?
            answer = self.rpc('TaskCreate', OtdbID=otdb_id, TemplateName=template_name, CampaignName=campaign_name, Specification=specification)
        elif mom_id:
            answer = self.rpc('TaskCreate', MomID=mom_id, TemplateName=template_name, CampaignName=campaign_name, Specification=specification)
        else:
            raise OTDBPRCException("TaskCreate was called without OTDB or Mom ID")
        if not answer["Success"]:
            raise OTDBPRCException("TaskCreate failed for MoM ID %i" % (mom_id,))
        return {"mom_id": answer["MomID"], "otdb_id": answer["OtdbID"]}

    def taskGetStatus(self, otdb_id):
        return self.rpc('TaskGetStatus', otdb_id=otdb_id)['status']

    def taskSetStatus(self, otdb_id=None, new_status="", update_timestamps=True):
        answer = self.rpc('TaskSetStatus', OtdbID=otdb_id, NewStatus=new_status, UpdateTimestamps=update_timestamps)
        if not answer["Success"]:
            raise OTDBPRCException("TaskSetStatus failed for %i" % (otdb_id,))
        return {"mom_id": answer["MomID"], "otdb_id": answer["OtdbID"]}

    def taskSetSpecification(self, otdb_id=None, specification={}):
        answer = self.rpc('TaskSetSpecification', OtdbID=otdb_id, Specification=specification)
        if "Errors" in answer:
            for key, problem in answer["Errors"].iteritems():
                logger.warning("TaskSetSpecification for %i failed to set key %s because of %s" % (otdb_id, key, problem))
            raise OTDBPRCException("TaskSetSpecification failed to set all keys for %i" % (otdb_id,))
        return {"mom_id": answer["MomID"], "otdb_id": answer["OtdbID"]}
    
    def taskPrepareForScheduling(self, otdb_id=None, starttime="", endtime=""):
        answer = self.rpc('TaskPrepareForScheduling', OtdbID= otdb_id, StartTime=starttime, StopTime=endtime)
        return {"otdb_id": answer["OtdbID"]}

    def taskDelete(self, otdb_id=None):
        answer = self.rpc('TaskDelete', OtdbID=otdb_id)
        if not answer["Success"]:
            logger.warning("TaskDelete failed for %i" % (otdb_id,)) ##Probably was already deleted?
        return {"mom_id": answer["MomID"], "otdb_id": answer["OtdbID"]}

    def getDefaultTemplates(self):
        answer = self.rpc('GetDefaultTemplates')
        if not answer["DefaultTemplates"]:
            raise OTDBPRCException("GetDefaultTemplates returned an empty dict")
        return {"default_templates": answer["DefaultTemplates"]}

    def getStations(self):
        answer = self.rpc('GetStations')
        if not answer["Stations"]:
            raise OTDBPRCException("GetStations returned an empty dict")
        return {"stations": answer["Stations"]}

    def setProject(self, name=None, title="", pi="", co_i="", contact=""):
        if not name:
            raise OTDBPRCException("SetProject was called with an empty project")
        answer = self.rpc('SetProject', name=name, pi=pi, co_i=co_i, contact=contact)
        if not answer["projectID"]:
            raise OTDBPRCException("SetProject failed for %s" % (name,))
        return {"project_id": answer["projectID"]}


def do_tests(busname=DEFAULT_OTDB_SERVICE_BUSNAME, servicename=DEFAULT_OTDB_SERVICENAME):
    with OTDBRPC(busname=busname, servicename=servicename, broker='10.149.96.6') as rpc:
        print rpc.taskGetStatus(452728)

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
    do_tests()
