#!/usr/bin/env python

# rapublisher.py: publishes notification messages when resource assigner has done something
#
# Copyright (C) 2015
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: rapublisher.py 1580 2015-09-30 14:18:57Z loose $

"""
publishes notification messages when resource assigner has done something"""

from lofar.messaging.messagebus import ToBus
from lofar.messaging.messages import EventMessage
from lofar.sas.resourceassignment.resourceassigner.config import RA_NOTIFICATION_BUSNAME, RA_NOTIFICATION_PREFIX

import logging

logger = logging.getLogger(__name__)

class RAPublisher():
    def __init__(self,
                 busname=RA_NOTIFICATION_BUSNAME,
                 notification_prefix=RA_NOTIFICATION_PREFIX,
                 broker=None):
        """
        """
        self.tobus = ToBus(address=busname, broker=broker)
        self.notification_prefix = notification_prefix

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def open(self):
        self.tobus.open()

    def close(self):
        self.tobus.close()

    def notifyTaskSpecified(self, taskId, status):
        content = { 'task_id' : taskId, 'status' : status }
        logger.info('notifyTaskSpecified: %s' % content)
        msg = EventMessage(context=self.notification_prefix + 'TaskSpecified', content=content)
        self.tobus.send(msg)

