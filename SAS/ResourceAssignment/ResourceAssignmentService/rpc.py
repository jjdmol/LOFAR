#!/usr/bin/python

import logging
from lofar.messaging.RPC import RPC

''' Simple RPC client for Service lofarbus.RAS.*Z
'''

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)

class RARPCException(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "RARPCException: " + str(self.message)


class RARPC:
    def __init__(self, busname='lofarbus'):
        self.busname = busname

    def _rpc(self, service, timeout=10, **kwargs):
        try:
            rpckwargs = {'timeout': timeout}
            with RPC(service, busname=self.busname, **rpckwargs) as rpc:
                if kwargs:
                    res, status = rpc(**kwargs)
                else:
                    res, status = rpc()

                if status != 'OK':
                    message = "%s %s" % (status, res)
                    logger.error(message)
                    raise RARPCException(message)

                return res
        except Exception as e:
            logger.error(str(e))
            raise RARPCException(str(e))

    def getResourceClaimStatuses(self):
        return self._rpc('RAS.GetResourceClaimStatuses')

    def getResourceClaims(self):
        claims = self._rpc('RAS.GetResourceClaims')
        for claim in claims:
            claim['starttime'] = claim['starttime'].datetime()
            claim['endtime'] = claim['endtime'].datetime()
        return claims

    def getResourceGroupTypes(self):
        return self._rpc('RAS.GetResourceGroupTypes')

    def getResourceGroups(self):
        return self._rpc('RAS.GetResourceGroups')

    def getResourceTypes(self):
        return self._rpc('RAS.GetResourceTypes')

    def getResources(self):
        return self._rpc('RAS.GetResources')

    def getTask(self, id):
        task = self._rpc('RAS.GetTask', id=id)
        if task:
            task['starttime'] = task['starttime'].datetime()
            task['endtime'] = task['endtime'].datetime()
        return task

    def getTasks(self):
        tasks = self._rpc('RAS.GetTasks')
        for task in tasks:
            task['starttime'] = task['starttime'].datetime()
            task['endtime'] = task['endtime'].datetime()
        return tasks

    def getTaskTypes(self):
        return self._rpc('RAS.GetTaskTypes')

    def getTaskStatuses(self):
        return self._rpc('RAS.GetTaskStatuses')

    def getUnits(self):
        return self._rpc('RAS.GetUnits')

