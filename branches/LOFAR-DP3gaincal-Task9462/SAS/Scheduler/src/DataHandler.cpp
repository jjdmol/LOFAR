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

/*
bool DataHandler::readCSVFile(const QString &filename, SchedulerData &data)
{
    if (!filename.isEmpty())
    {
        ifstream file(filename.toStdString().c_str());
        if (file) {
            Task *pt(0);
            char buf[50];
            char buf2[500];
            string line;
            int bsize = sizeof(buf);
            stringstream sstr;
            // variables needed to create a task:
            unsigned int tid=0, pid=0, clock=0, ngt_time_wght=0, uVal(0);
            int task_status_int = -1, task_mode_int = -1, iVal(0);
            string target, status, strdt, task_mode_str;
            double priority = 0.0f;
            bool bool_value;
            processSubTypes subType = PST_UNKNOWN;

            while (!file.eof()) {
                tid=0; pid=0; clock=0; ngt_time_wght=0;
                iVal = 0;
                uVal = 0;
                target = ""; status = ""; strdt = "";
                priority = 0.0f;
                // read taskID
                file.getline(buf, bsize, ',');

                // if this is a commented line '#' then skip it
                if (buf[0] == '#') { getline(file, line); continue; }
                //else { file.getline(buf, bsize, ','); }

                sstr.str("");
                sstr.clear();
                sstr << buf;
                sstr >> tid;
                if (tid == 0) { // tid == 0 means we are probably at the end of the file now
                    continue;
                }

                // read task type
                getline(file, strdt, ',');
                string subtypestr;
                getline(file, subtypestr, ',');

                if (strdt != "") {
                    Task::task_type type = taskTypeFromString(strdt);
                    switch (type) {
                    case Task::OBSERVATION:
                        pt = data.newObservation(tid);
                        if (!pt) {
                            cerr << "Creating new observaton failed because task ID: " << tid << " already exists." << endl;
                            file.getline(buf2, sizeof(buf2)); // skip this line
                            continue;
                        }
                        break;
                    case Task::PIPELINE:
                        subType = stringToProcessSubType(subtypestr.c_str());
                        switch (subType) {
                        case PST_AVERAGING_PIPELINE:
                        case PST_CALIBRATION_PIPELINE:
                            pt = data.newCalibrationPipeline(tid, false);
                            break;
                        case PST_IMAGING_PIPELINE:
                        case PST_MSSS_IMAGING_PIPELINE:
                            pt = data.newImagingPipeline(tid, false);
                            break;
                        case PST_PULSAR_PIPELINE:
                            pt = data.newPulsarPipeline(tid, false);
                            break;
                        default:
                            pt = data.newPipeline(tid, false);
                            break;
                        }
                        if (!pt) {
                            cerr << "Creating new pipeline failed because task ID: " << tid << " already exists." << endl;
                            file.getline(buf2, sizeof(buf2)); // skip this line
                            continue;
                        }
                        break;
                    case Task::RESERVATION:
                        break;
                    case Task::MAINTENANCE:
                        break;
                    case Task::SYSTEM:
                        break;
                    default:
                        cerr << "Unknown task type for task ID: " << tid << " . Skipping." << endl;
                        file.getline(buf2, sizeof(buf2)); // skip this line
                        continue;
                    }
                    if (!pt) {
                        cerr << "Creating new task failed because task ID: " << tid << " already exists." << endl;
                        file.getline(buf2, sizeof(buf2)); // skip this line
                        continue;
                    }
                }
                else {
                    cerr << "Could not read task type for task ID: " << tid << ". Skipping." << endl;
                    file.getline(buf2, sizeof(buf2)); // skip this line
                    continue;
                }

                // read SAS ID
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> iVal;
                pt->setSASTreeID(iVal);

                // read MoM ID
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> iVal;
                pt->setMoMID(iVal);

                // read Group ID
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> uVal;
                pt->setGroupID(uVal);

                // read project name
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setProjectName(strdt);
                }
                // read task name
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setTaskName(strdt);
                }
                // read contact name
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setContactName(strdt);
                }
                // read contact phone
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setContactPhone(strdt);
                }
                // read contact e-mail
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setContactEmail(strdt);
                }
                // read Predecessor ID
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> pid;
                if (pid) {
                    // read predecessor minimum time difference
                    getline(file, strdt,',');
                    // read predecessor maximum time difference
                    getline(file, strdt,',');
                    pt->addPredecessor(pid);
                }
                else {
                    getline(file, strdt,',');
                    getline(file, strdt,',');
                }

                // read task status
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> task_status_int;

                // read task duration
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setDuration(strdt);
                }

                // read planned start
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setScheduledStart(strdt);
                }

                // read planned end
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setScheduledEnd(strdt);
                }

                // read first possible date
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setWindowFirstDay(strdt);
                }

                // read last possible date
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setWindowLastDay(strdt);
                }

                // read scheduling time window minimum time
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setWindowMinTime(strdt);
                }

                // read scheduling time window maximum time
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setWindowMaxTime(strdt);
                }

                // read fixed day boolean
                bool_value = false;
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> bool_value;
                pt->setFixDay(bool_value);

                // read fixed time boolean
                bool_value = false;
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> bool_value;
                pt->setFixTime(bool_value);

                // read priority
                sstr.str("");
                sstr.clear();
                file.getline(buf, bsize, ',');
                sstr << buf;
                sstr >> priority;
                pt->setPriority(priority);

                // unscheduled reason
                getline(file, strdt, ',');
                if (strdt != "") {
                    pt->setReason(strdt);
                }

                // if this is an observation
                if (pt->isObservation()) {
                    Observation *pObs = dynamic_cast<Observation *>(pt);
                    // read antenna mode
                    task_mode_int = -1;
                    sstr.str("");
                    sstr.clear();
                    file.getline(buf, bsize, ',');
                    sstr << buf;
                    sstr >> task_mode_int;
                    if (task_mode_int != -1) { // if task mode was specified as an integer in the file
                        pObs->setAntennaMode(static_cast<station_antenna_mode>(task_mode_int));
                    }
                    else { // if task mode was specified as a string in the file
                        sstr.clear();
                        sstr.seekp(0, std::ios::beg);
                        sstr >> task_mode_str;
                        pObs->setAntennaMode(task_mode_str);
                    }
                    // read station names
                    getline(file, strdt, ',');
                    if (strdt != "") {
                        pObs->setStations(strdt.c_str(),';');
                    }
                    // read clock frequency
                    sstr.str("");
                    sstr.clear();
                    file.getline(buf, bsize, ',');
                    sstr << buf;
                    sstr >> clock;
                    pObs->setStationClock(clock);
                    // read night-time weight factor
                    sstr.str("");
                    sstr.clear();
                    file.getline(buf, bsize, ',');
                    sstr << buf;
                    sstr >> ngt_time_wght;
                    pObs->setNightTimeWeightFactor(ngt_time_wght);
                }
                else { // skip to next line (task) in file
                    file.getline(buf2, sizeof(buf2)); // skip this line
                    continue;
                }
                // now check if this task has enough valid data to be scheduled, if not put on hold
                checkTask(pt);
                pt->calculateDataFiles();
            }
            file.close();
        }
        else {
            cerr << "Could not open file " << filename.toStdString() << "." << endl;
        }
        return true;
    }
    return false;
}
*/

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

bool DataHandler::loadProgramPreferences(void) {
	ifstream file;
	bool loadDefaults;
	file.open(PROGRAM_PREFERENCES_FILENAME, std::ios::binary);
	if (file.is_open()) {
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
			if (loadSettings(PROGRAM_DEFAULT_SETTINGS_FILENAME)) {
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

bool DataHandler::saveSettings(const QString &filename) const
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
		QDataStream out(&file);
		out << (unsigned)FILE_WRITE_VERSION;
		out << Controller::theSchedulerSettings;
		file.close();
        debugInfo("ss","Wrote settings to file: ", (filename.toStdString().c_str()));
		return true;
	}
    else {
        debugInfo("ss","Failed to write to file: ", (filename.toStdString().c_str()));
        return false;
    }
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
