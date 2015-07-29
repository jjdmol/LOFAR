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
import subprocess


if __name__ == "__main__":
    # Read the config file, fail of not supplied
    if len(sys.argv) != 2:
        print "Usage: startMCQDaemons.py config.cfg"
        print "Start the master and slave daemon as configured in the suplied"
        print "configuration file."

        exit(-1)

    config_path = sys.argv[1]
    if not os.path.isfile(config_path):
        print "supplied config file {0} does not exist".format(config_path)
        sys.exit(-1)

    config = ConfigParser.ConfigParser()
    config.read(config_path)

    # create or get parameters 
    install_directory = config.get( "DEFAULT", "lofarroot") 
    master_exec   = "pipelineMCQDaemon.py"
    slave_exec    = "pipelineSCQDaemon.py"
    master_host		= config.get( "daemon_hosts", "master_host") 

    slave_hosts_type = config.get( "daemon_hosts","slave_hosts_type")
    slave_hosts = None
    if slave_hosts_type == "static":
        slave_hosts   = config.get( "daemon_hosts", "slave_hosts") 
    elif slave_hosts_type == "range":
        print "debug 1"
        slave_root = config.get( "daemon_hosts", "slave_root") 
        range_min = int(config.get( "daemon_hosts", "range_min"))
        range_max = int(config.get( "daemon_hosts", "range_max"))

        node_root_template = slave_root+"{0}"
        slave_hosts = " ".join([node_root_template.format(str(node_idx).zfill(3)) 
                       for node_idx in range(range_min, range_max+1)])

        print slave_hosts
    else:
        print "debiug sdf"
        exit(2)
    


    # The default config file does not have master and slave 
    if ( len(master_host.strip()) == 0 or
         len(slave_hosts.strip()) == 0):
         print "The supplied daemon_hosts are empty"
         print "starting both Master and slave daemon on the local host"
         local_host = socket.gethostname()
         master_host = local_host
         slave_hosts = local_host
    
    slave_hosts_list = slave_hosts.split(" ")

    # Parts of the ssh command needed to start the daemons
    source_cmd = "source {0}/lofarinit.sh".format(install_directory)
    output_redirects = " > /dev/null 2> /dev/null < /dev/null"     
    #output_redirects = "> debug.out 2> debug.err < /dev/null"

    # First start the master
    print "starting master on host: {0}".format(master_host)
    start_daemon_cmd = "{0}/bin/{1} {2}".format(install_directory,
                                   master_exec, config_path)
    bashCommand = "ssh {0} nohup {1} ; {2}{3} &".format(
                master_host, source_cmd, start_daemon_cmd, output_redirects)       
    process = subprocess.Popen(bashCommand,  shell=True)


    # now start all the slaves
    for slave_host in slave_hosts_list:
        print "starting slave on host: {0}".format(slave_host)
        output_redirects = "> debug.out 2> debug.err < /dev/null"
        start_daemon_cmd = "{0}/bin/{1} {2}".format(install_directory,
                                   slave_exec, config_path)

        bashCommand = "ssh {0} 'nohup sh -c ''{1} ; {2}{3} &'''".format(
                slave_host, source_cmd, start_daemon_cmd, output_redirects)       
        print bashCommand
        process = subprocess.Popen(bashCommand,  shell=True)


    