#!/usr/bin/python
# $Id$

from lofar.messaging import adaptNameToEnvironment

DEFAULT_MOMQUERY_BUSNAME = adaptNameToEnvironment('lofar.ra.command')
DEFAULT_MOMQUERY_SERVICENAME = 'momqueryservice'

DEFAULT_MOM_BUSNAME = adaptNameToEnvironment('lofar.mom.command')
DEFAULT_MOM_SERVICENAME = ''
