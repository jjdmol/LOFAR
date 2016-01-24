/*
 * Controller.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11478 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-02-10 09:28:15 +0000 (Mon, 10 Feb 2014) $
 * First creation : 4-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/Controller.h $
 *
 */


#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <vector>
#include <string>
#include "lofar_scheduler.h"
#include "schedulesettingsdialog.h"
#include "DataHandler.h"
#include "schedulersettings.h"
#include "OTDBtree.h"
#include "SASConnection.h"

class DataMonitorConnection;
class CEPCleanMainWindow;
class DataHandler;

class Controller : public QObject
{
	Q_OBJECT

public:
	Controller(QApplication &app);
	virtual ~Controller();

	// interface members to SchedulerGUI
	void start(void); // show the SchedulerGUI

	const campaignMap &getCampaignList(void) const {return theSchedulerSettings.getCampaignList();} // returns the campaign (project) list

	void openSASTreeViewer(int treeID) const;

	// storage settings
    void showGUI(void);
    void openSettingsDialog(void); // opens the settings dialog
    void refreshStorageNodesInfo(void); // refreshes the storage nodes availability and capacity info in the settingsdialog
	bool isDataMonitorConnected(void) const;
	const storageHostsMap &getStorageNodes(void) const {return itsDMConnection->getStorageNodes();}
	const statesMap &getStorageNodesStates(void) const {return itsDMConnection->getStates();}
	size_t getNrOfStorageNodesAvailable(void) const {return itsDMConnection->getNrOfStorageNodesAvailable();}
	const hostPartitionsMap &getStoragePartitions(void) const {return itsDMConnection->getHostPartitions();}
	int getStorageRaidID(int node_id, const std::string &raidName) const {return itsDMConnection->getStorageRaidID(node_id, raidName);}
	std::string getStorageNodeName(int node_id) const {return itsDMConnection->getStorageNodeName(node_id);}
	std::string getStorageRaidName(int node_id, int raid_id) const {return itsDMConnection->getStorageRaidName(node_id, raid_id);}
	int getStorageNodeID(const std::string &name) const {return itsDMConnection->getStorageNodeID(name);}
	void setStorageNodes(const storageHostsMap &nodes) {itsDMConnection->setStorageNodes(nodes);}
	void setStoragePartitions(const hostPartitionsMap &partitions) {itsDMConnection->setStoragePartitions(partitions);}
	const AstroDateTime &now(void); // returns the current UTC time
	bool connectToDataMonitor(void);
	void disconnectDataMonitor(void);
//	bool updateProjects(void); // fetches the updates project (campaign) list from SAS and updates the GUI accordingly
    void cleanupDialogClosed(void) {itsCleanupMainWindow = 0;}

	QString lastSASError(void) const;
	bool checkSASSettings(void);
	int checkSASconnection(const QString &username, const QString &password, const QString &DBname, const QString &hostname);
	void setSASConnectionSettings(const QString &username, const QString &password, const QString &DBname, const QString &hostname);

private:
	void connectSignals(void); // connect the signals from the SchedulerGUI to the Scheduler
	void loadProgramPreferences(void);
    bool possiblySave(); // check if things need to be saved and then cleans up the data
    bool checkSettings() const;

public slots:
	void quit(void); // exit Scheduler application after some checks

private slots:
	void saveSettings(void);
	void saveDefaultSettings(void);
	void loadSettings(void);
	void loadDefaultSettings(void);
	void closeSettingsDialog(void);

public:
    static SchedulerSettings theSchedulerSettings; // everyone can access these settings
    static unsigned itsFileVersion; // used for storing the last read input file version

private:
	QApplication *application;
	ScheduleSettingsDialog *itsSettingsDialog;
    CEPCleanMainWindow *itsCleanupMainWindow;
	DataHandler *itsDataHandler;

	AstroDateTime itsTimeNow;

    //database connections
	SASConnection *itsSASConnection;
	DataMonitorConnection *itsDMConnection;
};

#endif /* CONTROLLER_H_ */

