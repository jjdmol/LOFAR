//  WH_Transpose: WorkHolder class for filling the DH_TFMatrix
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

#ifndef WH_TRANSPOSE_H
#define WH_TRANSPOSE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "Pipeline/DH_2DMatrix.h"
#include "BaseSim/DH_Empty.h"

/**
   The WH_Tranpose
 */

class WH_Transpose: public WorkHolder
{
public:

  WH_Transpose(const string& name,
	       unsigned int nin,      // nr of input channels
	       unsigned int nout,      // nr of output channels
	       int timeDim,
	       int freqDim,
	       int pols);
  
  virtual ~WH_Transpose();

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

private:
  /// Forbid copy constructor.
  WH_Transpose (const WH_Transpose&);

  /// Forbid assignment.
  WH_Transpose& operator= (const WH_Transpose&);


  /// Pointer to the array of input DataHolders.
  DH_2DMatrix** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_2DMatrix** itsOutHolders;

  int itsTimeDim;
  int itsFreqDim;
  int itsPols;

  /// Profiling States
  static int          theirProcessProfilerState; 
  
};

inline DH_2DMatrix* WH_Transpose::getInHolder (int channel)
{
  DbgAssertStr (channel >= 0,          "input channel too low");
  DbgAssertStr (channel < getInputs(), "input channel too high");
  return itsInHolders[channel];
}

inline DH_2DMatrix* WH_Transpose::getOutHolder (int channel)
{
  DbgAssertStr (channel >= 0,           "output channel too low");
  DbgAssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}


#endif
