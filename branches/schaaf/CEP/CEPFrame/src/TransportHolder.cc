//  TransportHolder.cc:
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
//  Revision 1.8  2002/05/14 07:54:47  gvd
//  Moved virtual functions to .cc file
//  Removed INCLUDES from ShMem Makefile.am
//  Add LOFAR_DEPEND to test/Makefile.am
//
//  Revision 1.7  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.6  2002/03/14 14:24:07  wierenga
//  New TransportHolder interface and implementation. The TransportHolder
//  is no longer dependent on the Transport class (this was a circular
//  dependency) and may now be used independent of the Transport class.
//
//  Revision 1.5  2001/08/16 14:33:08  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.4  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.3  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////


#include "BaseSim/TransportHolder.h"
#include <stdlib.h>


TransportHolder::TransportHolder()
{}

TransportHolder::~TransportHolder()
{}


void* TransportHolder::allocate (size_t size)
{
  return malloc(size);
}

void TransportHolder::deallocate (void*& ptr)
{
  free(ptr);
  ptr = 0;
}

bool TransportHolder::connectionPossible (int, int) const
{
  return false;
}
