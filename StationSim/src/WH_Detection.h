//#  WH_Detection.h:
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

#ifndef STATIONSIM_WH_DETECTION_H
#define STATIONSIM_WH_DETECTION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/DH_SampleR.h>
#include <aips/Arrays/Matrix.h>
/* #include <StationSim/DH_Flag.h> */
/* #include <StationSim/DH_Threshold.h> */


/**
   This WorkHolder analyses the complex data buffer.
*/

class WH_Detection: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_Detection(const string& name, unsigned nout,
	       unsigned int nrcu, unsigned int nsub1);

  virtual ~WH_Detection();

  /// Static function to create an object.
  static WorkHolder* construct(const string& name, int ninput, int noutput,
			       const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_Detection* make(const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  virtual DataHolder* getInHolder(int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DH_SampleR* getOutHolder(int channel);

private:
  /// Forbid copy constructor.
  WH_Detection(const WH_Detection&);

  /// Forbid assignment.
  WH_Detection& operator= (const WH_Detection&);

  /// Pointer to the array of input DataHolders.
  DH_SampleC itsInHolder;
  /// Pointer to the array of output DataHolders.
  DH_SampleR** itsOutHolders;

  /// Length of buffers.
  int itsNrcu;
  int itsNsub1;

  // A data holder to transport the threshold from the calibration to the detection part
  DH_SampleR itsThreshold;
  Matrix<DH_SampleR::BufferType> itsFlags;
};


#endif
