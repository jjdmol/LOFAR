// WH_PhaseShift.h: interface for the WH_PhaseShift class.
//
//  Copyright (C) 2000,2001,2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef STATIONSIM_WH_PHASESHIFT_H
#define STATIONSIM_WH_PHASESHIFT_H

#include <Common/Lorrays.h>
#include <BaseSim/WorkHolder.h>
#include <DataGen/DH_SampleR.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/DataGenConfig.h>
#include <StationSim/PhaseShift.h>
#include <StationSim/FFTW.h>


class WH_PhaseShift : public WorkHolder 
{
public:

  WH_PhaseShift (int inputs, 
				 int outputs, 
				 int nrcu,
				 DataGenerator * dg_config, 
				 int nfft, 
				 int source,
				 int windowsize);

  virtual ~WH_PhaseShift ();

  // Make a fresh copy of the WH object.
  virtual WH_PhaseShift* make (const string& name) const;

  virtual void process ();
  virtual void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  virtual DH_SampleR* getInHolder (int channel);

  /// Retrieve a pointer to the output data holder for the given channel
  virtual DH_SampleC* getOutHolder (int channel);

private:
  DH_SampleR** itsInHolders;
  DH_SampleC** itsOutHolders;

  int                  itsNrcu;
  int                  itsPos;
  int                  itsNfft;
  int                  itsSource;
  int                  itsPrevWindowSize;
  int                  itsCount;
  DataGenerator*       itsConfig;
  FFTW::Plan           itsForwardPlan;
  FFTW::Plan           itsInversePlan;
  LoVec_double         itsInputBuffer;
  LoVec_double         itsFreqShift;
  LoMat_dcomplex       itsOutputBuffer;
};


#endif
