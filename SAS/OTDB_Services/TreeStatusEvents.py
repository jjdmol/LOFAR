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
Daemon that watches the OTDB database for status changes of trees and publishes those on the messagebus.
"""

import os
import os.path
import sys, time, pg, datetime
import logging
from lofar.messaging import EventMessage, ToBus

QUERY_EXCEPTIONS = (TypeError, ValueError, MemoryError, pg.ProgrammingError, pg.InternalError)
alive = False

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)

# Define our own exceptions
class FunctionError(Exception):
    "Something when wrong during the execution of the function"
    pass
class DatabaseError(Exception):
    "Connection with the database could not be made"
    pass

# Task Specification Request
def PollForStatusChanges(start_time, otdb_connection):
    """
    Function that asked the database for status changes in the given period

    Input : start_time (datetime) - Oldest time of change to include in the selection.
    The times must be specified in the format YYYY-Mon-DD HH24:MI:SS.US.
    The selection delivers changes the match:  startime <= time_of_change

    Output: (list of tuples) - All status changes between the last polltime and the current time
                               Tuple = ( tree_id, new_state, time_of_change )

    Exceptions:
    ArgumentError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Try to get the specification information
    record_list = []
    try:
        record_list = otdb_connection.query("select treeid,state,modtime,creation from getStateChanges('%s',NULL)" %
                      (start_time.strftime("%F %T.%f"),)).getresult()
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while polling for state changes: %s"% exc_info)
    return record_list

def signal_handler(signum, frame):
    "Signal redirection to stop the daemon in a neat way."
    logger.info("Stopping program")
    global alive
    alive = False


if __name__ == "__main__":
    from optparse import OptionParser
    from lofar.common import dbcredentials
    import signal
    from lofar.sas.otdb.config import DEFAULT_OTDB_NOTIFICATION_BUSNAME as DEFAULT_NOTIFICATION_BUSNAME

    # Check the invocation arguments
    parser = OptionParser("%prog [options]")
    parser.add_option("-B", "--busname", dest="busname", type="string", default=DEFAULT_NOTIFICATION_BUSNAME,
                      help="Busname or queue-name the status changes are published on. [default: %default]")
    parser.add_option_group(dbcredentials.options_group(parser))
    (options, args) = parser.parse_args()

    dbcreds = dbcredentials.parse_options(options)

    if not options.busname:
        print "Missing busname"
        parser.print_help()
        sys.exit(1)

    # Set signalhandler to stop the program in a neat way.
    signal.signal(signal.SIGINT, signal_handler)

    alive = True
    connected = False
    otdb_connection = None
    with ToBus(options.busname) as send_bus:
        while alive:
            while alive and not connected:
                # Connect to the database
                try:
                    otdb_connection = pg.connect(**dbcreds.pg_connect_options())
                    connected = True
                    logger.info("Connected to database %s" % (dbcreds,))

                    # Get list of allowed tree states
                    allowed_states = {}
                    for (state_nr, name) in otdb_connection.query("select id,name from treestate").getresult():
                        allowed_states[state_nr] = name
                except (TypeError, SyntaxError, pg.InternalError), e:
                    connected = False
                    logger.error("Not connected to database %s, retry in 5 seconds: %s" % (dbcreds, e))
                    time.sleep(5)

            # When we are connected we can poll the database
            if connected:
                # Get start_time (= creation time of last retrieved record if any)
                try:
                    treestatuseventfilename = os.path.expanduser('~/.lofar/otdb_treestatusevent_state')
                    with open(treestatuseventfilename, 'r') as f:
                        line = f.readline()
                        if line.rfind('.') > 0:
                            start_time = datetime.datetime.strptime(line, "%Y-%m-%d %H:%M:%S.%f")
                        else:
                            start_time = datetime.datetime.strptime(line, "%Y-%m-%d %H:%M:%S")
                except Exception as e:
                    logger.warning(e)
                    # start scanning from events since 'now'
                    # this timestamp will be stored in the treestatuseventfilename file
                    start_time = datetime.datetime.utcnow()

                    try:
                        logger.info("creating %s" % (treestatuseventfilename,))
                        if not os.path.exists(os.path.dirname(treestatuseventfilename)):
                            os.mkdirs(os.path.dirname(treestatuseventfilename))

                        with open(treestatuseventfilename, 'w') as f:
                            f.write(start_time.strftime("%Y-%m-%d %H:%M:%S"))
                    except Exception as e:
                        logger.error(e)

                try:
                    logger.info("start_time=%s, polling database" % (start_time,))
                    record_list = PollForStatusChanges(start_time, otdb_connection)

                    for (treeid, state, modtime, creation) in record_list:
                        content = { "treeID" : treeid, "state" : allowed_states.get(state, "unknown_state"),
                                    "time_of_change" : modtime }
                        msg = EventMessage(context="otdb.treestatus", content=content)
                        logger.info("sending message treeid %s state %s modtime %s" % (treeid, allowed_states.get(state, "unknown_state"), modtime))
                        send_bus.send(msg)

                        logger.info("new start_time:=%s" % (creation,))

                        try:
                            with open(treestatuseventfilename, 'w') as f:
                                f.write(creation)
                        except Exception as e:
                            logger.error(e)
                except FunctionError, exc_info:
                    logger.error(exc_info)
                except Exception as e:
                    logger.error(e)

                # Redetermine the database status.
                connected = (otdb_connection and otdb_connection.status == 1)

                time.sleep(2)

