//  TransportHolder.h:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
// TransportHolder.h: interface for the TransportHolder class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_TRANSPORTHOLDER_H
#define CEPFRAME_TRANSPORTHOLDER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Common/lofar_string.h"

/**
   This class defines the base class for transport mechanism classes
   to transport data between connected DataHolders.
   Actually, the data transport is done between 2 TransportHolder objects
   belonging to the communicating DataHolder objects.

   Derived classes (e.g. TH_MPI) implement the concrete transport
   classes.

   If data have to be transported between different machines, they
   need to have the same data representation. It is not possible
   yet to transport data between e.g. a SUN and PC. This will be
   improved in the future.
*/

class Transport;

class TransportHolder
{
public:
  TransportHolder();

  virtual ~TransportHolder();

  /// Make an instance of the derived TransportHolder.
  virtual TransportHolder* make() const = 0;

  /// Recv the data sent by the connected TransportHolder.
  virtual bool recv (void* buf, int nbytes, int source, int tag) = 0;

  /// Send the data to the connected TransportHolder.
  virtual bool send (void* buf, int nbytes, int destination, int tag) = 0;

  /// Get the type of transport as a string.
  virtual string getType() const = 0;

  // Allocate/deallocate memory for use in communication.
  virtual void* allocate (size_t size);
  virtual void deallocate (void*& ptr);

  virtual bool connectionPossible (int srcRank, int dstRank) const;

  Transport * getTransport () { return itsTransport; }
  void setTransport (Transport * Trn) { 
    itsTransport = Trn; 
  }

  virtual bool isBlocking() const = 0;

private:
  Transport * itsTransport;
};




#endif
