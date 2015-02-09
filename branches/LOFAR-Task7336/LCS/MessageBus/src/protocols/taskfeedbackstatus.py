
#!/usr/bin/python
# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
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

#import lofar.messagebus.Message
import lofar.messagebus.message
import xml.dom.minidom as xml

LOFAR_STATUS_MSG_TEMPLATE = """
<task>
  <type/>
  <state/>
</task>"""

class TaskFeedbackStatus(lofar.messaging.message.Message):
  def __init__(self, from_, forUser, summary, momID, sasID, status):
    super(TaskFeedbackStatus, self).__init__(
      from_,
      forUser,
      summary,
      "lofar.task.feedback.status",
      "1.0.0",
      momID,
      sasID)

    payload_document = xml.parseString(LOFAR_STATUS_MSG_TEMPLATE)

    self._getXMLnode("message.payload").appendChild(payload_document.firstChild)

    self.type_ = "pipeline"
    self.state = "finished" if status else "aborted"

  def _property_list(self):
     properties = super(TaskFeedbackStatus, self)._property_list()
     
     properties.update( {
       "type_": "message.payload.task.type",
       "state": "message.payload.task.state",
     } )

     return properties

if __name__ == "__main__":
    msg = TaskFeedbackStatus("FROM", "FORUSER", "SUMMARY", "11111", "22222", True)
    print msg.document.toxml()

