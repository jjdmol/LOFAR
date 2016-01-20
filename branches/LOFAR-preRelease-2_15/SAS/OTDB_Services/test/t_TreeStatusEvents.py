#!/usr/bin/env python
#coding: iso-8859-15
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: Backtrace.cc 31468 2015-04-13 23:26:52Z amesfoort $
"""
RPC functions that allow access to (VIC) trees in OTDB.

TaskSpecificationRequest: get the specification(parset) of a tree as dict.
KeyUpdateCommand        : function to update the value of multiple (existing) keys.
StatusUpdateCommand     : finction to update the status of a tree.
"""

import sys, pg
import logging
from optparse import OptionParser
from lofar.messaging import FromBus

logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

if __name__ == "__main__":
    # Check the invocation arguments
    parser = OptionParser("%prog [options]")
    parser.add_option("-D", "--database", dest="dbName", type="string", default="",
                      help="Name of the database")
    parser.add_option("-H", "--hostname", dest="dbHost", type="string", default="sasdb",
                      help="Hostname of database server")
    parser.add_option("-B", "--busname", dest="busname", type="string", default="",
                      help="Busname or queue-name the status changes are published on")
    (options, args) = parser.parse_args()

    if not options.dbName:
        print "Missing database name"
        parser.print_help()
        sys.exit(1)

    if not options.dbHost:
        print "Missing database server name"
        parser.print_help()
        sys.exit(1)

    if not options.busname:
        print "Missing busname"
        parser.print_help()
        sys.exit(1)

    try:
        print "user=postgres, host=", options.dbHost, "dbname=", options.dbName
        otdb_connection = pg.connect(user="postgres", host=options.dbHost, dbname=options.dbName)
    except (TypeError, SyntaxError, pg.InternalError):
        print "DatabaseError: Connection to database could not be made"
        sys.exit(77)

    with FromBus(options.busname) as frombus:
        # First drain the queue
        no_exception = True
        while no_exception:
            try:
                msg = frombus.receive(timeout=1)
                frombus.ack(msg)
            except Exception:
                no_exception = False

        otdb_connection.query("select setTreeState(1, %d, %d::INT2,'%s')" % (1099266, 500, False))
        msg = frombus.receive(timeout=5)	  # TreeStateEVent are send every 2 seconds
        frombus.ack(msg)
        msg.show()
        try:
            ok = (msg.content['treeID'] == 1099266 and msg.content['state'] == 'queued')
        except IndexError:
            ok = False

    sys.exit(not ok)   # 0 = success
