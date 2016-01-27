#!/usr/bin/python

# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

# $Id: radb.py 33394 2016-01-25 15:53:55Z schaap $

'''
TODO: documentation
'''
import logging
import psycopg2
import psycopg2.extras
import datetime
import time
import json

from lofar.common.postgres import PostgresListener
from lofar.messaging import EventMessage, ToBus

logger = logging.getLogger(__name__)


class RADBListener(PostgresListener):
    def __init__(self,
                 host='10.149.96.6', #mcu005.control.lofar
                 database='resourceassignment',
                 username='resourceassignment',
                 password='123456',
                 busname='lofar.ra.notification',
                 broker=None):
        super(RADBListener, self).__init__(host, database, username, password)

        self.event_bus = ToBus(busname, broker=broker)

        self.setupPostgresNotifications('resource_allocation', 'task')
        self.subscribe('task_update', self.onTaskUpdated)
        self.subscribe('task_insert', self.onTaskInserted)
        self.subscribe('task_delete', self.onTaskDeleted)

        self.setupPostgresNotifications('resource_allocation', 'resource_claim')
        self.subscribe('resource_claim_update', self.onResourceClaimUpdated)
        self.subscribe('resource_claim_insert', self.onResourceClaimInserted)
        self.subscribe('resource_claim_delete', self.onResourceClaimDeleted)

    def onTaskUpdated(self, payload = None):
        self._sendNotification('TaskUpdated', payload)

    def onTaskInserted(self, payload = None):
        self._sendNotification('TaskInserted', payload)

    def onTaskDeleted(self, payload = None):
        self._sendNotification('TaskDeleted', payload)

    def onResourceClaimUpdated(self, payload = None):
        self._sendNotification('ResourceClaimUpdated', payload)

    def onResourceClaimInserted(self, payload = None):
        self._sendNotification('ResourceClaimInserted', payload)

    def onResourceClaimDeleted(self, payload = None):
        self._sendNotification('ResourceClaimDeleted', payload)

    def __enter__(self):
        super(RADBListener, self).__enter__()
        self.event_bus.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        super(RADBListener, self).__exit__(exc_type, exc_val, exc_tb)
        self.event_bus.close()

    def _sendNotification(self, subject, payload):
        try:
            content = json.loads(payload)
        except Exception as e:
            logger.error('Could not parse payload: %s\n%s' % (payload, e))
            content=None

        try:
            msg = EventMessage(context=subject, content=content)
            self.event_bus.send(msg)
        except Exception as e:
            logger.error(str(e))

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.INFO)

    with RADBListener() as listener:
        listener.waitWhileListening()



