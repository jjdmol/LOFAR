//  WH_Dest.h: WorkHolder class using DH_VarSize() objects and 
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

#ifndef WH_DEST_H
#define WH_DEST_H

#ifdef HAVE_CONFIG_H
#include <lofar_config.h>
#endif

#include "tinyCEP/WorkHolder.h"
#include "P2Perf/DH_VarSize.h"
#include "P2Perf/DHGrowStrategy.h"
#include "P2Perf/StopWatch.h"

/**
   The WH_Dest class implements a workholder with DH_VarSize
   objects as inputs and outputs. The process() method does nothing to
   the data (not even copy...) but can contain a performance measurement
   indication for the data transport bandwidth of the output DataHolders. 
 */

class WH_Dest: public LOFAR::WorkHolder
{
public:

  WH_Dest (DHGrowStrategy* DHGS, // the object that will grow the DH's
           const string& name="WH_Dest",
	   unsigned int nin=1,      // nr of input channels
           unsigned int size = 100,    // size of the packet in bytes
           unsigned int packetsPerGrowStep = 100); // this is how long the data size will remain constant
  
  virtual ~WH_Dest();

  virtual WorkHolder* make(const string& name);

  virtual void preprocess();
  
  /// Do a process step.
  virtual void process();

private:
  /// Forbid copy constructor.
  WH_Dest (const WH_Dest&);

  /// Forbid assignment.
  WH_Dest& operator= (const WH_Dest&);
  
  DHGrowStrategy * itsDHGrowStrategy;
  
  unsigned int itsSize;
  unsigned int itsPacketsPerGrowStep;
  unsigned int itsIteration;

  StopWatch * watch;
};
#endif
