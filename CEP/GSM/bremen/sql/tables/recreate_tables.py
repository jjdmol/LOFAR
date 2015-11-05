#!/usr/bin/env python
# -*- coding: utf-8 -*-
import copy
import re
import sys
import monetdb.sql as db
import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT

# TODO: Comment!


class Recreator(object):

    PROCEDURES = ['fill_temp_assoc_kind']
    TABLES = ['datasets', 'images', 'extractedsources',
              'assocxtrsources', 'detections',
              'runningcatalog', 'runningcatalog_fluxes',
              'temp_associations']

    def __init__(self, database="test", use_monet=True):
        self.monet = use_monet
        if use_monet:
            db_port = 50000
            db_autocommit = True
        db_host = "localhost"
        db_dbase = database
        db_user = "monetdb"
        db_passwd = "monetdb"
        if use_monet:
            self.conn = db.connect(hostname=db_host, database=db_dbase,
                                   username=db_user, password=db_passwd,
                                   port=db_port, autocommit=db_autocommit)
        else:
            connect = psycopg2.connect(host=db_host, user=db_user,
                                       dbname=db_dbase)
            connect.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
            self.conn = connect.cursor()

    def drop_table(self, tab_name):
        if self.monet:
            cur = self.conn.cursor()
            cur.execute("select count(*) from sys.tables where name = '%s';"
                        % tab_name)
        else:
            cur = self.conn
            cur.execute("select count(*) from pg_tables where tablename ='%s';"
                        % tab_name)
        data = cur.fetchone()
        if data[0] == 1:
            if self.monet:
                cur.execute("drop table %s;" % tab_name)
            else:
                cur.execute("drop table %s cascade;" % tab_name)
            print 'Table %s dropped' % tab_name

    # For MonetDB-PostgreSQL convertion.
    PG_SUBSTITUTOR = [
    (r'next value for "(.*?)"', r"nextval('\1'::regclass)"),
    (r'^create sequence .*?$', ''),
    (r'as integer', ''),
    (r' double ', ' double precision '),
    (r'current_timestamp\(\)', 'current_timestamp'),
    ]

    def refactor_lines(self, sql_lines):
        sql_lines = re.sub(r'/\*.*?\*/', '', sql_lines, flags=re.DOTALL)
        sql_lines = re.sub(r'--.*$', '', sql_lines, flags=re.MULTILINE)
        if not self.monet:
            for from_, to_ in self.PG_SUBSTITUTOR:
                sql_lines = re.sub(from_, to_, sql_lines,
                                   flags=re.MULTILINE | re.IGNORECASE)
        print sql_lines
        return sql_lines

    def create_table(self, tab_name):
        sql_file = open("create.table.%s.sql" % tab_name, 'r')
        sql_lines = ''.join(sql_file.readlines())
        sql_lines = self.refactor_lines(sql_lines)
        self.conn.execute(sql_lines)
        print "Table %s recreated" % tab_name

    def create_procedure(self, tab_name):
        if self.monet:
            sql_file = open("../create.procedure.%s.sql" % tab_name, 'r')
        else:
            sql_file = open("../pg/create.procedure.%s.sql" % tab_name, 'r')
        sql_lines = ''.join(sql_file.readlines())
        sql_lines = self.refactor_lines(sql_lines)
        #print sql_lines
        self.conn.execute(sql_lines)
        print "Procedure %s recreated" % tab_name

    def run(self):
        try:
            for procedure in self.PROCEDURES:
                if self.monet:
                    self.conn.execute("drop procedure %s;" % procedure)
                    print "drop procedure %s;" % procedure

            drop_tables = copy.copy(self.TABLES)
            drop_tables.reverse()
            print '=' * 20
            for table in drop_tables:
                self.drop_table(table)
            print '=' * 20
            for table in self.TABLES:
                self.create_table(table)
            print '=' * 20
            for procedure in self.PROCEDURES:
                self.create_procedure(procedure)
        except db.Error, e:
            raise e
        self.conn.close()
        return 0


if __name__ == '__main__':
    print sys.argv[0]
    if len(sys.argv) > 1:
        recr = Recreator(use_monet=(sys.argv[0] != './recreate_tables_pg.py'),
                         database=sys.argv[1])
    else:
        recr = Recreator(use_monet=(sys.argv[0] != './recreate_tables_pg.py'))
    recr.run()
