//  WH_Correlate: WorkHolder class for filling the DH_TFMatrix
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

#ifndef WH_CORRELATE_H
#define WH_CORRELATE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/WorkHolder.h"
#include "Transpose/DH_2DMatrix.h"
#include "CEPFrame/DH_Empty.h"

/**
   The WH_Tranpose
 */

class WH_Correlate: public WorkHolder
{
public:

  WH_Correlate(const string& name,
	       unsigned int nin,       // nr of input channels
	       unsigned int nout,      // nr of output channels
	       int stationDim,
	       int freqDim);
  
  virtual ~WH_Correlate();

  virtual WorkHolder* make(const string& name) const;

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_2DMatrix* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_Empty* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Correlate (const WH_Correlate&);

  /// Forbid assignment.
  WH_Correlate& operator= (const WH_Correlate&);


  /// Pointer to the array of input DataHolders.
  DH_2DMatrix** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_Empty** itsOutHolders;

  int itsTime;
  int itsStationDim;
  int itsFreqDim;

  /// Profiling States
  static int          theirProcessProfilerState; 
  
};

#endif
