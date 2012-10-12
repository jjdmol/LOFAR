#!/usr/bin/python
"""
A set of tools to get queries from sql-files.
SQL-file has to contain separators of the form:
    --#query_name
Then query_name can be passed to get_sql as the parameter.
%s or {..} syntax is supported for substitutions.
Parameters for substitution are passed to get_sql.
"""
from os import path
import re
from src.queries import *

SQL_LIST = {}

GLOBALS = {}


def _expand_value(value):
    """
    Replace all occurences of $$function()$$ by result of the function call.
    """
    def _expand_formula(matchvalues):
        """
        Expand $$..$$ by calculating value in $s.
        """
        return str(eval(matchvalues.group(0)[2:-2]))
    return re.sub(r'\$\$(.*?)\$\$', _expand_formula, value, count=0)


def _load_from_sql_list(filename):
    """
    Load sql-commands from file.
    Command name is prefixed by --#
    """
    sqls = open(filename, 'r')
    hashkey = None
    hashvalue = ''
    for line in iter(sqls.readline, ''):
        if (line.startswith('--#')):
            if hashkey:
                SQL_LIST[hashkey] = _expand_value(hashvalue)
                hashvalue = ''
            hashkey = line[3:].strip()
        elif (not line.startswith('--')) and line:
            if line.find('--') > 0:  # Drop comments
                line = line[:line.index('--')]
            hashvalue = '%s %s' % (hashvalue, line.strip())
    if hashkey:
        SQL_LIST[hashkey] = _expand_value(hashvalue)
    sqls.close()


def _substitute_globals(sql):
    def _substitute_global(matchvalue):
        if matchvalue.group(0)[1:-1] in GLOBALS:
            return str(GLOBALS[matchvalue.group(0)[1:-1]])
        else:
            return ''
    return re.sub(r'\[(.?)\]', _substitute_global, sql, count=0)


def get_sql(name, *params):
    """
    Returns an sql from the list by it's name with parameter substitution.
    """
    if not name in SQL_LIST:
        raise IndexError('Name %s not in sqllist.sql' % name)
    if (SQL_LIST[name].find('%') >= 0):
        return_sql = SQL_LIST[name] % (params)
    else:
        return_sql = SQL_LIST[name].format(*params)
    return _substitute_globals(return_sql)


for sqlfile in ['sqllist.sql',
                'sqllist_api.sql',
                'sqllist_associate.sql',
                'sqllist_join.sql',
                'sqllist_new.sql',
                'sqllist_update.sql',
                'sqllist_deduct.sql',
                'sqllist_group.sql']:
    _load_from_sql_list(path.dirname(path.abspath(__file__)) + '/%s' % sqlfile)
