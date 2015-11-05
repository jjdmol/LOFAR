#!/usr/bin/python
"""
Script to check if the required modules are installed.
"""
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def _testImport(libname, comment=''):
    try:
        __import__(libname)
        return True
    except ImportError:
        return False

def getOk(ok=True):
    if ok:
        return '%s OK %s' % (bcolors.OKGREEN, bcolors.ENDC)
    else:
        return '%s FAILED %s' % (bcolors.FAIL, bcolors.ENDC)

OK_STR = { True: 'OK', False: 'FAILED' }

def testImport(libname):
    print 'Module %s ... %s' % (libname, getOk(_testImport(libname)))

def testImportAlternative(lib1, lib2):
    b1 = _testImport(lib1)
    b2 = _testImport(lib2)
    print '%s %s / %s %s ... %s' % (lib1, OK_STR[b1], lib2, OK_STR[b2], getOk(b2 or b1))




print bcolors.HEADER, '='*10, 'CRITICAL', '='*10, bcolors.ENDC
testImport('pysvn')
testImportAlternative('monetdb', 'psycopg2')
testImport('numpy')
testImport('healpy')
testImportAlternative('configobj', 'lofar.parameterset')

print bcolors.HEADER, '='*10, 'API', '='*10, bcolors.ENDC
testImport('texttable')

print bcolors.HEADER, '='*10, 'Tests', '='*10, bcolors.ENDC
testImport('nose')
testImport('testconfig')
