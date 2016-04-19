/*
 * SASConnection.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 8-febr-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/SASConnection.cpp $
 *
 */

#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QString>
#include <QProgressDialog>
#include "lofar_utils.h"
#include "SASConnection.h"
#include "Controller.h"
#include "sasstatusdialog.h"
using std::string;
using std::vector;
using std::map;

SASConnection::SASConnection(void)
	: itsController(0), itsUploadDialog(0)
{
    QSqlDatabase::addDatabase( "QPSQL", "SASDB" );
}

SASConnection::SASConnection(Controller *controller)
	: itsController(controller)
{
    QSqlDatabase::addDatabase( "QPSQL", "SASDB" );
	itsUploadDialog = new SASUploadDialog(0, itsController);
}

SASConnection::~SASConnection() {
	QSqlDatabase::database( "SASDB" ).close();
	QSqlDatabase::removeDatabase( "SASDB" );

	if (itsUploadDialog) {
		delete itsUploadDialog;
	}
}

void SASConnection::init(const QString &username, const QString &password, const QString &DBName, const QString &hostname) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlDatabase::database( "SASDB" ).close();
	QSqlDatabase::removeDatabase( "SASDB" );
	sasDB = QSqlDatabase::addDatabase("QPSQL","SASDB");

	itsSASUserName = username;
	itsSASPassword = password;
    sasDB.setHostName(hostname);
    sasDB.setDatabaseName(DBName);
    sasDB.setUserName("postgres");
    sasDB.setPassword("");
}

void SASConnection::cleanup(void) {
    clearItsSASTasks();
    itsMomToSasIDmap.clear();
	itsSASVicTrees.clear();
	itsChangedTasks.clear();
	itsNewSchedulerTasks.clear();
	itsTreesToDelete.clear();
	itsSASdeletedTrees.clear();
}

int SASConnection::testConnect(const QString &username, const QString &password, const QString &DBname, const QString &hostname) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	sasDB.close();
	QSqlDatabase::removeDatabase( "SASDB" );
	sasDB = QSqlDatabase::addDatabase("QPSQL","SASDB");
	sasDB.setUserName("postgres");
	sasDB.setPassword("");
	sasDB.setHostName(hostname);
	sasDB.setDatabaseName(DBname);

	if (!sasDB.open()) {
		return -1; // could not connect to SAS database
	}
	else {
		QSqlQuery query(sasDB);
		query.exec("SELECT OTDBlogin('" + username + "','" + password + "')");
		if (query.next()) {
			if (query.value(0).toUInt() == 0) { // check authentication token (should not be zero)
				return -2; // no write permissions to SAS DB
			}
		}
		else return -3; // could not execute query on database
		query.finish();
	}
	//cleanup test connection to database
	return 0; // test OK.
}


QString SASConnection::lastConnectionError(void) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	return sasDB.lastError().text();
}

int SASConnection::connect(void) {
	return connect(Controller::theSchedulerSettings.getSASUserName(),
			Controller::theSchedulerSettings.getSASPassword(),
			Controller::theSchedulerSettings.getSASDatabase(),
			Controller::theSchedulerSettings.getSASHostName());
}

int SASConnection::connect(const QString &user, const QString &password, const QString &DBName, const QString &hostName) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QVariant value;
	if (!sasDB.open()) {  // sasDB could not be opened, re-init connection
		init(user, password, DBName, hostName);
		if (!sasDB.open()) {
			debugErr("sssssss","Could not establish SAS connection! Settings used:\n",
					"Username: ", sasDB.userName().toStdString().c_str(),
					", database: ", sasDB.databaseName().toStdString().c_str(),
					", hostname: ", sasDB.hostName().toStdString().c_str()
			);
			return -1; // could not connect to SAS database
		}
	}
	if (sasDB.isOpen()) {
		QSqlQuery query(sasDB);
		if (itsAuthToken.isEmpty()) {
			query.exec("SELECT OTDBlogin('" + itsSASUserName + "','" + itsSASPassword + "')");
			if (query.next()) {
				itsAuthToken = query.value(0).toString();
				if (itsAuthToken.isEmpty()) {
					return -2; // no write permissions to SAS DB
				}
			}
			query.finish();
		}

/*
		if (itsDataslotTemplateIDstr.isEmpty()) {
			// get the node ID of the dataslotsinfo node in the default template tree (needed to be able to create and save dataslot info for new trees)
			query.exec("SELECT nodeid from getVTitemList(" + QString::number(Controller::theSchedulerSettings.getSchedulerDefaultTemplate()) + ",'DataslotInfo')");
			if (query.next()) {
				itsDataslotTemplateIDstr = query.value(0).toString();
			}
			query.finish();
		}
*/

		// get the list of campaigns from SAS
		getCampaignsFromSAS();
	}
	return 0;
}


int SASConnection::getModifiedVICTrees(const QDateTime &afterdate) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );

	// check which trees still exist in sas database and remove them from itsSASVicTrees if they don't exist anymore
	std::vector<unsigned> existingVICs;
	QSqlQuery query("SELECT treeid FROM gettreelist('" + QString::number(VIC_TREE) + "','0',0,'','','')", sasDB);
	if (query.isActive()) {
		while (query.next()) {
			existingVICs.push_back(query.value(0).toUInt());
		}
	}
	query.finish();
	itsSASdeletedTrees.clear();
	for (std::map<unsigned, OTDBtree>::const_iterator oit = itsSASVicTrees.begin(); oit != itsSASVicTrees.end(); ++oit) {
		if (std::find(existingVICs.begin(), existingVICs.end(), oit->first) == existingVICs.end()) {
			itsSASdeletedTrees.push_back(oit->first);
		}
	}
	for (std::vector<unsigned>::const_iterator vit = itsSASdeletedTrees.begin(); vit != itsSASdeletedTrees.end(); ++vit) {
		itsSASVicTrees.erase(*vit);
	}

    // clear out itsSASmodifiedTasks
    for (std::map<unsigned, Task *>::const_iterator mit = itsSASmodifiedTasks.begin(); mit != itsSASmodifiedTasks.end(); ++mit) {
        delete mit->second;
    }
    itsSASmodifiedTasks.clear();

	AstroDateTime start_date(Controller::theSchedulerSettings.getEarliestSchedulingDay());
	AstroDateTime end_date = Controller::theSchedulerSettings.getLatestSchedulingDay().addDays(1);
    // add 1 second to prevent false positives of external changed tasks being caused by slow updates in OTDB of changes made by me
    QString fromDate(afterdate.addSecs(1).toString("yyyy-MM-dd hh:mm:ss"));

	// update the last download date before actually getting the changes.
	// This should make sure that changes made during this download will still be fetched next time a synchronize is run
	QDateTime prevDate(itsLastDownloadDate);
	updateLastDownloadDate();

	query.exec("SELECT * from getmodifiedtrees('" +
			fromDate + "','" +
			QString::number(VIC_TREE) + "')");

	if (query.isActive()) {
        std::pair<bool ,Task *> retVal;
		while (query.next()) {
			OTDBtree otdb_tree(query);
			const unsigned &otdbTreeID(otdb_tree.treeID());
			// update the task in itsSASVicTrees when it was already in there
			std::map<unsigned, OTDBtree>::iterator sit = itsSASVicTrees.find(otdbTreeID);
			if (sit != itsSASVicTrees.end()) {
				// task was already downloaded before, thus it needs updating again (download it again)
				retVal = getTaskFromSAS(otdbTreeID, otdb_tree);
                if (retVal.second) {
                    sit->second = retVal.second->SASTree(); // replace the tree in itsSASVicTrees with the updated tree
                    itsSASmodifiedTasks.insert(std::map<unsigned, Task *>::value_type(otdbTreeID, retVal.second)); // also put in itsSASmodifiedTasks
                }
			}
			else { // new tree that was not downloaded before
				// only download the tree if the start date is within the scheduling period or if the start time is not set
				if (!otdb_tree.startTime().isSet() || (otdb_tree.startTime() >= start_date && otdb_tree.stopTime() <= end_date)) {
					retVal = getTaskFromSAS(otdbTreeID, otdb_tree);
                    if (retVal.second) {
                        itsSASmodifiedTasks.insert(std::map<unsigned, Task *>::value_type(otdbTreeID, retVal.second));
                        itsSASVicTrees.insert(std::map<unsigned, OTDBtree>::value_type(otdbTreeID, retVal.second->SASTree()));
                    }
				}
			}
		}
	}
	else {
		itsLastDownloadDate = prevDate;
		return -1;// could not fetch any task from SAS, query not valid
	}

	query.finish();

	// also update all OTDBtrees in itsSASVicTrees that had a status change after itsLastDownloadDate and were already in itsSASVicTrees
	// use the SAS sql procedure getstatelist without a tree ID to get all state changes after a certain datetime
	query.exec("SELECT * FROM getstatelist(0,false,'" + fromDate + "',NULL) ORDER BY modtime DESC");
    SAStasks::const_iterator sasit;
	if (query.isActive()) {
		unsigned treeID;
		SAS_task_status SASstate;
        std::pair<bool, Task *> retVal;
		while (query.next()) {
			treeID = query.value(query.record().indexOf("treeid")).toUInt();
			if (itsSASVicTrees.find(treeID) != itsSASVicTrees.end()) { // only consider the state change if the task is already downloaded in the scheduler
                if (itsSASmodifiedTasks.find(treeID) == itsSASmodifiedTasks.end()) { // don't check state change if the task is already marked as changed and thus already in itsSASmodifiedTasks
					sasit = itsSASTasks.find(treeID);
					if (sasit != itsSASTasks.end()) {
						SASstate = static_cast<SAS_task_status>(query.value(query.record().indexOf("state")).toInt());
                        if (SASstate != sasit->second->getSAStreeState()) { // if current state in SAS is different from previously downloaded task state
							retVal = getTaskFromSAS(treeID);
                            if (retVal.second) {
                                itsSASmodifiedTasks.insert(std::map<unsigned, Task *>::value_type(treeID, retVal.second));
                                itsSASVicTrees[treeID] = retVal.second->SASTree();
                            }
						}
					}
				}
			}
		}
	}
	query.finish();

	return 0;
}

void SASConnection::updateLastDownloadDate(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT now()", sasDB);
	if (query.isActive()) {
		if (query.next()) {
			itsLastDownloadDate = QDateTime::fromString(query.value(0).toString(),"yyyy-MM-ddThh:mm:ss"); // one second added because some sas stored procedures will return changes *equal to* and greater this modification date, not
		}
	}
	query.finish();
}

// function getAllSASTasksWithinPeriod gets all OTDB trees (not complete vic trees but only their metadata) within the specified period
// This function will also check if the tree depends (has predecessors) on other trees and will the also download those predecessor trees
int SASConnection::getAllSASTasksWithinPeriod(int treeType, const AstroDateTime &begindate, const AstroDateTime &enddate) {
    int retVal(0);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	itsSASVicTrees.clear();
	QDateTime start_date = begindate.toQDateTime(), end_date = enddate.toQDateTime();

	QSqlQuery query("SELECT * from getTreesInPeriod('" +
					QString::number(treeType) + "','" +
					start_date.toString("yyyy-MM-dd hh:mm:ss") + "','" +
					end_date.toString("yyyy-MM-dd hh:mm:ss") + "')", sasDB);

	if (query.isActive()) {
		while (query.next()) {
			OTDBtree tree(query);
			itsSASVicTrees.insert(std::map<unsigned, OTDBtree>::value_type(tree.treeID(), tree));
		}
		query.finish();
	}
	else {
		query.finish();
		return -1;// could not fetch any task from SAS query not valid
	}

    if (!fetchAllPredecessorTasks()) {
        retVal = -2; // not all predecessors could be fetched from SAS
    }

    // store IDs in MomId to SasId table
    updateMoMToSasIDmapping();

    return retVal;
}

void SASConnection::updateMoMToSasIDmapping(void) {
    for (std::map<unsigned, OTDBtree>::const_iterator it = itsSASVicTrees.begin(); it != itsSASVicTrees.end(); ++it) {
        itsMomToSasIDmap[it->second.momID()] = it->second.treeID();
    }
}

bool SASConnection::alreadyDownloaded(unsigned id, id_type IDtype) const {
	if (IDtype == ID_MOM) {
			for (std::map<unsigned, OTDBtree>::const_iterator sit = itsSASVicTrees.begin(); sit != itsSASVicTrees.end(); ++sit) {
				if (sit->second.momID() == id) return true;
			}
		}
	else if (IDtype == ID_SAS) {
		for (std::map<unsigned, OTDBtree>::const_iterator sit = itsSASVicTrees.begin(); sit != itsSASVicTrees.end(); ++sit) {
			if (sit->first == id) return true;
		}
	}
	return false;
}

void SASConnection::translateMomPredecessors(IDvector &predecessors) {
    unsigned pred;
    for (IDvector::iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
        if (it->first == ID_MOM) {
            pred = itsMomToSasIDmap.value(it->second);
            if (pred) {
                it->first = ID_SAS;
                it->second = pred;
            }
        }
    }
}

bool SASConnection::fetchAllPredecessorTasks(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	bool bResult(true);
	QString IDonly;
	QSqlQuery query(sasDB);
	std::map<unsigned, OTDBtree> itsTmpSASVicTrees1(itsSASVicTrees), itsTmpSASVicTrees2;

    const QString & ObsIDPrefix(Controller::theSchedulerSettings.getObservationIDprefix());

	bool fetchPredecessor(false);
	if (!itsTmpSASVicTrees1.empty()) {
		itsProgressDialog.addText("Now fetching required predecessor tasks...");
	}
	while (!itsTmpSASVicTrees1.empty()) {
		for (std::map<unsigned, OTDBtree>::const_iterator it_1 = itsTmpSASVicTrees1.begin(); it_1 != itsTmpSASVicTrees1.end(); ++it_1) {
			// fetch all predecessor trees from the current tree and put them in itsTmpSASVicTrees2 and in itsSASVicTrees
			// first get the predecessor list from the current task
			const SAS_task_status &state(it_1->second.state());
			query.exec("SELECT limits from getVHitemList(" + QString::number(it_1->first) + ",'LOFAR.ObsSW.Observation.Scheduler.predecessors')");
			if (query.next()) {
				std::vector<QString> predecessors(string2VectorOfStrings(query.value(0).toString()));
				query.finish();
				QString isMomID, nonExistingPredecessors;
				for (std::vector<QString>::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
					// TODO: predecessor specified with scheduler task IDs should be translated to sas id during upload of the task
					fetchPredecessor = false;
					IDonly = it->right(it->size()-1);
					if (it->startsWith('M')) {
						if (!alreadyDownloaded(IDonly.toUInt(), ID_MOM)) {
							isMomID = "true";
							fetchPredecessor = true;
						}
					}
                    else if (it->startsWith(ObsIDPrefix)) { // SAS ID used for predecessor, fetch the predecessor directly
						if (!alreadyDownloaded(IDonly.toUInt(), ID_SAS)) {
							isMomID = "false";
							fetchPredecessor = true;
						}
					}
					else {
                        itsProgressDialog.addError(QString("Warning: incorrect predecessor:") + *it + " specified in SAS tree: " + QString::number(it_1->first) + ". Predecessor ID should start with M or " + ObsIDPrefix);
						bResult = false;
					}

					// fetch the predecessor task and push it in itsTmpSASVicTrees2
					if (fetchPredecessor) {
						query.exec("SELECT * from gettreeinfo(" + IDonly + ",'" + isMomID + "')");
						if (query.next()) {
							OTDBtree tree(query);
							query.finish();
							// check if predecessor is indeed a VIC tree
							if (tree.type() == VIC_TREE) {
								itsTmpSASVicTrees2.insert(std::map<unsigned, OTDBtree>::value_type(tree.treeID(), tree));
							}
							else if (!nonExistingPredecessors.isEmpty()) {
								nonExistingPredecessors += ",";
							}
							else {
								nonExistingPredecessors += *it;
							}
						}
						else if (state < SAS_STATE_FINISHED) { // only complain about non-existing predecessors if task still has to be performed
							if (!nonExistingPredecessors.isEmpty()) {
								nonExistingPredecessors += ",";
							}
							nonExistingPredecessors += *it;
						}
					}
				}
				if (!nonExistingPredecessors.isEmpty()) {
					itsProgressDialog.addError(QString("Warning: SAS tree: ") + QString::number(it_1->first) + " contains non-existing predecessor task(s):" + nonExistingPredecessors);
				}
			}
			else {
				query.finish();
				itsProgressDialog.addError(QString("Warning: Scheduler.predecessors info of SAS tree: ") + QString::number(it_1->first) + " could not be fetched");
				bResult = false;
			}
		}

		itsTmpSASVicTrees1 = itsTmpSASVicTrees2; // continue fetching predecessors of predecessors if any, until no more predecessors need fetching

		// add the predecessor trees to itsSASVictrees if they aren't already inserted there (which could happen if a task is also a predecessor
		// of another task and therefore was already fetched.
		for (std::map<unsigned, OTDBtree>::const_iterator tit = itsTmpSASVicTrees2.begin(); tit != itsTmpSASVicTrees2.end(); ++tit) {
			if (itsSASVicTrees.find(tit->first) == itsSASVicTrees.end()) {
				itsSASVicTrees.insert(std::map<unsigned, OTDBtree>::value_type(*tit));
			}
		}
		itsTmpSASVicTrees2.clear();
	}

	return bResult;
}

int SASConnection::createNewTree(Task &task) {
    // step 1: create a new template tree from a default template tree
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	QString authToken(itsAuthToken);
//	Task::task_type type(task.getType());
	if (sasDB.isOpen()) {
		bool scheduler_template_used(false);
		// step 1a: check for the existence of the requested default template,
		// if the default template id mentioned in the task is 0 then use the Scheduler default template
		// if it doesn't exist then return error
		int parentTemplateID = task.getOriginalTreeID();
		if (parentTemplateID == 0) { // unknown default template id, use the Scheduler default template to create this tree
			int schedulerDefaultTemplateID = Controller::theSchedulerSettings.getSchedulerDefaultTemplate();
			if (schedulerDefaultTemplateID) {
				parentTemplateID = schedulerDefaultTemplateID;
				itsProgressDialog.addError(QString("Warning: Task ") + QString::number(task.getID()) + " has unknown parent template ID, using 'Scheduler default template' to create the task in SAS");
				scheduler_template_used = true;
			}
			else {
				itsProgressDialog.addError("'Scheduler default template' does not exist in the database. Could not create the task in SAS");
				return 0;
			}
		}

		int newTemplateTreeID(0), newVICTreeID(0);
		QString newTemplateTreeIDstr;
		query.exec("SELECT * from copyTree(" +
				authToken + "," +
				QString::number(parentTemplateID) + ")"); // constructs a new template tree
		if (query.next()) {
			newTemplateTreeID = query.value(0).toInt();
			newTemplateTreeIDstr = QString::number(newTemplateTreeID);
			query.finish();

			// set the processType, processSubtype and strategy of the new template if the scheduler default template was used to create it
			if (scheduler_template_used) {
				query.exec("SELECT assignprocesstype(" +
						authToken + "," +
						newTemplateTreeIDstr + ",'" +
						task.getProcessType() + "','" +
						task.getProcessSubtypeStr() + "','" +
						task.getStrategy() + "')");

				if (!query.isActive()) { // query successfully executed?
					debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
							" query:", query.lastQuery().toStdString().c_str());
				}
				query.finish();
			}

            if (task.isObservation()) {
                Observation &observation(static_cast<Observation &>(task));
				QString templateBeamNodeID,templateTABNodeID, ObservationNodeID;
				// step 2a: (OBSERVATIONs only) create if needed the digital beams, analog beams and pencils (they cannot in principle be created in a VIC tree)
				// digital beams
				// get the node id of the Beam leaf in the template tree
                const std::map<unsigned, DigitalBeam> & digitalBeams = observation.getDigitalBeams();
				query.exec("SELECT nodeid from getVTitem(" + newTemplateTreeIDstr + ",'Observation')");
				if (query.next()) {
					ObservationNodeID = query.value(0).toString();
				}
				query.finish();
				if (digitalBeams.size() > 0) {
					query.exec("SELECT nodeid from getVTitemRecursive(" + newTemplateTreeIDstr + ",'Beam'," + ObservationNodeID + ")");
					if (query.next()) {
						templateBeamNodeID = query.value(0).toString(); // the nodeid (as a string) of the digital beam default beam used as 'template' to create new digital beams
						query.finish();

						QString newBeamNodeID;
						QString newTABNodeID;
						for (std::map<unsigned, DigitalBeam>::const_iterator dit = digitalBeams.begin(); dit != digitalBeams.end(); ++dit) {
							query.exec("SELECT dupVTnode(" + authToken + "," +
									newTemplateTreeIDstr + "," +
									templateBeamNodeID + ",'" + QString::number(dit->first) + "')"
							);
							if (query.next()) {
								newBeamNodeID = query.value(0).toString();
								saveDigitalBeamToSAStemplate(newTemplateTreeID, newBeamNodeID, dit->second);
							}
							query.finish();

							// add the Tied Array Beams for this digital beam
							const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams = dit->second.tiedArrayBeams();
							if (tiedArrayBeams.size() != 0) {

								// now get the node ID of the Tied Array Beam template component (for this digital beam)
								query.exec("SELECT nodeid from getVTitemRecursive(" + newTemplateTreeIDstr + ",'TiedArrayBeam'," + newBeamNodeID + ")");
								if (query.next()) {
									templateTABNodeID = query.value(0).toString();
								}
								query.finish();

								for (std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.begin(); tit != tiedArrayBeams.end(); ++tit) {
									query.exec("SELECT dupVTnode(" + authToken + "," +
											newTemplateTreeIDstr + "," +
											templateTABNodeID + ",'" + QString::number(tit->first) + "')"
									);

									if (query.next()) {
										newTABNodeID = query.value(0).toString(); // the node id of the newly created tied array beam
										saveTiedArrayBeamToSAStemplate(newTemplateTreeID, newTABNodeID, tit->second);
									}
									query.finish();
								}

								// save the number of Tied Array Beams in the default Tied Array Beam instance property
								OTDBnode node = getNode(newTemplateTreeID, templateTABNodeID);
								node.itsInstances = tiedArrayBeams.size();
								saveNode(node);
								// also set the Beam[x].nrTiedArrayBeams correctly
								setTemplateNodeByID(newTemplateTreeID, newBeamNodeID, "nrTiedArrayBeams", QString::number(node.itsInstances));
							}
						}
						// save the number of beams in the default beam instances property
						OTDBnode node = getNode(newTemplateTreeID, templateBeamNodeID);
						node.itsInstances = digitalBeams.size();
						saveNode(node);
						// also set Observation.nrBeams correctly
						setTemplateNodeByID(newTemplateTreeID, ObservationNodeID, "nrBeams", QString::number(digitalBeams.size()));
					}
				}

				// step2c: also create the single ANALOG BEAM (always, even if not HBA observation to enable later changing to HBA)
				query.exec("SELECT nodeid from getVTitemList(" + QString::number(newTemplateTreeID) + ",'AnaBeam')");
				if (query.next()) {
					QString analogBeamNodeID = query.value(0).toString(); // the nodeid of the analog template beam, used as template to create an analog beam
					query.finish();
					query.exec("SELECT dupVTnode(" + authToken + "," +
							QString::number(newTemplateTreeID) + "," +
							analogBeamNodeID + ",'0')"
					);
                    const Observation::analogBeamSettings & analogBeam = observation.getAnalogBeam();
					if (query.next()) {
						int nodeID = query.value(0).toInt();
						saveAnalogBeamToSAStemplate(newTemplateTreeID, nodeID, analogBeam);
					}
					query.finish();
					// save the number of beams in the default beam instances property
//					OTDBnode node = getNode(newTemplateTreeID, analogBeamNodeID);
//					node.itsInstances = 1; // there can be only one analog beam
//					saveNode(node);
					setTemplateNodeByID(newTemplateTreeID, ObservationNodeID, "nrAnaBeams", "1");
				}


			}

			// step : instantiate the VIC tree after which no beams can be added or deleted (but their properties can still be changed)
			newVICTreeID = instantiateVICTree(newTemplateTreeID);
			if (newVICTreeID != 0) {
				task.setSASTreeID(newVICTreeID);
				// this is a new task. Check if it has been set (PRE)SCHEDULED. If so generate the file list again so that it includes the SAS ID
				Task::task_status status(task.getStatus());
                if (task.hasStorage() && ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED))) {
                    task.storage()->generateFileList();
				}
				// step 4: apply task properties to the new VICtree
				saveTaskToSAS(newVICTreeID, task);
			}
			else {
                std::cerr << "ERROR:instantiateVICTree did not succeed" << std::endl << sasDB.lastError().text().toStdString() << std::endl;
			}
			// step 5: delete the intermediate template tree
			query.exec("SELECT deleteTree(" + authToken + "," + QString::number(newTemplateTreeID) + ")");
			return newVICTreeID;
		}
		else {
            std::cerr << "ERROR: could not create new tree. Is the default tree set correctly in the scheduler settings (tab SAS)?"
                      << query.lastError().text().toStdString() << std::endl;
			return 0;
		}
	}
	else {
        std::cerr << "createNewTree: SAS db is closed!" << std::endl;
		return 0;
	}
}

int SASConnection::instantiateVICTree(int baseTreeID) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	int newTreeID(0);
	QSqlQuery query("SELECT * from instanciateVHtree(" +
								  itsAuthToken + "," +
								  QString::number(baseTreeID) + ")", sasDB); // constructs AND executes the query
	if (query.next()) {
		newTreeID = query.value(0).toInt();
		if (newTreeID == 0) { // unable to create new VIC tree
			debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
					" query:", query.lastQuery().toStdString().c_str());
		}
	}
	return newTreeID;
}

// gets the history of state changes for the SAS tree
// the query fields are: 'treeid', 'momid', 'state', 'username' and 'modtime'
void SASConnection::showTaskStateHistory(int treeID) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if (connect() == 0) {
		itsStateHistoryDialog.clear();
		if (treeID != 0) {
			QSqlQuery query(QString("SELECT * FROM getstatelist(") + QString::number(treeID) + ",false,NULL,NULL)",sasDB);
			if (query.isActive()) {
				while (query.next()) {
					QString treeID = query.value(query.record().indexOf("treeid")).toString();
					QString momID = query.value(query.record().indexOf("momid")).toString();
					QString state = getSasTextState(query.value(query.record().indexOf("state")).toInt()).c_str();
					QString userName = query.value(query.record().indexOf("username")).toString();
					QDateTime datetime = QDateTime::fromString(query.value(query.record().indexOf("modtime")).toString(), "yyyy-MM-ddThh:mm:ss");
					itsStateHistoryDialog.addStateInfo(treeID, momID, state, userName, datetime);
				}
				itsStateHistoryDialog.show();
			}
			else {
				QMessageBox::critical(0,QObject::tr("State list retrieve error"), QObject::tr("Could not retrieve the state change history from the SAS database"));
                std::cerr << query.lastError().text().toStdString() << std::endl;
			}
		}
		else {
			QMessageBox::warning(0,QObject::tr("No history for new task"), QObject::tr("This seems to be a new task which is not in the SAS database yet"));
		}
	}
	else {
		QMessageBox::critical(0, QObject::tr("No connection to SAS"),
				QObject::tr("Could not connect to SAS database.\n Please check SAS connection settings."));
	}
}

std::vector<int> SASConnection::getAllVICTreeIDs(void) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	std::vector<int> vicTreeIDs;
	QSqlQuery query(QString("SELECT * from gettreelist('") + QString::number(VIC_TREE) +"','0','0','','','')", sasDB);
	while (query.next()) {
		vicTreeIDs.push_back(query.value(query.record().indexOf("treeID")).toInt());
	}
	return vicTreeIDs;
}

std::vector<unsigned> SASConnection::getUsedSASTaskIDs(void) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	std::vector<unsigned> taskIDs;
	QSqlQuery query(QString("SELECT * from getalltaskid()"), sasDB);
	while (query.next()) {
		taskIDs.push_back(query.value(0).toUInt());
	}
	return taskIDs;
}

// gets the current status of a task from SAS
Task::task_status SASConnection::getTaskStatus(int treeID) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if (sasDB.isOpen()) {
		QSqlQuery query("select state from gettreeinfo(" + QString::number(treeID) + ", false)", sasDB);
		if (query.next()) {
			return convertSASstatus(static_cast<SAS_task_status>(query.value(0).toInt()));
		}
	}
	return Task::TASK_STATUS_END;
}

AstroDateTime SASConnection::getScheduledStartTime(int treeID) const {
    AstroDateTime start;
    QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
    if (sasDB.isOpen()) {
        QSqlQuery query("SELECT limits from getVHitemList(" + QString::number(treeID) + ",'LOFAR.ObsSW.Observation.startTime')", sasDB);
        if (query.next()) {
            QString startTimeStr(query.value(0).toString());
            if (!startTimeStr.isEmpty()) {
                start = startTimeStr;
            }
        }
    }
    return start;
}



Task *SASConnection::createNewTaskFromTree(const OTDBtree &otdb_tree, const QSqlQuery *query) {
    Task *pTask(0);
    Task::task_type type(taskTypeFromString(otdb_tree.processType().toStdString()));
    switch(type) {
    case Task::OBSERVATION:
        if (query) pTask = new Observation(*query, otdb_tree);
        else pTask = new Observation(0, otdb_tree);
        break;
    case Task::PIPELINE:
        switch (otdb_tree.processSubType()) {
        case PST_AVERAGING_PIPELINE:
        case PST_CALIBRATION_PIPELINE:
            if (query) pTask = new CalibrationPipeline(*query, otdb_tree);
            else pTask = new CalibrationPipeline(0, otdb_tree);
            break;
        case PST_IMAGING_PIPELINE:
        case PST_MSSS_IMAGING_PIPELINE:
            if (query) pTask = new ImagingPipeline(*query, otdb_tree);
            else pTask = new ImagingPipeline(0, otdb_tree);
            break;
        case PST_PULSAR_PIPELINE:
            if (query) pTask = new PulsarPipeline(*query, otdb_tree);
            else pTask = new PulsarPipeline(0, otdb_tree);
            break;
        case PST_LONG_BASELINE_PIPELINE:
            if (query) pTask = new LongBaselinePipeline(*query, otdb_tree);
            else pTask = new LongBaselinePipeline(0, otdb_tree);
            break;
        default:
            if (query) pTask = new Pipeline(*query, otdb_tree);
            else pTask = new Pipeline(0, otdb_tree);
            break;
        }
        break;
    case Task::RESERVATION:
    case Task::MAINTENANCE:
        if (query) pTask = new StationTask(*query, otdb_tree, type);
        else pTask = new StationTask(0, otdb_tree, type);
        break;
    case Task::SYSTEM:
    default: // unknown task processType
        QString err("Unknown processType: " + otdb_tree.processType() + " in tree ID:" + QString::number(otdb_tree.treeID()) + ". Tree not downloaded.");
        itsProgressDialog.addError(err);
        qDebug() << err;

        break;
    }
    return pTask;
}

std::pair<bool, Task *> SASConnection::getTaskFromSAS(int treeID, OTDBtree otdb_tree) {
    std::pair<bool, Task *> retVal;
    retVal.first = false;
    retVal.second = 0;
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
		QString treeIDstr(QString::number(treeID));
		if (otdb_tree.treeID() == 0) { // no OTDB_tree supplied fetch it
			query.exec("select * from gettreeinfo(" + treeIDstr + ", false)");
			if (query.next()) {
				OTDBtree tree(query);
				otdb_tree = tree;
				query.finish();
			}
		}

		//check if this is indeed a VIC tree not a template
		if (otdb_tree.type() == VIC_TREE) {
			if (query.exec("SELECT * from getSchedulerInfo(" + QString::number(treeID) + ")")) { // first try to fetch all scheduler nodes in one go
				if (query.next()) {
                    retVal.second = createNewTaskFromTree(otdb_tree, &query);
                    if (retVal.second) {
                        // now fetch the other non-scheduler-branch settings from the SAS vic tree
                        if (getSASTaskProperties(treeID, *retVal.second)) {
                            retVal.first = true;
                        }
                        else {
                            retVal.second->setReason("Warning:not all SAS tree properties could be read");
                            retVal.first = false;
                        }
                    }
					query.finish();
					return retVal;
				}
			}
			else {
                query.finish();
                retVal.second = createNewTaskFromTree(otdb_tree);
                if (retVal.second) {
                    if (!getSchedulerInfo(treeID, *retVal.second)) { // fetching all scheduler nodes in one go didn't succeed, try fetching one-by-one
                        // one of the scheduler settings could not be fetched from SAS
                        getSASTaskProperties(treeID, *retVal.second); // get other SAS task properties from the SAS VIC tree
                        retVal.second->setReason("Warning:not all SAS tree properties could be read");
                        retVal.first = false;
                    }
                    else {
                        if (getSASTaskProperties(treeID, *retVal.second)) {
                            retVal.first = true;
                        }
                        else {
                            retVal.second->setReason("Warning:not all SAS tree properties could be read");
                            retVal.first = false;
                        }
                    }
                }
				return retVal;
			}
		}
		else {
            std::cout << "requested task is not a VIC tree!" << std::endl;
		}

        return retVal;
}

void SASConnection::updateDefaultTemplates(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	Controller::theSchedulerSettings.updateDefaultTemplates();
	// also update the default dataslot node id, used for creating data slots in VIC trees.
/*
	QSqlQuery query(sasDB);
	query.exec("SELECT nodeid from getVTitemList(" + QString::number(Controller::theSchedulerSettings.getSchedulerDefaultTemplate()) + ",'DataslotInfo')");
	if (query.next()) {
		itsDataslotTemplateIDstr = query.value(0).toString();
	}
	query.finish();
*/
}


void SASConnection::clearItsSASTasks(void) {
    for (SAStasks::iterator it = itsSASTasks.begin(); it != itsSASTasks.end(); ++it) {
        delete it->second;
    }
    itsSASTasks.clear();
}

bool SASConnection::downloadAllSASTasks(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	itsSASVicTrees.clear();
    clearItsSASTasks();

	if (sasDB.isOpen()) {
		updateDefaultTemplates();
		// show the progress dialog
		itsProgressDialog.clear();
		itsProgressDialog.show();
		itsProgressDialog.setWindowTitle("Download running schedule from SAS database");
		itsProgressDialog.addText(QObject::tr("Start Download SAS schedule procedure"));
		itsProgressDialog.addText(QObject::tr("Starting inventory of tasks to download from SAS database..."));
		AstroDateTime endDate = Controller::theSchedulerSettings.getLatestSchedulingDay() + AstroTime("23:59:59");

		QDateTime prevLastDownloadDate(itsLastDownloadDate);
		updateLastDownloadDate();

		int retVal(getAllSASTasksWithinPeriod(VIC_TREE, Controller::theSchedulerSettings.getEarliestSchedulingDay(), endDate));
		if (retVal == -1) {
			itsProgressDialog.addError(QObject::tr("ERROR: could not fetch tasks from SAS. Aborting download!"));
			itsProgressDialog.enableClose();
			itsLastDownloadDate = prevLastDownloadDate;
			return false;
		}
		else if (retVal == -2) {
			itsProgressDialog.addText(QObject::tr("Warning: Not all predecessor tasks were found in the SAS database."));
		}
		size_t nrOfSASTrees = itsSASVicTrees.size();
		if (nrOfSASTrees) {
			itsProgressDialog.addText(QObject::tr("Check complete. Number of tasks to download (within scheduling period): ") +
					QString::number(nrOfSASTrees));
			itsProgressDialog.addText(QObject::tr("Downloading tasks..."));
		}
		else {
			itsProgressDialog.addText(QObject::tr("Inventory complete. There are no tasks to download from SAS within the schedule period."));
		}

		if (nrOfSASTrees) {
			// copy SAS tree's to regular task objects in itsSASTasks map
			size_t count(0);
            std::pair<bool, Task *> retVal;
            for (std::map<unsigned, OTDBtree>::const_iterator it = itsSASVicTrees.begin(); it != itsSASVicTrees.end(); ++it) {
				retVal = getTaskFromSAS(it->first, it->second);
                if (retVal.second) {
                    itsSASTasks.insert(SAStasks::value_type(it->first, retVal.second));
                }
                itsProgressDialog.setProgressPercentage((++count * 100) / nrOfSASTrees);
			}

			itsProgressDialog.addText(QObject::tr("Task download complete."));
			itsProgressDialog.addText(QObject::tr("Number of downloaded valid tasks: ") + QString::number(itsSASTasks.size()));
		}
		// now insert the downloaded tasks into the schedule if required
        if (!itsSASTasks.empty()) {
			itsProgressDialog.addText(QObject::tr("Inserting tasks in scheduler..."));
			itsController->mergeDownloadedSASTasks();
			itsProgressDialog.addText(QObject::tr("Tasks inserted."));
			itsProgressDialog.addText(QObject::tr("Download schedule from SAS completed successfully."));
		}
		itsProgressDialog.enableClose();
		return true;
	}
	else return false; // no connection to SAS database
}


void SASConnection::showProgressUploadDialog(void) {
	itsProgressDialog.clear();
	itsProgressDialog.setWindowTitle("Upload new schedule to SAS database");
	itsProgressDialog.show();
}


bool SASConnection::checkSynchronizeNeeded(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if (sasDB.isOpen()) {
		updateDefaultTemplates();
		itsProgressDialog.addText(QObject::tr("Start synchronize SAS schedule procedure"));
		itsProgressDialog.addText(QObject::tr("Checking for changes in SAS database..."));

		int retVal(getModifiedVICTrees(itsLastDownloadDate));

		if (retVal == -1) {
			itsProgressDialog.addError(QObject::tr("ERROR: could not fetch tasks from SAS. Aborting synchronize!"));
			return false;
		}

		size_t nrOfModifiedSASTrees = itsSASmodifiedTasks.size();
		if (nrOfModifiedSASTrees != 0) {
			itsProgressDialog.addText(QObject::tr("Check complete. Number of modified tasks in SAS database: ") +
					QString::number(nrOfModifiedSASTrees));
			return true;
		}
		else {
			itsProgressDialog.addText(QObject::tr("Check complete. No modified tasks needed to be downloaded."));
			return true;
		}
	}
	else {
		itsProgressDialog.addError(QObject::tr("ERROR: Not connected to SAS database. Aborting synchronize!"));
		return false;
	}
}

bool SASConnection::startSynchronizeProcedure(const SchedulerData &scheduler_data) {
	// ------------------------------- STEP 1 -------------------------------
	// Do a new download from SAS database so that any intermediate changes are also taken into account
	bool issueChangeConflict(false);

	if (checkSynchronizeNeeded()) {
		itsProgressDialog.addText(QObject::tr("Start compare with SAS database schedule..."));
		itsNewSchedulerTasks.clear();
		itsChangedTasks.clear();
		itsUploadDialog->clear(); // resets the upload dialog

		// itsSASVicTrees contains all previously downloaded trees including modifications that have just been detected
		// vic trees that don't exist anymore in SAS have been deleted already from itsSASVicTrees and
		// their tree ID have been inserted in itsSASdeletedTrees
		// itsSASmodifiedTrees contain all tasks with modifications since last download

		// ------------------------------- STEP 1 -------------------------------
		const scheduledTasksMap &scheduled_tasks = scheduler_data.getScheduledTasks();
		unsigned treeID;
		for (scheduledTasksMap::const_iterator it = scheduled_tasks.begin(); it != scheduled_tasks.end(); ++it) {
			treeID = it->second->getSASTreeID();
			if (treeID == 0) { // new task added by scheduler
				// add to new scheduler tasks
				itsNewSchedulerTasks.push_back(it->second->getID());
				itsUploadDialog->addNewSchedulerTask(*(it->second));
			}
		}
		const reservationsMap &reservations = scheduler_data.getReservations();
		for (reservationsMap::const_iterator it = reservations.begin(); it != reservations.end(); ++it) {
			treeID = it->second->getSASTreeID();
			if (treeID == 0) { // new task added by scheduler
				// add to new scheduler tasks
				itsNewSchedulerTasks.push_back(it->second->getID());
				itsUploadDialog->addNewSchedulerTask(*(it->second));
			}
		}
		const unscheduledTasksDeque &unscheduled_tasks = scheduler_data.getUnscheduledTasks();
		for (unscheduledTasksDeque::const_iterator it = unscheduled_tasks.begin(); it != unscheduled_tasks.end(); ++it) {
			treeID = (*it)->getSASTreeID();
			if (treeID == 0) { // new task added by scheduler
					// add to new scheduler tasks
					itsNewSchedulerTasks.push_back((*it)->getID());
					itsUploadDialog->addNewSchedulerTask(**it);
					// For these tasks a new tree has to be build using the SAS DB API
			}
		}
		const pipelinesMap &pipeline_tasks = scheduler_data.getPipelineTasks();
		for (pipelinesMap::const_iterator it = pipeline_tasks.begin(); it != pipeline_tasks.end(); ++it) {
			treeID = it->second->getSASTreeID();
			if (treeID == 0) { // new task added by scheduler
				// add to new scheduler tasks
				itsNewSchedulerTasks.push_back(it->second->getID());
				itsUploadDialog->addNewSchedulerTask(*(it->second));
			}
		}


		// ------------------------------- STEP 2 -------------------------------
		// check for differences that need to be uploaded to SAS

		bool difDetected;
		SAStasks::const_iterator sit;
		std::vector<unsigned> new_tasks;
        for (std::map<unsigned, OTDBtree>::const_iterator it = itsSASVicTrees.begin(); it != itsSASVicTrees.end(); ++it) {
            task_diff diff;
			const unsigned &treeID(it->first);
			if (find(itsTreesToDelete.begin(), itsTreesToDelete.end(), it->first) == itsTreesToDelete.end()) { // don't consider trees that have to be deleted here
				sit = itsSASTasks.find(treeID);
				if (sit != itsSASTasks.end()) {
					// diff between current! SAS task state (at upload time) and previous SAS task state at download time
					// to prevent changing a task that in the meantime had a status change in SAS
                    if (it->second.state() == sit->second->SASTree().state()) {
						// only allow changes to tasks with state < FINISHED
							const Task *pTask = scheduler_data.getTask(treeID, ID_SAS);
							if (pTask) {
                                difDetected = pTask->diff(sit->second, diff); // diff between task currently in scheduler memory and the previously! downloaded SAS task
								if (difDetected) {
                                    QString difstr = pTask->diffString(diff);
									if (itsSASmodifiedTasks.find(treeID) == itsSASmodifiedTasks.end()) {
										itsUploadDialog->addChangedTask(*pTask, difstr);
										itsChangedTasks.insert(changedTasks::value_type(treeID, diff));
									}
									else { // the task was also externally modified, issue change conflict
										itsUploadDialog->addChangedTask(*pTask, difstr, true);
										issueChangeConflict = true;
									}
								}
								else {
									itsUploadDialog->addUnchangedTask(*pTask);
								}
							}
					}
					else {
						// special case the tree state was changed externally in sas database
						// check if this task was also changed by the user, if not just download without warning
						// if it was changed also by the user then warning that the changes will be lost and the task will be downloaded again
//						externallyChangedTasks.push_back(it->treeID());
						const Task *pTask = scheduler_data.getTask(treeID, ID_SAS);
						if (pTask) {
                            difDetected = pTask->diff(sit->second, diff);
							if (difDetected) {
                                QString difstr = pTask->diffString(diff);
								itsUploadDialog->addChangedTask(*pTask, difstr, true);
								issueChangeConflict = true;
							}
						}
					}
				}
                else {
                    // new tree in the SAS database detected, download it in the scheduler
                    new_tasks.push_back(treeID);
                }
			}
		}


        SAStasks::iterator sasit;
        Task *pCloneTask;
		// update externally modified tasks in scheduler now
        for (std::map<unsigned, Task *>::const_iterator sit = itsSASmodifiedTasks.begin(); sit != itsSASmodifiedTasks.end(); ++sit) {
			// replace the task in itsSAStasks with the updated task from SAS
            sasit = itsSASTasks.find(sit->first);
            if (sasit != itsSASTasks.end()) {
                pCloneTask = cloneTask(sit->second);
                if (pCloneTask) {
                    delete sasit->second;
                    sasit->second = pCloneTask;
                }
            }
            itsController->synchronizeTask(sit->second);
            if (find(new_tasks.begin(), new_tasks.end(), sit->first) != new_tasks.end()) {
                itsUploadDialog->addNewSASTask(sit->second);
			}
		}

		// ------------------------------- STEP 4 -------------------------------
		// Add the trees that have to be deleted from SAS
		for (std::vector<unsigned>::const_iterator it = itsTreesToDelete.begin(); it != itsTreesToDelete.end(); ++it) {
			sasit = itsSASTasks.find(*it);
			if (sasit != itsSASTasks.end()) {
                itsUploadDialog->addDeletedSchedulerTask(sasit->second);
			}
		}

		// add the tasks that have been deleted from sas in the upload dialog
		for (std::vector<unsigned>::const_iterator dsit = itsSASdeletedTrees.begin(); dsit != itsSASdeletedTrees.end(); ++dsit) {
			const Task *pTask = scheduler_data.getTask(*dsit, ID_SAS);
            itsUploadDialog->addDeletedSASTask(pTask);
            // directly delete the task from the scheduler because the tree does not exist anymore and nothing can be done abuot that (no cancel option)
            sasit = itsSASTasks.find(*dsit);
            if (sasit != itsSASTasks.end()) {
                delete sasit->second;
                itsSASTasks.erase(sasit);
            }
            itsController->expungeTask(*dsit);
		}
		itsSASdeletedTrees.clear();

		// ALL DONE
		itsProgressDialog.addText(QObject::tr("Compare finished."));
		itsProgressDialog.hide();
		itsUploadDialog->show();

		if (issueChangeConflict) {
			QMessageBox::warning(0, QObject::tr("External changes detected"),
					QObject::tr("Some task changes cannot be applied because their status has been changed in SAS. These tasks are marked red in the changed tasks table.\nThey have been updated to their current state in SAS.\nYou will have to redo the changes you made to these tasks"));
		}

		return true;
	}
	else {
		itsProgressDialog.enableClose();
		return false;
	}
}

void SASConnection::addToTreesToDelete(unsigned treeID, unsigned task_id) {
	if (find(itsTreesToDelete.begin(), itsTreesToDelete.end(), treeID) == itsTreesToDelete.end()) {
		itsTreesToDelete.push_back(treeID);
	}
	else {
		debugWarn("sis", "task ", task_id, "was already added to the list of SAS trees to delete");
	}
}

void SASConnection::removeFromSASTaskToDelete(unsigned treeID) {
	if (treeID) {
		std::vector<unsigned>::iterator it = find(itsTreesToDelete.begin(), itsTreesToDelete.end(), treeID);
		if (it != itsTreesToDelete.end()) {
			itsTreesToDelete.erase(it);
		}
		else {
			debugWarn("sis", "tree ", treeID, "was not found in the list of SAS trees to delete");
		}
	}
}

/*
void SASConnection::addChangedIDTasks(const changedIDTasks &changedIDTasks) {
	for (changedIDTasks::const_iterator it = changedIDTasks.begin(); it != changedIDTasks.end(); ++it) {
		itsChangedIDTasks.push_back(*it);
		const Task *pTask(itsController->getTask(it->newTaskID));
		if (pTask) {
		itsProgressDialog.addText(QString("Tree ") + QString::number(pTask->getSASTreeID()) +
				" has non unique task ID " + QString::number(it->oldTaskID) + ". Task ID is changed to " + QString::number(it->newTaskID));
		}
	}
}
*/

bool SASConnection::deleteTrees(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	bool bResult(true);
	QSqlQuery query(sasDB);
	QString delquery("SELECT deleteTree(" +	itsAuthToken + ",");
	for (std::vector<unsigned>::const_iterator it = itsTreesToDelete.begin(); it != itsTreesToDelete.end(); ++it) {
		if (!query.exec(delquery + QString::number(*it) + ")")) {
			debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
					" query:", query.lastQuery().toStdString().c_str());
			bResult = false;
		}
		query.finish();
	}
	itsTreesToDelete.clear();
	return bResult;
}

bool SASConnection::deleteTreesCleanup(const deleteVICmap &treeIDs, const QString &hostname, const QString &user, const QString &password) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	for (deleteVICmap::const_iterator DBit = treeIDs.begin(); DBit != treeIDs.end(); ++DBit) {
		if (sasDB.isOpen()) {
			sasDB.close();
		}
		QSqlDatabase::removeDatabase( "SASDB" );
		sasDB = QSqlDatabase::addDatabase("QPSQL","SASDB");
		sasDB.setDatabaseName(DBit->first);
		sasDB.setHostName(hostname);
	    sasDB.setUserName("postgres");
	    sasDB.setPassword("");

		if (sasDB.open()) {
			QSqlQuery query(sasDB);

			QString authToken;
			query.exec("SELECT OTDBlogin('" + user + "','" + password + "')");
			if (query.next()) {
				authToken = query.value(0).toString();
				if (authToken.isEmpty()) {
					QMessageBox::critical(0, "no SAS write permissions", "No write permissions in sas database. Could not delete trees.\nDid you specify the correct user name and password?");
					return false; // no write permissions to SAS DB
				}
			}
			query.finish();

			QString delquery("SELECT deleteTree(" +	authToken + ",");
			for (QStringList::const_iterator it = DBit->second.begin(); it != DBit->second.end(); ++it) {
				if (!query.exec(delquery + *it + ")")) {
					debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
							" query:", query.lastQuery().toStdString().c_str());
//					bResult = false;
				}
				query.finish();
			}
		}
		else return false;
	}
	return true;
}

bool SASConnection::markDataProductsDeleted(const deletedDataMap &data, const QString &hostname/*, const QString &user, const QString &password*/) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	for (deletedDataMap::const_iterator DBit = data.begin(); DBit != data.end(); ++DBit) {
		if (sasDB.isOpen()) {
			sasDB.close();
		}
		QSqlDatabase::removeDatabase( "SASDB" );
		sasDB = QSqlDatabase::addDatabase("QPSQL","SASDB");
		sasDB.setDatabaseName(DBit->first);
		sasDB.setHostName(hostname);
	    sasDB.setUserName("postgres");
	    sasDB.setPassword("");

		if (sasDB.open()) {

			QString dpNodeName("LOFAR.ObsSW.Observation.DataProducts.");
			for (std::vector<std::pair<int, dataProductTypes> >::const_iterator treeit = DBit->second.begin(); treeit != DBit->second.end(); ++treeit) {
				switch (treeit->second) {
				case DP_CORRELATED_UV:
					dpNodeName += "Output_Correlated.deleted";
					break;
				case DP_COHERENT_STOKES:
					dpNodeName += "Output_CoherentStokes.deleted";
					break;
				case DP_INCOHERENT_STOKES:
					dpNodeName += "Output_IncoherentStokes.deleted";
					break;
				case DP_INSTRUMENT_MODEL:
					dpNodeName += "Output_InstrumentModel.deleted";
					break;
                case DP_PULSAR:
                    dpNodeName += "Output_Pulsar.deleted";
                    break;
				case DP_SKY_IMAGE:
					dpNodeName += "Output_SkyImage.deleted";
					break;
				default:
					continue;
				}
				setNodeValue(treeit->first, dpNodeName, "true");
			}
		}
		else {
			debugErr("sssssss","Could not establish SAS connection! Settings used:\n",
					"Username: ", sasDB.userName().toStdString().c_str(),
					", database: ", sasDB.databaseName().toStdString().c_str(),
					", hostname: ", sasDB.hostName().toStdString().c_str()
			);
			return false;
		}
	}
	return true;
}



// getItemList(int aTreeID, const string& aNameFragment): get children of a tree by means of a name fragment
// this function was ported from LOFAR software 'TreeMaintenance::getItemList (treeIDType aTreeID, const string& aNameFragment)'
vector<OTDBnode> SASConnection::getItemList(int aTreeID, const QString &nameFragment) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	vector<OTDBnode> resultVec;
	QSqlQuery query("SELECT * from getVHitemList(" + QString::number(aTreeID) + ",'" + nameFragment + "')", sasDB);
	while (query.next()) {
		resultVec.push_back(OTDBnode(aTreeID, query));
	}
	return resultVec;
}

QVariant SASConnection::getNodeValue(int treeID, const QString &nameFragment, bool noWarnings) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT limits from getVHitemList(" + QString::number(treeID) + ",'" + nameFragment + "')", sasDB);
	if (query.next()) {
		return query.value(0);
	}
	else {
		if (!noWarnings) {
			itsProgressDialog.addError(QString("Warning: property:") + nameFragment + " of SAS tree:" + QString::number(treeID) + " could not be read from SAS.");
		}
		return QVariant(QVariant::Invalid);
	}
}

bool SASConnection::getDigitalBeams(int treeID, Observation &observation) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	bool bResult(true);
	int idx(0);
	QString beamName;
	QString propertyName;
	while(true) {
		beamName = "LOFAR.ObsSW.Observation.Beam[" + QString::number(idx) + "]";
		if (query.exec("SELECT name,limits from getVHitemList(" + QString::number(treeID) + ",'" + beamName + ".%')")) {
			if (query.size()) {
				DigitalBeam digitalBeam;
				std::map<unsigned, TiedArrayBeam> &TAB(digitalBeam.getTiedArrayBeamsForChange());
				while (query.next()) {
					propertyName = query.value(0).toString().remove(beamName + ".");
					if (propertyName.startsWith("Tie")) { // TiedArrayBeam
						// get tied array beam number
						int p1(propertyName.indexOf('[')+1), p2(propertyName.indexOf("]."));
						if (p1 != 0 && p2 != -1) {
							int TABnr(propertyName.mid(p1,p2-p1).toInt());
							propertyName = propertyName.mid(p2+2);
                            if (propertyName.startsWith("angle1")) {
								TAB[TABnr].setAngle1(query.value(1).toDouble());
							}
							else if (propertyName.startsWith("angle2")) {
								TAB[TABnr].setAngle2(query.value(1).toDouble());
							}
							else if (propertyName.startsWith("co")) { // coherent
								TAB[TABnr].setCoherent(query.value(1).toBool());
							}
							else if (propertyName.startsWith("dis")) { // dispersionMeasure
								TAB[TABnr].setDispersionMeasure(query.value(1).toDouble());
							}
						}
					}
					else if (propertyName.startsWith("angle1")) {
						digitalBeam.setAngle1Radian(query.value(1).toDouble());
					}
					else if (propertyName.startsWith("angle2")) {
						digitalBeam.setAngle2Radian(query.value(1).toDouble());
					}
					else if (propertyName.startsWith("dir")) { // directionType
						digitalBeam.setDirectionType(stringToBeamDirectionType(query.value(1).toString().toStdString()));
						switch (digitalBeam.directionType()) {
						default:
						case DIR_TYPE_J2000: // Right ascension & declination
						case DIR_TYPE_B1950:
						case DIR_TYPE_ICRS:
						case DIR_TYPE_ITRF:
						case DIR_TYPE_TOPO:
						case DIR_TYPE_APP:
						case DIR_TYPE_HADEC:
							digitalBeam.setUnits(ANGLE_PAIRS_HMS_DMS);
							break;
						case DIR_TYPE_AZELGEO:
						case DIR_TYPE_SUN:
						case DIR_TYPE_MOON:
						case DIR_TYPE_PLUTO:
						case DIR_TYPE_NEPTUNE:
						case DIR_TYPE_URANUS:
						case DIR_TYPE_SATURN:
						case DIR_TYPE_JUPITER:
						case DIR_TYPE_MARS:
						case DIR_TYPE_VENUS:
						case DIR_TYPE_MERCURY:
						case DIR_TYPE_GALACTIC:
						case DIR_TYPE_ECLIPTIC:
						case DIR_TYPE_COMET:
							digitalBeam.setUnits(ANGLE_PAIRS_DMS_DMS);
							break;
						}
					}
					else if (propertyName.startsWith("dur")) { // duration
						digitalBeam.setDuration(query.value(1).toUInt());
					}
					else if (propertyName.startsWith("nrTa")) { // nrTabRings
						digitalBeam.setNrTabRings(query.value(1).toInt());
					}
					else if (propertyName.startsWith("sta")) { // startTime
						digitalBeam.setStartTime(query.value(1).toUInt());
					}
					else if (propertyName.startsWith("sub")) { // subbandList
						QString subliststr(query.value(1).toString());
						if (!digitalBeam.setSubbandList(subliststr)) {
							itsProgressDialog.addError(QString("Warning: The subband list of tree: ") + QString::number(treeID) + " for beam: " + QString::number(idx) + " contains an error.");
							bResult = false;
						}
						if (Vector2StringList(digitalBeam.subbandList()) != subliststr) digitalBeam.setSubbandNotationChange(true); // mark for saving to SAS
					}
					else if (propertyName.startsWith("tab")) { // tabRingSize
						digitalBeam.setTabRingSize(query.value(1).toDouble());
					}
					else if (propertyName.startsWith("tar")) { // target
						digitalBeam.setTarget(query.value(1).toString().toStdString());
					}
				}
                observation.setDigitalBeam(idx++, digitalBeam);
			}
			else return bResult;
			query.finish();
		}
	}
	return bResult;
}

bool SASConnection::saveTiedArrayBeamSettings(int treeID, const Observation &task, const task_diff *diff) {
	bool bResult(true);
	if (diff) {
		if (diff->tiedarray_beam_settings) {
			const std::map<unsigned, DigitalBeam> &digiBeams = task.getDigitalBeams();
			for (std::map<unsigned, DigitalBeam>::const_iterator it = digiBeams.begin(); it != digiBeams.end(); ++it) {
				const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(it->second.tiedArrayBeams());
				for (std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.begin(); tit != tiedArrayBeams.end(); ++tit) {
					if (!saveTiedArrayBeamToSasVicTree(treeID, it->first, tit->first, tit->second)) bResult = false;
				}
			}
		}
	}
	return bResult;
}

bool SASConnection::saveTiedArrayBeamToSasVicTree(int treeID, int beamNr, int TABnr, const TiedArrayBeam &TAB) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	int nodeID;
	QString compNameShort, isCoherent;
	QString compName("LOFAR.ObsSW.Observation.Beam[" + QString::number(beamNr) + "].TiedArrayBeam[" + QString::number(TABnr) + "].");
	QString fetchTABidStr("SELECT nodeid,name from getVHitemList(" + QString::number(treeID) + ",'" + compName + "%')");
	if (query.exec(fetchTABidStr)) {
		while (query.next()) {
			nodeID = query.value(0).toInt();
			compNameShort = query.value(1).toString().remove(compName);
			if (compNameShort.startsWith("angle1")) {
				if (!setNodeValue(treeID, nodeID, QString::number(TAB.angle1(),'g',16))) bResult = false;
			}
			else if (compNameShort.startsWith("angle2")) {
				if (!setNodeValue(treeID, nodeID, QString::number(TAB.angle2(),'g',16))) bResult = false;
			}
			else if (compNameShort.startsWith("dis")) { // dispersionMeasure
				if (!setNodeValue(treeID, nodeID, QString::number(TAB.dispersionMeasure(),'g',16))) bResult = false;
			}
			else if (compNameShort.startsWith("coh")) { // coherent
				TAB.isCoherent() ? isCoherent = "true" : isCoherent = "false";
				if (!setNodeValue(treeID, nodeID, isCoherent)) bResult = false;
			}
		}
		query.finish();
	}
	else {
        std::cerr << query.lastError().text().toStdString() << std::endl;
		return false;
	}

	return bResult;
}


bool SASConnection::saveDigitalBeamSettings(int treeID, Observation &task, const task_diff *diff) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	const std::map<unsigned, DigitalBeam> &digiBeams = task.getDigitalBeams();
	bool bResult(true), doSubbandChangeCheck(false);
	if (diff) {
		if (diff->digital_beam_settings) {
			for (std::map<unsigned, DigitalBeam>::const_iterator it = digiBeams.begin(); it != digiBeams.end(); ++it) {
				if (!saveDigitalBeamToSasVicTree(treeID, it->first, it->second)) bResult = false;
			}
		}
		else doSubbandChangeCheck = true;
	}
	else doSubbandChangeCheck = true;

	if (doSubbandChangeCheck) { // check each beam if the notation of the subbands was changed, if it was then write the new notation to SAS
		QString queryFirstPart("SELECT nodeid from getVHitemList(" + QString::number(treeID) + ",'LOFAR.ObsSW.Observation.Beam[");
		for (std::map<unsigned, DigitalBeam>::const_iterator it = digiBeams.begin(); it != digiBeams.end(); ++it) {
			if (it->second.subbandNotationChange()) {
				// set subbandList
				query.exec(queryFirstPart + QString::number(it->first) + "].subbandList')");
				if (query.next()) {
					if (setNodeValue(treeID, query.value(0).toInt(), Vector2StringList(it->second.subbandList()))) {
						task.setSubbandNotationChange(it->first, false); // reset the subband notation change flag of this beam once it has been written back to SAS
					}
					else bResult = false;
				}
				else bResult = false;
				query.finish();
			}
		}
	}

	return bResult;
}


bool SASConnection::saveDigitalBeamToSasVicTree(int treeID, int beamNr, const DigitalBeam &digiBeam) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	QString beamstr = "SELECT nodeid from getVHitemList(" + QString::number(treeID) + ",'LOFAR.ObsSW.Observation.Beam[" + QString::number(beamNr) + "].";
	// set angle 1 value
	query.exec(beamstr + "angle1')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.angle1().radian(),'g',16))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set angle 2
	query.exec(beamstr + "angle2')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.angle2().radian(),'g',16))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set directionType
	query.exec(beamstr + "directionType')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), BEAM_DIRECTION_TYPES[digiBeam.directionType()])) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set duration
	query.exec(beamstr + "duration')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.duration().totalSeconds()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set nrTabRings
	query.exec(beamstr + "nrTabRings')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.nrTabRings()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set startTime
	query.exec(beamstr + "startTime')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.startTime().totalSeconds()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set subbandList
	query.exec(beamstr + "subbandList')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), Vector2StringList(digiBeam.subbandList()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set tabRingSize
	query.exec(beamstr + "tabRingSize')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.tabRingSize(),'g',16))) bResult = false;
	}
	else bResult = false;
	// set subbandList
	query.exec(beamstr + "target')");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), digiBeam.target().c_str())) return false;
	}
	else bResult = false;
	query.finish();

	return bResult;
}

bool SASConnection::saveAnalogBeamToSAStemplate(int treeID, int beamNodeId, const Observation::analogBeamSettings &analogBeam) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	QString beamStr = "SELECT nodeid from victemplate WHERE parentid=" + QString::number(beamNodeId) + " AND name='";
	// set angle 1 value
	query.exec(beamStr + "angle1'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(analogBeam.angle1.radian(),'g',16))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set angle 2
	query.exec(beamStr + "angle2'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(analogBeam.angle2.radian(),'g',16))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set directionType
	query.exec(beamStr + "directionType'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), BEAM_DIRECTION_TYPES[analogBeam.directionType])) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set duration
	query.exec(beamStr + "duration'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(analogBeam.duration.totalSeconds()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set startTime
	query.exec(beamStr + "startTime'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(analogBeam.startTime.totalSeconds()))) bResult = false;
	}
	else bResult = false;
	query.finish();

	return bResult;
}


bool SASConnection::saveTiedArrayBeamToSAStemplate(int treeID, const QString &TABNodeID, const TiedArrayBeam &TAB) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
//	QString TABNode(QString::number(TABNodeId));
	// save angle 1 value
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + TABNodeID + " AND name='angle1'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(TAB.angle1()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// save angle 2
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + TABNodeID + " AND name='angle2'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(TAB.angle2()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// save coherent flag
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + TABNodeID + " AND name='coherent'");
	if (query.next()) {
		QString isCoherent;
		TAB.isCoherent() ?  isCoherent = "true" : isCoherent = "false";
		if (!setNodeValue(treeID, query.value(0).toInt(), isCoherent)) bResult = false;
	}
	else bResult = false;
	query.finish();
	// save dispersion measure
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + TABNodeID + " AND name='dispersionMeasure'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(TAB.dispersionMeasure()))) bResult = false;
	}
	else bResult = false;
	query.finish();

	return bResult;
}



bool SASConnection::saveDigitalBeamToSAStemplate(int treeID, const QString &beamNodeId, const DigitalBeam &digiBeam) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
//	QString beamNode(QString::number(beamNodeId));
	// set angle 1 value
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='angle1'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.angle1().radian()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set angle 2
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='angle2'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.angle2().radian()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set directionType
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='directionType'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), BEAM_DIRECTION_TYPES[digiBeam.directionType()])) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set duration
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='duration'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.duration().totalSeconds()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set startTime
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='startTime'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.startTime().totalSeconds()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// nrTABrings
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='nrTabRings'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.nrTabRings()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set subbandList
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='subbandList'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), Vector2StringList(digiBeam.subbandList()))) bResult = false;
	}
	else bResult = false;
	query.finish();
	// set subbandList
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='target'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), digiBeam.target().c_str())) bResult = false;
	}
	else bResult = false;
	query.finish();
	// tabRingSize
	query.exec("SELECT nodeid from victemplate WHERE parentid=" + beamNodeId + " AND name='tabRingSize'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), QString::number(digiBeam.tabRingSize(),'g',16))) bResult = false;
	}
	else bResult = false;
	query.finish();


	return bResult;
}

/*
std::vector<unsigned> SASConnection::subbandStringToVector(const QString &subbandStr) const {
	std::vector<unsigned> subbandList;
	QStringList strList;
	QString tmp;
	if (!subbandStr.isEmpty()) {
		tmp = subbandStr.trimmed();
		if (tmp.size() > 2) {
			if ((tmp[0] == '[') & (tmp[tmp.size()-1] == ']')) {
				tmp = tmp.mid(1,tmp.size()-2);
				strList = tmp.split(',',QString::SkipEmptyParts);
				// we should now have a stringlist containing either individual subband numbers or a single ranges of subbands expressed as n..m
				int pos;
				unsigned sbstart, sbend;
				for (QStringList::const_iterator it = strList.begin(); it != strList.end(); ++it) {
					if ((pos = it->indexOf("..")) != -1) { // this is a range
						sbstart = it->left(pos).toUInt();
						sbend = it->mid(pos+2).toUInt();
						for (unsigned sb = sbstart; sb <= sbend; ++sb) {
							subbandList.push_back(sb);
						}
					}
					else subbandList.push_back(it->toUInt());
				}
			}
		}
	}
	return subbandList;
}
*/


// getSchedulerInfo returns false only for serious errors
bool SASConnection::getSchedulerInfo(int tree_id, Task &task) {
	bool bResult = true;
	QString treeID(QString::number(tree_id));
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);

	// task description
//	task.setTaskDescription(SAS_tree.description());
//	task.setSASTreeID(SAS_tree.treeID());
//    task.setSASTree(SAS_tree);
//	task.setType(SAS_tree.processType(), SAS_tree.processSubType(), SAS_tree.strategy());

	// duration, start and stop time
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskDuration')");
	if (query.next()) {
		unsigned secondsDuration = query.value(0).toUInt();
		if (secondsDuration) {
			AstroTime duration;
			duration = duration.addSeconds(secondsDuration);
			task.setDuration(duration);
		}
	}
	else {
		itsProgressDialog.addError(QString("Error: Scheduler.taskDuration node of SAS tree: ") + treeID + " could not be fetched");
		bResult = false;
	}
	query.finish();

/*
	bool startSet(SAS_tree.startTime().isSet()), endSet(SAS_tree.stopTime().isSet());
	if (startSet && endSet) {
		task.setScheduledPeriod(SAS_tree.startTime(), SAS_tree.stopTime());
	}
	else if (!startSet & !endSet) { // this is true when the task has just been approved from MoM and no start and end times are defined
		query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskDuration')");
		if (query.next()) {
			unsigned secondsDuration = query.value(0).toUInt();
			if (secondsDuration) {
				AstroTime duration;
				duration = duration.addSeconds(secondsDuration);
				task.setDuration(duration);
			}
		}
		else {
			itsProgressDialog.addError(QString("Error: Scheduler.taskDuration node of SAS tree: ") + treeID + " could not be fetched");
			bResult = false;
		}
		query.finish();
	}
	else if (startSet & !endSet) {
		query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskDuration')");
		if (query.next()) {
			unsigned secondsDuration = query.value(0).toUInt();
			if (secondsDuration) {
				AstroTime duration;
				duration = duration.addSeconds(secondsDuration);
				task.setDuration(duration);
			}
		}
		else {
			itsProgressDialog.addError(QString("Warning: Scheduler.taskDuration node of SAS tree: ") + treeID + " could not be fetched");
			bResult = false;
		}
		query.finish();
	}
	else if (!startSet & endSet) {
		query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskDuration')");
		if (query.next()) {
			unsigned secondsDuration = query.value(0).toUInt();
			if (secondsDuration) {
				AstroTime duration;
				task.setDuration(duration.addSeconds(secondsDuration));
				task.setScheduledStart(SAS_tree.stopTime() - duration);
			}
			else {
				task.setScheduledStart(SAS_tree.stopTime());
				task.setDuration(AstroTime(0));
			}
		}
		else {
			itsProgressDialog.addError(QString("Warning: Scheduler.taskDuration node of SAS tree: ") + treeID + " could not be fetched");
			bResult = false;
		}
		query.finish();
	}
*/

	// status is set according to the SAS status from SAS_tree
//	task.setStatusSAS(SAS_tree.state());

    // TODO: remove backwards compatible task type, this is replaced by processType meta-data
    // taskType
    if (task.getType() == Task::UNKNOWN) {
        debugWarn("sis", "task type for tree ", tree_id, " is UNKNOWN. Probably caused by processType meta-data not set in SAS VIC-tree. Now fetching Scheduler.taskType node");
        query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskType')");
        if (query.next()) { task.setType(static_cast<Task::task_type>(query.value(0).toUInt())); }
        else {
            itsProgressDialog.addError(QString("Warning: Scheduler.taskName node of SAS tree: ") + treeID + " could not be fetched");
        }
        query.finish();
    }

	// contactEmail
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.contactEmail')");
	if (query.next()) { task.setContactEmail(query.value(0).toString().toStdString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.contactEmail node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();
	// contactName
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.contactName')");
	if (query.next()) { task.setContactName(query.value(0).toString().toStdString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.contactName node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// contactPhone
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.contactPhone')");
	if (query.next()) { task.setContactPhone(query.value(0).toString().toStdString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.contactPhone node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// firstPossibleDay
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.firstPossibleDay')");
	if (query.next()) {
		int day = query.value(0).toInt();
		if (day) task.setWindowFirstDay(day);
        else task.setWindowFirstDay(std::max(QDate::currentDate().toJulianDay() - J2000_EPOCH, (qint64)Controller::theSchedulerSettings.getEarliestSchedulingDay().toJulian()));
	}
	else { // serious error
		itsProgressDialog.addError(QString("Error: Scheduler.firstPossibleDay node of SAS tree: ") + treeID + " could not be fetched");
		bResult = false;
	}
	query.finish();

	// fixedDay
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.fixedDay')");
	if (query.next()) { task.setFixDay(query.value(0).toBool()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.fixedDay node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// fixedTime
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.fixedTime')");
	if (query.next()) { task.setFixTime(query.value(0).toBool()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.fixedTime node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// lastPossibleDay
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.lastPossibleDay')");
	if (query.next()) {
		int day = query.value(0).toInt();
		if (day) task.setWindowLastDay(day);
		else task.setWindowLastDay(Controller::theSchedulerSettings.getLatestSchedulingDay().toJulian());
	} else {
		itsProgressDialog.addError(QString("Error: Scheduler.lastPossibleDay node of SAS tree: ") + treeID + " could not be fetched");
		bResult = false;
	}
	query.finish();

	// predecessor
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.predecessors')");
	if (query.next()) { task.setPredecessors(query.value(0).toString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.predecessors node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// predMaxTimeDif
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.predMaxTimeDif')");
	if (query.next()) { task.setPredecessorMaxTimeDif(query.value(0).toString().toStdString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.predMaxTimeDif node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// predMinTimeDif
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.predMinTimeDif')");
	if (query.next()) { task.setPredecessorMinTimeDif(query.value(0).toString().toStdString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.predMinTimeDif node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// priority
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.priority')");
	if (query.next()) { task.setPriority(query.value(0).toDouble()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.priority node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// projectName
//	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.projectName')");
//	if (query.next()) { task.setProjectName(query.value(0).toString().toStdString()); }
//	else {
//		itsProgressDialog.addError(QString("Warning: Scheduler.projectName node of SAS tree: ") + treeID + " could not be fetched");
//	}
//	query.finish();

    if (task.isObservation()) {
        Observation &obsTask(static_cast<Observation &>(task));
        // nightTimeWeightFactor
        query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.nightTimeWeightFactor')");
        if (query.next()) { obsTask.setNightTimeWeightFactor(query.value(0).toInt()); }
        else {
            itsProgressDialog.addError(QString("Warning: Scheduler.nightTimeWeightFactor node of SAS tree: ") + treeID + " could not be fetched");
        }
        query.finish();

        // reservation
        query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.reservation')");
        if (query.next()) { obsTask.setReservation(query.value(0).toUInt()); }
        else {
            itsProgressDialog.addError(QString("Warning: Scheduler.reservation node of SAS tree: ") + treeID + " could not be fetched");
        }
        query.finish();
    }

    TaskStorage *tStorage(task.storage());
    if (tStorage) {
        // storageSelectionMode
        query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.storageSelectionMode')");
        if (query.next()) { tStorage->setStorageSelectionMode(static_cast<storage_selection_mode>(query.value(0).toInt())); }
        else {
            itsProgressDialog.addError(QString("Warning: Scheduler.storageSelectionMode node of SAS tree: ") + treeID + " could not be fetched");
        }
        query.finish();
    }

	// taskID
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskID')");
	if (query.next()) {
		unsigned taskID(query.value(0).toUInt());
		if (taskID) {
			task.setID(taskID);
		}
//		else {
//			itsProgressDialog.addError(QString("Warning: Scheduler.taskID node of SAS tree: ") + treeID + " is zero (0)");
//		}
	}
	else {
		itsProgressDialog.addError(QString("Error: Scheduler.taskID node of SAS tree: ") + treeID + " could not be fetched");
		bResult = false;
	}
	query.finish();

	// taskName
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskName')");
	if (query.next()) { task.setTaskName(query.value(0).toString().toStdString()); }
	else {
		itsProgressDialog.addError(QString("Warning: Scheduler.taskName node of SAS tree: ") + treeID + " could not be fetched");
	}
	query.finish();

	// error reason (fetched only when task status == ERROR)
	if (task.getStatus() == Task::ERROR) {
		query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.reason')");
		if (query.next()) { task.setReason(query.value(0).toString().toStdString()); }
		else {
			itsProgressDialog.addError(QString("Warning: Scheduler.reason node of SAS tree: ") + treeID + " could not be fetched");
		}
		query.finish();
	}

	// windowMaximumTime
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.windowMaximumTime')");
	if (query.next()) {
		task.setWindowMaxTime(query.value(0).toString());
	}
	else {
		task.setWindowMaxTime(AstroTime("23:59:59"));
		itsProgressDialog.addError(QString("Error: Scheduler.windowMaximumTime node of SAS tree: ") + treeID + " could not be fetched");
		bResult = false;
	}
	query.finish();

	// windowMinimumTime
	query.exec("SELECT limits from getVHitemList(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.windowMinimumTime')");
	if (query.next()) {
		task.setWindowMinTime(query.value(0).toString());
	}
	else {
		task.setWindowMinTime(AstroTime());
		itsProgressDialog.addError(QString("Error: Scheduler.windowMinimumTime node of SAS tree: ") + treeID + " could not be fetched");
		bResult = false;
	}

	return bResult;
}

bool SASConnection::saveSchedulerProperties(int treeID, const Task &task, const task_diff *diff) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QString strValue;
	if (diff) {
		if (diff->task_id) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskID", QString::number(task.getID()));
		if (diff->contact_email) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.contactEmail", task.getContactEmail());
		if (diff->contact_name) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.contactName", task.getContactName());
		if (diff->contact_phone) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.contactPhone", task.getContactPhone());
		if (diff->first_possible_day) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.firstPossibleDay", QString::number(task.getWindowFirstDay().toJulian()));
		if (diff->fixed_day) {
			task.getFixedDay() ? strValue = "true" : strValue = "false";
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.fixedDay", strValue);
		}
		if (diff->fixed_time) {
			task.getFixedTime() ? strValue = "true" : strValue = "false";
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.fixedTime", strValue);
		}
		if (diff->last_possible_day) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.lastPossibleDay", QString::number(task.getWindowLastDay().toJulian()));
        if (diff->predecessors) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.predecessors", task.getPredecessorsString());
		if (diff->pred_max_time_dif) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.predMaxTimeDif", task.getPredecessorMaxTimeDif().toString().c_str());
		if (diff->pred_min_time_dif) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.predMinTimeDif", task.getPredecessorMinTimeDif().toString().c_str());
		if (diff->priority) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.priority", QString::number(task.getPriority()));
		if (task.getStatus() == Task::ERROR) {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.reason", task.getReason().c_str());
		}
		if (diff->task_duration) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskDuration", QString::number(task.getDuration().totalSeconds()));
		if (diff->task_name) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskName", task.getTaskName());
		if (diff->task_type) {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskType", QString::number(static_cast<int>(task.getType())));
		}
		if (diff->window_maximum_time) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.windowMaximumTime", task.getWindowMaxTime().toString(1).c_str());
		if (diff->window_minimum_time) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.windowMinimumTime", task.getWindowMinTime().toString(1).c_str());
        if (task.isObservation()) {
            const Observation &obs(static_cast<const Observation &>(task));
            if (diff->reservation) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.reservation", QString::number(obs.getReservation()));
            if (diff->night_time_weight_factor) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.nightTimeWeightFactor", QString::number(obs.getNightTimeWeightFactor()));
        }
        const TaskStorage *tStor(task.storage());
        if (tStor) {
            if (diff->output_storage_settings) bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.storageSelectionMode", QString::number(static_cast<int>(tStor->getStorageSelectionMode())));
        }
	}
	else {
		QString fixedDay, fixedTime, late;
		task.getFixedDay() ? fixedDay = "true" : fixedDay = "false";
		task.getFixedTime() ? fixedTime = "true" : fixedTime = "false";
        QString reservationNr, ntimefac, storMode;
        if (task.isObservation()) {
            const Observation &obs(static_cast<const Observation &>(task));
            reservationNr = QString::number(obs.getReservation());
            ntimefac = QString::number(obs.getNightTimeWeightFactor());
        }
        const TaskStorage *task_storage(task.storage());
        if (task_storage) {
            storMode = QString::number(static_cast<int>(task_storage->getStorageSelectionMode()));
        }

		QSqlQuery query("SELECT saveSchedulerInfo(" +
				itsAuthToken + "," +
				QString::number(treeID) + ",'" +
				task.getContactEmail() + "','" +
				task.getContactName() + "','" +
				task.getContactPhone() + "','" +
				QString::number(task.getWindowFirstDay().toJulian()) + "','" +
				fixedDay + "','" +
				fixedTime + "','" +
				QString::number(task.getWindowLastDay().toJulian()) + "','" +
				late + "','" +
                ntimefac + "','" +
                task.getPredecessorsString() + "','" +
				task.getPredecessorMaxTimeDif().toString().c_str() + "','" +
				task.getPredecessorMinTimeDif().toString().c_str() + "','" +
				QString::number(task.getPriority()) + "','" +
                task.getReason().c_str() + "','J2000','" +
                reservationNr + "','" +
                storMode + "','" +
				QString::number(task.getDuration().totalSeconds()) + "','" +
				QString::number(task.getID()) + "','" +
				task.getTaskName() + "','" +
				QString::number(static_cast<int>(task.getType())) + "','" + // TODO: Task type is not being update to the database (is this a saveSchedulerInfo.sql function error?)
				task.getWindowMaxTime().toString(1).c_str() + "','" +
				task.getWindowMinTime().toString(1).c_str() + "')"
				, sasDB);

		if (query.next()) {
			if (!query.value(query.record().indexOf("saveSchedulerInfo")).toBool()) {
				debugWarn("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
						" query:", query.lastQuery().toStdString().c_str());
				return false;
			}
		}
		else {
			// save all properties manually
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.contactEmail", task.getContactEmail());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.contactName", task.getContactName());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.contactPhone", task.getContactPhone());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.firstPossibleDay", QString::number(task.getWindowFirstDay().toJulian()));
			task.getFixedDay() ? strValue = "true" : strValue = "false";
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.fixedDay", strValue);
			task.getFixedTime() ? strValue = "true" : strValue = "false";
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.fixedTime", strValue);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.lastPossibleDay", QString::number(task.getWindowLastDay().toJulian()));
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.late", strValue);
            bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.predecessors", task.getPredecessorsString());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.predMaxTimeDif", task.getPredecessorMaxTimeDif().toString().c_str());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.predMinTimeDif", task.getPredecessorMinTimeDif().toString().c_str());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.priority", QString::number(task.getPriority()));
			if (task.getStatus() == Task::ERROR) {
				bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.reason", task.getReason().c_str());
			}
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskDuration", QString::number(task.getDuration().totalSeconds()));
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskName", task.getTaskName());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.taskType", QString::number(static_cast<int>(task.getType())));
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.windowMaximumTime", task.getWindowMaxTime().toString(1).c_str());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.windowMinimumTime", task.getWindowMinTime().toString(1).c_str());
            if (task.isObservation()) {
                bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.reservation", reservationNr);
                bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.nightTimeWeightFactor", ntimefac);
            }
            if (task_storage) {
                bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.Scheduler.storageSelectionMode", storMode);
            }


            return bResult;
		}
	}
	return true;
}

QString SASConnection::getTreeParset(int treeID) {
	if (connect() == 0) {
	QString parset;
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
    QSqlQuery query(sasDB);
	if (query.exec("SELECT nodeid from getVHitemList(" + QString::number(treeID) + ",'LOFAR')")) {
		if (query.next()) {
			QString topNodeID = query.value(0).toString();
			query.finish();
            if (query.exec("SELECT * from exportTree(" + itsAuthToken + "," +
					QString::number(treeID) + "," + topNodeID + ")")) {
				if (query.next()) {
					parset = query.value(0).toString();
					if (parset.isEmpty()) {
						debugWarn("si","SAS returned empty tree (exportTree function) for tree:", treeID);
					}
				}
				query.finish();
            }
		}
	}
	return parset;
	}
	else {
		QMessageBox::critical(0, QObject::tr("No connection to SAS"),
				QObject::tr("Could not connect to SAS database.\n Please check SAS connection settings."));
		return "";
	}
}

QString SASConnection::getMetaData(int treeID) {
    QString parset;
    if (connect() == 0) {
        QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
        QSqlQuery query(sasDB);
        if (query.exec("SELECT * from exportmetadata(" + QString::number(treeID) + ")")) {
            if (query.next()) {
                parset = query.value(0).toString();
                if (parset.isEmpty()) {
                    debugWarn("si","SAS empty meta data (exportMetaData function) for tree:", treeID);
                }
            }
            query.finish();
        }
    }
    else {
        QMessageBox::critical(0, QObject::tr("No connection to SAS"),
                              QObject::tr("Could not connect to SAS database.\n Please check SAS connection settings."));
    }

    return parset;
}

bool SASConnection::abortTask(int treeID) {
	// get the current status of the tree from SAS
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("select state from gettreeinfo(" + QString::number(treeID) + ", false)", sasDB);
	if (query.next()) {
		int state = query.value(0).toInt();
		if ((state == SAS_STATE_ACTIVE) | (state == SAS_STATE_QUEUED)) {
			//if the status is SAS_STATE_QUEUED or SAS_STATE_ACTIVE then allow the abort
			return setTreeState(treeID, SAS_STATE_ABORTED);
		}
		else {
			debugWarn("sis","SASConnection:abortTask, tree:", treeID, " is not queued nor active, it could not be aborted");
			return false;
		}
	}
	else {
        std::cerr << "SASConnection::abortTask: Error, could not get the tree state of tree: " << treeID << std::endl << query.lastError().text().toStdString() << std::endl;
		return false;
	}
}

bool SASConnection::setTaskOnHold(int treeID) {
	if (setTreeState(treeID, SAS_STATE_ON_HOLD)) {
		SAStasks::iterator it = itsSASTasks.find(treeID);
		if (it != itsSASTasks.end()) {
            it->second->setStatus(Task::ON_HOLD);
		}
		std::map<unsigned, OTDBtree>::iterator sit = itsSASVicTrees.find(treeID);
		if (sit != itsSASVicTrees.end()) {
            sit->second.setState(SAS_STATE_ON_HOLD);
		}
		return true;
	}
	else return false;
}

bool SASConnection::setTasksOnHold(const std::vector<int> trees) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if (sasDB.open()) {
		for (std::vector<int>::const_iterator it = trees.begin(); it != trees.end(); ++it) {
			bResult &= setTaskOnHold(*it);
		}
	}
	else return false;

	return bResult;
}

OTDBnode SASConnection::getNode(int treeID, const QString &nodeID) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT * from getNode(" +
				QString::number(treeID) + "," +
				nodeID + ")", sasDB);
	if (query.next()) {
		return OTDBnode(treeID, query);
	}
	else return OTDBnode();
}

bool SASConnection::saveNode(const OTDBnode &node) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT updateVTnode(" +
			itsAuthToken + "," +
			QString::number(node.treeID()) + "," +
			QString::number(node.nodeID()) + ",'" +
			QString::number(node.instances()) + "','" +
			node.limits().c_str() + "')", sasDB);
//	std::cout << query.lastQuery().toStdString() << std::endl;
	if (!query.isActive()) { // query successfully executed?
		debugWarn("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
				" query:", query.lastQuery().toStdString().c_str());
		return false;
	}
	return true;
}

bool SASConnection::setTemplateNodeByID(int treeID, const QString &parentNodeID, const QString &nodeName, const QString &valueStr) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	query.exec(QString("SELECT nodeid from victemplate WHERE parentid=") + parentNodeID + " AND name='" + nodeName + "'");
	if (query.next()) {
		if (!setNodeValue(treeID, query.value(0).toInt(), valueStr)) return false;
	}
	else {
        std::cerr << "SASConnection::setTemplateNodeValue: Error" << std::endl << query.lastError().text().toStdString() << std::endl;
		return false;
	}
	return true;
}

bool SASConnection::setTemplateNodeByName(int treeID, const QString &parentNodeName, const QString &nodeName, const QString &valueStr) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	query.exec("SELECT nodeid from getVTitemList(" + QString::number(treeID) + ",'" + parentNodeName + "')");
	if (query.next()) {
		int nodeID = query.value(0).toInt();
		query.finish();
		query.exec(QString("SELECT nodeid from victemplate WHERE parentid=") + QString::number(nodeID) + " AND name='" + nodeName + "'");
		if (query.next()) {
			if (!setNodeValue(treeID, query.value(0).toInt(), valueStr)) return false;
		}
		else {
            std::cerr << "SASConnection::setTemplateNodeValue: Error" << std::endl << query.lastError().text().toStdString() << std::endl;
			return false;
		}
	}
	else {
		debugErr("sssi", "SASConnection::setTemplateNodeValue: parent node '", parentNodeName.toStdString().c_str(), "' not found for template tree: ", treeID);
        std::cerr << "SASConnection::setTemplateNodeValue: Error" << std::endl << query.lastError().text().toStdString() << std::endl;
		return false;
	}
	return true;
}


bool SASConnection::setNodeValue(int treeID, const QString &nameFragment, const QString &valueStr, bool warnings) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	vector<OTDBnode> fieldList = getItemList(treeID, nameFragment);
	if (fieldList.size() == 1) { // should be unique property
		QSqlQuery query("SELECT updateVTnode(" +
				itsAuthToken + "," +
				QString::number(treeID) + "," +
				QString::number(fieldList.front().nodeID()) + ",'1','" +
				valueStr
				+ "')", sasDB);
		if (!query.isActive()) { // query successfully executed?
			debugWarn("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
					" query:", query.lastQuery().toStdString().c_str());
			return false;
		}
	}
	else if (warnings) {
		itsLastErrorString += QString("Non unique node '") + nameFragment + "' for tree:" + QString::number(treeID) + "\n";
		return false;
	}
	else return false;

	return true;
}

bool SASConnection::setNodeValue(int treeID, int nodeID, const QString &valueStr) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT updateVTnode(" +
			itsAuthToken + "," +
			QString::number(treeID) + "," +
			QString::number(nodeID) + ",'1','" +
			valueStr
			+ "')", sasDB);
	if (!query.isActive()) { // query successfully executed?
		debugWarn("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
				" query:", query.lastQuery().toStdString().c_str());
		return false;
	}

	return true;
}

bool SASConnection::getStationSettings(int treeID, StationTask &task) {
	bool bResult(true);
	QVariant value;
	// station list
	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.VirtualInstrument.stationList");
	if (value.isValid()) {
		task.setStations(value.toString(),',');
	}
	else bResult = false;
	// TODO: get station beamformers (super stations)

	// get the station antennamode
	if (task.getType() != Task::MAINTENANCE) {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaSet");
		if (value.isValid()) {
			task.setSASAntennaMode(value.toString().toStdString());
		}
		else bResult = false;

		// get station RCU filter type
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.bandFilter");
		if (value.isValid()) {
			task.setSASFilterType(value.toString().toStdString());
		}
		else bResult = false;

		// get station clock frequency
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.clockMode");
		if (value.isValid()) {
            if (value.toString().toStdString().compare("<<Clock160") == 0) {
				task.setStationClock(clock_160Mhz);
			}
			else {
				task.setStationClock(clock_200Mhz);
			}
		}
		else bResult = false;

		// TBB piggybacking allowed
        if (task.isObservation()) {
            Observation &obs = static_cast<Observation &>(task);
            value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl.tbbPiggybackAllowed");
            if (value.isValid()) {
                obs.setTBBPiggybackAllowed(value.toBool());
            }
            else { // tbbPiggybackAllowed flag could not be read, this is not a severe error
                std::cerr << "could not read TBB piggyback flag for tree:" << treeID << std::endl;
            }
            // Aartfaac piggybacking allowed
            value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl.aartfaacPiggybackAllowed");
            if (value.isValid()) {
                obs.setAartfaacPiggybackAllowed(value.toBool());
            }
            else { // aartfaacPiggybackAllowed flag could not be read, this is not a severe error
                std::cerr << "could not read Aartfaac piggyback flag for tree:" << treeID << std::endl;
            }
        }
	}

	return bResult;
}

bool SASConnection::saveStationSettings(int treeID, const StationTask &task, const task_diff *diff) {
	bool bResult(true);

	// set the list of stations in SAS
	if (diff) {
		if (diff->station_ids) {
			QString stations(task.getSASStationList().c_str());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.VirtualInstrument.stationList", stations);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl._hostname", stations);
		}
		// TODO: set station beamformers (super stations)

		// set the station antennamode
		if (diff->antenna_mode) {
			QString antennaSet(task.getSASAntennaSet());
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaSet", antennaSet);
			if (antennaSet.startsWith("HBA")) {
				bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaArray", "HBA");
			}
			else {
				bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaArray", "LBA");
			}
		}
		// set station RCU filter type
		if (diff->filter_type)
		bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.bandFilter", task.getSASFilterType());

		// set station clock frequency
		if (diff->clock_frequency) {
            if (task.getStationClock() == clock_160Mhz) {
				//clock mode
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.clockMode","<<Clock160")) return false;
				// samplesPerSecond
				if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.samplesPerSecond", STR_CLOCK160_SAMPLESPERSECOND)) return false;
				// systemClock
				if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.sampleClock", STR_CLOCK160_SAMPLECLOCK)) return false;
			}
			else {
                if (!setNodeValue(treeID,"LOFAR.ObsSW.Observation.clockMode","<<Clock200")) return false;
				// samplesPerSecond
				if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.samplesPerSecond", STR_CLOCK200_SAMPLESPERSECOND)) return false;
				// systemClock
				if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.sampleClock", STR_CLOCK200_SAMPLECLOCK)) return false;
			}
		}
		// nr of dataslots per RSP board (called 'nrSlotsInFrame' in SAS)
//		if (diff->nrOfDataslotsPerRSPBoard)
//			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.nrSlotsInFrame", QString::number(task.getNrOfDataslotsPerRSPboard()));

        const Observation *obs = dynamic_cast<const Observation *>(&task);
        if (obs) {
            if (diff->TBBPiggybackAllowed)
                bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl.tbbPiggybackAllowed",
                                        (obs->getTBBPiggybackAllowed() ? "true" : "false"));
            if (diff->AartfaacPiggybackAllowed)
                 bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl.aartfaacPiggybackAllowed",
                                         (obs->getAartfaacPiggybackAllowed() ? "true" : "false"));
        }
	}
	else {
		QString stations(task.getSASStationList().c_str());
		bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.VirtualInstrument.stationList", stations);
		bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl._hostname", stations);
		// TODO: set station beamformers (super stations)

		// set the station antennamode
		QString antennaSet(task.getSASAntennaSet());
		bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaSet", antennaSet);
		if (antennaSet.startsWith("HBA")) {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaArray", "HBA");
		}
		else {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.antennaArray", "LBA");
		}
		// set station RCU filter type
		bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.bandFilter", task.getSASFilterType());

		// set station clock frequency
        if (task.getStationClock() == clock_160Mhz) {
			//clock mode
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.clockMode","<<Clock160")) return false;
			// samplesPerSecond
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.samplesPerSecond", STR_CLOCK160_SAMPLESPERSECOND)) return false;
			// systemClock
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.sampleClock", STR_CLOCK160_SAMPLECLOCK)) return false;
		}
		else {
            if (!setNodeValue(treeID,"LOFAR.ObsSW.Observation.clockMode","<<Clock200")) return false;
			// samplesPerSecond
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.samplesPerSecond", STR_CLOCK200_SAMPLESPERSECOND)) return false;
			// systemClock
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.sampleClock", STR_CLOCK200_SAMPLECLOCK)) return false;
		}
		// nr of dataslots per RSP board (called 'nrSlotsInFrame' in SAS)
//		bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.nrSlotsInFrame", QString::number(task.getNrOfDataslotsPerRSPboard()));
        //This is nonsense code as diff = NULL here, it somehow works under certain compilers because the dynamic_cast fails (obs=NULL)
		// TBB piggyback allowed?
        //const Observation *obs = dynamic_cast<const Observation *>(&task);
        //if (obs) {
        //    if (diff->TBBPiggybackAllowed)
        //        bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl.tbbPiggybackAllowed",
        //                                (obs->getTBBPiggybackAllowed() ? "true" : "false"));
        //    if (diff->AartfaacPiggybackAllowed)
        //         bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.StationControl.aartfaacPiggybackAllowed",
        //                                 (obs->getAartfaacPiggybackAllowed() ? "true" : "false"));
        //}
    }

	return bResult;

}


bool SASConnection::saveInputStorageSettings(int treeID, const Task &task) {
    bool bResult(true);
    const TaskStorage *task_storage(task.storage());
    if (task_storage) {
        const std::map<dataProductTypes, TaskStorage::inputDataProduct> &inputDataProducts(task_storage->getInputDataProducts());
        std::map<dataProductTypes, TaskStorage::inputDataProduct>::const_iterator flit;
        QString locationsStr, filenamesStr, skipVectorStr, enabledStr;

        for (dataProductTypes dp = _BEGIN_DATA_PRODUCTS_ENUM_; dp < _END_DATA_PRODUCTS_ENUM_-1; dp = dataProductTypes(dp + 1)) {
            flit = inputDataProducts.find(dp);
            enabledStr = task_storage->isInputDataProduktEnabled(dp) ? "true" : "false";
            if (flit != inputDataProducts.end()) {
                locationsStr = "[" + flit->second.locations.join(",") + "]";
                filenamesStr = "[" + flit->second.filenames.join(",") + "]";
                skipVectorStr = boolVector2StringVector(flit->second.skip);
                switch (dp) {
                case DP_COHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.skip", skipVectorStr)) bResult = false;
                    break;
                case DP_INCOHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.skip", skipVectorStr)) bResult = false;
                    break;
                case DP_CORRELATED_UV:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.skip", skipVectorStr)) bResult = false;
                    break;
                case DP_INSTRUMENT_MODEL:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.skip", skipVectorStr)) bResult = false;
                    break;
                case DP_SKY_IMAGE:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.skip", skipVectorStr)) bResult = false;
                    break;
                default:
                    break;
                }
            }
            else {
                switch (dp) {
                case DP_COHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.filenames", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.skip", "[]")) bResult = false;
                    break;
                case DP_INCOHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.filenames", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.skip", "[]")) bResult = false;
                    break;
                case DP_CORRELATED_UV:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.filenames", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.skip", "[]")) bResult = false;
                    break;
                case DP_INSTRUMENT_MODEL:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.filenames", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.skip", "[]")) bResult = false;
                    break;
                case DP_SKY_IMAGE:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.enabled", enabledStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.filenames", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.skip", "[]")) bResult = false;
                    break;
                default:
                    break;
                }
            }
        }
    }
	return bResult;
}

bool SASConnection::saveOutputStorageSettings(int treeID, const Task &task, const task_diff *diff) {
    bool bResult(true);
    if (task.getOutputDataproductCluster() == "CEP4") { //For CEP4 we're skipping this. /AR
        return bResult;
    }
    const TaskStorage *task_storage(task.storage());
    if (task_storage) {
        QString trueStr("true"), falseStr("false");
        // which output data to generate
        const TaskStorage::enableDataProdukts &odp(task_storage->getOutputDataProductsEnabled());
        if (diff) {
            if (diff->output_data_types) {
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.enabled", (odp.correlated ? trueStr : falseStr))) bResult = false;
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.enabled", (odp.coherentStokes ? trueStr : falseStr))) bResult = false;
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.enabled", (odp.incoherentStokes ? trueStr : falseStr))) bResult = false;
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.enabled", (odp.instrumentModel ? trueStr : falseStr))) bResult = false;
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.enabled", (odp.pulsar ? trueStr : falseStr))) bResult = false;
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.enabled", (odp.skyImage ? trueStr : falseStr))) bResult = false;
            }
            if (diff->output_data_products) {
                bResult &= saveOutputDataProducts(treeID, task);
            }
        }
        else {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.enabled", (odp.correlated ? trueStr : falseStr))) bResult = false;
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.enabled", (odp.coherentStokes ? trueStr : falseStr))) bResult = false;
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.enabled", (odp.incoherentStokes ? trueStr : falseStr))) bResult = false;
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.enabled", (odp.instrumentModel ? trueStr : falseStr))) bResult = false;
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.enabled", (odp.pulsar ? trueStr : falseStr))) bResult = false;
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.enabled", (odp.skyImage ? trueStr : falseStr))) bResult = false;
            bResult &= saveOutputDataProducts(treeID, task);
        }
    }
    return bResult;
}

bool SASConnection::saveOutputDataProducts(int treeID, const Task &task) {
    bool bResult(true);
    const TaskStorage *task_storage(task.storage());
    if (task_storage) {
        QString mountPointsStr, locationsStr, filenamesStr, skipVectorStr;
        const std::map<dataProductTypes, TaskStorage::outputDataProduct> &outputDataProdukt(task_storage->getOutputDataProducts());
        bool testMode(Controller::theSchedulerSettings.getIsTestEnvironment());
        Task::task_type type(task.getType());
        std::map<dataProductTypes, TaskStorage::outputDataProduct>::const_iterator flit;
        for (dataProductTypes dp = _BEGIN_DATA_PRODUCTS_ENUM_; dp < _END_DATA_PRODUCTS_ENUM_-1; dp = dataProductTypes(dp + 1)) {
            flit = outputDataProdukt.find(dp); //flit = file list iterator
            if (flit != outputDataProdukt.end()) {
                if (task_storage->isOutputDataProduktAssigned(dp)) {
                    // compile the vector strings for SAS
                    locationsStr = "[" + flit->second.locations.join(",") + "]";
                    filenamesStr = "[" + flit->second.filenames.join(",") + "]";
                    mountPointsStr = "[" + flit->second.uniqueMointPoints.join(",") + "]";
                    skipVectorStr = boolVector2StringVector(flit->second.skip);
                }
                else {
                    locationsStr = "[]";
                    filenamesStr = "[]";
                    mountPointsStr = "[]";
                    skipVectorStr = "[]";
                }

                switch (dp) {
                case DP_COHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.mountpoints", mountPointsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.filenames", filenamesStr)) bResult = false;
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.skip", skipVectorStr)) bResult = false;
                    }
                    if (testMode) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.namemask", NAMEMASK_COHERENT_STOKES_TEST)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.dirmask", DIRMASK_COHERENT_STOKES_TEST)) bResult = false;
                    }
                    else {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.namemask", NAMEMASK_COHERENT_STOKES)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.dirmask", DIRMASK_COHERENT_STOKES)) bResult = false;
                    }
                    break;
                case DP_INCOHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.mountpoints", mountPointsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.filenames", filenamesStr)) bResult = false;
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.skip", skipVectorStr)) bResult = false;
                    }
                    if (testMode) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.namemask", NAMEMASK_INCOHERENT_STOKES_TEST)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.dirmask", DIRMASK_INCOHERENT_STOKES_TEST)) bResult = false;
                    }
                    else {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.namemask", NAMEMASK_INCOHERENT_STOKES)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.dirmask", DIRMASK_INCOHERENT_STOKES)) bResult = false;
                    }
                    break;
                case DP_CORRELATED_UV:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.mountpoints", mountPointsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.filenames", filenamesStr)) bResult = false;
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.skip", skipVectorStr)) bResult = false;
                    }
                    if (testMode) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.namemask", NAMEMASK_CORRELATED_TEST)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.dirmask", DIRMASK_CORRELATED_TEST)) bResult = false;
                    }
                    else {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.namemask", NAMEMASK_CORRELATED)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.dirmask", DIRMASK_CORRELATED)) bResult = false;
                    }
                    break;
                case DP_INSTRUMENT_MODEL:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.mountpoints", mountPointsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.skip", skipVectorStr)) bResult = false;
                    if (testMode) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.namemask", NAMEMASK_INSTRUMENT_MODEL_TEST)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.dirmask", DIRMASK_INSTRUMENT_MODEL_TEST)) bResult = false;
                    }
                    else {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.namemask", NAMEMASK_INSTRUMENT_MODEL)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.dirmask", DIRMASK_INSTRUMENT_MODEL)) bResult = false;
                    }
                    break;
                case DP_PULSAR:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.mountpoints", mountPointsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.skip", skipVectorStr)) bResult = false;
                    if (testMode) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.namemask", NAMEMASK_PULSAR_TEST)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.dirmask", DIRMASK_PULSAR_TEST)) bResult = false;
                    }
                    else {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.namemask", NAMEMASK_PULSAR)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.dirmask", DIRMASK_PULSAR)) bResult = false;
                    }
                    break;
                case DP_SKY_IMAGE:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.locations", locationsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.mountpoints", mountPointsStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.filenames", filenamesStr)) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.skip", skipVectorStr)) bResult = false;
                    if (testMode) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.namemask", NAMEMASK_SKY_IMAGE_TEST)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.dirmask", DIRMASK_SKY_IMAGE_TEST)) bResult = false;
                    }
                    else {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.namemask", NAMEMASK_SKY_IMAGE)) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.dirmask", DIRMASK_SKY_IMAGE)) bResult = false;
                    }
                    break;
                default:
                    continue;
                }
            }
            else { // write empty key values in the SAS database for this unused dataproduct, to make sure no old storage locations stay behind in SAS
                switch (dp) {
                case DP_COHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.mountpoints", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.filenames", "[]")) bResult = false;
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.skip", "[]")) bResult = false;
                    }
                    break;
                case DP_INCOHERENT_STOKES:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.mountpoints", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.filenames", "[]")) bResult = false;
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.skip", "[]")) bResult = false;
                    }
                    break;
                case DP_CORRELATED_UV:
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.locations", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.mountpoints", "[]")) bResult = false;
                    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.filenames", "[]")) bResult = false;
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.skip", "[]")) bResult = false;
                    }
                    break;
                case DP_INSTRUMENT_MODEL: // always pipeline
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.locations", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.mountpoints", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.filenames", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.skip", "[]")) bResult = false;
                    }
                    break;
                case DP_PULSAR: // always pipeline
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.locations", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.mountpoints", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.filenames", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.skip", "[]")) bResult = false;
                    }
                    break;
                case DP_SKY_IMAGE: // always pipeline
                    if (type == Task::PIPELINE) {
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.locations", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.mountpoints", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.filenames", "[]")) bResult = false;
                        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.skip", "[]")) bResult = false;
                    }
                    break;
                default:
                    continue;
                }
            }
        }
    }
    return bResult;
}


void SASConnection::getCampaignInfo(Task &task) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	string campaign = task.SASTree().campaign();
	task.setProjectID(campaign);
	QSqlQuery query(sasDB);
	// we have to use QSqlQuery prepare and bindValue functions to automatically escape the campaign name which may contain special characters such as apostrophs
	query.prepare("SELECT * from getcampaign(:campaignName)");
	query.bindValue(":campaignName",  QString(campaign.c_str()));
	if (query.exec()) {
		if (query.next()) {
			task.setProjectName(query.value(query.record().indexOf("title")).toString().toStdString());
			task.setContactName(query.value(query.record().indexOf("contact")).toString().toStdString());
			task.setProjectCO_I(query.value(query.record().indexOf("co_i")).toString().toStdString());
			task.setProjectPI(query.value(query.record().indexOf("pi")).toString().toStdString());
		}
	}
	else {
		itsProgressDialog.addError(QString("Error: The campaign info of SAS tree: ") + QString::number(task.SASTree().treeID()) + " could not be read");
	}
}

void SASConnection::getInputStorageSettings(int treeID, Task &task) {
    TaskStorage *task_storage(task.storage());
    if (task_storage) {
        QVariant valueVector, enabledKey;
        QString keyPrefix;
        bool enabledValue(false);
        std::vector<dataProductTypes> fetchDataProducts;
        if (task.isPipeline()) {
            processSubTypes pst(task.getProcessSubtype());
            if (pst == PST_IMAGING_PIPELINE || pst == PST_MSSS_IMAGING_PIPELINE || pst == PST_AVERAGING_PIPELINE || pst == PST_LONG_BASELINE_PIPELINE) {
                fetchDataProducts.push_back(DP_CORRELATED_UV);
            }
            else if (pst == PST_CALIBRATION_PIPELINE) {
                fetchDataProducts.push_back(DP_CORRELATED_UV);
                fetchDataProducts.push_back(DP_INSTRUMENT_MODEL);
            }
            else if (pst == PST_PULSAR_PIPELINE) {
                fetchDataProducts.push_back(DP_COHERENT_STOKES);
                fetchDataProducts.push_back(DP_INCOHERENT_STOKES);
            }
            else {
                fetchDataProducts.push_back(DP_COHERENT_STOKES);
                fetchDataProducts.push_back(DP_INCOHERENT_STOKES);
                fetchDataProducts.push_back(DP_CORRELATED_UV);
                fetchDataProducts.push_back(DP_INSTRUMENT_MODEL);
            }
        }
        else if (task.isObservation()) { // only fetch input data product types relevant for the task type
            return; // observation has no input data products
        }
        else if (task.isUnknown()) {
            fetchDataProducts.push_back(DP_CORRELATED_UV);
            fetchDataProducts.push_back(DP_COHERENT_STOKES);
            fetchDataProducts.push_back(DP_INCOHERENT_STOKES);
            fetchDataProducts.push_back(DP_INSTRUMENT_MODEL);
            //		fetchDataProducts.push_back(DP_SKY_IMAGE);
        }
        for (std::vector<dataProductTypes>::const_iterator dpit = fetchDataProducts.begin(); dpit != fetchDataProducts.end(); ++dpit) {
            switch (*dpit) {
            case DP_COHERENT_STOKES:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Input_CoherentStokes.";
                break;
            case DP_CORRELATED_UV:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Input_Correlated.";
                break;
            case DP_INCOHERENT_STOKES:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Input_IncoherentStokes.";
                break;
            case DP_INSTRUMENT_MODEL:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Input_InstrumentModel.";
                break;
            case DP_SKY_IMAGE:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Input_SkyImage.";
                break;
            default:
                break;
            }
            // now fetch the identifications from SAS and add them to the task as an input data product identification
            enabledKey = getNodeValue(treeID, keyPrefix + "enabled");
            if (enabledKey.isValid()) enabledValue = enabledKey.toBool();
            task_storage->setInputDataProductEnabled(*dpit, enabledValue);
            QStringList identificationsList, filenames, locations;
            std::vector<bool> skipVector;

            valueVector = getNodeValue(treeID, keyPrefix + "identifications");
            if (valueVector.isValid()) {
                identificationsList = valueVector.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);
                if (!identificationsList.empty()) {
                    task_storage->addInputDataProductID(*dpit, identificationsList);
                }
            }
            valueVector = getNodeValue(treeID, keyPrefix + "filenames");
            if (valueVector.isValid()) {
                filenames = valueVector.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);
            }
            valueVector = getNodeValue(treeID, keyPrefix + "locations");
            if (valueVector.isValid()) {
                locations = valueVector.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);
            }
            valueVector = getNodeValue(treeID, keyPrefix + "skip");
            if (valueVector.isValid()) {
                skipVector = string2VectorOfBools(valueVector.toString());
            }
            if (filenames.size() == locations.size()) {
                if (filenames.size() > 0) {
                    task_storage->setInputFilesForDataProduct(*dpit, filenames, locations, skipVector);
                }
            }
            else {
                itsProgressDialog.addError(QString("tree:") + QString::number(treeID) + " input data product type:" + DATA_PRODUCTS[*dpit] + " filenames and locations array have unequal size");
            }
        }
    }
}

void SASConnection::getOutputStorageSettings(int treeID, Task &task) {
    TaskStorage *task_storage(task.storage());
    if (task_storage) {
        QStringList nodeList, raidList;
        QString storageLocationsKey, keyPrefix;
        QString outputCluster; // Added to support CEP2/4 switch /AR
        QVariant enabledKey;
        bool enabledValue;
        task_storage->unAssignStorage(); // clear the tasks storage, we will be adding incrementally (Task::setStorage() doesn't delete existing storage locations
        std::vector<dataProductTypes> fetchDataProducts;
        if (task.isObservation()) { // only fetch output data product types relevant for the task type
            fetchDataProducts.push_back(DP_CORRELATED_UV);
            fetchDataProducts.push_back(DP_COHERENT_STOKES);
            fetchDataProducts.push_back(DP_INCOHERENT_STOKES);
            //		fetchDataProducts.push_back(DP_INSTRUMENT_MODEL);
            //		fetchDataProducts.push_back(DP_SKY_IMAGE);
        }
        else if (task.isPipeline()) {
            processSubTypes pst(task.getProcessSubtype());
            if (pst == PST_CALIBRATION_PIPELINE) {
                fetchDataProducts.push_back(DP_CORRELATED_UV);
                fetchDataProducts.push_back(DP_INSTRUMENT_MODEL);
            }
            else if (pst == PST_AVERAGING_PIPELINE || pst == PST_LONG_BASELINE_PIPELINE) {
                fetchDataProducts.push_back(DP_CORRELATED_UV);
            }
            else if (pst == PST_IMAGING_PIPELINE || pst == PST_MSSS_IMAGING_PIPELINE) {
                fetchDataProducts.push_back(DP_SKY_IMAGE);
            }
            else if (pst == PST_PULSAR_PIPELINE) {
                fetchDataProducts.push_back(DP_PULSAR);
            }
        }
        else if (task.isUnknown()) {
            fetchDataProducts.push_back(DP_CORRELATED_UV);
            fetchDataProducts.push_back(DP_COHERENT_STOKES);
            fetchDataProducts.push_back(DP_INCOHERENT_STOKES);
            fetchDataProducts.push_back(DP_INSTRUMENT_MODEL);
            fetchDataProducts.push_back(DP_PULSAR);
            fetchDataProducts.push_back(DP_SKY_IMAGE);
        }
        for (std::vector<dataProductTypes>::const_iterator dpit = fetchDataProducts.begin(); dpit != fetchDataProducts.end(); ++dpit) {
            nodeList.clear();
            raidList.clear();
            switch (*dpit) {
            case DP_COHERENT_STOKES:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Output_CoherentStokes.";
                break;
            case DP_CORRELATED_UV:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Output_Correlated.";
                break;
            case DP_INCOHERENT_STOKES:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Output_IncoherentStokes.";
                break;
            case DP_INSTRUMENT_MODEL:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Output_InstrumentModel.";
                break;
            case DP_PULSAR:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Output_Pulsar.";
                break;
            case DP_SKY_IMAGE:
                keyPrefix = "LOFAR.ObsSW.Observation.DataProducts.Output_SkyImage.";
                break;
            default:
                continue;
            }
            enabledKey = getNodeValue(treeID, keyPrefix + "enabled");
            if (enabledKey.isValid()) {
                enabledValue = enabledKey.toBool();
                task_storage->setOutputDataProductEnabled(*dpit, enabledValue);
            }

            storageLocationsKey = keyPrefix + "mountpoints";
            QVariant value;

            value = getNodeValue(treeID, storageLocationsKey);
            if (value.isValid()) {
                QStringList storageLocationList = value.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);//string2VectorOfStrings(value.toString());
                if (!storageLocationList.empty()) {
                    // parse the long location string array into separate lists of nodes and raid arrays
                    for (QStringList::iterator it = storageLocationList.begin(); it != storageLocationList.end(); ++it)
                    {
                        nodeList.append(it->section(':',0,0)); // extracts lseXXX from "lseXXX:/dataY"
                        raidList.append(it->section('/',1,1,QString::SectionIncludeLeadingSep)); // extracts dataY from "lseXXX:/dataY"
                    }
                    if (!task_storage->setStorage(*dpit, nodeList, raidList)) {
                        itsProgressDialog.addError(QString("Warning: The storage settings of SAS tree: ") + QString::number(treeID) + " has non existing storage nodes or raid sets.");
                    }
                    task_storage->setOutputFileMountpoints(*dpit, storageLocationList);
                    if (task.getStatus() >= Task::SCHEDULED) {
                        task_storage->setOutputDataProductAssigned(*dpit, true);
                    }
                }
                else {
                    task_storage->clearOutputFileMountpoints(*dpit);
                }
            }

            QStringList filenames, locations;
            std::vector<bool> skipVector;

            value = getNodeValue(treeID, keyPrefix + "filenames");
            if (value.isValid()) {
                filenames = value.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);//string2VectorOfStrings(value.toString());
            }

            value = getNodeValue(treeID, keyPrefix + "locations");
            if (value.isValid()) {
                locations = value.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);//string2VectorOfStrings(value.toString());
            }

            value = getNodeValue(treeID, keyPrefix + "skip");
            if (value.isValid()) {
                skipVector = string2VectorOfBools(value.toString());
            }

            if (filenames.size() == locations.size()) {
                if (skipVector.size() != (unsigned)filenames.size()) {
                    skipVector.assign(filenames.size(), false);
                }
                task_storage->setOutputFilesForDataProduct(*dpit, filenames, locations, skipVector);
            }
            else {
                itsProgressDialog.addError(QString("tree:") + QString::number(treeID) + " output data product type:" + DATA_PRODUCTS[*dpit] + " filenames and locations array have unequal size");
            }

            // also set the output data product identifications
            value = getNodeValue(treeID, keyPrefix + "identifications");
            if (value.isValid()) {
                QStringList identificationsList = value.toString().remove('[').remove(']').split(',',QString::SkipEmptyParts);// string2VectorOfStrings(value.toString());
                task_storage->addOutputDataProductID(*dpit, identificationsList);
            }

            // get values for storage cluster /AR
            value = getNodeValue(treeID, keyPrefix + "storageClusterName");
            if (value.isValid()) {
                QString cluster = value.toString();
                if (!cluster.isEmpty()) {
                    if (outputCluster.isEmpty()) {
                        outputCluster = cluster;
                    }
                    else {
                        if (cluster != outputCluster) {
                            itsProgressDialog.addError(QString("tree:") + QString::number(treeID) + " output data product type:" + DATA_PRODUCTS[*dpit] + " different output clusters are not supported");
                        }
                    }
                }
                //We probably will not need to support this: task_storage->addOutputDataProductCluster(*dpit, ?); /AR
            }
        }
        if (!outputCluster.isEmpty()) {
            task.setOutputDataproductCluster(outputCluster);
        }
        else {
            task.setOutputDataproductCluster("CEP2"); // CEP2 is default for backward compatibility /AR
        }
    }
}

bool SASConnection::setTreeSchedule(int treeID, const Task &task) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if ((task.getScheduledStart().isSet()) && (task.getScheduledEnd().isSet())) {
		QSqlQuery query("SELECT setSchedule(" +
				itsAuthToken + "," +
				QString::number(treeID) + ",'" +
				task.getScheduledStart().toSASDateTimeString().c_str() + "','" +
				task.getScheduledEnd().toSASDateTimeString().c_str() + "','true')", sasDB);
		if (query.isActive()) { // query successfully executed?
			query.finish();
			return true;
		}
		else {
			debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
					" query:", query.lastQuery().toStdString().c_str());
			return false;
		}
	}
	return true;
}

bool SASConnection::setTreeState(int treeID, int SAS_state) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT setTreeState(" +
			itsAuthToken + "," +
			QString::number(treeID) + ",'" +
			QString::number(SAS_state) + "')", sasDB);
	if (query.isActive()) { // query successfully executed?
		query.finish();
		return true;
	}
	else {
		debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
				" query:", query.lastQuery().toStdString().c_str());
		return false;
	}
}

bool SASConnection::saveMoMinfo(int treeID, const Task &task) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if (!task.SASTree().campaign().empty()) {
	QSqlQuery query("SELECT setmominfo(" +
			itsAuthToken + "," +
			QString::number(treeID) + "," + QString::number(task.SASTree().momID()) +
			"," + QString::number(task.SASTree().groupID()) +
			",'" + task.SASTree().campaign().c_str() + "')", sasDB);
//	std::cout << query.lastQuery().toStdString() << std::endl;
//	std::cout << query.lastError().text().toStdString() << std::endl;

	if (query.isActive()) { // query successfully executed?
		query.finish();
		return true;
	}
	else {
		debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
				" query:", query.lastQuery().toStdString().c_str());
		return false;
	}
	}
	else {
		debugWarn("sis", "Could not save mom info to SAS for tree:", treeID, " because campaign is empty");
		return true;
	}
}




bool SASConnection::saveDescription(int treeID, const Task &task) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("SELECT setDescription(" +
			itsAuthToken + "," +
			QString::number(treeID) + ",'" +
			task.SASTree().description().c_str() + "')", sasDB);
	if (query.isActive()) { // query successfully executed?
		query.finish();
		return true;
	}
	else {
		debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
				" query:", query.lastQuery().toStdString().c_str());
		return false;
	}
}

bool SASConnection::saveAnalogBeamSettings(int treeID, const Observation &task, const task_diff *diff) {
	bool bResult(true);
	// analog beam
    const Observation::analogBeamSettings &analogBeam = task.getAnalogBeam();
	if (diff) {
		// analog beam angle 1 and 2
		if (diff->ana_beam_angle1)
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].angle1", QString::number(analogBeam.angle1.radian()))) bResult = false;
		if (diff->ana_beam_angle2)
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].angle2", QString::number(analogBeam.angle2.radian()))) bResult = false;
		// analog beam coordinate system
		if (diff->ana_beam_direction_type)
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].directionType", BEAM_DIRECTION_TYPES[analogBeam.directionType])) bResult = false;
		// analog beam start time
		if (diff->ana_beam_starttime)
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].startTime", QString::number(analogBeam.startTime.totalSeconds()))) bResult = false;
		// analog beam duration
		if (diff->ana_beam_duration)
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].duration", QString::number(analogBeam.duration.totalSeconds()))) bResult = false;
	}
	else {
		// analog beam angle 1 and 2
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].angle1", QString::number(analogBeam.angle1.radian()))) bResult = false;
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].angle2", QString::number(analogBeam.angle2.radian()))) bResult = false;
		// analog beam coordinate system
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].directionType", BEAM_DIRECTION_TYPES[analogBeam.directionType])) bResult = false;
		// analog beam start time
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].startTime", QString::number(analogBeam.startTime.totalSeconds()))) bResult = false;
		// analog beam duration
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].duration", QString::number(analogBeam.duration.totalSeconds()))) bResult = false;
	}
	return bResult;
}

bool SASConnection::getAnalogBeamSettings(int treeID, Observation &task) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QVariant value;

	QSqlQuery query("SELECT nodeid from getVHitemList(" + QString::number(treeID) + ",'LOFAR.ObsSW.Observation.AnaBeam[0]')", sasDB);
	if (query.next()) {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].angle1");
        if (value.isValid()) task.setAnalogBeamAngle1(value.toDouble());
		else bResult = false;

		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].angle2");
        if (value.isValid()) task.setAnalogBeamAngle2(value.toDouble());
		else bResult = false;

		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].directionType");
        if (value.isValid()) task.setAnalogBeamDirectionType(stringToBeamDirectionType(value.toString().toStdString()));
		else bResult = false;

		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].duration");
        if (value.isValid()) task.setAnalogBeamDuration(value.toInt());
		else bResult = false;

		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.AnaBeam[0].startTime");
        if (value.isValid()) task.setAnalogBeamStartTime(value.toInt());
		else bResult = false;
	}
	return bResult;
}


bool SASConnection::saveDataSlots(int treeID, const Observation &task) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	const dataSlotMap &dataslots = task.getDataSlots();
	QString strParentDataslotNodeID, antennaFieldName, antennaFieldName2, stationName, RSPBoardList, DataSlotList, antennaField(task.getAntennaField().c_str());
	// empty existing dataslot fields in SAS
	// TODO: SASConnection::saveDataSlots, should be a delete node for VIC trees to delete the old nodes completely

	// get the node ID of the parent node (Observation.Dataslots node) in the VIC tree under which the dataslot info needs to be saved
	if (query.exec("SELECT nodeid from getVHitemList(" + QString::number(treeID) + ",'LOFAR.ObsSW.Observation.Dataslots')")) {
		if (query.next()) {
			strParentDataslotNodeID = query.value(0).toString();
		}
	}
	else {
		debugErr("sis","The Dataslots node doesn't exist in the VIC tree ", treeID, ", Cannot save dataslot info to this tree");
		return false;
	}
	query.finish();

	// get the node ID of the dataslots node of the (default) template used for this task
	QString dataSlotTemplateIDstr;
//	if (itsDataslotTemplateIDstr.isEmpty()) {
	query.exec("SELECT nodeid from getVTitemList(" + QString::number(task.getOriginalTreeID()) + ",'DataslotInfo')");
	if (query.next()) {
		dataSlotTemplateIDstr = query.value(0).toString();
	}
	else {
		unsigned pid(Controller::theSchedulerSettings.getSchedulerDefaultTemplate());
		if (pid != 0) {
            std::cerr << "parent default template for tree " << treeID << " does not exist anymore. Using the scheduler default template to create the DataslotInfo nodes." << std::endl;
			query.exec("SELECT nodeid from getVTitemList(" + QString::number(pid) + ",'DataslotInfo')");
			if (query.next()) {
				dataSlotTemplateIDstr = query.value(0).toString();
			}
		}
		else {
            std::cerr << "could not write the DataslotInfo for tree:" << treeID << " because the parent template:" << task.getOriginalTreeID() << " does not exist" << std::endl;
			bResult = false;
		}
	}
	query.finish();
//	}
//	else {
//		dataSlotTemplateIDstr = itsDataslotTemplateIDstr;
//	}

	if (!dataSlotTemplateIDstr.isEmpty()) {
		for (dataSlotMap::const_iterator it = dataslots.begin(); it != dataslots.end(); ++it) { // iterates over all stations (antennafields)
			stationName = Controller::theSchedulerSettings.getStationName(it->first).c_str();
			if (stationName.startsWith("CS")) { // core station has HBA0 and HBA1
				if (antennaField.compare("HBA_DUAL") == 0) { // antenna field is HBA_DUAL for both HBA_DUAL and HBA_DUAL_INNER antenna modes
					antennaFieldName = stationName + "HBA0";
					antennaFieldName2 =  stationName + "HBA1";
				}
				else {
					antennaFieldName = stationName + antennaField;
					antennaFieldName2 = "";
				}
			}
			else { // non core station only has HBA (no HBA0 nor HBA1)
				if (antennaField.startsWith("HBA")) {
					antennaFieldName = stationName + "HBA";
					antennaFieldName2 = "";
				}
				else {
					antennaFieldName = stationName + antennaField;
					antennaFieldName2 = "";
				}
			}
			RSPBoardList = "[";
			DataSlotList = "[";
			for (stationDataSlotMap::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {// iterates over the RSPboards and assigned dataslots
				RSPBoardList += QString::number(sit->second.second - sit->second.first + 1) + "*" + QString::number(sit->first) + ",";
				DataSlotList += QString::number(sit->second.first) + ".." + QString::number(sit->second.second) + ",";
			}
			if (!(it->second.empty())) { // if dataslots were written in previous for loop, then delete the last comma and add a closing bracket
				RSPBoardList = RSPBoardList.left(RSPBoardList.length()-1) + "]"; // deletes the last comma and adds ']'
				DataSlotList = DataSlotList.left(DataSlotList.length()-1) + "]"; // deletes the last comma and adds ']'
			}
			else { // only add closing bracket
				RSPBoardList += "]";
				DataSlotList += "]"; // deletes the last comma and adds ']'
			}

			bResult &= createAndWriteDataSlotNodes(treeID, dataSlotTemplateIDstr, strParentDataslotNodeID, antennaFieldName, RSPBoardList, DataSlotList);
			if (!antennaFieldName2.isEmpty()) {
				bResult &= createAndWriteDataSlotNodes(treeID, dataSlotTemplateIDstr, strParentDataslotNodeID, antennaFieldName2, RSPBoardList, DataSlotList);
			}
		}
	}

	return bResult;
}

bool SASConnection::createAndWriteDataSlotNodes(int treeID, const QString &dataSlotTemplateIDstr, const QString &strParentDataslotNodeID, const QString &antennaFieldName,
		const QString &RSPBoardList, const QString &DataSlotList) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );

	// check if this antennafield for this station already exists in the SAS tree
	QVariant value(getNodeValue(treeID, "LOFAR.ObsSW.Observation.Dataslots." + antennaFieldName, true));
	if (!value.isValid()) { // if the antennafield node under dataslots doesn't exist create it
		QSqlQuery query(sasDB);
		QString strTreeID(QString::number(treeID));
		// dataslotinfo nodes do not yet exist for this station -> create them
		if (!query.exec("select * from instanciateVHleafNode(" +
				dataSlotTemplateIDstr + "," +
				strTreeID + "," +
				strParentDataslotNodeID + ",'-1','LOFAR.ObsSW.Observation.Dataslots." + antennaFieldName + "')")) {
			QString err("could not create dataslot nodes for tree:" + strTreeID + " and antennafield:" + antennaFieldName);
			err += "\n" + query.lastError().text() + "\nquery:" + query.lastQuery();
			debugErr("s", err.toStdString().c_str());
			itsLastErrorString += err + "\n";
			bResult = false;
		}
		query.finish();
	}
	// save the actual values to the dataslot info node
	if (bResult) {
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.Dataslots." + antennaFieldName + ".RSPBoardList", RSPBoardList)) bResult = false;
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.Dataslots." + antennaFieldName + ".DataslotList", DataSlotList)) bResult = false;
	}
	return bResult;
}

bool SASConnection::getCampaignsFromSAS(void) {
	Controller::theSchedulerSettings.clearCampaigns();
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query(sasDB);
	campaignMap campaigns;
	if (query.exec("select * from getcampaignlist()")) {
		campaignInfo campaign;
		while (query.next()) {
			campaign.id = query.value(query.record().indexOf("id")).toUInt();
			campaign.name = query.value(query.record().indexOf("name")).toString().toStdString();
			campaign.title = query.value(query.record().indexOf("title")).toString().toStdString();
			campaign.PriInvestigator = query.value(query.record().indexOf("pi")).toString().toStdString();
			campaign.CoInvestigator = query.value(query.record().indexOf("co_i")).toString().toStdString();
			campaign.contact = query.value(query.record().indexOf("contact")).toString().toStdString();
			// store in itsCampaigns
			campaigns.insert(campaignMap::value_type(campaign.name, campaign));
		}
		Controller::theSchedulerSettings.setCampaigns(campaigns);
		return true;
	}
	else {
        std::cerr << query.lastError().text().toStdString() << std::endl;
		return false;
	}
}

bool SASConnection::saveProcessingSettings(int treeID, const Observation &task, const task_diff *diff) {
	bool bResult(true);
	QString trueStr("true"), falseStr("false");
    const Observation::RTCPsettings &RTCPsettings = task.getRTCPsettings();
	if (diff) {
		if (diff->RTCP_coherent_stokes_settings) {
			// Coherent Stokes
			if (RTCPsettings.coherentType < _END_DATA_TYPES) {
				// Analogous setting for COBALT
                if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.which", DATA_TYPES[RTCPsettings.coherentType] )) bResult = false;
			}
			else {
				bResult = false;
                std::cerr << "ERROR: tree " << treeID << " coherent data type (which) not set!" << std::endl;
			}
            // Coherent Stokes settings
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", QString::number(RTCPsettings.coherentTimeIntegrationFactor) )) bResult = false;
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", QString::number(RTCPsettings.coherentChannelsPerSubband) )) bResult = false;
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.subbandsPerFile", QString::number(RTCPsettings.coherentSubbandsPerFile) )) bResult = false;
		}
		if (diff->RTCP_incoherent_stokes_settings) {
			// Incoherent Stokes
			if (RTCPsettings.incoherentType < _END_DATA_TYPES) {
				if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.which", DATA_TYPES[RTCPsettings.incoherentType] )) bResult = false;
			}
			else {
				bResult = false;
                std::cerr << "ERROR: tree " << treeID << " incoherent data type (which) not set!" << std::endl;
			}
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor", QString::number(RTCPsettings.incoherentTimeIntegrationFactor) )) bResult = false;
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.nrChannelsPerSubband", QString::number(RTCPsettings.incoherentChannelsPerSubband) )) bResult = false;
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.subbandsPerFile", QString::number(RTCPsettings.incoherentSubbandsPerFile) )) bResult = false;
		}
		// pencil info
        if (diff->RTCP_pencil_flys_eye)
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.flysEye", (RTCPsettings.flysEye ? trueStr : falseStr))) bResult = false;
        if (diff->RTCP_correct_bandpass) {
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.correctBandPass", (RTCPsettings.correctBandPass ? trueStr : falseStr))) bResult = false;
		}
        if (diff->RTCP_delay_compensation) {
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.delayCompensation", (RTCPsettings.delayCompensation ? trueStr : falseStr))) bResult = false;
		}
		if (diff->RTCP_nr_bits_per_sample)
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.nrBitsPerSample", QString::number(RTCPsettings.nrBitsPerSample))) bResult = false;
		if (diff->RTCP_channels_per_subband) {
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrChannelsPerSubband", QString::number(RTCPsettings.channelsPerSubband))) bResult = false;
		}
		if (diff->RTCP_coherent_dedispersion) {
			if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.coherentDedisperseChannels", (RTCPsettings.coherentDedisperseChannels ? trueStr : falseStr))) bResult = false;
		}
	}
	else {
		// coherent Stokes data
		if (RTCPsettings.coherentType < _END_DATA_TYPES) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.which", DATA_TYPES[RTCPsettings.coherentType] )) bResult = false;
		}
		else {
			bResult = false;
            std::cerr << "ERROR: tree " << treeID << " coherent data type (which) not set!" << std::endl;
		}
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", QString::number(RTCPsettings.coherentTimeIntegrationFactor) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", QString::number(RTCPsettings.coherentChannelsPerSubband) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.subbandsPerFile", QString::number(RTCPsettings.coherentSubbandsPerFile) )) bResult = false;
        // Incoherent Stokes
		if (RTCPsettings.incoherentType < _END_DATA_TYPES) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.which", DATA_TYPES[RTCPsettings.incoherentType] )) bResult = false;
		}
		else {
			bResult = false;
            std::cerr << "ERROR: tree " << treeID << " incoherent stokes type (which) not set!" << std::endl;
		}
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor", QString::number(RTCPsettings.incoherentTimeIntegrationFactor) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.nrChannelsPerSubband", QString::number(RTCPsettings.incoherentChannelsPerSubband) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.subbandsPerFile", QString::number(RTCPsettings.incoherentSubbandsPerFile) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.flysEye", (RTCPsettings.flysEye ? trueStr : falseStr))) bResult = false;
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.correctBandPass", (RTCPsettings.correctBandPass ? trueStr : falseStr))) bResult = false;
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.delayCompensation", (RTCPsettings.delayCompensation ? trueStr : falseStr))) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.nrBitsPerSample", QString::number(RTCPsettings.nrBitsPerSample))) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrChannelsPerSubband", QString::number(RTCPsettings.channelsPerSubband))) bResult = false;
		if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.coherentDedisperseChannels", (RTCPsettings.coherentDedisperseChannels ? trueStr : falseStr))) bResult = false;
	}

	return bResult;
}

bool SASConnection::saveCobaltBlockSize(int treeID, const Observation &task) {
    bool bResult(true);
    BlockSize bs = calcCobaltBlockSize(task.getRTCPsettings(), task.storage()->getOutputDataProductsEnabled(), task.getStationClockFrequency());
    QString correlatorIntTime(QString::number(bs.integrationTime)),
            blockSize(QString::number(bs.blockSize)),
            nrBlocks(QString::number(bs.nrBlocks)),
            nrSubblocks(QString::number(bs.nrSubblocks));
    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.integrationTime", correlatorIntTime)) bResult = false;
    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrBlocksPerIntegration", nrBlocks)) bResult = false;
    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrIntegrationsPerBlock", nrSubblocks)) bResult = false;
    if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.blockSize", blockSize)) bResult = false;
    return bResult;
}

BlockSize SASConnection::calcCobaltBlockSize(Observation::RTCPsettings rtcp, TaskStorage::enableDataProdukts enabledOutputs, unsigned short clockMHz) const {
    size_t factor = BlockSize::calcFactor(
                enabledOutputs.correlated,
                rtcp.channelsPerSubband,
		rtcp.correlatorIntegrationTime,
                enabledOutputs.coherentStokes,
                rtcp.coherentChannelsPerSubband,
                rtcp.coherentTimeIntegrationFactor,
                enabledOutputs.incoherentStokes,
                rtcp.incoherentChannelsPerSubband,
                rtcp.incoherentTimeIntegrationFactor);

    return BlockSize(rtcp.correlatorIntegrationTime, factor, clockMHz);
}

bool SASConnection::getProcessingSettings(int treeID, Observation &task) {
	bool bResult(true);
	QVariant value;
    Observation::RTCPsettings RTCPsettings;

	// coherent Stokes data
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.which");
	if (value.isValid()) {
		RTCPsettings.coherentType = stringToStokesType(value.toString().toStdString());
	}
	else {
		RTCPsettings.coherentType = DATA_TYPE_UNDEFINED;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor");
	if (value.isValid()) {
		RTCPsettings.coherentTimeIntegrationFactor = value.toUInt();
	}
	else {
		RTCPsettings.coherentTimeIntegrationFactor = 0;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband");
	if (value.isValid()) {
		RTCPsettings.coherentChannelsPerSubband = value.toUInt();
	}
	else {
		RTCPsettings.coherentChannelsPerSubband = 0;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.subbandsPerFile");
	if (value.isValid()) {
		RTCPsettings.coherentSubbandsPerFile = value.toUInt();
	}
	else {
		RTCPsettings.coherentSubbandsPerFile = 0;
		bResult = false;
	}

	// incoherent Stokes data
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.which");
	if (value.isValid()) {
		RTCPsettings.incoherentType = stringToStokesType(value.toString().toStdString());
	}
	else {
		RTCPsettings.incoherentType = DATA_TYPE_UNDEFINED;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor");
	if (value.isValid()) {
		RTCPsettings.incoherentTimeIntegrationFactor = value.toUInt();
	}
	else {
		RTCPsettings.incoherentTimeIntegrationFactor = 0;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.nrChannelsPerSubband");
	if (value.isValid()) {
		RTCPsettings.incoherentChannelsPerSubband = value.toUInt();
	}
	else {
		RTCPsettings.incoherentChannelsPerSubband = 0;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.subbandsPerFile");
	if (value.isValid()) {
		RTCPsettings.incoherentSubbandsPerFile = value.toUInt();
	}
	else {
		RTCPsettings.incoherentSubbandsPerFile = 0;
		bResult = false;
	}

    // flys eye switch
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.flysEye", true);
	if (value.isValid()) {
		RTCPsettings.flysEye = value.toBool();
	}
	else {
        // backwards compatibility for moved flys eye switch
        value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.OLAP.PencilInfo.flysEye");
        if (value.isValid()) {
            RTCPsettings.flysEye = value.toBool();
        }
        else {
            RTCPsettings.flysEye = false;
            bResult = false;
        }
	}

    // other Cobalt options and settings
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.correctBandPass");
	if (value.isValid()) {
		RTCPsettings.correctBandPass = value.toBool();
	}
	else {
		RTCPsettings.correctBandPass = false;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.delayCompensation");
	if (value.isValid()) {
		RTCPsettings.delayCompensation = value.toBool();
	}
	else {
		RTCPsettings.delayCompensation = false;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.integrationTime");
	if (value.isValid()) {
		RTCPsettings.correlatorIntegrationTime = value.toDouble();
	}
	else {
		RTCPsettings.correlatorIntegrationTime = 0.0;
		bResult = false;
	}
	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.nrBitsPerSample", true);
	if (value.isValid()) {
		RTCPsettings.nrBitsPerSample = value.toInt();
	}
	else {
        itsProgressDialog.addError(QString("Warning: could not read Observation.nrBitsPerSample of SAS tree:" + QString::number(treeID)));
        RTCPsettings.nrBitsPerSample = 16;
        bResult = false;
    }

	// set the number of dataslots per RSP board according to the read nrBitsPerSample
	switch (RTCPsettings.nrBitsPerSample) {
	default:
	case 16:
		task.setNrOfDataslotsPerRSPboard(MAX_DATASLOT_PER_RSP_16_BITS +1);
		break;
	case 8:
		task.setNrOfDataslotsPerRSPboard(MAX_DATASLOT_PER_RSP_8_BITS +1);
		break;
	case 4:
		task.setNrOfDataslotsPerRSPboard(MAX_DATASLOT_PER_RSP_4_BITS +1);
		break;
	}


    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.coherentDedisperseChannels");
	if (value.isValid()) {
		RTCPsettings.coherentDedisperseChannels = value.toBool();
	}
	else {
		RTCPsettings.coherentDedisperseChannels = false;
		bResult = false;
	}
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrChannelsPerSubband");
	if (value.isValid()) {
		RTCPsettings.channelsPerSubband = value.toInt();
	}
	else {
		RTCPsettings.channelsPerSubband = 0;
		bResult = false;
	}

	task.setRTCPsettings(RTCPsettings);
	return bResult;
}


bool SASConnection::getCalibrationSettings(int treeID, CalibrationPipeline &task) {
	bool bResult(true);
	QVariant value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Calibration.SkyModel");
	if (value.isValid()) {
        task.setSkyModel(value.toString());
	}
    else {
        task.setSkyModel("");
        bResult = false;
    }
	return bResult;
}

bool SASConnection::getDemixingSettings(int treeID, CalibrationPipeline &task) {
	bool bResult(true);
	QVariant value;
    DemixingSettings settings;

	bool DPPP_demixer_step(false), DPPP0_demixer_step(false), DPPP_averager_step(false), DPPP0_averager_step(false);

	// first check which nodes exists and which steps are done for DPPP
	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.steps", true);
	if (value.isValid()) {
		QString steps(value.toString());
		if (steps.contains("demixer")) {
			DPPP_demixer_step = true;
            settings.itsDemixingEnabled = true;
		}

		if (steps.contains("averager")) {
			DPPP_averager_step = true;
		}
	}
	else {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].steps", true);
		if (value.isValid()) {
			QString steps(value.toString());
			if (steps.contains("demixer")) {
				DPPP0_demixer_step = true;
                settings.itsDemixingEnabled = true;
			}
			if (steps.contains("averager")) {
				DPPP0_averager_step = true;
			}
		}
		else {
			itsProgressDialog.addError("Could not fetch the DPPP steps for tree:" + QString::number(treeID));
		}
	}

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.demix_always");
	if (value.isValid()) {
        settings.itsDemixAlways = value.toString().remove("[").remove("]");
	}
	else {
        settings.itsDemixAlways = "";
		bResult = false;
	}
	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.demix_if_needed");
	if (value.isValid()) {
        settings.itsDemixIfNeeded = value.toString().remove("[").remove("]");
	}
	else {
        settings.itsDemixIfNeeded = "";
		bResult = false;
	}
	// (user supplied) demixing SkyModel
	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.SkyModel");
	if (value.isValid()) {
        settings.itsSkyModel = value.toString();
	}
	else {
        settings.itsSkyModel = "";
		bResult = false;
	}

	// averaging freq step, first try fetch from DPPP.freqstep, if not existing then DPPP[0].freqstep, else from averager.freqstep
//	settings.freqStep = 0;
//	settings.timeStep = 0;
//	settings.demixFreqStep = 0;
//	settings.demixTimeStep = 0;

	if (DPPP_demixer_step) {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.freqstep");
		if (value.isValid()) {
            settings.itsFreqStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.timestep");
		if (value.isValid()) {
            settings.itsTimeStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.demixfreqstep");
		if (value.isValid()) {
            settings.itsDemixFreqStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.demixtimestep");
		if (value.isValid()) {
            settings.itsDemixTimeStep = value.toUInt();
		}
	}
	else if (DPPP0_demixer_step) {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.freqstep");
		if (value.isValid()) {
            settings.itsFreqStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.timestep");
		if (value.isValid()) {
            settings.itsTimeStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.demixfreqstep");
		if (value.isValid()) {
            settings.itsDemixFreqStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.demixtimestep");
		if (value.isValid()) {
            settings.itsDemixTimeStep = value.toUInt();
		}
	}
	else if (DPPP_averager_step) {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.averager.freqstep");
		if (value.isValid()) {
            settings.itsFreqStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.averager.timestep");
		if (value.isValid()) {
            settings.itsTimeStep = value.toUInt();
		}
	}
	else if (DPPP0_averager_step) {
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].averager.freqstep");
		if (value.isValid()) {
            settings.itsFreqStep = value.toUInt();
		}
		value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].averager.timestep");
		if (value.isValid()) {
            settings.itsTimeStep = value.toUInt();
		}
	}

    task.setDemixingSettings(settings);
	return bResult;

}

bool SASConnection::saveCalibrationSettings(int treeID, const CalibrationPipeline &task, const task_diff *diff) {
	bool bResult(true);
	if (diff) {
		if (diff->calibration_skymodel) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Calibration.SkyModel", task.skyModel())) bResult = false;
		}
	}
	else {
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Calibration.SkyModel", task.skyModel())) bResult = false;
	}
	return bResult;
}

bool SASConnection::saveDemixingSettings(int treeID, const DemixingSettings &demix, const task_diff *diff) {
	bool bResult(true);
	if (diff) {
		// first check which nodes exists and which steps are done for DPPP
		bool DPPP_demixer_step(false), DPPP0_demixer_step(false), DPPP_averager_step(false), DPPP0_averager_step(false);
		QVariant value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.steps", true);
		if (value.isValid()) {
			QString steps(value.toString());
			if (steps.contains("demixer")) {
				DPPP_demixer_step = true;
			}
			if (steps.contains("averager")) {
				DPPP_averager_step = true;
			}
		}
		else {
			value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].steps", true);
			if (value.isValid()) {
				QString steps(value.toString());
				if (steps.contains("demixer")) {
					DPPP0_demixer_step = true;
				}
				if (steps.contains("averager")) {
					DPPP0_averager_step = true;
				}
			}
			else {
				itsProgressDialog.addError("Could not fetch the DPPP steps for tree:" + QString::number(treeID));
			}
		}

		if (diff->demix_always) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.demix_always", "[" + demix.demixAlways() + "]")) bResult = false;
		}
		if (diff->demix_if_needed) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.demix_if_needed", "[" + demix.demixIfNeeded() + "]")) bResult = false;
		}
		if (diff->demix_skymodel) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.SkyModel", demix.skyModel())) bResult = false;
		}
		if (diff->freqstep) {
			bool res(false);
            QString freqStep(QString::number(demix.freqStep()));
			if (DPPP_demixer_step) {
                res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.freqstep", freqStep);
			}
			if (DPPP0_demixer_step) {
                res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.freqstep", freqStep);
			}
			if (DPPP_averager_step) {
                res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.averager.freqstep", freqStep);
			}
			if (DPPP0_averager_step) {
                res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].averager.freqstep", freqStep);
			}
			if ((DPPP_demixer_step || DPPP0_demixer_step || DPPP_averager_step || DPPP0_averager_step) && !res) {
				itsProgressDialog.addError("failed to save DPPP freqstep for tree:" + QString::number(treeID));
				bResult = false;
			}
		}
		if (diff->timestep) {
			bool res(false);
            QString timeStep(QString::number(demix.timeStep()));
			if (DPPP_demixer_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.timestep", timeStep);
			}
			if (DPPP0_demixer_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.timestep", timeStep);
			}
			if (DPPP_averager_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.averager.timestep", timeStep);
			}
			if (DPPP0_averager_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].averager.timestep", timeStep);
			}
			if ((DPPP_demixer_step || DPPP0_demixer_step || DPPP_averager_step || DPPP0_averager_step) && !res) {
				itsProgressDialog.addError("failed to save DPPP timestep for tree:" + QString::number(treeID));
				bResult = false;
			}
		}
		if (diff->demix_freqstep) {
			bool res(false);
            QString demixFreqStep(QString::number(demix.demixFreqStep()));
			if (DPPP_demixer_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.demixfreqstep", demixFreqStep);
			}
			if (DPPP0_demixer_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.demixfreqstep", demixFreqStep);
			}
			if ((DPPP_demixer_step || DPPP0_demixer_step) && !res) {
				itsProgressDialog.addError("failed to save DPPP demixer.demixfreqstep for tree:" + QString::number(treeID));
				bResult = false;
			}
		}
		if (diff->demix_timestep) {
			bool res(false);
            QString demixTimeStep(QString::number(demix.demixTimeStep()));
			if (DPPP_demixer_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.demixtimestep", demixTimeStep);
			}
			if (DPPP0_demixer_step) {
				res |= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.demixtimestep", demixTimeStep);
			}
			if ((DPPP_demixer_step || DPPP0_demixer_step) && !res) {
				itsProgressDialog.addError("failed to save DPPP demixer.demixfreqstep for tree:" + QString::number(treeID));
				bResult = false;
			}
		}
	}
	else {
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.demix_always", "[" + demix.demixAlways() + "]")) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.demix_if_needed", "[" + demix.demixIfNeeded() + "]")) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.PreProcessing.SkyModel", demix.skyModel())) bResult = false;
        QString freqStep(QString::number(demix.freqStep())),
                timeStep(QString::number(demix.timeStep())),
                demixFreqStep(QString::number(demix.demixFreqStep())),
                demixTimeStep(QString::number(demix.demixTimeStep()));

		// first check which nodes exists and which steps are done for DPPP
		bool DPPP_demixer_step(false), DPPP0_demixer_step(false), DPPP_averager_step(false), DPPP0_averager_step(false);
		QVariant value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.steps", true);
		if (value.isValid()) {
			QString steps(value.toString());
			if (steps.contains("demixer")) {
				DPPP_demixer_step = true;
			}
			if (steps.contains("averager")) {
				DPPP_averager_step = true;
			}
		}
		else {
			value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].steps", true);
			if (value.isValid()) {
				QString steps(value.toString());
				if (steps.contains("demixer")) {
					DPPP0_demixer_step = true;
				}
				if (steps.contains("averager")) {
					DPPP0_averager_step = true;
				}
			}
			else {
				itsProgressDialog.addError("Could not fetch the DPPP steps for tree:" + QString::number(treeID));
			}
		}

		if (DPPP_demixer_step) {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.freqstep", freqStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.timestep", timeStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.demixfreqstep", demixFreqStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.demixer.demixtimestep", demixTimeStep);
		}
		if (DPPP0_demixer_step){
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.freqstep", freqStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.timestep", timeStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.demixfreqstep", demixFreqStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].demixer.demixtimestep", demixTimeStep);
		}
		if (DPPP_averager_step) {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.averager.timestep", timeStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP.averager.freqstep", freqStep);
		}
		if (DPPP0_averager_step) {
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].averager.timestep", timeStep);
			bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.DPPP[0].averager.freqstep", freqStep);
		}
	}

	return bResult;
}

bool SASConnection::savePulsarSettings(int treeID, const PulsarPipeline &pulsarPipe, const task_diff *diff) {
    if (diff) {
        if (!diff->pulsar_pipeline_settings) {
            return true;
        }
    }

    bool bResult(true);
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.2bf2fits_extra_opts", pulsarPipe.twoBf2fitsExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.digifil_extra_opts", pulsarPipe.digifilExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.dspsr_extra_opts", pulsarPipe.dspsrExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.prepdata_extra_opts", pulsarPipe.prepDataExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.prepfold_extra_opts", pulsarPipe.prepFoldExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.prepsubband_extra_opts", pulsarPipe.prepSubbandExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.pulsar", pulsarPipe.pulsarName());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.rfifind_extra_opts", pulsarPipe.rfiFindExtra());
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.8bit_conversion_sigma", QString::number(pulsarPipe.eightBitConversionSigma(),'g',16));
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.dynamic_spectrum_time_average", QString::number(pulsarPipe.dynamicSpectrumAvg(),'g',16));
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.rrats_dm_range", QString::number(pulsarPipe.rratsDmRange(),'g',16));
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.decode_nblocks", QString::number(pulsarPipe.decodeNblocks()));
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.decode_sigma", QString::number(pulsarPipe.decodeSigma()));
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.tsubint", QString::number(pulsarPipe.tsubInt()));
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.nofold", pulsarPipe.noFold() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.nopdmp", pulsarPipe.noPdmp() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.norfi", pulsarPipe.noRFI() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.raw_to_8bit", pulsarPipe.rawTo8Bit() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.rrats", pulsarPipe.rrats() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.single_pulse", pulsarPipe.singlePulse() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.skip_dspsr", pulsarPipe.skipDspsr() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.skip_dynamic_spectrum", pulsarPipe.skipDynamicSpectrum() ? "true" : "false");
    bResult &= setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.skip_prepfold", pulsarPipe.skipPrepfold() ? "true" : "false");

    return bResult;
}

bool SASConnection::getPulsarSettings(int treeID, PulsarPipeline &pulsarPipe) {
    bool bResult(true);
    QVariant value;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.2bf2fits_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setTwoBf2fitsExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.digifil_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setDigifilExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.dspsr_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setDspsrExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.prepdata_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setPrepDataExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.prepfold_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setPrepFoldExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.prepsubband_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setPrepSubbandExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.pulsar");
    if (value.isValid()) {
        pulsarPipe.setPulsarName(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.rfifind_extra_opts");
    if (value.isValid()) {
        pulsarPipe.setRFIfindExtra(value.toString());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.8bit_conversion_sigma");
    if (value.isValid()) {
        pulsarPipe.setEightBitConversionSigma(value.toDouble());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.dynamic_spectrum_time_average");
    if (value.isValid()) {
        pulsarPipe.setDynamicSpectrumAvg(value.toDouble());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.rrats_dm_range");
    if (value.isValid()) {
        pulsarPipe.setRratsDmRange(value.toDouble());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.decode_nblocks");
    if (value.isValid()) {
        pulsarPipe.setDecodeNblocks(value.toInt());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.decode_sigma");
    if (value.isValid()) {
        pulsarPipe.setDecodeSigma(value.toInt());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.tsubint");
    if (value.isValid()) {
        pulsarPipe.setTsubInt(value.toInt());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.nofold");
    if (value.isValid()) {
        pulsarPipe.setNoFold(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.nopdmp");
    if (value.isValid()) {
        pulsarPipe.setNoPdmp(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.norfi");
    if (value.isValid()) {
        pulsarPipe.setNoRFI(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.raw_to_8bit");
    if (value.isValid()) {
        pulsarPipe.setRawTo8Bit(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.rrats");
    if (value.isValid()) {
        pulsarPipe.setRRATS(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.single_pulse");
    if (value.isValid()) {
        pulsarPipe.setSinglePulse(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.skip_dspsr");
    if (value.isValid()) {
        pulsarPipe.setSkipDspsr(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.skip_dynamic_spectrum");
    if (value.isValid()) {
        pulsarPipe.setSkipDynamicSpectrum(value.toBool());
    }
    else bResult = false;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Pulsar.skip_prepfold");
    if (value.isValid()) {
        pulsarPipe.setSkipPrepfold(value.toBool());
    }
    else bResult = false;

    return bResult;
}


bool SASConnection::getImagingSettings(int treeID, ImagingPipeline &task) {
	bool bResult(true);
	QVariant value;

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.slices_per_image");
	if (value.isValid()) {
        task.setSlicesPerImage(value.toUInt());
	}
	else {
        task.setSlicesPerImage(0);
		bResult = false;
	}

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.subbands_per_image");
	if (value.isValid()) {
        task.setSubbandsPerImage(value.toUInt());
	}
	else {
        task.setSubbandsPerImage(0);
		bResult = false;
	}

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.AWimager.npix");
	if (value.isValid()) {
        task.setNrOfPixels(value.toUInt());
	}
	else {
        task.setNrOfPixels(0);
		bResult = false;
	}

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.AWimager.cellsize");
	if (value.isValid()) {
        task.setCellSize(value.toString());
	}
	else {
        task.setCellSize("");
		bResult = false;
	}

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.fov");
	if (value.isValid()) {
        task.setFov(value.toFloat());
	}
	else {
        task.setFov(0.0);
		bResult = false;
	}

	value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.specify_fov");
	if (value.isValid()) {
        task.setSpecifyFov(value.toBool());
	}
	else {
        task.setSpecifyFov(true);
		bResult = false;
	}

	return bResult;
}

bool SASConnection::saveImagingSettings(int treeID, const ImagingPipeline &task, const task_diff *diff) {
    bool bResult(true);
    if (diff) {
        if (diff->Imaging_nr_slices_per_image) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.slices_per_image", QString::number(task.slicesPerImage()) )) bResult = false;
        }
        if (diff->Imaging_nr_subbands_per_image) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.subbands_per_image", QString::number(task.subbandsPerImage()) )) bResult = false;
        }
        if (diff->Imaging_npix) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.AWimager.npix", QString::number(task.nrOfPixels()) )) bResult = false;
        }
        if (diff->Imaging_cellsize) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.AWimager.cellsize", task.cellSize())) bResult = false;
        }
        if (diff->Imaging_fov) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.fov", QString::number(task.fov(),'g',16))) bResult = false;
        }
        if (diff->Imaging_specify_fov) {
            QString specifyFOV = task.specifyFov() ? "true" : "false";
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.specify_fov", specifyFOV)) bResult = false;
        }
    }
    else {
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.slices_per_image", QString::number(task.slicesPerImage()) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.subbands_per_image", QString::number(task.subbandsPerImage()) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.AWimager.npix", QString::number(task.nrOfPixels()) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.AWimager.cellsize", task.cellSize())) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.Imaging.fov", QString::number(task.fov(),'g',16))) bResult = false;
    }

    return bResult;
}

// TODO: Code smell OO: Should the ownership of this function be in the SASCOnnection?
// THe sasconnection should not know about the longbaseline.
bool SASConnection::getLongBaselineSettings(int treeID, LongBaselinePipeline &task) {
    bool bResult(true);
    QVariant value;

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.LongBaseline.subbands_per_subbandgroup");
    if (value.isValid()) {
        task.setSubbandsPerSubbandGroup(value.toUInt());
    }
    else {
        task.setSubbandsPerSubbandGroup(0);
        bResult = false;
    }

    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.LongBaseline.subbandgroups_per_ms");
    if (value.isValid()) {
        task.setSubbandGroupsPerMS(value.toUInt());
    }
    else {
        task.setSubbandGroupsPerMS(0);
        bResult = false;
    }

    return bResult;
}

bool SASConnection::saveLongBaselineSettings(int treeID, const LongBaselinePipeline &task, const task_diff *diff) {
    bool bResult(true);
    if (diff) {
        if (diff->LongBaseline_nr_sb_per_sbgroup) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.LongBaseline.subbands_per_subbandgroup", QString::number(task.subbandsPerSubbandGroup()) )) bResult = false;
        }
        if (diff->LongBaseline_nr_sbgroup_per_MS) {
            if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.LongBaseline.subbandgroups_per_ms", QString::number(task.subbandGroupsPerMS()) )) bResult = false;
        }
    }
    else {
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.LongBaseline.subbands_per_subbandgroup", QString::number(task.subbandsPerSubbandGroup()) )) bResult = false;
        if (!setNodeValue(treeID, "LOFAR.ObsSW.Observation.ObservationControl.PythonControl.LongBaseline.subbandgroups_per_ms", QString::number(task.subbandGroupsPerMS()) )) bResult = false;
    }

    return bResult;
}

bool SASConnection::getScheduledTimes(int treeID, Task &task) {
    bool bResult(true);
    QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
    QVariant value;
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.startTime");
    if (value.isValid()) {
        if (!value.toString().isEmpty()) { // empty value means that it is not currently set in OTDB
            AstroDateTime startTime(value.toDateTime());
            if (startTime.isSet()) {
                task.setScheduledStart(startTime);
            }
        }
    }
    else bResult = false;
    value = getNodeValue(treeID, "LOFAR.ObsSW.Observation.stopTime");
    if (value.isValid()) {
        if (!value.toString().isEmpty()) {
            AstroDateTime endTime(value.toDateTime());
            if (endTime.isSet()) {
                task.setScheduledEnd(endTime);
            }
        }
    }
    else bResult = false;
    return bResult;
}

//gets all 'non-scheduler' branch properties from the SAS vic tree and stores them in task
bool SASConnection::getSASTaskProperties(int treeID, Task &task) {
	bool bResult(true);
	//  here task properties which are relevant for reservation as well as for maintenance tasks

	getCampaignInfo(task);
	bResult &= getScheduledTimes(treeID, task);

    if (task.isStationTask()) {
        StationTask &statTask(static_cast<StationTask &>(task));
        bResult &= getStationSettings(treeID, statTask);
        if (statTask.isObservation()) {
            Observation &obs(static_cast<Observation &>(statTask));
            bResult &= getAnalogBeamSettings(treeID, obs);
            bResult &= getProcessingSettings(treeID, obs);
            bResult &= getDigitalBeams(treeID, obs);
            getOutputStorageSettings(treeID, obs);
        }
    }
    else if (task.isPipeline()) {
        Pipeline &pipeline = static_cast<Pipeline &>(task);
        if (pipeline.isImagingPipeline()) {
            ImagingPipeline &impipe(static_cast<ImagingPipeline &>(pipeline));
            bResult &= getImagingSettings(treeID, impipe);
		}
        else if (pipeline.isCalibrationPipeline()) {
            CalibrationPipeline &calpipe(static_cast<CalibrationPipeline &>(pipeline));
            bResult &= getDemixingSettings(treeID, calpipe);
            bResult &= getCalibrationSettings(treeID, calpipe);
		}
        else if (pipeline.isPulsarPipeline()) {
            PulsarPipeline &pulspipe(static_cast<PulsarPipeline &>(pipeline));
            bResult &= getPulsarSettings(treeID, pulspipe);
        }
        else if (pipeline.isLongBaselinePipeline()) {
            LongBaselinePipeline &lbpipe(static_cast<LongBaselinePipeline &>(pipeline));
            bResult &= getLongBaselineSettings(treeID, lbpipe);
        }
		/*bResult &= */getInputStorageSettings(treeID, task);
		/*bResult &= */getOutputStorageSettings(treeID, task);
//        task.storage()->recalculateCheck(); // recalculate the tasks data files, total data size and bandwidth
	}
	return bResult;
}

OTDBtree SASConnection::getTreeInfo(int treeID) const {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	QSqlQuery query("select * from gettreeinfo(" + QString::number(treeID) + ", false)", sasDB);
	if (query.next()) {
		OTDBtree otdb_tree(query);
		query.finish();
		return otdb_tree;
	}
	else return OTDBtree();
}

bool SASConnection::saveTaskToSAS(int treeID, Task &task, const task_diff *diff) {
	bool bResult(true);
	itsLastErrorString = "";
	Task::task_status status = task.getStatus();
	bResult &= saveSchedulerProperties(treeID, task, diff);

    if (task.isStationTask()) { //OBSERVATION, RESERVATION or MAINTENANCE
        bResult &= saveStationSettings(treeID, static_cast<StationTask &>(task), diff);

        if (task.isObservation()) {
            Observation &obs(static_cast<Observation &>(task));
            bResult &= saveStationSettings(treeID, obs, diff);
            if (QString(obs.getAntennaModeStr()).startsWith("HBA")) { // only try to save analog beam settings when HBA mode is chosen. (Anabeam might not exist in SAS)
                bResult &= saveAnalogBeamSettings(treeID, obs, diff);
            }
            bResult &= saveDigitalBeamSettings(treeID, obs, diff);
            bResult &= saveTiedArrayBeamSettings(treeID, obs, diff);
            bResult &= saveProcessingSettings(treeID, obs, diff);
            if ((status == Task::PRESCHEDULED) || (status == Task::SCHEDULED)) {
                bResult &= saveDataSlots(treeID, obs);
            }

            // Cobalt Correlator BlockSize
            if (status == Task::PRESCHEDULED || status == Task::SCHEDULED) { // in SCHEDULED state always update BlockSize //FIXME? Added PRESCHEDULED for CEP4 /AR
                bResult &= saveCobaltBlockSize(treeID, obs);
            }
            else if (diff) {
                if (diff->RTCP_cor_int_time) {
                    bResult &= saveCobaltBlockSize(treeID, obs);
                }
            }
        }
    }
    else if (task.isPipeline()) {
        pipelineType ptype = static_cast<Pipeline &>(task).pipelinetype();
        if (ptype == PIPELINE_IMAGING) {
            bResult &= saveImagingSettings(treeID, static_cast<ImagingPipeline &>(task), diff);
		}
        else if (ptype == PIPELINE_CALIBRATION) {
            CalibrationPipeline &calPipe(static_cast<CalibrationPipeline &>(task));
            bResult &= saveDemixingSettings(treeID, calPipe.demixingSettings(), diff);
            bResult &= saveCalibrationSettings(treeID, calPipe, diff);
		}
        else if (ptype == PIPELINE_PULSAR) {
            savePulsarSettings(treeID, static_cast<PulsarPipeline &>(task), diff);
        }
        else if (ptype == PIPELINE_LONGBASELINE) {
            saveLongBaselineSettings(treeID, static_cast<LongBaselinePipeline &>(task), diff);
        }
	}

    if (diff) { //FIXME if diff than we do this, otherwise we do it any way? This seems redundant. /AR
		// all the following differences can potentially change the number of output files being written,
	    // therefore, we update the storage keys in SAS when anyone of them has changed
		if (diff->output_data_types || diff->output_storage_settings || diff->output_data_products ||
		    diff->digital_beam_settings || diff->RTCP_coherent_stokes_settings || diff->RTCP_incoherent_stokes_settings)
			bResult &= saveOutputStorageSettings(treeID, task, diff);

		if (diff->input_data_products)
			bResult &= saveInputStorageSettings(treeID, task);
		if (diff->scheduled_start || diff->scheduled_end)
			bResult &= setTreeSchedule(treeID, task);
		if (diff->description)
			bResult &= saveDescription(treeID, task);
		if (diff->task_status)
			bResult &= setTreeState(treeID, task.SASTree().state());
		if (diff->project_ID || diff->group_ID)
			bResult &= saveMoMinfo(treeID, task);
	}
	else {
		bResult &= saveInputStorageSettings(treeID, task);
		bResult &= saveOutputStorageSettings(treeID, task);
		bResult &= setTreeSchedule(treeID, task);
		bResult &= saveDescription(treeID, task);
		bResult &= setTreeState(treeID, task.SASTree().state());
		bResult &= saveMoMinfo(treeID, task);
	}

	return bResult;
}

void SASConnection::storePublishDates(const Task *pTask) {
    if (pTask->getScheduledStart().isSet()) {
        itsChangedDates.push_back(pTask->getScheduledStart().date());
    }
    if (pTask->getScheduledEnd().isSet()) {
        itsChangedDates.push_back(pTask->getScheduledEnd().date());
    }
    // also check for each task if it was moved from another week than the current week.
    // If that is the case the original week (from where the task was moved) also needs to be added to the changedDates
    unsigned treeID(pTask->getSASTreeID());
    if (treeID) {
        for (std::map<unsigned, OTDBtree>::const_iterator it = itsSASVicTrees.begin(); it != itsSASVicTrees.end(); ++it) {
            if (treeID == it->first) {
                const AstroDate &startDate(it->second.startTime().date());
                const AstroDate &endDate(it->second.stopTime().date());
                // if the week number differs also push back the original start/end date
                if (startDate.getWeek() != pTask->getScheduledStart().date().getWeek()) {
                    itsChangedDates.push_back(startDate);
                }
                if (endDate.getWeek() != pTask->getScheduledEnd().date().getWeek()) {
                    itsChangedDates.push_back(endDate);
                }

            }
        }
    }
}

void SASConnection::determineUploadedDateRange(void) {
    if (!itsChangedDates.empty()) {
        AstroDate earliest(itsChangedDates.front()), latest(itsChangedDates.front());
        for (std::vector<AstroDate>::const_iterator it = itsChangedDates.begin()+1; it != itsChangedDates.end(); ++it) {
            if (*it < earliest) earliest = *it;
            if (*it > latest) latest = *it;
        }
        itsUploadedDateRange = std::pair<AstroDate, AstroDate>(earliest, latest);
    }
}

bool SASConnection::commitScheduleToSAS(SchedulerData &data) {
	bool bResult(true);
	int treeID;
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );

	itsProgressDialog.disableClose();
	itsProgressDialog.show();
	itsProgressDialog.setProgressPercentage(0);
	size_t totalTrees(itsNewSchedulerTasks.size() + itsTreesToDelete.size() + itsChangedTasks.size());
	size_t count(0);
	vector<unsigned> removeTasks;
    itsChangedDates.clear();

	// ------------------------------------STEP 2 -----------------------------------------------
	// upload changed task properties of tasks with changes
    Task *pCloneTask;
	if (!itsChangedTasks.empty()) {
		itsProgressDialog.addText("Uploading task changes...");
		for (changedTasks::const_iterator it = itsChangedTasks.begin(); it != itsChangedTasks.end(); ++it) {
            unsigned treeID(it->first);
            Task *pTask = data.getTaskForChange(treeID, ID_SAS);
			if (pTask) {
				if (saveTaskToSAS(treeID, *pTask, &(it->second))) { // first = treeID, second is the task, third = task changes
                    // update itsSAStasks if changes are uploaded successfully
                    storePublishDates(pTask);
                    SAStasks::iterator sit = itsSASTasks.find(treeID);
                    if (sit != itsSASTasks.end()) {
                        delete sit->second;
                        itsSASTasks.erase(sit);
                    }
                    pCloneTask = cloneTask(pTask); // create a cloned task for future compare of changes in itsSASTasks
                    if (pCloneTask) {
                        itsSASTasks[treeID] = pCloneTask;
                    }
					itsSASVicTrees[treeID] = pTask->SASTree();
                    itsProgressDialog.addText(QString("successful uploaded changes for tree:") + QString::number(treeID));
                    removeTasks.push_back(treeID);
                }
				else {
                    itsProgressDialog.addError(QString("failed to save changes to tree:") + QString::number(treeID));
					itsProgressDialog.addError(itsLastErrorString);
					bResult = false;
				}
			}
			itsProgressDialog.setProgressPercentage(++count/totalTrees*100);
		}
		// remove the correctly uploaded changedIDTasks
		for (vector<unsigned>::const_iterator it = removeTasks.begin(); it != removeTasks.end(); ++it) {
			itsChangedTasks.erase(*it);
		}
		removeTasks.clear();
	}

	// ------------------------------------STEP 2 -----------------------------------------------
	// add all new tasks to the SAS database
	if (!itsNewSchedulerTasks.empty()) {
		itsProgressDialog.addText("Creating trees for new tasks...");
		int parentTree;
		for (std::vector<unsigned>::const_iterator it = itsNewSchedulerTasks.begin(); it != itsNewSchedulerTasks.end(); ++it) {
			Task *pTask = data.getTaskForChange(*it);
			if (pTask) {
				// check which default template should be used for creating the tree (it could have been changed)
				parentTree = Controller::theSchedulerSettings.getSASDefaultTreeID(pTask);
				pTask->setOriginalTreeID(parentTree);
				treeID = createNewTree(*pTask);
				if (treeID != 0) {
					// download the OTDBtree properties from the just created task from SAS to make sure we have an exact copy of what is in SAS
					QSqlQuery query("select * from gettreeinfo(" + QString::number(treeID) + ", false)", sasDB);
					if (query.next()) {
						OTDBtree otdb_tree(query);
						query.finish();
                        pTask->setSASTree(otdb_tree);
					}
                    storePublishDates(pTask); // have to do this BEFORE changing itsSASVicTrees below
                    itsSASTasks[treeID] = cloneTask(pTask); // create a cloned task for future compare of changes in itsSASTasks
					itsSASVicTrees.insert(std::map<unsigned, OTDBtree>::value_type(treeID, pTask->SASTree()));
					removeTasks.push_back(*it);
					itsProgressDialog.addText(QString("created new tree:") + QString::number(treeID) + " for task:" + QString::number(pTask->getID()));
                }
				else { // error not able to create VIC tree
					itsProgressDialog.addError(QString("failed to create new VIC tree for task:") + QString::number(pTask->getID()));
					bResult = false;
				}
				itsProgressDialog.setProgressPercentage(++count/totalTrees*100);
			}
		}
		// remove the correctly uploaded changedIDTasks
		for (vector<unsigned>::const_iterator it = removeTasks.begin(); it != removeTasks.end(); ++it) {
			for (std::vector<unsigned>::iterator cit = itsNewSchedulerTasks.begin(); cit != itsNewSchedulerTasks.end(); ++cit) {
				if (*cit == *it) itsNewSchedulerTasks.erase(cit);
				break;
			}
		}
		removeTasks.clear();
	}

	// ------------------------------------STEP 3 -----------------------------------------------
	// delete the tasks that need to be deleted
	if (!itsTreesToDelete.empty()) {
		itsProgressDialog.addText("Deleting tasks...");
	}
	QSqlQuery query(sasDB);
	for (std::vector<unsigned>::const_iterator it = itsTreesToDelete.begin(); it != itsTreesToDelete.end(); ++it) {
		if (query.exec("SELECT deleteTree(" +
				itsAuthToken + "," +
				QString::number(*it) + ")")) {
            // store the changed dates for the auto publish functionality
            SAStasks::iterator sit = itsSASTasks.find(*it);
            if (sit != itsSASTasks.end()) {
                storePublishDates(sit->second);
                delete sit->second;
                itsSASTasks.erase(sit);
            }
			itsSASVicTrees.erase(*it);
			itsProgressDialog.addText(QString("deleted tree:") + QString::number(*it));
		}
		else {
			debugErr("ssss", "SAS DB ", query.lastError().text().toStdString().c_str(),
					" query:", query.lastQuery().toStdString().c_str());
			itsProgressDialog.addError(QString("failed to delete tree:") + QString::number(*it) +
					" (" + query.lastError().text() + ")");
			bResult = false;
		}
		query.finish();
		itsProgressDialog.setProgressPercentage(++count/totalTrees*100);
	}
	itsTreesToDelete.clear();

	itsProgressDialog.setProgressPercentage(100);

	// TODO: update error tasks
	itsUploadDialog->hide();

	if (!bResult) {
		itsProgressDialog.addError("Done with errors.");
		QMessageBox::critical(0, QObject::tr("Error: could not apply all changes"), QObject::tr("Could not apply all changes to SAS database "));
	}
	else {
		itsProgressDialog.addText("Done.");
	}

	itsProgressDialog.enableClose();

	updateLastDownloadDate();

    // determine and store uploaded date range for auto publish functionality
    determineUploadedDateRange();

	return bResult;
}

std::vector<DefaultTemplate> SASConnection::getDefaultTemplates(void) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	std::vector<DefaultTemplate> templates;
	if (!sasDB.isOpen()) {
		return templates;
	}

	DefaultTemplate templateInfo;
	QSqlQuery query(sasDB), query2(sasDB);
	if (query.exec("SELECT * from getdefaulttemplates()")) {
		while (query.next()) {
			templateInfo.treeID = query.value(query.record().indexOf("treeID")).toInt();
			templateInfo.name = query.value(query.record().indexOf("name")).toString();
			templateInfo.processType = query.value(query.record().indexOf("processType")).toString().toUpper();
			templateInfo.processSubtype = query.value(query.record().indexOf("processSubtype")).toString();
			templateInfo.strategy = query.value(query.record().indexOf("strategy")).toString();
			// now get the description of this default template
			if (query2.exec("SELECT state,description FROM gettreeinfo('" + QString::number(templateInfo.treeID) +"',false)")) {
				if (query2.next()) {
					templateInfo.status = query2.value(query2.record().indexOf("state")).toInt();
					templateInfo.description = query2.value(query2.record().indexOf("description")).toString();
				}
			}
			query2.finish();
			templates.push_back(templateInfo);
		}
	}
	query.finish();
	// also fetch the node ID of the dataslotsinfo node in the default template tree (needed to be able to create and save dataslot info for new trees)

//	query.exec("SELECT nodeid from getVTitemList(" + QString::number(Controller::theSchedulerSettings.getSASDefaultTreeID()) + ",'DataslotInfo')");
//	if (query.next()) {
//		itsDataslotTemplateIDstr = query.value(0).toString();
//	}
//	query.finish();

	return templates;
}

const Task *SASConnection::fetchPredecessorObservation(const QString predStr) {
    std::vector<QString> predecessors(string2VectorOfStrings(predStr));
    QString isMomID, IDonly, ObsIDPrefix(Controller::theSchedulerSettings.getObservationIDprefix());
    bool fetchPredecessor;
    QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
    QSqlQuery query(sasDB);
    for (std::vector<QString>::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
        // TODO: predecessor specified with scheduler task IDs should be translated to sas id during upload of the task
        fetchPredecessor = false;
        IDonly = it->right(it->size()-1);
        if (it->startsWith('M')) {
                isMomID = "true";
                fetchPredecessor = true;
        }
        else if (it->startsWith(ObsIDPrefix)) { // SAS ID used for predecessor, fetch the predecessor directly
                isMomID = "false";
                fetchPredecessor = true;
        }
        else {
            return 0;
        }

        // fetch the predecessor task and push it in itsTmpSASVicTrees2
        if (fetchPredecessor) {
            query.exec("SELECT * from gettreeinfo(" + IDonly + ",'" + isMomID + "')");
            if (query.next()) {
                OTDBtree tree(query);
                query.finish();
                // check if predecessor is indeed a VIC tree
                if (tree.type() == VIC_TREE && tree.processType() == "OBSERVATION") {
                    std::pair<bool, Task *> retVal = getTaskFromSAS(tree.treeID(), tree);
                    return retVal.second;
                }
                else return 0;
            }
        }
    }
    return 0;
}

/*
bool SASConnection::checkSASStatus(void) {
    QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
    int result = connect();
    if (result == 0) { // connection ok // write permission ok
        QString processType("Pipeline"), processSubType("Averaging Pipeline"), strategy("");
        SAS_task_status firstState(SAS_STATE_FINISHED);
        bool empty(true);
        QString query("SELECT * from getTreesInPeriod('" + QString::number(VIC_TREE) + "','" +
                      Controller::theSchedulerSettings.getEarliestSchedulingDay().toString().c_str() + "','" +
                      Controller::theSchedulerSettings.getLatestSchedulingDay().toString().c_str() +
                      "') WHERE state=" + QString::number(firstState));
        if (!processType.isEmpty()) {
            empty = false;
            query += " AND processtype='" + processType + "'";
        }
        if (!processSubType.isEmpty()) {
            if (!empty) {
                query += " AND ";
            }
            query += "processsubtype='" + processSubType + "'";
            empty = false;
        }
        if (!strategy.isEmpty()){
            if (!empty) {
                query += " AND ";
            }
            query += "strategy='" + strategy + "'";
            empty = false;
        }

        //query += " ORDER BY starttime LIMIT 50";

        std::cout << query.toStdString() << std::endl;

        std::map<unsigned, Task *> tasks;
        QSqlQuery sqlquery(sasDB);
        if (sqlquery.exec(query)) {
            if (sqlquery.isActive()) {
                while (sqlquery.next()) {
                    OTDBtree tree(sqlquery);
                    std::pair<bool, Task *> retVal = getTaskFromSAS(tree.treeID(), tree);
                    if (retVal.second) {
                        tasks.insert(std::map<unsigned, Task *>::value_type(tree.treeID(), retVal.second));
                    }
                }
                sqlquery.finish();
            }
            else {
                sqlquery.finish();
                return -1;// could not fetch any task from SAS query not valid
            }

            // gather statistics about the tasks
            const CalibrationPipeline *calPipe(0);
//            float factor;
            QString outstr;
            const Observation *predecessor(0);
            if (!tasks.empty()) {
                std::cout << outstr.toStdString() << "writing file 'statistics.csv'" << std::endl;

            for (std::map<unsigned, Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {

                AstroDateTime timedif(it->second->SASTree().stopTime() - it->second->SASTree().startTime());
                //unsigned estdur(it->second->getDuration().totalSeconds());
                unsigned realdur(timedif.totalSeconds());
//                if (estdur >= realdur) factor = estdur / realdur;
//                else factor = realdur / estdur;

                outstr += it->second->SASTree().startTime().date().toString().c_str() + QString(", ")
                + QString::number(it->second->getSASTreeID()) + QString(", ")
                        + it->second->getProjectID() + QString(", ")
                        + it->second->getTaskName() + QString(", ")
//                        + it->second->getStatusStr() + QString(", ")
//                        + it->second->getDuration().toString().c_str() + QString(", ")
//                        + QString::number(it->second->getDuration().totalSeconds()) + ", "
                        + timedif.time().toString().c_str() + QString(", ")
                        + QString::number(timedif.totalSeconds());
//                        + QString::number(factor,'f',1);

                calPipe = dynamic_cast<const CalibrationPipeline *>(it->second);
                if (calPipe) {
                    QString demixSources = calPipe->demixingSettings().demixAlways();
                    outstr += QString(", [") + demixSources.replace(',',';') + "], ";
                }
                else {
                    outstr += ", [], ";
                }

                // fetch the predecessor observation
                predecessor = dynamic_cast<const Observation *>(fetchPredecessorObservation(it->second->getPredecessorsString()));
                if (predecessor) { // found the predecessor observation
                    unsigned predDur(predecessor->getDuration().totalSeconds());
                    outstr += QString::number(predDur) + ", ";
                    if (realdur >= predDur) {
                        outstr += QString::number((float)realdur / predDur, 'f', 1);
                    }
                    else {
                        outstr += QString::number((float)predDur / realdur, 'f', 1);
                    }

                    outstr += ", " + QString::number(predecessor->getRTCPsettings().channelsPerSubband) + ", "
                            + QString::number(predecessor->getRTCPsettings().correlatorIntegrationTime) + ", ";
                    const StationTask *pStation = static_cast<const StationTask *>(predecessor);
                    outstr += QString::number(pStation->getNrVirtualStations()) + ", ";
                    outstr += QString(pStation->getAntennaModeStr()).left(3);

                    const Pipeline *pipe(dynamic_cast<const Pipeline *>(it->second));
                    if (pipe) {

                        const std::map<dataProductTypes, TaskStorage::inputDataProduct> &inputdata(pipe->storage()->getInputDataProducts());
                        const std::map<dataProductTypes, TaskStorage::inputDataProduct>::const_iterator iit = inputdata.find(DP_CORRELATED_UV);
                        if (iit != inputdata.end()) {
                            if (!iit->second.identifications.isEmpty()) {
                                QString inputIdent(iit->second.identifications.at(0)); // should have only one identification for Correlated data input
//                                outstr += ", " + inputIdent;
                                const std::map<dataProductTypes, TaskStorage::outputDataProduct> &predData(predecessor->storage()->getOutputDataProducts());
                                const std::map<dataProductTypes, TaskStorage::outputDataProduct>::const_iterator oit = predData.find(DP_CORRELATED_UV);
                                if (oit != predData.end()) {
                                    if (!oit->second.identifications.isEmpty()) {
                                        int beamIdx(oit->second.identifications.indexOf(inputIdent)); // which beam index?
                                        const std::map<unsigned, DigitalBeam> &beams(predecessor->getDigitalBeams());
                                        const std::map<unsigned, DigitalBeam>::const_iterator bit = beams.find(beamIdx);
                                        if (bit != beams.end()) {
                                            outstr += ", " + QString::number(bit->second.nrSubbands());
                                        }
                                    }
                                }

                            }
                        }
                    }
                }

                outstr += "\n";
            }


                QFile file("statistics.csv");
                file.open(QIODevice::ReadWrite);
                file.write(outstr.toStdString().c_str());
                file.close();
            }

        }

    }
    return true;
}
*/


bool SASConnection::checkSASStatus(void) {
	bool bResult(true);
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	SASStatusDialog * statDlg = new SASStatusDialog();
	statDlg->show();
	int result = connect();
	if (result == 0) { // connection ok // write permission ok
		statDlg->addText("SAS connection OK.");
		statDlg->addText("SAS write permissions OK");
		// checking functions
		statDlg->addText("Checking SAS database functions:");
		QSqlQuery query(sasDB);
		if (query.exec("SELECT * from getTreesInPeriod('" +
						QString::number(VIC_TREE) + "','2010-01-01 00:00:00', '2010-01-01 23:59:59')")) {
			statDlg->addText("function: 'getTreesInPeriod' OK");
		}
		else {
			statDlg->addError("function: 'getTreesInPeriod' ERROR");
			statDlg->addError(sasDB.lastError().text());
			bResult = false;
		}
		query.finish();
		int schedulerDefaultTemplateID = Controller::theSchedulerSettings.getSchedulerDefaultTemplate();
		QString SASDefaultTreeIDstr = QString::number(schedulerDefaultTemplateID);
		if (query.exec(QString("SELECT * from getvtitemlist(") + SASDefaultTreeIDstr + ",'taskID')")) {
			if (query.next()) {
				statDlg->addText(QString("function: 'SAS default template tree ") + SASDefaultTreeIDstr + " OK");
				int newTreeID(0);
				if (query.exec(QString("SELECT * from instanciateVHtree(")  +
						itsAuthToken + "," + SASDefaultTreeIDstr + ")")) {
					query.next();
					newTreeID = query.value(query.record().indexOf("instanciateVHtree")).toInt();
					if (newTreeID) {
						statDlg->addText("function: 'instanciateVHtree' OK");
						// check getVHitemList
						vector<OTDBnode> fieldList = getItemList(newTreeID, "LOFAR.ObsSW.Observation.Scheduler.taskID");
						if (fieldList.size() == 1) { // should be unique property
							statDlg->addText("function: 'getVHitemList' OK");
						// check updateVTnode function
						if (query.exec("SELECT updateVTnode(" +
								itsAuthToken + "," +
								QString::number(newTreeID) + "," +
								QString::number(fieldList.front().nodeID()) + ",'1','100')")) {
							// check if update ok
							query.next();
							if (query.value(query.record().indexOf("updatevtnode")).toBool()) {
								statDlg->addText("function: 'updateVTnode' OK");

								// check function getSchedulerInfo
								query.finish();
								int taskid(-1);
								if (query.exec("SELECT * from getSchedulerInfo(" + QString::number(newTreeID) + ")")) {
									query.next();
									taskid = query.value(query.record().indexOf("taskID")).toUInt();
									if (taskid == 100) {
										statDlg->addText("function: 'getSchedulerInfo' OK");
									}
									else {
										statDlg->addError("function: 'getSchedulerInfo' did not return the expected value!");
										bResult = false;
									}
								}
								else {
									statDlg->addError("function: 'getSchedulerInfo' ERROR (Can also be caused by a wrong SAS Scheduler master template tree)");
									statDlg->addError(sasDB.lastError().text());
									bResult = false;
								}
							}
							else {
								statDlg->addError("function: 'updateVTnode' ERROR");
								statDlg->addError(sasDB.lastError().text());
								bResult = false;
							}
						}
						else {
							statDlg->addError("function: 'updateVTnode' ERROR");
							statDlg->addError(sasDB.lastError().text());
							bResult = false;
						}

						}
						else {
							statDlg->addError("function: 'getVHitemList' ERROR returned multiple taskID fields for one tree!");
							bResult = false;
						}
						// check deleteTree function
						if (newTreeID) {
							if (!query.exec("SELECT deleteTree(" +
									itsAuthToken + "," +
									QString::number(newTreeID) + ")")) {
								statDlg->addError("function: 'deleteTree' ERROR");
								statDlg->addError(sasDB.lastError().text());
							}
							else {
								statDlg->addText("function: 'deleteTree' OK");
							}
						}
					}
					else {
						statDlg->addError("function: 'instanciateVHtree' could not create tree ERROR");
						statDlg->addError(sasDB.lastError().text());
						bResult = false;
					}
				}
				else {
					statDlg->addError("function: 'instanciateVHtree' ERROR");
					statDlg->addError(sasDB.lastError().text());
					bResult = false;
				}
			}
			else {
				statDlg->addError(QString("function: 'SAS default VIC template tree ") + SASDefaultTreeIDstr + " Not found. (Is the default template treeID correct?)");
//				statDlg->addError(sasDB.lastError().text());
				bResult = false;
			}
		}
		else {
			statDlg->addError("function: 'getvtitemlist' ERROR");
			statDlg->addError(sasDB.lastError().text());
		}
	}
	else if (result == -1) {
		// no connection to SAS database
		statDlg->addError("SAS connection ERROR: ");
		statDlg->addError(sasDB.lastError().text());
		bResult = false;
	}
	else if (result == -2) {
		// connection ok but no write permissions to SAS database
		statDlg->addText("SAS connection OK.");
		statDlg->addError("SAS write permissions ERROR. No write permissions to database! Please check SAS User name and password.");
		bResult = false;
	}
//	else if (result == 3) {
//		statDlg->addError("SAS connection ERROR:");
//		statDlg->addError("Error: could not get clock settings from SAS database.");
//		bResult = false;
//	}

	if (bResult) {
		statDlg->addText("Everything OK!");
	}
	else {
		statDlg->addText("ERRORS detected!");
	}
	return bResult;
}


std::string getSasTextState(int sas_state) {
	switch (sas_state) {
    // Why not create a sas_state to state str map?
    // This switch is not needed.
    // a switch statement is almost always a good candidate for OO refactoring
	case SAS_STATE_IDLE:
		return task_states_str[Task::IDLE];
		break;
	case SAS_STATE_DESCRIBED:
		return task_states_str[Task::DESCRIBED];
		break;
	case SAS_STATE_PREPARED:
		return task_states_str[Task::PREPARED];
		break;
	case SAS_STATE_APPROVED:
		return task_states_str[Task::UNSCHEDULED];
		break;
	case SAS_STATE_ON_HOLD:
		return task_states_str[Task::ON_HOLD];
		break;
	case SAS_STATE_PRESCHEDULED:
		return task_states_str[Task::PRESCHEDULED];
		break;
	case SAS_STATE_SCHEDULED:
		return task_states_str[Task::SCHEDULED];
		break;
	case SAS_STATE_QUEUED:
		return task_states_str[Task::STARTING];
		break;
	case SAS_STATE_ACTIVE:
		return task_states_str[Task::ACTIVE];
		break;
	case SAS_STATE_COMPLETING:
		return task_states_str[Task::COMPLETING];
		break;
	case SAS_STATE_FINISHED:
		return task_states_str[Task::FINISHED];
		break;
	case SAS_STATE_ABORTED:
		return task_states_str[Task::ABORTED];
		break;
	case SAS_STATE_ERROR:
		return task_states_str[Task::ERROR];
		break;
	case SAS_STATE_OBSOLETE:
		return task_states_str[Task::OBSOLETE];
		break;
	}
	return task_states_str[Task::IDLE];
}
