//  WH_Src.h: WorkHolder class using DH_FixedSize() objects and
//                 measuring performance
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

#ifndef WH_SRC_H
#define WH_SRC_H

#ifdef HAVE_CONFIG_H
#include <lofar_config.h>
#endif

#include "tinyCEP/WorkHolder.h"
#include "3BlockPerf/DH_FixedSize.h"
#include "3BlockPerf/StopWatch.h"

namespace LOFAR {

/**
   The WH_Src class implements a workholder with a DH_FixedSize
   object as output. The process() method puts a simple pattern in the output
 */

class WH_Src: public LOFAR::WorkHolder
{
public:

  WH_Src (const string& name,
	  unsigned int size,     // size of the packet in bytes
	  unsigned int packetsPerMeasurment,
	  unsigned int flopsPerByte); // the number of flopsPerByte is not used in this WH, it is only printed on stdout
  
  virtual ~WH_Src();

  virtual WorkHolder* make(const string& name);

  /// Do a process step.
  virtual void process();
private:
  /// Forbid copy constructor.
  WH_Src (const WH_Src&);

  /// Forbid assignment.
  WH_Src& operator= (const WH_Src&);

  /// Fixed size
  unsigned int itsSize;

  /// Used to do timing on communication
  StopWatch *watch;
  unsigned int itsIteration;
  unsigned int itsPacketsPerMeasurement;
  unsigned int itsDataSize;
  unsigned int itsFlopsPerByte;
};


}
#endif
