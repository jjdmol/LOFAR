//  WH_Delay: WorkHolder class for Delaying input DataHolders
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
//////////////////////////////////////////////////////////////////////

#ifndef WH_DELAY_H
#define WH_DELAY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/WorkHolder.h"
#include "Transpose/DH_2DMatrix.h"
#include "CEPFrame/DH_Empty.h"
#include "CEPFrame/CyclicBuffer.h"

class WH_Delay: public WorkHolder
{
public:

  WH_Delay(const string& name,
	       unsigned int nin,      // nr of input channels
	       unsigned int nout,     // nr of output channels
	       int timeDim,
	       int freqDim);

  
  virtual ~WH_Delay();

  virtual WorkHolder* make(const string& name) const;

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_2DMatrix* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_2DMatrix* getOutHolder (int channel);

  void setDelay(const int delay);

private:
  /// Forbid copy constructor.
  WH_Delay (const WH_Delay&);

  /// Forbid assignment.
  WH_Delay& operator= (const WH_Delay&);


  /// Pointer to the array of input DataHolders.
  DH_2DMatrix** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_2DMatrix** itsOutHolders;

  /// Profiling States
  static int          theirProcessProfilerState; 
  
  // the delay in steps
  int itsDelay; 

  int itsTimeDim;
  int itsFreqDim;

  CyclicBuffer<DH_2DMatrix*> *itsBuffer;
};

#endif
