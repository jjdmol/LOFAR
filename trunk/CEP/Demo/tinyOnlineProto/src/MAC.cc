//  MAC.cc:
//
//  Copyright (C) 2000, 2001
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
//////////////////////////////////////////////////////////////////////

// OnLineProto specific include
#include <tinyOnlineProto/MAC.h>

namespace LOFAR
{
   MAC::MAC ()
     {
	c = 299792480.0;
	w_e = 2 * pi / (24 * 3600);

	itsDeclination = 0;
	itsStartHourangle = 0;
	itsChannelBandwidth = 1000;       //Hz
	itsNumberOfBeamlets = NBEAMLETS;
	itsBeamletSize = BFBW;
	itsTotalBandwidth = NBEAMLETS*BFBW*itsChannelBandwidth; //Hz
	itsLOfrequency = 224000000;
	itsNumberOfStations = NSTATIONS;
	itsFrequencies.resize (itsNumberOfBeamlets);
	itsFrequencies = blitz::tensor::i *itsBeamletSize*itsChannelBandwidth+itsLOfrequency;

	itsStations[0] = (Station*)new Station (0, 0.0, 0.0, 0.0);
	itsStations[1] = (Station*)new Station (1, 350000.0, 0.0, 0.0);
	for (int i = 2; i < itsNumberOfStations; i++) {
	  itsStations[i] = (Station*)new Station (i, 0.0, 0.0, 0.0);
	}
     }

   
   MAC::MAC (const MAC& m)
     {
	c = m.c;
	w_e = m.w_e;

	itsDeclination = m.itsDeclination;
	itsStartHourangle = m.itsStartHourangle;
	itsChannelBandwidth = m.itsChannelBandwidth; 
	itsBeamletSize = m.itsBeamletSize;
	itsTotalBandwidth = m.itsTotalBandwidth;
	itsLOfrequency = m.itsLOfrequency;
	itsNumberOfStations = m.itsNumberOfStations;
	itsNumberOfBeamlets = m.itsNumberOfBeamlets;
	itsFrequencies.resize(itsNumberOfBeamlets);
	itsFrequencies = blitz::tensor::i *itsBeamletSize*itsChannelBandwidth+itsLOfrequency;

	itsStations[0] = (Station*)new Station (0, 0.0, 0.0, 0.0);
	itsStations[1] = (Station*)new Station (1, 350000.0, 0.0, 0.0);
	for (int i = 2; i < itsNumberOfStations; i++) {
	  itsStations[i] = m.itsStations[i];
	}
     }
      
   MAC::~MAC()
     {
     }
}// namespace LOFAR
