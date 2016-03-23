#!/usr/bin/env python
# $Id: service.py $

'''
Simple Service listening
'''

import logging
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface

from lofar.sas.resourceassignment.resourceassignmentestimator.resource_estimators import *
from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME

logger = logging.getLogger(__name__)

class ResourceEstimatorHandler(MessageHandlerInterface):
    def __init__(self, **kwargs):
        super(ResourceEstimatorHandler, self).__init__(**kwargs)
        self.observation = ObservationResourceEstimator()
        #self.longbaseline_pipeline = LongBaselinePipelineResourceEstimator()
        #self.calibration_pipeline = CalibrationPipelineResourceEstimator()
        #self.pulsar_pipeline = PulsarPipelineResourceEstimator()
        #self.imaging_pipeline = ImagePipelineResourceEstimator()

    def handle_message(self, content):
        otdb_id = content["otdb_id"]
        parsets = content["parsets"]
        return self._get_estimated_resources(otdb_id, parsets) ##TODO also handle MoM tasks in RA 1.2

    #def _getPredecessors(self, parset):
        

    def _get_estimated_resources(self, otdb_id, parsets):
        logger.info('get_estimated_resources on: %s' % parsets)
        result = {}

        result[str(otdb_id)] = self.observation.estimate(parsets[str(otdb_id)])

        #TODO: implement properly
        #pipeline_input_files = result['observation']['output_files']

        #longbaseline = LongBaselinePipelineResourceEstimator(parsetDict, input_files=pipeline_input_files)
        #result.update(longbaseline.result_as_dict())

        #calibration = CalibrationPipelineResourceEstimator(parsetDict, input_files=pipeline_input_files)
        #result.update(calibration.result_as_dict())

        #pulsar = PulsarPipelineResourceEstimator(parsetDict, input_files=pipeline_input_files)
        #result.update(pulsar.result_as_dict())

        #image = ImagePipelineResourceEstimator(parsetDict, input_files=pipeline_input_files)
        #result.update(image.result_as_dict())

        return result[str(otdb_id)] ## temp hack


def createService(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME, broker=None):
    return Service(servicename=servicename,
                   servicehandler=ResourceEstimatorHandler,
                   busname=busname,
                   broker=broker,
                   numthreads=1,
                   verbose=True)

def main():
    from optparse import OptionParser
    from lofar.messaging import setQpidLogLevel
    from lofar.common.util import waitForInterrupt

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the resourceassigner service')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_BUSNAME, help="Name of the bus exchange on the qpid broker, default: %s" % DEFAULT_BUSNAME)
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=DEFAULT_SERVICENAME, help="Name for this service, default: %s" % DEFAULT_SERVICENAME)
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    setQpidLogLevel(logging.INFO)
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    with createService(busname=options.busname, servicename=options.servicename, broker=options.broker):
        waitForInterrupt()

if __name__ == '__main__':
    main()
