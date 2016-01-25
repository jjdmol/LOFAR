#!/usr/bin/python
# $Id$

from os import stat, path
# make sure config.py is mode 600 to hide passwords
if oct(stat(path.realpath(__file__)).st_mode & 0777) != '0600':
    print 'Please change permissions of config.py to 600'
    exit(-1)

# do not commit passwd in svn
# this file should have permissions 600
radb_password=''

