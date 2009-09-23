//#  KernelConnection.h: Class representing the network connection to a kernel.
//#
//#  Copyright (C) 2002-2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

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

  } // namespace BBS

} // namespace LOFAR

#endif
