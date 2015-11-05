/*
 * neighboursolution.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 23, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/neighboursolution.cpp $
 *
 */

#include <vector>
#include "neighboursolution.h"

NeighbourSolution::NeighbourSolution() {
}

NeighbourSolution::~NeighbourSolution() {
}


NeighbourSolution & NeighbourSolution::operator= (const SchedulerDataBlock &rhs) {
	if (this != &rhs) {
		*(static_cast<SchedulerDataBlock *>(this)) = rhs;
	}
	return *this;
}

NeighbourSolution & NeighbourSolution::operator= (const NeighbourSolution &rhs) {
	if (this != &rhs) {
		*(static_cast<SchedulerDataBlock *>(this)) = rhs;
	}
	return *this;
}

void NeighbourSolution::setChangedTasks(const std::vector<Task> & changedTasks) {
	this->changedTasks = changedTasks;
	calculateDeltaPenalty();
}

void NeighbourSolution::calculateDeltaPenalty() {
	int penalty = 0;
	for (std::vector<Task>::iterator it = changedTasks.begin(); it != changedTasks.end(); ++it) {
		penalty += it->getPenalty();
	}
}

