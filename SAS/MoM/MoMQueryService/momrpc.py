#!/usr/bin/python

import sys
import logging
from optparse import OptionParser
from lofar.messaging.RPC import RPC, RPCException, RPCWrapper
from lofar.mom.momqueryservice.config import DEFAULT_MOM_BUSNAME, DEFAULT_MOM_SERVICENAME

logger = logging.getLogger(__file__)

class MoMRPC(RPCWrapper):
    def copyTask(self, mom2id, newTaskName=None, newTaskDescription=None):
        logger.info("calling copyTask rpc for mom2id %s" % (mom2id))
        new_task_mom2id = self.rpc('TaskCopy', mom2Id=mom2id) #, newTaskName=newTaskName, newTaskDescription=newTaskDescription)
        logger.info("mom2id of copied task = %s" % (new_task_mom2id))
        return new_task_mom2id

def main():
    # Check the invocation arguments
    parser = OptionParser('%prog [options]',
                          description='do rpc calls to the momservice from the commandline')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option('-b', '--busname', dest='busname', type='string', default=DEFAULT_MOM_BUSNAME, help='Name of the bus exchange on the qpid broker [default: %default]')
    parser.add_option('-s', '--servicename', dest='servicename', type='string', default=DEFAULT_MOM_SERVICENAME, help='Name for this service [default: %default]')
    parser.add_option('--mom2id', dest='mom2id_to_copy', type='int', help='[REQUIRED] mom2id of the task to copy.')
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    if options.mom2id_to_copy == None:
        parser.print_help()
        parser.error('Missing required option mom2id')

    verbose = bool(options.verbose)

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if verbose else logging.INFO)

    with MoMRPC(busname=options.busname, servicename=options.servicename, broker=options.broker, verbose=verbose) as rpc:
        print rpc.copyTask(options.mom2id_to_copy)

if __name__ == '__main__':
    main()
