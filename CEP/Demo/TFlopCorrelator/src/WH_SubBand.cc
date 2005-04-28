//#  WH_SubBand.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <WH_SubBand.h>

#include <complex>

using namespace LOFAR;

WH_SubBand::WH_SubBand(const string& name) {

  // Need input: number of filter taps
  itsNtaps = 8;
  itsNcoeff = 8;

  // Initialize the delay line
  delayLine = new FilterType[2*itsNtaps]; 
  memset(delayLine,0,2*itsNtaps*sizeof(FilterType));
  delayPtr = delayLine;

  // Need input: filter coefficients
  coeffPtr = new FilterType[itsNcoeff];
  for (int j = 0; j < itsNcoeff; j++) {
    __real__ coeffPtr[j] = (j + 1);
  }

}

WH_SubBand::~WH_SubBand() {
}

WorkHolder* WH_SubBand::construct(const string& name) {
  return new WH_SubBand(name);
}

WH_SubBand* WH_SubBand::make(const string& name) {
  return new WH_SubBand(name);
}

void WH_SubBand::process() {
  
}

void WH_SubBand::dump() {
}

void WH_SubBand::adjustDelayPtr() { 
  for (int i = itsNtaps - 2; i >= 0; i--) {
    delayLine[i+1] = delayLine[i];
  }

}
