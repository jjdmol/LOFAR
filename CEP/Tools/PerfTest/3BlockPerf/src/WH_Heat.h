//  WH_Heat.h: WorkHolder class that performs a number of flops per byte
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

#ifndef WH_HEAT_H
#define WH_HEAT_H

#ifdef HAVE_CONFIG_H
#include <lofar_config.h>
#endif

#include "tinyCEP/WorkHolder.h"
#include "3BlockPerf/DH_FixedSize.h"

namespace LOFAR {

/**
   The WH_Heat class implements a workholder with a DH_FixedSize
   object as input and output. The process() method performs a specified number of flops per byte
 */

class WH_Heat: public LOFAR::WorkHolder
{
public:

  WH_Heat (const string& name="WH_Heat",
	   unsigned int size = 100,    // size of the packet in bytes
	   unsigned int flopsPB = 1); //number of flops per byte
  
  virtual ~WH_Heat();

  virtual WorkHolder* make(const string& name);

  virtual void preprocess();
  /// Do a process step.
  virtual void process();
private:
  /// Forbid copy constructor.
  WH_Heat (const WH_Heat&);

  /// Forbid assignment.
  WH_Heat& operator= (const WH_Heat&);

  unsigned int itsSize;
  unsigned int itsFlopsPerByte;
  DH_FixedSize::valueType itsFactor; // the factor to multiple with
  unsigned int itsNoValues; // number of values in a DataHolder
  unsigned int itsNoMultiplications;
};

}
#endif
