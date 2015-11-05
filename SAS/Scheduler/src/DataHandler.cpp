/*
 * DataHandler.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 5-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DataHandler.cpp $
 *
 */

#include "DataHandler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib> // Needed getenv
#include "astrotime.h"
#include "astrodatetime.h"
#include "task.h"
#include "schedulerdatablock.h"
#include "Controller.h"
#include "station.h"
#include "lofar_scheduler.h"
#include "pipeline.h"
#include "calibrationpipeline.h"
#include "imagingpipeline.h"
#include "pulsarpipeline.h"
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::vector;
using std::map;

DataHandler::DataHandler(const Controller *controller)
: itsController(controller)
{
}

DataHandler::~DataHandler()
{
}

void DataHandler::checkTask(Task *task) {
	bool bResult = false;
	if (!(task->getWindowFirstDay().isSet())) {
		if (!(task->getWindowLastDay().isSet())) {
			if (!(task->getWindowMinTime().isSet())) {
				if (!(task->getWindowMaxTime().isSet())) {
					bResult = true;
				}
			}
		}
	}
	if (!bResult) {
		task->setStatus(Task::ERROR);
		task->setReason("incomplete specification");
	}
}


bool DataHandler::writeCSVFile(const QString &filename, const std::vector<const Task *> tasks) {
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

    	QTextStream out(&file);

		// write data headers
		out << "# ";
		for (unsigned i=0; i < NR_DATA_HEADERS; ++i) {
			out << DATA_HEADERS[i] << ",";
		}
		out << endl;

		// write the tasks properties
		for (std::vector<const Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
			writeTaskToFile(out, *it);
		}

		file.close();

		debugInfo("sss","File: ", filename.toStdString().c_str(), " written correctly.");
		return true;
	}
	else {
		debugErr("sss","Cannot write to file ", filename.toStdString().c_str(), " !");
		return false;
	}
}

void DataHandler::writeTaskToFile(QTextStream &out, const Task *ptask) {
    out << ptask->getID() << ","
        << ptask->getTypeStr() << ","
        << ptask->getSASTreeID() << ","
        << ptask->getMomID() << ","
        << ptask->getGroupID() << ","
        << ptask->getProjectName() << ","
        << QString(ptask->getTaskName()).replace(",","_").toStdString().c_str() << ","
        << QString(ptask->getTaskDescription()).replace(",","_").toStdString().c_str() << ","
        << ptask->getStatusStr() << ","
        << QString(ptask->getContactName()).replace(",","_") << ","
        << QString(ptask->getContactPhone()).replace(",","_") << ","
        << ptask->getContactEmail() << ","
        << ptask->getPredecessorsString(';') << ","
        << ptask->getDuration().toString().c_str() << ","
        << ptask->getScheduledStart().toString().c_str() << ","
        << ptask->getScheduledEnd().toString().c_str() << ","
        << ptask->getWindowFirstDay().toString().c_str() << ","
        << ptask->getWindowLastDay().toString().c_str() << ","
        << ptask->getWindowMinTime().toString().c_str() << ","
        << ptask->getWindowMaxTime().toString().c_str() << ","
        << ptask->getFixedDay() << ","
        << ptask->getFixedTime() << ","
        << ptask->getPriority();
    if (ptask->hasStorage()) {
        out << "," << ptask->storage()->getTotalStoragekBytes();
    }

    if (ptask->isObservation()) {
        const Observation * pObs = static_cast<const Observation *>(ptask);
        out << pObs->getStationNamesStr(';').c_str() << "," << pObs->getAntennaModeStr() << "," << pObs->getStationClockStr() << ","
            << pObs->getFilterTypeStr() << "," << pObs->getNrOfSubbands();

        unsigned res_id(pObs->getReservation());
        if (res_id) {
            out << itsController->getReservationName(res_id);
        }
    }

    out << endl;
}

bool DataHandler::openSchedule(const QString &filename, SchedulerData &data) {
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) {
		QDataStream in(&file);
		in >> Controller::itsFileVersion; // set the current file version read from file
		std::cout << "version: " << Controller::itsFileVersion << std::endl;
		bool test(Controller::theSchedulerSettings.getIsTestEnvironment()); // remember current test environment setting, should not be changed when loading a schedule from disk
		in >> Controller::theSchedulerSettings >> data;
		Controller::theSchedulerSettings.setIsTestEnvironment(test);
		file.close();
		debugInfo("ss","Read file: ", filename.toStdString().c_str());
		itsFileName = filename;
		return true;
	}
	else
		return false;
}

bool DataHandler::saveSchedule(const QString &filename, const SchedulerData &data) {
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);
		out << (quint32)FILE_WRITE_VERSION << Controller::theSchedulerSettings << data;
		file.close();
		debugInfo("ss","Wrote file: ", filename.toStdString().c_str());
		itsFileName = filename;
		return true;
	}
	else
		return false;
}

// Returns the default settingsfile installed in the LOFARROT/share/scheduler
// if available. If not found returns the default name of the scheduler settings
// file. (old behaviour)
std::string DataHandler::getDefaultSettingsPath()
{
    std::string path;

    char * lofarRoot = getenv("LOFARROOT");
    // Test if env returned the lofarroot variable
    if (lofarRoot) //linux specific path concat. Is ugly
        path = string(lofarRoot)+ "/share/scheduler/" + PROGRAM_DEFAULT_SETTINGS_FILENAME;

    else // LOFARROOT not found
        path = string(PROGRAM_DEFAULT_SETTINGS_FILENAME);

    return path;
}

bool DataHandler::loadProgramPreferences(void) {
	ifstream file;
	bool loadDefaults;

	file.open(PROGRAM_PREFERENCES_FILENAME, std::ios::binary);
    if (file.is_open())
    {
		try {
			read_primitive<bool>(file, loadDefaults);
			Controller::theSchedulerSettings.setLoadDefaultSettingsOnStartup(loadDefaults);
		}
		catch (std::bad_alloc &) {
			debugErr("s","preferences file corrupted, canceling reading program preferences");
			file.close();
			return false;
		}
		file.close();
		if (loadDefaults) {
            std::string defaultSettingsPath = getDefaultSettingsPath();
            if (loadSettings(QString(defaultSettingsPath.c_str()))) {
				return true;
			}
			else return false;
		}
		return true;
	}
	else
		return false;
}

bool DataHandler::saveProgramPreferences(void) {
	ofstream file;
	file.open(PROGRAM_PREFERENCES_FILENAME, std::ios::binary);
	if (file.is_open()) {
		write_primitive<bool>(file, Controller::theSchedulerSettings.getLoadDefaultSettingsOnStartUp());
		file.close();
		debugInfo("ss","Wrote preferences to file: ", PROGRAM_PREFERENCES_FILENAME);
		return true;
	}
	else
		return false;
}

bool DataHandler::saveSettings(const QString &filename) const {
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);
		out << (unsigned)FILE_WRITE_VERSION;
		out << Controller::theSchedulerSettings;
		file.close();
		debugInfo("ss","Wrote settings to file: ", filename.toStdString().c_str());
		return true;
	}
	else
		return false;
}

bool DataHandler::loadSettings(const QString &filename) {
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) {
		QDataStream in(&file);
		try {
			in >> Controller::itsFileVersion; // set the current file version read from file
//			file.close();
			if (Controller::itsFileVersion != 0) {
				in >> Controller::theSchedulerSettings;
			}
			else {
				debugErr("ss","Could not read file version from settings file: ", filename.toStdString().c_str());
				return false;
			}
		}
		catch (std::bad_alloc &) {
			file.close();
			debugErr("sss","settings file: ", filename.toStdString().c_str()," corrupted, canceling reading program settings");
			return false;
		}
		file.close();
		debugInfo("ss","Read settings from file: ", filename.toStdString().c_str());
		return true;
	}
	else {
		debugErr("ss","Could not open settings file: ", filename.toStdString().c_str());
		return false;
	}
}
