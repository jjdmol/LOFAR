//#  WH_WeightDetermination.h:
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

#ifndef STATIONSIM_WH_WeightDetermination_H
#define STATIONSIM_WH_WeightDetermination_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/ArrayConfig.h>
#include <StationSim/DataGenConfig.h>
#include <Common/Lorrays.h>

class WH_WeightDetermination: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_WeightDetermination (const string& name,
						  unsigned int nin, 
						  unsigned int nout,
						  unsigned int nant, 
						  string s,
						  double phi,
						  double theta);

  virtual ~WH_WeightDetermination();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_WeightDetermination* make (const string& name) const;

  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  // This Workholder has no inputs. This member is not functional.
  virtual DataHolder* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleC* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_WeightDetermination (const WH_WeightDetermination&);

  /// Forbid assignment.
  WH_WeightDetermination& operator= (const WH_WeightDetermination&);

  LoVec_dcomplex steerv (double u, double v, LoVec_double px, LoVec_double py) ;

  // OutHolders
  DH_SampleC** itsOutHolders; 

  unsigned int itsNrcu;
  ArrayConfig itsArray;
  double itsPhi;
  double itsTheta;
};
#endif
