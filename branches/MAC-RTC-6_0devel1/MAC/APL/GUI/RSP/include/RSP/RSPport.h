//#  RSPport.h: one line description
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_RSP_RSPPORT_H
#define LOFAR_RSP_RSPPORT_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/Net/Socket.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/WGSettings.h>

namespace LOFAR {
  using GCF::TM::GCFEvent;
  using RSP_Protocol::WGSettings;

  namespace RSP {

// \addtogroup RSP
// @{

//# Forward Declarations
//class forward;

// Description of class.
class RSPport
{
public:
	// Construct a port with a connection to a RSP driver on the given 
	// hostmachine.
	RSPport(string	aHostname);
	~RSPport();

	// Get the boardsstatus of 1 or more RSPboards.
	vector<BoardStatus> getBoardStatus(uint32	RCUmask);

	// With 'setWaveformSettings' the waveform generators on the digital
	// boards can be programmed. 
	// Mode: 0 = off , 1 = on
	// Frequency <= 100 Mhz (100e6)
	// Amplitude <= 2.1e9   (2^31)
	bool setWaveformSettings(uint32		RCUmask,
							 uint32		mode,
							 double		frequency,
							 uint8		phase,
							 uint32		amplitude);

	// Get the settings of the waveform generators.
	vector<struct WGSettings::WGRegisterType> 
						getWaveformSettings(uint32		RCUmask);

	// GetSubbandStats returns the signalstrength in each subband.
	vector<double> 	getSubbandStats(uint32	RCUmask);

	// GetBeamletStats returns the signalstrength in each beamlet.
	vector<double> 	getBeamletStats(uint32	RCUmask);

	// Control commands
	bool	setFilter	(uint32		RCUmask, int32		filterNr);
	bool	sendClear	(uint32		RCUmask);
	bool	sendReset	(uint32		RCUmask);
	bool	sendSync	(uint32		RCUmask);

	// get info of the environment of the RSPdriver
	uint32	getNrRCUs()			const	{ return (itsNrRCUs);	}
	uint32	getNrRSPboards()	const	{ return (itsNrRSPboards);	}
	uint32	getMaxRSPboards()	const	{ return (itsMaxRSPboards);	}

private:
	// Copying is not allowed
	RSPport();
	RSPport (const RSPport& that);
	RSPport& operator= (const RSPport& that);

	void 		send   (GCFEvent*	anEvent);
	GCFEvent&	receive();

	bool	sendRSUControl (uint32 RCUmask, uint32	controlType);

	//# Datamembers
	uint16		itsPort;
	string		itsHost;
	Socket*		itsSocket;
	uint32		itsNrRCUs;
	uint32		itsNrRSPboards;
	uint32		itsMaxRSPboards;
};

// @}

  } // namespace RSP
} // namespace LOFAR

#endif
