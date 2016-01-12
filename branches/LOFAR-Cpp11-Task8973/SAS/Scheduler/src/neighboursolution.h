/*
 * neighboursolution.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 23, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/neighboursolution.h $
 *
 */

#ifndef NEIGHBOURSOLUTION_H_
#define NEIGHBOURSOLUTION_H_

#include "schedulerdatablock.h"
#include "task.h"

class NeighbourSolution : public SchedulerDataBlock {
public:
	NeighbourSolution();
	virtual ~NeighbourSolution();

	NeighbourSolution & operator= (const SchedulerDataBlock &);
	NeighbourSolution & operator= (const NeighbourSolution &);

    void addChangedTask(Task &task) { changedTasks.push_back(task); }
    void setChangedTasks(const std::vector<Task> & changedTasks);
    int getDeltaPenalty(void) const { return deltaPenalty; }

private:
	void calculateDeltaPenalty();

private:
	std::vector<Task> changedTasks;
	int deltaPenalty;
};

#endif /* NEIGHBOURSOLUTION_H_ */
