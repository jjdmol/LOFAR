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

def do_rpc(rpc_instance, arg_dict):
#    try:
    (data, status) = (rpc_instance)(**arg_dict)
    if status != "OK":
        raise Exception("Status returned is %s" % status)
    for key in sorted(data):
        print "%s ==> %s" % (key, data[key])
#    except OverflowError as e:
#        pass
    print "======"

if __name__ == "__main__":
    busname = sys.argv[1] if len(sys.argv) > 1 else "simpletest"

    with RPC("TaskSpecification", ForwardExceptions=True, busname=busname, timeout=10) as task_spec_request:
        do_rpc(task_spec_request, {'OtdbID':1099269})	# PIC
        do_rpc(task_spec_request, {'OtdbID':1099238})	# Template
        do_rpc(task_spec_request, {'OtdbID':1099266})	# VIC

    with RPC("StatusUpdateCmd", ForwardExceptions=True, busname=busname, timeout=5) as status_update_command:
        # PIC
        (data, status) = status_update_command(OtdbID=1099269, NewStatus='finished', UpdateTimestamps=True)
        print status, data
        # Template
        (data, status) = status_update_command(OtdbID=1099238, NewStatus='finished', UpdateTimestamps=True)
        print status, data
        # VIC
        (data, status) = status_update_command(OtdbID=1099266, NewStatus='finished', UpdateTimestamps=True)
        print status, data

        # Nonexisting tree
        try:
            (data, status) = status_update_command(OtdbID=10, NewStatus='finished', UpdateTimestamps=True)
        except RPCException as e:
            print "Caught expected exception on invalid treeID in status update"

        # VIC tree: invalid status
        try:
            (data, status) = status_update_command(OtdbID=1099266, NewStatus='what_happend', UpdateTimestamps=True)
        except RPCException as e:
            print "Caught expected exception on invalid status in status update"


    with RPC("KeyUpdateCmd", ForwardExceptions=True, busname=busname, timeout=5) as key_update:
        # VIC tree: valid
        (data, status) = key_update(OtdbID=1099266,
                          Updates={'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.pythonHost':'NameOfTestHost'})
        print status, data

        # Template tree: not supported yet
        try:
            (data, status) = key_update(OtdbID=1099238,
                          Updates={'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.pythonHost':'NameOfTestHost'})
        except RPCException as e:
            print "Caught expected exception on invalid treetype in key update"

        # PIC tree: not supported yet
        try:
            (data, status) = key_update(OtdbID=1099269, Updates={'LOFAR.PIC.Core.CS001.status_state':'50'})
        except RPCException as e:
            print "Caught expected exception on invalid treetype (PIC) in key update"

        # Non exsisting tree
        try:
            (data, status) = key_update(OtdbID=10,
                          Updates={'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.pythonHost':'NameOfTestHost'})
        except RPCException as e:
            print "Caught expected exception on invalid treeID in key update"

        # VIC tree: wrong key
        try:
            (data, status) = key_update(OtdbID=1099266,
                          Updates={'LOFAR.ObsSW.Observation.ObservationControl.PythonControl.NoSuchKey':'NameOfTestHost'})
        except RPCException as e:
            print "Caught expected exception on invalid key in key update"

