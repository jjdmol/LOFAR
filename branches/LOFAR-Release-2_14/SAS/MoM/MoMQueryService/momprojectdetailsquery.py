#!/usr/bin/python

import sys
import logging
from lofar.messaging.RPC import RPC

''' Simple RPC client for Service momqueryservice.GetProjectDetails
'''

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger("momprojectdetailsquery")


def getProjectDetails(ids, busname='momqueryservice'):
    '''get the project details for one or more mom ids
    :param ids single or list of mom ids_string
    :param string busname name of the bus on which the service listens
    :rtype dict with project details'''
    if isinstance(ids, int) or isinstance(ids, str):
        ids = [ids]
    ids = [str(x) for x in ids]
    ids_string = ', '.join(ids)

    with RPC('GetProjectDetails', busname=busname, timeout=10) as getProjectDetails:
        res, status = getProjectDetails(ids_string)

        if status != 'OK':
            logger.error("%s %s" % (status, res))
            return status

        return res


def main(ids, busname='momqueryservice'):
    '''get the project details for one or more mom ids
    and print the results
    :param ids single or list of mom ids_string
    :param string busname name of the bus on which the service listens
    :rtype dict with project details'''
    logger.info("Requesting details for: %s" % (str(ids)))
    result = getProjectDetails(ids, busname)

    for id, obj in result.items():
        logger.info('Object %s' % (id))
        for k, v in obj.items():
            logger.info('  %s: %s' % (k, v))


if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print 'Please provide one or more mom ids'
        sys.exit(1)

    ids = ','.join(sys.argv[1:])
    main(ids)
