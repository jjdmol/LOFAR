//#  FeedbackService.h: Pass key-value pairs to the database.
//#
//#  Copyright (C) 2015
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

#ifndef FEEDBACK_SERVICE_H_
#define FEEDBACK_SERVICE_H_

// \file FeedbackService.h
// Pass key-value pairs to the database.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <MessageBus/MsgBus.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace Feedback {

using   MACIO::GCFEvent;
using   GCF::TM::GCFTimerPort;
using   GCF::TM::GCFPort;
using   GCF::TM::GCFPortInterface;
using   GCF::TM::GCFTask;

// \addtogroup package
// @{


class FeedbackService : public GCFTask;
{
public:
	FeedbackService();
	explicit FeedbackService (one_parameter);
	virtual/*?*/ ~FeedbackService();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	GCFEvent::TResult	connect2OTDB_state(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult	operational_state (GCFEvent& e, GCFPortInterface& p);

	// Copying is not allowed
	FeedbackService(const FeedbackService&	that) {};
	FeedbackService& operator=(const FeedbackService& that) {};

	//# --- Datamembers ---
	GCFTimerPort*		itsTimer;			// generic timer
	OTDBConnection*		itsOTDBconn;		// connection with OTDB
	MultiBus			itsMultiBus;		// session with Qpid.

};

//# --- Inline functions ---

// ... example
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const FeedbackService& aFeedbackService)
{	
	return (c.print(os));
}


// @}
  } // namespace Feedback
} // namespace LOFAR

#endif
