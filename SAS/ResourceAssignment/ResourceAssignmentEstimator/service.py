#!/usr/bin/env python
# $Id: service.py $

'''
Simple Service listening
'''

import logging
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt

from lofar.sas.resourceassignment.resourceassignmentestimator.resource_estimators import *

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)


class ResourceEstimatorHandler(MessageHandlerInterface):
    def __init__(self, **kwargs):
        super(ResourceEstimatorHandler, self).__init__(**kwargs)

        self.service2MethodMap = {'ResourceEstimation': self._get_estimated_resources}

    def _get_estimated_resources(self):
        result = {}

        observation = ObservationResourceEstimator(self.kwargs)
        result.update(observation.result_as_dict())

        pipeline_input_files = result['observation']['output_files']

        longbaseline = LongBaselinePipelineResourceEstimator(self.kwargs, input_files=pipeline_input_files)
        result.update(longbaseline.result_as_dict())

        calibration = CalibrationPipelineResourceEstimator(self.kwargs, input_files=pipeline_input_files)
        result.update(calibration.result_as_dict())

        pulsar = PulsarPipelineResourceEstimator(self.kwargs, input_files=pipeline_input_files)
        result.update(pulsar.result_as_dict())

        image = ImagePipelineResourceEstimator(self.kwargs, input_files=pipeline_input_files)
        result.update(image.result_as_dict)

        return result


def createService(busname='lofarbus'):
    return Service('ResourceEstimation',
                   ResourceEstimatorHandler,
                   busname=busname,
                   numthreads=1)

def main():
    with createService(busname='lofarbus'):
        waitForInterrupt()

if __name__ == '__main__':
    main()
