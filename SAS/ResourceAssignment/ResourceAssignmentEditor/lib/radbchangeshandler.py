#!/usr/bin/env python

# RADBChangesHandler.py
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
# $Id: RADBChangesHandler.py 1580 2015-09-30 14:18:57Z loose $

"""
RADBChangesHandler listens on the lofar notification message bus and calls (empty) on<SomeMessage> methods when such a message is received.
Typical usage is to derive your own subclass from RADBChangesHandler and implement the specific on<SomeMessage> methods that you are interested in.
"""

from lofar.sas.resourceassignment.database.config import DEFAULT_NOTIFICATION_BUSNAME, DEFAULT_NOTIFICATION_SUBJECTS
from lofar.sas.resourceassignment.database.radbbuslistener import RADBBusListener
from lofar.common.util import waitForInterrupt
from lofar.mom.momqueryservice.momqueryrpc import MoMRPC
from lofar.mom.momqueryservice.config import DEFAULT_BUSNAME as DEFAULT_MOM_BUSNAME
from lofar.mom.momqueryservice.config import DEFAULT_SERVICENAME as DEFAULT_MOM_SERVICENAME
from lofar.sas.resourceassignment.resourceassignmenteditor.mom import updateTaskMomDetails

import qpid.messaging
import logging
from datetime import datetime, timedelta
from threading import Lock, Condition

logger = logging.getLogger(__name__)

CHANGE_UPDATE_TYPE = 'update'
CHANGE_INSERT_TYPE = 'insert'
CHANGE_DELETE_TYPE = 'delete'

class RADBChangesHandler(RADBBusListener):
    def __init__(self, busname=DEFAULT_NOTIFICATION_BUSNAME, subjects=DEFAULT_NOTIFICATION_SUBJECTS, broker=None, momrpc=None, **kwargs):
        """
        RADBChangesHandler listens on the lofar notification message bus and keeps track of all the change notifications.
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        super(RADBChangesHandler, self).__init__(busname=busname, subjects=subjects, broker=broker, **kwargs)

        self._changes = []
        self._lock = Lock()
        self._changedCondition = Condition()
        self._changeNumber = 0L
        self._momrpc = momrpc

    def _handleChange(self, change):
        '''_handleChange appends a change in the changes list and calls the onChangedCallback.
        :param change: dictionary with the change'''
        with self._lock:
            change['timestamp'] = datetime.utcnow().isoformat()
            self._changeNumber += 1
            change['changeNumber'] = self._changeNumber
            self._changes.append(change)

        self.clearChangesBefore(datetime.utcnow()-timedelta(minutes=5))

        with self._changedCondition:
            self._changedCondition.notifyAll()

    def onTaskUpdated(self, old_task, new_task):
        '''onTaskUpdated is called upon receiving a TaskUpdated message.
        :param task: dictionary with the updated task'''
        new_task['starttime'] = new_task['starttime'].datetime()
        new_task['endtime'] = new_task['endtime'].datetime()
        task_change = {'changeType':CHANGE_UPDATE_TYPE, 'objectType':'task', 'value':new_task}
        self._handleChange(task_change)

    def onTaskInserted(self, task):
        '''onTaskInserted is called upon receiving a TaskInserted message.
        :param task: dictionary with the inserted task'''
        task['starttime'] = task['starttime'].datetime()
        task['endtime'] = task['endtime'].datetime()
        updateTaskMomDetails(task, self._momrpc)
        task_change = {'changeType':CHANGE_INSERT_TYPE, 'objectType':'task', 'value':task}
        self._handleChange(task_change)

    def onTaskDeleted(self, task):
        '''onTaskDeleted is called upon receiving a TaskDeleted message.
        :param task: dictionary with the deleted task'''
        task_change = {'changeType':CHANGE_DELETE_TYPE, 'objectType':'task', 'value':task}
        self._handleChange(task_change)

    def onResourceClaimUpdated(self, old_claim, new_claim):
        '''onResourceClaimUpdated is called upon receiving a ResourceClaimUpdated message.
        :param task: dictionary with the updated claim'''
        new_claim['starttime'] = new_claim['starttime'].datetime()
        new_claim['endtime'] = new_claim['endtime'].datetime()
        claim_change = {'changeType':CHANGE_UPDATE_TYPE, 'objectType':'resourceClaim', 'value':new_claim}
        self._handleChange(claim_change)

    def onResourceClaimInserted(self, claim):
        '''onResourceClaimInserted is called upon receiving a ResourceClaimInserted message.
        :param claim: dictionary with the inserted claim'''
        claim['starttime'] = claim['starttime'].datetime()
        claim['endtime'] = claim['endtime'].datetime()
        claim_change = {'changeType':CHANGE_INSERT_TYPE, 'objectType':'resourceClaim', 'value':claim}
        self._handleChange(claim_change)

    def onResourceClaimDeleted(self, claim):
        '''onResourceClaimDeleted is called upon receiving a ResourceClaimDeleted message.
        :param claim: dictionary with the deleted claim'''
        claim_change = {'changeType':CHANGE_DELETE_TYPE, 'objectType':'resourceClaim', 'value':claim}
        self._handleChange(claim_change)

    def onResourceAvailabilityUpdated(self, old_availability, new_availability):
        claim_change = {'changeType':CHANGE_UPDATE_TYPE, 'objectType':'resourceAvailability', 'value':new_availability}
        self._handleChange(claim_change)

    def onResourceCapacityUpdated(self, old_capacity, new_capacity):
        claim_change = {'changeType':CHANGE_UPDATE_TYPE, 'objectType':'resourceCapacity', 'value':new_capacity}
        self._handleChange(claim_change)

    def getMostRecentChangeNumber(self):
        with self._lock:
            if self._changes:
                return self._changes[-1]['changeNumber']
        return -1L

    def clearChangesBefore(self, timestamp):
        if isinstance(timestamp, datetime):
            timestamp = timestamp.isoformat()

        with self._lock:
            self._changes = [x for x in self._changes if x['timestamp'] >= timestamp]

    def getChangesSince(self, changeNumber):
        with self._changedCondition:
            while True:
                with self._lock:
                    changesSince = [x for x in self._changes if x['changeNumber'] > changeNumber]

                    if changesSince:
                        return changesSince

                self._changedCondition.wait()
