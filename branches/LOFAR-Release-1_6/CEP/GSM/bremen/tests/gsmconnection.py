#!/usr/bin/python
import os
import unittest
import monetdb
from monetdb.monetdb_exceptions import DatabaseError as MonetDatabaseError
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
        with self.assertRaises((MonetDatabaseError,
                                psycopg2.OperationalError)):
            self.cm.get_connection(database='test_nonexistent',
                                   username='test1',
                                   password='test1')

    def test_bad_sql(self):
        conn = self.cm.get_connection(database='test')
        self.assertTrue(conn.established())
        with self.assertRaises((MonetDatabaseError,
                               psycopg2.DatabaseError)):
            conn.execute('select abracadabra from xxxtable;')
        conn.rollback()
        bad_sql = """update assocxtrsources
   set weight = weight*(select ta.flux_fraction
                   from temp_associations ta
                  where ta.runcat_id = assocxtrsources.runcat_id
                    and not exists (select 1 from runningcatalog  r
                      where r.first_xtrsrc_id = ta.xtrsrc_id
                    )
                    and assocxtrsources.xtrsrc_id <> ta.xtrsrc_id
                    and ta.lr_method = 1
                    and ta.kind = 3),
       lr_method = -4
 where exists (select ta.flux_fraction
                   from temp_associations ta
                  where ta.runcat_id = assocxtrsources.runcat_id
                    and not exists (select 1 from runningcatalog  r
                      where r.first_xtrsrc_id = ta.xtrsrc_id
                    )
                    and assocxtrsources.xtrsrc_id <> ta.xtrsrc_id
                    and ta.lr_method = 1
                    and ta.kind = 3);"""
        if self.is_monet:
            with self.assertRaises(MonetDatabaseError):
                conn.execute(bad_sql)
            self.assertFalse(conn.established())

    def test_store_properties(self):
        self.cm.params['port'] = 12345
        self.cm.save_params('temp_params.data')
        self.cm.params['port'] = 12346
        self.cm.load_params('temp_params.data')
        self.assertEquals(self.cm.params['port'], 12345)
        os.remove('temp_params.data')
