//  WH_Filter.h:
//
//  Copyright (C) 2002
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
//
//////////////////////////////////////////////////////////////////////

#ifndef STATIONSIM_WH_FILTER_H
#define STATIONSIM_WH_FILTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/WorkHolder.h"
#include "StationSim/DH_Sample.h"
#include <Common/lofar_vector.h>


/**
   This WorkHolder filters and downsamples the merged data.
*/

class WH_Filter: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_Filter (const string& name, unsigned nout,
	     unsigned int nrcu, unsigned int nsubband,
	     const string& coeffFileName);

  virtual ~WH_Filter();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_Filter* make (const string& name) const;

  /// Preprocess (open coeff file and allocate internal buffer).
  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_Sample* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_Sample* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Filter (const WH_Filter&);

  /// Forbid assignment.
  WH_Filter& operator= (const WH_Filter&);


  /// Pointer to the array of input DataHolders.
  DH_Sample itsInHolder;
  /// Pointer to the array of output DataHolders.
  DH_Sample** itsOutHolders;

  /// Length of buffers.
  int itsNrcu;
  int itsNsubband;
  string itsCoeffName;
  int itsNcoeff;
  vector<float> itsCoeff;
  
  DH_Sample::BufferType** itsBuffer;
  int itsBufPos;
};


#endif
