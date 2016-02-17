/*
 * SASConnection.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11576 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-04-03 13:19:53 +0000 (Thu, 03 Apr 2014) $
 * First creation : 8-febr-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/SASConnection.cpp $
 *
 */

#include "lofar_utils.h"
#include "SASConnection.h"
#include "Controller.h"
#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QString>
#include <QProgressDialog>
#include "TaskDataProducts.h"
#include "cepcleanmainwindow.h"
using std::string;
using std::vector;
using std::map;

SASConnection::SASConnection(void)
    : itsController(0)
{
    QSqlDatabase::addDatabase( "QPSQL", "SASDB" );
}

SASConnection::SASConnection(Controller *controller)
	: itsController(controller)
{
    QSqlDatabase::addDatabase( "QPSQL", "SASDB" );
}

SASConnection::~SASConnection() {
	QSqlDatabase::database( "SASDB" ).close();
	QSqlDatabase::removeDatabase( "SASDB" );
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
		// get the list of campaigns from SAS
		getCampaignsFromSAS();
	}
	return 0;
}


bool SASConnection::getAllDataProductsFromSAS(CEPCleanMainWindow *cleanupDialog, const QString &dbName) {
	QSqlDatabase sasDB = QSqlDatabase::database( "SASDB" );
	if (connect() == 0) {
		std::vector<OTDBtree> trees;
		QSqlQuery query("select * from gettreegroup('4','0')", sasDB); // gets all VIC trees with state >= APPROVED and stop time < now
		while (query.next()) {
			trees.push_back(OTDBtree(query));
		}
		query.finish();
		QProgressDialog progressDialog("Analyzing data products from database " + dbName, "Cancel", 0, trees.size(), cleanupDialog);
		progressDialog.setWindowModality(Qt::WindowModal);
		progressDialog.setMinimumDuration(0);

		int count(0);
        QString pid, type;

		for (std::vector<OTDBtree>::const_iterator it = trees.begin(); it != trees.end(); ++it) {
			progressDialog.setValue(count);
			QCoreApplication::processEvents();
			if (progressDialog.wasCanceled()) {
				return false;
			}

			ProjectDataProducts &cData(cleanupDialog->updateProjectData(it->campaign().c_str())); // creates the project data for the campaign if it doesn't exist yet

			// which dataproducts have been enabled?
			query.exec("SELECT parentid,name from getvhitemlist(" + QString::number(it->treeID()) + ",'LOFAR.ObsSW.Observation.DataProducts.Output_%.enabled') WHERE limits='true'");
			bool hasEnabledDataProducts(false);
			while (query.next()) {
				hasEnabledDataProducts = true;
				TaskDataProducts &taskData(cData.updateTaskData(it->treeID())); // creates the task data for the task with treeID in the campaign's project data if it doesn't exist yet
				taskData.setSASTree(*it);
				taskData.setSASDBName(sasDB.databaseName());

				type = query.value(query.record().indexOf("name")).toString();
				if (!type.isNull()) {
					if (type.contains("_Correlated")) {
						DataProductInfo &dp = taskData.addDataProduct(DP_CORRELATED_UV);
						dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
					}
					else if (type.contains("_CoherentStokes") || type.contains("_Beamformed")) {
						DataProductInfo &dp = taskData.addDataProduct(DP_COHERENT_STOKES);
						dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
					}
					else if (type.contains("_IncoherentStokes")) {
						DataProductInfo &dp = taskData.addDataProduct(DP_INCOHERENT_STOKES);
						dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
					}
					else if (type.contains("_InstrumentModel")) {
						DataProductInfo &dp = taskData.addDataProduct(DP_INSTRUMENT_MODEL);
						dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
					}
                    else if (type.contains("_Pulsar")) {
                        DataProductInfo &dp = taskData.addDataProduct(DP_PULSAR);
                        dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
                    }
                    else if (type.contains("_SkyImage")) {
						DataProductInfo &dp = taskData.addDataProduct(DP_SKY_IMAGE);
						dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
					}
					else {
						DataProductInfo &dp = taskData.addDataProduct(DP_UNKNOWN_TYPE);
						dp.parentid = query.value(query.record().indexOf("parentid")).toInt();
					}
				}
			}
			query.finish();

			if (hasEnabledDataProducts) {
				// get all other properties of the (enabled) dataproducts of THIS task
				QString treeID(QString::number(it->treeID()));
				TaskDataProducts &taskData(cData.updateTaskData(it->treeID()));

				std::map<dataProductTypes, DataProductInfo> &dataProducts(taskData.dataProductsForChange());

				query.exec("SELECT limits from getvhitemlist(" + treeID + ",'LOFAR.ObsSW.Observation.Scheduler.taskName')");
				if (query.next()) {
					taskData.setTaskName(query.value(0).toString());
				}
				query.finish();

				QString strVal;
				for (std::map<dataProductTypes, DataProductInfo>::iterator dpit = dataProducts.begin(); dpit != dataProducts.end(); ++dpit) {
					pid = QString::number(dpit->second.parentid);
					query.exec("SELECT limits from getvhitemlist(" + treeID + ",'LOFAR.ObsSW.Observation.DataProducts.Output_%.deleted') WHERE parentid='" + pid + "'");
					if (query.next()) {
						query.value(0).toString().startsWith('f') ? dpit->second.deleted = false : dpit->second.deleted = true;
						query.finish();
					}
					query.exec("SELECT limits from getvhitemlist(" + treeID + ",'LOFAR.ObsSW.Observation.DataProducts.Output_%.filenames') WHERE parentid='" + pid + "'");
					if (query.next()) {
						strVal = query.value(0).toString().remove("[").remove("]");
						query.finish();
						if (!strVal.isEmpty()) {
							dpit->second.fileNames = strVal.split(",");
							size_t size(dpit->second.fileNames.size());
							dpit->second.fileSizes.assign(size, 0); // make filesizes array equally long
							dpit->second.exists.assign(size, false); // make exists array equally long
							query.exec("SELECT limits from getvhitemlist(" + treeID + ",'LOFAR.ObsSW.Observation.DataProducts.Output_%.locations') WHERE parentid='" + pid + "'");
							// eg: [locus054:/data/L32350/,locus087:/data/L32350/]
							if (query.next()) {
								dpit->second.locations = query.value(0).toString();
								query.finish();

								// nodes (not unique, should be equally long as dpit->second.fileNames
								dpit->second.nodes = dpit->second.locations.remove("[").remove("]").split(",");
								int pos;
								for (QStringList::iterator nit = dpit->second.nodes.begin(); nit != dpit->second.nodes.end(); ++nit) {
									pos = nit->indexOf(':');
									dpit->second.localPaths.append(nit->mid(pos+1));
									*nit = nit->left(pos);
								}
							}
						}
					}

					query.exec("SELECT limits from getvhitemlist(" + treeID + ",'LOFAR.ObsSW.Observation.DataProducts.Output_%.retentiontime') WHERE parentid='" + pid + "'");
					if (query.next()) {
						dpit->second.retentionTime = query.value(0).toInt();
						query.finish();
						// determine expiry date
						AstroDate eDate = it->stopTime().getDate();
						dpit->second.expiryDate.setDate(eDate.getYear(), eDate.getMonth(), eDate.getDay());
						dpit->second.expiryDate = dpit->second.expiryDate.addDays(dpit->second.retentionTime);
						// determine if expired already
						dpit->second.expiryDate <= QDate::currentDate() ? dpit->second.expired = true : dpit->second.expired = false;
					}
				}
			}
			progressDialog.setValue(++count);
			QCoreApplication::processEvents();
		}
	}
	else {
		QMessageBox::critical(0, QObject::tr("No connection to SAS"),
				QObject::tr("Could not connect to SAS database.\n Please check SAS connection settings."));
	}
	return true;
}


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
				}
				query.finish();
			}
		}
		else return false;
	}
	return true;
}

bool SASConnection::markDataProductsDeleted(const deletedDataMap &data, const QString &hostname) {
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
