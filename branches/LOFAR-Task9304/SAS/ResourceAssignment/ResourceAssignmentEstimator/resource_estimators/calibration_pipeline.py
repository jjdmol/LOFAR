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
from lofar.parameterset import parameterset

logger = logging.getLogger(__name__)

class CalibrationPipelineResourceEstimator(BaseResourceEstimator):
    """ CalibrationPipelineResourceEstimator

    """
    def __init__(self, kwargs, input_files):
        BaseResourceEstimator.__init__(self, name='calibration_pipeline')
        self.required_keys = ('correlated.enabled', 'correlated.demixing_settings.freq_step',
                          'correlated.demixing_settings.time_step', 'instrument_model.enabled')

    def _calculate(self, parset, input_files):
        """ Estimate for calibration pipeline
        calculates: datasize (number of files, file size), bandwidth
        """
        logger.debug("start estimate '{}'".format(self.name))
        parset = parameterset(parset).makeSubset('dp.output')
        output_files = {}
        duration = int(kwargs.get('observation.duration', 1))
        if 'dp_correlated_uv' in input_files:
            if parset['correlated']['enabled'] == 'true':
                logger.debug("calculate correlated data size")
                freq_step = int(parset['correlated']['demixing_settings']['freq_step'])
                time_step = int(parset['correlated']['demixing_settings']['time_step'])
                reduction_factor = freq_step * time_step
                input_file_size = int(input_files['dp_correlated_uv']['file_size'])
                output_file_size = 0.0
                if reduction_factor > 0:
                    new_size = input_file_size / reduction_factor
                    output_file_size = new_size + new_size / 64.0 * (1.0 + reduction_factor) + new_size / 2.0
                output_files['dp_correlated_uv'] = {'nr_files': int(input_files['dp_correlated_uv']['nr_files']), 'file_size': int(output_file_size)}
                logger.debug("dp_correlated_uv: {} files {} bytes each".format(int(input_files['dp_correlated_uv']['nr_files']), int(output_file_size)))

            if parset['instrument_model']['enabled'] == 'true':
                logger.debug("calculate instrument-model data size")
                output_files['dp_instrument_model'] = {'nr_files': int(input_files['dp_correlated_uv']['nr_files']), 'file_size': 1000}
                logger.debug("dp_instrument_model: {} files {} bytes each".format(int(input_files['dp_correlated_uv']['nr_files']), 1000))

            # count total data size
            total_data_size = 0
            for values in output_files.itervalues():
                total_data_size += values['nr_files'] * values['file_size']
            total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second
        return {"total_data_size":total_data_size, "total_bandwidth":total_bandwidth, "output_files":output_files}

