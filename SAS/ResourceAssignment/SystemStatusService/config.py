#!/usr/bin/python
# $Id$

from lofar.messaging import adaptNameToEnvironment

DEFAULT_SSDB_BUSNAME = adaptNameToEnvironment('lofar.ssdb.command')
DEFAULT_SSDB_SERVICENAME = 'SSDBService'
