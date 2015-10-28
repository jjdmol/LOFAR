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

import os,sys,time,pg, signal
from optparse import OptionParser

QUERY_EXCEPTIONS = (TypeError, ValueError, MemoryError, pg.ProgrammingError, pg.InternalError)
Alive = False

# Define our own exceptions
class FunctionError(Exception):
    "Something when wrong during the execution of the function"
    pass
class DatabaseError(Exception):
    "Connection with the database could not be made"
    pass

# Task Specification Request
def PollForStatusChanges(start_time, end_time, otdb_connection):
    """
    Function that asked the database for status changes in the given period

    Input : start_time (string) - Oldest time of change to include in the selection.
            end_time (string)   - Most recent time of change to include in the selection
    The times must be specified in the format YYYY-Mon-DD HH24:MI:SS.US.
    The selection delivers changes the match:  startime <= time_of_change < end_time

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
        record_list = otdb_connection.query("select treeid,state,modtime,creation from getStateChanges('%s','%s')" % 
                      (start_time, end_time)).getresult()
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while polling for state changes: %s"% exc_info)
    return record_list

def signal_handler(signum, frame):
    "Signal redirection to stop the daemon in a neat way."
    print "Stopping program"
    global Alive
    Alive = False


if __name__ == "__main__":
    """
    Daemon that sets-up a set of servicess for the OTDB database.
    """
    # Check the invocation arguments
    parser = OptionParser("%prog [options]")
    parser.add_option("-D", "--database", dest="dbName", type="string", default="", 
                      help="Name of the database")
    parser.add_option("-H", "--hostname", dest="dbHost", type="string", default="sasdb", 
                      help="Hostname of database server")
    (options, args) = parser.parse_args()

    if not options.dbName:
        print "Missing database name"
        parser.print_help()
        sys.exit(0)

    if not options.dbHost:
        print "Missing database server name"
        parser.print_help()
        sys.exit(0)

    # Set signalhandler to stop the program in a neat way.
    signal.signal(signal.SIGINT, signal_handler)

    Alive = True
    connected = False
    otdb_connection = None
    while Alive:
        while Alive and not connected:
            # Connect to the database
            try:
                otdb_connection = pg.connect(user="postgres", host=options.dbHost, dbname=options.dbName)
                connected = True
            except (TypeError, SyntaxError, pg.InternalError):
                connected = False
                print "DatabaseError: Connection to database could not be made, reconnect attempt in 5 seconds"
                time.sleep(5)
             
        # When we are connected we can poll the database
        if connected:
            # Get start_time (= creation time of last retrieved record if any)
            start_time = ''
            try:
                start_time = open('time_save.txt', 'rb').read()
            except IOError:
                start_time = "2015-01-01 00:00:00.00"
            print "start_time=", start_time

            try:
                record_list = PollForStatusChanges(start_time, "now", otdb_connection)
            except FunctionError, exc_info:
                print exc_info
            else:
                for (treeid, state, modtime, creation) in record_list:
                    print treeid, state, modtime, creation
                    open('time_save.txt', 'wb').write(creation)
                    start_time = creation
                print "==="

            # Redetermine the database status.
            connected = (otdb_connection and otdb_connection.status == 1)

            time.sleep(2)

