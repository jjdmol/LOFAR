/*
 * SASConnection.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 8-febr-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/SASConnection.h $
 *
 */

#ifndef SASCONNECTION_H_
#define SASCONNECTION_H_

#include <vector>
#include <map>
#include <string>
#include "OTDBtree.h"
#include "OTDBnode.h"
class Controller;
class CEPCleanMainWindow;

#define PIC_TREE 10
#define TEMPLATE_TREE 20
#define VIC_TREE	30
#define DOWNLOAD_MODE true
#define UPLOAD_MODE false

typedef std::map<int, unsigned> SASVICTreeTaskIDMapping; // first: VIC tree ID, second: task ID
typedef std::map<QString, std::vector<std::pair<int, dataProductTypes> > > deletedDataMap;
// deleteVICmap: key = database name, value = stringlist of vic tree ids to delete
typedef std::map<QString, QStringList> deleteVICmap;


class SASConnection {
public:
	SASConnection();
	SASConnection(Controller *controller);
	virtual ~SASConnection();

	void init(const QString &username, const QString &password, const QString &DBName, const QString &hostname);
	int connect(void);
	int connect(const QString &username, const QString &password, const QString &database, const QString &host);
	int testConnect(const QString &username, const QString &password, const QString &DBname, const QString &hostname);
	void disconnect(void) {	QSqlDatabase::database( "SASDB" ).close(); QSqlDatabase::removeDatabase( "SASDB" ); }

	// get the SAS authentication token from the SAS database
	const QString &getAuthToken(void) const {return itsAuthToken;}

	// check SAS connection status, database status, database access rights, database integrity and show this in a status dialog
	bool checkSASStatus(void);
	// fetches the existing projects (campaign info) from SAS and stores them in theSchedulerSettings
	bool getCampaignsFromSAS(void);
	// return the number of tasks that will be deleted from SAS with the next upload
	unsigned nrTaskToDeleteFromSAS(void) const {return itsTreesToDelete.size();}

	QString lastConnectionError(void) const;
	OTDBtree getTreeInfo(int treeID) const;
	QString getTreeParset(int treeID); // gets the complete tree (parset) as a string
	bool getAllDataProductsFromSAS(CEPCleanMainWindow *cleanupDialog, const QString &dbName);
	// delete SAS trees via specification of treeIDs in stringlist
	bool deleteTreesCleanup(const deleteVICmap &treeIDs, const QString &hostname, const QString &user, const QString &password);
	bool markDataProductsDeleted(const deletedDataMap &data, const QString &hostname/*, const QString &user, const QString &password*/);
	// gets the 'limits' value as a QVariant type (a QVariant can be converted to any other type)
	QVariant getNodeValue(int aTreeID, const QString &nameFragment, bool noWarnings = false);

private:
	OTDBnode getNode(int treeID, const QString &nodeID) const;
    std::vector<OTDBnode> getItemList(int aTreeID, const QString &nameFragment) const;
    // sets a single SAS node value in the database for specific treeID, and unique property name fragment
    bool setNodeValue(int treeID, const QString &nameFragment, const QString &valueStr, bool warnings = true);

	// deletes all trees from SAS from which the treeID is in itsTreesToDelete vector
	bool deleteTrees(void);

	inline bool sasTreeExists(int treeID) const {
		return (itsSASVicTreeTaskIDMap.find(treeID) != itsSASVicTreeTaskIDMap.end());
	}

private:
    std::vector<unsigned> itsSASdeletedTrees;
	std::map<unsigned, OTDBtree> itsSASVicTrees; // contains the metadata of all SAS trees within the schedule's period

	SASVICTreeTaskIDMapping itsSASVicTreeTaskIDMap;
	std::vector<unsigned> itsNewSchedulerTasks;
	std::vector<unsigned> itsTreesToDelete; // contains SAS tree IDs not taskIDs (used for deletion of SAS trees)

	Controller *itsController;
	QString itsAuthToken;

	QString itsSASUserName, itsSASPassword;
    QString itsLastErrorString;
};

std::string getSasTextState(int sas_state);

#endif /* SASCONNECTION_H_ */
