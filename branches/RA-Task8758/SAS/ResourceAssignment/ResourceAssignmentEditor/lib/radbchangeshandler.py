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

from lofar.sas.resourceassignment.database.config import DEFAULT_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.radbbuslistener import RADBBusListener
from lofar.common.util import waitForInterrupt

import qpid.messaging
import logging
from datetime import datetime
from threading import Lock

logger = logging.getLogger(__name__)

CHANGE_UPDATE_TYPE = 'update'
CHANGE_INSERT_TYPE = 'insert'
CHANGE_DELETE_TYPE = 'delete'

class RADBChangesHandler(RADBBusListener):
    def __init__(self, busname=DEFAULT_BUSNAME, subject='RADB.*', broker=None, **kwargs):
        """
        RADBChangesHandler listens on the lofar notification message bus and keeps track of all the change notifications.
        :param busname: valid Qpid address (default: lofar.ra.notification)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        address = "%s/%s" % (busname, subject)
        super(RADBChangesHandler, self).__init__(busname=busname, subject=subject, broker=broker, **kwargs)

        self._changes = []
        self._lock = Lock()

        self.onChangedCallback = None

    def _handleChange(self, change):
        '''_handleChange appends a change in the changes list and calls the onChangedCallback.
        :param change: dictionary with the change'''
        change['timestamp'] = datetime.utcnow().isoformat()
        with self._lock:
            self._changes.append(change)

        if self.onChangedCallback:
            self.onChangedCallback()

    def onTaskUpdated(self, task):
        '''onTaskUpdated is called upon receiving a TaskUpdated message.
        :param task: dictionary with the updated task'''
        task_change = {'changeType':CHANGE_UPDATE_TYPE, 'objectType':'task', 'value':task}
        self._handleChange(task_change)

    def onTaskInserted(self, task):
        '''onTaskInserted is called upon receiving a TaskInserted message.
        :param task: dictionary with the inserted task'''

        task_change = {'changeType':CHANGE_INSERT_TYPE, 'objectType':'task', 'value':task}
        self._handleChange(task_change)

    def onTaskDeleted(self, task):
        '''onTaskDeleted is called upon receiving a TaskDeleted message.
        :param task: dictionary with the deleted task'''
        task_change = {'changeType':CHANGE_DELETE_TYPE, 'objectType':'task', 'value':task}
        self._handleChange(task_change)

    def onResourceClaimUpdated(self, claim):
        '''onResourceClaimUpdated is called upon receiving a ResourceClaimUpdated message.
        :param task: dictionary with the updated claim'''
        claim_change = {'changeType':CHANGE_UPDATE_TYPE, 'objectType':'resourceClaim', 'value':claim}
        self._handleChange(claim_change)

    def onResourceClaimInserted(self, claim):
        '''onResourceClaimInserted is called upon receiving a ResourceClaimInserted message.
        :param claim: dictionary with the inserted claim'''
        claim_change = {'changeType':CHANGE_INSERT_TYPE, 'objectType':'resourceClaim', 'value':claim}
        self._handleChange(claim_change)

    def onResourceClaimDeleted(self, claim):
        '''onResourceClaimDeleted is called upon receiving a ResourceClaimDeleted message.
        :param claim: dictionary with the deleted claim'''
        claim_change = {'changeType':CHANGE_DELETE_TYPE, 'objectType':'resourceClaim', 'value':claim}
        self._handleChange(claim_change)

    def clearChangesBefore(self, timestamp):
        if isinstance(timestamp, datetime):
            timestamp = timestamp.isoformat()
            
        with self._lock:
            self._changes = [x for x in self._changes if x['timestamp'] >= timestamp]

    def getChangesSince(self, timestamp):
        if isinstance(timestamp, datetime):
            timestamp = timestamp.isoformat()
        print
        print 'getChangesSince: '
        with self._lock:
            changesSince = [x for x in self._changes if x['timestamp'] > timestamp]
            for x in changesSince:
                print x['timestamp']
            return changesSince

