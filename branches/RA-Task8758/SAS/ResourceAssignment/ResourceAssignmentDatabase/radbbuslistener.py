#!/usr/bin/env python

# RADBBusListener.py: RADBBusListener listens on the lofar otdb message bus and calls (empty) on<SomeMessage> methods when such a message is received.
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
RADBBusListener listens on the lofar otdb message bus and calls (empty) on<SomeMessage> methods when such a message is received.
Typical usage is to derive your own subclass from RADBBusListener and implement the specific on<SomeMessage> methods that you are interested in.
"""

from lofar.messaging.messagebus import AbstractBusListener
from lofar.sas.resourceassignment.database.config import DEFAULT_BUSNAME
from lofar.common.util import waitForInterrupt

import qpid.messaging
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class RADBBusListener(AbstractBusListener):
    def __init__(self, busname=DEFAULT_BUSNAME, subject='RADB.*', broker=None, **kwargs):
        """
        RADBBusListener listens on the lofar otdb message bus and calls (empty) on<SomeMessage> methods when such a message is received.
        Typical usage is to derive your own subclass from RADBBusListener and implement the specific on<SomeMessage> methods that you are interested in.
        :param address: valid Qpid address (default: lofar.otdb.status)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        address = "%s/%s" % (busname, subject)
        super(RADBBusListener, self).__init__(address, broker, **kwargs)

    def _handleMessage(self, msg):
        logger.debug("RADBBusListener.handleMessage: %s" %str(msg))

        if msg.subject == 'RADB.TaskUpdated':
            self.onTaskUpdated(msg.content)
        elif msg.subject == 'RADB.TaskInserted':
            self.onTaskInserted(msg.content)
        elif msg.subject == 'RADB.TaskDeleted':
            self.onTaskDeleted(msg.content)
        elif msg.subject == 'RADB.ResourceClaimUpdated':
            self.onResourceClaimUpdated(msg.content)
        elif msg.subject == 'RADB.ResourceClaimInserted':
            self.onResourceClaimInserted(msg.content)
        elif msg.subject == 'RADB.ResourceClaimDeleted':
            self.onResourceClaimDeleted(msg.content)
        else:
            logger.error("RADBBusListener.handleMessage: unknown subject: %s" %str(msg.subject))

    def onTaskUpdated(self, task):
        pass

    def onTaskInserted(self, task):
        pass

    def onTaskDeleted(self, task):
        pass

    def onResourceClaimUpdated(self, claim):
        pass

    def onResourceClaimInserted(self, claim):
        logger.info("RADBBusListener.onResourceClaimInserted: %s" %str(claim))

    def onResourceClaimDeleted(self, claim):
        logger.info("RADBBusListener.onResourceClaimDeleted: %s" %str(claim))

if __name__ == '__main__':
    with RADBBusListener(broker='10.149.96.6') as listener:
        waitForInterrupt()

__all__ = ["RADBBusListener"]
