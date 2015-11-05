//# TaskSpecificationSystem.h: Protocol for emission of status feedback information
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
//# $Id: TaskSpecificationSystem.h 30864 2015-02-03 16:07:56Z mol $

#ifndef LOFAR_MESSAGEBUS_TASK_SPECIFICATION_SYSTEM_H
#define LOFAR_MESSAGEBUS_TASK_SPECIFICATION_SYSTEM_H

#include <MessageBus/Message.h>
#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>

namespace LOFAR {
namespace Protocols {

class TaskSpecificationSystem: public MessageContent
{
public:
    TaskSpecificationSystem(
                        // Name of the service or process producing this message
                        const std::string &from,

                        // End-user responsible for this request (if applicable)
                        const std::string &forUser,

                        // Human-readable summary describing this request
                        const std::string &summary,

                        // Identifiers for the context of this message
                        const std::string &momID,
                        const std::string &sasID,

                        // Payload: a parset containing the generated feedback
                        const ParameterSet &feedback
                      ): 
        MessageContent( from, forUser, summary, "task.specification.system", "1.0.0", momID, sasID)
    {
           std::string buffer;
        feedback.writeBuffer(buffer);
        setTXTPayload(buffer);
    }

    // Parse a message
    TaskSpecificationSystem(const qpid::messaging::Message qpidMsg) :
        MessageContent(qpidMsg)
    { }

    ParameterSet specifications() const {
        ParameterSet result;
        result.adoptBuffer(payload.get());
        return result;
    }
};

  } // namespace Protocols
} // namespace LOFAR

#endif

