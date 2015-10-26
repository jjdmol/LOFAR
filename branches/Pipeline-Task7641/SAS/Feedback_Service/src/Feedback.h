//#  Feedback.h: Pass key-value pairs to the database.
//#
//#  Copyright (C) 2015
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef FEEDBACK_SERVICE_H_
#define FEEDBACK_SERVICE_H_

// \file Feedback.h
// Pass key-value pairs to the database.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <OTDB/OTDBconnection.h>
#include <MessageBus/FromBus.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace SAS {

using   MACIO::GCFEvent;
using   GCF::TM::GCFTimerPort;
using   GCF::TM::GCFPort;
using   GCF::TM::GCFPortInterface;
using   GCF::TM::GCFTask;
using   OTDB::OTDBconnection;

// \addtogroup package
// @{


class Feedback : public GCFTask
{
public:
	Feedback();
	~Feedback();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	GCFEvent::TResult	connect2OTDB_state(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult	operational_state (GCFEvent& e, GCFPortInterface& p);
	bool passKVpairsToOTDB(int obsID, const string&	content);

	// Copying is not allowed
	Feedback(const Feedback&	that);
	Feedback& operator=(const Feedback& that);

	//# --- Datamembers ---
	GCFTimerPort*		itsTimer;			// generic timer
	OTDBconnection*		itsOTDBconn;		// connection with OTDB
	FromBus*			itsMsgQueue;		// session with Qpid.

};

// @}
  } // namespace SAS
} // namespace LOFAR

#endif
