#!/usr/bin/python
from psycopg2.extensions import ISOLATION_LEVEL_READ_COMMITTED
import psycopg2
from src.unifiedConnection import UnifiedConnection


class PgConnection(UnifiedConnection):
    """
    Connection object for PostgreSQL.
    """
    def __init__(self, **params):
        super(PgConnection, self).__init__()
        par = self.map_params(params)
        self.conn = psycopg2.connect(**par)
        self.conn.set_isolation_level(ISOLATION_LEVEL_READ_COMMITTED)

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
            #'database': 'dbname',
            'autocommit': None,
            'port': None
        }

        result = {}
        for key, value in somedict.iteritems():
            if key in mapper:
                if mapper[key] is not None:
                    result[mapper[key]] = value
            else:
                result[key] = value
        return result

    def _start_transaction(self):
        """
        Start transaction.
        """
        try:
            self.log.debug('BEGIN')
            self.conn.cursor().execute('BEGIN')
        except psycopg2.InternalError:
            self.log.debug('ROLLBACK')
            self.conn.rollback()
            self.log.debug('BEGIN')
            self.conn.cursor().execute('BEGIN')

    def rollback(self):
        """
        Rollback transaction.
        """
        self.log.debug('ROLLBACK')
        self.conn.rollback()

    def _get_lastcount(self, cursor):
        """
        Get rowcount of the last executed statement.
        """
        if cursor.statusmessage.split()[0] == 'SELECT':
            return cursor.rowcount
        else:
            return 0

    def call_procedure(self, procname):
        """
        Proper procedure call (for Monet/Postgres compatibility.)
        """
        cur = self.conn.cursor()
        self._execute_with_cursor('select %s' % procname, cur)
        cur.close()

    def established(self):
        """
        :returns: True if the connection is active.
        """
        if self.conn:
            return not self.conn.closed
        else:
            return False

