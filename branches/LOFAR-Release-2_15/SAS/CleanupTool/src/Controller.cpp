/*
 * Controller.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11478 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-02-10 09:28:15 +0000 (Mon, 10 Feb 2014) $
 * First creation : 4-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/Controller.cpp $
 *
 */

#include "Controller.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "lofar_scheduler.h"
#include "schedulersettings.h"
#include "cepcleanmainwindow.h"
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QString>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include "SASConnection.h"
#include "DataMonitorConnection.h"

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::pair;

SchedulerSettings Controller::theSchedulerSettings = SchedulerSettings();
unsigned Controller::itsFileVersion = 0;

Controller::Controller(QApplication &app) :
    application(&app) , itsSettingsDialog(0), itsCleanupMainWindow(0) //, unCommittedChanges(false), data_loaded(false)
{
	Controller::theSchedulerSettings.setController(this);

    itsCleanupMainWindow = new CEPCleanMainWindow(this);
	itsSettingsDialog = new ScheduleSettingsDialog(this);
	itsSASConnection = new SASConnection(this);
	// connection to the Data Monitor
    itsDMConnection = new DataMonitorConnection();
	itsDataHandler = new DataHandler(this);
	itsDataHandler->setFileName("");
	loadProgramPreferences();
	connectSignals();
}

Controller::~Controller() {
	if (itsSettingsDialog) delete itsSettingsDialog;
	if (itsSASConnection) delete itsSASConnection;
	if (itsDMConnection) delete itsDMConnection;
    if (itsCleanupMainWindow) delete itsCleanupMainWindow;
	if (itsDataHandler) delete itsDataHandler;
}


void Controller::loadProgramPreferences(void) {
	if (itsDataHandler->loadProgramPreferences()) {
	}
}

void Controller::connectSignals(void)
{
	// subscribe to signals from the gui->
    itsCleanupMainWindow->connect(itsCleanupMainWindow->gui().action_Save_settings, SIGNAL(triggered()), this, SLOT(saveDefaultSettings()));
    itsCleanupMainWindow->connect(itsCleanupMainWindow->gui().action_Load_settings, SIGNAL(triggered()), this, SLOT(loadDefaultSettings()));
//	gui->connect(gui->getSchedulerGUIClass().actionLoad_default_settings, SIGNAL(triggered()), this, SLOT(loadDefaultSettings()));
    itsCleanupMainWindow->connect(itsCleanupMainWindow->gui().action_Quit, SIGNAL(triggered()), this, SLOT(quit()));
    itsCleanupMainWindow->connect(itsSettingsDialog, SIGNAL(actionSaveSettings()), this, SLOT(closeSettingsDialog()));
}

const AstroDateTime &Controller::now(void) {
	QDateTime currentTime = QDateTime::currentDateTimeUtc();
	itsTimeNow = AstroDateTime(currentTime.date().day(), currentTime.date().month(), currentTime.date().year(),
			currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second());
	return itsTimeNow;
}

void Controller::setSASConnectionSettings(const QString &username, const QString &password, const QString &DBname, const QString &hostname) {
	Controller::theSchedulerSettings.setSASConnectionSettings(username, password, DBname, hostname);
}

void Controller::showGUI(void) {
    itsCleanupMainWindow->show();
}

bool Controller::checkSASSettings(void) {
	if (theSchedulerSettings.getSASUserName().isEmpty()) {
		QMessageBox::critical(0, tr("SAS user name not specified"),
				tr("SAS user name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (theSchedulerSettings.getSASPassword().isEmpty()) {
		QMessageBox::critical(0, tr("SAS password not set"),
				tr("SAS password is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (theSchedulerSettings.getSASDatabase().isEmpty()) {
		QMessageBox::critical(0, tr("SAS database name not specified"),
				tr("SAS database name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}
	else if (theSchedulerSettings.getSASHostName().isEmpty()) {
		QMessageBox::critical(0, tr("SAS host name not specified"),
				tr("SAS host name is not specified.\n Please specify this via menu 'Settings' -> menu option 'Schedule settings' -> 'SAS' tab"));
		return false;
	}

	return true;
}

int Controller::checkSASconnection(const QString &username, const QString &password, const QString &DBname, const QString &hostname) {
	return itsSASConnection->testConnect(username, password, DBname, hostname);
}

QString Controller::lastSASError(void) const {
	return itsSASConnection->lastConnectionError();
}

void Controller::saveSettings(void) {
    QString filename = itsCleanupMainWindow->fileDialog(tr("Save Settings"), "set", tr("Settings files (*.set)"),1);
	if (!filename.isEmpty()) {
		itsDataHandler->saveSettings(filename);
		//set status string
//		std::string statStr = "Settings saved to file ";
//		statStr += filename.toStdString();
//		gui->setStatusText(statStr.c_str());
	}
}

void Controller::saveDefaultSettings(void) {
	if (itsDataHandler->saveSettings(PROGRAM_DEFAULT_SETTINGS_FILENAME)) {
        QMessageBox::information(itsCleanupMainWindow, "Saved default settings", tr("Default settings have been saved."));
	}
	else {
        QMessageBox::critical(itsCleanupMainWindow, "Default settings could not be saved", tr("ERROR: Default settings could not be saved."));
	}
}

void Controller::loadDefaultSettings(void) {
	QMessageBox msgBox;

	if (itsDataHandler->loadSettings(PROGRAM_DEFAULT_SETTINGS_FILENAME)) {
		msgBox.setText(tr("Default settings have been loaded."));
	} else {
		msgBox.setText(tr("ERROR: Default settings could not be loaded."));
	}
	msgBox.exec();
}

void Controller::loadSettings(void) {
    QString filename = itsCleanupMainWindow->fileDialog(tr("Load Settings"), "set", tr("Settings files (*.set)"));
	if (!filename.isEmpty()) {
        if (itsDataHandler->loadSettings(filename)) {
//			gui->setStatusText("Settings loaded");
		}
	}
}


void Controller::openSettingsDialog(void) {
	itsSettingsDialog->show();
}

void Controller::closeSettingsDialog() {
    theSchedulerSettings.updateSettings(itsSettingsDialog);
    itsSettingsDialog->close();
}

bool Controller::connectToDataMonitor(void) {
	bool bResult(true);
	if (!itsDMConnection->isOpen()) {
		QMessageBox mb(QMessageBox::Information, "Please wait for connection to data monitor",
				"Trying to connect to the data monitor.\n Please wait.");
		mb.show();
		QApplication::setOverrideCursor(Qt::WaitCursor);
		QCoreApplication::processEvents(); // force update to paint dialog

		if (itsDMConnection->init(theSchedulerSettings.getDMUserName(),
					theSchedulerSettings.getDMPassword(),
					theSchedulerSettings.getDMDatabase(),
					theSchedulerSettings.getDMHostName())) {
			if (itsDMConnection->connect()) {
				itsDMConnection->updateStorageNodes();
				itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());
				mb.hide();
			}
			else {
				bResult = false;
			}
		}
		else {
			bResult = false;
		}
		QApplication::restoreOverrideCursor();
	}
	else {
		bResult = false;
	}

	return bResult;

}

void Controller::disconnectDataMonitor(void) {
	itsDMConnection->disconnect();
}

bool Controller::isDataMonitorConnected(void) const {
    return itsDMConnection->isOpen();
}


void Controller::refreshStorageNodesInfo(void) {
    if (!itsDMConnection->isOpen()) {
        if (!connectToDataMonitor()) {
            QMessageBox::warning(itsCleanupMainWindow, "Could not connect to data monitor", "Could not connect to data monitor.\nPlease check the data monitor connection settings.\nStorage node info might not be up to date.");
        }
    }
    itsSettingsDialog->updateStorageNodeInfoTree(itsDMConnection->getStorageNodes(), itsDMConnection->getStates(), itsDMConnection->getHostPartitions());

}


bool Controller::possiblySave()
{ 	// check if data needs to be saved before cleaning up data
    // returns true if data saving went OK or when no data needs to be saved.
    // returns false if user decided to cancel the save or something else went wrong with saving the data.
        // ask user if he wants to save project file
        int answer = QMessageBox::question(itsCleanupMainWindow, tr("Save data"),
                tr("Do you want to save the cleanup data to disk?"),
                QMessageBox::Save,
                QMessageBox::No,
                QMessageBox::Cancel) ;
        if (answer == QMessageBox::No) {
            return true;
        }
        else if (answer == QMessageBox::Save) {
            if (1) { // save the data
                return true;
            }
            else return false; // something went wrong saving the data
        }
        else return false;
    return true;  // no changes need to be saved
}


void Controller::start()
{
    showGUI();
}


void Controller::quit()
{
    itsSASConnection->disconnect();
//    if (possiblySave())
//    {
        itsDataHandler->saveProgramPreferences();
        application->quit();
//    }
}


void Controller::openSASTreeViewer(int treeID) const {
	QString parsetTree(itsSASConnection->getTreeParset(treeID));
	OTDBtree otdb_tree(itsSASConnection->getTreeInfo(treeID));
	if (!parsetTree.isEmpty()) {
        itsCleanupMainWindow->parsetTreeView(parsetTree, otdb_tree);

	}
	else {
		QMessageBox::warning(0, tr("Empty tree parset returned"),
				tr("SAS could not create a tree parset for tree ") + QString::number(treeID));
		QApplication::beep();
	}
}
