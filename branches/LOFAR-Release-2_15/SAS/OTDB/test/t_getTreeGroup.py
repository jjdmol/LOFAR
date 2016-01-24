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
Test the modified getTreeGroup stored procedure in the database.
The tests assume a 'unittest_db' that contains the following trees:

treeid   treetype     LOFAR.ObsSW.Observation.Cluster.ProcessingCluster.clusterName
-----------------------------------------------------------------------------------
1099268  Observation  CEP2
1099270  Observation  CEP4
1099271  Observation  <empty>
1099272  Observation  <no such field>
1099266  Pipeline     CEP2
1099273  Pipeline     CEP4
1099274  Pipeline     <empty>
1099275  Pipeline     <no such field>

When cluster-argument is empty all trees are returned (=backwards compatability)
When cluster argument is e.g. 'CEP2' all CEP2 PIPELINES are returned.
When cluster argument is e.g. '!CEP2' all trees EXCEPT the CEP2 PIPELINES are returned
"""

import sys, pg
import logging

logging.basicConfig(stream=sys.stdout, level=logging.WARNING)
logger = logging.getLogger(__name__)

dbcontent = {
    1099268: "CEP2-Obs", 1099266 : "CEP2-PL",
    1099270: "CEP4-Obs", 1099273 : "CEP4-PL",
    1099271: "empty-Obs",1099274 : "empty-PL",
    1099272: "None-Obs", 1099275 : "None-PL" }

def construct_answer(cluster):
    """
    Implement the same algorithm as the SQL query we call
    """
    if cluster == '':
       return [ (x,) for x in dbcontent.keys() ]
    if cluster[0] == "!":
        return [ (key,) for (key,value) in dbcontent.iteritems() if value != "%s-PL"%cluster[1:] ]
    else:
		return [ (key,) for (key,value) in dbcontent.iteritems() if value == "%s-PL"%cluster ]

# Execute the getTreeGroup query
def getTreeGroup(dbconnection, grouptype, period, cluster):
    """
    Do the database call and sort the result.
    """
    # Try to get the specification information
    return sorted(dbconnection.query("select treeid from getTreeGroup(%d, %d, '%s')" % (grouptype, period, cluster)).getresult())

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "Syntax: %s username hostname database" % sys.argv[0]
        sys.exit(1)

    username = sys.argv[1]
    hostname = sys.argv[2]
    database = sys.argv[3]

    otdb_connection = pg.connect(user=username, host=hostname, dbname=database)

    try:
        success = True
        for cluster in ['', 'CEP2', '!CEP2', 'CEP4', '!CEP4' ]:
            success = success & (construct_answer(cluster) == getTreeGroup(otdb_connection, 0, 0, cluster))
    except Exception as e:
        print e
        success = False
    
    sys.exit(not(success))  # return 0 on success.
