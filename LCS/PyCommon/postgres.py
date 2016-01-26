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

# $Id$

'''
Module with nice postgres helper methods and classes.
'''

import logging
from threading import Thread, Lock
from Queue import Queue, Empty
import select
import psycopg2
import psycopg2.extras
import psycopg2.extensions

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                    level=logging.INFO)
logger = logging.getLogger(__name__)


class PostgresListener(object):
    ''' This class lets you listen to postgress notifications
    It execute callbacks when a notifocation occurs.
    Make your own subclass with your callbacks and subscribe them to the appriate channel.
    Example:

    class MyListener(PostgresListener):
        def __init__(self, host, database, username, password):
            super(MyListener, self).__init__(host=host, database=database, username=username, password=password)
            self.subscribe('foo', self.foo)
            self.subscribe('bar', self.bar)

        def foo(self, payload = None):
            print "Foo called with payload: ", payload

        def bar(self, payload = None):
            print "Bar called with payload: ", payload

    with MyListener(...args...) as listener:
        #either listen like below in a loop doing stuff...
        while True:
            #do stuff or wait,
            #the listener calls the callbacks meanwhile in another thread

        #... or listen like below blocking
        #while the listener calls the callbacks meanwhile in this thread
        listener.waitWhileListening()
    '''
    def __init__(self,
                 host='',
                 database='',
                 username='',
                 password=''):
        '''Create a new PostgresListener'''
        self.conn = psycopg2.connect(host=host,
                                     user=username,
                                     password=password,
                                     database=database)
        self.conn.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)
        self.cursor = self.conn.cursor()
        self.__listening = False
        self.__lock = Lock()
        self.__callbacks = {}
        self.__waiting = False
        self.__queue = Queue()

    def subscribe(self, notification, callback):
        '''Subscribe to a certain postgres notification.
        Call callback method in case such a notification is received.'''
        logger.info("Subscribed %sto %s" % ('and listening ' if self.isListening() else '', notification))
        with self.__lock:
            self.cursor.execute("LISTEN %s;", (psycopg2.extensions.AsIs(notification),))
            self.__callbacks[notification] = callback

    def unsubscribe(self, notification):
        '''Unubscribe from a certain postgres notification.'''
        logger.info("Unsubscribed from %s" % notification)
        with self.__lock:
            self.cursor.execute("UNLISTEN %s;", (psycopg2.extensions.AsIs(notification),))
            if notification in self.__callbacks:
                del self.__callbacks[notification]

    def setupPostgresNotifications(self, schema, table, updateNotification=True, insertNotification=True, deleteNotification=True):
        items = []
        if updateNotification:
            items.append(('update', 'NEW'))

        if insertNotification:
            items.append(('insert', 'NEW'))

        if deleteNotification:
            items.append(('delete', 'OLD'))

        for item in items:
            sql = '''
            CREATE OR REPLACE FUNCTION {schema}.notify_{table}_{action}()
            RETURNS trigger AS $$
            BEGIN
            PERFORM pg_notify(CAST('{table}_{action}' AS text), row_to_json({value})::text);
            RETURN {value};
            END;
            $$ LANGUAGE plpgsql;

            DROP TRIGGER IF EXISTS trigger_notify_{table}_{action} ON {schema}.{table};

            CREATE TRIGGER trigger_notify_{table}_{action}
            AFTER {action} ON {schema}.{table}
            FOR EACH ROW
            EXECUTE PROCEDURE {schema}.notify_{table}_{action}();
            '''.format(schema=schema,
                       table=table,
                       action=item[0],
                       value=item[1])
            self.cursor.execute(sql)

    def isListening(self):
        '''Are we listening? Has the listener been started?'''
        with self.__lock:
            return self.__listening

    def start(self):
        '''Start listening. Does nothing if already listening.
        When using the listener in a context start() and stop()
        are called upon __enter__ and __exit__

        This method return immediately.
        Listening and calling callbacks takes place on another thread.
        If you want to block processing and call the callbacks on the main thread,
        then call waitWhileListening() after start.
        '''
        if self.isListening():
            return

        logger.info("Started listening to %s" % ', '.join([str(x) for x in self.__callbacks.keys()]))

        def eventLoop():
            while self.isListening():
                if select.select([self.conn],[],[],2) != ([],[],[]):
                    self.conn.poll()
                    while self.conn.notifies:
                        try:
                            notification = self.conn.notifies.pop(0)
                            logger.debug("Received notification on channel %s payload %s" % (notification.channel, notification.payload))

                            if self.isWaiting():
                                # put notification on Queue
                                # let waiting thread handle the callback
                                self.__queue.put((notification.channel, notification.payload))
                            else:
                                # call callback on this listener thread
                                self._callCallback(notification.channel, notification.payload)
                        except Exception as e:
                            logger.error(str(e))

        self.__thread = Thread(target=eventLoop)
        self.__thread.daemon = True
        self.__listening = True
        self.__thread.start()

    def stop(self):
        '''Stop listening. (Can be restarted)'''
        with self.__lock:
            if not self.__listening:
                return
            self.__listening = False

        self.__thread.join()
        self.__thread = None

        logger.info("Stopped listening")
        self.stopWaiting()

    def __enter__(self):
        '''starts the listener upon contect enter'''
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        '''stops the listener upon contect enter'''
        self.stop()

    def _callCallback(self, channel, payload = None):
        '''call the appropiate callback based on channel'''
        try:
            callback = None
            with self.__lock:
                if channel in self.__callbacks:
                    callback = self.__callbacks[channel]

            if callback:
                if payload:
                    callback(payload)
                else:
                    callback()
        except Exception as e:
            logger.error(str(e))

    def isWaiting(self):
        '''Are we waiting in the waitWhileListening() method?'''
        with self.__lock:
            return self.__waiting

    def stopWaiting(self):
        '''break from the blocking waitWhileListening() method'''
        with self.__lock:
            if self.__waiting:
                self.__waiting = False
                logger.info("Continuing from blocking waitWhileListening")

    def waitWhileListening(self):
        '''
        block calling thread until interrupted or
        until stopWaiting is called from another thread
        meanwhile, handle the callbacks on this thread
        '''
        logger.info("Waiting while listening to %s" % ', '.join([str(x) for x in self.__callbacks.keys()]))

        with self.__lock:
            self.__waiting = True

        while self.isWaiting():
            try:
                notification = self.__queue.get(True, 1)
                channel = notification[0]
                payload = notification[1]

                self._callCallback(channel, payload)
            except KeyboardInterrupt:
                # break
                break
            except Empty:
                pass

        self.stopWaiting()
