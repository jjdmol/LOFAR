#!/usr/bin/python
import logging
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT, \
                                ISOLATION_LEVEL_READ_COMMITTED
import psycopg2
import time
from src.gsmlogger import get_gsm_logger


class PgConnection(object):
    """
    Connection object for PostgreSQL.
    """
    def __init__(self, **params):
        par = self.map_params(params)
        self.conn = psycopg2.connect(**par)
        #import ipdb; ipdb.set_trace()
        self.conn.set_isolation_level(ISOLATION_LEVEL_READ_COMMITTED)
        self.autocommit = False
        self.log = get_gsm_logger('pgsql', 'sql.log')
        self.log.setLevel(logging.INFO)
        self.profile = False

    @staticmethod
    def is_monet():
        """
        For quick distinction between MonetDB and Postgres.
        """
        return False

    @staticmethod
    def map_params(somedict):
        """
        Map MonetDB connection params to PostgreSQL ones.
        If value is None in the mapper, parameter is removed.
        """
        mapper = {
            'hostname': 'host',
            'username': 'user',
            'database': 'dbname',
            'autocommit': None,
            'port': None
        }

        result = {}
        for key, value in somedict.iteritems():
            if key in mapper:
                if mapper[key] != None:
                    result[mapper[key]] = value
            else:
                result[key] = value
        return result

    def set_autocommit(self, value):
        """
        Change commit level for connection.
        """
        if value:
            self.conn.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        else:
            self.conn.set_isolation_level(ISOLATION_LEVEL_READ_COMMITTED)
        self.autocommit = value

    def commit(self):
        """
        Commit only if it is needed.
        """
        if not self.autocommit:
            self.conn.commit()

    def start(self):
        """
        Start transaction.
        """
        if not self.autocommit:
            self.conn.cursor().execute('BEGIN')

    def execute(self, query):
        """
        Overriding execute method with logging.
        """
        if self.profile:
            start = time.time()
        cur = self.conn.cursor()
        result = cur.execute(query)
        cur.close()
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
                query_set = query_set.strip('; ').split(';')
        cursor = self.conn.cursor()
        for query in query_set:
            cursor.execute(query)
        #We have to be sure that there is anything to fetch.
        if cursor.rowcount > 0 and \
           cursor.statusmessage.split()[0] == 'SELECT' and \
           not quiet:
            return cursor.fetchall()
        else:
            return True

    def exec_return(self, query):
        """
        Run a single query and return the first value from resultset.
        """
        result = []
        try:
            cursor = self.conn.cursor()
            cursor.execute(query)
            result = cursor.fetchone()[0]
            if isinstance(result, long):
                result = int(result)
        except psycopg2.Error, exc:
            self.log.error("Failed on query: %s. Error: %s" % (query, exc))
            raise exc
        finally:
            cursor.close()
        return result

    def get_cursor(self, query):
        """
        Create and return a cursor for a given query.
        """
        cur = self.conn.cursor()
        cur.execute(query)
        return cur

    def cursor(self):
        """
        Create and return a cursor for a given query.
        """
        cur = self.conn.cursor()
        return cur

    def close(self):
        """
        Close the connection.
        """
        self.conn.close()

    def established(self):
        """
        :returns: True if the connection is active.
        """
        if self.conn:
            return not self.conn.closed
        else:
            return False

    def call_procedure(self, procname):
        """
        Proper procedure call (for Monet/Postgres compatibility.)
        """
        cur = self.conn.cursor()
        cur.execute('select %s' % procname)
        cur.close()
