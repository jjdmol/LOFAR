/*
 * SASConnection.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 8-febr-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/SASConnection.h $
 *
 */

#ifndef SASCONNECTION_H_
#define SASCONNECTION_H_

#include <vector>
#include <map>
#include <string>
#include "blocksize.h"
#include "astrodatetime.h"
#include "schedulerdata.h"
#include "schedulerdatablock.h"
#include "task.h"
#include "OTDBtree.h"
#include "OTDBnode.h"
#include "sasuploaddialog.h"
#include "sasprogressdialog.h"
#include "statehistorydialog.h"
#include "TiedArrayBeam.h"
#include "calibrationpipeline.h"
#include "pulsarpipeline.h"
#include "imagingpipeline.h"
#include "longbaselinepipeline.h"
class Controller;
class SchedulerData;
class CEPCleanMainWindow;

#define PIC_TREE 10
#define TEMPLATE_TREE 20
#define VIC_TREE	30
#define DOWNLOAD_MODE true
#define UPLOAD_MODE false

typedef std::map<unsigned, Task *> SAStasks;
typedef std::vector<Task> ErroneousSASTasks;
typedef std::map<unsigned, task_diff> changedTasks;
// deletedDataMap: key = SAS DB name e.g. 'LOFAR_4', value vector: < tree ID, Data product type >
typedef std::map<QString, std::vector<std::pair<int, dataProductTypes> > > deletedDataMap;
// deleteVICmap: key = database name, value = stringlist of vic tree ids to delete
typedef std::map<QString, QStringList> deleteVICmap;


class changedIDTask {
public:
	unsigned newTaskID;
	unsigned oldTaskID;
	task_diff diff; // to keep track of other change in this task
};

typedef std::vector<changedIDTask> changedIDTasks;

class SASConnection {
public:
	SASConnection();
	SASConnection(Controller *controller);
	virtual ~SASConnection();

	void init(const QString &username, const QString &password, const QString &DBName, const QString &hostname);
	void setLastDownloadDate(const QDateTime &date) { itsLastDownloadDate = date; }
	void cleanup(void); // do a cleanup
	int connect(void);
	int connect(const QString &username, const QString &password, const QString &database, const QString &host);
	int testConnect(const QString &username, const QString &password, const QString &DBname, const QString &hostname);
	void disconnect(void) {	QSqlDatabase::database( "SASDB" ).close(); QSqlDatabase::removeDatabase( "SASDB" ); }
	bool downloadAllSASTasks(void/*bool mode = DOWNLOAD_MODE*/);
	bool checkSynchronizeNeeded(void);
	std::vector<unsigned> getUsedSASTaskIDs(void) const;// {return itsSASTaskIDs;}
    const std::pair<AstroDate, AstroDate> &getUploadedDateRange(void) const {return itsUploadedDateRange;}

    void setAutoPublishEnabled(bool enable) {itsUploadDialog->setAutoPublishEnabled(enable);}
    bool autoPublish(void) const {return itsUploadDialog->autoPublish();}
	// addToTreesToDelete: used by Controller when a task is deleted from the scheduler
	void addToTreesToDelete(unsigned treeID, unsigned task_id);
	// used by Controller when a previously deleted task is undeleted to add it back to the record of SAS tasks
	void removeFromSASTaskToDelete(unsigned treeID);
	// get the SAS authentication token from the SAS database
	const QString &getAuthToken(void) const {return itsAuthToken;}

	bool abortTask(int treeID); // aborts a vic tree that is active or queued
	bool setTaskOnHold(int treeID);// {return setTreeState(treeID, SAS_STATE_ON_HOLD);} // set the tree to the ON_HOLD state
	bool setTasksOnHold(const std::vector<int> trees); // set multiple trees on hold
	const SAStasks &SASTasks(void) const {return itsSASTasks;}
	// opens the clean upload dialog
	void showProgressUploadDialog(void);
	// hides the upload progress dialog
	void closeProgressUploadDialog(void) {itsProgressDialog.hide();}
	SASProgressDialog &progressDialog(void) {return itsProgressDialog;}
	//  start checking for changes that will be committed to the SAS database and the schedule
	bool startSynchronizeProcedure(const SchedulerData &scheduler_data);
	// commit the schedule to the SAS database and create and alter tasks in the database
	bool commitScheduleToSAS(SchedulerData &data);
	// check SAS connection status, database status, database access rights, database integrity and show this in a status dialog
	bool checkSASStatus(void);
	// shows the history of state changes in a separate dialog
	void showTaskStateHistory(int treeID);
	// fetches the existing projects (campaign info) from SAS and stores them in theSchedulerSettings
	bool getCampaignsFromSAS(void);
	// return the number of tasks that will be deleted from SAS with the next upload
	unsigned nrTaskToDeleteFromSAS(void) const {return itsTreesToDelete.size();}
	// gets the current status of a task from SAS
	Task::task_status getTaskStatus(int treeID) const;
    // gets the scheduled start time of the specified tree
    AstroDateTime getScheduledStartTime(int treeID) const;
	// gets a single Task from the SAS database
    std::pair<bool, Task *> getTaskFromSAS(int treeID, OTDBtree otdb_tree = OTDBtree());
	// get all default templates from SAS
	std::vector<DefaultTemplate> getDefaultTemplates(void);
	// add an error string to the progress dialog
	void addProgressError(const QString &error) {itsProgressDialog.addError(error);}
	// add an info string to the progress dialog
	void addProgressInfo(const QString &msg) {itsProgressDialog.addText(msg);}
    // translate MoM ID to SAS ID
    unsigned momIdToSasId(unsigned momid) const {return itsMomToSasIDmap.value(momid, 0);}
    // get the mom to sas ID mapping table
    const QMap<unsigned, unsigned> &momToSasIDmap(void) const {return itsMomToSasIDmap;}
    // translates all mom IDs in the supplied IDvector to SAS ids and directly updates the IDvector
    void translateMomPredecessors(IDvector &predecessors);

	QString lastConnectionError(void) const;
	OTDBtree getTreeInfo(int treeID) const;
    QString getTreeParset(int treeID); // gets the complete tree (parset) as a string
    QString getMetaData(int treeID);
	// delete SAS trees via specification of treeIDs in stringlist
	bool deleteTreesCleanup(const deleteVICmap &treeIDs, const QString &hostname, const QString &user, const QString &password);
	bool markDataProductsDeleted(const deletedDataMap &data, const QString &hostname/*, const QString &user, const QString &password*/);
	// gets the 'limits' value as a QVariant type (a QVariant can be converted to any other type)
	QVariant getNodeValue(int aTreeID, const QString &nameFragment, bool noWarnings = false);

private:
	// gets all 'non-scheduler branch' properties of the task from the SAS VIC tree
	bool alreadyDownloaded(unsigned id, id_type IDtype) const;
	bool getSASTaskProperties(int treeID, Task &task);
    bool getProcessingSettings(int treeID, Observation &task);
    bool getCalibrationSettings(int treeID, CalibrationPipeline &task);
    bool getDemixingSettings(int treeID, CalibrationPipeline &task);
    bool getImagingSettings(int treeID, ImagingPipeline &task);
    bool getLongBaselineSettings(int treeID, LongBaselinePipeline &task);
    bool getPulsarSettings(int treeID, PulsarPipeline &pulsarPipe);
    bool getStationSettings(int treeID, StationTask &task);
    bool getAnalogBeamSettings(int treeID, Observation &task);
    bool getDigitalBeams(int aTreeID, Observation &observation);
	void getInputStorageSettings(int treeID, Task &task);
	void getOutputStorageSettings(int treeID, Task &task);
	bool getScheduledTimes(int treeID, Task &task);
    bool getSchedulerInfo(int tree_id, Task &task);
	void getCampaignInfo(Task &task);
	void updateDefaultTemplates(void);
    void updateMoMToSasIDmapping(void); // updates the map used for translating Mom IDs to SAS IDs
    void storePublishDates(const Task *pTask);
    void clearItsSASTasks(void);
    const Task * fetchPredecessorObservation(const QString predStr);
	// get a complete OTDB node from the SAS database
	OTDBnode getNode(int treeID, const QString &nodeID) const;
	// To get a list of all VIC trees of one of the following groups:
	// groupType = "1": observations that are scheduled to start the next 'period' minutes
	//             "2": active observations ; period is ignored
	//             "3": observations that were finished during the last 'period' minutes
//	std::vector<OTDBtree> getTreeGroup(const std::string &groupType, const std::string &period) const;
	std::vector<OTDBnode> getItemList(int aTreeID, const QString &nameFragment) const;
	// checks for all modified VIC trees in the sas database and if modified updates them in itsSASVicTrees
	int getModifiedVICTrees(const QDateTime &afterdate);
	int getAllSASTasksWithinPeriod(int treeType, const AstroDateTime &begindate, const AstroDateTime &enddate);
	bool fetchAllPredecessorTasks(void);
    // create a new task (observation/pipeline) from a given otdb_tree and query
    Task *createNewTaskFromTree(const OTDBtree &otdb_tree, const QSqlQuery *query = 0);
	// create a new vic tree from the default template tree set in the task
	// (via a intermediate template tree) in the SAS database
	int createNewTree(Task &task);
	// create a new VIC tree from the template tree with ID baseTreeID in the SAS DB. returns the new treeId on success and 0 on failure
	int instantiateVICTree(int baseTreeID);
	// set a node value in a SAS template
	bool setTemplateNodeByName(int treeID, const QString &parentNodeName, const QString &nodeName, const QString &valueStr) const;
	// set a node value in a SAS template; preferred function if we already know the parent node id
	bool setTemplateNodeByID(int treeID, const QString &parentNodeID, const QString &nodeName, const QString &valueStr) const;
	// sets a single SAS node value in the database for specific treeID, and unique property name fragment
	bool setNodeValue(int treeID, const QString &nameFragment, const QString &valueStr, bool warnings = true);
	// sets a single SAS node value in the database for specific treeID and nodeID
	bool setNodeValue(int treeID, int nodeID, const QString &valueStr) const;
	// save a complete OTDBnode to the database
	bool saveNode(const OTDBnode &node) const;
	// save a single task to SAS
	bool saveTaskToSAS(int treeID, Task &task, const task_diff *diff = 0);
	// set the start and stop times of the task in the database
	bool setTreeSchedule(int treeID, const Task &task) const;
	// save the tree state to SAS DB according to the SAStree.state in the task
	bool setTreeState(int treeID, int SAS_state) const;
	// save the tree description to the SAS database according to the SAStree.state in the task
	bool saveDescription(int treeID, const Task &task) const;
	// save campaign info to SAS
	bool saveMoMinfo(int treeID, const Task &task) const;
	// sets all scheduler task properties in the SAS tree with ID treeID
	bool saveSchedulerProperties(int treeID, const Task &task, const task_diff *diff = 0);
	// save the settings of the analog beam to a SAS victree
    bool saveAnalogBeamSettings(int treeID, const Observation &task, const task_diff *diff = 0);
	// save the settings of the analog beam to a SAS template tree
    bool saveAnalogBeamToSAStemplate(int treeID, int beamNodeId, const Observation::analogBeamSettings &analogBeam);
	// save the settings of a single digital beam to a SAS victree
	bool saveDigitalBeamToSasVicTree(int treeID, int beamNr, const DigitalBeam &digiBeam);
	// save the settings of a single digital beam to a SAS template tree
	bool saveDigitalBeamToSAStemplate(int treeID, const QString &beamNodeId, const DigitalBeam &digiBeam);
	// stores all digital beam settings in SAS DB
    bool saveDigitalBeamSettings(int treeID, Observation &task, const task_diff *diff = 0);
    // stores all TAB settings in SAS DB to a template
	bool saveTiedArrayBeamToSAStemplate(int treeID, const QString &TABNodeID, const TiedArrayBeam &TAB);
	// save the settings of a single pencil beam to a SAS victree
	bool saveTiedArrayBeamToSasVicTree(int treeID, int beamNr, int TABnr, const TiedArrayBeam &TAB);
    // stores all TAB settings in SAS DB to a VIC tree
    bool saveTiedArrayBeamSettings(int treeID, const Observation &task, const task_diff *diff);
	// stores the processing (OLAP) settings in the SAS DB
    bool saveProcessingSettings(int treeID, const Observation &task, const task_diff *diff = 0);
	// stores the imaging pipeline settings in the SAS DB
    bool saveImagingSettings(int treeID, const ImagingPipeline &task, const task_diff *diff = 0);
    // stores the pulsar pipeline settings in the SAS DB
    bool savePulsarSettings(int treeID, const PulsarPipeline &task, const task_diff *diff = 0);
	// stores the pipeline calibration settings in the SAS DB
    bool saveCalibrationSettings(int treeID, const CalibrationPipeline &task, const task_diff *diff = 0);
    // stores the long baseline pipeline settings in the SAS DB
    bool saveLongBaselineSettings(int treeID, const LongBaselinePipeline &task, const task_diff *diff);
    // stores the pipeline demixing settings in the SAS DB
    bool saveDemixingSettings(int treeID, const DemixingSettings &, const task_diff *diff = 0);
	// stores the stations settings in the SAS DB
    bool saveStationSettings(int treeID, const StationTask &task, const task_diff *diff = 0);
	// saves the output storage (node) settings of the task to the SAS database
	bool saveOutputStorageSettings(int treeID, const Task &task, const task_diff *diff = 0);
	// saves the output data products specifications to SAS (filenames, locations, etc.)
	bool saveOutputDataProducts(int treeID, const Task &task);
	// saves the input storage settings for the task to the SAS database
	bool saveInputStorageSettings(int treeID, const Task &task);
	// save the assigned dataslots to SAS
    bool saveDataSlots(int treeID, const Observation &task);
    // calculate and save Cobalt BlockSize for Correlator
    bool saveCobaltBlockSize(int treeID, const Observation &task);
	// deletes all trees from SAS from which the treeID is in itsTreesToDelete vector
	bool deleteTrees(void);
	// helper function for creation and filling of dataslot nodes
	bool createAndWriteDataSlotNodes(int treeID, const QString &dataSlotTemplateIDstr, const QString &strParentDataslotNodeID, const QString &antennaFieldName,
			const QString &RSPBoardList, const QString &DataSlotList);
	std::vector<int> getAllVICTreeIDs(void) const;
	// updateLastDownloadDate updates the last SAS update time by fetching it from the SAS database
	void updateLastDownloadDate(void);
    void determineUploadedDateRange(void);

	// COBALT STUFF
    BlockSize calcCobaltBlockSize(Observation::RTCPsettings rtcp, TaskStorage::enableDataProdukts enabledOutputs, unsigned short clockMHz) const;

private:
    SAStasks itsSASTasks; // reflects the current state of the tasks in the SAS database
    std::map<unsigned, Task *> itsSASmodifiedTasks; // holds externally changed tasks and new tasks that do not yet exist in the scheduler
	std::vector<unsigned> itsSASdeletedTrees;
	std::map<unsigned, OTDBtree> itsSASVicTrees; // contains the metadata of all SAS trees within the schedule's period

    QMap<unsigned, unsigned> itsMomToSasIDmap;
	changedTasks itsChangedTasks;
	std::vector<unsigned> itsNewSchedulerTasks;
	std::vector<unsigned> itsTreesToDelete; // contains SAS tree IDs not taskIDs (used for deletion of SAS trees)
    std::vector<AstroDate> itsChangedDates;
    std::pair<AstroDate, AstroDate> itsUploadedDateRange;

	Controller *itsController;
	SASUploadDialog *itsUploadDialog;
	SASProgressDialog itsProgressDialog;
	StateHistoryDialog itsStateHistoryDialog;
	QString itsAuthToken;

	QDateTime itsLastDownloadDate;

	QString itsSASUserName, itsSASPassword;
    QString itsLastErrorString;
};

std::string getSasTextState(int sas_state);

#endif /* SASCONNECTION_H_ */
