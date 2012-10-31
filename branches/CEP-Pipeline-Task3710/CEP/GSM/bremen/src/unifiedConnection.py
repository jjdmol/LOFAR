#!/usr/bin/python
import time
import monetdb.sql as monetdb
import psycopg2
import logging
from src.gsmlogger import get_gsm_logger
from monetdb.monetdb_exceptions import OperationalError
from monetdb.mapi import STATE_READY


class UnifiedConnection(object):
    """
    """
    def __init__(self, **params):
        """
        """
        self.conn = None
        self.lastcount = 0
        self.in_transaction = False
        self.profile = False
        self.log = get_gsm_logger('sql', 'sql.log')
        self.log.setLevel(logging.DEBUG)

    @staticmethod
    def is_monet():
        """
        For quick distinction between MonetDB and Postgres.
        """
        return True

    def close(self):
        """
        Closing connection.
        """
        self.conn.close()

    def _start_transaction(self):
        """
        Custom implementation of "Start transaction".
        """
        self.log.debug('START TRANSACTION;')
        self.conn.execute('START TRANSACTION;')

    def start(self):
        """
        Begin transaction.
        """
        if not self.in_transaction:
            self._start_transaction()
        self.in_transaction = True

    def rollback(self):
        if self.in_transaction:
            self.log.debug('ROLLBACK;')
            self.conn.execute('ROLLBACK;')

    def commit(self):
        """
        Commit only if it is needed.
        """
        if self.profile:
            start = time.time()
        try:
            if self.in_transaction:
                self.log.debug('COMMIT;')
                self.conn.commit()
            self.in_transaction = False
        except OperationalError as oerr:
            self.log.error(oerr)
            raise oerr
        if self.profile:
            self.log.debug('Time: %.3f SQL: COMMIT' % (time.time() - start))

    def _execute_with_cursor(self, query, cursor):
        """
        Run the SQL statement and return it's result.
        Log (with profiling information, if required).
        """
        if self.profile:
            start = time.time()
        if query.strip()[-1:] != ';':
            query = query + ';'
        try:
            self.start()
            self.log.debug(query.replace('\n', ' '))
            result = cursor.execute(query)
        except Exception as oerr:
            self.log.error(query.replace('\n', ' '))
            self.log.error(oerr)
            raise oerr
        if self.profile:
            self.log.debug('Time: %.3f SQL: %s' % (time.time() - start,
                                                   query.replace('\n', ' ')))
        else:
            self.log.debug(query.replace('\n', ' '))
        return result

    def execute(self, query):
        """
        Overriding execute method with logging.
        """
        cur = self.conn.cursor()
        result = self._execute_with_cursor(query, cur)
        self.lastcount = result
        cur.close()
        return result

    def _get_lastcount(self, cursor):
        """
        Get a number of rows in the last SELECT.
        """
        return self.lastcount

    def execute_set(self, query_set, quiet=True):
        """
        Execute several SQL statements and return the last result.
        """
        if not isinstance(query_set, list):
            if not isinstance(query_set, str):
                raise ValueError("Got %s instead of list of SQLs" %
                                 str(query_set))
            else:
                query_set = query_set.strip('; ').split(';')
        cursor = self.conn.cursor()
        self.lastcount = 0
        for query in query_set:
            self._execute_with_cursor(query, cursor)
        if self._get_lastcount(cursor) > 0 and not quiet:
            return cursor.fetchall()
        else:
            return True

    def exec_return(self, query, single_column=True):
        """
        Run a single query and return the first value from resultset.
        """
        result = []
        try:
            cursor = self.conn.cursor()
            self._execute_with_cursor(query, cursor)
            if single_column:
                result = cursor.fetchone()[0]
                if isinstance(result, long):
                    result = int(result)
            else:
                result = cursor.fetchone()
        except (psycopg2.Error, monetdb.Error), exc:
            self.log.error("Failed on query: %s. Error: %s" % (query, exc))
            raise exc
        except NoneType, exc:
            self.log.error("Failed on query: %s. No data returned" % query)
            raise exc
        finally:
            cursor.close()
        return result

    def established(self):
        """
        :returns: True if the connection is active.
        """
        if self.conn.mapi and self.conn.mapi.state == STATE_READY:
            return True
        else:
            return False

    def call_procedure(self, procname):
        """
        Proper procedure call (for Monet/Postgres compatibility.)
        """
        self.execute('call %s' % procname)

    def cursor(self):
        """
        Simple alias for cursor().
        """
        return self.conn.cursor()

    def get_cursor(self, query):
        """
        Create and return a cursor for a given query.
        """
        cur = self.conn.cursor()
        self._execute_with_cursor(query, cur)
        return cur
