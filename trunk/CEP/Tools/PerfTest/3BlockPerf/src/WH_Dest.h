//  WH_Dest.h: WorkHolder class using DH_FixedSize() objects 
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
//
//
//////////////////////////////////////////////////////////////////////

#ifndef WH_DEST_H
#define WH_DEST_H

#ifdef HAVE_CONFIG_H
#include <lofar_config.h>
#endif

#include "tinyCEP/WorkHolder.h"
#include "3BlockPerf/DH_FixedSize.h"

namespace LOFAR {

/**
   The WH_Dest class implements a workholder with a DH_VarSize
   object as input and outputs. The process() method does nothing to
   the data 
 */

class WH_Dest: public LOFAR::WorkHolder
{
public:

  WH_Dest (const string& name="WH_Dest",
           unsigned int size = 100);    // size of the packet in bytes
  
  virtual ~WH_Dest();

  virtual WorkHolder* make(const string& name);

  /// Do a process step.
  virtual void process();

private:
  /// Forbid copy constructor.
  WH_Dest (const WH_Dest&);

  /// Forbid assignment.
  WH_Dest& operator= (const WH_Dest&);
  
  unsigned int itsSize;
};
}
#endif
