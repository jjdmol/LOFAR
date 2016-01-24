/*
 * schedulerdatablock.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 12, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulerdatablock.cpp $
 *
 */

#include <vector>
#include <map>
#include <deque>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <QDateTime>
#include <QDataStream>
#include "lofar_scheduler.h"
#include "schedulerdatablock.h"
#include "schedulersettings.h"
#include "task.h"
#include "astrodatetime.h"
#include "Controller.h"
#include "station.h"
#include "SASConnection.h"
#include "calibrationpipeline.h"
#include "imagingpipeline.h"
#include "pulsarpipeline.h"
#include "longbaselinepipeline.h"
using std::map;
using std::string;
using std::stringstream;
using std::vector;
using std::deque;
using std::max;
using std::min;

SchedulerDataBlock::SchedulerDataBlock() :
	itsSaveRequired(false), itsUploadRequired(false), nullTask(0) {
}

SchedulerDataBlock::SchedulerDataBlock(const SchedulerDataBlock &other) {
	*this = other;
}

SchedulerDataBlock::~SchedulerDataBlock() {
	for (unscheduledTasksDeque::iterator i = unscheduledTasks.begin(); i != unscheduledTasks.end(); ++i)
		delete *i;
	// do the same for scheduledTasks
	for (scheduledTasksMap::iterator i = scheduledTasks.begin(); i != scheduledTasks.end(); ++i)
		delete i->second;
	//and for reservations
	for (reservationsMap::iterator i = itsReservations.begin(); i != itsReservations.end(); ++i)
		delete i->second;
	// do the same for inactiveTasks
	for (inActiveTasksMap::iterator i = inactiveTasks.begin(); i != inactiveTasks.end(); ++i)
		delete i->second;

}

QDataStream& operator<<(QDataStream &out, const SchedulerDataBlock &data) {
	if (out.status() == QDataStream::Ok) {

		out << (quint32)data.itsUsedTaskIDs.size(); // number of taskIDs
		for (std::vector<unsigned>::const_iterator it = data.itsUsedTaskIDs.begin(); it != data.itsUsedTaskIDs.end(); ++it) {
            out << (quint32) *it;
		}

		out << (quint32)data.unscheduledTasks.size(); // number of unscheduled tasks
		for (unscheduledTasksDeque::const_iterator it = data.unscheduledTasks.begin(); it != data.unscheduledTasks.end(); ++it) {
            data.writeTaskToStream(out, *it); // writes a task object
		}

		out << (quint32)data.scheduledTasks.size();
		for (scheduledTasksMap::const_iterator it = data.scheduledTasks.begin(); it != data.scheduledTasks.end(); ++it) {
            out << (quint32) it->first;
            data.writeTaskToStream(out, it->second); // write a task object
		}

		out << (quint32)data.itsPipelines.size();
		for (pipelinesMap::const_iterator it = data.itsPipelines.begin(); it != data.itsPipelines.end(); ++it) {
            out << (quint32) it->first;
            data.writeTaskToStream(out, it->second); // write a task object
		}

		out << (quint32)data.itsReservations.size();
		for (reservationsMap::const_iterator it = data.itsReservations.begin(); it != data.itsReservations.end(); ++it) {
            out << (quint32) it->first;
            data.writeTaskToStream(out, it->second); // write a task object
		}

		out << (quint32)data.inactiveTasks.size();
		for (inActiveTasksMap::const_iterator it = data.inactiveTasks.begin(); it != data.inactiveTasks.end(); ++it) {
            out << (quint32) it->first;
            data.writeTaskToStream(out, it->second); // write a task object
		}

		out << (quint32)data.errorTasks.size();
		for (errorTasksMap::const_iterator it = data.errorTasks.begin(); it	!= data.errorTasks.end(); ++it) {
			out << (quint32) it->first // task ID
			    << (quint32) it->second.size();
			for (std::vector<data_headers>::const_iterator dit = it->second.begin(); dit != it->second.end(); ++dit) {
				out << (quint16) *dit; // error column
			}
		}

		out << (quint32)data.itsStations.size();
		for (stationsMap::const_iterator it = data.itsStations.begin(); it != data.itsStations.end(); ++it) {
			out << (quint32) it->first << it->second; // write Station object
		}

		out << (quint32) data.itsPenalty;
	}
	return out;
}

QDataStream& operator>>(QDataStream &in, SchedulerDataBlock &data) {
	if (in.status() == QDataStream::Ok) {
        quint32 nrOfObjects(0), nrOfObjects2(0), ID(0);
        quint16 column(0);
		data.unscheduledTasks.clear();
		data.scheduledTasks.clear();
		data.errorTasks.clear();
		data.itsStations.clear();
		data.itsPipelines.clear();
		data.inactiveTasks.clear();
		data.itsUsedTaskIDs.clear();

		in >> nrOfObjects; // read the number of used task IDs
		for (quint32 i = 0; i < nrOfObjects; ++i) {
			in >> ID;
			data.itsUsedTaskIDs.push_back(ID);
		}

		in >> nrOfObjects; // read the number of unscheduled tasks
		for (quint32 i = 0; i < nrOfObjects; ++i) {
            data.unscheduledTasks.push_back(data.readTaskFromStream(in));
		}

        in >> nrOfObjects; // read the number of scheduled tasks
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            in >> ID;
            data.scheduledTasks[ID] = static_cast<StationTask *>(data.readTaskFromStream(in));
		}

        in >> nrOfObjects; // read the number of pipeline tasks
		for (quint32 i = 0; i < nrOfObjects; ++i) {
            in >> ID;
            data.itsPipelines[ID] = static_cast<Pipeline *>(data.readTaskFromStream(in));
		}

        in >> nrOfObjects; // read the number of reservation tasks
		for (quint32 i = 0; i < nrOfObjects; ++i) {
            in >> ID;
            data.itsReservations[ID] = static_cast<StationTask *>(data.readTaskFromStream(in));
		}

		if (Controller::itsFileVersion >= 3) {
            in >> nrOfObjects; // read the number of inactive tasks
			for (quint32 i = 0; i < nrOfObjects; ++i) {
                in >> ID;
                data.inactiveTasks[ID] = data.readTaskFromStream(in);
			}
		}

		in >> nrOfObjects; // read the number of error tasks
		for (quint32 i = 0; i < nrOfObjects; ++i) {
			in >> ID >> nrOfObjects2;
			for (quint32 j = 0; j < nrOfObjects2; ++j) {
				in >> column;// error column number
				data.errorTasks[ID].push_back((data_headers) column);
			}
		}

		in >> nrOfObjects; // read the number of stations
		Station station;
		for (quint32 i = 0; i < nrOfObjects; ++i) {
			in >> ID >> station; // read a station object
			data.itsStations[ID] = station;
		}

		in >> data.itsPenalty;
	}
	return in;
}

void SchedulerDataBlock::writeTaskToStream(QDataStream &out, Task *pTask) const {
    out << (quint8) pTask->getType();
    switch (pTask->getType()) {
    case Task::OBSERVATION:
        out << static_cast<Observation &>(*pTask);
        break;
    case Task::PIPELINE:
    {
        // TODO: OO code smell. Create a virtual static stream function on the pipeline classes
        Pipeline *pipe = static_cast<Pipeline *>(pTask);
        out << (quint8) pipe->pipelinetype();
        switch (pipe->pipelinetype()) {
        case PIPELINE_CALIBRATION:
            out << static_cast<CalibrationPipeline &>(*pTask);
            break;
        case PIPELINE_IMAGING:
            out << static_cast<ImagingPipeline &>(*pTask);
            break;
        case PIPELINE_PULSAR:
            out << static_cast<PulsarPipeline &>(*pTask);
            break;
        case PIPELINE_LONGBASELINE:
            out << static_cast<LongBaselinePipeline &>(*pTask);
            break;
        default:
            break;
        }
        break;
    }
    case Task::RESERVATION:
    case Task::MAINTENANCE:
        out << static_cast<StationTask &>(*pTask);
        break;
    case Task::SYSTEM:
        out << *pTask;
        break;
    default:
        break;
    }
}

Task *SchedulerDataBlock::readTaskFromStream(QDataStream &in) {
    Task *pTask(0);
    quint8 iVal;
    in >> iVal;
    Task::task_type type = static_cast<Task::task_type>(iVal);
    switch (type) {
    case Task::OBSERVATION:
        pTask = new Observation();
        in >> static_cast<Observation &>(*pTask); // read Observation properties into pTask
        break;
    case Task::PIPELINE:
        in >> iVal; // read the pipeline type
        switch (static_cast<pipelineType>(iVal)) {
        case PIPELINE_CALIBRATION:
            pTask = new CalibrationPipeline();
            in >> static_cast<CalibrationPipeline &>(*pTask); // read Calibration Pipeline properties into pTask
            break;
        case PIPELINE_IMAGING:
            pTask = new ImagingPipeline();
            in >> static_cast<ImagingPipeline &>(*pTask); // read Observation properties into pTask
            break;
        case PIPELINE_PULSAR:
            pTask = new PulsarPipeline();
            in >> static_cast<PulsarPipeline &>(*pTask); // read Pulsar Pipeline properties into pTask
            break;
        case PIPELINE_LONGBASELINE:
            pTask = new LongBaselinePipeline();
            in >> static_cast<LongBaselinePipeline &>(*pTask); // read Long-Baseline Pipeline properties into pTask
            break;
        default:
            pTask = new Pipeline();
            in >> static_cast<Pipeline &>(*pTask); // read Pipeline properties into pTask
        }
        break;
    case Task::RESERVATION:
    case Task::MAINTENANCE:
        pTask = new StationTask(type);
        in >> static_cast<StationTask &>(*pTask); // read StationTask properties into pTask
        break;
    case Task::SYSTEM:
        pTask = new Task();
        in >> *pTask; // read Task properties into pTask
        break;
    default:
        break;
    }
    return pTask;
}

SchedulerDataBlock & SchedulerDataBlock::operator=(const SchedulerDataBlock &rhs) {
	if (this != &rhs) {
		//unscheduled tasks
		if (!(unscheduledTasks.empty())) {
			for (unscheduledTasksDeque::iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
				delete *it;
				*it = 0;
			}
			unscheduledTasks.clear();
		}
		for (unscheduledTasksDeque::const_iterator it =	rhs.getUnscheduledTasks().begin(); it != rhs.getUnscheduledTasks().end(); ++it) {
			Task *pTask = new Task(*(*it));
			this->addTask(pTask, DONT_CHECK_FREE_ID); // the NO_CHECK skips the check to see if the ID is free. (we already know it is free)
		}

		// scheduled tasks
		if (!(scheduledTasks.empty())) {
			for (scheduledTasksMap::iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
				delete it->second;
				it->second = 0;
			}
			scheduledTasks.clear();
		}
		for (scheduledTasksMap::const_iterator it = rhs.getScheduledTasks().begin(); it != rhs.getScheduledTasks().end(); ++it) {
            Task *pTask = cloneTask(it->second);
            if (pTask) {
                scheduledTasks.insert(std::pair<unsigned, StationTask *>(it->second->getID(), static_cast<StationTask *>(pTask)));
            }
		}

		// pipeline tasks
		if (!(itsPipelines.empty())) {
			for (pipelinesMap::iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
				delete it->second;
				it->second = 0;
			}
			itsPipelines.clear();
		}
		for (pipelinesMap::const_iterator it = rhs.getPipelineTasks().begin(); it != rhs.getPipelineTasks().end(); ++it) {
            Task *pTask = cloneTask(it->second);
            if (pTask) {
                itsPipelines.insert(std::pair<unsigned, Pipeline *>(it->second->getID(), static_cast<Pipeline *>(pTask)));
            }
		}

		//reservations
		if (!(itsReservations.empty())) {
			for (reservationsMap::iterator it = itsReservations.begin(); it != itsReservations.end(); ++it) {
				delete it->second;
				it->second = 0;
			}
			itsReservations.clear();
		}
		for (reservationsMap::const_iterator it = rhs.getReservations().begin(); it != rhs.getReservations().end(); ++it) {
            Task *pTask = cloneTask(it->second);
            if (pTask) {
                itsReservations.insert(std::pair<unsigned, StationTask *>(it->second->getID(), static_cast<StationTask *>(pTask)));
            }
		}

		// inactive tasks
		if (!(inactiveTasks.empty())) {
			for (inActiveTasksMap::iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++it) {
				delete it->second;
				it->second = 0;
			}
			inactiveTasks.clear();
		}
		for (inActiveTasksMap::const_iterator it = rhs.getInactiveTasks().begin(); it != rhs.getInactiveTasks().end(); ++it) {
            Task *pTask = cloneTask(it->second);
            if (pTask) {
                inactiveTasks.insert(std::pair<unsigned, Task *>(it->second->getID(), pTask));
            }
		}

		itsStations = rhs.getStations();
		errorTasks = rhs.getErrorTasks();
		itsUsedTaskIDs = rhs.getUsedTaskIDs();
		itsStorage = rhs.getStorage();
		itsUsedTaskIDs = rhs.getUsedTaskIDs();
		itsExistingSASTaskIDs = rhs.getSASUsedTaskIDs();
		itsPenalty = rhs.getPenalty();
		itsSaveRequired = rhs.getSaveRequired();
		itsUploadRequired = rhs.getUploadRequired();
	}
	return *this;
}

// adds a new task to the list of unscheduled tasks
bool SchedulerDataBlock::addTask(Task *task, bool check_ID_free) {
    unsigned id(task->getID());
    if (check_ID_free) {
		if (!isTaskIDFree(task->getID())) {
			debugWarn("sis","could not add task: ", task->getID(), " because that ID is already used.");
			return false;
		}
	}
    if (task->isPipeline()) {
        itsPipelines[id] = static_cast<Pipeline *>(task);
    }
    else if (task->isMaintenance() || task->isReservation()) {
        itsReservations[id] = static_cast<StationTask *>(task);
    }
    else {
        unscheduledTasks.push_back(task);
    }
    itsUsedTaskIDs.push_back(id);
    return true;
}

void SchedulerDataBlock::addUsedTaskID(unsigned taskID) {
	if (isTaskIDFree(taskID)) {
		itsUsedTaskIDs.push_back(taskID);
	}
}

void SchedulerDataBlock::addUsedTaskIDs(const std::vector<unsigned> &taskIDs) {
	for (std::vector<unsigned>::const_iterator it = taskIDs.begin(); it != taskIDs.end(); ++it) {
		if (isTaskIDFree(*it)) {
			itsUsedTaskIDs.push_back(*it);
		}
	}
}

void SchedulerDataBlock::updateUsedTaskIDs(void) {
	itsUsedTaskIDs.clear();
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
			itsUsedTaskIDs.push_back(it->first);
	}
	for (reservationsMap::const_iterator it = itsReservations.begin(); it != itsReservations.end(); ++it) {
			itsUsedTaskIDs.push_back(it->first);
	}
	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
			itsUsedTaskIDs.push_back((*it)->getID());
	}
	for (inActiveTasksMap::const_iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++it) {
			itsUsedTaskIDs.push_back(it->first);
	}
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
			itsUsedTaskIDs.push_back(it->first);
	}
}

void SchedulerDataBlock::removeUsedTaskID(unsigned task_id) {
	std::vector<unsigned>::iterator erit = find(itsUsedTaskIDs.begin(), itsUsedTaskIDs.end(), task_id);
	if (erit != itsUsedTaskIDs.end()) {
		itsUsedTaskIDs.erase(erit);
		return;
	}

    debugWarn("sis","could not remove task id: ", task_id, " because it was not in the list of used task IDs.");
}

unsigned SchedulerDataBlock::getNewTaskID(bool override_SAS_taskIDs) const {
	unsigned id;
	for (id = 1; id < MAX_UNSIGNED; ++id) {
		if (isTaskIDFree(id)) {
			if (!override_SAS_taskIDs) {
				if (isTaskIDFreeInSAS(id)) {
					return id;
				}
			}
			else return id;
		}
	}
	return 0;
}

/*
unsigned SchedulerDataBlock::getNewTaskID(const std::vector<unsigned> &exclude_IDs) const {
	for (unsigned id = 1; id < MAX_UNSIGNED; ++id) {
		if (find(exclude_IDs.begin(), exclude_IDs.end(), id) == exclude_IDs.end()) {
			if (isTaskIDFree(id))
				return id;
		}
	}
	return 0;
}
*/

bool SchedulerDataBlock::moveTaskToInactive(unsigned taskID) {
	for (unscheduledTasksDeque::iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		if ((*it)->getID() == taskID) {
			if ((*it)->getStatus() >= Task::FINISHED) {
                // move the task from unscheduled deque to inactive tasks map
				inactiveTasks.insert(inActiveTasksMap::value_type((*it)->getID(), (*it)));
				unscheduledTasks.erase(it);
				return true;
			}
			else return false;
		}
	}
	for (scheduledTasksMap::iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		if (it->second->getID() == taskID) {
			if (it->second->getStatus() >= Task::FINISHED) {
                // move the task from scheduled tasks map to inactive tasks map
				inactiveTasks.insert(inActiveTasksMap::value_type(it->second->getID(), it->second));
				it->second = 0;
				scheduledTasks.erase(it);
				return true;
			}
		}
	}
	for (pipelinesMap::iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		if (it->second->getID() == taskID) {
			if (it->second->getStatus() >= Task::FINISHED) {
                // move the task from pipelines tasks map to inactive tasks map
				inactiveTasks.insert(inActiveTasksMap::value_type(it->second->getID(), it->second));
				it->second = 0;
				itsPipelines.erase(it);
				return true;
			}
		}
	}
	return false;
}

void SchedulerDataBlock::printUsedTaskIDs(void) const {
	if (!itsUsedTaskIDs.empty()) {
		std::cout << "Task IDs in use: ";
		for (std::vector<unsigned>::const_iterator it = itsUsedTaskIDs.begin(); it != itsUsedTaskIDs.end()-1; ++it) {
			std::cout << *it << ",";
		}
		std::cout << itsUsedTaskIDs.back() << std::endl;
	}
	else {
		std::cout << "no task IDs in use" << std::endl;
	}
}


StationTask *SchedulerDataBlock::newReservation(unsigned task_id, bool override_SAS_taskIDs) {
	if ((task_id == 0) || (!isTaskIDFree(task_id))) { // reservation ID=zero means get a new ID
		task_id = getNewTaskID(override_SAS_taskIDs);
	}
	if (!override_SAS_taskIDs) {
		if (!isTaskIDFreeInSAS(task_id))
			return 0;
	}
    StationTask *r = new StationTask(task_id, Task::RESERVATION);
	itsReservations[task_id] = r;
	itsUsedTaskIDs.push_back(task_id);
	return r;
}

Observation *SchedulerDataBlock::newObservation(unsigned task_id, bool override_SAS_taskIDs) {
    Observation *pTask(0);
    task_id = getTaskID(task_id, override_SAS_taskIDs);
    if (task_id) {
        pTask = new Observation(task_id);
        unscheduledTasks.push_back(pTask);
        itsUsedTaskIDs.push_back(task_id);
    }
    return pTask;
}

Pipeline *SchedulerDataBlock::newPipeline(unsigned task_id, pipelineType type, bool override_SAS_taskIDs) {
    Pipeline *pTask(0);
    task_id = getTaskID(task_id, override_SAS_taskIDs);
    if (task_id) {
        switch (type) {
        case PIPELINE_CALIBRATION:
            pTask = new CalibrationPipeline(task_id);
            break;
        case PIPELINE_IMAGING:
            pTask = new ImagingPipeline(task_id);
            break;
        case PIPELINE_PULSAR:
            pTask = new PulsarPipeline(task_id);
            break;
        case PIPELINE_LONGBASELINE:
            pTask = new LongBaselinePipeline(task_id);
            break;
        default:
            pTask = new Pipeline(task_id);
            debugWarn("sis", "SchedulerDataBlock::newPipeline: Pipeline with task id: ", task_id, " is of unknown type.");
            break;
        }
        itsPipelines[task_id] = pTask;
        itsUsedTaskIDs.push_back(task_id);
    }
    return pTask;
}

unsigned SchedulerDataBlock::getTaskID(unsigned task_id, bool override_SAS_taskIDs) {
    if (task_id == 0) return getNewTaskID(override_SAS_taskIDs); // provided task_id = 0, generate a new task_id
    else if (isTaskIDFree(task_id)) return task_id; // provided task_id is is not used yet, use it.
    else return getNewTaskID(override_SAS_taskIDs); // generate a new ID
}

Task *SchedulerDataBlock::newTask(unsigned task_id, const OTDBtree &SAS_tree, bool override_SAS_taskIDs) {
    task_id = getTaskID(task_id, override_SAS_taskIDs);
    Task *t = new Task(task_id, SAS_tree);
	unscheduledTasks.push_back(t);
	itsUsedTaskIDs.push_back(task_id);
	return t;
}

Task *SchedulerDataBlock::newTask(unsigned task_id, bool override_SAS_taskIDs) {
    task_id = getTaskID(task_id, override_SAS_taskIDs);
    Task *t = new Task(task_id);
    unscheduledTasks.push_back(t);
    itsUsedTaskIDs.push_back(task_id);
    return t;
}

Task *SchedulerDataBlock::deleteTask(unsigned id, id_type IDtype, bool erase) {
    const Task *pTask(getTask(id, IDtype));
    Task * retTask(0);
    if (pTask) {
        unsigned task_id(pTask->getID());

        if (pTask->isScheduled()) {
            unscheduleTask(task_id); // moves the task to the unscheduled queue as well
        }
        removeUsedTaskID(task_id);

        // no need to search for the task in the scheduledTasksMap because the task is unscheduled now
        unscheduledTasksDeque::iterator uit = unscheduledTasks.begin();
        while(uit != unscheduledTasks.end()) {
            if ((*uit)->getID() == task_id) {
                if (erase) delete *uit; else retTask = *uit;
                unscheduledTasks.erase(uit);
                return retTask;
            }
            ++uit;
        }

        pipelinesMap::iterator pit = itsPipelines.find(task_id);
        if (pit != itsPipelines.end()) {
            if (erase) delete pit->second; else retTask = pit->second;
            itsPipelines.erase(pit);
            return retTask;
        }

        inActiveTasksMap::iterator it = inactiveTasks.find(task_id);
        if (it != inactiveTasks.end()) {
            if (it->second->isStationTask() && it->second->getStatus() >= Task::SCHEDULED) { // if status is greater or equal to scheduled it means that is has been planned on the stations
                removeTaskFromStations(static_cast<StationTask *>(it->second));
            }
            if (erase) delete it->second; else retTask = it->second;
            inactiveTasks.erase(it);
            return retTask;
        }

        reservationsMap::iterator rit = itsReservations.find(task_id);
        if (rit != itsReservations.end()) {
            if (erase) delete rit->second; else retTask = rit->second;
            itsReservations.erase(rit);
            return retTask;
        }
    }

    return 0;
}


// remove a task is inactive (finished, aborted, etc) from the stations
void SchedulerDataBlock::removeTaskFromStations(StationTask *task) {
	stationsMap::iterator ssit;
    const std::map<std::string, unsigned> &stations = task->getStations();
	for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
		ssit = itsStations.find(sit->second); // first check if the station still exists, the user may have deleted it with the settingsdialog
		if (ssit != itsStations.end()) {
            ssit->second.removeTask(task->getID());
		}
	}
}

void SchedulerDataBlock::unscheduleAll(void) {
	for (scheduledTasksMap::iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		unscheduledTasks.push_back(it->second);
		it->second->setStatus(Task::UNSCHEDULED);
	}
	scheduledTasks.clear();
	for (stationsMap::iterator sit = itsStations.begin(); sit != itsStations.end(); ++sit) {
		sit->second.removeAllTasks();
	}
}

/*
void SchedulerDataBlock::unscheduleAll(bool forced_unschedule) {
	if (forced_unschedule) {
		for (scheduledTasksMap::iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
			unscheduledTasks.push_back(it->second);
			it->second->setStatus(Task::UNSCHEDULED);
		}
		scheduledTasks.clear();
		for (stationsMap::iterator sit = itsStations.begin(); sit != itsStations.end(); ++sit) {
			sit->second.removeAllTasks();
		}
	}
	else { // forced_unschedule = false -> we may not unschedule tasks with their may_not_unschedule flag set (this automatically includes reservations)
		std::vector<unsigned> taskIDs;
		for (scheduledTasksMap::iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
			taskIDs.push_back(it->first);
		}
		for (std::vector<unsigned>::const_iterator it = taskIDs.begin(); it != taskIDs.end(); ++it) {
			unscheduleTask(*it, false);
		}
	}
}
*/

bool SchedulerDataBlock::unscheduleTask(unsigned task_id) {
	Task *pTask(0);
	scheduledTasksMap::iterator it = scheduledTasks.find(task_id);
	if (it != scheduledTasks.end()) {
		pTask = it->second;
		scheduledTasks.erase(it);
		pTask->setStatus(Task::UNSCHEDULED);
		unscheduledTasks.push_back(pTask);
	}
	else {
		pipelinesMap::iterator it = itsPipelines.find(task_id);
		if (it != itsPipelines.end()) {
			pTask = it->second;
			pTask->setStatus(Task::UNSCHEDULED);
		}
		else {
			reservationsMap::iterator it = itsReservations.find(task_id);
			if (it != itsReservations.end()) {
				pTask = it->second;
				//			debugInfo("sis", "Reservation: ", task_id, " is being unscheduled");
				pTask->setStatus(Task::ON_HOLD);
			}
		}
	}
	if (pTask) {
		if (pTask->isStationTask()) {
			stationsMap::iterator ssit;
            const std::map<std::string, unsigned> &stations =  static_cast<StationTask *>(pTask)->getStations();
			for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
				ssit = itsStations.find(sit->second); // first check if the station still exists, the user may have deleted it with the settingsdialog
                if (ssit != itsStations.end()) {
					ssit->second.removeTask(task_id);
				}
			}
		}
		return true;
	}
	else {
//		debugWarn("sis", "Could not unschedule task: ", task_id, " because it is not scheduled");
		return false;
	}
}

std::vector<unsigned> SchedulerDataBlock::moveTask(Task *pTask, const AstroDateTime &new_start, bool unschedule_conflicting_tasks) {
    std::vector<unsigned> conflictIDs;

    AstroDateTime new_end = new_start + pTask->getDuration();

    if (pTask->isStationTask()) {
        // go by all Stations in this task to see if we have to kick out conflicting tasks
        // check if any of the conflicting tasks are SCHEDULED
        // only PRESCHEDULED tasks may be thrown out of the schedule
        StationTask *psTask(static_cast<StationTask *>(pTask));
        const std::map<std::string, unsigned> &stations =  psTask->getStations();
        for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
            conflictIDs = itsStations[sit->second].getTaskswithinTimeSpan(new_start, new_end);
            if (unschedule_conflicting_tasks) {
                for (std::vector<unsigned>::const_iterator tit = conflictIDs.begin(); tit != conflictIDs.end(); ++tit) {
                    const Task *pTask = getTask(*tit);
                    if (pTask->getStatus() == Task::SCHEDULED) { // only PRESCHEDULED tasks may be unscheduled
                        return conflictIDs; // we cannot continue with the move because of conflicting tasks that cannot be unscheduled
                    }
                }
                // apparently there are no conflicting fixed tasks, continue with the unscheduling of conflicting tasks
                for (std::vector<unsigned>::const_iterator uit = conflictIDs.begin(); uit != conflictIDs.end(); ++uit) {
                    unscheduleTask(*uit);
                }
            }
            else if (!conflictIDs.empty()) {
                return conflictIDs;
            }
        } // else leave conflicting task in the schedule and continue with the move

        // update the stations bookkeeping
        stationsMap::iterator sit;
        for (std::map<std::string, unsigned>::const_iterator sidit = stations.begin(); sidit != stations.end(); ++sidit) {
            if ((sit = itsStations.find(sidit->second)) != itsStations.end()) {
                sit->second.moveTask(pTask->getID(), pTask->getScheduledStart(), pTask->getScheduledEnd());
            }
        }
    }

    // do the actual move
    pTask->setScheduledStart(new_start);
    pTask->syncStartStopTimes(); // also update the start and stop time in the meta-data of OTDBtree

    return conflictIDs;
}

// function to check if the task is still within predecessor range
// return value:
// -1: both the lower limit and upper limit have been reached (no movement possible)
// 0: clear of both limits can move left (earlier) or right (later)
// 1: lower limit reached, can move right (later)
// 2: upper limit reached, can move left (earlier)
short int SchedulerDataBlock::withinPredecessorsRange(const Task *task, const AstroDateTime &new_start) const {
	const IDvector &predecessors(task->getPredecessors());
//	const AstroDateTime &start(task->getScheduledStart()), &end(task->getScheduledEnd());
	bool uppper_limit_reached(false), lower_limit_reached(false);
	for (IDvector::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
		const Task *predecessor(getTask(it->second, it->first));

//		const AstroDateTime &pred_start(predecessor->getScheduledStart());
		const AstroDateTime &pred_end(predecessor->getScheduledEnd());
		if (!uppper_limit_reached) {
			if (new_start > pred_end + task->getPredecessorMaxTimeDif()) uppper_limit_reached = true;
		}
		if (!lower_limit_reached) {
			if (new_start < pred_end + task->getPredecessorMaxTimeDif()) lower_limit_reached = true;
		}
		if (lower_limit_reached && uppper_limit_reached) return -1;
	}
	if (lower_limit_reached && uppper_limit_reached) return -1;
	else if (lower_limit_reached) return 1;
	else if (uppper_limit_reached) return 2;
	else return 0;
}

bool SchedulerDataBlock::tryMoveTaskToAdjacentDay(unsigned task_id, bool unschedule_conflicting_tasks) {
    std::pair<bool, std::vector<unsigned> > result;
	result.first = false;
	Task *task = getTaskForChange(task_id);
	AstroDateTime new_start;
	if (task->hasPredecessors()) {
		if (task->getShiftDirection() == SHIFT_RIGHT) {
			new_start = task->getScheduledStart().addDays(1);
			int w(withinPredecessorsRange(task,new_start)); // test ranges to predecessors
			if (w == 2) { // upper limit reached
				task->setShiftDirection(SHIFT_LEFT); // task would move out of predecessor range, change direction
				new_start = task->getScheduledStart().subtractDays(1);
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
			else if ((w == 0) || (w == 1)) { // no limit or only lower limit reached
				if ((new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay()) ||
						(new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
                    return false;
				}
				if ((new_start < task->getFirstPossibleDateTime()) || (new_start > task->getLastPossibleDateTime())) {
                    return false;
				}
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
            else return false; // no movement possible
		}
		else { // shift left
			new_start = task->getScheduledStart().subtractDays(1);
			int w(withinPredecessorsRange(task,new_start)); // test ranges to predecessors
			if (w == 1) { // upper limit reached
				task->setShiftDirection(SHIFT_RIGHT); // task would move out of predecessor range, change direction
				new_start = task->getScheduledStart().addDays(1);
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
			else if ((w == 0) || (w == 2)) { // no limit or only upper limit reached
				if ((new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay()) ||
						(new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
                    return false;
				}
				if ((new_start < task->getFirstPossibleDateTime()) || (new_start > task->getLastPossibleDateTime())) {
                    return false;
				}
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
            else return false; // no movement possible
		}
	}
	// task doesn't have predecessors
	if (task->getShiftDirection() == SHIFT_RIGHT) {
		new_start = task->getScheduledStart().addDays(1);
		if ((new_start > task->getLastPossibleDateTime()) || (new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
			task->setShiftDirection(SHIFT_LEFT);
			new_start = task->getScheduledStart().subtractDays(1);
		}
	}
	else { // shift left
		new_start = task->getScheduledStart().subtractDays(1);
		if ((new_start < task->getFirstPossibleDateTime()) || (new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay())) {
			task->setShiftDirection(SHIFT_RIGHT);
			new_start = task->getScheduledStart().addDays(1);
		}
	}
    return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
}

bool SchedulerDataBlock::tryShiftTaskWithinDay(unsigned task_id, bool unschedule_conflicting_tasks) {
	Task *task = getTaskForChange(task_id);
	AstroDateTime new_start;
	if (task->getShiftDirection() == SHIFT_RIGHT) {
		new_start = task->getScheduledEnd() + task->getDuration();
		if (new_start.getDate() == task->getScheduledEnd().getDate()) { // still on same day?
			int w(withinPredecessorsRange(task,new_start)); // test ranges to predecessors
			if (w == 2) { // upper limit reached
				task->setShiftDirection(SHIFT_LEFT); // task would move out of predecessor range, change direction
				new_start = task->getScheduledStart() - task->getDuration();
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
			else if ((w == 0) || (w == 1)) { // no limit or only lower limit reached
				if ((new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay()) |
						(new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
                    return false;
				}
				if ((new_start < task->getFirstPossibleDateTime()) || (new_start > task->getLastPossibleDateTime())) {
                    return false;
				}
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
            else return false; // no movement possible
		}
		else {
			task->setShiftDirection(SHIFT_LEFT);
			new_start = task->getScheduledStart() - task->getDuration();
            return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
		}
	}
	else { // shift left
		new_start = task->getScheduledStart() - task->getDuration();
		if (new_start.getDate() == task->getScheduledEnd().getDate()) { // still on same day?
			int w(withinPredecessorsRange(task,new_start)); // test ranges to predecessors
			if (w == 1) { // upper predecessor limit reached
				task->setShiftDirection(SHIFT_RIGHT); // task would move out of predecessor range, change direction
				new_start = task->getScheduledStart() + task->getDuration();
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
			else if ((w == 0) || (w == 2)) { // no limit or only upper limit reached
				if ((new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay()) ||
						(new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
                    return false;
				}
				if ((new_start < task->getFirstPossibleDateTime()) || (new_start > task->getLastPossibleDateTime())) {
                    return false;
				}
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
            else return false; // no movement possible
		}
		else {
			task->setShiftDirection(SHIFT_RIGHT);
			new_start = task->getScheduledStart() + task->getDuration();
            return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
		}
	}
}

/*
bool SchedulerDataBlock::tryShiftTask(unsigned task_id, bool unschedule_conflicting_tasks) {
	Task *task = getTaskForChange(task_id);
	if (task->hasPredecessor()) {
		unsigned predTaskID = task->getPredecessor();
		const Task *predecessor = getTask(predTaskID);
		if (getTask(predTaskID)->getStatus() == Task::SCHEDULED) {
			if (task->getShiftDirection() == SHIFT_RIGHT) {
				if (task->getScheduledEnd() > predecessor->getScheduledEnd() + task->getPredecessorMaxTimeDif()) {
					task->setShiftDirection(SHIFT_LEFT); // task would move out of predecessor range, change direction
					return shiftTask(task_id, SHIFT_LEFT, unschedule_conflicting_tasks);
				}
			}
			else { // shift left
				if (task->getScheduledStart() - task->getDuration() < predecessor->getScheduledEnd() + task->getPredecessorMinTimeDif()) {
					task->setShiftDirection(SHIFT_RIGHT); // task would move to close to predecessor, change direction
					return shiftTask(task_id, SHIFT_RIGHT, unschedule_conflicting_tasks);
				}
			}
		}
	}
	return shiftTask(task->getID(), task->getShiftDirection(), unschedule_conflicting_tasks);
}
*/
bool SchedulerDataBlock::shiftTask(unsigned task_id, bool unschedule_conflicting_tasks) {
	Task *task = getTaskForChange(task_id);
	AstroDateTime new_start;
	if (task->hasPredecessors()) {
		if (task->getShiftDirection() == SHIFT_RIGHT) {
			new_start = task->getScheduledStart() + task->getDuration();
			int w(withinPredecessorsRange(task,new_start)); // test ranges to predecessors
			if (w == 2) { // upper limit reached
				task->setShiftDirection(SHIFT_LEFT); // task would move out of predecessor range, change direction
				new_start = task->getScheduledStart() - task->getDuration();
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
			else if ((w == 0) || (w == 1)) { // no limit or only lower limit reached
				if ((new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay()) ||
						(new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
                    return false;
				}
				if ((new_start < task->getFirstPossibleDateTime()) || (new_start > task->getLastPossibleDateTime())) {
                    return false;
				}
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
            else return false; // no movement possible
		}
		else { // shift left
			new_start = task->getScheduledStart() - task->getDuration();
			int w(withinPredecessorsRange(task,new_start)); // test ranges to predecessors
			if (w == 1) { // upper predecessor limit reached
				task->setShiftDirection(SHIFT_RIGHT); // task would move out of predecessor range, change direction
				new_start = task->getScheduledStart() + task->getDuration();
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
			else if ((w == 0) || (w == 2)) { // no limit or only upper limit reached
				if ((new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay()) ||
						(new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
                    return false;
				}
				if ((new_start < task->getFirstPossibleDateTime()) || (new_start > task->getLastPossibleDateTime())) {
                    return false;
				}
                return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
			}
            else return false; // no movement possible
		}
	}
	// task doesn't have predecessors
	if (task->getShiftDirection() == SHIFT_RIGHT) {
		new_start = task->getScheduledStart() + task->getDuration();
		if ((new_start > task->getLastPossibleDateTime()) || (new_start > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
			task->setShiftDirection(SHIFT_LEFT);
			new_start = task->getScheduledStart() - task->getDuration();
		}
	}
	else { // shift left
		new_start = task->getScheduledStart() - task->getDuration();
		if ((new_start < task->getFirstPossibleDateTime()) || (new_start < Controller::theSchedulerSettings.getEarliestSchedulingDay())) {
			task->setShiftDirection(SHIFT_RIGHT);
			new_start = task->getScheduledStart() + task->getDuration();
		}
	}
    return (moveTask(task, new_start, unschedule_conflicting_tasks).empty());
}

bool SchedulerDataBlock::changeTaskStartTime(unsigned task_id, const AstroDateTime &new_start) {
	Task *pTask = getTaskForChange(task_id);
    if (pTask) {
        pTask->setScheduledStart(new_start);
        if (pTask->isStationTask()) {
            StationTask *psTask(static_cast<StationTask *>(pTask));
            const taskStationsMap &stations = psTask->getStations();
            for (taskStationsMap::const_iterator stit = stations.begin(); stit != stations.end(); ++stit) {
                stationsMap::iterator statit = itsStations.find(stit->second);
                if (statit != itsStations.end()) {
                    statit->second.moveTask(task_id, new_start, new_start + psTask->getDuration());
                }
            }
        }
		return true;
	}
	else return false;
}

bool SchedulerDataBlock::changeTaskEndTime(unsigned task_id, const AstroDateTime &new_end) {
	Task *pTask = getTaskForChange(task_id);
    if (pTask) {
        pTask->setScheduledEnd(new_end);
        if (pTask->isStationTask()) {
            StationTask *psTask(static_cast<StationTask *>(pTask));
            const taskStationsMap &stations = psTask->getStations();
            for (taskStationsMap::const_iterator stit = stations.begin(); stit != stations.end(); ++stit) {
                stationsMap::iterator statit = itsStations.find(stit->second);
                if (statit != itsStations.end()) {
                    statit->second.moveTask(task_id, new_end - psTask->getDuration(), new_end);
                }
            }
        }
        return true;
    }
    else return false;
}

bool SchedulerDataBlock::changeTaskDuration(unsigned task_id, const AstroTime &new_duration) {
    Task *pTask = getTaskForChange(task_id);
    if (pTask) {
        pTask->setDuration(new_duration);
        if (pTask->isStationTask()) {
            StationTask *psTask(static_cast<StationTask *>(pTask));
            const taskStationsMap &stations = psTask->getStations();
            AstroDateTime new_end(psTask->getScheduledStart() + new_duration);
            for (taskStationsMap::const_iterator stit = stations.begin(); stit != stations.end(); ++stit) {
                stationsMap::iterator statit = itsStations.find(stit->second);
                if (statit != itsStations.end()) {
                    statit->second.moveTask(task_id, psTask->getScheduledStart(), new_end);
                }
            }
        }
        return true;
    }
    else return false;
}

bool SchedulerDataBlock::changeTaskSchedule(unsigned task_id, const AstroDateTime &new_start, const AstroDateTime &new_end) {
	Task *pTask(0);
	scheduledTasksMap::iterator it = scheduledTasks.find(task_id);
	if (it != scheduledTasks.end()) {
		pTask = it->second;
	}
	else {
		reservationsMap::iterator it = itsReservations.find(task_id);
		if (it != itsReservations.end()) {
			pTask = it->second;
		}
	}
	if (pTask) {
		pTask->setScheduledStart(new_start);
		pTask->setScheduledEnd(new_end);
        if (pTask->isStationTask()) {
            StationTask *psTask(static_cast<StationTask *>(pTask));
            const taskStationsMap &stations = psTask->getStations();
            for (taskStationsMap::const_iterator stit = stations.begin(); stit != stations.end(); ++stit) {
                stationsMap::iterator statit = itsStations.find(stit->second);
                if (statit != itsStations.end()) {
                    statit->second.moveTask(task_id, new_start, new_end);
                }
            }
        }
		return true;
	}
	else return false; // task not found
}

bool SchedulerDataBlock::rescheduleAbortedTask(unsigned task_id, const AstroDateTime &start) {
	inActiveTasksMap::iterator it = inactiveTasks.find(task_id);
	if (it != inactiveTasks.end()) {
		Task * pTask = it->second;
		AstroDateTime new_start(start);
		if (findFirstOpportunity(pTask, new_start)) {
			// this is an aborted (inactiveTask) task and therefore it is not scheduled, so no need to unschedule it
			pTask->setScheduledStart(new_start);
			scheduleTask(pTask);
			pTask->setStatus(Task::PRESCHEDULED);
			return true;
		}
	}
	return false;
}


bool SchedulerDataBlock::rescheduleTask(unsigned task_id, const AstroDateTime &new_start) {
    size_t count(0);
    Task * pTask(0);
    scheduledTasksMap::const_iterator it = scheduledTasks.find(task_id);
    if (it != scheduledTasks.end()) {
        pTask = it->second;
    }
    else {
        reservationsMap::const_iterator rit = itsReservations.find(task_id);
        if (rit != itsReservations.end()) {
            pTask = rit->second;
        }
    }
    if (pTask) {
        if (pTask->isStationTask()) {
            StationTask *psTask(static_cast<StationTask *>(pTask));

            Task::task_type type = psTask->getType();
            if ((type == Task::RESERVATION) | (type == Task::MAINTENANCE)) {
                // a reservation or maintenance task is always rescheduled at the requested new_start time (even if it conflicts with other tasks)
                psTask->setScheduledStart(new_start);
                stationsMap::iterator sit;
                // go by all stations to reschedule the task in the stations their task lists
                const taskStationsMap &stations =  psTask->getStations();
                for (taskStationsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
                    sit = itsStations.find(it->second);
                    if (sit != itsStations.end()) {
                        sit->second.moveTask(task_id, psTask->getScheduledStart(), psTask->getScheduledEnd());
                    }
                }
                return true;
            }
            else { // try reschedule regular task
                const AstroTime &min_time_between_tasks = Controller::theSchedulerSettings.getMinimumTimeBetweenTasks();
                Station * pStation = 0;
                bool opening_found = true, found_mutual_opening = false;
                AstroDateTime first_opening(new_start), previous_opening;
                const taskStationsMap &stations =  psTask->getStations();
                vector<unsigned int> checkStations;
                // first remove the task from the stations So that its time slot is not occupied anymore
                stationsMap::iterator sit;
                for (taskStationsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
                    sit = itsStations.find(it->second);
                    if (sit != itsStations.end()) {
                        sit->second.removeTask(task_id);
                        checkStations.push_back(it->second);
                    }
                    else {
                        debugErr("sssis", "SchedulerDataBlock::rescheduleTask: Station ",
                                 it->first.c_str(), " in the list of task ", psTask->getID(), " which doesn't seem to exist!");
                    }
                }

                if (!checkStations.empty()) {
                    // now find a new mutual time slot on the stations
                    while (!found_mutual_opening && opening_found) {
                        count = 0;
                        for (vector<unsigned int>::const_iterator sit = checkStations.begin(); sit != checkStations.end(); ++sit) {
                            ++count;
                            previous_opening = first_opening;
                            if ((pStation = getStationForChange(*sit))) {
                                if (pStation->findFirstOpportunity(*psTask, first_opening, min_time_between_tasks)) { // if possible to schedule at this station then first_possible holds the possible start time
                                    if (first_opening != previous_opening) {
                                        if (psTask->hasPredecessors()) {
                                            if (withinPredecessorsRange(psTask,first_opening) != 0) {
                                                opening_found = false;
                                                holdNextTask(MAX_DISTANCE_PREDECESSOR);
                                                break;
                                            }
                                        }
                                        if (count != 1) {
                                            break; // starts again to ask all stations starting from the last found opening
                                        }
                                    }
                                    if (count == checkStations.size()) {
                                        found_mutual_opening = true;
                                    }
                                } else { // one of the stations didn't find a possible scheduling time, skip other stations
                                    opening_found = false;
                                    break; // break out for loop, don't ask other stations for schedule opening
                                }
                            }
                        }
                    }
                    if (found_mutual_opening && (psTask->getStations().size() > 0)) {
                        // schedule the task at the stations on its new position
                        psTask->setScheduledStart(first_opening);
                    }

                    // go by all stations to add the task again in the stations their task lists
                    for (vector<unsigned int>::const_iterator it = checkStations.begin(); it != checkStations.end(); ++it) {
                        sit = itsStations.find(*it);
                        sit->second.addTasktoStation(task_id, psTask->getScheduledStart(), psTask->getScheduledEnd());
                    }

                    if (found_mutual_opening && (psTask->getStations().size() > 0))
                        return true;
                    else
                        return false;
                }
                else return false; // no existing stations left in task list
            }
        }
    }
    return false;
}


bool SchedulerDataBlock::findFirstOpportunity(const Task *pTask, AstroDateTime &start_time, unsigned reservation_id) {
    if (pTask->isStationTask()) {
        const StationTask *psTask(static_cast<const StationTask *>(pTask));
        AstroDateTime first_opening(start_time), previous_opening;
        size_t count(0);
        const std::map<std::string, unsigned> &stations =  psTask->getStations();
        if (!stations.empty()) {
            Station * pStation = 0;
            while (1) {
                count = 0;
                for (std::map<std::string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
                    ++count;
                    previous_opening = first_opening;
                    if ((pStation = getStationForChange(sit->second))) {
                        if (pStation->findFirstOpportunity(*psTask, first_opening, Controller::theSchedulerSettings.getMinimumTimeBetweenTasks(), reservation_id)) { // if possible to schedule at this station then first_possible holds the possible start time
                            if (first_opening != previous_opening) {
                                if (count != 1) {
                                    break; // starts again to ask all stations starting from the last found opening
                                }
                            }
                            if (count == stations.size()) {
                                start_time = first_opening;
                                return true;
                            }
                        }
                        else return false;
                    }
                    else return false;
                }
            }
        }
    }
    return true;
}

void SchedulerDataBlock::updateTasksStationIDs(void) {
	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
        if ((*it)->isStationTask()) {
            static_cast<StationTask *>(*it)->updateStationIDs();
        }
	}
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
        if ((it->second)->isStationTask()) {
            static_cast<StationTask *>(it->second)->updateStationIDs();
        }
	}
	for (inActiveTasksMap::const_iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++it) {
        if ((it->second)->isStationTask()) {
            static_cast<StationTask *>(it->second)->updateStationIDs();
        }
	}
}

void SchedulerDataBlock::tryScheduleUnscheduledTasks(void) {
	//TODO: SchedulerDataBlock::tryScheduleUnscheduledTasks Needs a complete rewrite!!!
	/*
	const AstroTime &min_time_between_tasks = Controller::theSchedulerSettings.getMinimumTimeBetweenTasks();
	Task task;
	Task::task_status status;
	Station *pStation = 0;
	bool opening_found, found_mutual_opening;
#ifdef DEBUG_SCHEDULER
	const Task * pTask(0);
#endif
	size_t count(0);
	AstroDateTime previous_opening, first_opening;
	unsigned nr_unscheduled = getNrUnscheduled();
	for (unsigned i = 0; i != nr_unscheduled; ++i) { // iterate over all unscheduled tasks
		opening_found = true;
		found_mutual_opening = false;
		if (getNextTask(task)) {
			status = task.getStatus();
			if (status == Task::ON_HOLD) {
				holdNextTask(TASK_ON_HOLD);
				opening_found = false;
			}
			else if (status == Task::ERROR) {
				holdNextTask();
				opening_found = false;
			}
			else if (checkTaskBoundaries(task)) {
				// check with the stations requested by the task if the requested time is already occupied. If not schedule the task
				first_opening = task.getFirstPossibleDateTime();
				if (task.hasLinkedTask()) {
					// TODO: Maybe the user should have a choice if he doesn't want a task to be scheduled if this doesn't fit with a predecessor or a successor
					if (task.hasPredecessors()) {
						if (getTaskStatus(task.getPredecessor()) == Task::SCHEDULED) { // don't schedule the task if the predecessor isn't scheduled
							first_opening = max(getTaskEndTime(task.getPredecessor()) + task.getPredecessorMinTimeDif(), first_opening);
							if (first_opening > Controller::theSchedulerSettings.getLatestSchedulingDay()) {
								holdNextTask(OUTSIDE_SCHEDULE_BOUNDS);
								opening_found= false;
							}
						} else {
							holdNextTask(PREDECESSOR_UNSCHEDULED); // don't schedule task if predecessor isn't scheduled
							opening_found = false;
						}
					}
					//TODO: A Task can have multiple successors! So calculate for all predecessors and take the latest time
					if (task.hasSuccesors()) {
						for (std::vector<unsigned>::const_iterator it = task.getSuccessors().begin(); it != task.getSuccessors().end(); ++it) {
							first_opening = max(getTaskFirstPossibleDate(*it) - getTaskPredecessorMaxTimeDif(*it) - task.getDuration(), first_opening);
						}
						if ((first_opening < Controller::theSchedulerSettings.getEarliestSchedulingDay()) |
								(first_opening > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
							holdNextTask(OUTSIDE_SCHEDULE_BOUNDS);
							opening_found= false;
						}
					}
				}
			}
			else { // task has first possible and last possible dates out of schedule bounds
				holdNextTask(OUTSIDE_SCHEDULE_BOUNDS);
				opening_found = false;
			}
			while (!found_mutual_opening && opening_found) {
				count = 0;
				const std::map<std::string, unsigned> &stations =  task.getStations();
				for (std::map<std::string, unsigned>::const_iterator it = stations.begin(); it != stations.end(); ++it) {
					++count;
					if ((pStation = getStationForChange(it->second))) {
						previous_opening = first_opening;
						if (pStation->findFirstOpportunity(task, first_opening, min_time_between_tasks)) {
							// if possible to schedule at this station then first_possible holds the possible start time
							if (first_opening != previous_opening) {
								// check if the opening found is not too far from the predecessor if there is a predecessor
								if (task.hasPredecessor()) {
									if (first_opening > getTaskEndTime(task.getPredecessor()) + task.getPredecessorMaxTimeDif()) {
										opening_found = false;
										holdNextTask(MAX_DISTANCE_PREDECESSOR);
										break;
									}
								}
								if (count != 1) {
									break; // starts again to ask all stations starting from the last found opening
								}
							}
							if (count == stations.size()) {
								found_mutual_opening = true;
							}
						} else { // one of the stations didn't find a possible scheduling time, skip other stations
							opening_found = false;
							holdNextTask(NO_MUTUAL_OPENING_FOUND);
							break; // break out for loop, don't ask other stations for schedule opening
						}
					} else {
						debugErr("sssisis", "SchedulerDataBlock::tryScheduleUnscheduledTasks: Station ", it->first.c_str(), ", with ID: ", it->second, " in the list of task ", task.getID(), " which doesn't seem to exist!");
						opening_found = false;
						holdNextTask(NON_EXISTING_STATION);
						break; // break out for loop, don't ask other stations for schedule opening
					}
				}
			}
			if (found_mutual_opening && (task.getStations().size() > 0)) {
				if (scheduleNextTask(first_opening)) {
#ifdef DEBUG_SCHEDULER
					if ((pTask = getScheduledTask(task.getID()))) {
						debugInfo("sissss", "Task: ", pTask->getID(), " is scheduled at start: ",
								pTask->getScheduledStart().toString().c_str(), " and end: ",	pTask->getScheduledEnd().toString().c_str());
						//std::cout << "Task: " << pTask->getID() << " is scheduled at start: " << pTask->getScheduledStart().toString()
						//		<< " and end: " << pTask->getScheduledEnd().toString() << "." << std::endl;
					} else {
						debugErr(
								"sis", "SchedulerDataBlock::tryScheduleUnscheduledTasks: scheduled task ", task.getID(),
								"was not correctly scheduled!");
					}
#endif
				}
			}
		}
	}
	*/
}

void SchedulerDataBlock::scheduleFixedTasks(void) {
	Task * pTask;
	for (unsigned i = 0; i < unscheduledTasks.size(); ++i) {
		//pTask = getTaskForChange(unscheduledTasks.at(i)->getID());
		pTask = unscheduledTasks.at(i);
		if (pTask->getFixedDay() && pTask->getFixedTime()) {
			if (pTask->getStatus() == Task::UNSCHEDULED) { // do not schedule or move task when state is other than APPROVED
				if (checkTaskBoundaries(*pTask)) {
				if (pTask->getScheduledStart().isSet()) {
					if (pTask->getDuration().isSet()) {
						pTask->setScheduledEnd(pTask->getScheduledStart() + pTask->getDuration());
						scheduleTask(pTask);
					} else if (pTask->getScheduledEnd().isSet()) {
						pTask->setDuration(pTask->getScheduledEnd() - pTask->getScheduledStart());
						scheduleTask(pTask);
					}
				} else if (pTask->getFirstPossibleDateTime().isSet()) {
					if (pTask->getDuration().isSet()) {
						pTask->setScheduledStart(pTask->getFirstPossibleDateTime());
						scheduleTask(pTask);
					}
				}
			}
				else {
					pTask->setReason(OUTSIDE_SCHEDULE_BOUNDS);
				}
			}
		}
	}
}

void SchedulerDataBlock::moveUnscheduledTask(unsigned from, unsigned to) {
	unscheduledTasks.insert(unscheduledTasks.begin() + to, unscheduledTasks.at(from)); // insert the element before the successor
	if (to < from) {
		++from; // increase from by one to compensate for the inserted element before the from element
	}
	unscheduledTasks.at(from) = 0; // prevents destruction of the task object pointed to by the pointer at the from position
	unscheduledTasks.erase(unscheduledTasks.begin() + from); // now we can safely erase the element from the old position
}

void SchedulerDataBlock::sortUnscheduledTasks2Priority(void) {
	// TODO: SchedulerDataBlock::sortUnscheduledTasks2Priority needs complete rewrite!!!
	/*
	// first sort then put the predecessors before their successors
	if (!unscheduledTasks.empty()) {
		sort(unscheduledTasks.begin(), unscheduledTasks.end(), cmp_TaskPriority());
		vector<unsigned> predecessor_chain;
		unsigned predecessor;
		bool end_of_chain_reached;
		// now move the predecessors before their successors
		for (unsigned i = 0; i != unscheduledTasks.size() - 1; ++i) { // cannot use iterators because they are invalidated after erase and insert operations later on
			predecessor = unscheduledTasks.at(i)->getPredecessor();
			if (predecessor) {
				predecessor_chain.push_back(predecessor);
				end_of_chain_reached = false;
				while (true) {
					// find the predecessor in the list of unscheduled tasks
					unsigned ipre = i + 1;
					for (; ipre != unscheduledTasks.size(); ++ipre) { //only search AFTER the current task, predecessors before it don't have to be moved!
						if (unscheduledTasks.at(ipre)->getID() == predecessor) { // found the predecessor in the unscheduled tasks after current task
							predecessor	= unscheduledTasks.at(ipre)->getPredecessor();
							if (!predecessor) {
								end_of_chain_reached = true;
								break;
							}
							predecessor_chain.push_back(predecessor);
						}
					}
					if (end_of_chain_reached | (ipre >= unscheduledTasks.size()))
						break; // while loop
				}
				// we arrived at the front of the chain of sequential tasks
				// now reverse the list and place the tasks in the right sequence
				for (std::vector<unsigned>::reverse_iterator rit =
					predecessor_chain.rbegin(); rit != predecessor_chain.rend(); ++rit) {
					for (unsigned ipre = i + 1; ipre != unscheduledTasks.size(); ++ipre) { //only search AFTER the current task, predecessors before it don't have to be moved!
						if (unscheduledTasks.at(ipre)->getID() == *rit) { // found the predecessor in the unscheduled tasks after current task
							moveUnscheduledTask(ipre, i++);
							break; // break out of search loop
						}
					}
				}
				predecessor_chain.clear();
			}
		}
	}
	*/
}

bool SchedulerDataBlock::checkTaskBoundaries(const Task &task) const {
	if (task.getFirstPossibleDateTime() > Controller::theSchedulerSettings.getLatestSchedulingDay()) {
		return false;
	}
	if (task.getLastPossibleDateTime() < Controller::theSchedulerSettings.getEarliestSchedulingDay()) {
		return false;
	}
	return true;
}

bool SchedulerDataBlock::taskExists(unsigned task_id) const {
	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		if ((*it)->getID() == task_id)
			return true;
	}
	if (scheduledTasks.find(task_id) != scheduledTasks.end()) {
		return true;
	}
	if (itsPipelines.find(task_id) != itsPipelines.end()) {
		return true;
	}
	if (inactiveTasks.find(task_id) != inactiveTasks.end()) {
		return true;
	}
	return false;
}


const Task *SchedulerDataBlock::getTask(unsigned task_id, id_type IDtype) const {
	switch (IDtype) {
	case ID_SCHEDULER:
		return getTaskByTaskID(task_id);
		break;
	case ID_MOM:
		return getTaskByMomID(task_id);
		break;
	case ID_SAS:
		return getTaskBySASTreeID(task_id);
		break;
	}
	return 0;
}

const Observation *SchedulerDataBlock::getObservation(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const Observation *>(getTask(taskID, IDtype)); // will return 0 if task is not an observation
}

const StationTask *SchedulerDataBlock::getStationTask(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const StationTask *>(getTask(taskID, IDtype)); // will return 0 if task is not a StationTask
}

const Pipeline *SchedulerDataBlock::getPipeline(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const Pipeline *>(getTask(taskID, IDtype)); // will return 0 if task is not an pipeline
}

const CalibrationPipeline *SchedulerDataBlock::getCalibrationPipeline(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const CalibrationPipeline *>(getTask(taskID, IDtype)); // will return 0 if task is not an CalibrationPipeline
}

const ImagingPipeline *SchedulerDataBlock::getImagingPipeline(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const ImagingPipeline *>(getTask(taskID, IDtype)); // will return 0 if task is not an ImagingPipeline
}

const PulsarPipeline *SchedulerDataBlock::getPulsarPipeline(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const PulsarPipeline *>(getTask(taskID, IDtype)); // will return 0 if task is not an PulsarPipeline
}

const LongBaselinePipeline *SchedulerDataBlock::getLongBaselinePipeline(unsigned taskID, id_type IDtype) const {
    return dynamic_cast<const LongBaselinePipeline *>(getTask(taskID, IDtype)); // will return 0 if task is not an LongBaselinePipeline
}

const StationTask *SchedulerDataBlock::getReservation(unsigned reservation_id) const {
	reservationsMap::const_iterator it = itsReservations.find(reservation_id);
	if (it != itsReservations.end()) {
		return it->second;
	} else return 0;
}

const StationTask *SchedulerDataBlock::getScheduledTask(unsigned task_id) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.find(task_id);
	if (it != scheduledTasks.end()) {
		return it->second;
	}
	else {
		reservationsMap::const_iterator rit = itsReservations.find(task_id);
		if (rit != itsReservations.end()) return rit->second;
	}
	return 0;
}

StationTask *SchedulerDataBlock::getScheduledTask(unsigned task_id) {
    scheduledTasksMap::const_iterator it = scheduledTasks.find(task_id);
    if (it != scheduledTasks.end()) {
        return it->second;
    }
    else {
        reservationsMap::const_iterator rit = itsReservations.find(task_id);
        if (rit != itsReservations.end()) return rit->second;
    }
    return 0;
}

const Observation *SchedulerDataBlock::getScheduledObservation(unsigned task_id) const {
    scheduledTasksMap::const_iterator it = scheduledTasks.find(task_id);
    if (it != scheduledTasks.end()) {
        return dynamic_cast<Observation *>(it->second);
    }
    return 0;
}


const Pipeline *SchedulerDataBlock::getPipelineTask(unsigned task_id) const {
	pipelinesMap::const_iterator it = itsPipelines.find(task_id);
	if (it != itsPipelines.end()) {
		return it->second;
	}
	return 0;
}

const Task *SchedulerDataBlock::getInactiveTask(unsigned task_id) const {
	inActiveTasksMap::const_iterator it = inactiveTasks.find(task_id);
	if (it != inactiveTasks.end()) {
		return it->second;
	}
	else return 0;
}

const Task *SchedulerDataBlock::getUnscheduledTask(unsigned task_id) const {
	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it
			!= unscheduledTasks.end(); ++it) {
		if ((*it)->getID() == task_id) {
			return *it;
		}
	}
	return 0;
}

Task *SchedulerDataBlock::getInactiveTaskForChange(unsigned task_id) const {
	inActiveTasksMap::const_iterator it = inactiveTasks.find(task_id);
	if (it != inactiveTasks.end()) {
		return it->second;
	} else {
		return 0;
	}
}

Task *SchedulerDataBlock::getUnscheduledTaskForChange(unsigned task_id) const {
	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		if ((*it)->getID() == task_id) {
			return *it;
		}
	}
	return 0;
}

StationTask *SchedulerDataBlock::getReservationForChange(unsigned task_id) {
	reservationsMap::const_iterator it = itsReservations.find(task_id);
	if (it != itsReservations.end()) {
		return it->second;
	}
	else return 0;
}

Pipeline *SchedulerDataBlock::getPipelineForChange(unsigned task_id) {
	pipelinesMap::const_iterator it = itsPipelines.find(task_id);
	if (it != itsPipelines.end()) {
		return it->second;
	}
	else return 0;
}

Task *SchedulerDataBlock::getTaskForChange(unsigned task_id, id_type type) {
	switch (type) {
	case ID_SCHEDULER:
		return getTaskForChangeByTaskID(task_id);
		break;
	case ID_MOM:
		return getTaskForChangeByMomID(task_id);
		break;
	case ID_SAS:
		return getTaskForChangeBySASTreeID(task_id);
		break;
	}
	return 0;
}

const Task *SchedulerDataBlock::getTaskByTaskID(unsigned task_id) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.find(task_id);
	if (it != scheduledTasks.end())
		return it->second;

    unscheduledTasksDeque::const_iterator uit = unscheduledTasks.begin();
    while(uit != unscheduledTasks.end()) {
        if ((*uit)->getID() == task_id) return *uit;
        ++uit;
    }

    pipelinesMap::const_iterator pit = itsPipelines.find(task_id);
    if (pit != itsPipelines.end())
        return pit->second;

	inActiveTasksMap::const_iterator iti = inactiveTasks.find(task_id);
	if (iti != inactiveTasks.end())
		return iti->second;

	reservationsMap::const_iterator rit = itsReservations.find(task_id);
	if (rit != itsReservations.end()) {
		return rit->second;
	}

    return 0;
}

const Task *SchedulerDataBlock::getTaskBySASTreeID(quint32 sas_tree_id) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.begin();
	while(it != scheduledTasks.end()) {
		if (it->second->getSASTreeID() == sas_tree_id) return it->second;
		++it;
	}

	inActiveTasksMap::const_iterator iit = inactiveTasks.begin();
	while(iit != inactiveTasks.end()) {
		if (iit->second->getSASTreeID() == sas_tree_id) return iit->second;
		++iit;
	}

	reservationsMap::const_iterator rit = itsReservations.begin();
	while(rit != itsReservations.end()) {
		if (rit->second->getSASTreeID() == sas_tree_id) return rit->second;
		++rit;
	}

	unscheduledTasksDeque::const_iterator uit = unscheduledTasks.begin();
	while(uit != unscheduledTasks.end()) {
		if ((*uit)->getSASTreeID() == sas_tree_id) return *uit;
		++uit;
	}

	pipelinesMap::const_iterator pit = itsPipelines.begin();
	while(pit != itsPipelines.end()) {
		if (pit->second->getSASTreeID() == sas_tree_id) return pit->second;
		++pit;
	}

	return 0;
}

const Task *SchedulerDataBlock::getTaskByMomID(quint32 momID) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.begin();
	while(it != scheduledTasks.end()) {
		if (it->second->getMomID() == momID) return it->second;
		++it;
	}

	inActiveTasksMap::const_iterator iit = inactiveTasks.begin();
	while(iit != inactiveTasks.end()) {
		if (iit->second->getMomID() == momID) return iit->second;
		++iit;
	}

	reservationsMap::const_iterator rit = itsReservations.begin();
	while(rit != itsReservations.end()) {
		if (rit->second->getMomID() == momID) return rit->second;
		++rit;
	}

	unscheduledTasksDeque::const_iterator uit = unscheduledTasks.begin();
	while(uit != unscheduledTasks.end()) {
		if ((*uit)->getMomID() == momID) return *uit;
		++uit;
	}

	pipelinesMap::const_iterator pit = itsPipelines.begin();
	while(pit != itsPipelines.end()) {
		if (pit->second->getMomID() == momID) return pit->second;
		++pit;
	}

	return 0;
}

Task *SchedulerDataBlock::getTaskForChangeByTaskID(unsigned task_id) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.find(task_id);
	if (it != scheduledTasks.end())
		return it->second;

	unscheduledTasksDeque::const_iterator uit = unscheduledTasks.begin();
	while(uit != unscheduledTasks.end()) {
		if ((*uit)->getID() == task_id) return *uit;
		++uit;
	}

	pipelinesMap::const_iterator pit = itsPipelines.find(task_id);
	if (pit != itsPipelines.end())
		return pit->second;

	reservationsMap::const_iterator rit = itsReservations.find(task_id);
	if (rit != itsReservations.end()) {
		return rit->second;
	}

	inActiveTasksMap::const_iterator iti = inactiveTasks.find(task_id);
	if (iti != inactiveTasks.end())
		return iti->second;

	return 0;
}

Task *SchedulerDataBlock::getTaskForChangeBySASTreeID(quint32 sas_tree_id) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.begin();
	while(it != scheduledTasks.end()) {
		if (it->second->getSASTreeID() == sas_tree_id) return it->second;
		++it;
	}

	inActiveTasksMap::const_iterator iit = inactiveTasks.begin();
	while(iit != inactiveTasks.end()) {
		if (iit->second->getSASTreeID() == sas_tree_id) return iit->second;
		++iit;
	}

	reservationsMap::const_iterator rit = itsReservations.begin();
	while(rit != itsReservations.end()) {
		if (rit->second->getSASTreeID() == sas_tree_id) return rit->second;
		++rit;
	}

	unscheduledTasksDeque::const_iterator uit = unscheduledTasks.begin();
	while(uit != unscheduledTasks.end()) {
		if ((*uit)->getSASTreeID() == sas_tree_id) return *uit;
		++uit;
	}

	pipelinesMap::const_iterator pit = itsPipelines.begin();
	while(pit != itsPipelines.end()) {
		if (pit->second->getSASTreeID() == sas_tree_id) return pit->second;
		++pit;
	}

	return 0;
}

Task *SchedulerDataBlock::getTaskForChangeByMomID(quint32 momID) const {
	scheduledTasksMap::const_iterator it = scheduledTasks.begin();
	while(it != scheduledTasks.end()) {
		if (it->second->getMomID() == momID) return it->second;
		++it;
	}

	inActiveTasksMap::const_iterator iit = inactiveTasks.begin();
	while(iit != inactiveTasks.end()) {
		if (iit->second->getMomID() == momID) return iit->second;
		++iit;
	}

	reservationsMap::const_iterator rit = itsReservations.begin();
	while(rit != itsReservations.end()) {
		if (rit->second->getMomID() == momID) return rit->second;
		++rit;
	}

	unscheduledTasksDeque::const_iterator uit = unscheduledTasks.begin();
	while(uit != unscheduledTasks.end()) {
		if ((*uit)->getMomID() == momID) return *uit;
		++uit;
	}

	pipelinesMap::const_iterator pit = itsPipelines.begin();
	while(pit != itsPipelines.end()) {
		if (pit->second->getMomID() == momID) return pit->second;
		++pit;
	}

	return 0;
}


// goes by all scheduler tasks and checks for status changes (for instance because the task has been downloaded again and overwritten)
// then it takes appropriate action to unschedule/move the task to the correct map (unscheduled/scheduled/inactive)
void SchedulerDataBlock::checkStatusChanges(void) {
	// check all tasks in the scheduled tasks map
	vector<unsigned> doUnscheduleTasks, doScheduleTasks, doInactivateTasks;
	Task::task_status status;
	for (scheduledTasksMap::const_iterator sit = scheduledTasks.begin(); sit != scheduledTasks.end(); ++sit) {
		if (sit->second) {
			status = sit->second->getStatus();
			if (status < Task::PRESCHEDULED || status > Task::COMPLETING) {
//			if ((status != Task::PRESCHEDULED) && (status != Task::SCHEDULED) && (status != Task::STARTING) && (status != Task::ACTIVE)) {
				// add to tasks to be unscheduled
				doUnscheduleTasks.push_back(sit->first);
			}
		}
	}
	// do the actual unscheduling / moving
	Task *pTask;
	for (vector<unsigned>::const_iterator uit = doUnscheduleTasks.begin(); uit != doUnscheduleTasks.end(); ++uit) {
		status = getScheduledTask(*uit)->getStatus(); // remember the current status of the task before taking action
		unscheduleTask(*uit);
		pTask = getUnscheduledTaskForChange(*uit);
		if (pTask) {
			pTask->setStatus(status);
			// now check if the previously unscheduled task needs to be moved to inactive map (they do not need to be moved to scheduled map because that's where they came from)
			if (status >= Task::FINISHED) {
				moveTaskToInactive(*uit);
			}
		}
	}


	// check all tasks in the inactive tasks map
	doScheduleTasks.clear();
	doUnscheduleTasks.clear();
	for (inActiveTasksMap::const_iterator iit = inactiveTasks.begin(); iit != inactiveTasks.end(); ++iit) {
		if (iit->second) {
			status = iit->second->getStatus(); // remember the current status of the task before taking action
			if (status < Task::FINISHED) { // not inactive anymore?
				// where does it need to go? unscheduled or scheduled map?
				if (status >= Task::PRESCHEDULED && status <= Task::COMPLETING) {
//				if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED) || (status == Task::STARTING) || (status == Task::ACTIVE)) {
					doScheduleTasks.push_back(iit->first);
				}
				else {
					doUnscheduleTasks.push_back(iit->first);
				}
			}
		}
	}
	// do the actual unscheduling / scheduling
	for (vector<unsigned>::const_iterator uit = doUnscheduleTasks.begin(); uit != doUnscheduleTasks.end(); ++uit) {
		moveTaskFromInactive(*uit);
	}
	for (vector<unsigned>::const_iterator sit = doScheduleTasks.begin(); sit != doScheduleTasks.end(); ++sit) {
		status = getInactiveTask(*sit)->getStatus(); // remember the current status of the task before taking action
		pTask = getInactiveTaskForChange(*sit);
		scheduleTask(pTask); // this also takes care of removing the task from the inactive map
        pTask = getScheduledTask(*sit);
		if (pTask) {
			pTask->setStatus(status); // set the previous status after scheduling
		}
	}


	// check all tasks in the unscheduled tasks map
	doScheduleTasks.clear();
	doInactivateTasks.clear();
	for (unscheduledTasksDeque::const_iterator usit = unscheduledTasks.begin(); usit != unscheduledTasks.end(); ++usit) {
		if (*usit) {
			status = (*usit)->getStatus(); // remember the current status of the task before taking action
			// does it need to be moved? where to? unscheduled or scheduled map?
			if (status >= Task::PRESCHEDULED && status <= Task::COMPLETING) {
//			if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED) || (status == Task::STARTING) || (status == Task::ACTIVE)) {
				doScheduleTasks.push_back((*usit)->getID());
			}
			else {
				doInactivateTasks.push_back((*usit)->getID());
			}
		}
	}
	// schedule the tasks that need to be scheduled
	for (vector<unsigned>::const_iterator sit = doScheduleTasks.begin(); sit != doScheduleTasks.end(); ++sit) {
		status = getUnscheduledTask(*sit)->getStatus(); // remember the current status of the task before taking action
		pTask = getUnscheduledTaskForChange(*sit);
		scheduleTask(pTask); // this also takes care of removing the task from the unscheduled deque
        pTask = getScheduledTask(*sit);
		if (pTask) {
			pTask->setStatus(status); // set the previous status after scheduling
		}
	}
	// move the tasks that need to be moved to the inactivemap
	for (vector<unsigned>::const_iterator iit = doInactivateTasks.begin(); iit != doInactivateTasks.end(); ++iit) {
		moveTaskToInactive(*iit);
	}
}

Task * SchedulerDataBlock::moveTaskFromInactive(unsigned task_id) {
	inActiveTasksMap::iterator it = inactiveTasks.find(task_id);
	if (it != inactiveTasks.end()) {
        Task *pTask(it->second);
        if (pTask->isPipeline()) {
            itsPipelines[it->first] = static_cast<Pipeline *>(pTask);
		}
		else {
            unscheduledTasks.push_back(pTask);
		}
		inactiveTasks.erase(it);
		return pTask;
	}
	return 0;
}

const Station *SchedulerDataBlock::getStation(unsigned station_id) const {
	stationsMap::const_iterator it = itsStations.find(station_id);
	if (it != itsStations.end()) {
		return &(it->second);
	}
	return 0;
}

Station *SchedulerDataBlock::getStationForChange(unsigned station_id) {
	stationsMap::iterator it = itsStations.find(station_id);
	if (it != itsStations.end()) {
		return &(it->second);
	}
	return 0;
}

bool SchedulerDataBlock::getNextTask(Task &task) const {
	if (!unscheduledTasks.empty()) {
		task = *(unscheduledTasks.front());
		return true;
	} else {
		debugErr("s",
				"SchedulerDataBlock::getNextTask: There are no unscheduled tasks.");
		return false;
	}
}

bool SchedulerDataBlock::scheduleNextTask(const AstroDateTime &schedule_time) {
	if (!unscheduledTasks.empty()) {
		Task * utp = unscheduledTasks.front();
		//		debugInfo("siss", "scheduling task: ", utp->getID(), " at: ", schedule_time.toString().c_str());
		utp->setScheduledStart(schedule_time);
		scheduleTask(utp);
		return true;
	} else {
		debugErr("s",
				"SchedulerDataBlock: There are no unscheduled tasks to schedule!");
		return false;
	}
}

bool SchedulerDataBlock::checkStationConflicts(StationTask *pTask) {
    const taskStationsMap &task_stations = pTask->getStations();
    std::vector<unsigned> overlappingTasks;
    for (taskStationsMap::const_iterator sit = task_stations.begin(); sit != task_stations.end(); ++sit) {
        for (stationsMap::const_iterator statit = itsStations.begin(); statit != itsStations.end(); ++statit) {
            if (sit->second == statit->first) { // found station
                // check conflicting overlapping tasks for this station. If conflicts then set the conflict in the checked task
                overlappingTasks = statit->second.getTaskswithinTimeSpan(pTask->getScheduledStart(), pTask->getScheduledEnd());
                // check parallel task constraints
                for (std::vector<unsigned>::const_iterator parTaskIDit = overlappingTasks.begin(); parTaskIDit != overlappingTasks.end(); ++parTaskIDit) {
                    if (*parTaskIDit != pTask->getID()) { // Station::getOverlappingTasks also returns the current task which obviously always overlaps with itself (exclude this task)
                        const Task *parallelTask = getScheduledTask(*parTaskIDit); // the next overlapping task on this station
                        if (parallelTask) {
                            if (parallelTask->isMaintenance()) {
                                pTask->setConflict(CONFLICT_MAINTENANCE);
                                pTask->setReason("Station " + sit->first + " is in maintenance. No tasks can be scheduled on that station during the maintenance period");
                                return false;
                            }
                            else if (pTask->isObservation() && parallelTask->isReservation()) {
                                if (static_cast<const Observation *>(pTask)->getReservation() != *parTaskIDit) {
                                    pTask->setConflict(CONFLICT_RESERVATION);
                                    pTask->setReason("Station " + sit->first + " is reserved. This observation is not part of that reservation and can therefore not be scheduled on that station during the reservation period");
                                    return false;
                                }
                            }
                            else if (pTask->isReservation() && parallelTask->isObservation()) {
                                if (static_cast<const Observation *>(parallelTask)->getReservation() != pTask->getID()) {
                                    pTask->setConflict(CONFLICT_RESERVATION);
                                    pTask->setReason("Reservation conflicts with scheduled observation:" + int2String(parallelTask->getID()) + " on station:" + sit->first);
                                    return false;
                                }
                            }
                            else if (pTask->isMaintenance()) {
                                pTask->setConflict(CONFLICT_MAINTENANCE);
                                pTask->setReason("Maintenance conflicts with scheduled task:" + int2String(parallelTask->getID()) + " on station:" + sit->first);
                                return false;
                            }
                            else {
                                pTask->setConflict(CONFLICT_STATIONS);
                                pTask->setReason("Observation conflicts with scheduled task:" + int2String(parallelTask->getID()) + " on station:" + sit->first);
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    pTask->clearConflict(CONFLICT_STATIONS);
    pTask->clearConflict(CONFLICT_OUT_OF_DATASLOTS);
    pTask->clearConflict(CONFLICT_BITMODE);
    pTask->clearConflict(CONFLICT_RESERVATION);
    pTask->clearConflict(CONFLICT_MAINTENANCE);
    return true;
}

/*
bool SchedulerDataBlock::checkStationConflicts(Task *pTask) {
	const taskStationsMap &task_stations = pTask->getStations();
	std::vector<unsigned> overlappingTasks;
	int NrDataSlotsLeft(0);
	unsigned short bitMode(pTask->getBitMode());
	switch (bitMode) {
	default:
	case 16:
		NrDataSlotsLeft = (MAX_DATASLOT_PER_RSP_16_BITS + 1) * 4; // 4 RSP boards
		break;
	case 8:
		NrDataSlotsLeft = (MAX_DATASLOT_PER_RSP_8_BITS + 1) * 4;
		break;
	case 4:
		NrDataSlotsLeft = (MAX_DATASLOT_PER_RSP_4_BITS + 1) * 4;
		break;
	}
	NrDataSlotsLeft -= pTask->getNrOfSubbands();
	Task::task_type otherType;
	std::vector<unsigned int> countedTasks;
	for (taskStationsMap::const_iterator sit = task_stations.begin(); sit != task_stations.end(); ++sit) {
		for (stationsMap::const_iterator statit = itsStations.begin(); statit != itsStations.end(); ++statit) {
			if (sit->second == statit->first) { // found station
				// check conflicting overlapping tasks for this station. If conflicts then set the conflict in the checked task
				overlappingTasks = statit->second.getTaskswithinTimeSpan(pTask->getScheduledStart(), pTask->getScheduledEnd());
				// check parallel task constraints
				for (std::vector<unsigned>::const_iterator parTaskIDit = overlappingTasks.begin(); parTaskIDit != overlappingTasks.end(); ++parTaskIDit) {
					if (*parTaskIDit != pTask->getID()) { // Station::getOverlappingTasks also returns the current task which obviously always overlaps with itself (exclude this task)
						const Task *parallelTask = getScheduledTask(*parTaskIDit); // the next overlapping task on this station
						if (parallelTask) {
							otherType = parallelTask->getType();
							if (otherType == Task::MAINTENANCE) {
								pTask->setConflict(CONFLICT_MAINTENANCE);
								pTask->setReason("Station " + sit->first + " is in maintenance. No tasks can be scheduled on that station during the maintenance period");
								return false;
							}
							else if (otherType == Task::RESERVATION) {
								if (pTask->getReservation() != *parTaskIDit) {
									pTask->setConflict(CONFLICT_RESERVATION);
									pTask->setReason("Station " + sit->first + " is reserved. This task is not coupled to that reservation and can therefore not be scheduled on that station during the reservation period");
									return false;
								}
							}
							else if (pTask->isReservation()) {
								if (parallelTask->getReservation() != pTask->getID()) {
									pTask->setConflict(CONFLICT_RESERVATION);
									pTask->setReason("Reservation conflicts with scheduled task:" + int2String(parallelTask->getID()) + " on station:" + sit->first);
									return false;
								}
							}
							else if (pTask->isMaintenance()) {
								pTask->setConflict(CONFLICT_MAINTENANCE);
								pTask->setReason("Maintenance conflicts with scheduled task:" + int2String(parallelTask->getID()) + " on station:" + sit->first);
								return false;
							}
							// check that bit mode is the same for parallel tasks
							else if (parallelTask->getBitMode() == bitMode) {
								// check the number of subbands
								if (find(countedTasks.begin(), countedTasks.end(), *parTaskIDit) == countedTasks.end()) { // did we count the dataslots of this parallel task already?
									countedTasks.push_back(*parTaskIDit);
									NrDataSlotsLeft -= parallelTask->getNrOfSubbands();
									if (NrDataSlotsLeft < 0) {
										// too many dataslots required, set conflict
										pTask->setConflict(CONFLICT_OUT_OF_DATASLOTS);
										pTask->setReason(string("Too many subbands on station:") + sit->first
												+ "\nConflicting task:" + int2String(parallelTask->getID()) + " - " + parallelTask->getTaskName() + " (" + int2String(parallelTask->getSASTreeID())
												+ ")\nThe tasks are either overlapping or too close to each other");
										return false;
									}
								}
							}
							else { // there is a bit mode conflict for this task and the parallel task
								pTask->setConflict(CONFLICT_BITMODE);
								pTask->setReason("Bit mode conflict with task:" + int2String(parallelTask->getID()) + " on station " + sit->first);
								return false;
							}
						}
					}
				}
			}
		}
	}
	pTask->clearConflict(CONFLICT_OUT_OF_DATASLOTS);
	pTask->clearConflict(CONFLICT_BITMODE);
	pTask->clearConflict(CONFLICT_RESERVATION);
	pTask->clearConflict(CONFLICT_MAINTENANCE);
	return true;
}
*/

bool SchedulerDataBlock::scheduleTask(Task * pTask) {
	unsigned taskID = pTask->getID();
	bool scheduledCorrect(false);
	if (pTask->isReservation() || pTask->isMaintenance()) {
		reservationsMap::iterator it = itsReservations.find(taskID);
		if (it != itsReservations.end()) {
			it->second->setStatus(Task::PRESCHEDULED); // sets the reservation's state to PRESCHEDULED
			it->second->setReason(NO_ERROR);
			scheduledCorrect = true;
		}
	}
	else if (pTask->isObservation()) {
        std::pair<scheduledTasksMap::iterator, bool> ret = scheduledTasks.insert(scheduledTasksMap::value_type(taskID, static_cast<Observation *>(pTask)));
		ret.first->second->setStatus(Task::SCHEDULED); // sets the task state to SCHEDULED
		ret.first->second->setReason(NO_ERROR);
		scheduledCorrect = ret.second;
	}
	else if (pTask->isPipeline()) {
		pTask->setStatus(Task::SCHEDULED); // sets the task state to SCHEDULED
		pTask->setReason(NO_ERROR);
		scheduledCorrect = true;
	}
	if (scheduledCorrect) { // if inserted correctly
		if (pTask->isStationTask()) {
			stationsMap::iterator sit;
			// go by all stations to schedule the task in the stations their task lists
            const taskStationsMap &stations =  static_cast<Observation *>(pTask)->getStations();
			for (taskStationsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
				sit = itsStations.find(it->second);
				if (sit != itsStations.end()) {
					sit->second.addTasktoStation(taskID, pTask->getScheduledStart(), pTask->getScheduledEnd());
				}
			}
		}

		if (!pTask->isReservation() && !pTask->isMaintenance() && !pTask->isPipeline()) {
			// remove from unscheduled tasks or inactive tasks (in case of aborted task that is rescheduled)
			bool foundInUnscheduledTasks(false);
			for (unscheduledTasksDeque::iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
				if ((*it)->getID() == taskID) {
					unscheduledTasks.erase(it);
					foundInUnscheduledTasks = true;
					break;
				}
			}
			if (!foundInUnscheduledTasks) {
				inactiveTasks.erase(taskID);
			}
		}
		return true;
	} else {
		debugErr("si", "SchedulerDataBlock: Could not schedule task: ", taskID);
		return false;
	}
}

const AstroDateTime & SchedulerDataBlock::getTaskStartTime(unsigned taskID) const {
	return getTask(taskID)->getScheduledStart();
}

const AstroDateTime & SchedulerDataBlock::getTaskEndTime(unsigned taskID) const {
	return getTask(taskID)->getScheduledEnd();
}

AstroDateTime SchedulerDataBlock::getTaskFirstPossibleDate(unsigned taskID) const {
	return getTask(taskID)->getFirstPossibleDateTime();
}

const AstroTime & SchedulerDataBlock::getTaskPredecessorMaxTimeDif(
		unsigned taskID) const {
	return getTask(taskID)->getPredecessorMaxTimeDif();
}

Task::task_status SchedulerDataBlock::getTaskStatus(unsigned taskID) const {
	return getTask(taskID)->getStatus();
}

void SchedulerDataBlock::holdNextTask(const std::string &reason) {
	if (!unscheduledTasks.empty()) {
		Task *pt = unscheduledTasks.front();

//		if (reason != UNSCHEDULED_REASON_END) {
		pt->setReason(reason);
//		}

		unscheduledTasks.pop_front();
		unscheduledTasks.push_back(pt); // put at the end
		if (pt->getStatus() == Task::SCHEDULED) {
			pt->setStatus(Task::UNSCHEDULED);
		}
	}
}

void SchedulerDataBlock::holdNextTask(unscheduled_reasons reason) {
	if (!unscheduledTasks.empty()) {
		Task *pt = unscheduledTasks.front();

		if (reason != UNSCHEDULED_REASON_END) {
			pt->setReason(unscheduled_reason_str[reason]);
		}

		unscheduledTasks.pop_front();
		unscheduledTasks.push_back(pt); // put at the end
		if (pt->getStatus() == Task::SCHEDULED) {
			pt->setStatus(Task::UNSCHEDULED);
		}
	}
}


void SchedulerDataBlock::updateStations(void) {
	const stationNameIDMapping & stations =	Controller::theSchedulerSettings.getStations();
	stationsMap stationsCopy = itsStations;
	itsStations.clear();

	unsigned station_id;
	stationsMap::iterator sit;
	for (stationNameIDMapping::const_iterator it = stations.begin(); it	!= stations.end(); ++it) {
		station_id = it->second;
		sit = stationsCopy.find(station_id);
		if (sit == stationsCopy.end()) { // new station
			Station newStation(it->first, station_id);
			std::pair<stationsMap::iterator, bool> ret = itsStations.insert( stationsMap::value_type(station_id, newStation) );
			if (!ret.second) {
				debugWarn("sssi", "SchedulerDataBlock::updateStations, Warning, could not insert new station: ", newStation.getName().c_str(), " with ID:", newStation.getStationID());
			}
		} else {
			itsStations.insert(stationsMap::value_type(sit->first, sit->second));
		}
	}
}

bool SchedulerDataBlock::addStation(unsigned station_id, const std::string &name) {
	if (itsStations.find(station_id) == itsStations.end()) {
		Station newStation(name, station_id);
		/*std::pair<stationsMap::iterator, bool> ret = */
		itsStations.insert(	stationsMap::value_type(station_id, newStation)	);
		return true;
	} else {
		debugWarn("sis","SchedulerDataBlock::addStation: Trying to add station with ID: ", station_id, " that already exist.");
		return false;
	}
}

std::vector<Task *> SchedulerDataBlock::getReservationsVector(void) const {
    std::vector<Task *> tasks;
	for (reservationsMap::const_iterator it = itsReservations.begin(); it != itsReservations.end(); ++it) {
		if (it->second->isReservation()) {
			tasks.push_back(it->second);
		}
	}
	return tasks;
}

std::vector<Task *> SchedulerDataBlock::getMaintenanceVector(void) const {
    std::vector<Task *> tasks;
	for (reservationsMap::const_iterator it = itsReservations.begin(); it != itsReservations.end(); ++it) {
		if (it->second->isMaintenance()) {
			tasks.push_back(it->second);
		}
	}
	return tasks;
}

std::vector<Task *> SchedulerDataBlock::getScheduledTasksVector(void) const {
	std::vector<Task *> tasks;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		tasks.push_back(it->second);
	}
	return tasks;
}

std::vector<Task *> SchedulerDataBlock::getInactiveTaskVector(void) const {
	std::vector<Task *> tasks;
	for (inActiveTasksMap::const_iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++it) {
		tasks.push_back(it->second);
	}
	return tasks;
}

std::vector<Task *> SchedulerDataBlock::getUnScheduledTasksVector(void) const {
	std::vector<Task *> tasks;

	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		tasks.push_back(*it);
	}
	return tasks;
}

std::vector<Task *> SchedulerDataBlock::getPipelinesVector(void) const {
    std::vector<Task *> pipelines;
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		pipelines.push_back(it->second);
	}
	// sort according to start time
	sort(pipelines.begin(), pipelines.end(), cmp_taskScheduledStart());
	return pipelines;
}

std::vector<Pipeline *> SchedulerDataBlock::getScheduledPipelinesVector(void) const {
    std::vector<Pipeline *> pipelines;
	Task::task_status status;
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		status = it->second->getStatus();
		if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
			pipelines.push_back(it->second);
		}
	}
	// sort according to start time
	sort(pipelines.begin(), pipelines.end(), cmp_taskScheduledStart());
	return pipelines;
}

std::vector<Task *> SchedulerDataBlock::getTasksInScheduleSortStartTime(void) const {
	std::vector<Task *> sortedTasks;
	//scheduledTasksMap & scheduled_tasks = data.getScheduledTasks();
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		sortedTasks.push_back(it->second);
	}
	for (reservationsMap::const_iterator it = itsReservations.begin(); it != itsReservations.end(); ++it) {
		if (it->second->getStatus() == Task::PRESCHEDULED) {
			sortedTasks.push_back(it->second);
		}
	}
	for (inActiveTasksMap::const_iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++it) {
		sortedTasks.push_back(it->second);
	}

	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	return sortedTasks;
}

std::map<unsigned, std::vector<Task *> > SchedulerDataBlock::getGroupedObservations(Task::task_status state) const {
	std::map<unsigned, std::vector<Task *> > tasks;
	unsigned groupID;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		groupID = it->second->getGroupID();
		if (groupID != 0) {
			if ((state != Task::TASK_STATUS_END) && (it->second->getStatus() == state)) {
				tasks[groupID].push_back(it->second);
			}
		}
	}
	if (state < Task::PRESCHEDULED) {
		for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
			groupID = (*it)->getGroupID();
			if (groupID != 0) {
				if ((state != Task::TASK_STATUS_END) && ((*it)->getStatus() == state)) {
					tasks[groupID].push_back(*it);
				}
			}
		}
	}
	for (std::map<unsigned, std::vector<Task *> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
		std::sort(it->second.begin(), it->second.end(), cmp_taskScheduledStart());
	}
	return tasks;
}

std::vector<unsigned> SchedulerDataBlock::tasksInGroup(unsigned groupID, selector_types type) const {
	std::vector<unsigned> groupTasks;
	unsigned taskGroup;
	bool add(true);

	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		taskGroup = it->second->getGroupID();
		if (taskGroup == groupID) {
			switch (type) {
            case SEL_OBSERVATIONS:
				add = (it->second->getType() == Task::OBSERVATION) ? true : false;
				break;
            case SEL_CALIBRATOR_PIPELINES:
            case SEL_TARGET_PIPELINES:
            case SEL_IMAGING_PIPELINES:
            case SEL_PREPROCESSING_PIPELINES:
            case SEL_PULSAR_PIPELINES:
            case SEL_LONGBASELINE_PIPELINES:
                add = false; // pipelines are not in scheduledTasks map
				break;
			default: // all tasks
				add = true;
				break;
			}
			if (add) {
				groupTasks.push_back(it->first);
			}
		}
	}

	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		taskGroup = (*it)->getGroupID();
		if (taskGroup == groupID) {
			switch (type) {
            case SEL_OBSERVATIONS:
                add = ((*it)->getType() == Task::OBSERVATION) ? true : false;
				break;
            case SEL_CALIBRATOR_PIPELINES:
            case SEL_TARGET_PIPELINES:
            case SEL_IMAGING_PIPELINES:
            case SEL_PREPROCESSING_PIPELINES:
            case SEL_PULSAR_PIPELINES:
            case SEL_LONGBASELINE_PIPELINES:
                add = false; // pipelines are not in scheduledTasks map
				break;
			default: // all tasks
				add = true;
				break;
			}
			if (add) {
				groupTasks.push_back((*it)->getID());
			}
		}
	}
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		taskGroup = it->second->getGroupID();
		if (taskGroup == groupID) {
			switch (type) {
            case SEL_OBSERVATIONS:
                add = false;
				break;
            case SEL_CALIBRATOR_PIPELINES:
                add = ((it->second->getType() == Task::PIPELINE) && (it->second->getStrategy().contains("calibrator", Qt::CaseInsensitive))) ? true : false;
				break;
            case SEL_TARGET_PIPELINES:
                add = ((it->second->getType() == Task::PIPELINE) && (it->second->getStrategy().contains("target", Qt::CaseInsensitive))) ? true : false;
				break;
            case SEL_IMAGING_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_IMAGING_PIPELINE || it->second->getProcessSubtype() == PST_MSSS_IMAGING_PIPELINE) ? true : false;
				break;
            case SEL_LONGBASELINE_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_LONG_BASELINE_PIPELINE) ? true : false;
                break;
            case SEL_PREPROCESSING_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_AVERAGING_PIPELINE) ? true : false;
				break;
            case SEL_PULSAR_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_PULSAR_PIPELINE) ? true : false;
                break;
            default: // all tasks
				add = true;
				break;
			}
			if (add) {
				groupTasks.push_back(it->first);
			}
		}
	}
	for (inActiveTasksMap::const_iterator it = inactiveTasks.begin(); it != inactiveTasks.end(); ++ it) {
		taskGroup = it->second->getGroupID();
		if (taskGroup == groupID) {
			switch (type) {
            case SEL_OBSERVATIONS:
                add = (it->second->getType() == Task::OBSERVATION) ? true : false;
				break;
            case SEL_CALIBRATOR_PIPELINES:
                add = ((it->second->getType() == Task::PIPELINE) && (it->second->getStrategy().contains("calibrator", Qt::CaseInsensitive))) ? true : false;
				break;
            case SEL_TARGET_PIPELINES:
                add = ((it->second->getType() == Task::PIPELINE) && (it->second->getStrategy().contains("target", Qt::CaseInsensitive))) ? true : false;
				break;
            case SEL_IMAGING_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_IMAGING_PIPELINE || it->second->getProcessSubtype() == PST_MSSS_IMAGING_PIPELINE) ? true : false;
                break;
            case SEL_LONGBASELINE_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_LONG_BASELINE_PIPELINE) ? true : false;
                break;
            case SEL_PREPROCESSING_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_AVERAGING_PIPELINE) ? true : false;
				break;
            case SEL_PULSAR_PIPELINES:
                add = (it->second->getProcessSubtype() == PST_PULSAR_PIPELINE) ? true : false;
                break;
            default: // all tasks
				add = true;
				break;
			}
			if (add) {
				groupTasks.push_back(it->first);
			}
		}
	}

	return groupTasks;
}

std::map<unsigned, std::vector<Task *> > SchedulerDataBlock::getGroupedTasks(Task::task_status state) const {
	std::map<unsigned, std::vector<Task *> > tasks;
	unsigned groupID;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		groupID = it->second->getGroupID();
		if (groupID != 0) {
			if (it->second->getStatus() == state) {
				tasks[groupID].push_back(it->second);
			}
		}
	}
	if (state < Task::PRESCHEDULED) {
		for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
			groupID = (*it)->getGroupID();
			if (groupID != 0) {
				if ((*it)->getStatus() == state) {
					tasks[groupID].push_back(*it);
				}
			}
		}
	}
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		groupID = it->second->getGroupID();
		if (groupID != 0) {
			if (it->second->getStatus() == state) {
				tasks[groupID].push_back(it->second);
			}
		}
	}

	for (std::map<unsigned, std::vector<Task *> >::iterator it = tasks.begin(); it != tasks.end(); ++it) {
		std::sort(it->second.begin(), it->second.end(), cmp_taskScheduledStart());
	}
	return tasks;
}

std::vector<Task *> SchedulerDataBlock::getPreScheduledTasksSortStartTime(void) const {
	std::vector<Task *> sortedTasks;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		if (it->second->getStatus() == Task::PRESCHEDULED)
			sortedTasks.push_back(it->second);
	}
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		if (it->second->getStatus() == Task::PRESCHEDULED)
			sortedTasks.push_back(it->second);
	}

	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	return sortedTasks;
}

std::vector<Task *> SchedulerDataBlock::getScheduledTasksSortStartTime(void) const {
	std::vector<Task *> sortedTasks;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		if (it->second->getStatus() == Task::SCHEDULED)
			sortedTasks.push_back(it->second);
	}
	for (pipelinesMap::const_iterator it = itsPipelines.begin(); it != itsPipelines.end(); ++it) {
		if (it->second->getStatus() == Task::SCHEDULED)
			sortedTasks.push_back(it->second);
	}

	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	return sortedTasks;
}

std::vector<Task *> SchedulerDataBlock::getScheduledObservationsSortStartTime(void) const {
	std::vector<Task *> sortedTasks;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		if ((it->second->getType() == Task::OBSERVATION) && (it->second->getStatus() == Task::SCHEDULED))
			sortedTasks.push_back(it->second);
	}

	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	return sortedTasks;
}

std::vector<Task *> SchedulerDataBlock::getPreScheduledObservationsSortStartTime(void) const {
	std::vector<Task *> sortedTasks;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		if ((it->second->getType() == Task::OBSERVATION) && (it->second->getStatus() == Task::PRESCHEDULED))
			sortedTasks.push_back(it->second);
	}

	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	return sortedTasks;
}


std::vector<Observation *> SchedulerDataBlock::getFutureObservationsSortStartTime(void) const {
	QDateTime cT = QDateTime::currentDateTimeUtc();
	AstroDateTime now(cT.date().day(), cT.date().month(), cT.date().year(),
			cT.time().hour(), cT.time().minute(), cT.time().second());
    std::vector<Observation *> sortedTasks;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
        if ((it->second->isObservation()) && (it->second->getScheduledStart() >= now)) {
            sortedTasks.push_back(static_cast<Observation *>(it->second));
		}
	}

	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskScheduledStart());
	return sortedTasks;
}


std::vector<Task *> SchedulerDataBlock::getUnScheduledTasksSortFirstDate(void) const {
	//unscheduledTasksDeque &unscheduledTasks = data.getUnscheduledTasks();
	std::vector<Task *> sortedTasks;

	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		sortedTasks.push_back(*it);
	}
	sort(sortedTasks.begin(), sortedTasks.end(), cmp_taskFirstPossibleDateTime());
	return sortedTasks;
}

std::map<std::string, std::vector<Task *> > SchedulerDataBlock::getPublishTasks(const AstroDateTime &start, const AstroDateTime &end) const {
	const std::vector<Task *> &sortedTasks = getTasksInScheduleSortStartTime();
	std::map<std::string, std::vector<Task *> > tasks;
	std::string projectID;
	for (std::vector<Task *>::const_iterator it = sortedTasks.begin(); it != sortedTasks.end(); ++it) {
		const AstroDateTime &taskStart = (*it)->getScheduledStart();
		const AstroDateTime &taskEnd = (*it)->getScheduledEnd();
		if ((taskEnd >= start) && (taskStart <=  end)) {
			const std::string &projectID((*it)->getProjectID());
			tasks[projectID].push_back(*it);
		}
	}
	return tasks;
}

unsigned SchedulerDataBlock::calculatePenalty(void) {
	itsPenalty = 0;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		itsPenalty += it->second->calculatePenalty();
	}
	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		if ((*it)->getStatus() != Task::ERROR) {
			itsPenalty += UNSCHEDULED_TASK_PENALTY;
		}
	}
	return itsPenalty;
}

void SchedulerDataBlock::cleanup(void) {
	// free memory pointed taken by all vector elements for unscheduledTasks
	for (unscheduledTasksDeque::iterator i = unscheduledTasks.begin(); i != unscheduledTasks.end(); ++i) {
		delete *i;
	}
	unscheduledTasks.clear(); // clear the vector itself

	// do the same for scheduledTasks
	for (scheduledTasksMap::iterator i = scheduledTasks.begin(); i != scheduledTasks.end(); ++i) {
		delete i->second;
	}
	scheduledTasks.clear(); // clear the map itself

	// and for reservations
	for (reservationsMap::iterator i = itsReservations.begin(); i != itsReservations.end(); ++i) {
		delete i->second;
	}
	itsReservations.clear();

	// pipelines
	for (pipelinesMap::iterator i = itsPipelines.begin(); i != itsPipelines.end(); ++i) {
		delete i->second;
	}
	itsPipelines.clear();

	// and for inactive tasks
	for (inActiveTasksMap::iterator i = inactiveTasks.begin(); i != inactiveTasks.end(); ++i) {
		delete i->second;
	}
	inactiveTasks.clear();

	errorTasks.clear();
	itsStations.clear();
	itsUsedTaskIDs.clear();
	itsPenalty = -1;
	itsSaveRequired = false;
	itsUploadRequired = false;
}


unscheduled_reasons SchedulerDataBlock::checkTaskStations(StationTask *pTask) {
	// check for forbidden station combinations
	const map<string, unsigned> &stations = pTask->getStations();
	if (!stations.empty()) {
		// check for non-existing stations
		for (map<string, unsigned>::const_iterator sit = stations.begin(); sit != stations.end(); ++sit) {
			if (!(Controller::theSchedulerSettings.stationExist(sit->first))) {
				errorTasks[pTask->getID()].push_back(STATION_ID);
				pTask->setReason(std::string("non existing station ") + sit->first);
				if (pTask->isScheduled()) {
					unscheduleTask(pTask->getID());
				}
				pTask->setStatus(Task::ERROR);
				return NON_EXISTING_STATION;// only one station error for this task is enough to have it reported as error
			}
		}
	}
	else { // empty station list
		errorTasks[pTask->getID()].push_back(STATION_ID);
		pTask->setReason(NO_STATIONS_DEFINED);
		pTask->setStatus(Task::ERROR);
		return NO_STATIONS_DEFINED;
	}
	return NO_ERROR;
}

bool SchedulerDataBlock::checkTasksForErrors(void) {
	Task *task;
	for (errorTasksMap::iterator tit = errorTasks.begin(); tit != errorTasks.end(); ++tit) {
		task = getTaskForChange(tit->first);
		if (task) {
			if (task->getStatus() == Task::ERROR) {
				task->setStatus(Task::UNSCHEDULED);
			}
			task->setReason(NO_ERROR);
		}
	}

	errorTasks.clear();

	for (unscheduledTasksDeque::const_iterator it = unscheduledTasks.begin(); it != unscheduledTasks.end(); ++it) {
		checkTask(*it);
	}

	std::vector<unsigned> scheduledTasksWithErrors;
	for (scheduledTasksMap::const_iterator it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it) {
		if (!checkTask(it->second)) scheduledTasksWithErrors.push_back(it->first);
	}

	Task *pTask(0);
	for (std::vector<unsigned>::const_iterator it = scheduledTasksWithErrors.begin(); it != scheduledTasksWithErrors.end(); ++it) {
		unscheduleTask(*it);
		pTask = getUnscheduledTaskForChange(*it);
		if (pTask) {
			pTask->setStatus(Task::ERROR);
		}
	}

	if (errorTasks.empty()) return true;
	else return false;
}

bool SchedulerDataBlock::checkTask(Task *pTask) { // do a 'manual' check for errors on a single task
	Task::task_status status(pTask->getStatus());
	if (status != Task::ON_HOLD) { // ON_HOLD tasks should not be checked for errors. They are typically set to ON_HOLD manually because they have an error that needs fixing
		unsigned taskID = pTask->getID();

		// check predecessors
		const IDvector &predecessors(pTask->getPredecessors());
		for (IDvector::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
			if (getTask(it->second, it->first) == 0) {
				if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
					unscheduleTask(taskID);
					pTask->setStatus(Task::ERROR);
				}
				pTask->setReason(PREDECESSOR_NOT_FOUND);
				return false;
			}
		}

		// check times
		if (pTask->getScheduledStart() > pTask->getScheduledEnd()) {
			if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
				unscheduleTask(taskID);
				pTask->setStatus(Task::ERROR);
			}
			errorTasks[pTask->getID()].push_back(PLANNED_START);
			errorTasks[pTask->getID()].push_back(PLANNED_END);
			pTask->setReason(START_LATER_THEN_END);
			return false;
		}

		// check stations
        if (pTask->isStationTask()) {
            if (checkTaskStations(static_cast<StationTask *>(pTask)) != NO_ERROR) {
				return false;
			}

            // check if required fields are specified (only for observations)
            if (pTask->isObservation()) {
                Observation *pObs(static_cast<Observation *>(pTask));
                if (pObs->getAntennaMode() == 0) {
                    if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
                        unscheduleTask(taskID);
                        pObs->setStatus(Task::ERROR);
                    }
                    errorTasks[pObs->getID()].push_back(ANTENNA_MODE);
                    pTask->setReason(ANTENNA_MODE_UNSPECIFIED);
                    return false;
                }
                if (pObs->getStationClock() == UNSPECIFIED_CLOCK) {
                    if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
                        unscheduleTask(taskID);
                        pObs->setStatus(Task::ERROR);
                    }
                    errorTasks[pObs->getID()].push_back(CLOCK_FREQUENCY);
                    pObs->setReason(CLOCK_FREQUENCY_UNSPECIFIED);
                    return false;
                }
                if (pObs->getFilterType() == 0) {
                    if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
                        unscheduleTask(taskID);
                        pObs->setStatus(Task::ERROR);
                    }
                    errorTasks[pObs->getID()].push_back(FILTER_TYPE);
                    pObs->setReason(FILTER_TYPE_UNSPECIFIED);
                    return false;
                }

                const Observation::RTCPsettings rtcp(pObs->getRTCPsettings());
                const TaskStorage::enableDataProdukts odp(pObs->storage()->getOutputDataProductsEnabled());
                if ((odp.coherentStokes || odp.incoherentStokes) && !rtcp.flysEye && (pObs->totalNrTABs() == 0)) {
                    pObs->setReason(unscheduled_reason_str[NO_TABS_DEFINED]);
                    if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
                        unscheduleTask(taskID);
                        pObs->setStatus(Task::ERROR);
                    }
                    return false;
                }
            }
        }


		// everything ok, no more errors
		if (status == Task::ERROR) { // if the task previously was in the ERROR state then set it to UNSCHEDULED if no errors are detected anymore
			pTask->setStatus(Task::UNSCHEDULED);
		}
		pTask->setReason(NO_ERROR);
		return true;
	}
	else return true;
}

bool SchedulerDataBlock::newErrorTasks(void) {
	Task *pTask = 0;
	for (errorTasksMap::const_iterator it = errorTasks.begin(); it != errorTasks.end(); ++it) {
		if ((pTask = getTaskForChange(it->first))) {
			if (pTask->getStatus() != Task::ERROR) // only one task that didn't have ERROR status set is required
				return true;
		}
	}
	return false;
}

void SchedulerDataBlock::markErrorTasksStatus(void) {
	Task * pTask = 0;
	for (errorTasksMap::iterator it = errorTasks.begin(); it != errorTasks.end(); ++it) {
		if ((pTask = getTaskForChange(it->first)))
			pTask->setStatus(Task::ERROR);
	}
}

void SchedulerDataBlock::clearError(unsigned taskID, data_headers header) {
	errorTasksMap::iterator it = errorTasks.find(taskID);
	if (it != errorTasks.end()) {
		std::vector<data_headers>::iterator dit = find(it->second.begin(), it->second.end(), header);
		if (dit != it->second.end()) {
			it->second.erase(dit);
			if (it->second.empty()) {
				errorTasks.erase(it);
			}
		}
	}
}

unsigned SchedulerDataBlock::getRandomScheduledTaskID(void) {
	scheduledTasksMap::iterator it = scheduledTasks.begin();
	int rndIdx;
	float rnd;
	if (!scheduledTasks.empty()) {
		rnd = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
		rndIdx = static_cast<int> (rnd * static_cast<float> (scheduledTasks.size()));
		while (--rndIdx >= 0) {
			++it;
		}
		return it->first;
	}
	else return 0;
}

