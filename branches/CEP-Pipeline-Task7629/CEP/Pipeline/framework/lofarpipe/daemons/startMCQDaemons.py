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

master_exec   = "pipelineMCQDaemon.py"
slave_exec    = "pipelineSCQDaemon.py"



def get_master_and_slave_list_from_config(config):
    # TODO: candidate for common functionality
    master_host		= config.get( "daemon_hosts", "master_host") 

    slave_hosts_type = config.get( "daemon_hosts","slave_hosts_type")
    slave_hosts = None
    if slave_hosts_type == "static":
        slave_hosts   = config.get( "daemon_hosts", "slave_hosts") 
    elif slave_hosts_type == "range":
        slave_root = config.get( "daemon_hosts", "slave_root") 
        range_min = int(config.get( "daemon_hosts", "range_min"))
        range_max = int(config.get( "daemon_hosts", "range_max"))

        node_root_template = slave_root+"{0}"
        slave_hosts = " ".join([node_root_template.format(str(node_idx).zfill(3)) 
                       for node_idx in range(range_min, range_max+1)])

    else:
        print "unknown slave_hosts_type, check config file"
        exit(2)
    
    # When no master/host are defined (default config
    if ( len(master_host.strip()) == 0 or
         len(slave_hosts.strip()) == 0):
         print "The supplied daemon_hosts are empty"
         print "starting both Master and slave daemon on the local host"
         local_host = socket.gethostname()
         master_host = local_host
         slave_hosts = local_host
    
    slave_hosts_list = slave_hosts.split(" ")

    return (master_host, slave_hosts_list)



def start_daemons(config):
    # create or get parameters 
    install_directory = config.get( "DEFAULT", "lofarroot") 

    (master_host, slave_hosts_list) = get_master_and_slave_list_from_config(
                                        config)

    # ***********************************************************************
    # Starting of the daemons
    print "starting master on host: {0}".format(master_host)

    # We need to source both the lofarinit and qpid
    qpid_source =  ". /opt/qpid/.profile"
    lofar_source = ". {0}/lofarinit.sh".format(install_directory)

    output_redirects = " > /dev/null 2> /dev/null < /dev/null"     
    #output_redirects = "> debug.out 2> debug.err < /dev/null"

    # Specific for the master start
    # The basic command we want to run on the remote host: exec + config
    start_daemon_command = "{0}/bin/{1} {2}".format(
        install_directory, master_exec, config_path)

    # Combine, the source, the executable and redirect output
    set_env_start_daemon_cmd = "{0};{1};{2}{3}".format(
        qpid_source, lofar_source, start_daemon_command, output_redirects)

    # Now add the ssh command with correct settings for remote starting of 
    # the dameon: nohup, correct shell version and the & to start in bg
    sshCommandStr = "ssh {0} \"nohup /bin/bash -c '{1} &' \"".format(
                  master_host, set_env_start_daemon_cmd)
    process = subprocess.Popen(sshCommandStr,  shell=True)



    # now start all the slaves
    for slave_host in slave_hosts_list:
        print "starting slave on host: {0}".format(slave_host)
        # The basic command we want to run on the remote host: exec + config

        start_daemon_command = "{0}/bin/{1} {2}".format(
            install_directory, slave_exec, config_path)

        # Combine, the source, the executable and redirect output
        set_env_start_daemon_cmd = "{0};{1};{2}{3}".format(
            qpid_source, lofar_source, start_daemon_command, output_redirects)

        # Now add the ssh command with correct settings for remote starting of 
        # the dameon: nohup, correct shell version and the & to start in bg
        sshCommandStr = "ssh {0} \"nohup /bin/bash -c '{1} &' \"".format(
                      slave_host, set_env_start_daemon_cmd)
        process = subprocess.Popen(sshCommandStr,  shell=True)


def stop_daemons(config):
    (master_host, slave_hosts_list) = get_master_and_slave_list_from_config(
      config)
    # first kill the master
    sshCommandStr = "ssh {0} 'killall {1}' ".format(
                  master_host, master_exec)
    process = subprocess.Popen(sshCommandStr,  shell=True)


    # For the slave we need to ssh to the actual node
    # now start all the slaves
    for slave_host in slave_hosts_list:
        sshCommandStr = "ssh {0} 'killall {1}' ".format(
                  slave_host, slave_exec)
        process = subprocess.Popen(sshCommandStr,  shell=True)



def usage():
    print "Usage: startMCQDaemons.py [start/stop] config.cfg "
    print "Start or or stop the master and slave daemons as configured in the suplied"
    print "configuration file."
    print "Stopping is performed by issueing a killall on the respective nodes"
    exit(-1)



if __name__ == "__main__":
    # Read the config file, fail of not supplied
    n_args = len(sys.argv)
    if n_args != 3:
        usage()

    config_path = sys.argv[2]
    config_path = os.path.abspath(config_path)
    if not os.path.isfile(config_path):
        print "supplied config file {0} does not exist".format(config_path)
        sys.exit(-1)

    config = ConfigParser.ConfigParser()
    config.read(config_path)

    if (sys.argv[1] == "start"):
        start_daemons(config)
    elif (sys.argv[1] == "stop"):
        stop_daemons(config)
    else:
        usage()
      


