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
#include "OnLineProto/MAC.h"

namespace LOFAR
{
   MAC::MAC ()
     {
	c = 299792480;
	w_e = 2 * pi / (24 * 3600);

	itsIntegrationTime = 0.001;
	itsDeclination = 0;
	itsStartHourangle = 0;
	itsChannelBandwidth = 1000;       
	itsFrequencies.resize (itsBeamletSize * itsNumberOfBeamlets);
	itsFrequencies = tensor::i;
	itsBeamletSize = 256;
	itsTotalBandwidth = 32768000;
	itsLOfrequency = 20000000;
	itsNumberOfStations = 2;
	itsNumberOfBeamlets = 128;

	for (int i = 0; i < itsNumberOfStations; i++) {
	  itsStations[i] = new Station (i, 0, 0, 0);
	}
     }
   
   MAC::MAC (const MAC& m);
     {
	c = m.c;
	w_e = m.w_e;

	m.itsIntegrationTime = itsIntegrationTime;           
	m.itsDeclination = itsDeclination;
	m.itsStartHourangle = itsStartHourangle;
	m.itsChannelBandwidth = itsChannelBandwidth;       
	m.itsFrequencies = itsFrequencies;
	m.itsBeamletSize = itsBeamletSize;
	m.itsTotalBandwidth = itsTotalBandwidth;
	m.itsLOfrequency = itsLOfrequency;
	m.itsNumberOfStations = itsNumberOfStations;
	m.itsNumberOfBeamlets = itsNumberOfBeamlets;

	for (int i = 0; i < itsNumberOfStations; i++) {
	  itsStations[i] = m.itsStation[i];
	}
     }
      
   MAC::~MAC()
     {
     }
}// namespace LOFAR
