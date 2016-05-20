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

DATAPRODUCTS = "Observation.DataProducts."
PIPELINE = "Observation.ObservationControl.PythonControl."

#Observation.DataProducts.Output_Correlated.storageClusterName=
#Observation.ObservationControl.PythonControl.AWimager

class ImagePipelineResourceEstimator(BaseResourceEstimator):
    """ ResourceEstimator for Imaging Pipelines
    """
    def __init__(self, kwargs, input_files):
        logger.info("init ImagePipelineResourceEstimator")
        BaseResourceEstimator.__init__(self, name='imaging_pipeline')
        self.required_keys = ('Observation.startTime',
                              'Observation.stopTime',
                              DATAPRODUCTS + 'Input_Correlated.enabled',
                              DATAPRODUCTS + 'Output_SkyImage.enabled',
                              PIPELINE + 'Imaging.slices_per_image',
                              PIPELINE + 'Imaging.subbands_per_image')

    def _calculate(self, parset, input_files):
        """ Estimate for Imaging Pipeline. Also gets used for MSSS Imaging Pipeline
        calculates: datasize (number of files, file size), bandwidth
        input_files should look something like:
        'input_files': 
        {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104}, ...}
        
        reply is something along the lines of:
        {'bandwidth': {'total_size': 19021319494},
        'storage': {'total_size': 713299481024,
        'output_files': 
          {'img': {'nr_of_img_files': 481, 'img_file_size': 148295}
        }}
        """
        logger.debug("start estimate '{}'".format(self.name))
        logger.info('parset: %s ' % parset)
        result = {'errors': []}
        duration = self._getDuration(parset.getString('Observation.startTime'), parset.getString('Observation.stopTime'))
        slices_per_image = parset.getInt(PIPELINE + 'Imaging.slices_per_image', 0) #TODO, should these have defaults?
        subbands_per_image = parset.getInt(PIPELINE + 'Imaging.subbands_per_image', 0)

        if not parset.getBool(DATAPRODUCTS + 'Output_SkyImage.enabled'):
            logger.warning('Output_SkyImage is not enabled')
            result['errors'].append('Output_SkyImage is not enabled')
        if not 'uv' in input_files:
            logger.warning('Missing UV Dataproducts in input_files')
            result['errors'].append('Missing UV Dataproducts in input_files')
        else:
            nr_input_subbands = input_files['uv']['nr_of_uv_files']
        if not slices_per_image or not subbands_per_image:
            logger.warning('slices_per_image or subbands_per_image are not valid')
            result['errors'].append('Missing UV Dataproducts in input_files')
        if nr_input_subbands % (subbands_per_image * slices_per_image) > 0:
            logger.warning('slices_per_image and subbands_per_image not a multiple of number of inputs')
            result['errors'].append('slices_per_image and subbands_per_image not a multiple of number of inputs')
        if result['errors']:
            return result

        logger.debug("calculate sky image data size")
        result['output_files'] = {}
        nr_images = nr_input_subbands / (subbands_per_image * slices_per_image)
        result['output_files']['img'] = {'nr_of_img_files': nr_images, 'img_file_size': 1000} # 1 kB was hardcoded in the Scheduler 
        logger.debug("sky_images: {} files {} bytes each".format(result['output_files']['img']['nr_of_img_files'], result['output_files']['img']['img_file_size']))

        # count total data size
        total_data_size = result['output_files']['img']['nr_of_img_files'] * result['output_files']['img']['img_file_size'] # bytes
        total_bandwidth = int(ceil((total_data_size * 8) / duration))  # bits/second
        result['storage'] = {'total_size': total_data_size}
        result['bandwidth'] = {'total_size': total_bandwidth}
        return result

