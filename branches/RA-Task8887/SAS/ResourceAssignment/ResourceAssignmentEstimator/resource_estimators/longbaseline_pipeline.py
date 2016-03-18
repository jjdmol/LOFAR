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
from .parameterset import ParameterSet

logger = logging.getLogger(__name__)


class LongBaselinePipelineResourceEstimator(BaseResourceEstimator):
    """ LongBaselinePipelineResourceEstimator
    """
    def __init__(self, kwargs, input_files):
        BaseResourceEstimator.__init__(self, name='longbaseline_pipeline')
        logger.debug("init LongBaselinePipelineResourceEstimator")
        self.parset = ParameterSet(kwargs).make_subset('dp.output')
        self.duration = int(kwargs.get('observation.duration', 1))
        self.input_files = input_files
        self.required_keys = ('correlated.enabled', 'longbaseline.subband_groups_per_ms',
                          'longbaseline.subbands_per_subband_group')
        if self.checkParsetForRequiredKeys():
            self.estimate()
        return

    def estimate(self):
        """ Estimate for calibration pipeline
        calculates: datasize (number of files, file size), bandwidth
        """
        logger.debug("start estimate '{}'".format(self.name))
        if 'dp_correlated_uv' in self.input_files:
            if self.parset['correlated']['enabled'] == 'true':
                logger.debug("calculate long baseline data size")

                nr_output_files = 0
                subband_groups_per_ms = int(self.parset['longbaseline']['subband_groups_per_ms'])
                subbands_per_subband_group = int(self.parset['longbaseline']['subbands_per_subband_group'])
                if subband_groups_per_ms and subbands_per_subband_group:
                    nr_input_files = int(self.input_files['dp_correlated_uv']['nr_files'])
                    if (nr_input_files % (subbands_per_subband_group * subband_groups_per_ms)) == 0:
                        nr_output_files = nr_input_files / (subbands_per_subband_group * subband_groups_per_ms)
                        self.output_files['dp_correlated_uv'] = {'nr_files': nr_output_files, 'file_size': 1000}
                        logger.debug("dp_correlated_uv: {} files {} bytes each".format(nr_output_files, 1000))

            # count total data size
            for values in self.output_files.itervalues():
                self.total_data_size += values['nr_files'] * values['file_size']
            self.total_bandwidth = ceil((self.total_data_size * 8) / self.duration)  # bits/second
        return

