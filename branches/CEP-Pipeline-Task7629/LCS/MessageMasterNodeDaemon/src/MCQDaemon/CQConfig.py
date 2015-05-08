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

import os
import socket
import pwd

import lofar.messagebus.message as message

# TODO: should this be increased each commit??
# First set the protocol version 
protocolVersion="0.0.1"

# Session parameters
broker = "127.0.0.1"                             # Localhost 
hostname = socket.gethostname()
username = pwd.getpwuid(os.getuid()).pw_name

# QUeue names session specific
returnQueueTemplate     = "MCQDaemon.{0}.return.{1}"
logTopicTemplate        = "MCQDaemon.{0}.log.{1}"
parameterQueueTemplate  = "NCQDaemon.{0}.parameters.{1}"

# Config for the master/node framework
nodeCommandQueueTemplate= "{0}.{1}.NCQueueDaemon.CommandQueue"
masterCommandQueueTemplate="{0}.{1}.MCQueueDaemon.CommandQueue"

def create_returnQueue_name(uuid):
    """
    Returns a fully constructed name for a unique returnQueue unique
    """
    return  returnQueueTemplate.format(username, uuid)

def create_logTopic_name(uuid):
    """
    Returns a fully constructed name for a unique returnQueue unique
    """
    return  logTopicTemplate.format(username, uuid)

def create_parameterQueue_name(uuid):
    """
    Returns a fully constructed name for a unique returnQueue unique
    """
    return  parameterQueueTemplate.format(username, uuid)

def create_nodeCommandQueue_name(i_hostname=None):
    """
    Returns a fully constructed name for a unique returnQueue unique
    """
    l_hostname = None
    if i_hostname is None:
        l_hostname = hostname
    else:
        l_hostname = i_hostname
    return  nodeCommandQueueTemplate.format(username, 
                                            l_hostname)

def create_masterCommandQueue_name():
    """
    Returns a fully constructed name for the master command queue name
    """
    return masterCommandQueueTemplate.format(username, hostname)


def create_msg_header(from_template, for_template, summary,
                      protocol):
    """
    create a msg header from supplied template parameters


    """
    msg = message.MessageContent(
                from_=from_template.format(username, hostname),
                forUser=for_template.format(username),
                summary=summary,
                protocol=protocol,
                protocolVersion=protocolVersion, 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )

    return msg

# Protocol details
msg_fromNCQDaemon_template  = "{0}.{1}.NCQDaemon"
msg_forMCQDaemon_template   = "{0}.MSQDaemon"
msg_forNCQLib_template      = "{0}.NCQDaemon"

# Log msg
log_msg_protocol_name_template = "{0}LogMsg"
log_msg_summary_template       = "{0} log message"
log_msg_levels        = ['CRITICAL','ERROR', 'WARNING', 'INFO', 'DEBUG']

def create_validated_log_msg(level, log_data, sender):
    """
    Creates a fully instantiated log msg.

    It performs a check of the supplied arguments, sets the correct msg
    fields
    """
    # First check if the level is known, if not, set to warning and
    # prepent the log line with an error
    if level not in log_msg_levels:        
        prefix = "Uknown Error Level<{0}>: ".format(level)
        level = 'WARNING'
        log_data = prefix + log_data

    # Create the header
    msg = create_msg_header(msg_fromNCQDaemon_template,
                msg_forMCQDaemon_template,
                log_msg_summary_template.format(sender),
                log_msg_protocol_name_template.format(sender))

    # add the data to send
    msg.payload = {'level':   level,
                   'log_data':log_data}

    return msg

# Return msg
return_msg_protocol_name_template = "{0}ReturnMsg"
return_msg_summary_template       = "{0} return message"
return_msg_types                  = ['exit_value']


def _validate_return_exit_dict(payload):
    """
    Checks if the return payload is correct according to the protocol
    """
    # First the correct msg type
    keys = payload.keys()
    if 'type' in keys:
        if not payload['type'] in return_msg_types:
            return False
    else:
         return False

    # Then check the other keys
    if 'exit_value' not in keys:
         return False

    if 'uuid' not in keys:
         return False

    if 'job_uuid' not in keys:
         return False

    return True

def create_validated_return_msg(exit_dict, sender):
    """
    Creates a fully instantiated log msg.

    It performs a check of the supplied arguments, sets the correct msg
    fields

     payload = {'type':"exit_value",
                'exit_value':exit_status,
                'uuid':uuid,
                'job_uuid':job_uuid}
    """
    # First check if msg type is correct
    if not _validate_return_exit_dict(exit_dict):
        raise Exception("Incorrect return msg payload received: {0}".format(
                        repr(payload)))

    # Create the header
    msg = create_msg_header(msg_fromNCQDaemon_template,
                msg_forMCQDaemon_template,
                return_msg_summary_template.format(sender),
                return_msg_protocol_name_template.format(sender))

    # add the data to send
    msg.payload = exit_dict

    return msg

# parameter msg
parameter_msg_protocol_name_template = "{0}ParameterMsg"
parameter_msg_summary_template       = "{0} parameter message"

def _validate_parameter_dict(payload):
    """
    Checks if the return payload is correct according to the protocol
    """
    # First the correct msg type
    keys = payload.keys()
    if 'uuid' not in keys:
         return False
    if 'job_uuid' not in keys:
         return False
    if 'parameters' not in keys:
         return False
    else:    # Check content of the parameters dict()
        parameter_keys = payload['parameters'].keys()
        if 'node' not in parameter_keys:
             return False
        if 'environment' not in parameter_keys:
             return False
        if 'cmd' not in parameter_keys:
             return False
        if 'job_parameters' not in parameter_keys:
             return False
        if 'cdw' not in parameter_keys:
             return False

    return True
              
def create_validated_parameter_msg(payload, sender):
    """
    Creates a fully instantiated log msg.
    """
    # First check if msg type is correct
    if not _validate_parameter_dict(payload):
        raise Exception("Incorrect parameter msg payload received: {0}".format(
                        repr(payload)))

    # Create the header
    msg = create_msg_header(msg_fromNCQDaemon_template,
                msg_forMCQDaemon_template,
                parameter_msg_summary_template.format(sender),
                parameter_msg_protocol_name_template.format(sender))

    # add the data to send
    msg.payload = payload

    return msg


 