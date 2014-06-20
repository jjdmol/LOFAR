//
//  setObjectState.cc : Small utility to allow scripts to set an objectstate.
//
//  Copyright (C) 2011
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
//  $Id: tDPservice.cc 10538 2007-10-03 15:04:43Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/lofar_fstream.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DPservice.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RTDBCommon/RTDButilities.h>
#include "setObjectState.h"

int	gError = 0;

namespace LOFAR {
  using namespace GCF;
  using namespace GCF::TM;
  using namespace GCF::PVSS;
  using namespace RTDB;
  namespace APL {
    namespace RTDBCommon {


mainTask::mainTask(const string& name) : 
	GCFTask((State)&mainTask::setState, name),
	itsTimerPort(0),
	itsDPservice(0)
{
	LOG_DEBUG_STR("mainTask(" << name << ")");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);
}

//
// destructor
//
mainTask::~mainTask()
{
	if (itsDPservice) {
		delete itsDPservice;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// setState (event, port)
//
GCFEvent::TResult mainTask::setState(GCFEvent& event, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("setState:" << eventName(event));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: 
		break;

	case F_INIT: {
		if (GCFScheduler::_argc == 4) {
			itsUpdateCount = updateState(GCFScheduler::_argv[1], GCFScheduler::_argv[2], atoi(GCFScheduler::_argv[3])) ? 1 : 0;
		}
		else {
			ifstream		inputFile;
			inputFile.open(GCFScheduler::_argv[2], ifstream::in);
			if (!inputFile) {
				cout << "Unable to open file " << GCFScheduler::_argv[2] << endl;
				gError = -2;
				GCFScheduler::instance()->stop();
			}

			if (inputFile.eof()) {
				cout << "Inputfile is empty" << endl;
				gError = -3;
				GCFScheduler::instance()->stop();
			}

			itsUpdateCount = 0;
			string	line;
			do {
				getline (inputFile, line);	// read a line
				if (line.empty()) {			// skip empty lines
					continue;
				}
				vector<char>	DPname(line.size());
				int				value;
				if (sscanf(line.c_str(), "%s %d", &DPname[0], &value) != 2) {
					cout << "Wrong format: " << line << endl;
					gError = -3;
					// try other lines...
				}
				else {
					itsUpdateCount += updateState(GCFScheduler::_argv[1], &DPname[0], value) ? 1 : 0;
				}
			} while(inputFile);
			inputFile.close();
		}
		itsTimerPort->setTimer(3.0); // Don't wait forever on DP_SET
	}
	break;

	case F_TIMER:
		gError = -1;
		GCFScheduler::instance()->stop();
	break;

	case DP_SET:
		if (--itsUpdateCount <= 0) {
			gError = 0;
			GCFScheduler::instance()->stop();
		}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

bool mainTask::updateState(const string&	who,
						   const string&	objectName,
						   uint32			newStateIndex)
{
	// check newState value
	if (newStateIndex >= RTDB_OBJ_STATE_NR_OF_VALUES) {
		LOG_ERROR_STR(newStateIndex << " is not a legal objectStateIndex, " << objectName << " will be left unchanged");
		cout << newStateIndex << " is not a legal objectStateIndex, " << objectName << " will be left unchanged" << endl;
		return (false);
	}

	// the DP we must write to has three elements. Make a vector of the names and the new
	// values and write them to the database at once.
	vector<string>		fields;
	vector<GCFPValue*>	values;
	fields.push_back("DPName");
	fields.push_back("stateNr");
	fields.push_back("message");
	fields.push_back("force");
	values.push_back(new GCFPVString(objectName+".status.state"));
	values.push_back(new GCFPVInteger(objectStateIndex2Value(newStateIndex)));
	values.push_back(new GCFPVString(who));
	values.push_back(new GCFPVBool(true));

	LOG_DEBUG_STR(who << " is setting " << objectName << " to " << objectStateIndex2Value(newStateIndex));

	PVSSresult	result = itsDPservice->setValue("__navObjectState", fields, values, 0.0, true);
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR("Call to PVSS for setObjectState returned: " << result);
	}

	// free allocated GCFValues.
	for (int i = values.size()-1 ; i >= 0; i--) {
		delete values[i];
	}

	return (result == SA_NO_ERROR);
}

    } // namepsace RTDBCommon
  } // namspace APL
} // namespace LOFAR

// -------------------- MAIN --------------------
using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::APL::RTDBCommon;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char* argv[])
{
	if (argc != 3 && argc != 4) {
		cout << "Syntax: " << argv[0] << " who datapoint stateNr" << endl;
		cout << "  OR    " << argv[0] << " who filename" << endl;
		cout << "  who        : Identification who changes the state, name of program or user" << endl;
		cout << "  datapoint  : PVSS datapoint name including database name" << endl;
		cout << "  stateNr    : 0 - Off" << endl;
		cout << "               1 - Operational" << endl;
		cout << "               2 - Maintenance" << endl;
		cout << "               3 - Test" << endl;
		cout << "               4 - Suspicious" << endl;
		cout << "               5 - Broken" << endl;
		return (1);
	}

	if (argc == 4 && objectStateIndex2Value(atoi(argv[3])) < 0) {
		cout << "stateNr " << argv[3] << " is not a legal value." << endl;
		return (2);
	}

	gError = 0;
	TM::GCFScheduler::instance()->init(argc, argv);

	mainTask update("setObjectStateTask");  
	update.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return (gError);
}
