#!/usr/bin/python
# $Id$

DEFAULT_BUSNAME = 'lofar.ra.command'
DEFAULT_SERVICENAME = 'RAService'

try:
    from lofar.sas.resourceassignment.resourceassigner.config import DEFAULT_NOTIFICATION_BUSNAME as RATASKSCHEDULED_NOTIFICATION_BUSNAME
    from lofar.sas.resourceassignment.resourceassigner.config import RATASKSCHEDULED_NOTIFICATIONNAME as RATASKSCHEDULED_NOTIFICATIONNAME
except ImportError:
    RATASKSCHEDULED_NOTIFICATION_BUSNAME = 'lofar.ra.notification'
    RATASKSCHEDULED_NOTIFICATIONNAME = 'RA.TaskScheduled'
#    RATASKCONFLICT_NOTIFICATIONNAME = 'RA.TaskConflict'
