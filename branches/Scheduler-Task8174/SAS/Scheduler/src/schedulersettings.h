/*
 * schedulersettings.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Aug 6, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulersettings.h $
 *
 */

#ifndef SCHEDULERSETTINGS_H_
#define SCHEDULERSETTINGS_H_

#include <map>
#include <vector>
#include <string>
#include <fstream>

// preferredProjectStorageMap: preferred storage nodes based on projects (key = project ID)
typedef std::map<int, std::vector<int> > preferredProjectStorageMap;

enum sortMode {
	SORT_NONE,
	SORT_USAGE,
	SORT_SIZE
};

#include "task.h"

// preferredDataProductStorageMap: preferred storage nodes based on product data type
typedef std::map<dataProductTypes, std::vector<int> > preferredDataProductStorageMap;
// the sun rises and sun sets of every defined station specified as a pair of double (julian day) values
typedef std::map <unsigned int, std::vector<std::pair<double, double> > > stationSunMap;
// latitude and longitude of all defined stations
// the map's key is the unique station ID and the pair of doubles contain the latitude and longitude angles in radians
typedef std::map <unsigned int, std::pair<double, double> > stationPositionsMap;

typedef std::map<int, std::string> storageNodeNameIDmap;
typedef std::map<std::string, std::pair<double, double> > stationDefinitionsMap;
typedef std::vector <std::pair<std::string, unsigned int> > stationNameIDMapping;

enum storageNodeDistribution {
	STORAGE_DISTRIBUTION_FLAT_USAGE,
	STORAGE_DISTIBUTION_LEAST_FRAGMENTED,
	END_STORAGE_DISTRIBUTION
};
#define NR_STORAGE_DISTRIBUTIONS END_STORAGE_DISTRIBUTION

#include "lofar_utils.h"
#include "lofar_scheduler.h"
#include "astrodate.h"
#include "astrotime.h"
#include "schedulesettingsdialog.h"
#include "StorageNode.h"


class DefaultTemplate {
public:
	quint32 treeID;
	QString name;
	quint16 status;
	QString processType, processSubtype;
	QString strategy;
	QString description;
};

class Controller;
class ScheduleSettingsDialog;

typedef std::vector<std::pair<unsigned int, AstroDate> > scheduleWeekVector; // contains the week number and the monday of all the schedule weeks

class SchedulerSettings {
public:
	SchedulerSettings();
	virtual ~SchedulerSettings();

	friend QDataStream & operator<<(QDataStream &out, const SchedulerSettings &settings);
	friend QDataStream & operator>>(QDataStream &in, SchedulerSettings &settings);

	//getters
	const std::map<quint32, DefaultTemplate> &getDefaultTemplates(void) const {return itsDefaultTemplates;}
	const stationDefinitionsMap &getStationList(void) const {return itsStationList;}
    bool getUserAcceptedPenaltyEnabled(void) const {return itsUserAcceptedPenaltyEnabled;}
    unsigned int getUserAcceptedPenalty(void) const {return itsUserAcceptedPenalty;}
	bool stationExist(const std::string &stationName) const;
	const stationNameIDMapping & getStations(void) const {return itsStationNameIDMapping;}
	unsigned int getStationID(const std::string &station_name) const;
	std::string getStationName(unsigned int station_id) const;
	bool getStationPosition(unsigned int stationID, std::pair<double, double> &position);
	const std::vector<std::pair<double, double> > &getStationSunVector(unsigned int stationID) const;
	double getStationSunRise(unsigned int stationID, const AstroDate & day) const {return getStationSunRiseAndSet(stationID, day).first;}
	double getStationSunSet(unsigned int stationID, const AstroDate & day) const {return getStationSunRiseAndSet(stationID, day).second;}
	const std::pair<double, double> & getStationSunRiseAndSet(unsigned int stationID, const AstroDate & day) const;
    const AstroDate & getEarliestSchedulingDay(void) const {return itsEarliestDay;}
    const AstroDate & getLatestSchedulingDay(void) const {return itsLatestDay;}
    bool getMaxNrOptimizationsEnabled(void) const {return itsMaxNrOptimizationsEnabled;}
    unsigned getMaxNrOptimizations(void) const {return itsMaxNrOptimizations;}
    const AstroTime & getMinimumTimeBetweenTasks(void) const {return itsMinTimeBetweenTasks;}
    bool getLoadDefaultSettingsOnStartUp(void) const {return itsLoadDefaultSettings;}
    bool getIsTestEnvironment(void) const {return itsIsTestEnvironment;}
    const QString &getObservationIDprefix(void) const {return itsObservationIDprefix;}
	unsigned getShortTermScheduleDurationWeeks(void) const {return itsShortTermScheduleDuration;}
	unsigned getScheduleDurationMonths(void) const {return itsScheduleDuration;}
	quint16 getMaxNrOfFilesPerStorageNode(void) const {return itsMaxNrOfFilesPerStorageNode;}
	bool getAllowUnscheduleFixedTasks(void) const {return itsAllowUnscheduleFixedTasks;}
	bool getFocusTaskAtClick(void) const {return itsfocusTaskAtClick;}
	const QString &getSASUserName(void) const {return itsSASUserName;}
	const QString &getSASPassword(void) const {return itsSASPassword;}
	const QString &getSASDatabase(void) const {return itsSASDatabase;}
	const QString &getSASHostName(void) const {return itsSASHostName;}
	quint32 getSchedulerDefaultTemplate(void) const {return itsSchedulerDefaultTemplate;}
	quint32 getSASDefaultTreeID(const QString &ptype = "", const QString &pstype = "", const QString &strategy = "") const;// {return itsSASDefaultTree;}
	quint32 getSASDefaultTreeID(const Task *pTask) const;
	QStringList getAllProcessTypes(void) const;
	QStringList getAllProcessSubTypes(const QString &processType) const;
	QStringList getAllStrategies(const QString &processType, const QString &processSubtype) const;
	const QString &getDMUserName(void) const {return itsDMUserName;}
	const QString &getDMPassword(void) const {return itsDMPassword;}
	const QString &getDMDatabase(void) const {return itsDMDatabase;}
	const QString &getDMHostName(void) const {return itsDMHostName;}
	const QString &getLocalPublishPath(void) const {return itsLocalPublishPath;}
	const QString &getWebServerPublishPath(void) const {return itsWebServerPublishPath;}
	const QString &getSchedulerAccountName(void) const {return itsSchedulerAccountName;}
	const QString &getWebServerName(void) const {return itssWebServerName;}
	const QString &getPrivateKeyFile(void) const {return itsPrivateKeyFile;}
    bool getAutoPublish(void) const {return itsAutoPublish;}
    bool publishLocal(void) const {return itsPublishLocal;}
	const scheduleWeekVector &getScheduleWeeks(void) const {return itsScheduleWeeks;}
	int getStorageNodeID(const std::string &name) const;
	std::string getStorageNodeName(int node_id) const;
	std::string getStorageRaidName(int node_id, int raid_id) const;
	int getStorageRaidID(int node_id, const std::string &raidName) const;
	const storageHostsMap &getStorageNodes(void) const;
	const statesMap &getStorageNodesStates(void) const;
	size_t getNrOfStorageNodesAvailable(void) const;
	const hostPartitionsMap &getStoragePartitions(void) const;
	const double &getStorageNodeBandWidth(void) const {return itsStorageNodeBandWidth;}
	short int getStorageFillPercentage(void) const {return itsStorageFillPercentage;}
	double getRaidMaxWriteSpeed(void) const {return itsRaidMaxWriteSpeed;}
	const storageNodeDistribution &getStorageDistribution(void) const {return itsDataDistributionScheme;}
	bool getAllowMultipleRaidPerStorageNode(void) const {return itsAllowMultipleRaidPerNode;}
	const preferredProjectStorageMap &getPreferredProjectStorage(void) const {return itsPreferredProjectStorage;}
	const preferredDataProductStorageMap &getPreferredDataProductStorage(void) const {return itsPreferredDataProductStorage;}
	const QStringList &getDemixSources(void) const {return itsDemixSources;}

	//setters
	void setController(Controller *controller) {itsController = controller;}
	void updateSettings(const ScheduleSettingsDialog *settingsDialog);
	void setEarliestSchedulingDay(const AstroDate &date) {itsEarliestDay = date;}
	void setLatestSchedulingDay(const AstroDate &date) {itsLatestDay = date;}
	void setMinTimeBetweenTasks(const AstroTime &min_time) {itsMinTimeBetweenTasks = min_time;}
	void defineScheduleAndStations(const stationDefinitionsMap &stations, const AstroDate &start_day, const AstroDate &end_day);
	void setStationList(const stationDefinitionsMap &stationList) {itsStationList = stationList;}
	void setStationPosition(unsigned int stationID, const double &latitude, const double &longitude);
    void setUserAcceptedPenaltyEnabled(bool endis) {itsUserAcceptedPenaltyEnabled = endis;}
    void setUserAcceptedPenalty(unsigned int user_penalty) {itsUserAcceptedPenalty = user_penalty;}
    void setmaxNrOptimizationsEnabled(bool endis) {itsMaxNrOptimizationsEnabled = endis;}
	void setMaxNrOptimizations(unsigned maxNrOpt) {itsMaxNrOptimizations = maxNrOpt;}
	void setLoadDefaultSettingsOnStartup(bool load_defaults) {itsLoadDefaultSettings = load_defaults;}
	void setIsTestEnvironment(bool is_test_environment) {itsIsTestEnvironment = is_test_environment;}
	void setShortTermScheduleDurationWeeks(unsigned weeks) {itsShortTermScheduleDuration = weeks;}
	void setScheduleDurationMonths(unsigned months) {itsScheduleDuration = months;}
	void setMaxNrOfFilesPerStorageNode(quint16 nr_files) {itsMaxNrOfFilesPerStorageNode = nr_files;}
	void setAllowUnscheduleFixedTasks(bool allow) {itsAllowUnscheduleFixedTasks = allow;}
	void setFocusTaskAtClick(bool focus) {itsfocusTaskAtClick = focus;}
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
    void setAutoPublish(bool enabled) {itsAutoPublish = enabled;}
    void setLocalPublishPath(const QString &publish_path) {itsLocalPublishPath = publish_path;}
	void setSchedulerAccountName(const QString &account_name) {itsSchedulerAccountName = account_name;}
	void setWebServerPublishPath(const QString &publish_path) {itsWebServerPublishPath = publish_path;}
	void setWebServerName(const QString &server_name) {itssWebServerName = server_name;}
	void setPrivateKeyFile(const QString &private_key_file) {itsPrivateKeyFile = private_key_file;}
	void setPublishLocal(bool local) {itsPublishLocal = local;}
	void setStorageNodeBandWidth(const double &bandwidth) {itsStorageNodeBandWidth = bandwidth;}
	void setStorageFillPercentage(short int percentage) {itsStorageFillPercentage = percentage;}
	void setRaidMaxWriteSpeed(const double &write_speed) {itsRaidMaxWriteSpeed = write_speed;}
	void setStorageDistribution(const storageNodeDistribution &distribution) {itsDataDistributionScheme = distribution;}
	void setAllowMultipleRaidPerStorageNode(bool allow) {itsAllowMultipleRaidPerNode = allow;}
	// return the campaign list containing the existing projects
	const campaignMap &getCampaignList(void) const {return itsCampaigns;} // returns the campaign (project) list
	const campaignInfo &getCampaignInfo(const std::string &campaign_name);
	void clearCampaigns(void) {itsCampaigns.clear();}
	bool addCampaign(const campaignInfo &campaign);
	void setCampaigns(const campaignMap &campaigns) {itsCampaigns = campaigns;}
	const std::map<quint32, DefaultTemplate> &updateDefaultTemplates(void);
	void clearDefaultTemplates(void) {itsDefaultTemplates.clear();}
	void setPreferredProjectStorage(const preferredProjectStorageMap &map) {itsPreferredProjectStorage = map;}
	void setPreferredDataProductStorage(const preferredDataProductStorageMap &map) {itsPreferredDataProductStorage = map;}
	void setTestEnvironmentMode(bool enableTestMode); // set the scheduler to test environment mode
	void setDemixSources(const QStringList &demix_sources) {itsDemixSources = demix_sources;}

private:
	void defineStations(const stationDefinitionsMap &stations, bool recalculateExistingStations);
	void calculateScheduleWeeks(void);
	void calculateSunRisesAndSunDowns(unsigned int stationID);

private:
	Controller *itsController;
	std::pair<double, double> wrongSunRiseSetValue;
	quint32 itsSchedulerDefaultTemplate;
	std::map<quint32, DefaultTemplate> itsDefaultTemplates;
	stationDefinitionsMap itsStationList;
	AstroDate itsEarliestDay, itsLatestDay;
	scheduleWeekVector itsScheduleWeeks;
	AstroTime itsMinTimeBetweenTasks;
    quint16 uniqueStationID; //, itsMinNrOfStorageNodes;
	preferredDataProductStorageMap itsPreferredDataProductStorage;
	preferredProjectStorageMap itsPreferredProjectStorage;
	storageNodeDistribution itsDataDistributionScheme;
	bool itsAllowMultipleRaidPerNode;
	stationNameIDMapping itsStationNameIDMapping;
	stationPositionsMap itsStationPositions;
	stationSunMap itsStationSunMap;
	double itsStorageNodeBandWidth, itsRaidMaxWriteSpeed; // bandwidth in kbit/s, raid write speed in kbyte/sec
	quint8 itsStorageFillPercentage;
	bool itsUserAcceptedPenaltyEnabled, itsMaxNrOptimizationsEnabled, itsAllowUnscheduleFixedTasks;
	bool itsLoadDefaultSettings, itsfocusTaskAtClick, itsIsTestEnvironment;
	quint16 itsUserAcceptedPenalty, itsMaxNrOptimizations;
	quint16 itsShortTermScheduleDuration, itsScheduleDuration;
	quint16 itsNrOfDaysInSchedule, itsMaxNrOfFilesPerStorageNode;
	// SAS database connection attributes
	QString itsSASUserName, itsSASPassword, itsSASDatabase, itsSASHostName, itsObservationIDprefix,
        itsDMUserName, itsDMPassword, itsDMDatabase, itsDMHostName;
	QStringList itsDemixSources;
    bool itsPublishLocal, itsAutoPublish;
	QString itsLocalPublishPath, itsWebServerPublishPath, itssWebServerName, itsSchedulerAccountName, itsPrivateKeyFile;
	campaignMap itsCampaigns;
};

#endif /* SCHEDULERSETTINGS_H_ */
