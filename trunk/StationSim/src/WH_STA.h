//#  WH_STA.h:
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
//#  Chris Broekema, january 2003.
//#

#ifndef STATIONSIM_WH_STA_H
#define STATIONSIM_WH_STA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/DH_SampleR.h>
#include <Common/Lorrays.h>

class WH_STA: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_STA (const string& name,
		  int nin, 
		  int nout,
		  int nant,
		  int maxnrfi,
		  int buflength,
		  int alg
		  );
  virtual ~WH_STA();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_STA* make (const string& name) const;

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
  virtual DataHolder* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_STA (const WH_STA&);

  /// Forbid assignment.
  WH_STA& operator= (const WH_STA&);

  LoMat_dcomplex CreateContigeousBuffer (const LoMat_dcomplex& aBuffer, int pos);
  LoMat_dcomplex TransposeMatrix (const LoMat_dcomplex& aMat);
  LoVec_double ReverseVector (const LoVec_double& aVec);
  LoMat_dcomplex ReverseMatrix (const LoMat_dcomplex& aMat, int dim);

  /// In- and OutHolders
  DH_SampleC** itsInHolders;
  DH_SampleC** itsOutHolders; 
  DH_SampleR   itsNumberOfRFIs;

  /// Length of buffers.
  int itsNrcu;
  int itsMaxRFI;
  int itsBufLength;
	
  LoMat_dcomplex itsBuffer;
  int itsPos;
  LoMat_dcomplex itsEvectors;
  LoVec_double   itsEvalues;
  LoMat_dcomplex itsAcm;
  int            itsAlg;
};
#endif
