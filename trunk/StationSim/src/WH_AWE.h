//#  WH_AWE.h:
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
//#  Chris Broekema, november 2002.
//#
//#  $Id$
//#

#ifndef STATIONSIM_WH_AWE_H
#define STATIONSIM_WH_AWE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/DH_Weight.h>

/**
   This workholder contains the main AWE procedure.
   This includes processing of the snapshot fifo, 
   selection of the adaptive algorithm and calling 
   the specified algorithm. Inputs consist of a 
   snapshot vector and a fifo. Outputs are a weight 
   vector, and an updated fifo.
*/

class WH_AWE: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_AWE (const string& name,
	  unsigned int nin, 
	  unsigned int nout,
	  unsigned int nant,
	  unsigned int buflength);

  virtual ~WH_AWE();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_AWE* make (const string& name) const;

  /// Generate a snapshot matrix from the FIFO
  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  /// The first one is the sampled data.
  /// The second one is the selected subbands.
  virtual DH_SampleC* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_Weight* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_AWE (const WH_AWE&);

  /// Forbid assignment.
  WH_AWE& operator= (const WH_AWE&);

  /// Calculate a steer vector

  /// In- and OutHolders
  DH_SampleC itsInHolder;
  DH_Weight  itsOutHolder;
  


  /// Length of buffers.
  unsigned int itsNrcu;
  unsigned int itsBufLength;

  LoVec_double   px, py; // Array configuration vectors
  LoMat_dcomplex itsBuffer;

  string   itsDipoleName;
  ifstream itsDipoleFile;
  vector<float> itsDipoleLoc;

  LoVec_dcomplex WH_AWE::steerv (double u, double v, LoVec_double px, LoVec_double py) ;
  LoVec_dcomplex WH_AWE::getWeights (LoVec_dcomplex B, LoVec_dcomplex d) ;
  LoVec_dcomplex WH_AWE::getWeights (LoMat_dcomplex B, LoVec_dcomplex d) ;
};
#endif
