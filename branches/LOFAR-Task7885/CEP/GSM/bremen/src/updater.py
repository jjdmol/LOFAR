#!/usr/bin/python
from src.sqllist import get_sql

_UPDATER_EXTRAS = {
    'runningcatalog': ['runcatid'],
    'runningcatalog_fluxes': ['runcat_id', 'band', 'stokes'],
}


def _refactor_update(sql):
    """
    Special refactoring for MonetDB update..from imitation.
    """
    def _get_extra_conditions(tabname):
        return ' '.join(map(lambda x: 'and {0}.{1} = x.{1}'.format(tabname, x),
                            _UPDATER_EXTRAS[tabname]))
    sqlupdate, sqlfrom = sql.strip().split('from', 1)
    table, sqlupd_list = sqlupdate.split('set')
    sqlupd_list = sqlupd_list.split(',')
    table = table.split()[1]
    if sqlfrom.endswith(';'):
        sqlfrom = sqlfrom[:-1]
    sqlfrom_split = sqlfrom.split('where', 1)
    if len(sqlfrom_split) > 1:
        [sqlfrom2, sqlwhere] = sqlfrom_split
        sqlwhere = 'where %s' % sqlwhere
    else:
        sqlfrom2 = sqlfrom
        sqlwhere = ''
    for field in _UPDATER_EXTRAS[table]:
        sqlwhere = sqlwhere.replace('%s.%s' % (table, field), 'x.%s' % field)

    update_field = []
    for sqlf in sqlupd_list:
        field, update_stmt = sqlf.split('=')
        update_field.append('%s = (select %s from %s x, %s %s %s)' % (
                                field, update_stmt.replace(table, 'x'),
                                table, sqlfrom2, sqlwhere,
                                _get_extra_conditions(table)))
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
