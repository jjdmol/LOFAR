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

""" Base class for Resource Estimators
"""
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class BaseResourceEstimator(object):
    """ Base class for all other resource estmiater classes
    """
    def __init__(self, name):
        self.name = name
        #self.error = ""
        #self.parset = {}
        self.required_keys = ()
        #self.input_files = {}
        #self.output_files = {}
        #self.duration = 0  # in seconds
        #self.total_data_size = 0  # size in bytes
        #self.total_bandwidth = 0  # in bytes/second

    def _checkParsetForRequiredKeys(self, parset):
        """ Check if all required keys needed are available """
        logger.debug("required keys: %s" % ', '.join(self.required_keys))
        logger.debug("parset   keys: %s" % ', '.join(parset.keys()))
        missing_keys = set(self.required_keys) - set(parset.keys())
        if missing_keys:
            logger.error("missing keys: %s" % ', '.join(missing_keys))
            return False
        return True
    
    def _getDateTime(date_time):
        return datetime.strptime(date_time, '%Y-%m-%d %H:%M:%S')

    def _calculate(self, parset, input_files={}):
        raise NotImplementedError('estimate() in base class is called. Please implement estimate() in your subclass')

    def estimate(self, parset, input_files={}):
        """ Create estimates for a single process based on its parset and input files"""
        if self._checkParsetForRequiredKeys(parset):
            estimates = self._calculate(parset, input_files)
        else:
            raise ValueError('The parset is incomplete')
        result = {}
        result[self.name] = {}
        result[self.name]['total_data_size'] = estimates['total_data_size']
        result[self.name]['total_bandwidth'] = estimates['total_bandwidth']
        result[self.name]['output_files']    = estimates['output_files']
        return result

    def estimate(self):
        raise NotImplementedError('estimate() in base class is called. Please implement estimate() in your subclass')

#    def result_as_dict(self):
#        """ return estimated values as dict """
#        return result
