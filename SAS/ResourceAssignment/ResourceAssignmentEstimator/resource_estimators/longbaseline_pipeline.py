# base_resource_estimator.py
#
# Copyright (C) 2016
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: base_resource_estimator.py 33534 2016-02-08 14:28:26Z schaap $

import logging
from math import ceil
from base_resource_estimator import BaseResourceEstimator

logger = logging.getLogger(__name__)

DATAPRODUCTS = "Observation.DataProducts."
PIPELINE = "Observation.ObservationControl.PythonControl."

#Observation.DataProducts.Output_Correlated.storageClusterName=

class LongBaselinePipelineResourceEstimator(BaseResourceEstimator):
    """ ResourceEstimator for Long Baseline Pipelines
    """
    def __init__(self, kwargs, input_files):
        logger.info("init LongBaselinePipelineResourceEstimator")
        BaseResourceEstimator.__init__(self, name='longbaseline_pipeline')
        self.required_keys = ('Observation.startTime',
                              'Observation.stopTime',
                              DATAPRODUCTS + 'Input_Correlated.enabled',
                              DATAPRODUCTS + 'Output_Correlated.enabled',
                              PIPELINE + 'LongBaseline.subbandgroups_per_ms',
                              PIPELINE + 'LongBaseline.subbands_per_subbandgroup')

    def _calculate(self, parset, input_files):
        """ Estimate for Long Baseline Pipeline
        calculates: datasize (number of files, file size), bandwidth
        input_files should look something like:
        'input_files': 
        {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104}, ...}
        
        reply is something along the lines of:
        {'bandwidth': {'total_size': 19021319494},
        'storage': {'total_size': 713299481024,
        'output_files': 
          {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104}
        }}
        """
        logger.debug("start estimate '{}'".format(self.name))
        logger.info('parset: %s ' % parset)
        result = {'errors': []}
        duration = self._getDuration(parset.getString('Observation.startTime'), parset.getString('Observation.stopTime'))
        subbandgroups_per_ms = parset.getInt(PIPELINE + 'LongBaseline.subbandgroups_per_ms', 0) #TODO, should these have defaults?
        subbands_per_subbandgroup = parset.getInt(PIPELINE + 'LongBaseline.subbands_per_subbandgroup', 0)

        if not parset.getBool(DATAPRODUCTS + 'Output_Correlated.enabled'):
            logger.warning('Output_Correlated is not enabled')
            result['errors'].append('Output_Correlated is not enabled')
        if not 'uv' in input_files:
            logger.warning('Missing UV Dataproducts in input_files')
            result['errors'].append('Missing UV Dataproducts in input_files')
        else:
            nr_input_files = input_files['uv']['nr_of_uv_files']
        if not subbandgroups_per_ms or not subbands_per_subbandgroup:
            logger.warning('subbandgroups_per_ms or subbands_per_subbandgroup are not valid')
            result['errors'].append('Missing UV Dataproducts in input_files')
        if nr_input_files % (subbands_per_subband_group * subband_groups_per_ms) > 0:
            logger.warning('subbandgroups_per_ms and subbands_per_subbandgroup not a multiple of number of inputs')
            result['errors'].append('subbandgroups_per_ms and subbands_per_subbandgroup not a multiple of number of inputs')
        if result['errors']:
            return result

        logger.debug("calculate correlated data size")
        result['output_files'] = {}
        nr_output_files = nr_input_files / (subbands_per_subbandgroup * subbandgroups_per_ms)
        result['output_files']['uv'] = {'nr_of_uv_files': nr_output_files, 'uv_file_size': 1000} # 1 kB was hardcoded in the Scheduler 
        logger.debug("correlated_uv: {} files {} bytes each".format(result['output_files']['uv']['nr_of_uv_files'], result['output_files']['uv']['uv_file_size']))

        # count total data size
        total_data_size = result['output_files']['uv']['nr_of_uv_files'] * result['output_files']['uv']['uv_file_size'] # bytes
        total_bandwidth = int(ceil((total_data_size * 8) / duration))  # bits/second
        result['storage'] = {'total_size': total_data_size}
        result['bandwidth'] = {'total_size': total_bandwidth}
        return result
