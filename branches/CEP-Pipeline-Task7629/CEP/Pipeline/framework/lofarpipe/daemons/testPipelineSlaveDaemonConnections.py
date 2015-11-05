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
import sys
import os
import socket
import ConfigParser


import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message   
import lofar.messagebus.CQCommon as CQCommon
import lofar.messagebus.CQExceptions as CQExceptions

       


if __name__ == "__main__":
    # Read the config file, fail of not supplied
    if len(sys.argv) != 2:
        print "Usage: pipelineSCQDaemon config.cfg"
        exit(-1)

    config_path = sys.argv[1]
    if not os.path.isfile(config_path):
        print "supplied config file {0} does not exist".format(config_path)
        sys.exit(-1)
    config = ConfigParser.ConfigParser()
    config.read(config_path)

    # create or get parameters 
    hostname = socket.gethostname()
    broker_raw = hostname

    busname = config.get(               "DEFAULT", "busname")
    port = config.get("DEFAULT", "broker_port")
    broker = "{0}:{1}".format(broker_raw, port)
    
    (master_host, slave_hosts_list) = CQCommon.get_master_and_slave_list_from_config(
                                                      config)

    print "Connecting to Bus: {0}".format(busname)
    toBus = msgbus.ToBus(busname, broker = broker)
    print "Connected"

    returnSubject = "testPipelineSlaveDaemonConnections"
    
    print "Connecting to Bus: {0}".format(busname)
    namedFromBus = msgbus.FromBus(busname + "/" + returnSubject,
                                 broker = broker)
    print "Connected"
    
    slaveSubjectTemplate = config.get( "slave_cqdaemon",
                             "command_queue_topic_template")
    


    silent_nodes, dict_of_nodes = CQCommon.sendEchoToSlaveListReturnResponce(
                toBus, namedFromBus, returnSubject,
                slaveSubjectTemplate, slave_hosts_list,port, 5)

    print "****************************************"

    sorted_keys = sorted(dict_of_nodes.keys())

    
    responding_nodes = []
    for node in sorted_keys:
        if dict_of_nodes[node]:
            responding_nodes.append(node)
    print "Nodes responding to echos n = {0}: ".format(len(responding_nodes))
    print responding_nodes
    print ""

    failing_nodes = []
    for node in sorted_keys:
        if not dict_of_nodes[node]:
            failing_nodes.append(node)
    print "Nodes NOT responding to echos n = {0}: ".format(len(failing_nodes))
    print failing_nodes






