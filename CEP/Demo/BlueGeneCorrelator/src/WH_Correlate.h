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

#ifndef BLUEGENEDEMO_WH_CORRELATE_H
#define BLUEGENEDEMO_WH_CORRELATE_H

#include <lofar_config.h>
#include <Common/lofar_complex.h>

#include <tinyCEP/WorkHolder.h>
#include <DH_CorrCube.h>
#include <DH_Vis.h>

#ifdef __BLRTS__
#include <mpi.h>
#endif

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
			 unsigned int channels);

  virtual ~WH_Correlate();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, 
				unsigned int channels);

  /// Make a fresh copy of the WH object.
  virtual WH_Correlate* make (const string& name);

  /// Initialize MPI environment 
  virtual void preprocess();

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump();

private:
  static const int itsNelements = NSTATIONS;  // number of stations/inputs
  static const int itsNsamples  = SAMPLES;    // number of time samples to integrate over
  static const int itsNchannels = CHANNELS;   // number of selected freq. channels to correlate

  int myrank;

  complex<float> signal[itsNelements][itsNsamples][itsNchannels] ;
  complex<float> corr  [itsNelements][itsNelements][itsNchannels] ;
   
  /// Forbid copy constructor.
  WH_Correlate (const WH_Correlate&);

  /// Forbid assignment.
  WH_Correlate& operator= (const WH_Correlate&);

  // main correlator routine
  void correlator_core(complex<float> signal[NSTATIONS][SAMPLES], 
		       complex<float> corr[NSTATIONS][NSTATIONS]);

  // correlator core (unrolled)
  void correlator_core_unrolled(complex<float> s[NSTATIONS][SAMPLES],
  				complex<float> c[NSTATIONS][NSTATIONS]);

  void master();
  void slave(const int rank);
    
  int itsFBW; // frequency bandwidth of the DH_Beamlet 

  /// The input is assumed to be a matrix of N*M
  /// N = the number of inputs (either antennas or stations)
  /// M = the number of discreet samples to integrate over. This may 
  ///     be frequency channels or time samples.
  ///     M is defined to be 1000

};

}

#endif
