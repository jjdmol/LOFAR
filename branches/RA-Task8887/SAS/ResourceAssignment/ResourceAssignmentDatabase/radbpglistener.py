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
from optparse import OptionParser

from lofar.common.postgres import PostgresListener, makePostgresNotificationQueries
from lofar.messaging import EventMessage, ToBus
from lofar.sas.resourceassignment.database.config import DEFAULT_NOTIFICATION_BUSNAME, DEFAULT_NOTIFICATION_PREFIX
from lofar.common import dbcredentials

logger = logging.getLogger(__name__)

class RADBPGListener(PostgresListener):
    def __init__(self,
                 busname=DEFAULT_NOTIFICATION_BUSNAME,
                 notification_prefix=DEFAULT_NOTIFICATION_PREFIX,
                 dbcreds=None,
                 broker=None):
        super(RADBPGListener, self).__init__(dbcreds.host, dbcreds.database, dbcreds.user, dbcreds.password)

        self.notification_prefix = notification_prefix
        self.event_bus = ToBus(busname, broker=broker)

        self.subscribe('task_update_with_task_view', self.onTaskUpdated)
        self.subscribe('task_insert_with_task_view', self.onTaskInserted)
        self.subscribe('task_delete', self.onTaskDeleted)

        # when the specification starttime and endtime are updated, then that effects the task as well
        # so subscribe to specification_update, and use task_view as view_for_row
        self.subscribe('specification_update_with_task_view', self.onSpecificationUpdated)

        self.subscribe('resource_claim_update_with_resource_claim_view', self.onResourceClaimUpdated)
        self.subscribe('resource_claim_insert_with_resource_claim_view', self.onResourceClaimInserted)
        self.subscribe('resource_claim_delete', self.onResourceClaimDeleted)

    def onTaskUpdated(self, payload = None):
        self._sendNotification('TaskUpdated', payload, ['starttime', 'endtime'])

    def onTaskInserted(self, payload = None):
        self._sendNotification('TaskInserted', payload, ['starttime', 'endtime'])

    def onTaskDeleted(self, payload = None):
        self._sendNotification('TaskDeleted', payload)

    def onSpecificationUpdated(self, payload = None):
        # when the specification starttime and endtime are updated, then that effects the task as well
        # so send a TaskUpdated notification
        self._sendNotification('TaskUpdated', payload, ['starttime', 'endtime'])

    def onResourceClaimUpdated(self, payload = None):
        self._sendNotification('ResourceClaimUpdated', payload, ['starttime', 'endtime'])

    def onResourceClaimInserted(self, payload = None):
        self._sendNotification('ResourceClaimInserted', payload, ['starttime', 'endtime'])

    def onResourceClaimDeleted(self, payload = None):
        self._sendNotification('ResourceClaimDeleted', payload)

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
            for state in ('old', 'new'):
                if state in contentDict:
                    for field in fields:
                        try:
                            if contentDict[state] and field in contentDict[state]:
                                timestampStr = contentDict[state][field]
                                if timestampStr.rfind('.') > -1:
                                    timestamp = datetime.strptime(timestampStr, '%Y-%m-%d %H:%M:%S.%f')
                                else:
                                    timestamp = datetime.strptime(timestampStr, '%Y-%m-%d %H:%M:%S')

                                contentDict[state][field] = timestamp
                        except Exception as e:
                            logger.error('Could not convert field \'%s\' to datetime: %s' % (field, e))

            return contentDict
        except Exception as e:
            logger.error('Could not parse payload: %s\n%s' % (contentDict, e))


    def _sendNotification(self, subject, payload, timestampFields = None):
        try:
            content = json.loads(payload)

            if 'new' in content and content['new'] and 'old' in content and content['old']:
                # check if new and old are equal.
                # however, new and old can be based on different views,
                # so, only check the values for the keys they have in common
                new_keys = set(content['new'].keys())
                old_keys = set(content['old'].keys())
                common_keys = new_keys & old_keys
                equal_valued_keys = [k for k in common_keys if content['new'][k] == content['old'][k]]
                if len(equal_valued_keys) == len(common_keys):
                    logger.info('new and old values are equal, not sending notification. %s' % (content['new']))
                    return

            if timestampFields:
                content = self._formatTimestampsAsIso(timestampFields, content)
        except Exception as e:
            logger.error('Could not parse payload: %s\n%s' % (payload, e))
            content=None

        try:
            msg = EventMessage(context=self.notification_prefix + subject, content=content)
            logger.info('Sending notification: ' + str(msg).replace('\n', ' '))
            self.event_bus.send(msg)
        except Exception as e:
            logger.error(str(e))

def main():
    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the radb postgres listener which listens to changes on some tables in the radb and publishes the changes as notifications on the bus.')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_NOTIFICATION_BUSNAME, help="Name of the publication bus on the qpid broker, [default: %default]")
    parser.add_option("-n", "--notification_prefix", dest="notification_prefix", type="string", default=DEFAULT_NOTIFICATION_PREFIX, help="The prefix for all notifications of this publisher, [default: %default]")
    parser.add_option_group(dbcredentials.options_group(parser))
    parser.set_defaults(dbcredentials="RADB")
    (options, args) = parser.parse_args()

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.INFO)

    dbcreds = dbcredentials.parse_options(options)

    logger.info("Using dbcreds: %s" % dbcreds.stringWithHiddenPassword())

    with RADBPGListener(busname=options.busname,
                        notification_prefix=options.notification_prefix,
                        dbcreds=dbcreds,
                        broker=options.broker) as listener:
        listener.waitWhileListening()

def make_radb_postgres_notification_queries():
    print makePostgresNotificationQueries('resource_allocation', 'task', 'INSERT', view_for_row='task_view')
    print makePostgresNotificationQueries('resource_allocation', 'task', 'UPDATE', view_for_row='task_view')
    print makePostgresNotificationQueries('resource_allocation', 'task', 'DELETE')
    print makePostgresNotificationQueries('resource_allocation', 'specification', 'UPDATE', view_for_row='task_view', view_selection_id='specification_id')
    print makePostgresNotificationQueries('resource_allocation', 'resource_claim', 'INSERT', view_for_row='resource_claim_view')
    print makePostgresNotificationQueries('resource_allocation', 'resource_claim', 'UPDATE', view_for_row='resource_claim_view')
    print makePostgresNotificationQueries('resource_allocation', 'resource_claim', 'DELETE')

if __name__ == '__main__':
    main()
