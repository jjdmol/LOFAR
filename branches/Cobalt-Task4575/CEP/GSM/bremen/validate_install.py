#!/usr/bin/python
"""
Script to check if the required modules are installed.
"""


class BColors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'


def _test_import(libname):
    """
    Try importing the library.
    """
    try:
        __import__(libname)
        return True
    except ImportError:
        return False


def getOk(ok=True):
    """
    Get the coloured "OK"/"FAILED" line.
    """
    if ok:
        return '%s OK %s' % (BColors.OKGREEN, BColors.ENDC)
    else:
        return '%s FAILED %s' % (BColors.FAIL, BColors.ENDC)


OK_STR = { True: 'OK', False: 'FAILED' }


def test_import(libname):
    """
    Test if a module libname is available.
    """
    print 'Module %s ... %s' % (libname, getOk(_test_import(libname)))


def test_import_alternative(lib1, lib2):
    """
    Test if either of lib1/lib2 is available.
    """
    b1 = _test_import(lib1)
    b2 = _test_import(lib2)
    print '%s %s / %s %s ... %s' % (lib1, OK_STR[b1], lib2, OK_STR[b2], getOk(b2 or b1))


def print_head(name):
    """
    Print a fancy title.
    """
    print BColors.HEADER, '='*10, name, '='*10, BColors.ENDC


print_head('CRITICAL')
test_import('pysvn')
test_import_alternative('monetdb', 'psycopg2')
test_import('numpy')
test_import('healpy')
test_import_alternative('configobj', 'lofar.parameterset')

print_head('API')
test_import('texttable')

print_head('Tests')
test_import('nose')
test_import('testconfig')
