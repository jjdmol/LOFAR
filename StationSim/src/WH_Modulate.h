// WH_Modulate.h: interface for the WH_Modulate class.
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

#ifndef STATIONSIM_WH_MODULATE_H
#define STATIONSIM_WH_MODULATE_H

#include <BaseSim/WorkHolder.h>
#include <StationSim/DH_SampleR.h>
#include <StationSim/Modulate.h>
#include <StationSim/DataGenConfig.h>


class WH_Modulate : public WorkHolder 
{
public:

  WH_Modulate (int nin, 
			   int nout, 
			   string mod_type, 
			   double carrier_freq,
			   double samp_freq, 
			   double opt, 
			   double amp, 
			   int window_size = 32);

  virtual ~WH_Modulate ();

  /// Make a fresh copy of the WH object.
  virtual WH_Modulate* make (const string& name) const;

  virtual void process ();
  virtual void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  virtual DH_SampleR* getInHolder (int channel);

  /// Retrieve a pointer to the output data holder for the given channel
  virtual DH_SampleR* getOutHolder (int channel);

private:

  DH_SampleR** itsInHolders;
  DH_SampleR** itsOutHolders;

  int           itsWindowSize;
  int           itsPos;
  double        itsCarrierFreq;
  double        itsSampFreq;
  double        itsOpt;
  double        itsAmp;
  double        itsTc;
  double        itsPhi;
  double        itsP;
  double        itsp;
  string        itsModType;
  LoVec_double  itsInputBuffer;
  LoVec_double  itsOutputBuffer;
};


#endif
