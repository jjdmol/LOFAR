#!/usr/bin/python

import sys
import logging
from lofar.messaging.RPC import RPC

''' Simple RPC client for Service momqueryservice.GetProjectDetails
'''

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)


def getTaskStatuses(busname='raservice'):
    with RPC('GetTaskStatuses', busname=busname, timeout=10) as rpc:
        res, status = rpc()

        if status != 'OK':
            logger.error("%s %s" % (status, res))
            return status

        return res


def main(busname='raservice'):
    result = getTaskStatuses(busname)

    for obj in result:
        id = obj['id']
        logger.info('Object %s' % (id))
        for k, v in obj.items():
            logger.info('  %s: %s' % (k, v))


if __name__ == '__main__':
    main()
