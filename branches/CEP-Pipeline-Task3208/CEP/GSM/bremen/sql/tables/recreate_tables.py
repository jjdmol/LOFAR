#!/usr/bin/env python
# -*- coding: utf-8 -*-
import copy
import re
import monetdb
import monetdb.sql as db

def drop_table(conn, tab_name):
    cur = conn.cursor();
    cur.execute("select count(*) from sys.tables where name = '%s';" % tab_name)
    data = cur.fetchone()
    if data[0] == 1:
        cur.execute("drop table %s;" % tab_name)
        print 'Table %s dropped' % tab_name
    cur.close()

def create_table(conn, tab_name):
    sql_file = open("create.table.%s.sql" % tab_name, 'r')
    sql_lines = ''.join(sql_file.readlines())
    sql_lines = re.sub(r'/\*.*?\*/','', sql_lines, flags=re.DOTALL)
    sql_lines = re.sub(r'--.*$','', sql_lines, flags=re.MULTILINE)
    conn.execute(sql_lines)
    print "Table %s recreated" % tab_name

def main():
    db_host = "localhost"
    db_dbase = "test"
    db_user = "monetdb"
    db_passwd = "monetdb"
    db_port = 50000
    db_autocommit = True

    TABLES = ['frequencybands', 'datasets', 'images',
              'assocxtrsources', 'detections',
              'runningcatalog', 'runningcatalog_fluxes', 'temprunningcatalog',
              'temp_associations']

    try:
        conn = db.connect(hostname=db_host, database=db_dbase, username=db_user, password=db_passwd, port=db_port, autocommit = db_autocommit)
        drop_tables = copy.copy(TABLES)
        drop_tables.reverse()
        print '='*20
        for table in drop_tables:
            drop_table(conn, table)
        print '='*20
        for table in TABLES:
            create_table(conn, table)
    except db.Error, e:
        raise
    return 0

if __name__ == '__main__':
    main()

