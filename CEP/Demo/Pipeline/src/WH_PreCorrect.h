//  WH_PreCorrect: WorkHolder class for filling the DH_TFMatrix
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
//////////////////////////////////////////////////////////////////////

#ifndef WH_PRECORRECT_H
#define WH_PRECORRECT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "Pipeline/DH_2DMatrix.h"

/**
   The WH_PreCorrect performs a single correction on the input data flow 
   (one multiplication of every data word).
 */

class WH_PreCorrect: public WorkHolder
{
public:

  WH_PreCorrect(const string& name,
	       unsigned int channels,  // nr of input=output channels
	       int stationDim,
	       int freqDim,
	       int pol);
  
  virtual ~WH_PreCorrect();


  void  setCorrectionVector(int Field, myComplex8 Value);
  void  setCorrectionVector(myComplex8 *Value);

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
  WH_PreCorrect (const WH_PreCorrect&);

  /// Forbid assignment.
  WH_PreCorrect& operator= (const WH_PreCorrect&);


  /// Pointer to the array of input DataHolders.
  DH_2DMatrix** itsInHolders;
  /// Pointer to the array of output DataHolders.
  DH_2DMatrix** itsOutHolders;

  int itsTime;
  int itsStationDim;
  int itsFreqDim;
  int itsPols;

  DH_2DMatrix::DataType *itsCorrectionVector;

  /// Profiling States
  static int          theirProcessProfilerState; 
  
};

#endif
