#!/usr/bin/python
# $Id$

#DEFAULT_BUSNAME = 'lofar.ra.command'
#DEFAULT_SERVICENAME = 'RAtoORTBTaskSpecificationPropagationService'

try:
    from lofar.sas.resourceassignment.resourceassigner.config import RA_NOTIFICATION_BUSNAME
    from lofar.sas.resourceassignment.resourceassigner.config import RA_NOTIFICATION_SUBJECTS
except ImportError:
    RA_NOTIFICATION_BUSNAME='lofar.ra.notification'
    RA_NOTIFICATION_SUBJECTS='RA.*'
