#!/usr/bin/python

import sys
import logging
from lofar.messaging.RPC import RPC

''' Simple RPC client for Service momqueryservice.GetProjectDetails
'''

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)

class RPCException(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "RPCException: " + str(self.message)

busname = 'raservice'

def _rpc(service, **kwargs):
    try:
        rpckwargs = {'timeout': 10}
        with RPC(service, busname=busname, **rpckwargs) as rpc:
            if kwargs:
                res, status = rpc(**kwargs)
            else:
                res, status = rpc()

            if status != 'OK':
                message = "%s %s" % (status, res)
                logger.error(message)
                raise RPCException(message)

            return res
    except Exception as e:
        logger.error(str(e))
        raise RPCException(str(e))

def getResourceClaimStatuses():
    return _rpc('RAS.GetResourceClaimStatuses')

def getResourceClaims():
    return _rpc('RAS.GetResourceClaims')

def getResourceGroupTypes():
    return _rpc('RAS.GetResourceGroupTypes')

def getResourceGroups():
    return _rpc('RAS.GetResourceGroups')

def getResourceTypes():
    return _rpc('RAS.GetResourceTypes')

def getResources():
    return _rpc('RAS.GetResources')

def getTask(id):
    task = _rpc('RAS.GetTask', id=id)
    if task:
        task['starttime'] = task['starttime'].datetime()
        task['endtime'] = task['endtime'].datetime()
    return task

def getTasks():
    tasks = _rpc('RAS.GetTasks')
    for task in tasks:
        task['starttime'] = task['starttime'].datetime()
        task['endtime'] = task['endtime'].datetime()
    return tasks

def getTaskTypes():
    return _rpc('RAS.GetTaskTypes')

def getTaskStatuses():
    return _rpc('RAS.GetTaskStatuses')

def getUnits():
    return _rpc('RAS.GetUnits')


def main():
    def resultPrint(method):
        print '\n-- ' + str(method.__name__) + ' --'

        for obj in method():
            print '  Object %s' % (obj['id'] if 'id' in obj else '')

            for k, v in obj.items():
                print '    %s: %s' % (k, v)

    resultPrint(getTaskStatuses)
    resultPrint(getTaskTypes)
    resultPrint(getResourceClaimStatuses)
    resultPrint(getUnits)
    resultPrint(getResourceTypes)
    resultPrint(getResourceGroupTypes)
    resultPrint(getResources)
    resultPrint(getResourceGroups)
    resultPrint(getTasks)
    resultPrint(getResourceClaims)



if __name__ == '__main__':
    main()
