//# MWConnection.cc: Abstract base class for all MWConnections
//#
//# Copyright (c) 2007
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

#include <MWCommon/MWConnection.h>
#include <Blob/BlobString.h>
#include <Blob/BlobHeader.h>


namespace LOFAR { namespace CEP {

  MWConnection::~MWConnection()
  {}

  void MWConnection::init()
  {}

  bool MWConnection::isConnected() const
  {
    return true;
  }

  void MWConnection::read (LOFAR::BlobString& buf)
  {
    // Try to get the length of the message.
    // If it succeeds, read the data.
    int msgLen = getMessageLength();
    if (msgLen > 0) {
      buf.resize (msgLen);
      receive (buf.data(), msgLen);
    } else {
      // Otherwise read blob header first to get the length.
      LOFAR::BlobHeader hdr;
      receive (&hdr, sizeof(hdr));
      msgLen = hdr.getLength();
      buf.resize (msgLen);
      memcpy (buf.data(), &hdr, sizeof(hdr));
      receive (buf.data() + sizeof(hdr), msgLen-sizeof(hdr));
    }
  }

  void MWConnection::write (const LOFAR::BlobString& buf)
  {
    send (buf.data(), buf.size());
  }

}} // end namespaces
