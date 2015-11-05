#!/usr/bin/python
"""
Database connection with logging.
Overrides MonetDB connection object.
"""
import time
import monetdb.sql as db
import logging
from src.gsmlogger import get_gsm_logger
from monetdb.monetdb_exceptions import DatabaseError


class MonetLoggedConnection(db.connections.Connection):
    """
    Connection with logging.
    Overrides MonetDB connection object.
    """
    def __init__(self, **params):
        super(MonetLoggedConnection, self).__init__(**params)
        self.profile = False
        self.log = get_gsm_logger('sql', 'sql.log')
        self.log.setLevel(logging.INFO)

    @staticmethod
    def is_monet():
        """
        For quick distinction between MonetDB and Postgres.
        """
        return True

    def start(self):
        """
        Begin transaction.
        """
        self.execute('START TRANSACTION;')

    def execute(self, query):
        """
        Overriding execute method with logging.
        """
        if self.profile:
            start = time.time()
        if query.strip()[-1:] != ';':
            query = query + ';'
        try:
            result = super(MonetLoggedConnection, self).execute(query)
        except DatabaseError as oerr:
            self.log.error(oerr)
            raise oerr
        if self.profile:
            self.log.debug('Time: %.3f SQL: %s' % (time.time() - start,
                                                 query.replace('\n', ' ')))
        else:
            self.log.debug(query.replace('\n', ' '))
        return result

    def execute_set(self, query_set, quiet=True):
        """
        Execute several SQL statements and return the last result.
        """
        if not isinstance(query_set, list):
            if not isinstance(query_set, str):
                raise ValueError("Got %s instead of list of SQLs" %
                                 str(query_set))
            else:
                query_set = query_set.split(';')
        cursor = self.cursor()
        lastcount = 0
        for query in query_set:
            if quiet:
                cursor.execute(query)
            else:
                lastcount = cursor.execute(query)
        if lastcount > 0:
            return cursor.fetchall()
        else:
            return True

    def exec_return(self, query):
        """
        Run a single query and return the first value from resultset.
        """
        result = []
        try:
            cursor = self.cursor()
            cursor.execute(query)
            result = cursor.fetchone()[0]
        except db.Error, exc:
            self.log.error("Failed on query: %s. Error: %s" % (query, exc))
            raise exc
        finally:
            cursor.close()
        return result

    def get_cursor(self, query):
        """
        Create and return a cursor for a given query.
        """
        cur = self.cursor()
        cur.execute(query)
        return cur

    def established(self):
        """
        :returns: True if the connection is active.
        """
        if self.mapi:
            return True
        else:
            return False

    def call_procedure(self, procname):
        """
        Proper procedure call (for Monet/Postgres compatibility.)
        """
        self.execute('call %s' % procname)
