#!/usr/bin/env python

# RADBBusListener.py
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
# $Id: RADBBusListener.py 1580 2015-09-30 14:18:57Z loose $

"""
RADBBusListener listens on the lofar notification message bus and calls (empty) on<SomeMessage> methods when such a message is received.
Typical usage is to derive your own subclass from RADBBusListener and implement the specific on<SomeMessage> methods that you are interested in.
"""

from lofar.messaging.messagebus import AbstractBusListener
from lofar.sas.resourceassignment.database.config import DEFAULT_NOTIFICATION_BUSNAME, DEFAULT_NOTIFICATION_SUBJECTS
from lofar.common.util import waitForInterrupt

import qpid.messaging
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class RADBBusListener(AbstractBusListener):
    def __init__(self, busname=DEFAULT_NOTIFICATION_BUSNAME, subjects=DEFAULT_NOTIFICATION_SUBJECTS, broker=None, **kwargs):
        """
        RADBBusListener listens on the lofar notification message bus and calls (empty) on<SomeMessage> methods when such a message is received.
        Typical usage is to derive your own subclass from RADBBusListener and implement the specific on<SomeMessage> methods that you are interested in.
        :param busname: valid Qpid address (default: lofar.ra.notification)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        self.subject_prefix = (subjects.split('.')[0]+'.') if '.' in subjects else ''

        address = "%s/%s" % (busname, subjects)
        super(RADBBusListener, self).__init__(address, broker, **kwargs)


    def _handleMessage(self, msg):
        logger.info("on%s: %s" % (msg.subject.replace(self.subject_prefix, ''), str(msg.content).replace('\n', ' ')))

        if msg.subject == '%sTaskUpdated' % self.subject_prefix:
            self.onTaskUpdated(msg.content.get('old'), msg.content.get('new'))
        elif msg.subject == '%sTaskInserted' % self.subject_prefix:
            self.onTaskInserted(msg.content.get('new'))
        elif msg.subject == '%sTaskDeleted' % self.subject_prefix:
            self.onTaskDeleted(msg.content.get('old'))
        elif msg.subject == '%sResourceClaimUpdated' % self.subject_prefix:
            self.onResourceClaimUpdated(msg.content.get('old'), msg.content.get('new'))
        elif msg.subject == '%sResourceClaimInserted' % self.subject_prefix:
            self.onResourceClaimInserted(msg.content.get('new'))
        elif msg.subject == '%sResourceClaimDeleted' % self.subject_prefix:
            self.onResourceClaimDeleted(msg.content.get('old'))
        elif msg.subject == '%sResourceAvailabilityUpdated' % self.subject_prefix:
            self.onResourceAvailabilityUpdated(msg.content.get('old'), msg.content.get('new'))
        elif msg.subject == '%sResourceCapacityUpdated' % self.subject_prefix:
            self.onResourceCapacityUpdated(msg.content.get('old'), msg.content.get('new'))
        else:
            logger.error("RADBBusListener.handleMessage: unknown subject: %s" %str(msg.subject))

    def onTaskUpdated(self, old_task, new_task):
        '''onTaskUpdated is called upon receiving a TaskUpdated message.
        :param old_task: dictionary with the task before the update
        :param new_task: dictionary with the updated task'''
        pass

    def onTaskInserted(self, new_task):
        '''onTaskInserted is called upon receiving a TaskInserted message.
        :param new_task: dictionary with the inserted task'''
        pass

    def onTaskDeleted(self, old_task):
        '''onTaskDeleted is called upon receiving a TaskDeleted message.
        :param old_task: dictionary with the deleted task'''
        pass

    def onResourceClaimUpdated(self, old_claim, new_claim):
        '''onResourceClaimUpdated is called upon receiving a ResourceClaimUpdated message.
        :param old_claim: dictionary with the claim before the update
        :param new_claim: dictionary with the updated claim'''
        pass

    def onResourceClaimInserted(self, new_claim):
        '''onResourceClaimInserted is called upon receiving a ResourceClaimInserted message.
        :param new_claim: dictionary with the inserted claim'''
        pass

    def onResourceClaimDeleted(self, old_claim):
        '''onResourceClaimDeleted is called upon receiving a ResourceClaimDeleted message.
        :param old_claim: dictionary with the deleted claim'''
        pass

    def onResourceAvailabilityUpdated(self, old_availability, new_availability):
        '''onResourceAvailabilityUpdated is called upon receiving a ResourceAvailabilityUpdated message.
        :param old_availability: dictionary with the resource availability before the update
        :param new_availability: dictionary with the updated availability'''
        pass

    def onResourceCapacityUpdated(self, old_capacity, new_capacity):
        '''onResourceCapacityUpdated is called upon receiving a ResourceCapacityUpdated message.
        :param old_capacity: dictionary with the resource capacity before the update
        :param new_capacity: dictionary with the updated capacity'''
        pass


if __name__ == '__main__':
    with RADBBusListener(broker=None) as listener:
        waitForInterrupt()

__all__ = ["RADBBusListener"]
