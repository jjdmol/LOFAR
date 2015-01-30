//#  Feedback.cc: one_line_description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Feedback/Feedback.h>

namespace LOFAR {
  namespace Feedback {

static Feedback*	thisFeedback = 0;

//
// Feedback()
//
Feedback::Feedback() :
	GCFTask		((State)&Feedback::connect2OTDB_state, "FeedbackService"),
	itsTimerPort(0),
	itsOTDBConn (0)
{
	// Log who we are.
	LOG_INFO(Version::getInfo<@@@Version>("FeedbackService"));

	// attach timer
	itsTimer = new GCFTimerPort(*this, "TimerPort");
	ASSERTSTR(itsTimer, "Cannot allocate timer");

	// get the name of the queues to monitor
	vector<string>	queuenames = globalParameterSet()->getStringVector("FeedbackQueuenames");
	ASSERTSTR(!queuenames.empty(), "Queuenames not specified in label 'FeedbackQueuenames'");

	// connect to bussystem
	itsMultiBus = MultiBus(queuenames[0]);
	for (int i = 1; i < queuenames.size(); ++i) {
		itsMultibus.add(queuenames[i]);
	}

	// redirect signal handlers
	thisFeedback = this;
	signal (SIGINT,  Feedback::sigintHandler);   // ctrl-c
	signal (SIGTERM, Feedback::sigintHandler);   // kill
}

//
// ~Feedback()
//
Feedback::~Feedback()
{}

//
// sigintHandler(signum)
//
void Feedback::sigintHandler(int signum)
{   
	LOG_INFO (formatString("SIGINT signal detected (%d)",signum));
    
	if (thisFeedback) {
		thisFeedback->finish();
	}
}

//
// finish
//
void Feedback::finish()
{   
	GCFScheduler::instance()->stop();
}



//
//	connect2OTDB_state(event, port);
//
GCFEvent::TResult Feedback::connect2OTDB_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect2OTDB:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_INIT:	break;

	case F_ENTRY: 
	case F_TIMER: {
		// Try to setup the connection with the database
		string	confFile = globalParameterSet()->getString("OTDBconfFile", "SASGateway.conf");
		ConfigLocator	CL;
		string	filename = CL.locate(confFile);
		LOG_DEBUG_STR("Trying to read database information from file " << filename);
		ParameterSet	otdbconf;
		otdbconf.adoptFile(filename);
		string database = otdbconf.getString("SASGateway.OTDBdatabase");
		string dbhost   = otdbconf.getString("SASGateway.OTDBhostname");
		itsOTDBconn = new OTDBconnection("paulus", "boskabouter", database, dbhost);
		if (!itsOTDBconn->connect()) {
			LOG_FATAL_STR("Cannot connect to database " << database << " on machine " << dbhost << ", retry in 10 seconds");
			itsTimer->setTimer(10.0);
			return (GCFEvent::HANDLED);
		}
		LOG_INFO_STR("Connected to database " << database << " on machine " << dbhost);
		TRAN(Feedback::operational_state):
	} break;

	default:
		LOG_DEBUG_STR("connect2OTDB: not handled:" << eventName(event));
		break;
	} // switch
	
	return (GCFEvent::HANDLED);
}

//
//	operational_state(event, port);
//
GCFEvent::TResult Feedback::operational_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: 
	case F_TIMER:  {
		// wait (blocking) for next message
		Message msg = itsMultiBus.getMessage();
		LOG_DEBUG_STR(..);

		if (!itsOTDBconn->connect()) {
			LOG_ERROR("Lost connection with OTDB, starting reconnect cycle");
			delete itsOTDBconn;
			itsOTDBconn = 0;
			itsMultiBus.nack();
			TRAN (Feedback::connect2OTDB_state);
			return (GCFEvent::HANDLED);
		}

		// Yeah, still connected.
		string	content = msg.content();
		if (passKVpairsToOTDB(content)) {
			itsMultiBus.ack();
			LOG_DEBUG_STR(...);
		}
		else {
			itsMultibus.reject();
			LOG_ERROR_STR(...);
		}
		itsTimer->setTimer(0.0);
	} break;

	default:
		LOG_DEBUG_STR("operational: not handled:" << eventName(event));
		break;
	} // switch

	return (GCFEvent::HANDLED);
}


//
// passKVpairsToOTDB()
//
// Handler for procssing the messages that are received on the bus.
//
bool Feedback::passKVpairsToOTDB(const string&	content)
{
	int			obsID(getObservationNr(getName()));	// TODO
	TreeValue   tv(itsOTDBconn, obsID);

	// read parameterset
	ParameterSet	metadata;
	metadata.adoptString(content);

	// Loop over the parameterset and send the information to the SAS database
	// During the transition phase from parameter-based to record-based storage in OTDB the
	// nodenames ending in '_' are implemented both as parameter and as record.
	ParameterSet::iterator		iter = metadata.begin();
	ParameterSet::iterator		end  = metadata.end();
	while (iter != end) {
		string	key(iter->first);	// make destoyable copy
		rtrim(key, "[]0123456789");
//		bool	doubleStorage(key[key.size()-1] == '_');
		bool	isRecord(iter->second.isRecord());
		//   isRecord  doubleStorage
		// --------------------------------------------------------------
		//      Y          Y           store as record and as parameters
		//      Y          N           store as parameters
		//      N          *           store parameter
		if (!isRecord) {
			LOG_DEBUG_STR("BASIC: " << iter->first << " = " << iter->second);
			tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
		}
		else {
//			if (doubleStorage) {
//				LOG_DEBUG_STR("RECORD: " << iter->first << " = " << iter->second);
//				tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
//			}
			// to store is a node/param values the last _ should be stipped of
			key = iter->first;		// destroyable copy
//			string::size_type pos = key.find_last_of('_');
//			key.erase(pos,1);
			ParameterRecord	pr(iter->second.getRecord());
			ParameterRecord::const_iterator	prIter = pr.begin();
			ParameterRecord::const_iterator	prEnd  = pr.end();
			while (prIter != prEnd) {
				LOG_DEBUG_STR("ELEMENT: " << key+"."+prIter->first << " = " << prIter->second);
				tv.addKVT(key+"."+prIter->first, prIter->second, ptime(microsec_clock::local_time()));
				prIter++;
			}
		}
		iter++;
	}
	LOG_INFO_STR(metadata.size() << " metadata values send to SAS");
	return (true);
}



  } // namespace Feedback
} // namespace LOFAR
