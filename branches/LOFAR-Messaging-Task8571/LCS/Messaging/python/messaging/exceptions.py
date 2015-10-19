# exceptions.py: Exceptions used by the lofar.messaging module.
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: exceptions.py 1580 2015-09-30 14:18:57Z loose $

"""
Exceptions used by the lofar.messaging module.
"""

class MessagingError(Exception):
    """
    Base exception class for the lofar.messaging package.
    """
    pass


class InvalidMessage(MessagingError):
    """
    Exception raised when an invalid message is received.
    """
    pass


class MessageBusError(MessagingError):
    """
    Exception raised by the FromBus and ToBus classes if communication fails.
    """
    pass

class MessageFactoryError(MessagingError):
    """
    Exception raised when the `MessageFactory` does not know how to create
    a message object, because it wasn't registered with it.
    """
    pass

