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
# $Id$
"""

"""

from lofar.messaging.RPC import RPC

def getParset(obsid, status, otdb_busname="lofar.otdb.command"):
    with RPC("TaskSpecification", busname=otdb_busname, timeout=10) as parset_rpc:
        result, _ = parset_rpc(OtdbID=obsid)

    return result

if __name__ == "__main__":
    from optparse import OptionParser
    import logging
    import sys

    logging.basicConfig(stream=sys.stdout, level=logging.INFO)
    logger = logging.getLogger(__name__)

    # Check the invocation arguments
    parser = OptionParser("%prog -o obsid [options]")
    parser.add_option("-B", "--busname", dest="busname", type="string", default="lofar.otdb.command",
                      help="Busname on which OTDB commands are sent")
    parser.add_option("-o", "--obsid", dest="obsid", type="int", default=0,
                      help="Observation/tree ID to get parset of")
    (options, args) = parser.parse_args()

    if not options.busname or not options.obsid:
        parser.print_help()
        sys.exit(1)

    parset = getParset(options.obsid, otdb_busname=options.busname)

    for k,v in parset.iteritems():
        print "%s = %s" % (k,v)

