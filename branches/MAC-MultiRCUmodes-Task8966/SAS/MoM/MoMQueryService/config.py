#!/usr/bin/python
# $Id$

from os import stat, path, chmod
import logging

logger = logging.getLogger(__name__)

# make sure config.py is mode 600 to hide passwords
if oct(stat(path.realpath(__file__)).st_mode & 0777) != '0600':
    logger.info('Changing permissions of config.py to 600')
    try:
        chmod(__file__, 0600)
    except Exception as e:
        print 'Error: Could not change permissions on config.py: ' + str(e)
        exit(-1)

# do not commit passwd in svn
# this file should have permissions 600
momreadonly_passwd='daub673(ming'

DEFAULT_BUSNAME = 'lofar.ra.command'
DEFAULT_SERVICENAME = 'momqueryservice'
