
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

from lofar.messagebus.message import MessageContent

class TaskFeedbackProcessing(MessageContent):
  def __init__(self, from_, forUser, summary, momID, sasID, feedback):
    super(TaskFeedbackProcessing, self).__init__(
      from_,
      forUser,
      summary,
      "task.feedback.processing",
      "1.0.0",
      momID,
      sasID)

    self.payload = str(feedback)
