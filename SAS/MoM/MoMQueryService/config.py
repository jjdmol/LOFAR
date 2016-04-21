#!/usr/bin/python
# $Id$

from lofar.messaging import adaptNameToEnvironment

DEFAULT_BUSNAME = adaptNameToEnvironment('lofar.ra.command')
DEFAULT_SERVICENAME = 'momqueryservice'
