//#  WH_Analysis.h:
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

#ifndef STATIONSIM_WH_ANALYSIS_H
#define STATIONSIM_WH_ANALYSIS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleR.h>
#include <StationSim/DH_SampleC.h>


/**
   This WorkHolder analyses the complex data buffer.
*/

class WH_Analysis: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_Analysis (const string& name, unsigned nout,
	       unsigned int nx, unsigned int ny,
	       double threshold);

  virtual ~WH_Analysis();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_Analysis* make (const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DH_SampleC* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleR* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_Analysis (const WH_Analysis&);

  /// Forbid assignment.
  WH_Analysis& operator= (const WH_Analysis&);


  /// Pointer to the array of input DataHolders.
  DH_SampleC itsInHolder;
  /// Pointer to the array of output DataHolders.
  DH_SampleR** itsOutHolders;

  /// Length of buffers.
  int itsNx;
  int itsNy;
  double itsThreshold;
};


#endif
