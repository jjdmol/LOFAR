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
#include <Common/Lorrays.h>


class WH_STA: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_STA (const string& name,
	  unsigned int nin, 
	  unsigned int nout,
	  unsigned int nant,
	  unsigned int maxnrfi,
	  unsigned int buflength
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
  virtual DH_SampleC* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_STA (const WH_STA&);

  /// Forbid assignment.
  WH_STA& operator= (const WH_STA&);

  /// Calculate a steer vector

  /// In- and OutHolders
  DH_SampleC** itsInHolders;
  DH_SampleC** itsOutHolders; 

  /// Length of buffers.
  unsigned int itsNrcu;
  unsigned int itsMaxRFI;
  unsigned int itsBufLength;

  LoMat_dcomplex itsBuffer;
  LoMat_dcomplex itsSigBuffer;
  LoVec_dcomplex itsSnapshot;
  unsigned int itsCurPos;
};
#endif
