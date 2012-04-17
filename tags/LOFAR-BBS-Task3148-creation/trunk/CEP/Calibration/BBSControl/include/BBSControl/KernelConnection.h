//# KernelConnection.h: Class representing the network connection to a kernel.
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

#ifndef LOFAR_BBSCONTROL_KERNELCONNECTION_H
#define LOFAR_BBSCONTROL_KERNELCONNECTION_H

// \file
// Class representing the network connection to a kernel.

#include <Common/lofar_smartptr.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/Messages.h>
#include <BBSControl/Types.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    class KernelConnection
    {
    public:
      // Convenience typedef
      typedef shared_ptr<BlobStreamableConnection> Connection;

      KernelConnection() :
        itsIndex(KernelIndex(-1))
      {}

      KernelConnection(const Connection& connection, const KernelIndex& index) :
        itsConnection(connection), itsIndex(index)
      {}

      const KernelIndex& index() const { return itsIndex; }

      // Receive a kernel message.
      shared_ptr<const KernelMessage> recvMessage() const;

      // Send a solver message.
      void sendMessage(const SolverMessage& message) const;

    private:
      // Connection to our kernel.
      Connection itsConnection;

      // ID of the kernel we're connected to.
      KernelIndex itsIndex;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
