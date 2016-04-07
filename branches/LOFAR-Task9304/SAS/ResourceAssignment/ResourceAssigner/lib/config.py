#!/usr/bin/python
# $Id$

DEFAULT_BUSNAME = 'lofar.ra.command'
DEFAULT_SERVICENAME = 'RAService'

try:
    from lofar.sas.resourceassignment.rataskspecified.config import DEFAULT_NOTIFICATION_BUSNAME as RATASKSPECIFIED_NOTIFICATION_BUSNAME
    from lofar.sas.resourceassignment.rataskspecified.config import RATASKSPECIFIED_NOTIFICATIONNAME as RATASKSPECIFIED_NOTIFICATIONNAME
except ImportError:
    RATASKSPECIFIED_NOTIFICATION_BUSNAME = 'lofar.ra.notification'
    RATASKSPECIFIED_NOTIFICATIONNAME = 'OTDB.TaskSpecified'
