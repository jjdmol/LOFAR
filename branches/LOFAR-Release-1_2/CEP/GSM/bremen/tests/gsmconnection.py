#!/usr/bin/python
import os
import unittest
import monetdb
import monetdb.sql as db
import psycopg2
from src.gsmconnectionmanager import GSMConnectionManager
from tests.switchable import SwitchableTest

class ConnectionTest(SwitchableTest):

    def test_default_connection(self):
        conn = self.cm.get_connection()
        self.assertTrue(conn.established())

    def test_other_connection(self):
        conn = self.cm.get_connection(database='test')
        self.assertTrue(conn.established())

    def test_fail_connection(self):
        with self.assertRaises((monetdb.monetdb_exceptions.DatabaseError,
                                psycopg2.OperationalError)):
            self.cm.get_connection(database='test_nonexistent',
                                   username='test1',
                                   password='test1')

    def test_store_properties(self):
        self.cm.params['port'] = 12345
        self.cm.save_params('temp_params.data')
        self.cm.params['port'] = 12346
        self.cm.load_params('temp_params.data')
        self.assertEquals(self.cm.params['port'], 12345)
        os.remove('temp_params.data')
