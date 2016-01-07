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
Daemon that sets-up a set of servicess for the OTDB database.

RPC functions that allow access to (VIC) trees in OTDB.

TaskSpecificationRequest: get the specification(parset) of a tree as dict.
KeyUpdateCommand        : function to update the value of multiple (existing) keys.
StatusUpdateCommand     : finction to update the status of a tree.
"""

import sys, time, pg
import logging
from lofar.messaging.Service import *

QUERY_EXCEPTIONS = (TypeError, ValueError, MemoryError, pg.ProgrammingError, pg.InternalError)

logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

# Define our own exceptions
class FunctionError(Exception):
    "Something when wrong during the execution of the function"
    pass
class DatabaseError(Exception):
    "Connection with the database could not be made"
    pass

# Task Specification Request
def TaskSpecificationRequest(input_dict, db_connection):
    """
    RPC function that retrieves the task specification from a tree.

    Input : OtdbID (integer) - ID of the tree to retrieve the specifications of
    Output: (dict)           - The 'parset' of the tree

    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Check the input
    if not isinstance(input_dict, dict):
        raise AttributeError("TaskSpecificationRequest: Expected a dict as input")
    try:
        tree_id = input_dict['OtdbID']
    except KeyError, info:
        raise AttributeError("TaskSpecificationRequest: Key %s is missing in the input" % info)

    # Try to get the specification information
    try:
        logger.info("TaskSpecificationRequest:%s" % input_dict)
        top_node = db_connection.query("select nodeid from getTopNode('%s')" % tree_id).getresult()[0][0]
        treeinfo = db_connection.query("select exportTree(1, '%s', '%s')" % (tree_id, top_node)).getresult()[0][0]
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while requesting specs of tree %d: %s"% (tree_id, exc_info))
    # When the query was succesfull 'treeinfo' is now a string that contains many 'key = value' lines seperated
    # with newlines. To make it more usable for the user we convert that into a dict...

    # Note: a PIC tree is a list of keys while a Template tree and a VIC tree is a list of key-values.
    #       Since we don't know what kind of tree was requested we assume a Template/VIC tree (most likely)
    #       but if this ends in exceptions we fall back to a PIC tree.
    answer_dict = {}
    answer_list = []
    for line in treeinfo.split('\n'): # make seperate lines of it.
        try:
            # assume a 'key = value' line
            (key, value) = line.split("=", 1)
            answer_dict[key] = value
        except ValueError:
            # oops, no '=' on the line, must be a PIC tree that was queried: make a list iso a dict
            answer_list.append(line)
    if len(answer_list) > 1:		# there is always one empty line, ignore that one...
        answer_dict["tree"] = answer_list
    return answer_dict

# Status Update Command
def StatusUpdateCommand(input_dict, db_connection):
    """
    RPC function to update the status of a tree.

    Input : OtdbID    (integer) - ID of the tree to change the status of.
            NewStatus (string)  - The new status of the tree. The following values are allowed:
              described, prepared, approved, on_hold, conflict, prescheduled, scheduled, queued,
              active, completing, finished, aborted, error, obsolete
            UpdateTimestamps (boolean) - Optional parameter to also update the timestamp of the metadata of the
              tree when the status of the tree is changed into 'active', 'finished' or 'aborted'. Resp. starttime
              or endtime. Default this option is ON.
    Output: (boolean) - Reflects the successful update of the status.

    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function. 
                   The text of the exception explains what is wrong.
    """
    # Check input
    if not isinstance(input_dict, dict):
        raise AttributeError("StatusUpdateCommand: Expected a dict as input")
    try:
        tree_id      = input_dict['OtdbID']
        new_status   = input_dict['NewStatus']
        update_times = True
        if input_dict.has_key("UpdateTimestamps"):
            update_times = bool(input_dict["UpdateTimestamps"])
        logger.info("StatusUpdateCommand(%s,%s,%s)" % (tree_id, new_status, update_times))
    except KeyError, info:
        raise AttributeError("StatusUpdateCommand: Key %s is missing in the input" % info)

    # Get list of allowed tree states
    allowed_states = {}
    try:
        for (state_nr, name) in db_connection.query("select id,name from treestate").getresult():
            allowed_states[name] = state_nr
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while getting allowed states of tree %d: %s" % (tree_id, exc_info))

    # Check value of new_status argument
    if not new_status in allowed_states:
        raise FunctionError("The newstatus(=%s) for tree %d must have one of the following values:%s" %
                            (new_status, tree_id, allowed_states.keys()))

    # Finally try to change the status
    try:
        success = (db_connection.query("select setTreeState(1, %d, %d::INT2,%s)" %
            (tree_id, allowed_states[new_status], str(update_times))).getresult()[0][0] == 't')
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while setting the status of tree %d: %s" % (tree_id, exc_info))
    return str(success)


# Key Update Command
def KeyUpdateCommand(input_dict, db_connection):
    """
    RPC function to update the values of a tree.

    Input : OtdbID  (integer) - ID of the tree to change the status of.
            Updates (dict)    - The key-value pairs that must be updated.
    Output: (dict)
            'Errors' (dict)    Refects the problems that occured {'key':'problem'}
                               Field is empty if all fields could be updated.
    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Check input
    if not isinstance(input_dict, dict):
        raise AttributeError("Expected a dict as input")
    try:
        tree_id     = input_dict['OtdbID']
        update_list = input_dict['Updates']
    except KeyError, info:
        raise AttributeError("KeyUpdateCommand: Key %s is missing in the input" % info)
    if not isinstance(tree_id, int):
        raise AttributeError("KeyUpdateCommand (tree=%d): Field 'OtdbID' must be of type 'integer'" % tree_id)
    if not isinstance(update_list, dict):
        raise AttributeError("KeyUpdateCommand (tree=%d): Field 'Updates' must be of type 'dict'" % tree_id)
    logger.info("KeyUpdateCommand for tree: %d", tree_id)

    # Finally try to update all keys
    errors = {}
    for (key, value) in update_list.iteritems():
        try:
            record_list = (db_connection.query("select nodeid,instances,limits from getvhitemlist (%d, '%s')" %
                           (tree_id, key))).getresult()
            if len(record_list) == 0:
                errors[key] = "Not found for tree %d" % tree_id
                continue
            if len(record_list) > 1:
                errors[key] = "Not a unique key, found %d occurrences for tree %d" % (len(record_list), tree_id)
                continue
            # When one record was found record_list is a list with a single tuple (nodeid, instances, current_value)
            node_id   = record_list[0][0]
            instances = record_list[0][1]
            db_connection.query("select updateVTnode(1,%d,%d,%d::INT2,'%s')" % (tree_id, node_id, instances, value))
            print "%s: %s ==> %s" % (key, record_list[0][2], value)
        except QUERY_EXCEPTIONS, exc:
            errors[key] = str(exc)
    if len(errors):
        raise FunctionError(("Not all key were updated:", errors))
    return errors

class PostgressMessageHandlerInterface(MessageHandlerInterface):
    """
    Implements a generic message handlers for services that are tied to a postgres database.
    kwargs must contain the keys:
        database <string>    Name of the database to connect to
        db_user  <string>    Name of the user used for logging into the database
        db_host  <string>    Name of the machine the database server is running
        function <type>      Function to call when a message is received on the message bus.
    """
    def __init__(self, **kwargs):
        super(PostgressMessageHandlerInterface, self).__init__()
        self.dbcreds  = kwargs.pop("dbcreds")
        if len(kwargs):
            raise AttributeError("Unknown keys in arguments of 'DatabaseTiedMessageHandler: %s" % kwargs)
        self.connection = None
        self.connected = False

    def prepare_receive(self):
        "Called in main processing loop just before a blocking wait for messages is done."
        "Make sure we are connected with the database."
        self.connected = (self.connection and self.connection.status == 1)
        while not self.connected:
            try:
                self.connection = pg.connect(user=self.dbcreds["user"], passwd=self.dbcreds["password"], host=self.dbcreds["host"], port=self.dbcreds["port"] or -1, dbname=self.dbcreds["database"])
                self.connected = True
                logger.info("Connected to database %s on host %s" % (self.dbcreds["database"], self.dbcreds["host"]))
            except (TypeError, SyntaxError, pg.InternalError), e:
                self.connected = False
                logger.error("Not connected to database %s on host %s (anymore), retry in 5 seconds: %s"
                             % (self.dbcreds["database"], self.dbcreds["host"], e))
                time.sleep(5)

class PostgressTaskSpecificationRequest(PostgressMessageHandlerInterface):
    """
    Embedding of the TaskSpecificationRequest function in the postgress service class.
    """
    def __init__(self, **kwargs):
        super(PostgressTaskSpecificationRequest, self).__init__(**kwargs)

    def handle_message(self, msg):
        " Connect to the right function"
        return TaskSpecificationRequest(msg, self.connection)


class PostgressStatusUpdateCommand(PostgressMessageHandlerInterface):
    """
    Embedding of the TaskSpecificationRequest function in the postgress service class.
    """
    def __init__(self, **kwargs):
        super(PostgressStatusUpdateCommand, self).__init__(**kwargs)

    def handle_message(self, msg):
        " Connect to the right function"
        return StatusUpdateCommand(msg, self.connection)


class PostgressKeyUpdateCommand(PostgressMessageHandlerInterface):
    """
    Embedding of the TaskSpecificationRequest function in the postgress service class.
    """
    def __init__(self, **kwargs):
        super(PostgressKeyUpdateCommand, self).__init__(**kwargs)

    def handle_message(self, msg):
        " Connect to the right function"
        return KeyUpdateCommand(msg, self.connection)


if __name__ == "__main__":
    from optparse import OptionParser
    from lofar.common import dbcredentials
    from lofar.common.util import waitForInterrupt

    # Check the invocation arguments
    parser = OptionParser("%prog [options]")
    parser.add_option("-B", "--busname", dest="busname", type="string", default="testbus",
                      help="Busname or queue-name on which RPC commands are received")
    parser.add_option_group(dbcredentials.options_group(parser))
    (options, args) = parser.parse_args()

    dbcreds = dbcredentials.parse_options(options)

    if not options.busname:
        print "Missing busname"
        parser.print_help()
        sys.exit(1)

    serv1 = Service("TaskSpecification", PostgressTaskSpecificationRequest,
                    busname=options.busname, numthreads=1,
                    handler_args = {"dbcreds" : dbcreds})
    serv2 = Service("StatusUpdateCmd",   PostgressStatusUpdateCommand,
                    busname=options.busname, numthreads=1,
                    handler_args = {"dbcreds" : dbcreds})
    serv3 = Service("KeyUpdateCmd",      PostgressKeyUpdateCommand,
                    busname=options.busname, numthreads=1,
                    handler_args = {"dbcreds" : dbcreds})

    with serv1, serv2, serv3:
        logger.info("Started the OTDB services")
        waitForInterrupt()

    logger.info("Stopped the OTDB services")

