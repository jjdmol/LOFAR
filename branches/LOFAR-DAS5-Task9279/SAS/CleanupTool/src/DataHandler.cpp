/*
 * DataHandler.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 5-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/DataHandler.cpp $
 *
 */

#include "DataHandler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <QFile>
#include "Controller.h"
#include "lofar_scheduler.h"
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
