// WH_Modulate.cc: implementation of the WH_Modulate class.
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

#include <DataGen/WH_Modulate.h>
#include <Common/Debug.h>


WH_Modulate::WH_Modulate (int nin, 
						  int nout, 
						  string mod_type, 
						  double carrier_freq, 
						  double samp_freq, 
						  double opt, 
						  double amp, 
						  int window_size)
: WorkHolder     (nin, nout, "aWorkHolder", "WH_Modulate"),
  itsWindowSize  (window_size),
  itsPos         (0),  
  itsCarrierFreq (carrier_freq),
  itsSampFreq    (samp_freq),
  itsOpt         (opt),
  itsAmp         (amp),
  itsTc          (samp_freq / carrier_freq),
  itsPhi         (0),
  itsP           (0),
  itsp           (0),
  itsModType     (mod_type)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nin > 0) {
    itsInHolders = new DH_SampleR* [nin];
  }
  if (nout > 0) {
    itsOutHolders = new DH_SampleR* [nout];
  }
  // Create the input DH-s.
  char str[8];

  for (int i = 0; i < nin; i++) {
    sprintf (str, "%d", i);
    itsInHolders[i] = new DH_SampleR (string ("in_") + str, 1, 1);
  }
  // Create the output DH-s.
  if (nin == 0) {
    nin = 1;
  }
  for (int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleR (string ("out_") + str, 1, 1);
  }
  itsInputBuffer.resize (itsWindowSize);
  itsOutputBuffer.resize (itsWindowSize);
}


WH_Modulate::~WH_Modulate ()
{
  for (int ch = 0; ch < getInputs (); ch++) {
    delete itsInHolders[ch];
  }
  for (int ch = 0; ch < getOutputs (); ch++) {
    delete itsOutHolders[ch];
  }
}


WH_Modulate* WH_Modulate::make (const string &) const
{
  return new WH_Modulate (getInputs (), 
						  getOutputs (), 
						  itsModType,
						  itsCarrierFreq, 
						  itsSampFreq, 
						  itsOpt, 
						  itsAmp,
						  itsWindowSize);
}


void WH_Modulate::process ()
{
  DH_SampleR::BufferType * bufin = itsInHolders[0]->getBuffer ();
  itsInputBuffer (itsPos) = bufin[0];
  itsPos = (itsPos + 1) % itsInputBuffer.size ();

  if (itsPos == 0) {
    // find out what modulationscheme has to be applied to the signal
    // modulate the signal
    if (itsModType == "AMDSB") {
      itsOutputBuffer = modulate::amdsb (itsInputBuffer, 
										 itsCarrierFreq,
										 itsSampFreq, 
										 itsPhi) * itsAmp;
	} else if (itsModType == "AMDSB_TC") {
      itsOutputBuffer = modulate::amdsb_tc (itsInputBuffer, 
											itsCarrierFreq,
											itsSampFreq, 
											itsPhi, 
											itsOpt) * itsAmp;
    } else if (itsModType == "AMSSB") {
      itsOutputBuffer = modulate::amssb (itsInputBuffer, 
										 itsCarrierFreq,
										 itsSampFreq, 
										 itsPhi) * itsAmp;
	} else if (itsModType == "FM") {
      itsOutputBuffer = modulate::fm (itsInputBuffer, 
									  itsCarrierFreq, 
									  itsSampFreq,
									  itsOpt, 
									  itsPhi) * itsAmp;
	} else if (itsModType == "PM") {
      itsOutputBuffer = modulate::pm (itsInputBuffer, 
									  itsCarrierFreq, 
									  itsSampFreq,
									  itsOpt, 
									  itsPhi) * itsAmp;
	}

    // Calculate the proper phase for the carrier frequency
    itsP = itsTc - itsp;
    while (itsP < itsWindowSize) {
      itsP += itsTc;
	}
    itsp = itsTc - (itsP - itsWindowSize);
    itsPhi = itsp / itsSampFreq;
  }

  if (getOutputs () > 0) {
    for (int i = 0; i < getOutputs (); i++) {
      getOutHolder (i)->getBuffer ()[0] = itsOutputBuffer (itsPos);
    }
  }
}

void WH_Modulate::dump () const
{
}


DH_SampleR* WH_Modulate::getInHolder (int channel)
{
  return itsInHolders[channel];
}

DH_SampleR* WH_Modulate::getOutHolder (int channel)
{
  return itsOutHolders[channel];
}
