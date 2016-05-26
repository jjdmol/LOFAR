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
        self.longbaseline_pipeline = LongBaselinePipelineResourceEstimator()
        self.calibration_pipeline = CalibrationPipelineResourceEstimator()
        self.pulsar_pipeline = PulsarPipelineResourceEstimator()
        self.imaging_pipeline = ImagePipelineResourceEstimator()

    def handle_message(self, content):
        specification_tree = content["specification_tree"]
        return self._get_estimated_resources(specification_tree) ##TODO also handle MoM tasks in RA 1.2

    def get_subtree_estimate(self, specification_tree):
        otdb_id = specification_tree['otdb_id']
        parset = specification_tree['specification']
        if specification_tree['task_type'] == 'observation':
            return {str(otdb_id): self.observation.verify_and_estimate(parset)}
        elif specification_tree['task_type'] == 'pipeline':
            branch_estimates = {}
            for branch in specification_tree['predecessors']:
                branch_estimates.update(get_subtree_estimate(branch))
                
            if specification_tree['task_subtype'] in ['averaging pipeline', 'calibration pipeline']:
                for id, estimate in branch_estimates:
                    if not 'im' in estimate['output_files'] and 'uv' in estimate['output_files']: # Not a calibrator pipeline
                        logger.info('found %d as the target of pipeline %d' % (id, otdb_id))
                        input_files = estimate['output_files'] # Need sap here as well
                return {str(otdb_id): self.calibration_pipeline.verify_and_estimate(parset, input_files)}
                
            if specification_tree['task_subtype'] in ['imaging pipeline', 'imaging pipeline msss']:
                if len(branch_estimates) > 1:
                    logger.error('Imaging pipeline %d should not have multiple predecessors: %s' % (otdb_id, branch_estimates.keys() ) )
                input_files = branch_estimates.items()[0][1]['ouput_files']
                return {str(otdb_id): self.calibration_pipeline.verify_and_estimate(parset, input_files)}
                
            if specification_tree['task_subtype'] in ['long baseline pipeline']:
                if len(branch_estimates) > 1:
                    logger.error('Long baseline pipeline %d should not have multiple predecessors: %s' % (otdb_id, branch_estimates.keys() ) )
                input_files = branch_estimates.items()[0][1]['ouput_files']
                return {str(otdb_id): self.longbaseline_pipeline.verify_and_estimate(parset, input_files)}

            if specification_tree['task_subtype'] in ['pulsar pipeline']:
                if len(branch_estimates) > 1:
                    logger.error('Pulsar pipeline %d should not have multiple predecessors: %s' % (otdb_id, branch_estimates.keys() ) )
                input_files = branch_estimates.items()[0][1]['ouput_files']
                return {str(otdb_id): self.pulsar_pipeline.verify_and_estimate(parset, input_files)}
            
        else: # reservation, maintenance, system tasks?
            logger.info("It's not a pipeline or observation: %s" % otdb_id)
            return {str(otdb_id): {}}

    def _get_estimated_resources(self, specification_tree):
        """ Input is like:
            {"otdb_id": otdb_id, "state": 'prescheduled', 'specification': ...,
             'task_type': "pipeline", 'task_subtype': "long baseline pipeline",
            'predecessors': [...]}
        
            reply is something along the lines of:
            {'452648': 
              {'observation': 
                {'bandwidth': {'total_size': 19021319494},
                'storage': {'total_size': 713299481024,
                'output_files': 
                  {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104},
                  'saps': [{'sap_nr': 0, 'properties': {'nr_of_uv_files': 319}},
                           {'sap_nr': 1, 'properties': {'nr_of_uv_files': 81}}, 
                           {'sap_nr': 2, 'properties': {'nr_of_uv_files': 81}}
            ]}}}}}
        """
        logger.info('get_estimated_resources on: %s' % specification_tree)
        return self.get_subtree_estimate(specification_tree)

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
