#!/usr/bin/python
# $Id$

from lofar.messaging import adaptNameToEnvironment

DEFAULT_OTDB_SERVICE_BUSNAME = adaptNameToEnvironment('lofar.otdb.command')
DEFAULT_OTDB_SERVICENAME = 'OTDBService'

DEFAULT_OTDB_NOTIFICATION_BUSNAME= adaptNameToEnvironment('lofar.otdb.notification')
DEFAULT_OTDB_NOTIFICATION_SUBJECT='TaskStatus'
