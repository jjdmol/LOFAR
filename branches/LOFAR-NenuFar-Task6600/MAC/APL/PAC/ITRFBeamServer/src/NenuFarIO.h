//#  NenuFarIO.h: Implements all IO with the NenuFar Server.
//#
//#  Copyright (C) 2002-2014
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
//#  $Id: $

#ifndef BS_NENUFARIO_H_
#define BS_NENUFARIO_H_

// \file NenuFarIO.h
// Implements all IO with the NenuFar Server.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Control.h>
#include "NenuFarAdmin.h"

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace BS {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;

// \addtogroup package
// @{

class NenuFarIO : public GCFTask
{
public:
	explicit NenuFarIO (NenuFarAdmin*	nnfAdmin);
	~NenuFarIO();

	// NenuFarIO& operator=(const NenuFarIO& that);

	// for operator <<
	ostream& print (ostream& os) const
	{	return (os); }

private:
	// During the initial state all connections with the other programs are made.
	GCFEvent::TResult connect2server	   (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult monitorAdmin		   (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult tranceiveAdminChange (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult finish_state		   (GCFEvent& event, GCFPortInterface& port);

	// Copying is not allowed
	NenuFarIO(const NenuFarIO&	that);

	//# --- Datamembers ---
	string	itsNCUhostname;		// hostname of the NCU system
	int		itsNCUportnr;		// portnumber the NCU system listens on
	double	itsReconInterval;	// how often to try to reconnect

	int		itsIOtimeout;		// how long to wait for answers for the NCU

	NenuFarAdmin*			itsBeams;
	NenuFarAdmin::BeamInfo	itsCurrentBeam;
	GCFTimerPort*			itsTimerPort;
	GCFTCPPort*				itsNCUport;
	bool					itsIOerror;
};

//# --- Inline functions ---

// ... example
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const NenuFarIO& aNenuFarIO)
{	
	return (aNenuFarIO.print(os));
}


// @}
  } // namespace BS
} // namespace LOFAR

#endif
