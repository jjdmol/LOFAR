#!/usr/bin/env python

# rotspservice.py: RAtoOTDBTaskSpecificationPropagator listens on the lofar ?? bus and calls onTaskScheduled
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: rotspservice.py 1580 2015-09-30 14:18:57Z loose $

"""
TaskScheduledListener listens to a bus on which tasks handled by the ResourceAssigner get published.
It will then try to propagate the changes to OTDB as Scheduled or Conflict.
"""

import qpid.messaging
import logging
from datetime import datetime
import time

from lofar.sas.resourceassignment.RAtoOTDBTaskSpecificationPropagator.RABusListener import RATaskScheduledBusListener
from lofar.messaging.RPC import RPC, RPCException

import lofar.sas.resourceassignment.resourceassignmentservice.rpc as rarpc ## RA DB
from lofar.sas.resourceassignment.resourceassigner.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME
from lofar.sas.resourceassignment.resourceassigner.config import RATASKSCHEDULED_NOTIFICATION_BUSNAME, RATASKSCHEDULED_NOTIFICATIONNAME
from lofar.sas.resourceassignment.RAtoOTDBTaskSpecificationPropagator.translator import RAtoOTDBTranslator

logger = logging.getLogger(__name__)

class RATaskScheduledListener(RATaskScheduledBusListener):
    def __init__(self,
                 busname=RATASKSCHEDULED_NOTIFICATION_BUSNAME,
                 subject=RATASKSCHEDULED_NOTIFICATIONNAME,
                 broker=None,
                 assigner=None,
                 **kwargs):
        """
        RATaskScheduledListener listens on the lofar ?? bus and calls onTaskScheduled
        :param busname: valid Qpid address (default: lofar.ra.notification)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
                options=     <dict>    Dictionary of options passed to QPID
                exclusive= <bool>    Create an exclusive binding so no other services can consume duplicate messages (default: False)
                numthreads= <int>    Number of parallel threads processing messages (default: 1)
                verbose=     <bool>    Output extra logging over stdout (default: False)
        """
        super(RATaskScheduledListener, self).__init__(busname=busname, subject=subject, broker=broker, **kwargs)

        self.translator = translator
        if not self.translator:
            self.translator =  RAtoOTDBTranslator()

    def onTaskScheduled(self, otdbId, momId, modificationTime):
        logger.info('onTaskScheduled: otdbId=%s' % otdbId)

        self.translator.doTranslation(otdbId, momId, 'scheduled')

#    def onTaskConflict(self, otdbId, momId, modificationTime):
#        logger.info('onTaskConflict: otdbId=%s' % otdbId)
#
#        self.translator.doTranslation(otdbId, momId, 'conflict')


__all__ = ["TaskScheduledListener"]

def main():
    from optparse import OptionParser
    from lofar.messaging import setQpidLogLevel
    from lofar.common.util import waitForInterrupt

    from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as RADB_BUSNAME
    from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as RADB_SERVICENAME
    from lofar.sas.resourceassignment.otdbservice.config import DEFAULT_BUSNAME as OTDB_BUSNAME
    from lofar.sas.resourceassignment.otdb.config import DEFAULT_SERVICENAME as OTDB_SERVICENAME

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the resourceassigner service')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_BUSNAME, help="Name of the bus exchange on the qpid broker, default: %s" % DEFAULT_BUSNAME)
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=DEFAULT_SERVICENAME, help="Name for this service, default: %s" % DEFAULT_SERVICENAME)
    parser.add_option("--notification_busname", dest="notification_busname", type="string", default=RATASKSCHEDULED_NOTIFICATION_BUSNAME, help="Name of the notification bus on which taskscheduled messages are published, default: %s" % RATASKSCHEDULED_NOTIFICATION_BUSNAME)
    parser.add_option("--notification_subject", dest="notification_subject", type="string", default=RATASKSCHEDULED_NOTIFICATIONNAME, help="Subject of the published taskscheduled messages to listen for, default: %s" % RATASKSCHEDULED_NOTIFICATIONNAME)
    parser.add_option("--radb_busname", dest="radb_busname", type="string", default=RADB_BUSNAME, help="Name of the bus on which the RADB service listens, default: %s" % RADB_BUSNAME)
    parser.add_option("--radb_servicename", dest="radb_servicename", type="string", default=RADB_SERVICENAME, help="Name of the RADB service, default: %s" % RADB_SERVICENAME)
    parser.add_option("--otdb_busname", dest="otdb_busname", type="string", default=OTDB_BUSNAME, help="Name of the bus on which the OTDB service listens, default: %s" % OTDB_BUSNAME)
    parser.add_option("--otdb_servicename", dest="otdb_servicename", type="string", default=OTDB_SERVICENAME, help="Name of the OTDB service, default: %s" % OTDB_SERVICENAME)
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    setQpidLogLevel(logging.INFO)
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    with RAtoOTDBTranslator(radb_busname=options.radb_busname,
                            radb_servicename=options.radb_servicename,
                            otdb_busname=options.otdb_busname,
                            otdb_servicename=options.otdb_servicename,
                            broker=options.broker) as translator:
        with TaskScheduledListener(busname=options.notification_busname,
                                   subject=options.notification_subject,
                                   broker=options.broker,
                                   translator=translator) as listener:
            waitForInterrupt()

if __name__ == '__main__':
    main()
