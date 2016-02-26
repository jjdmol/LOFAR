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

TaskGetSpecification    : get the specification(parset) of a task as dict.
TaskSetSpecification    : set the specification(parset) of a task.
KeyUpdateCommand        : function to update the value of multiple (existing) keys.
StatusUpdateCommand     : finction to update the status of a task.
"""

import sys, time, pg
import logging
from lofar.messaging.Service import *
from lofar.common.util import waitForInterrupt

QUERY_EXCEPTIONS = (TypeError, ValueError, MemoryError, pg.ProgrammingError, pg.InternalError)

logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
logger = logging.getLogger(__name__)

# Define our own exceptions
class FunctionError(Exception):
    "Something when wrong during the execution of the function"
    pass
class DatabaseError(Exception):
    "Connection with the database could not be made"
    pass

# Task Get IDs
def TaskGetIDs(input_dict, db_connection, return_tuple=True):
    """
    RPC function that returns the MoMID and OTDBid from the user input, verifies the input in the database.

    Input : dict with either the key OtdbID (integer) or the key MoMID (integer).
            This key is used to search for the specified tree.
            return_tuple (bool) : Tuples can not be send with QPID, but for internal use we prefer them

    Output: (task_found, otdb_id, mom_id) : 
            When task does not exist than otdb_id and mom_id contain the user defined values
            When task exists then otdb_id and mom_id contain the real ids (retrieved from the database)

    Exceptions:
        AttributeError: Input not conform the specs
    """
    # Get task identifier: OtdbId or MoMID
    if not isinstance(input_dict, dict):
        raise AttributeError("Expected a dict as input")

    if 'OtdbID' not in input_dict and 'MoMID' not in input_dict:
        raise AttributeError("Need 'OtdbID' or 'MoMID' key for task retrieval")

    otdb_id = input_dict.get('OtdbID', None)
    mom_id  = input_dict.get('MoMID',  None)

    # Try to get the taskInfo (to find out whether or not the task already exists.
    if otdb_id is not None:
        try:
            (real_otdb_id, real_mom_id) =\
                db_connection.query("select treeid,momid from getTreeInfo({}, False)".format(otdb_id)).getresult()[0]
            return (True, real_otdb_id, real_mom_id) if return_tuple else [True, real_otdb_id, real_mom_id]
        except QUERY_EXCEPTIONS:
            pass

    # Task not found on otdb_id, try mom_id
    if  mom_id is not None: 
        try:
            (real_otdb_id, real_mom_id) =\
                db_connection.query("select treeid,momid from getTreeInfo({}, True)".format(mom_id)).getresult()[0]
            return (True, real_otdb_id, real_mom_id) if return_tuple else [True, real_otdb_id, real_mom_id]
        except QUERY_EXCEPTIONS:
            pass

    # Task not found in any way return input values to the user
    return (False, otdb_id, mom_id) if return_tuple else [False, otdb_id, mom_id]


# Task Get Specification
def TaskGetSpecification(input_dict, db_connection):
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
        raise AttributeError("TaskGetSpecification: Expected a dict as input")
    try:
        tree_id = input_dict['OtdbID']
    except KeyError, info:
        raise AttributeError("TaskGetSpecification: Key %s is missing in the input" % info)

    # Try to get the specification information
    try:
        logger.info("TaskGetSpecification:%s" % input_dict)
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

# Task Set Specification
def TaskSetSpecification(input_dict, db_connection):
    """
    RPC function that (create and) updates the contents of a template or VIC tree.

    Input : dict with either the key OtdbID (integer) or the key MoMID (integer).
            This key is used to search for the specified tree.
            TemplateName      - Optional: Needed when the task doesn't exist and has to be created.
            Updates (dict)    - The key-value pairs that must be updated.

    Implemented workflow (all checks for errors are left out):
    check if there is a tree with the given OtdbID or MoMID
    if not
        instanciate a template from default template 'DefaultTemplateName'
    update the keys in the tree

    Output: OtdbID - integer : OTDB id of the updated tree.
            MoMID  - integer : MoM id of the updated tree.
            Errors - dict    : dict of the keys that could not be updated and th reason why.
                               Empty dict when everything went well.

    Exceptions:
    ...
    AttributeError: There is something wrong with the given input values.
    FunctionError:  An error occurred during the execution of the function.
                    The text of the exception explains what is wrong.
    """
    # Solve ID(s) that the user may have specified and return the validated values.
    (found_task, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # when otdb_id = None task is not in the database
    # if we searched on OtdbID and the task is not found then is it end-of-story
    if not found_task and otdb_id is not None:
        raise FunctionError("Task with OtdbID/MoMID {}/{} does not exist".format(otdb_id, mom_id))

    # if we searched on MomID and the task is not found that we try to create a task(template)
    if not found_task and mom_id is not None:
        selected_template = input_dict.get('TemplateName', None)
        if selected_template is None:
            raise AttributeError("TaskSetSpecification: Need 'TemplateName' key to create a task")
        try:
            template_info = db_connection.query("select treeid,name from getDefaultTemplates()").getresult()
            # template_info is a list with tuples: (treeid,name)
            print "TEMPLATENAMES:", template_info
            all_names = [ name for (_,name) in template_info if name[0] != '#' ]
            if not selected_template in all_names:
                raise AttributeError("DefaultTemplate '{}' not found, available are:{}".format(selected_template, all_names))
            # Yeah, default template exist, now make a copy of it.
            template_ids = [ tasknr for (tasknr,name) in template_info if name == selected_template ]
            if len(template_ids) != 1:
                raise FunctionError("Programming error: matching task_ids for template {} are {}".\
                      format(selected_template, template_ids))
            otdb_id = db_connection.query("select copyTree(1,{})".format(template_ids[0])).getresult()[0][0]
            print "###NEW_TREE=",otdb_id
            # give new tree the mom_id when mom_id was specified by the user.
            if is_mom_id:
                db_connection.query("select setMomInfo(1,{},{},0,'no campaign')".format(treeid, mom_id))
        except QUERY_EXCEPTIONS, exc_info:
            raise FunctionError("Error while create task from template {}: {}".format(selected_template, exc_info))

    # When we are here we always have a task, so do the key updates
    try:
        update_result = KeyUpdateCommand({'OtdbID':otdb_id, 'Updates':input_dict['Updates']}, db_connection, 
                                         always_return_result_dict=True)
    except (AttributeError, FunctionError), exc_info:
        update_result = exc_info

    answer = {}
    answer['OtdbID'] = otdb_id
    answer['MoMID']  = mom_id
    answer['Errors'] = update_result
    return answer


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
def KeyUpdateCommand(input_dict, db_connection, always_return_result_dict=False):
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
    if len(errors) and not always_return_result_dict:
        raise FunctionError(("Not all key were updated:", errors))
    return errors


# Task Prepare For Scheduling
def TaskPrepareForScheduling(input_dict, db_connection):
    """
    RPC function to close the definition fase and make the task schedulable (by converting the template task to an VIC task)

    Input : OtdbID   (integer) - ID of the task to change the status of.
            StartTime (string) - Proposes starttime (optional)
            StopTime  (string) - Proposed endtime (optional)

    Output: OtdbID  (integer)  - ID of 'schedulable' task

    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Solve ID(s) that the user may have specified and return the validated values.
    (found_task, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if not found_task:
        raise FunctionError("Task with OtdbID/MoMID {}/{} does not exist".format(otdb_id, mom_id))

    # get the information of the task
    try: 
        (task_id,task_type,task_state) = db_connection.query("select treeid,type,state from getTreeInfo({},False)"\
                                         .format(otdb_id)).getresult()[0]
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("TaskPrepareForScheduling: {}".format(exc_info))

    # Check type
    # Get list of defined types
    type_names = {}
    type_nrs   = {}
    try:
        for (nr, name) in db_connection.query("select id,name from treetype").getresult():
            type_names[name] = nr
            type_nrs[nr]     = name
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while getting list of task types for tree {}: {}".format(otdb_id, exc_info))

    # Tree may not be of the type 'hardware'
    if task_type == type_names['hardware']:
        raise FunctionError("TaskPrepareForScheduling: Task {} has the wrong type ('hardware')".format(task_id))

    # If task is of the type VItemplate convert it to a VHtree
    if task_type == type_names['VItemplate']:
        try:
            # create executable task
            new_task_id = db_connection.query("select instanciateVHtree(1,{})".format(task_id)).getresult()[0][0]
            # get the characteristics
            (task_id, task_type, task_state) = db_connection.query("select treeid,type,state from getTreeInfo({},False)"\
                                           .format(new_task_id)).getresult()[0]
        except QUERY_EXCEPTIONS, exc_info:
            raise FunctionError("TaskPrepareForScheduling: failed for task {}: {}"\
                                .format(otdb_id, exc_info))

    # Get list of defines tree states
    state_names = {}
    state_nrs   = {}
    try:
        for (nr, name) in db_connection.query("select id,name from treestate").getresult():
            state_names[name] = nr
            state_nrs[nr]     = name
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while getting list of task states for tree {}: {}".format(otdb_id, exc_info))

    # make sure the tree is in the right state
    if task_state != state_names['approved']:
        try:
            db_connection.query("select setTreeState(1,{},{}::INT2,True)".format(task_id, state_names['approved']))
        except QUERY_EXCEPTIONS, exc_info:
            raise FunctionError("Error while setting task {} to 'approved': {}".format(task_id, exc_info))
    # QPID can't return an integer, make a list of it.
    return [task_id]


# Task Delete
def TaskDelete(input_dict, db_connection):
    """
    RPC function to close the definition fase and make the task schedulable (by converting the template task to an VIC task)

    Input : OtdbID   (integer) - ID of the task to change the status of.
    Output: success  (bool)    - result of function

    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Solve ID(s) that the user may have specified and return the validated values.
    (found_task, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if not found_task:
        raise FunctionError("Task with OtdbID/MoMID {}/{} does not exist".format(otdb_id, mom_id))

    # delete the task
    try: 
       db_connection.query("select deleteTree(1,{})".format(otdb_id))
       return 'True'
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("TaskDelete {}: {}".format(otdb_id, exc_info))



class PostgressMessageHandler(MessageHandlerInterface):
    """
    Implements a generic message handlers for services that are tied to a postgres database.
    kwargs must contain the keys:
        database <string>    Name of the database to connect to
        db_user  <string>    Name of the user used for logging into the database
        db_host  <string>    Name of the machine the database server is running
        function <type>      Function to call when a message is received on the message bus.
    """
    def __init__(self, **kwargs):
        super(PostgressMessageHandler, self).__init__()
        self.dbcreds  = kwargs.pop("dbcreds")
        if len(kwargs):
            raise AttributeError("Unknown keys in arguments of 'DatabaseTiedMessageHandler: %s" % kwargs)
        self.connection = None
        self.connected = False

        self.service2MethodMap = {
            "TaskGetSpecification":     self._TaskGetSpecification,
            "TaskSetSpecification":     self._TaskSetSpecification,
            "StatusUpdateCmd":          self._StatusUpdateCommand,
            "KeyUpdateCmd":             self._KeyUpdateCommand,
            "TaskPrepareForScheduling": self._TaskPrepareForScheduling,
            "TaskGetIDs":               self._TaskGetIDs,
            "TaskDelete":               self._TaskDelete
        }

    def prepare_receive(self):
        "Called in main processing loop just before a blocking wait for messages is done."
        "Make sure we are connected with the database."
        self.connected = (self.connection and self.connection.status == 1)
        while not self.connected:
            try:
                self.connection = pg.connect(**self.dbcreds.pg_connect_options())
                self.connected = True
                logger.info("Connected to database %s" % (self.dbcreds,))
            except (TypeError, SyntaxError, pg.InternalError), e:
                self.connected = False
                logger.error("Not connected to database %s, retry in 5 seconds: %s" % (self.dbcreds, e))
                time.sleep(5)

    # The following functions are called from the Service code.
    def _TaskGetSpecification(self, **kwargs):
        logger.info("_TaskGetSpecification({})".format(kwargs))
        return TaskGetSpecification(kwargs, self.connection)

    def _TaskSetSpecification(self, **kwargs):
        logger.info("_TaskSetSpecification({})".format(kwargs))
        return TaskSetSpecification(kwargs, self.connection)

    def _StatusUpdateCommand(self, **kwargs):
        logger.info("_StatusUpdateCommand({})".format(kwargs))
        return StatusUpdateCommand(kwargs, self.connection)

    def _KeyUpdateCommand(self, **kwargs):
        logger.info("_KeyUpdateCommand({})".format(kwargs))
        return KeyUpdateCommand(kwargs, self.connection)

    def _TaskPrepareForScheduling(self, **kwargs):
        logger.info("_TaskPrepareForScheduling({})".format(kwargs))
        return TaskPrepareForScheduling(kwargs, self.connection)

    def _TaskGetIDs(self, **kwargs):
        logger.info("_TaskGetIDs({})".format(kwargs))
        return TaskGetIDs(kwargs, self.connection, return_tuple=False)

    def _TaskDelete(self, **kwargs):
        logger.info("_TaskDelete({})".format(kwargs))
        return TaskDelete(kwargs, self.connection)


if __name__ == "__main__":
    from optparse import OptionParser
    from lofar.common import dbcredentials
    from lofar.common.util import waitForInterrupt
    from lofar.sas.otdb.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME
    from lofar.messaging import setQpidLogLevel

    # Check the invocation arguments
    parser = OptionParser("%prog [options]")
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_BUSNAME,
           help="Busname or queue-name on which RPC commands are received, default: %s" % DEFAULT_BUSNAME)
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=DEFAULT_SERVICENAME,
           help="Name for this service, default: %s" % DEFAULT_SERVICENAME)
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    parser.add_option_group(dbcredentials.options_group(parser))
    (options, args) = parser.parse_args()

    setQpidLogLevel(logging.INFO)
    dbcreds = dbcredentials.parse_options(options)
    print "###dbcreds:", dbcreds

    with Service(options.servicename,
                 PostgressMessageHandler,
                 busname=options.busname,
                 use_service_methods=True,
                 numthreads=1,
                 handler_args={"dbcreds" : dbcreds},
                 verbose=True):
        waitForInterrupt()

    logger.info("Stopped the OTDB services")

