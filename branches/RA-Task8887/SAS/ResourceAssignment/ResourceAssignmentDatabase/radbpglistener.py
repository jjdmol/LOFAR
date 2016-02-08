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
from datetime import datetime
import time
import json

from lofar.common.postgres import PostgresListener
from lofar.messaging import EventMessage, ToBus
from lofar.sas.resourceassignment.database.config import radb_password, DEFAULT_BUSNAME

logger = logging.getLogger(__name__)

class RADBPGListener(PostgresListener):
    def __init__(self,
                 host='10.149.96.6', #mcu005.control.lofar
                 database='resourceassignment',
                 username='resourceassignment',
                 password=radb_password,
                 busname=DEFAULT_BUSNAME,
                 broker=None):
        super(RADBPGListener, self).__init__(host, database, username, password)

        self.event_bus = ToBus(busname, broker=broker)

        self.setupPostgresNotifications('resource_allocation', 'task', view_for_row='task_view')
        self.subscribe('task_update', self.onTaskUpdated)
        self.subscribe('task_insert', self.onTaskInserted)
        self.subscribe('task_delete', self.onTaskDeleted)

        self.setupPostgresNotifications('resource_allocation', 'resource_claim', view_for_row='resource_claim_view')
        self.subscribe('resource_claim_update', self.onResourceClaimUpdated)
        self.subscribe('resource_claim_insert', self.onResourceClaimInserted)
        self.subscribe('resource_claim_delete', self.onResourceClaimDeleted)

    def onTaskUpdated(self, payload = None):
        self._sendNotification('RADB.TaskUpdated', payload, ['starttime', 'endtime'])

    def onTaskInserted(self, payload = None):
        self._sendNotification('RADB.TaskInserted', payload, ['starttime', 'endtime'])

    def onTaskDeleted(self, payload = None):
        self._sendNotification('RADB.TaskDeleted', payload)

    def onResourceClaimUpdated(self, payload = None):
        self._sendNotification('RADB.ResourceClaimUpdated', payload, ['starttime', 'endtime'])

    def onResourceClaimInserted(self, payload = None):
        self._sendNotification('RADB.ResourceClaimInserted', payload, ['starttime', 'endtime'])

    def onResourceClaimDeleted(self, payload = None):
        self._sendNotification('RADB.ResourceClaimDeleted', payload)

    def __enter__(self):
        super(RADBPGListener, self).__enter__()
        self.event_bus.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        super(RADBPGListener, self).__exit__(exc_type, exc_val, exc_tb)
        self.event_bus.close()

    def _formatTimestampsAsIso(self, fields, contentDict):
        '''convert all requested fields in the contentDict to proper isoformat datetime strings.
        In postgres we use timestamps without timezone.
        By convention we only enter utc values.
        But, if they are json encoded by postgress, they are not properly formatted with the in isoformat with a 'Z' at the end.
        So, parse the requested fields, and return them as datetime.
        '''
        try:
            for field in fields:
                try:
                    timestampStr = contentDict[field]
                    if timestampStr.rfind('.') > -1:
                        timestamp = datetime.strptime(timestampStr, '%Y-%m-%d %H:%M:%S.%f')
                    else:
                        timestamp = datetime.strptime(timestampStr, '%Y-%m-%d %H:%M:%S')

                    contentDict[field] = timestamp
                except Exception as e:
                    logger.error('Could not convert field \'%s\' to datetime: %s' % (field, e))

            return contentDict
        except Exception as e:
            logger.error('Could not parse payload: %s\n%s' % (contentDict, e))


    def _sendNotification(self, subject, payload, timestampFields = None):
        try:
            content = json.loads(payload)
            if timestampFields:
                content = self._formatTimestampsAsIso(timestampFields, content)
        except Exception as e:
            logger.error('Could not parse payload: %s\n%s' % (payload, e))
            content=None

        try:
            msg = EventMessage(context=subject, content=content)
            logger.info('Sending notification: ' + str(msg).replace('\n', ' '))
            self.event_bus.send(msg)
        except Exception as e:
            logger.error(str(e))

def main(busname=DEFAULT_BUSNAME):
    with RADBPGListener(busname=busname, password=radb_password) as listener:
        listener.waitWhileListening()

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.INFO)
    main(busname=DEFAULT_BUSNAME)



