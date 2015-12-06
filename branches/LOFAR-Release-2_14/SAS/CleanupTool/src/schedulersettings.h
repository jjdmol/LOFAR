/*
 * schedulersettings.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11825 $
 * Last change by : $Author: jong $
 * Change date    : $Date: 2014-09-25 11:35:16 +0000 (Thu, 25 Sep 2014) $
 * First creation : Aug 6, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/schedulersettings.h $
 *
 */

#ifndef SCHEDULERSETTINGS_H_
#define SCHEDULERSETTINGS_H_

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "lofar_utils.h"
#include "lofar_scheduler.h"
#include "schedulesettingsdialog.h"

class Controller;
class ScheduleSettingsDialog;

class SchedulerSettings {
public:
	SchedulerSettings();
	virtual ~SchedulerSettings();

	friend QDataStream & operator<<(QDataStream &out, const SchedulerSettings &settings);
	friend QDataStream & operator>>(QDataStream &in, SchedulerSettings &settings);

	const QString &getSASUserName(void) const {return itsSASUserName;}
	const QString &getSASPassword(void) const {return itsSASPassword;}
	const QString &getSASDatabase(void) const {return itsSASDatabase;}
	const QString &getSASHostName(void) const {return itsSASHostName;}

	const QString &getDMUserName(void) const {return itsDMUserName;}
	const QString &getDMPassword(void) const {return itsDMPassword;}
	const QString &getDMDatabase(void) const {return itsDMDatabase;}
	const QString &getDMHostName(void) const {return itsDMHostName;}

    bool getLoadDefaultSettingsOnStartUp(void) const {return itsLoadDefaultSettings;}
    int getStorageNodeID(const std::string &name) const;
	std::string getStorageNodeName(int node_id) const;
	std::string getStorageRaidName(int node_id, int raid_id) const;
	int getStorageRaidID(int node_id, const std::string &raidName) const;
	const storageHostsMap &getStorageNodes(void) const;
	const statesMap &getStorageNodesStates(void) const;
	size_t getNrOfStorageNodesAvailable(void) const;
	const hostPartitionsMap &getStoragePartitions(void) const;
    const QMap<QString, QPair<bool, QString> > &getExcludeStrings(void) const {return itsExcludeStrings;}
    QStringList getExludedStrings(void) const;

	//setters
	void setController(Controller *controller) {itsController = controller;}
	void updateSettings(const ScheduleSettingsDialog *settingsDialog);
	void setLoadDefaultSettingsOnStartup(bool load_defaults) {itsLoadDefaultSettings = load_defaults;}
	void setSASConnectionSettings(const QString &username, const QString &password, const QString &DBname, const QString &hostname) {
		itsSASUserName = username;
		itsSASPassword = password;
		itsSASDatabase = DBname;
		itsSASHostName = hostname;
	}
	void setSASUserName(const QString &SAS_username) {itsSASUserName = SAS_username;}
	void setSASPassword(const QString &SAS_password) {itsSASPassword = SAS_password;}
	void setSASDatabase(const QString &SAS_database) {itsSASDatabase = SAS_database;}
	void setSASHostName(const QString &SAS_hostname) {itsSASHostName = SAS_hostname;}
	void setDMUserName(const QString &DM_username) {itsDMUserName = DM_username;}
	void setDMPassword(const QString &DM_password) {itsDMPassword = DM_password;}
	void setDMDatabase(const QString &DM_database) {itsDMDatabase = DM_database;}
	void setDMHostName(const QString &DM_hostname) {itsDMHostName = DM_hostname;}

    // return the campaign list containing the existing projects
	const campaignMap &getCampaignList(void) const {return itsCampaigns;} // returns the campaign (project) list
	const campaignInfo &getCampaignInfo(const std::string &campaign_name);
    void clearCampaigns(void) {itsCampaigns.clear();}
    bool addCampaign(const campaignInfo &campaign);
    void setCampaigns(const campaignMap &campaigns) {itsCampaigns = campaigns;}
    void setExcludeStrings(const QMap<QString, QPair<bool, QString> > &strings) {itsExcludeStrings = strings;}

private:
	Controller *itsController;
    bool itsLoadDefaultSettings;
	// SAS database connection attributes
	QString itsSASUserName, itsSASPassword, itsSASDatabase, itsSASHostName, itsObservationIDprefix,
        itsDMUserName, itsDMPassword, itsDMDatabase, itsDMHostName;
    QMap<QString, QPair<bool, QString> > itsExcludeStrings; // list of user defined string to exclude from any deletion, value bool set to true if can be deleted
	campaignMap itsCampaigns;
};

#endif /* SCHEDULERSETTINGS_H_ */
