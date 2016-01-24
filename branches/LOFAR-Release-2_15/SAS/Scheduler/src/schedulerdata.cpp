/*
 * schedulerdata.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 26, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulerdata.cpp $
 *
 */

#ifdef DEBUG_SCHEDULER
#include <iostream>
#endif
#include "schedulerdata.h"
#include "schedulerdatablock.h"
#include "lofar_scheduler.h"
#include "task.h"
#include "Controller.h"

SchedulerData::SchedulerData() {
}

SchedulerData::~SchedulerData() {
}

void SchedulerData::cleanup(void) {
	// create a fresh data block
	itsData.cleanup();
	itsUndoStack.clear(); // TODO: check if this calls the destructor of every SchedulerDataBlock and cleans up the task objects
	itsRedoStack.clear();
	// create the stations if any were defined in the scheduler settings
	updateStations();
}

bool SchedulerData::undo(bool store_redo) {
	if (!itsUndoStack.empty()) {
		if (store_redo) {
			itsRedoStack.push_back(itsData);
		}
		itsData = itsUndoStack.back();
		itsUndoStack.pop_back();
		return true;
	}
	else return false; // no more undo levels
}

bool SchedulerData::redo(void) {
	if (!itsRedoStack.empty()) {
		itsUndoStack.push_back(itsData);
		itsData = itsRedoStack.back();
		itsRedoStack.pop_back();
		return true;
	}
	else return false; // no more undo levels
}

void SchedulerData::setDataHeaders(std::vector<std::string> const &headers) {
	itsDataHeaders.clear();
	for (std::vector<std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it){
		itsDataHeaders.push_back(*it);
	}
}
