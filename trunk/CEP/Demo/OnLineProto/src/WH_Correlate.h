//# WH_Correlate.h: 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef ONLINEPROTO_WH_CORRELATE_H
#define ONLINEPROTO_WH_CORRELATE_H

#include <lofar_config.h>

#include "CEPFrame/WorkHolder.h"
#include "OnLineProto/DH_CorrCube.h"
#include "OnLineProto/DH_Vis.h"
#include <ACC/ParameterSet.h>

#include <blitz/blitz.h>
#include <blitz/array.h>

namespace LOFAR
{

/**
   TBW
*/

class WH_Correlate: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  explicit WH_Correlate (const string& name,
			 unsigned int channels,
			 const ParameterSet& ps);

  virtual ~WH_Correlate();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, 
				unsigned int channels,
				const ParameterSet& ps);

  /// Make a fresh copy of the WH object.
  virtual WH_Correlate* make (const string& name);

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  /// Forbid copy constructor.
  WH_Correlate (const WH_Correlate&);

  /// Forbid assignment.
  WH_Correlate& operator= (const WH_Correlate&);

  // main correlator routine
  void correlator_core(blitz::Array<complex<float>, 2>& signal,
		       blitz::Array<complex<float>, 2>& corr);

  // correlator core (unrolled)
  void correlator_core_unrolled(blitz::Array<complex<float>, 2>& s,
				blitz::Array<complex<float>, 2>& c);
    
  int itsFBW; // frequency bandwidth of the DH_Beamlet 

  /// The input is assumed to be a matrix of N*M
  /// N = the number of inputs (either antennas or stations)
  /// M = the number of discreet samples to integrate over. This may 
  ///     be frequency channels or time samples.
  ///     M is defined to be 1000

    const int itsNelements;  // number of stations/inputs
    const int itsNitems;     // number of frequency channels * number of time samples
    const ParameterSet itsPS;

};

}

#endif
