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
from lofar.common.datetimeutils import totalSeconds
from datetime import datetime, timedelta
from lofar.parameterset import parameterset

logger = logging.getLogger(__name__)


class BaseResourceEstimator(object):
    """ Base class for all other resource estmiater classes
    """
    def __init__(self, name):
        self.name = name
        self.required_keys = ()

    def _checkParsetForRequiredKeys(self, parset):
        """ Check if all required keys needed are available """
        logger.debug("required keys: %s" % ', '.join(self.required_keys))
        logger.debug("parset   keys: %s" % ', '.join(parset.keys()))
        missing_keys = set(self.required_keys) - set(parset.keys())
        if missing_keys:
            logger.error("missing keys: %s" % ', '.join(missing_keys))
            return False
        return True
    
    def _getDateTime(self, date_time):
        return datetime.strptime(date_time, '%Y-%m-%d %H:%M:%S')

    def _getDuration(self, start, end):
        startTime = self._getDateTime(start)
        endTime = self._getDateTime(end)
        if startTime >= endTime:
            logger.warning("startTime is after endTime")
            return 1 ##TODO To prevent divide by zero later
        return totalSeconds(endTime - startTime)
        #TODO check if this makes duration = int(parset.get('duration', 0)) as a key reduntant?

    def _calculate(self, parset, input_files={}):
        raise NotImplementedError('calculate() in base class is called. Please implement calculate() in your subclass')

    def verify_and_estimate(self, parset, input_files={}):
        """ Create estimates for a single process based on its parset and input files"""
        if self._checkParsetForRequiredKeys(parset):
            estimates = self._calculate(parameterset(parset), input_files)
        else:
            raise ValueError('The parset is incomplete')
        result = {}
        result[self.name] = {}
        result[self.name]['storage'] = estimates['storage']
        result[self.name]['bandwidth'] = estimates['bandwidth']
        return result
