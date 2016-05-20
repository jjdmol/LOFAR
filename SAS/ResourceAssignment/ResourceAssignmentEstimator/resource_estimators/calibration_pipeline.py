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

class CalibrationPipelineResourceEstimator(BaseResourceEstimator):
    """ ResourceEstimator for Calibration Pipelines
    """
    def __init__(self, kwargs, input_files):
        logger.info("init CalibrationPipelineResourceEstimator")
        BaseResourceEstimator.__init__(self, name='calibration_pipeline')
        self.required_keys = ('Observation.startTime',
                              'Observation.stopTime',
                              DATAPRODUCTS + 'Input_Correlated.enabled',
                              DATAPRODUCTS + 'Output_InstrumentModel.enabled',
                              DATAPRODUCTS + 'Output_Correlated.enabled',
                              PIPELINE + 'DPPP.demixer.freqstep',
                              PIPELINE + 'DPPP.demixer.timestep')

    def _calculate(self, parset, input_files):
        """ Estimate for CalibrationPipeline. Also gets used for AveragingPipeline
        calculates: datasize (number of files, file size), bandwidth
        input_files should look something like:
        'input_files': 
        {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104}, ...}
        
        reply is something along the lines of:
        {'bandwidth': {'total_size': 19021319494},
        'storage': {'total_size': 713299481024,
        'output_files': 
          {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104},
          'im': {'nr_of_im_files': 481, 'im_file_size': 148295}
        }}
        """
        logger.debug("start estimate '{}'".format(self.name))
        logger.info('parset: %s ' % parset)
        result = {'errors': []}
        duration = self._getDuration(parset.getString('Observation.startTime'), parset.getString('Observation.stopTime'))
        freq_step = parset.getInt(PIPELINE + 'DPPP.demixer.freqstep', 1) #TODO, should these have defaults?
        time_step = parset.getInt(PIPELINE + 'DPPP.demixer.timestep', 1)
        reduction_factor = freq_step * time_step

        if not parset.getBool(DATAPRODUCTS + 'Output_Correlated.enabled'):
            logger.warning('Output_Correlated is not enabled')
            result['errors'].append('Output_Correlated is not enabled')
        if not 'uv' in input_files:
            logger.warning('Missing UV Dataproducts in input_files')
            result['errors'].append('Missing UV Dataproducts in input_files')
        if reduction_factor < 1:
            logger.warning('freqstep * timestep is not valid')
            result['errors'].append('freqstep * timestep is not positive')
        if result['errors']:
            return result

        logger.debug("calculate correlated data size")
        result['output_files'] = {}
        input_file_size = input_files['uv']['uv_file_size']
        output_file_size = 0.0
        new_size = input_file_size / float(reduction_factor)
        output_file_size = new_size + new_size / 64.0 * (1.0 + reduction_factor) + new_size / 2.0
        result['output_files']['uv'] = {'nr_of_uv_files': input_files['uv']['nr_of_uv_files'], 'uv_file_size': int(output_file_size)}
        logger.debug("correlated_uv: {} files {} bytes each".format(result['output_files']['uv']['nr_of_uv_files'], result['output_files']['uv']['uv_file_size']))

        if parset.getBool(DATAPRODUCTS + 'Output_InstrumentModel.enabled'):
            logger.debug("calculate instrument-model data size")
            result['output_files']['im'] = {'nr_of_im_files': input_files['uv']['nr_of_uv_files'], 'im_file_size': 1000} # 1 kB was hardcoded in the Scheduler
            logger.debug("correlated_uv: {} files {} bytes each".format(result['output_files']['im']['nr_of_im_files'], result['output_files']['im']['im_file_size']))

        # count total data size
        total_data_size = result['output_files']['uv']['nr_of_uv_files'] * result['output_files']['uv']['uv_file_size'] + \
                          result['output_files']['im']['nr_of_im_files'] * result['output_files']['im']['im_file_size']  # bytes
        total_bandwidth = int(ceil((total_data_size * 8) / duration))  # bits/second
        result['storage'] = {'total_size': total_data_size}
        result['bandwidth'] = {'total_size': total_bandwidth}
        return result

