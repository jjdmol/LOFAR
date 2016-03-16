#!/usr/bin/env python

# RABusListener.py: RABusListener listens on the lofar ra message bus and calls (empty) on<SomeMessage> methods when such a message is received.
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
# $Id: RABusListener.py 33534 2016-02-08 14:28:26Z schaap $

"""
RABusListener listens on the lofar ra message bus and calls (empty) on<SomeMessage> methods when such a message is received.
Typical usage is to derive your own subclass from RABusListener and implement the specific on<SomeMessage> methods that you are interested in.
"""

from lofar.messaging.messagebus import AbstractBusListener
from lofar.sas.resourceassignment.resourceassigner.config import RA_NOTIFICATION_BUSNAME, RA_NOTIFICATION_SUBJECTS

import qpid.messaging
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class RANotificationBusListener(AbstractBusListener):
    def __init__(self, busname=RA_NOTIFICATION_BUSNAME, subject=RA_NOTIFICATION_SUBJECTS, broker=None, **kwargs):
        """
        RANotificationBusListener listens on the lofar ra message bus and calls (empty) on<SomeMessage> methods when such a message is received.
        Typical usage is to derive your own subclass from RANotificationBusListener and implement the specific on<SomeMessage> methods that you are interested in.
        :param address: valid Qpid address (default: lofar.ra.notification)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        address = "%s/%s" % (busname, subject)
        super(RANotificationBusListener, self).__init__(address, broker, **kwargs)

    def _handleMessage(self, msg):
        logger.debug("RANotificationBusListener._handleMessage: %s" % str(msg))

        if msg.subject == "RA.TaskStatusChanged": ##maybe we should read the prefix from a config?
            content = msg.content
            logger.info('TaskStatusChanged received: status:%s id:%s otdb_id:%s mom_id:%s' % (content["status"], content["task_id"], content["otdb_id"], content["mom_id"]))
            if content["status"] == "scheduled":
                self.onTaskScheduled(content["task_id"], content["otdb_id"], content["mom_id"])
            elif content["status"] == "conflict":
                self.onTaskConflict(content["task_id"], content["otdb_id"], content["mom_id"])
            else:
                logger.warning('RANotificationBusListener received unknown status: %s' % (content["status"],))


    def onTaskScheduled(self, raId, otdbId, momId):
        pass

    def onTaskConflict(self, raId, otdbId, momId):
        pass


__all__ = ["RANotificationBusListener"]
