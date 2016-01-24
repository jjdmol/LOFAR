/*
 * CEPCleanMainWindow.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11125 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2013-03-05 15:24:46 +0100 (Tue, 05 Mar 2013) $
 * First creation : 8-sept-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/cepcleanmainwindow.h $
 *
 */

#ifndef CEPCLEANMAINWINDOW_H
#define CEPCLEANMAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_cepcleanmainwindow.h"
#include <QProcess>
#include <QProgressBar>
#include <QFile>
#include <vector>
#include "lofar_scheduler.h"
#include "ProjectDataProducts.h"
#include "dataproductinfodialog.h"
#include "SASConnection.h"

class QPushButton;
class Controller;
class PieView;
class CEPdeleteDialog;
class SASConnectDialog;
class FilterDataDialog;
class QTextStream;
class ParsetTreeViewer;

class file {
public:
	unsigned sasID;
	QString fileName, path;//, owner, group, permissions;
	quint64 fileSize; // kBytes
};

// key: node ID, value = vector of files
typedef std::map<int, std::vector<file> > filesMap;

class CEPCleanMainWindow : public QMainWindow
{
    Q_OBJECT

#define PROJECT_ITEM 1
#define TASK_ITEM 2
#define DATA_PRODUCT_ITEM 3

// usedNodeSpaceMap: key=node idx, value=used space by known data
typedef std::map<unsigned, quint64> usedNodeSpaceMap;

public:
    CEPCleanMainWindow(Controller *controller);
    ~CEPCleanMainWindow();

    ProjectDataProducts &updateProjectData(const QString &projectName) {return itsProjectsData[projectName];}
    void updateAndShow(bool updateFileInfo = false);
	bool deleteTreesCleanup(const deleteVICmap &treeIDs); /*{return itsSASConnection->deleteTreesCleanup(treeIDs, hostname, user, pass);}*/
	bool markDataProductsDeleted(const deletedDataMap &data);// {return itsSASConnection->markDataProductsDeleted(data);}
	bool checkSASsettings(void);
	void writeLog(const QString &);
    void parsetTreeView(const QString &parset, const OTDBtree &otdb_tree);
    QString fileDialog (const QString &title, const QString &def_suffix, const QString &filter, int save_load = 0);
    const Ui::CEPCleanMainWindowClass & gui(void) const {return ui;}

private:
	void addPieViews(void);
	void createStorageNodesTab(int rows, int colums);
    void updateFileInfo(const file &f, const QString &nodeName);
    void writeFileInfo(const file &f, const QString &nodeName, std::map<dataProductTypes, DataProductInfo> &dataproducts); // used by updateFileInfo
    bool fetchNodesData(void);
    bool getNodeLogFile(void);
    void resetFileExistanceInfo(void);
    void saveProjectData(QDataStream &out);
    void readProjectData(QDataStream &in);
    void updateTreeInfoAfterDelete(bool SASupdated);
    void updateProjectItemSizes(QTreeWidgetItem *projectItem);
    void setNodePercentageUsed(unsigned idx, int percentage);
    void setNodePercentageUnknown(unsigned idx, int percentage);
    void setNodesSpace(unsigned idx, const quint64 &totalSize, const quint64 &usedSize);
    bool checkExcludeStrings(const QString &);

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void closeEvent(QCloseEvent *event);

private slots:
    void openSettingsDialog(void) const;
	void done(void);
    void updateStorageNodesTab(void);
	void checkItemChange(QTreeWidgetItem *item, int column);
	void doDelete(void);
	void doUpdateFiles(void);
	void parseDuResult(int, QProcess::ExitStatus);
	void showItemDetails(QTreeWidgetItem *item, int /*col*/);
    void collapseItems(void);
    void expandItems(void);
    void checkItems(void);
    void showTotals(void);
    void unCheckItems(void);
    void checkVICtree(void);
    void unCheckVICtree(void);
    void enableDeleteButton(void) { ui.menuDelete->setEnabled(true);}
    void saveToDisk(void) const;
    void readFromDisk(void);
    void openDatabase(void);
    void filterData(void);
    void updateProjectPieView(void);
    void updateTasksPie(void); // creates a pie plot of the single selected project from project tree view
    void updateTasksPie(const QModelIndex &index);
    void updateTasksPie(const QString &projectName);

private:
    Ui::CEPCleanMainWindowClass ui;

    // Parset Tree View Dialog;
    ParsetTreeViewer *itsTreeViewer;

    std::map<unsigned, QLabel *> itsFreeSpaceLabels, itsNodes;
    std::map<unsigned, QProgressBar *> itsUsedSpaceBars, itsUnknownSpaceBars;
    usedNodeSpaceMap itsUsedNodesSpace;
    QPushButton *itsRefreshStorageUsageButton;
    PieView *itsPieProjectsView, *itsPieTasksView;
    QAbstractItemModel *itsProjectPieModel, *itsTasksPieModel;
    DataProductInfoDialog itsDataProductInfoDialog;
    Controller *itsController;
	SASConnection *itsSASConnection;
	QString itsHostName, itsUserName, itsPassword;
	SASConnectDialog *itsSASConnectDialog;
	FilterDataDialog *itsFilterDialog;
	QTextStream *itsLog;
    QString itsLogDir, itsLastPath;
    QMap<QString, bool> itsFoundExcludedStrings;
	QFile *itsLogFile;
	bool itsCreateLog, itsKeepLog;
    // itsProjectData map, key = project name, value = ProjectDataProducts which contain all the tasks and their data products of this project
    std::map<QString, ProjectDataProducts> itsProjectsData; // contains all the info on all data products of all projects
	file nullFile;
	std::vector<std::pair<QProcess *,QString> > itsDuFetchProcesses;

	std::map<QString, QStringList> itsNodesDeleteCommands;
	deleteVICmap itsVICtreesToDelete;
};

#endif // CEPCLEANMAINWINDOW_H
