#!/usr/bin/python
from exceptions import ValueError
import time
import monetdb.sql as db
import logging
from src.gsmlogger import get_gsm_logger
from monetdb.monetdb_exceptions import DatabaseError
"""
Database connection with logging.
Overrides MonetDB connection object.
"""


class MonetLoggedConnection(db.connections.Connection):
    """
    Connection with logging.
    Overrides MonetDB connection object.
    """
    def __init__(self, **params):
        super(MonetLoggedConnection, self).__init__(**params)
        self.profile = False
        self.log = get_gsm_logger('sql', 'sql.log')
        self.log.setLevel(logging.DEBUG)

    def execute(self, query):
        """
        Overriding execute method with logging.
        """
        if self.profile:
            start = time.time()
        self.log.debug(query.replace('\n', ' '))
        try:
            result = super(MonetLoggedConnection, self).execute(query)
        except DatabaseError as oerr:
            self.log.error(oerr)
            raise oerr
        if self.profile:
            self.log.debug('Time spent: %s' % (time.time() - start))
        return result

    def execute_set(self, query_set):
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
        for query in query_set:
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
