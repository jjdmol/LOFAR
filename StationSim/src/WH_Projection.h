//#  WH_Projection.h:
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

#ifndef STATIONSIM_WH_Projection_H
#define STATIONSIM_WH_Projection_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/DH_SampleR.h>
#include <StationSim/DataGenConfig.h>
#include <Common/Lorrays.h>


class WH_Projection: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_Projection (const string& name,
				 unsigned int nin, 
				 unsigned int nout,
				 unsigned int nant,
				 unsigned int maxnrfi,
		                 bool tapstream);
  
  virtual ~WH_Projection();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_Projection* make (const string& name) const;
  
  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  /// The first one is the sampled data.
  /// The second one is the selected subbands.
  virtual DataHolder* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleC* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Projection (const WH_Projection&);

  /// Forbid assignment.
  WH_Projection& operator= (const WH_Projection&);

  /// In- and OutHolders
  DH_SampleC** itsInHolders;
  DH_SampleC** itsOutHolders; 
  DH_SampleR itsNumberOfRFIs;
  DH_SampleC itsRFISources;

  /// Length of buffers.
  unsigned int itsNrcu;
  unsigned int itsMaxRFI;
  int itsDetectedRFIs;
  LoVec_dcomplex itsWeight;
  LoMat_dcomplex itsV;
  LoVec_dcomplex itsA;
  bool itsTapStream;

  LoVec_dcomplex WH_Projection::getWeights (LoVec_dcomplex B, LoVec_dcomplex d) ;
  LoVec_dcomplex WH_Projection::getWeights (LoMat_dcomplex V, LoVec_dcomplex a) ;
  LoVec_dcomplex WH_Projection::mv_mult(LoMat_dcomplex A, LoVec_dcomplex B);
  LoVec_dcomplex WH_Projection::vm_mult (const LoVec_dcomplex& A, const LoMat_dcomplex& B); 
};
#endif
