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
//  $Log$
//  Revision 1.12  2002/05/14 07:54:47  gvd
//  Moved virtual functions to .cc file
//  Removed INCLUDES from ShMem Makefile.am
//  Add LOFAR_DEPEND to test/Makefile.am
//
//  Revision 1.11  2002/05/08 14:20:25  wierenga
//  Added allocate/deallocate and connectionPossible methods.
//
//  Revision 1.10  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.9  2002/03/14 14:24:07  wierenga
//  New TransportHolder interface and implementation. The TransportHolder
//  is no longer dependent on the Transport class (this was a circular
//  dependency) and may now be used independent of the Transport class.
//
//  Revision 1.8  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.7  2001/10/05 11:50:37  gvd
//  Added getType function
//
//  Revision 1.6  2001/08/16 14:33:08  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.5  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.4  2001/02/05 14:53:05  loose
//  Added GPL headers
//

// TransportHolder.h: interface for the TransportHolder class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_TRANSPORTHOLDER_H
#define BASESIM_TRANSPORTHOLDER_H

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


private:
};


#endif
