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

class PulsarPipelineResourceEstimator(BaseResourceEstimator):
    """ ResourceEstimator for Pulsar Pipelines
    """
    def __init__(self, kwargs, input_files):
        logger.info("init PulsarPipelineResourceEstimator")
        BaseResourceEstimator.__init__(self, name='pulsar_pipeline')
        self.required_keys = ('Observation.startTime',
                              'Observation.stopTime',
                              DATAPRODUCTS + 'Input_CoherentStokes.enabled',
                              DATAPRODUCTS + 'Input_IncoherentStokes.enabled',
                              DATAPRODUCTS + 'Output_Pulsar.enabled')

    def _calculate(self, parset, input_files):
        """ Estimate for Pulsar Pipeline
        calculates: datasize (number of files, file size), bandwidth
        input_files should look something like:
        'input_files': 
        {'cs': {'nr_of_cs_files': 48, 'nr_of_cs_stokes': 4, 'cs_file_size': 1482104}, ...}
        
        reply is something along the lines of:
        {'bandwidth': {'total_size': 19021319494},
        'storage': {'total_size': 713299481024,
        'output_files': 
          {'pulp': {'nr_of_pulp_files': 48, 'pulp_file_size': 185104}
        }}
        """
        logger.debug("start estimate '{}'".format(self.name))
        logger.info('parset: %s ' % parset)
        result = {'errors': []}
        duration = self._getDuration(parset.getString('Observation.startTime'), parset.getString('Observation.stopTime'))

        if not parset.getBool(DATAPRODUCTS + 'Output_Pulsar.enabled'):
            logger.warning('Output_Pulsar is not enabled')
            result['errors'].append('Output_Pulsar is not enabled')
        if not 'cs' in input_files and not 'is' in input_files:
            logger.warning('Missing Both CS and IS Dataproducts in input_files')
            result['errors'].append('Missing Both CS and IS Dataproducts in input_files')
        if result['errors']:
            return result

        logger.debug("calculate pulp data size")
        result['output_files'] = {}
        nr_output_files = 0
        if 'cs' in input_files:
            nr_input_files = input_files['cs']['nr_of_cs_files']
            if input_files['cs']['nr_of_cs_stokes'] == 4: ##TODO Check if this is the same as coherent_stokes_type == 'XXYY'
                nr_output_files += nr_input_files / 4     ## Then nr_output_files = nr_input_files / input_files['cs']['nr_of_cs_stokes']
            else:
                nr_output_files += nr_input_files

        if 'is' in input_files:
            nr_input_files = input_files['is']['nr_of_is_files']
            nr_output_files += nr_input_files

        result['output_files']['pulp'] = {'nr_of_pulp_files': nr_output_files, 'pulp_file_size': 1000} # 1 kB was hardcoded in the Scheduler 
        logger.debug("correlated_uv: {} files {} bytes each".format(result['output_files']['pulp']['nr_of_pulp_files'], result['output_files']['pulp']['pulp_file_size']))

        # count total data size
        total_data_size = result['output_files']['pulp']['nr_of_pulp_files'] * result['output_files']['pulp']['pulp_file_size'] # bytes
        total_bandwidth = int(ceil((total_data_size * 8) / duration))  # bits/second
        result['storage'] = {'total_size': total_data_size}
        result['bandwidth'] = {'total_size': total_bandwidth}
        return result
