#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Tool to recreate all tables/procedures in the database.
"""
import argparse
import copy
import re
from os import path
import monetdb.sql as db
import monetdb.monetdb_exceptions as me
import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
import subprocess


class Recreator(object):
    """
    Tool to recreate all tables/procedures in the database.
    """

    # all procedures to be recreated
    PROCEDURES = ['fill_temp_assoc_kind']

    # list of views to be recreated
    VIEWS = ['v_catalog_info']

    # list of tables to be recreated
    TABLES = ['frequencybands', 'datasets', 'runs', 'images',
              'extractedsources', 'assocxtrsources', 'detections',
              'runningcatalog', 'runningcatalog_fluxes',
              'temp_associations', 'image_stats']

    def __init__(self, database="test", use_monet=True):
        self.monet = use_monet
        if use_monet:
            db_port = 50000
            db_autocommit = True
        db_host = "localhost"
        db_dbase = database
        self.database = database
        db_user = "monetdb"
        db_passwd = "monetdb"
        if use_monet:
            self.conn = db.connect(hostname=db_host, database=db_dbase,
                                   username=db_user, password=db_passwd,
                                   port=db_port,
                                   autocommit=db_autocommit)
        else:
            connect = psycopg2.connect(host=db_host, user=db_user,
                                       database=db_dbase)
            connect.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
            self.conn = connect.cursor()

    def get_table_exists(self, tab_name):
        """
        Check if the table exists in the database.
        """
        if self.monet:
            cur = self.conn.cursor()
            cur.execute(
                "select count(*) from sys.tables where name = '%s';"
                        % tab_name)
        else:
            cur = self.conn
            cur.execute(
                "select count(*) from pg_tables where tablename ='%s';"
                        % tab_name)
        data = cur.fetchone()
        return data[0] == 1

    def drop_table(self, tab_name):
        """
        Drop table if it exists.
        """
        if self.get_table_exists(tab_name):
            if self.monet:
                self.conn.execute("drop table %s;" % tab_name)
            else:
                self.conn.execute("drop table %s cascade;" % tab_name)
                self.conn.execute(
                 "drop sequence if exists seq_%s cascade;" % tab_name)
            print 'Table %s dropped' % tab_name

    # For MonetDB-PostgreSQL convertion.
    PG_SUBSTITUTOR = [
    (r'next value for "(.*?)"', r"nextval('\1'::regclass)"),
    #(r'^create sequence (.*?)$', ''),
    (r'as integer', ''),
    (r' double ', ' double precision '),
    (r'current_timestamp\(\)', 'current_timestamp'),
    ]

    def refactor_lines(self, sql_lines):
        """
        Prepare SQL code for MonetDB/PostgreSQL.
        Remove all comments, make necessary substitutions.
        """
        sql_lines = re.sub(r'/\*.*?\*/', '', sql_lines, flags=re.DOTALL)
        sql_lines = re.sub(r'--.*$', '', sql_lines, flags=re.MULTILINE)
        if not self.monet:
            # Has to apply substitutions for PostgreSQL.
            for from_, to_ in self.PG_SUBSTITUTOR:
                sql_lines = re.sub(from_, to_, sql_lines,
                                   flags=re.MULTILINE | re.IGNORECASE)
        return sql_lines

    def create_table(self, tab_name):
        """
        Create a table with a given name.
        """
        self.run_sql_file("sql/tables/create.table.%s.sql" % tab_name)
        print "Table %s recreated" % tab_name

    def create_view(self, view_name):
        """
        Create a view with a given name.
        """
        self.run_sql_file("sql/create.view.%s.sql" % view_name)
        print "View %s recreated" % view_name

    def create_procedure(self, tab_name):
        """
        Create a procedure with a given name.
        Procedure SQL is located in the project files:
        sql/pg/create.procedure.NAME.sql (PostrgeSQL) or
        sql/create.procedure.NAME.sql (MonetDB).
        """
        if self.monet:
            sql_file = open("sql/create.procedure.%s.sql" % tab_name, 'r')
        else:
            sql_file = open("sql/pg/create.procedure.%s.sql" % tab_name, 'r')
        sql_lines = ''.join(sql_file.readlines())
        sql_lines = self.refactor_lines(sql_lines)
        #print sql_lines
        self.conn.execute(sql_lines)
        print "Procedure %s recreated" % tab_name

    def run_sql_file(self, filename):
        """
        Execute SQL from file (with proper substitutions for psql.
        """
        sql_file = open(filename, 'r')
        sql_lines = ''.join(sql_file.readlines())
        sql_lines = self.refactor_lines(sql_lines)
        self.conn.execute(sql_lines)

    def reload_frequencies(self):
        if self.monet:
            self.conn.execute("copy into frequencybands from '%s';" %
                              path.realpath('sql/tables/freq.dat'))
        else:
            sp = subprocess.Popen(['psql', '-U', 'monetdb',
                                   '-d', self.database, '-c',
                                   "copy frequencybands " \
                                   "from stdin delimiter '|'" \
                                   " null 'null';"],
               stdout=subprocess.PIPE,
               stdin=subprocess.PIPE)
            for line in open('sql/tables/freq.dat', 'r').readlines():
                sp.stdin.write(line)
            sp.communicate()
        print 'Frequencies loaded'

    def run(self):
        try:
            for procedure in self.PROCEDURES:
                if self.monet:
                    try:
                        self.conn.execute("drop procedure %s;" %
                                          procedure)
                        print "drop procedure %s;" % procedure
                    except (psycopg2.ProgrammingError,
                            me.OperationalError):
                        pass
            for view in self.VIEWS:
                try:
                    self.conn.execute("drop view %s;" % view)
                except (psycopg2.ProgrammingError, me.OperationalError):
                    pass
                print "drop view %s;" % view

            drop_tables = copy.copy(self.TABLES)
            drop_tables.reverse()
            print '=' * 20
            for table in drop_tables:
                self.drop_table(table)
            print '=' * 20
            for table in self.TABLES:
                self.create_table(table)
            if not self.monet:
                self.run_sql_file('sql/pg/indices.sql')
                print 'Indices recreated'
            print '=' * 20
            for procedure in self.PROCEDURES:
                self.create_procedure(procedure)
            for view in self.VIEWS:
                self.create_view(view)
            self.reload_frequencies()
        except db.Error, e:
            raise e
        self.conn.close()
        return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="""
    ***Database recreator.
    ***Created by A. Mints (2012).
       *WARNING!!!* Clears all data from the database.""",
    formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument('-D', '--database', type=str, default='test',
                        help='Database to recreate')
    parser.add_argument('-M', '--monetdb', action="store_true",
                        default=False,
                        help='Use MonetDB instead of PostgreSQL')
    args = parser.parse_args()
    recr = Recreator(use_monet=args.monetdb, database=args.database)
    recr.run()
