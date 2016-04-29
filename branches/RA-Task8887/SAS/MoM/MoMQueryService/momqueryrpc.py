#!/usr/bin/python

import sys
import logging
from optparse import OptionParser
from lofar.messaging.RPC import RPC, RPCException, RPCWrapper
from lofar.mom.momqueryservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME

''' Simple RPC client for Service momqueryservice
'''

logger = logging.getLogger(__file__)


class MoMQueryRPC(RPCWrapper):
    def getProjectDetails(self, ids):
        '''get the project details for one or more mom ids
        :param ids single or list of mom ids
        :rtype dict with project details'''
        if isinstance(ids, int) or isinstance(ids, str):
            ids = [ids]
        ids = [str(x) for x in ids]
        ids_string = ', '.join(ids)

        logger.info("Requesting details for mom objects: %s" % (str(ids_string)))
        result = self.rpc('GetProjectDetails', mom_ids=ids_string)
        logger.info("Received details for %s mom objects" % (len(result)))
        return result

    def getProjects(self):
        '''get all projects
        :rtype dict with all projects'''
        logger.info("Requesting all projects")
        projects = self.rpc('GetProjects')
        for project in projects:
            project['statustime'] = project['statustime'].datetime()

        logger.info("Received %s projects" % (len(projects)))
        return projects


def main():
    # Check the invocation arguments
    parser = OptionParser('%prog [options]',
                          description='do requests to the momqueryservice from the commandline')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option('-b', '--busname', dest='busname', type='string', default=DEFAULT_BUSNAME, help='Name of the bus exchange on the qpid broker, default: %s' % DEFAULT_BUSNAME)
    parser.add_option('-s', '--servicename', dest='servicename', type='string', default=DEFAULT_SERVICENAME, help='Name for this service, default: %s' % DEFAULT_SERVICENAME)
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    parser.add_option('-P', '--projects', dest='projects', action='store_true', help='get list of all projects')
    parser.add_option('-p', '--project_details', dest='project_details', type='int', help='get project details for mom object with given id')
    (options, args) = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_help()

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.INFO if options.verbose else logging.WARN)

    with MoMQueryRPC(busname=options.busname, servicename=options.servicename, broker=options.broker) as rpc:
        if options.projects:
            projects = rpc.getProjects()
            for project in projects:
                print project

        if options.project_details:
            projects_details = rpc.getProjectDetails(options.project_details)
            if projects_details:
                for k, v in projects_details.items():
                    print '  %s: %s' % (k, v)
            else:
                print 'No results'

if __name__ == '__main__':
    main()
