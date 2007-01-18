//# BGLConnection.h: Class which handles transport between DataHolders
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_TRANSPORT_BGLCONNECTION_H
#define LOFAR_TRANSPORT_BGLCONNECTION_H

// \file
// Connects DataHolder(s) with a TransportHolder. 

//# Includes
#include <Transport/Connection.h>

namespace LOFAR
{

// \addtogroup Transport
// @{


// This class inherits from Connection but reimplements the blocking reads and
// writes so that reads and writes are done 

class BGLConnection : public Connection
{
  public:
    /// Construct the BGLConnection object.  For good performance, data sent
    // from/to the Blue Gene/L should send messages that are aligned on 16
    // bytes and have a length that is a multiple of 16 bytes.  Moreover,
    // read and write sizes should match.
    // THIS CLASS ASSUMES THAT A BLOB IS ALLOCATED AT A 16-BYTE BOUNDARY AND
    // HAS A CAPACITY THAT IS A MULTIPLE OF 16 BYTES, even though its size
    // need not be a multiple of 16 bytes.
    // BGLConnection connects a source DataHolder to a destination DataHolder
    // with a TransportHolder object. Data flows from dhSource -> dhDest
    // The DH's must be UNinitialized DH's.
    BGLConnection(const string& name, DataHolder* dhSource, DataHolder* dhDest, 
		  TransportHolder* th);

    ~BGLConnection();

    /// Read the data.
    virtual State read ();

    /// Send the data.
    virtual State write ();

  private:
    unsigned align(unsigned size, unsigned alignment);
};

inline unsigned BGLConnection::align(unsigned size, unsigned alignment)
{
  return (size + alignment - 1) & ~(alignment - 1);
}

// @} // Doxygen endgroup Transport

} // end namespace


#endif 
