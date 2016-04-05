#!/usr/bin/env python

# ResourceAssigner.py: ResourceAssigner listens on the lofar ?? bus and calls onTaskSpecified
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
# $Id: raservice.py 1580 2015-09-30 14:18:57Z loose $

"""
TaskSpecifiedListener listens to a bus on which specified tasks get published. It will then try
to assign resources to these tasks.
"""

import qpid.messaging
import logging
from datetime import datetime
import time

from lofar.sas.resourceassignment.rataskspecified.RABusListener import RATaskSpecifiedBusListener
from lofar.messaging.RPC import RPC, RPCException

import lofar.sas.resourceassignment.resourceassignmentservice.rpc as rarpc
from lofar.sas.resourceassignment.rataskspecified.config import DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME
from lofar.sas.resourceassignment.rataskspecified.config import DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT
from lofar.sas.resourceassignment.resourceassigner.assignment import ResourceAssigner

logger = logging.getLogger(__name__)

class SpecifiedTaskListener(RATaskSpecifiedBusListener):
    def __init__(self,
                 busname=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME,
                 subject=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT,
                 broker=None,
                 assigner=None,
                 **kwargs):
        """
        SpecifiedTaskListener listens on the lofar ?? bus and calls onTaskSpecified
        :param busname: valid Qpid address (default: lofar.otdb.status)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
                options=     <dict>    Dictionary of options passed to QPID
                exclusive= <bool>    Create an exclusive binding so no other services can consume duplicate messages (default: False)
                numthreads= <int>    Number of parallel threads processing messages (default: 1)
                verbose=     <bool>    Output extra logging over stdout (default: False)
        """
        super(SpecifiedTaskListener, self).__init__(busname=busname, subject=subject, broker=broker, **kwargs)

        self.assigner = assigner
        if not self.assigner:
            self.assigner = ResourceAssigner()

    def onTaskSpecified(self, otdb_id, specification_tree):
        logger.info('onTaskSpecified: otdb_id=%s' % otdb_id)

        self.assigner.doAssignment(specification_tree)

__all__ = ["SpecifiedTaskListener"]

def main():
    from optparse import OptionParser
    from lofar.messaging import setQpidLogLevel
    from lofar.common.util import waitForInterrupt

    from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as RADB_BUSNAME
    from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as RADB_SERVICENAME
    from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_BUSNAME as RE_BUSNAME
    from lofar.sas.resourceassignment.resourceassignmentestimator.config import DEFAULT_SERVICENAME as RE_SERVICENAME
    from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_BUSNAME
    from lofar.sas.systemstatus.service.config import DEFAULT_SSDB_SERVICENAME

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the resourceassigner service')
    parser.add_option('-q', '--broker', dest='broker', type='string',
                      default=None,
                      help='Address of the qpid broker, default: localhost')
    parser.add_option("--notification_busname", dest="notification_busname", type="string",
                      default=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME,
                      help="Name of the notification bus on which taskspecified messages are published. [default: %default]")
    parser.add_option("--notification_subject", dest="notification_subject", type="string",
                      default=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT,
                      help="Subject of the published taskspecified messages to listen for. [default: %default]")
    parser.add_option("--radb_busname", dest="radb_busname", type="string",
                      default=RADB_BUSNAME,
                      help="Name of the bus on which the radb service listens. [default: %default]")
    parser.add_option("--radb_servicename", dest="radb_servicename", type="string",
                      default=RADB_SERVICENAME,
                      help="Name of the radb service. [default: %default]")
    parser.add_option("--re_busname", dest="re_busname", type="string",
                      default=RE_BUSNAME,
                      help="Name of the bus on which the resource estimator service listens. [default: %default]")
    parser.add_option("--re_servicename", dest="re_servicename", type="string",
                      default=RE_SERVICENAME,
                      help="Name of the resource estimator service. [default: %default]")
    parser.add_option("--ssdb_busname", dest="ssdb_busname", type="string",
                      default=DEFAULT_SSDB_BUSNAME,
                      help="Name of the bus on which the ssdb service listens. [default: %default]")
    parser.add_option("--ssdb_servicename", dest="ssdb_servicename", type="string",
                      default=DEFAULT_SSDB_SERVICENAME,
                      help="Name of the ssdb service. [default: %default]")
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    setQpidLogLevel(logging.INFO)
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    with ResourceAssigner(radb_busname=options.radb_busname,
                          radb_servicename=options.radb_servicename,
                          re_busname=options.re_busname,
                          re_servicename=options.re_servicename,
                          ssdb_busname=options.ssdb_busname,
                          ssdb_servicename=options.ssdb_servicename,
                          broker=options.broker) as assigner:
        with SpecifiedTaskListener(busname=options.notification_busname,
                                   subject=options.notification_subject,
                                   broker=options.broker,
                                   assigner=assigner) as listener:
            waitForInterrupt()

if __name__ == '__main__':
    main()
