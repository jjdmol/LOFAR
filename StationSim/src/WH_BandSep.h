//#  WH_BandSep.h:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

#ifndef STATIONSIM_WH_BANDSEP_H
#define STATIONSIM_WH_BANDSEP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleR.h>
#include <StationSim/DH_SampleC.h>
#include <Common/lofar_vector.h>

#include <aips/Arrays/Vector.h>
#include <aips/Mathematics/FFTServer.h>


/**
   This WorkHolder downsamples the data and splits them into subbands.
*/

class WH_BandSep: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_BandSep (const string& name, unsigned nout,
	      unsigned int nrcu, unsigned int nsubband,
	      const string& coeffFileName);

  virtual ~WH_BandSep();

  /// Static function to create an object.
  static WorkHolder* construct(const string& name, int ninput, int noutput,
			       const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_BandSep* make(const string& name) const;

  /// Preprocess (open coeff file and allocate internal buffer).
  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_SampleR* getInHolder(int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleC* getOutHolder(int channel);

private:
  /// Forbid copy constructor.
  WH_BandSep(const WH_BandSep&);

  /// Forbid assignment.
  WH_BandSep& operator= (const WH_BandSep&);


  /// Pointer to the array of input DataHolders.
  DH_SampleR itsInHolder;
  /// Pointer to the array of output DataHolders.
  DH_SampleC** itsOutHolders;

  /// Length of buffers.
  int itsNrcu;
  int itsNsubband;
  int itsFilterLength;
  string itsCoeffName;
  vector<vector<DH_SampleR::BufferType> > itsFilters;  // nsubband vectors
  vector<vector<DH_SampleR::BufferType> > itsBuffers;  // nrcu buffers
  Vector<DH_SampleR::BufferType> itsConv;
  Vector<DH_SampleC::BufferType> itsFFTBuf;
  FFTServer<DH_SampleR::BufferType,DH_SampleC::BufferType> itsFFTserver;
  int itsSubPos;
  int itsFiltPos;

  // DEBUG
  ofstream osDebugInput;
  int iCount;
};


#endif
