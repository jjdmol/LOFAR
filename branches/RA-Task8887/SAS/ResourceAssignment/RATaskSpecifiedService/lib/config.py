#!/usr/bin/python
# $Id$

from lofar.messaging import adaptNameToEnvironment

DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME=adaptNameToEnvironment('lofar.ra.notification')
DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT='RA.TaskSpecified'
