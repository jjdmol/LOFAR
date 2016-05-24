#!/usr/bin/python
# $Id$

from lofar.messaging import adaptNameToEnvironment

DEFAULT_NOTIFICATION_BUSNAME = adaptNameToEnvironment('lofar.ra.notification')
DEFAULT_NOTIFICATION_PREFIX = 'RADB.'
DEFAULT_NOTIFICATION_SUBJECTS=DEFAULT_NOTIFICATION_PREFIX+'*'
