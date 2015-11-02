#!/usr/bin/python
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

logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

def do_rpc(rpc_instance, arg_dict):
    try:
        (data,status) = (rpc_instance)(arg_dict)
        if (status != "OK"):
            raise Exception("Status returned is %s" % status)
        print type(data)
        for (key,value) in data.iteritems():
            print "%s ==> %s" % (key, value)
    except OverflowError as e:
        pass
    print "======"

if __name__ == "__main__":
    busname = sys.argv[1] if len(sys.argv) > 1 else "simpletest"

#    with RPC("TaskSpecification", ForwardExceptions=True, busname=busname, timeout=2) as task_spec_request:
#        do_rpc(task_spec_request, {'OtdbID':63370})
#        do_rpc(task_spec_request, {'OtdbID':146300})
#        do_rpc(task_spec_request, {'OtdbID':82111})

#    print StatusUpdateCmd({'OtdbID':146300, 'NewStatus':'finished', 'UpdateTimestamps':True})

    with RPC("KeyUpdateCmd", ForwardExceptions=True, busname=busname, timeout=2) as key_update:
        print key_update({'OtdbID':63370, 
                          'Updates':{'LOFAR.ObsSW.Observation.ObservationControl.OnlineControl._hostname':'CCU099ABC'}})

   

