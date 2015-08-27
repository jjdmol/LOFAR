//# TaskFeedbackState.h: Protocol for emission of status feedback information
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

#include <MessageBus/Message.h>
#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>

namespace LOFAR {

namespace Protocols {

class TaskFeedbackState: public MessageContent
{
public:
  Property type;
  Property state;

  TaskFeedbackState(
    // Name of the service or process producing this message
    const std::string &from,

    // End-user responsible for this request (if applicable)
    const std::string &forUser,

    // Human-readable summary describing this request
    const std::string &summary,

    // Identifiers for the context of this message
    const std::string &momID,
    const std::string &sasID,

    // Payload: a boolean indicating success
    bool success
  ):
  MessageContent(
    from,
    forUser,
    summary,
    "task.feedback.state",
    "1.0.0",
    momID,
    sasID)
  {
    addPayloadProperties();

    setXMLPayload(formatString(
      "<task>\n"
      "  <type>observation</type>\n"
      "  <state>%s</state>\n"
      "</task>",
      success ? "finished" : "aborted"));
  }

  // Parse a message
  TaskFeedbackState(const qpid::messaging::Message qpidMsg)
  :
    MessageContent(qpidMsg)
  {
    addPayloadProperties();
  }

  void addPayloadProperties()
  {
    type.attach (&itsContent, "message/payload/task/type");
    state.attach(&itsContent, "message/payload/task/state");
  }
};

} // namespace Protocols

} // namespace LOFAR

#endif

