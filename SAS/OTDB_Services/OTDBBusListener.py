#!/usr/bin/env python

# OTDBBusListener.py: OTDBBusListener listens on the lofar otdb message bus and calls (empty) on<SomeMessage> methods when such a message is received.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: messagebus.py 1580 2015-09-30 14:18:57Z loose $

"""
OTDBBusListener listens on the lofar otdb message bus and calls (empty) on<SomeMessage> methods when such a message is received.
Typical usage is to derive your own subclass from OTDBBusListener and implement the specific on<SomeMessage> methods that you are interested in.
"""

from lofar.messaging.messagebus import AbstractBusListener

import qpid.messaging
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class OTDBBusListener(AbstractBusListener):
    def __init__(self, busname='lofar.otdb.notification', subject='otdb.treestatus', broker=None, **kwargs):
        """
        OTDBBusListener listens on the lofar otdb message bus and calls (empty) on<SomeMessage> methods when such a message is received.
        Typical usage is to derive your own subclass from OTDBBusListener and implement the specific on<SomeMessage> methods that you are interested in.
        :param address: valid Qpid address (default: lofar.otdb.status)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        address = "%s/%s" % (busname, subject)
        super(OTDBBusListener, self).__init__(address, broker, **kwargs)

    def _handleMessage(self, msg):
        logger.debug("OTDBBusListener.handleMessage: %s" %str(msg))

        treeId =  msg.content['treeID']
        modificationTime = datetime.utcnow()
        if 'time_of_change' in msg.content:
            try:
                modificationTime = datetime.strptime(msg.content['time_of_change'], '%Y-%m-%dT%H:%M:%S.%f')
            except:
                pass

        if msg.content['state'] == 'described':
            self.onObservationDescribed(treeId, modificationTime)
        elif msg.content['state'] == 'prepared':
            self.onObservationPrepared(treeId, modificationTime)
        elif msg.content['state'] == 'approved':
            self.onObservationApproved(treeId, modificationTime)
        elif msg.content['state'] == 'on_hold':
            self.onObservationOnHold(treeId, modificationTime)
        elif msg.content['state'] == 'conflict':
            self.onObservationConflict(treeId, modificationTime)
        elif msg.content['state'] == 'prescheduled':
            self.onObservationPrescheduled(treeId, modificationTime)
        elif msg.content['state'] == 'scheduled':
            self.onObservationScheduled(treeId, modificationTime)
        elif msg.content['state'] == 'queued':
            self.onObservationQueued(treeId, modificationTime)
        elif msg.content['state'] == 'active':
            self.onObservationStarted(treeId, modificationTime)
        elif msg.content['state'] == 'completing':
            self.onObservationCompleting(treeId, modificationTime)
        elif msg.content['state'] == 'finished':
            self.onObservationFinished(treeId, modificationTime)
        elif msg.content['state'] == 'aborted':
            self.onObservationAborted(treeId, modificationTime)

    def onObservationDescribed(self, treeId, modificationTime):
        pass

    def onObservationPrepared(self, treeId, modificationTime):
        pass

    def onObservationApproved(self, treeId, modificationTime):
        pass

    def onObservationOnHold(self, treeId, modificationTime):
        pass

    def onObservationConflict(self, treeId, modificationTime):
        pass

    def onObservationPrescheduled(self, treeId, modificationTime):
        pass

    def onObservationScheduled(self, treeId, modificationTime):
        pass

    def onObservationQueued(self, treeId, modificationTime):
        pass

    def onObservationStarted(self, treeId, modificationTime):
        pass

    def onObservationCompleting(self, treeId, modificationTime):
        pass

    def onObservationFinished(self, treeId, modificationTime):
        pass

    def onObservationAborted(self, treeId, modificationTime):
        pass

__all__ = ["OTDBBusListener"]
