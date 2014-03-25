//# KernelConnection.cc: class representing the network connection to a kernel.
//#
//# Copyright (C) 2002-2008
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

#include <lofar_config.h>
#include <BBSControl/KernelConnection.h>

namespace LOFAR
{
  namespace BBS
  {
    shared_ptr<const KernelMessage> KernelConnection::recvMessage() const
    {
      return shared_ptr<const KernelMessage>
        (dynamic_cast<const KernelMessage*>(itsConnection->recvObject()));
    }

    void KernelConnection::sendMessage(const SolverMessage& message) const
    {
      itsConnection->sendObject(message);
    }

  } // namespace BBS

} // namespace LOFAR

