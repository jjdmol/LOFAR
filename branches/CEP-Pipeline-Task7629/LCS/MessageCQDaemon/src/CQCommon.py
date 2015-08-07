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

import lofar.messagebus.message as message
import time
from datetime import datetime, timedelta

def create_msg(payload, subject=None):
    msg = message.MessageContent(
                    from_="test",
                    forUser="",
                    summary="summary",
                    protocol="protocol",
                    protocolVersion="test")
    msg.payload = payload

    if subject:
        msg.set_subject(subject)

    return msg

def get_master_and_slave_list_from_config(config):
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



def sendEchoToSlaveListReturnResponce(toBus, namedFromBus, returnSubject,
                slaveSubjectTemplate, slave_hosts_list, grace_period=30):
    """
    Send an echo msg to all slave in the list and wait for reply.

    Return tupe list 
      List contains the list of slaves not responding within grace_period
      seconds
    """
    echo_received = {}
    n_echo_send = len(slave_hosts_list)

    for slave in slave_hosts_list:
        payload = {'command':"echo",                      # echo msg
                   'type':'command',
                   'echo_type':"slave_echo",
                   'return_subject':returnSubject}  # send echo to this q

        msg = create_msg(payload, slaveSubjectTemplate.format(slave))
        toBus.send(msg)
        echo_received[slave] = False

    start_tick = datetime.now()
    end_waitperiod = start_tick + timedelta(0,grace_period)

    count = 0
    while datetime.now() < end_waitperiod:
        while True:
            msg_retrieved, data =  _get_next_msg_and_content(  namedFromBus)

            if not msg_retrieved:
                break
            (msg, unpacked_msg_content, msg_type) = data

            sending_host =  unpacked_msg_content['host']
            echo_received[sending_host] = True
            count += 1

        if count == n_echo_send:
            break

        time.sleep(1)

    all_nodes_responded = count == n_echo_send
    return all_nodes_responded, echo_received

        

def _get_next_msg_and_content(from_bus):
      """
      Helper function attempts to get the next msg from the command queue

      Unpacks the data and assign data to the second return value.
      Returns false and none if no valid msg is available

      Invalid msg are printed to logged and stored in the deadletter log
      """
      # Test if the timeout is in milli seconds or second
      while True:
          msg = from_bus.get(0.1)  #  use timeout, very short, well get 
          # there next time if more time is needed
          if msg == None:
              return False, None

          # Get the needed information from the msg
          unpacked_msg_content, msg_type  = _save_unpack_msg(msg)
          if not msg_type:  # if unpacking failed
              print "Could not process msg, incorrect content: {0}".format(
                  unpacked_msg_content)
              from_bus.ack(msg) 
              continue
                

          from_bus.ack(msg)

          return True, (msg, unpacked_msg_content, msg_type) 

# ****************************************************************************
# Candidate functions for external lib.
# **************************************************************************

def _save_unpack_msg(msg):
        """
        Private helper function unpacks a received msg and casts it to 
        a msg_Content dict, the command string is also extracted
        content and command are returned as a pair
        returns None if an error was encountered
        """
        msg_content = None
        msg_type = None
        try:
            
            # currently the expected payload is a dict
            msg_content = eval(msg.content().payload)
            msg_type =  msg_content['type']

        except Exception, ex:
            print "Failed evaluating msg data"
            # TODO: return 'type' is not the same in error case..
            # tight coupling with calling function.
            return msg.content().payload, False  

        return msg_content, msg_type

