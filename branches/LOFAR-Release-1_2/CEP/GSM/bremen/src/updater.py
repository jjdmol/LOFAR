#!/usr/bin/python
from src.sqllist import get_sql


def _refactor_update(sql):
    """
    Special refactoring for MonetDB update..from imitation.
    """
    sqlupdate, sqlfrom = sql.strip().split('from', 1)
    if sqlfrom.endswith(';'):
        sqlfrom = sqlfrom[:-1]
    sqlupd_list = sqlupdate.split('set')[1].split(',')
    table = sqlupdate.split('set')[0].split()[1]
    update_field = []
    for sqlf in sqlupd_list:
        field, update_stmt = sqlf.split('=')
        update_field.append('%s = (select %s from %s)' % (field,
                                                    update_stmt, sqlfrom))
    result = []
    for field in update_field:
        result.append("""update %s set %s
    where exists (select 1 from %s);""" % (table, field, sqlfrom))
    return result


def run_update(conn, sql_name, *params):
    """
    Run update on a given connection. Refactor it for MonetDB if needed.
    """
    sql = get_sql(sql_name, *params)
    if conn.is_monet():
        conn.execute_set(_refactor_update(sql))
    else:
        conn.execute(sql)
