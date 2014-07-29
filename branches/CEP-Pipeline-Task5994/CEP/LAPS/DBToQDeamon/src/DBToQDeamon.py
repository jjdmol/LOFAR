#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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
import os,sys,time,pg
from optparse import OptionParser
import laps.MsgBus


def createParsetFile(treeID, nodeID, fileName):
    """
    Create a parset file with name fileName from tree treeID starting at nodeID.
    """
    parset = otdb.query("select * from exportTree(%s, %s, %s)" % (1, treeID, nodeID)).getresult()
    print "   Creating parset %s" % fileName
    file = open(fileName, 'w');
    file.write(parset[0][0])
    file.close()

if __name__ == '__main__':
    """
    DBToQDeamon checks the LOFAR database every n seconds for new AutoPipeline trees.
    """
    parser = OptionParser("Usage: %prog [options]" )
    parser.add_option("-D", "--database",
                      dest="dbName",
                      type="string",
                      default="",
                      help="Name of OTDB database to use")

    parser.add_option("-H", "--host",
                      dest="dbHost",
                      type="string",
                      default="sasdb",
                      help="Hostname of OTDB database")

    # parse arguments
    (options, args) = parser.parse_args()

    if not options.dbName:
        print "Provide the name of OTDB database to use!"
        print
        parser.print_help()
        sys.exit(0)

    dbName = options.dbName
    dbHost = options.dbHost

    # calling stored procedures only works from the pg module for some reason.
    otdb = pg.connect(user="postgres", host=dbHost, dbname=dbName)

    # connect to messaging system
    msgbus = laps.MsgBus.Bus()

    # Check if a component LOFAR of this version exists
    treeList = otdb.query("select treeID from getTreeGroup(5,60)").dictresult()
    for t in treeList:
        print t['treeid']
        topNodeID = otdb.query("select nodeid from getTopNode(%s)" % t['treeid']).getresult()[0][0]
        parset = otdb.query("select * from exportTree(%s, %s, %s)" % (1, t['treeid'], topNodeID)).getresult()
        ###print parset[0][0]
        msgbus.send(parset[0][0],"Observation%d" %(t['treeid']))

        ### set state to 'queued'
        ### otdb.query("select * from setTreeState(1, %s, 500, false)" % t['treeid'])
    
    otdb.close()
    sys.exit(0)
