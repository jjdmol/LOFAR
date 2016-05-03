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

                           TVP (Template/Vic/Pic)
---------------------------------------------------------------------------------------------------
TaskGetSpecification       TVP : get the specification(parset) of a task as dict.
TaskCreate                 TV- : create a task (if not already existing) and store the specification
TaskSetSpecification       TV- : function to update the value of multiple (existing) keys.
TaskSetStatus              TVP : function to update the status of a task.
TaskPrepareForScheduling   TV- : creates or updates a task that can be scheduled (VIC with state approved)
TaskDelete                 TVP : deletes a tree with all related information.
TaskGetIDs                 TVP : returns the otdb_id/mom_id/task_type of the specified task.
GetDefaultTemplates        --- : Returns a list with default templates
GetStations                --- : Returns a list of the defined stations from the active PIC tree.
SetProject                 --- : Creates or updates the information of a project/campaign.
"""

import sys, time, pg
import logging
from lofar.messaging.Service import *
from lofar.common.util import waitForInterrupt

QUERY_EXCEPTIONS = (TypeError, ValueError, MemoryError, pg.ProgrammingError, pg.InternalError)

# The only assumptions we make are the tree types.
HARDWARE_TREE = 10
TEMPLATE_TREE = 20
VIC_TREE = 30

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
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
    RPC function that returns the MomID and OTDBid from the user input, verifies the input in the database.

    Input : dict with either the key OtdbID (integer) or the key MomID (integer).
            This key is used to search for the specified tree.
            return_tuple (bool) : Tuples can not be send with QPID, but for internal use we prefer them

    Output: (task_type, otdb_id, mom_id) : 
            When task does not exist (task_type = None) than otdb_id and mom_id contain the user defined values
            When task exists then otdb_id and mom_id contain the real ids (retrieved from the database)

    Exceptions:
        AttributeError: Input not conform the specs
    """
    # Get task identifier: OtdbID or MomID
    if not isinstance(input_dict, dict):
        raise AttributeError("Expected a dict as input")

    if 'OtdbID' not in input_dict and 'MomID' not in input_dict:
        raise AttributeError("Need 'OtdbID' or 'MomID' key for task retrieval")

    otdb_id = input_dict.get('OtdbID', None)
    mom_id  = input_dict.get('MomID',  None)

    # Try to get the taskInfo (to find out whether or not the task already exists.
    if otdb_id is not None:
        try:
            (real_otdb_id, real_mom_id,tree_type) =\
                db_connection.query("select treeid,momid,type from getTreeInfo({}, False)".format(otdb_id)).getresult()[0]
            return (tree_type, real_otdb_id, real_mom_id) if return_tuple else [tree_type, real_otdb_id, real_mom_id]
        except QUERY_EXCEPTIONS:
            pass

    # Task not found on otdb_id, try mom_id
    if  mom_id is not None: 
        try:
            (real_otdb_id, real_mom_id,tree_type) =\
                db_connection.query("select treeid,momid,type from getTreeInfo({}, True)".format(mom_id)).getresult()[0]
            return (tree_type, real_otdb_id, real_mom_id) if return_tuple else [tree_type, real_otdb_id, real_mom_id]
        except QUERY_EXCEPTIONS:
            pass

    # Task not found in any way return input values to the user
    return (None, otdb_id, mom_id) if return_tuple else [None, otdb_id, mom_id]


# Task Get Specification
def TaskGetSpecification(input_dict, db_connection):
    """
    RPC function that retrieves the task specification from a tree.

    Input : OtdbID (integer) - ID of the tree to retrieve the specification of
    Output: (dict)           - The 'parset' of the tree

    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Solve ID(s) that the user may have specified and return the validated values.
    (task_type, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if task_type is None:
        raise FunctionError("Task with OtdbID/MomID {}/{} does not exist".format(otdb_id, mom_id))

    # Try to get the specification information
    try:
        logger.info("TaskGetSpecification:%s" % input_dict)
        top_node = db_connection.query("select nodeid from getTopNode('%s')" % otdb_id).getresult()[0][0]
        treeinfo = db_connection.query("select exportTree(1, '%s', '%s')" % (otdb_id, top_node)).getresult()[0][0]
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while requesting specs of tree %d: %s"% (otdb_id, exc_info))
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
    return {'TaskSpecification':answer_dict}


# Task Create
def TaskCreate(input_dict, db_connection):
    """
    RPC function that (create and) updates the contents of a template or VIC tree.

    Input : dict with either the key OtdbID (integer) or the key MomID (integer).
            This key is used to search for the specified tree.
            TemplateName (string) - Optional: Needed when the task doesn't exist and has to be created.
            CampaignName (string) = Optional: Needed when the task doesn't exist and has to be created.
            Specification (dict)  - The key-value pairs that must be updated.

    Implemented workflow (all checks for errors are left out):
    check if there is a tree with the given OtdbID or MomID
    if not
        instanciate a template from default template 'DefaultTemplateName'
        assign the MomID and the campaign info to this task.
    update the keys in the tree

    Output: OtdbID - integer : OTDB id of the updated tree.
            MomID  - integer : MoM id of the updated tree.
            Errors - dict    : dict of the keys that could not be updated and th reason why.
                               Empty dict when everything went well.

    Exceptions:
    ...
    AttributeError: There is something wrong with the given input values.
    FunctionError:  An error occurred during the execution of the function.
                    The text of the exception explains what is wrong.
    """
    # Solve ID(s) that the user may have specified and return the validated values.
    (task_type, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # when otdb_id = None task is not in the database
    # if we searched on OtdbID and the task is not found then is it end-of-story
    if task_type is None and otdb_id is not None:
        raise FunctionError("Task with OtdbID/MomID {}/{} does not exist".format(otdb_id, mom_id))

    # if we searched on MomID and the task is not found that we try to create a task(template)
    if task_type is None and mom_id is not None:
        selected_template = input_dict.get('TemplateName', None)
        if selected_template is None:
            raise AttributeError("TaskCreate: Need 'TemplateName' key to create a task")
        try:
            template_info = db_connection.query("select treeid,name from getDefaultTemplates()").getresult()
            # template_info is a list with tuples: (treeid,name)
            all_names = [ name for (_,name) in template_info if name[0] != '#' ]
            if not selected_template in all_names:
                raise AttributeError("DefaultTemplate '{}' not found, available are:{}".format(selected_template, all_names))
            # Yeah, default template exist, now make a copy of it.
            template_ids = [ tasknr for (tasknr,name) in template_info if name == selected_template ]
            if len(template_ids) != 1:
                raise FunctionError("Programming error: matching task_ids for template {} are {}".\
                      format(selected_template, template_ids))
            otdb_id = db_connection.query("select copyTree(1,{})".format(template_ids[0])).getresult()[0][0]
            # give new tree the mom_id when mom_id was specified by the user.
            campaign_name  = input_dict.get('CampaignName','no campaign')
            if mom_id is None: mom_id = 0
            db_connection.query("select setMomInfo(1,{},{},0,'{}')".format(otdb_id, mom_id, campaign_name))
        except QUERY_EXCEPTIONS, exc_info:
            raise FunctionError("Error while create task from template {}: {}".format(selected_template, exc_info))

    # When we are here we always have a task, so do the key updates
    return TaskSetSpecification({'OtdbID':otdb_id, 'Specification':input_dict['Specification']}, db_connection)

# Task Get State
def TaskGetStatus(otdb_id, db_connection):
    result = db_connection.query("""select treestate.id, treestate.name from otdbtree
                                                   inner join treestate on treestate.id = otdbtree.state
                                                   where otdbtree.treeid = %s""" % (otdb_id,)).getresult()
    if result:
        return {'OtdbID':otdb_id, 'status_id': result[0][0], 'status': result[0][1]}

    return {'OtdbID':otdb_id, 'status_id': -1, 'status': 'unknown'}

# Task Set State
def TaskSetStatus(input_dict, db_connection):
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
    # Solve ID(s) that the user may have specified and return the validated values.
    (task_type, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if task_type is None:
        raise FunctionError("Task with OtdbID/MomID {}/{} does not exist".format(otdb_id, mom_id))

    try:
        new_status   = input_dict['NewStatus']
        update_times = bool(input_dict.get("UpdateTimestamps", True))
        logger.info("TaskSetStatus(%s,%s,%s)" % (otdb_id, new_status, update_times))
    except KeyError, info:
        raise AttributeError("TaskSetStatus: Key %s is missing in the input" % info)

    # Get list of allowed tree states
    allowed_states = {}
    try:
        for (state_nr, name) in db_connection.query("select id,name from treestate").getresult():
            allowed_states[name] = state_nr
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while getting allowed states of tree %d: %s" % (otdb_id, exc_info))

    # Check value of new_status argument
    if not new_status in allowed_states:
        raise FunctionError("The newstatus(=%s) for tree %d must have one of the following values:%s" %
                            (new_status, otdb_id, allowed_states.keys()))

    # Finally try to change the status
    try:
        success = (db_connection.query("select setTreeState(1, %d, %d::INT2,%s)" %
            (otdb_id, allowed_states[new_status], str(update_times))).getresult()[0][0] == 't')
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while setting the status of tree %d: %s" % (otdb_id, exc_info))
    return {'OtdbID':otdb_id, 'MomID':mom_id, 'Success':success}


# TaskSetSpecification
def TaskSetSpecification(input_dict, db_connection):
    """
    RPC function to update the values of a tree.

    Input : OtdbID  (integer) - ID of the tree to change the status of.
            Specification (dict)    - The key-value pairs that must be updated.
    Output: (dict)
            'Errors' (dict)    Refects the problems that occured {'key':'problem'}
                               Field is empty if all fields could be updated.
    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Solve ID(s) that the user may have specified and return the validated values.
    (task_type, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if task_type is None:
        raise FunctionError("Task with OtdbID/MomID {}/{} does not exist".format(otdb_id, mom_id))
    if task_type == HARDWARE_TREE:
        raise FunctionError("OtdbID/MomID {}/{} refers to a hardware tree.".format(otdb_id, mom_id))

    try:
        update_list = input_dict['Specification']
    except KeyError, info:
        raise AttributeError("TaskSetSpecification: Key %s is missing in the input" % info)
    if not isinstance(update_list, dict):
        raise AttributeError("TaskSetSpecification (tree=%d): Field 'Specification' must be of type 'dict'" % otdb_id)
    logger.info("TaskSetSpecification for tree: %d", otdb_id)

    # Finally try to update all keys
    errors = {}
    for (key, value) in update_list.iteritems():
        try:
            if task_type == TEMPLATE_TREE:
                (node_id,name) = db_connection.query("select nodeid,name from getVTitem({},'{}')"\
                               .format(otdb_id, key)).getresult()[0]
                record_list = db_connection.query("select nodeid,instances,limits from getVTitemlist ({},'{}') where nodeid={}"\
                               .format(otdb_id, name, node_id)).getresult()
            else: # VIC_TREE
                record_list = db_connection.query("select nodeid,instances,limits from getVHitemlist ({},'{}')"\
                               .format(otdb_id, key)).getresult()
            if len(record_list) == 0:
                errors[key] = "Not found for tree %d" % otdb_id
                continue
            if len(record_list) > 1:
                errors[key] = "Not a unique key, found %d occurrences for tree %d" % (len(record_list), otdb_id)
                continue
            # When one record was found record_list is a list with a single tuple (nodeid, instances, current_value)
            node_id   = record_list[0][0]
            instances = record_list[0][1]
            # Note: updateVTnode covers both template and VIC trees
            db_connection.query("select updateVTnode(1,%d,%d,%d::INT2,'%s')" % (otdb_id, node_id, instances, value))
            print "%s: %s ==> %s" % (key, record_list[0][2], value)
        except QUERY_EXCEPTIONS, exc:
            errors[key] = str(exc)

    answer = {}
    answer['OtdbID'] = otdb_id
    answer['MomID']  = mom_id
    if len(errors)>0:
        answer['Errors'] = errors
    return answer


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
    (task_type, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if task_type is None:
        raise FunctionError("Task with OtdbID/MomID {}/{} does not exist".format(otdb_id, mom_id))
    if task_type == HARDWARE_TREE:
        raise FunctionError("OtdbID/MomID {}/{} refers to a hardware tree.".format(otdb_id, mom_id))

    # get the information of the task
    try: 
        (task_id,task_type,task_state) = db_connection.query("select treeid,type,state from getTreeInfo({},False)"\
                                         .format(otdb_id)).getresult()[0]
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("TaskPrepareForScheduling: {}".format(exc_info))

    # Get list of defines tree states
    state_names = {}
    state_nrs   = {}
    try:
        for (nr, name) in db_connection.query("select id,name from treestate").getresult():
            state_names[name] = nr
            state_nrs[nr]     = name
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("Error while getting list of task states for tree {}: {}".format(otdb_id, exc_info))

    # If task is of the type VItemplate convert it to a VHtree
    delete_old_task = False
    if task_type == TEMPLATE_TREE:
        try:
            # create executable task
            new_task_id = db_connection.query("select instanciateVHtree(1,{})".format(task_id)).getresult()[0][0]
            # get the characteristics
            (task_id,task_type,task_state) = db_connection.query("select treeid,type,state from getTreeInfo({},False)"\
                                             .format(new_task_id)).getresult()[0]
            delete_old_task = True
        except QUERY_EXCEPTIONS, exc_info:
            raise FunctionError("TaskPrepareForScheduling: failed for task {}: {}".format(otdb_id, exc_info))
        # make sure the tree is in the right state
        if task_state != state_names['approved']:
            try:
                db_connection.query("select setTreeState(1,{},{}::INT2,True)".format(task_id, state_names['approved']))
            except QUERY_EXCEPTIONS, exc_info:
                raise FunctionError("Error while setting task {} to 'approved': {}".format(task_id, exc_info))

    if delete_old_task:
        TaskDelete({'OtdbID':otdb_id}, db_connection)

    #Set the scheduling times if they are specified.
    start_time = input_dict.get("StartTime", "")
    end_time   = input_dict.get("StopTime", "")
    if start_time != "" or end_time != "":
        try:
            db_connection.query("select setSchedule(1,{},'{}','{}')".format(task_id,start_time,end_time))
        except QUERY_EXCEPTIONS, exc_info:
            raise FunctionError("Error while setting schedule-times of task {} to '{}'-'{}': {}"\
                  .format(task_id, start_time, end_time, exc_info))

    return {'OtdbID':task_id, 'MomID':mom_id, 'Success':True}


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
    (task_type, otdb_id, mom_id) = TaskGetIDs(input_dict, db_connection) # throws on missing input

    # if task i not found it is end of story.
    if task_type is None:
        raise FunctionError("Task with OtdbID/MomID {}/{} does not exist".format(otdb_id, mom_id))

    # delete the task
    try: 
        db_connection.query("select deleteTree(1,{})".format(otdb_id))
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("TaskDelete {}: {}".format(otdb_id, exc_info))

    return {'OtdbID':otdb_id, 'MomID':mom_id, 'Success':True}


# Task Get Default Templates
def GetDefaultTemplates(input_dict, db_connection):
    """
    RPC function to retrieve all (active) default templates

    Output: Templates (dict) - Dict containing the characteristics of the default templates

    Exceptions:
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # get the information
    Templates = {}
    try: 
        for (treeid,name,proc_type,proc_subtype,strategy) in db_connection.query("select * from getDefaultTemplates()").getresult():
            if name[0] != '#':
                Templates[name] = { 'OtdbID':treeid, 'processType':proc_type, 'processSubtype':proc_subtype, 'Strategy':strategy}
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("GetDefaulTemplates: {}".format(exc_info))

    return { 'DefaultTemplates': Templates }

# Get Stations
def GetStations(input_dict, db_connection):
    """
    RPC function to retrieve all station names (and their relative location)

    Output: Stations (dict) - Dict containing all the stations

    Exceptions:
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # get the information
    Stations = {}
    try: 
        for fullname in db_connection.query("select * from getStations()").getresult():
            # (LOFAR.PIC.<location>.<name>,)
            level = fullname[0].split('.')
            if len(level) == 4:
                Stations[level[3]] = level[2]
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("GetStations: {}".format(exc_info))

    return { 'Stations': Stations }

# Set Project
def SetProject(input_dict, db_connection):
    """
    RPC function to create or update a project record

    Output: result (bool) - Reflecting the result of the function

    Exceptions:
    AttributeError: There is something wrong with the given input values.
    FunctionError: An error occurred during the execution of the function.
                   The text of the exception explains what is wrong.
    """
    # Check input
    if not isinstance(input_dict, dict):
        raise AttributeError("Expected a dict as input")
    try:
        project_name = input_dict['name']
        title        = input_dict['title']
        pi           = input_dict['pi']
        co_i         = input_dict['co_i']
        contact      = input_dict['contact']
    except KeyError, info:
        raise AttributeError("SetProject: Key %s is missing in the input" % info)
    logger.info("SetProject for project: {}".format(project_name))

    # get the information
    Stations = {}
    try: 
        project_id = db_connection.query("select saveCampaign(0,'{}','{}','{}','{}','{}')".format(project_name, title, pi, co_i, contact)).getresult()[0][0]
    except QUERY_EXCEPTIONS, exc_info:
        raise FunctionError("SetProject: {}".format(exc_info))

    return { "projectID": project_id }


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
            "TaskCreate":               self._TaskCreate,
            "TaskGetStatus":            self._TaskGetStatus,
            "TaskSetStatus":            self._TaskSetStatus,
            "TaskSetSpecification":     self._TaskSetSpecification,
            "TaskPrepareForScheduling": self._TaskPrepareForScheduling,
            "TaskGetIDs":               self._TaskGetIDs,
            "TaskDelete":               self._TaskDelete,
            "GetDefaultTemplates":      self._GetDefaultTemplates,
            "GetStations":              self._GetStations,
            "SetProject":               self._SetProject
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

    def _TaskCreate(self, **kwargs):
        logger.info("_TaskCreate({})".format(kwargs))
        return TaskCreate(kwargs, self.connection)

    def _TaskGetStatus(self, **kwargs):
        logger.info("_TaskGetStatus({})".format(kwargs))
        return TaskGetStatus(kwargs.get('otdb_id'), self.connection)

    def _TaskSetStatus(self, **kwargs):
        logger.info("_TaskSetStatus({})".format(kwargs))
        return TaskSetStatus(kwargs, self.connection)

    def _TaskSetSpecification(self, **kwargs):
        logger.info("_TaskSetSpecification({})".format(kwargs))
        return TaskSetSpecification(kwargs, self.connection)

    def _TaskPrepareForScheduling(self, **kwargs):
        logger.info("_TaskPrepareForScheduling({})".format(kwargs))
        return TaskPrepareForScheduling(kwargs, self.connection)

    def _TaskGetIDs(self, **kwargs):
        logger.info("_TaskGetIDs({})".format(kwargs))
        return TaskGetIDs(kwargs, self.connection, return_tuple=False)

    def _TaskDelete(self, **kwargs):
        logger.info("_TaskDelete({})".format(kwargs))
        return TaskDelete(kwargs, self.connection)

    def _GetDefaultTemplates(self, **kwargs):
        logger.info("_GetDefaultTemplates()")
        return GetDefaultTemplates(kwargs, self.connection)

    def _GetStations(self, **kwargs):
        logger.info("_GetStations()")
        return GetStations(kwargs, self.connection)

    def _SetProject(self, **kwargs):
        logger.info("_SetProject({})".format(kwargs))
        return SetProject(kwargs, self.connection)


if __name__ == "__main__":
    from optparse import OptionParser
    from lofar.common import dbcredentials
    from lofar.common.util import waitForInterrupt
    from lofar.sas.otdb.config import DEFAULT_OTDB_SERVICE_BUSNAME, DEFAULT_OTDB_SERVICENAME
    from lofar.messaging import setQpidLogLevel

    # Check the invocation arguments
    parser = OptionParser("%prog [options]")
    parser.add_option("-B", "--busname", dest="busname", type="string",
                      default=DEFAULT_OTDB_SERVICE_BUSNAME,
                      help="Busname or queue-name on which RPC commands are received. [default: %default")
    parser.add_option("-S", "--servicename", dest="servicename", type="string",
                      default=DEFAULT_OTDB_SERVICENAME,
                      help="Name for this service. [default: %default")
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    # Add options of dbcredentials: --database, --host, ...
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

