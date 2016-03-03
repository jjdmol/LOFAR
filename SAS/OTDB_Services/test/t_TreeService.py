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

import sys
import logging
from lofar.messaging.RPC import *

logging.basicConfig(stream=sys.stdout, level=logging.WARNING)
logger = logging.getLogger(__name__)

def do_rpc_catch_exception(exc_text, rpc_instance, arg_dict):
    try:
        print "** Executing {}({})...".format(rpc_instance.ServiceName,arg_dict)
        (data, status) = (rpc_instance)(**arg_dict)
        raise Exception("Expected an exception {}, didn't get any".format(exc_text))
    except Exception:
        print "Caught expected exception {}".format(exc_text)
    print "======"

def do_rpc(rpc_instance, arg_dict):
    print "** Executing {}({})...".format(rpc_instance.ServiceName,arg_dict)
    (data, status) = (rpc_instance)(**arg_dict)
    if status != "OK":
        raise Exception("Status returned is {}".format(status))
    if isinstance(data, dict):
        for key in sorted(data):
            print "%s ==> %s" % (key, data[key])
    else:
        print "data =", data
    print "======"
    return data

if __name__ == "__main__":
    busname = sys.argv[1] if len(sys.argv) > 1 else "simpletest"

    with RPC("OTDBService.TaskGetIDs", ForwardExceptions=True, busname="lofar.otdb.specification", timeout=10) as otdbRPC:
        # Existing: otdb_id:1099268, mom_id:353713
        do_rpc                    (otdbRPC, {'OtdbID': 1099268, 'MomID': 353713 })
        do_rpc                    (otdbRPC, {'OtdbID': 1099268, 'MomID': 5 })
        do_rpc                    (otdbRPC, {'OtdbID': 1099268, 'MomID': None })
        do_rpc                    (otdbRPC, {'OtdbID': 5, 'MomID': 353713 })
        do_rpc_catch_exception('', otdbRPC, {'OtdbID': 5, 'MomID': 5 })
        do_rpc_catch_exception('', otdbRPC, {'OtdbID': 5, 'MomID': None })
        do_rpc                    (otdbRPC, {'OtdbID': None, 'MomID': 353713 })
        do_rpc_catch_exception('', otdbRPC, {'OtdbID': None, 'MomID': 5 })
        do_rpc_catch_exception('', otdbRPC, {'OtdbID': None, 'MomID': None })

    with RPC("OTDBService.GetDefaultTemplates", ForwardExceptions=True, busname=busname, timeout=10) as otdbRPC:
        do_rpc(otdbRPC,{})

    with RPC("OTDBService.SetProject", ForwardExceptions=True, busname=busname, timeout=10) as otdbRPC:
        do_rpc(otdbRPC,{'name':"Taka Tuka Land", "title":"Adventure movie", "pi":"Pippi", "co_i":"Mr.Nelson", "contact":"Witje"})

    with RPC("OTDBService.TaskCreate", ForwardExceptions=True, busname=busname, timeout=10) as task_create:
        do_rpc(task_create, {'OtdbID':1099268, 'TemplateName':'BeamObservation', 'Specifications': {'state':'finished'}})
        do_rpc(task_create, {'MomID':353713,   'TemplateName':'BeamObservation', 'Specifications': {'state':'finished'}})
        data = do_rpc(task_create, {'MomID':12345, 'TemplateName':'BeamObservation', 'Specifications': {'state':'finished'}})
        new_tree1 = data['MomID']
        data = do_rpc(task_create, {'MomID':54321, 'TemplateName':'BeamObservation', 'Specifications': {'state':'finished'}})
        new_tree2= data['MomID']

    with RPC("OTDBService.TaskPrepareForScheduling", ForwardExceptions=True, busname=busname, timeout=10) as otdbRPC:
        do_rpc(otdbRPC, {'MomID':new_tree1})   # template
        do_rpc(otdbRPC, {'MomID':new_tree1})   # now a VIC tree
        do_rpc(otdbRPC, {'MomID':new_tree1, 'StartTime':'2016-03-01 12:00:00', 'StopTime':'2016-03-01 12:34:56'})
        do_rpc_catch_exception("on invalid stoptime", otdbRPC, 
                               {'MomID':new_tree1, 'StartTime':'2016-03-01 12:00:00', 'StopTime':'2016'})

    with RPC("OTDBService.TaskDelete", ForwardExceptions=True, busname=busname, timeout=10) as otdbRPC:
        do_rpc(otdbRPC, {'MomID':new_tree2})

    with RPC("OTDBService.TaskGetSpecification", ForwardExceptions=True, busname=busname, timeout=10) as otdbRPC:
#       do_rpc(otdbRPC, {'OtdbID':1099269})  # PIC
        do_rpc(otdbRPC, {'OtdbID':1099238})	  # Template
        do_rpc(otdbRPC, {'OtdbID':1099266})	  # VIC
        do_rpc_catch_exception('on non-existing treeID', otdbRPC, {'OtdbID':5}) # Non existing

    with RPC("OTDBService.TaskSetState", ForwardExceptions=True, busname=busname, timeout=5) as status_update_command:
        # PIC
        do_rpc(status_update_command, {'OtdbID':1099269, 'NewStatus':'finished', 'UpdateTimestamps':True})
        # Template
        do_rpc(status_update_command, {'OtdbID':1099238, 'NewStatus':'finished', 'UpdateTimestamps':True})
        # VIC
        do_rpc(status_update_command, {'OtdbID':1099266, 'NewStatus':'finished', 'UpdateTimestamps':True})

        # Nonexisting tree
        do_rpc_catch_exception('on invalid treeID', 
                               status_update_command, {'OtdbID':10, 'NewStatus':'finished', 'UpdateTimestamps':True})

        # VIC tree: invalid status
        do_rpc_catch_exception('on invalid status',
                               status_update_command, {'OtdbID':1099266, 'NewStatus':'what_happend', 'UpdateTimestamps':True})
        # Set PIC back to active...
        do_rpc(status_update_command, {'OtdbID':1099269, 'NewStatus':'active', 'UpdateTimestamps':True})


    with RPC("OTDBService.GetStations", ForwardExceptions=True, busname=busname, timeout=10) as otdbRPC:
        do_rpc(otdbRPC,{})

    with RPC("OTDBService.TaskSetSpecifications", ForwardExceptions=True, busname=busname, timeout=5) as key_update:
        # VIC tree: valid
        do_rpc(key_update, {'OtdbID':1099266,
               'Specifications':{'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.pythonHost':'NameOfTestHost'}})
        # Template tree: not supported yet
        do_rpc(key_update, {'OtdbID':1099238,
               'Specifications':{'LOFAR.ObsSW.Observation.Scheduler.priority':'0.1'}})
        # PIC tree: not supported yet
        do_rpc_catch_exception('on invalid treetype (PIC)', key_update, 
               {'OtdbID':1099269, 'Specifications':{'LOFAR.PIC.Core.CS001.status_state':'50'}})
        # Non exsisting tree
        do_rpc_catch_exception('on invalid treeID', key_update, {'OtdbID':10,
               'Specifications':{'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.pythonHost':'NameOfTestHost'}})
        # VIC tree: wrong key
        do_rpc_catch_exception('on invalid key', key_update, {'OtdbID':1099266,
               'Specifications':{'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.NoSuchKey':'NameOfTestHost'}})

