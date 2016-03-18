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

class ImagePipelineResourceEstimator(BaseResourceEstimator):
    """ ImagePipelineResourceEstimator

    """
    def __init__(self, kwargs, input_files):
        BaseResourceEstimator.__init__(self, name='image_pipeline')
        self.parset = ParameterSet(kwargs).make_subset('dp.output')
        self.duration = int(kwargs.get('observation.duration', 1))
        self.input_files = input_files
        self.required_keys = ('skyimage.enabled', 'skyimage.slices_per_image', 'skyimage.subbands_per_image')
        if self.checkParsetForRequiredKeys():
            self.estimate()
        return

    def estimate(self):
        """ Estimate for image pipeline
        calculates: datasize (number of files, file size), bandwidth
        """
        logger.debug("start estimate '{}'".format(self.name))
        if 'dp_correlated_uv' in self.input_files:
            if self.parset['skyimage']['enabled'] == 'true':
                logger.debug("calculate skyimage data size")
                slices_per_image = int(self.parset['skyimage']['slices_per_image'])
                subbands_per_image = int(self.parset['skyimage']['subbands_per_image'])
                if slices_per_image and subbands_per_image:
                    nr_input_subbands = int(self.input_files['dp_correlated_uv']['nr_files'])
                    if (nr_input_subbands % (subbands_per_image * slices_per_image)) == 0:
                        nr_images = nr_input_subbands / (subbands_per_image * slices_per_image)
                        self.output_files['dp_sky_image'] = {'nr_files': nr_images, 'file_size': 1000}
                        logger.debug("dp_sky_image: {} files {} bytes each".format(nr_images, 1000))

            # count total data size
            for values in self.output_files.itervalues():
                self.total_data_size += values['nr_files'] * values['file_size']
            self.total_bandwidth = ceil((self.total_data_size * 8) / self.duration)  # bits/second
        return

