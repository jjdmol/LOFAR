//# TaskFeedbackStatus.h: Protocol for emission of status feedback information
//#
//# Copyright (C) 2015
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_MESSAGEBUS_TASK_FEEDBACK_STATUS_H
#define LOFAR_MESSAGEBUS_TASK_FEEDBACK_STATUS_H

#ifdef HAVE_QPID
#include <MessageBus/Message.h>
#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>

namespace LOFAR {

namespace Protocols {

class TaskFeedbackStatus: public Message
{
public:
  TaskFeedbackStatus(
    // Name of the service or process producing this message
    const std::string &from,

    // End-user responsible for this request (if applicable)
    const std::string &forUser,

    // Human-readable summary describing this request
    const std::string &summary,

    // Payload: a boolean indicating success
    bool success
  ):
  Message(
    from,
    forUser,
    summary,
    "lofar.task.feedback.status",
    "1.0.0")
  {
    setXMLPayload(formatString(
      "<task>
        <type>observation</type>
        <state>%s</state>
      </task>",
      success ? "finished" : "aborted"));
  }

  // Parse a message
  TaskFeedbackStatus(const qpid::messaging::Message qpidMsg)
  :
    Message(qpidMsg)
  {
  }

  // Read a message from disk (header + payload)
  TaskFeedbackStatus(const std::string &rawContent)
  :
    Message(rawContent)
  {
  }

  ParameterSet feedback() const {
    ParameterSet result;
    result.adoptBuffer(payload());

    return result;
  }
};

} // namespace Protocols

} // namespace LOFAR

#endif

#endif

