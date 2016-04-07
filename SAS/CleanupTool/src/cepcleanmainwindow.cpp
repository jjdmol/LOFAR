/*
 * CEPCleanMainWindow.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11133 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2013-03-11 11:02:15 +0100 (Mon, 11 Mar 2013) $
 * First creation : 8-sept-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/cepcleanmainwindow.cpp $
 *
 */

#include "SASConnection.h"
#include "sasconnectdialog.h"
#include "filterdatadialog.h"
#include "cepcleanmainwindow.h"
#include "Controller.h"
#include "pieview.h"
#include "cepdeletedialog.h"
#include "DataTreeWidgetItem.h"
#include "lofar_utils.h"
#include "parsettreeviewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QStandardItemModel>
#include <QCloseEvent>
#include <cmath>

#define ITEM_TYPE_ROLE 33
#define DATA_TYPE_ROLE 34
#define VARIOUS_PATHS_ROLE 35

extern QString currentUser;

const char * DATA_PRODUCTS[NR_DATA_PRODUCT_TYPES] = {"Correlated", "Coherent Stokes", "Incoherent Stokes",
        "Instrument Model", "Pulsar", "Sky Image", "Unknown data type"};


CEPCleanMainWindow::CEPCleanMainWindow(Controller *controller)
    : itsController(controller), itsSASConnection(0), itsSASConnectDialog(0), itsFilterDialog(0), itsLog(0),
      itsLogFile(0), itsCreateLog(false), itsKeepLog(false)
{
	ui.setupUi(this);

	addPieViews();
	createStorageNodesTab(25,4);

	// get the current user
    if (currentUser == "lofarsys" || currentUser == "renting") {
		itsCreateLog = true;
		QString dateStr(QDateTime::currentDateTime().toString(Qt::ISODate));
		QString itsLogDir("log/CEPcleanup/"), logFileName = "CEPcleanLog_" + dateStr;
		QDir logDir;
        QDir::setCurrent(QDir::homePath());
        logDir.mkdir("log");
		logDir.cd("log");
		logDir.mkdir("CEPcleanup");
		itsLogFile = new QFile(itsLogDir + logFileName);
		if (itsLogFile->open(QIODevice::WriteOnly)) {
			itsLog = new QTextStream(itsLogFile);
			(*itsLog) << "CEP cleanup started on: " + dateStr << endl
					<< "users logged in currently:" << endl;
			QProcess process;
			process.setProcessChannelMode(QProcess::MergedChannels);
			process.start("last", QStringList());
			process.waitForFinished(-1);
			char buf[1024];
			QString line;
			while (process.readLine(buf,1024) != -1) {
				line = buf;
				if (line.contains("still logged in")) {
					(*itsLog) << buf;
				}
			}
			itsLogFile->close();
			process.close();
		}
		else {
			std::cerr << "could not create log file: " << logFileName.toStdString() << std::endl;
		}
	}
	else {
        QMessageBox::warning(this,"not running as lofarsys", "You are not running the cleanup as lofarsys.\nDeletion and saving settings are disabled.");
		ui.actionDelete->setEnabled(false);
        ui.action_Save_settings->setEnabled(false);
	}

	// set the general tree properties
	QStringList header;
	header << "project/task/data type" << "SAS id" << "run date" << "duration" << "expire date" << "expired" << "deleted" << "exist" << "# files" << "file size" << "total size" << "path" << "# nodes" << "SAS DB";
	ui.treeWidgetDataProducts->setColumnCount(header.size());
	ui.treeWidgetDataProducts->setHeaderLabels(header);
	ui.treeWidgetDataProducts->header()->setResizeMode(QHeaderView::Interactive);
	// connect some signals
    connect(ui.treeWidgetDataProducts,SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(checkItemChange(QTreeWidgetItem *, int)));
    connect(ui.treeWidgetDataProducts,SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(showItemDetails(QTreeWidgetItem *, int)));
    connect(ui.treeWidgetDataProducts->header(), SIGNAL(sectionClicked(int)), this, SLOT(updateProjectPieView()));
    connect(ui.actionDelete, SIGNAL(triggered()), this, SLOT(doDelete(void)));
    connect(ui.action_Save, SIGNAL(triggered()), this, SLOT(saveToDisk(void)));
    connect(ui.action_Load, SIGNAL(triggered()), this, SLOT(readFromDisk(void)));
    connect(ui.action_Open_Databases, SIGNAL(triggered()), this, SLOT(openDatabase(void)));
    connect(ui.actionFilter, SIGNAL(triggered()), this, SLOT(filterData(void)));
    connect(ui.action_Quit, SIGNAL(triggered()), this, SLOT(done(void)));
    connect(ui.actionChange_Settings, SIGNAL(triggered()), this, SLOT(openSettingsDialog(void)));
    this->setWindowFlags(Qt::Window);
}

void CEPCleanMainWindow::openSettingsDialog(void) const {
    itsController->openSettingsDialog();
}

QString CEPCleanMainWindow::fileDialog(const QString &title, const QString &def_suffix, const QString &filter, int save_load) {
    QFileDialog dialog;
    QFileInfo fi;
    QString path="";
    dialog.setFilters(filter.split('\n'));
    dialog.setWindowTitle(title);


    if (itsLastPath == "") {
        char *home = getenv("HOME");
        if (home) itsLastPath = home;
        else itsLastPath = ".";
    } else if (QFileInfo(itsLastPath).isDir()) {
        itsLastPath = QFileInfo(itsLastPath).canonicalPath();
    } else {
        itsLastPath = QFileInfo(itsLastPath).absoluteDir().canonicalPath();
    }

    dialog.setDirectory(itsLastPath);

    if (save_load == 0)
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
    else
        dialog.setAcceptMode(QFileDialog::AcceptSave);

    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setConfirmOverwrite(false); // we'll do that

    dialog.setDefaultSuffix(def_suffix);

    bool good = false;

    while (!good) {

        path = "";

        if (!dialog.exec()) break;

        QStringList files = dialog.selectedFiles();
        if (files.empty()) break;
        path = *files.begin();
//		std::cout << "path:" << path.toStdString() << std::endl;

        fi = path;

        if (fi.isDir()) {
            QMessageBox::critical(0, tr("Directory selected"),
                    tr("File \"%1\" is a directory").arg(path));
            continue;
        }
        if (save_load == 1) {
            if (fi.exists()) {
                if (QMessageBox::question(0, tr("File exists"),
                        tr("The file \"%1\" already exists.\nDo you want to overwrite it?").arg(path),
                        QMessageBox::Ok,
                        QMessageBox::Cancel) != QMessageBox::Ok) {
                    continue;
                }
            }
        }
        good = true;
    }
    itsLastPath = path;
    return path;
}


void CEPCleanMainWindow::addPieViews(void) {
    itsPieProjectsView = new PieView(ui.splitter);
    itsPieProjectsView->setObjectName(QString::fromUtf8("itsPieProjectsView"));
	itsProjectPieModel = new QStandardItemModel(0,2,this);
	itsPieProjectsView->setModel(itsProjectPieModel);
    QItemSelectionModel *selectionModel = new QItemSelectionModel(itsProjectPieModel);
    itsPieProjectsView->setSelectionModel(selectionModel);
    ui.splitter->addWidget(itsPieProjectsView);
    itsPieTasksView = new PieView(ui.splitter);
    itsPieTasksView->setObjectName(QString::fromUtf8("itsPieTasksView"));
    itsTasksPieModel = new QStandardItemModel(0,2,this);
    itsPieTasksView->setModel(itsTasksPieModel);
    QItemSelectionModel *selectionModel2 = new QItemSelectionModel(itsTasksPieModel);
    itsPieTasksView->setSelectionModel(selectionModel2);
    ui.splitter->addWidget(itsPieTasksView);
    QList<int> sizes;
    sizes << 1000 << 400;
    ui.splitter_2->setSizes(sizes);

    connect(itsPieProjectsView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(updateTasksPie(const QModelIndex &)));
}

CEPCleanMainWindow::~CEPCleanMainWindow()
{
	if (itsSASConnection) delete itsSASConnection;
	if (itsSASConnectDialog) delete itsSASConnectDialog;
	if (itsFilterDialog) delete itsFilterDialog;
	if (itsLog) {
		delete itsLog;
	}
	if (itsLogFile) {
		if (itsKeepLog) {
			itsLogFile->close();
		}
		else {
			itsLogFile->remove();
		}
		delete itsLogFile;
	}
}

void CEPCleanMainWindow::writeLog(const QString &line) {
	if (itsCreateLog) {
		if (!itsLogFile->isOpen()) {
			itsLogFile->open(QIODevice::Append);
		}
		(*itsLog) << line << endl;
	}
}

bool CEPCleanMainWindow::checkSASsettings(void) {
	QString user, password, hostName, DBName;

	if (!itsSASConnectDialog) {
		itsSASConnectDialog = new SASConnectDialog(this);
		while (true) {
			int retval(itsSASConnectDialog->exec());
			if (retval == 1) {
				user = itsSASConnectDialog->getUserName();
				password = itsSASConnectDialog->getPassword();
				hostName = itsSASConnectDialog->getHostName();
				DBName = itsSASConnectDialog->getDBName().front();
				if (hostName.isEmpty() || DBName.isEmpty()) {
					QMessageBox::warning(this,"Missing SAS connection settings","Please fill in a valid host name and database name");
				}
				else {
					if (itsSASConnection) {
						delete itsSASConnection;
					}
					itsSASConnection = new SASConnection();
					if (itsSASConnection->connect(user, password, DBName, hostName) == 0) return true;
					else return false;
				}
			}
			else return false;
		}
	}
	else {
		hostName = itsSASConnectDialog->getHostName();
		DBName = itsSASConnectDialog->getDBName().front();
		if (hostName.isEmpty() || DBName.isEmpty()) {
			QMessageBox::warning(this,"Missing SAS connection settings","Please fill in a valid host name and database name");
			while (itsSASConnectDialog->exec() != 1) {
				user = itsSASConnectDialog->getUserName();
				password = itsSASConnectDialog->getPassword();
				hostName = itsSASConnectDialog->getHostName();
				DBName = itsSASConnectDialog->getDBName().front();
				if (hostName.isEmpty() || DBName.isEmpty()) {
					QMessageBox::warning(this,"Missing SAS connection settings","Please fill in a valid host name and database name");
				}
				else {
					if (itsSASConnection) {
						delete itsSASConnection;
					}
					itsSASConnection = new SASConnection();
					if (itsSASConnection->connect(user, password, DBName, hostName) == 0) return true;
					else return false;
				}
			}
		}
		else {
			if (itsSASConnection) {
				delete itsSASConnection;
			}
			itsSASConnection = new SASConnection();
			if (itsSASConnection->connect(user, password, DBName, hostName) == 0) return true;
			else return false;
		}
	}
	return false;
}


void CEPCleanMainWindow::openDatabase(void) {
	if (!itsSASConnectDialog) {
		itsSASConnectDialog = new SASConnectDialog(this);
	}

    QString username(Controller::theSchedulerSettings.getSASUserName());
    itsSASConnectDialog->setUserName(username);
    QString password(Controller::theSchedulerSettings.getSASPassword());
    itsSASConnectDialog->setPassword(password);
    QString hostName(Controller::theSchedulerSettings.getSASHostName());
    itsSASConnectDialog->setHostName(hostName);
    QStringList DBNames(Controller::theSchedulerSettings.getSASDatabase().split(';'));
    itsSASConnectDialog->setDatabase(DBNames);

    if (itsSASConnectDialog->exec() == 1) {
		writeLog("Loading databases");

		bool hasData(false);
        for (int cur_db = 0; cur_db < DBNames.size(); ++cur_db) {
			if (itsSASConnection) {
				delete itsSASConnection;
			}
			itsSASConnection = new SASConnection();
            itsSASConnection->init(username, password, DBNames.at(cur_db), hostName);
			int result = itsSASConnection->connect();
			if (result == 0) {
                if (itsSASConnection->getAllDataProductsFromSAS(this, DBNames.at(cur_db))) {
                    writeLog("database:" + DBNames.at(cur_db) + " loaded");
					hasData = true;
				}
			}
			else {
                writeLog("ERROR could not connect to database:" + DBNames.at(cur_db));
				QMessageBox::critical(this,"Could not connect", "Could not connect to SAS database");
			}
		}

		if (hasData) {
			updateAndShow(true);
		}
	}
}


bool CEPCleanMainWindow::deleteTreesCleanup(const deleteVICmap &treeIDs) {
	if (itsHostName.isEmpty()) {
		QMessageBox::warning(this,"Not connected to SAS", "Not yet connected to the SAS database, please specify the connection settings");
		itsSASConnectDialog = new SASConnectDialog(this);
		if (itsSASConnectDialog->exec() == 1) {
			itsUserName = itsSASConnectDialog->getUserName();
			itsPassword = itsSASConnectDialog->getPassword();
			itsHostName = itsSASConnectDialog->getHostName();
//			QStringList DBName(itsSASConnectDialog->getDBName());
		}
		else return false;
	}
	return itsSASConnection->deleteTreesCleanup(treeIDs, itsHostName, itsUserName, itsPassword);
}

bool CEPCleanMainWindow::markDataProductsDeleted(const deletedDataMap &data) {
	if (itsHostName.isEmpty()) {
		QMessageBox::warning(this,"Not connected to SAS", "Not yet connected to the SAS database, please specify the connection settings");
		itsSASConnectDialog = new SASConnectDialog(this);
		if (itsSASConnectDialog->exec() == 1) {
//			itsUserName = itsSASConnectDialog->getUserName();
//			itsPassword = itsSASConnectDialog->getPassword();
			itsHostName = itsSASConnectDialog->getHostName();
//			QStringList DBName(itsSASConnectDialog->getDBName());
		}
		else return false;
	}
	return itsSASConnection->markDataProductsDeleted(data, itsHostName/*, itsUserName, itsPassword*/);
}


void CEPCleanMainWindow::saveToDisk(void) const {
	QFile file("cepdata.data");
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);
		out << (quint32)itsProjectsData.size();
		for (std::map<QString, ProjectDataProducts>::const_iterator pit = itsProjectsData.begin(); pit != itsProjectsData.end(); ++pit) {
			out << pit->first << pit->second;
		}
	}
	file.close();
}

void CEPCleanMainWindow::readFromDisk(void) {
	itsProjectsData.clear();
	QFile file("cepdata.data");
	if (file.open(QIODevice::ReadOnly)) {
		QDataStream in(&file);
		quint32 size;
		in >> size;
		QString projectName;
		ProjectDataProducts pdata;
		while (size--) {
			in >> projectName >> pdata;
			itsProjectsData[projectName] = pdata;
		}
	}
	file.close();
	updateAndShow(true);
}

void CEPCleanMainWindow::filterData(void) {
	ui.treeWidgetDataProducts->blockSignals(true);
	ui.treeWidgetDataProducts->setSortingEnabled(false);
	if (!itsFilterDialog) {
		itsFilterDialog = new FilterDataDialog(this);

		QStringList projectList;
		for (std::map<QString, ProjectDataProducts>::const_iterator pdit = itsProjectsData.begin(); pdit != itsProjectsData.end(); ++pdit) {
			projectList << pdit->first;
		}
		projectList.sort();
		itsFilterDialog->setProjects(projectList);
	}
	if (itsFilterDialog->exec() == 1) {

		// first unhide everything
		QTreeWidgetItemIterator allit(ui.treeWidgetDataProducts);
		while (*allit) {
			(*allit)->setHidden(false);
			++allit;
		}


		//filter data types
		if (itsFilterDialog->applyDataTypes()) {
			std::map<dataProductTypes, bool> dataTypes(itsFilterDialog->selectedDataTypes());
			QTreeWidgetItemIterator dit(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::NoChildren);
			bool hide, wasChecked;
			while (*dit) {
				if ((*dit)->data(0, ITEM_TYPE_ROLE) == DATA_PRODUCT_ITEM) {
					hide = !dataTypes[(dataProductTypes)(*dit)->data(0, DATA_TYPE_ROLE).toInt()];
					(*dit)->setHidden(hide);
					if (hide) {
						wasChecked = (*dit)->checkState(0) == Qt::Checked ? true : false;
						(*dit)->setCheckState(0,Qt::Unchecked);
						// also update parent project item checkstate if the current item was previously checked
						if (wasChecked) {
							bool partiallyChecked(false);
							QTreeWidgetItem *parentTaskItem((*dit)->parent());
							for (int i = 0; i < parentTaskItem->childCount(); ++i) {
								if (parentTaskItem->child(i)->checkState(0) == Qt::Checked) {
									partiallyChecked = true;
									break;
								}
							}
							// now set the checkstate of the parent Task item
							if (partiallyChecked) {
								parentTaskItem->setCheckState(0,Qt::PartiallyChecked);
								parentTaskItem->setCheckState(1,Qt::Unchecked); // disable the VIC tree deletion
								parentTaskItem->parent()->setCheckState(0,Qt::PartiallyChecked); // also marks the project item as partially checked
							}
							else {
								parentTaskItem->setCheckState(0,Qt::Unchecked);
							}
						}

					}
				}
				++dit;
			}
		}

		// filter projects
		QStringList projects(itsFilterDialog->selectedProjects());
		QTreeWidgetItemIterator pit(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::HasChildren);
		while (*pit) {
			if ((*pit)->data(0,ITEM_TYPE_ROLE) == PROJECT_ITEM) {
				if (projects.contains((*pit)->text(0))) {
					bool setHiddenProject(true);
					// check if there are child tasks for this project which are not filtered out (i.e. are not hidden)
					// if one is found then the project should be set visible
					for (int i = 0; i < (*pit)->childCount(); ++i) {
						if (!(*pit)->child(i)->isHidden()) {
							setHiddenProject = false;
							break;
						}
					}
					(*pit)->setHidden(setHiddenProject);
					if (setHiddenProject) {
						(*pit)->setCheckState(0,Qt::Unchecked);
					}
				}
				else {
					(*pit)->setHidden(true);
					(*pit)->setCheckState(0,Qt::Unchecked);
				}
			}
			++pit;
		}

		// filter on date range if needed
		if (itsFilterDialog->applyDates()) {
			QTreeWidgetItemIterator dit(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::HasChildren);
			QDate runDate, minDate(itsFilterDialog->minDate()), maxDate(itsFilterDialog->maxDate());
			while (*dit) {
				if ((*dit)->data(0,ITEM_TYPE_ROLE) == TASK_ITEM) {
					if (!(*dit)->isHidden()) {
						runDate = (*dit)->data(2, Qt::UserRole).toDate();
						if (runDate < minDate || runDate > maxDate) {
							(*dit)->setHidden(true);
							(*dit)->setCheckState(0,Qt::Unchecked);
						}
					}
				}
				++dit;
			}
		}

		// filter on data size if needed
		if (itsFilterDialog->applySizes()) {
			QTreeWidgetItemIterator sit(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::HasChildren);
			quint64 fsize, minSize(itsFilterDialog->minFileSize()), maxSize(itsFilterDialog->maxFileSize());
			while (*sit) {
				if ((*sit)->data(0,ITEM_TYPE_ROLE) == TASK_ITEM) {
					if (!(*sit)->isHidden()) {
						fsize = (*sit)->data(0, Qt::UserRole).toLongLong();
						if (fsize < minSize) {
							(*sit)->setHidden(true);
							(*sit)->setCheckState(0,Qt::Unchecked);
						}
						else if (maxSize != 0 && fsize > maxSize) {
							(*sit)->setHidden(true);
							(*sit)->setCheckState(0,Qt::Unchecked);
						}
					}
				}
				++sit;
			}
		}

		// hide the tasks & projects for which no data left after filtering
		if (itsFilterDialog->hideEmpty()) {
			QTreeWidgetItemIterator ait(ui.treeWidgetDataProducts);
			while (*ait) {
				if ((*ait)->data(0, ITEM_TYPE_ROLE) == TASK_ITEM) {
					bool setHiddenTask(true);
					for (int i = 0; i < (*ait)->childCount(); ++i) {
						if (!(*ait)->child(i)->isHidden()) {
							setHiddenTask = false;
							break;
						}
					}
					if (setHiddenTask) {
						(*ait)->setHidden(true);
						(*ait)->setCheckState(0,Qt::Unchecked);
					}
				}
				++ait;
			}
			QTreeWidgetItemIterator projit(ui.treeWidgetDataProducts);
			while (*projit) {
				if ((*projit)->data(0, ITEM_TYPE_ROLE) == PROJECT_ITEM) {
					bool setHiddenProject(true);
					for (int i = 0; i < (*projit)->childCount(); ++i) {
						if (!(*projit)->child(i)->isHidden()) {
							setHiddenProject = false;
							break;
						}
					}
					if (setHiddenProject) {
						(*projit)->setHidden(true);
						(*projit)->setCheckState(0,Qt::Unchecked);
					}
				}
				++projit;
			}
		}

		// update the total size info shown for projects & tasks

		QTreeWidgetItemIterator item(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::NotHidden|QTreeWidgetItemIterator::HasChildren);
		while (*item) {
			if ((*item)->data(0,ITEM_TYPE_ROLE) == PROJECT_ITEM) {
				updateProjectItemSizes(*item);
			}
			++item;
		}
	}
	if (itsFilterDialog->filterApplied()) {
		ui.tabWidget->setTabText(0,"Project Data (Filtered)");
	}
	else {
		ui.tabWidget->setTabText(0,"Project Data");
	}

	ui.treeWidgetDataProducts->blockSignals(false);
	ui.treeWidgetDataProducts->setSortingEnabled(true);

	updateProjectPieView();
}


void CEPCleanMainWindow::updateProjectItemSizes(QTreeWidgetItem *item) {
	DataTreeWidgetItem *task(0), *dataProduct(0);
	std::map<QString, ProjectDataProducts>::const_iterator projectIt;
	std::map<unsigned, TaskDataProducts>::const_iterator taskIt;
	std::map<dataProductTypes, DataProductInfo>::const_iterator dataProductIt;
	quint64 totalProjectSize(0), totalTaskSize(0);

	projectIt = itsProjectsData.find(item->text(0));
	if (projectIt!= itsProjectsData.end()) {
		totalProjectSize = 0;
		const std::map<unsigned, TaskDataProducts> &tasks(projectIt->second.taskDataProducts());
		for (int i = 0; i < item->childCount(); ++i) {
			task = static_cast<DataTreeWidgetItem *>(item->child(i));
			if (!task->isHidden()) {
				taskIt = tasks.find(task->text(1).toUInt()); // text(1) = sasID
				if (taskIt != tasks.end()) {
					totalTaskSize = 0;
					// iterate over the data products of the task
					const std::map<dataProductTypes, DataProductInfo> &dataProducts(taskIt->second.dataProducts());
					for (int di = 0; di < task->childCount(); ++di) {
						dataProduct = static_cast<DataTreeWidgetItem *>(task->child(di));
						if (!dataProduct->isHidden()) {
							dataProductIt = dataProducts.find((dataProductTypes)dataProduct->data(0, DATA_TYPE_ROLE).toInt());
							if (dataProductIt != dataProducts.end()) {
								totalTaskSize += dataProductIt->second.totalSize;
							}
						}
					}
					task->setText(10, QString(humanReadableUnits(totalTaskSize, SIZE_UNITS).c_str()));
					task->setDataValue(totalTaskSize);
					totalProjectSize += totalTaskSize;
				}
			}
		}
		item->setText(10, QString(humanReadableUnits(totalProjectSize, SIZE_UNITS).c_str()));
		static_cast<DataTreeWidgetItem *>(item)->setDataValue(totalProjectSize);
	}
}

void CEPCleanMainWindow::updateProjectPieView(void) {
	unsigned nrProjects(0);
	itsProjectPieModel->removeRows(0, itsProjectPieModel->rowCount(QModelIndex()), QModelIndex());
	QTreeWidgetItemIterator item(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::NotHidden|QTreeWidgetItemIterator::HasChildren);
//	itsProjectPieModel->setHeaderData(0, Qt::Horizontal, tr("Project"));
//	itsProjectPieModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
	quint64 projectSize;
	while (*item) {
		if ((*item)->data(0, ITEM_TYPE_ROLE) == PROJECT_ITEM) {
			itsProjectPieModel->insertRows(nrProjects, 1, QModelIndex());
			projectSize = (static_cast<DataTreeWidgetItem *>(*item))->dataValue();
			const QModelIndex &idx1(itsProjectPieModel->index(nrProjects, 1, QModelIndex()));
			itsProjectPieModel->setData(idx1, (*item)->text(0), Qt::UserRole);
			itsProjectPieModel->setData(itsProjectPieModel->index(nrProjects, 0, QModelIndex()), (*item)->text(0) + " (" + humanReadableUnits(projectSize).c_str() + ")");
			itsProjectPieModel->setData(itsProjectPieModel->index(nrProjects, 1, QModelIndex()), projectSize);
			++nrProjects;
		}
		++item;
	}
	itsPieProjectsView->update();
}

void CEPCleanMainWindow::updateTasksPie(void) {
	QTreeWidgetItem *item(ui.treeWidgetDataProducts->selectedItems().first());
	int itemType(item->data(0, ITEM_TYPE_ROLE).toInt());
	if (itemType == TASK_ITEM) { // if this is a task item
		item = item->parent();
	}
	else if (itemType == DATA_PRODUCT_ITEM) { // if this is a data product item
		item = item->parent()->parent();
	}
	updateTasksPie(item->text(0));
}

void CEPCleanMainWindow::updateTasksPie(const QModelIndex &index) {
	updateTasksPie(index.data(Qt::UserRole).toString());
}

void CEPCleanMainWindow::updateTasksPie(const QString &projectName) {
	if (!projectName.isEmpty()) {
		itsTasksPieModel->removeRows(0, itsTasksPieModel->rowCount(QModelIndex()), QModelIndex());
		QTreeWidgetItemIterator itemIterator(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::NotHidden|QTreeWidgetItemIterator::HasChildren);
		bool found_project(false);
		while (!found_project) {
			if ((*itemIterator)->data(0, ITEM_TYPE_ROLE) == PROJECT_ITEM) {
				if ((*itemIterator)->text(0) == projectName) {
					found_project = true;
					break;
				}
			}
			++itemIterator;
		}
		if (found_project) {
			quint64 taskSize;
			short addedChildIdx(0);
			DataTreeWidgetItem *taskChild;
			for (int idx = 0; idx < (*itemIterator)->childCount(); ++ idx) {
				taskChild = static_cast<DataTreeWidgetItem *>((*itemIterator)->child(idx));
				if (!taskChild->isHidden()) {
					itsTasksPieModel->insertRows(addedChildIdx, 1, QModelIndex());
					taskSize = taskChild->dataValue();
					itsTasksPieModel->setData(itsTasksPieModel->index(addedChildIdx, 0, QModelIndex()), taskChild->text(0) + " (" + humanReadableUnits(taskSize).c_str() + ")");
					itsTasksPieModel->setData(itsTasksPieModel->index(addedChildIdx, 1, QModelIndex()), taskSize);
					if (addedChildIdx++ > 50) break; // show only first 50 tasks
				}
			}
		}
		itsPieTasksView->setTitle("Project:" + projectName);
		itsPieTasksView->update();
	}
}



void CEPCleanMainWindow::saveProjectData(QDataStream &out) {
	out << (quint32)itsProjectsData.size();
	for (std::map<QString, ProjectDataProducts>::const_iterator pdit = itsProjectsData.begin(); pdit != itsProjectsData.end(); ++pdit) {
		out << pdit->first << pdit->second;
	}
}

void CEPCleanMainWindow::readProjectData(QDataStream &in) {
	itsProjectsData.clear();
	ProjectDataProducts pdata;
	QString projectName;
	quint32 size;
	in >> size;
	for (size_t i = 0; i < size; ++i) {
		in >> projectName >> pdata;
		itsProjectsData[projectName] = pdata;
	}
}

void CEPCleanMainWindow::closeEvent(QCloseEvent *event) {
	event->ignore();
	done();
}


void CEPCleanMainWindow::done(void) {
	if (itsCreateLog && itsKeepLog) {
		std::system(("mail -s " + itsLogFile->fileName() + " CEPclean -b jong@astron.nl,pizzo@astron.nl,frieswijk@astron.nl < " + itsLogFile->fileName()).toStdString().c_str());
	}
	itsController->cleanupDialogClosed();
	exit(0);
}

void CEPCleanMainWindow::showItemDetails(QTreeWidgetItem *item, int /*col*/) {
	itsDataProductInfoDialog.ui.listWidgetFiles->clear();
	if (item && (item->data(0,ITEM_TYPE_ROLE).toInt() == DATA_PRODUCT_ITEM)) {
		itsDataProductInfoDialog.ui.lineEditDataType->clear();
		itsDataProductInfoDialog.ui.lineEditTaskName->clear();
		itsDataProductInfoDialog.ui.lineEditSAS_ID->clear();
		itsDataProductInfoDialog.ui.lineEditRunDate->clear();
		itsDataProductInfoDialog.ui.lineEditDuration->clear();
		itsDataProductInfoDialog.ui.lineEditExpireDate->clear();
		itsDataProductInfoDialog.ui.checkBoxExpired->setChecked(false);
		itsDataProductInfoDialog.ui.checkBoxDeleted->setChecked(false);
		itsDataProductInfoDialog.ui.lineEditNrOfFiles->clear();
		itsDataProductInfoDialog.ui.lineEditPath->clear();
		itsDataProductInfoDialog.ui.lineEditTotalSize->clear();
		itsDataProductInfoDialog.ui.listWidgetFiles->clear();
		if (item->parent()) {
			itsDataProductInfoDialog.ui.lineEditTaskName->setText(item->parent()->text(0));
			itsDataProductInfoDialog.ui.lineEditSAS_ID->setText(item->parent()->text(1));
			itsDataProductInfoDialog.ui.lineEditRunDate->setText(item->parent()->text(2));
			itsDataProductInfoDialog.ui.lineEditDuration->setText(item->parent()->text(3));
		}
		else {
			itsDataProductInfoDialog.ui.lineEditTaskName->clear();
			itsDataProductInfoDialog.ui.lineEditSAS_ID->clear();
			itsDataProductInfoDialog.ui.lineEditRunDate->clear();
			itsDataProductInfoDialog.ui.lineEditDuration->clear();
		}
		itsDataProductInfoDialog.ui.lineEditDataType->setText(item->text(0));
		itsDataProductInfoDialog.ui.lineEditExpireDate->setText(item->text(4));

		(item->text(5) == "yes") ? itsDataProductInfoDialog.ui.checkBoxExpired->setChecked(true) :
				itsDataProductInfoDialog.ui.checkBoxExpired->setChecked(false);
		(item->text(6) == "yes") ? itsDataProductInfoDialog.ui.checkBoxDeleted->setChecked(true) :
				itsDataProductInfoDialog.ui.checkBoxDeleted->setChecked(false);

		itsDataProductInfoDialog.ui.lineEditNrOfFiles->setText(item->text(8));
		itsDataProductInfoDialog.ui.lineEditPath->setText(item->text(11));
		itsDataProductInfoDialog.ui.lineEditTotalSize->setText(item->text(10));

		// file list
		const QStringList &fileSizes(item->data(9, Qt::UserRole).toStringList()); // contains file sizes as text, '-' if file does not exist
		const QStringList &fileNames(item->data(8, Qt::UserRole).toStringList());
		const QStringList &nodes(item->data(12, Qt::UserRole).toStringList());
		QColor red("red"), black("black");
		for (int i = 0; i < fileSizes.size(); ++i) {
			QListWidgetItem *item;
			if (fileSizes.at(i) == "-") {
				item = new QListWidgetItem(fileNames.at(i) + "\t(-) " + nodes.at(i));
				item->setTextColor(red);
			}
			else {
				item = new QListWidgetItem(fileNames.at(i) + "\t(" + fileSizes.at(i) + ") "+ nodes.at(i));
				item->setTextColor(black);
			}
			itsDataProductInfoDialog.ui.listWidgetFiles->insertItem(i,item);
		}
		itsDataProductInfoDialog.show();
		itsDataProductInfoDialog.raise();
	}
}

void CEPCleanMainWindow::contextMenuEvent(QContextMenuEvent *event) {
	QMenu menu;
	QAction *action;
	action = menu.addAction("Collapse");
	connect(action, SIGNAL(triggered()), this, SLOT(collapseItems(void)));
	action = menu.addAction("Expand");
	connect(action, SIGNAL(triggered()), this, SLOT(expandItems(void)));
	action = menu.addAction("Check for deletion");
	connect(action, SIGNAL(triggered()), this, SLOT(checkItems(void)));
	action = menu.addAction("Uncheck for deletion");
	connect(action, SIGNAL(triggered()), this, SLOT(unCheckItems(void)));
	action = menu.addAction("Check VIC tree for deletion");
	connect(action, SIGNAL(triggered()), this, SLOT(checkVICtree(void)));
	action = menu.addAction("Uncheck VIC tree for deletion");
	connect(action, SIGNAL(triggered()), this, SLOT(unCheckVICtree(void)));
    action = menu.addAction("Show Totals");
    connect(action, SIGNAL(triggered()), this, SLOT(showTotals(void)));
	const QList<QTreeWidgetItem*> &selectedProjects(ui.treeWidgetDataProducts->selectedItems());
	if (selectedProjects.size() == 1) {
		action = menu.addAction("Create Pie chart of project");
		connect(action, SIGNAL(triggered()), this, SLOT(updateTasksPie(void)));
	}

	menu.exec(event->globalPos());
}

void CEPCleanMainWindow::collapseItems(void) {
	ui.treeWidgetDataProducts->setSortingEnabled(false);
	this->blockSignals(true);
	QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
	for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setExpanded(false);
	}
	this->blockSignals(false);
	ui.treeWidgetDataProducts->setSortingEnabled(true);
}

void CEPCleanMainWindow::expandItems(void) {
	ui.treeWidgetDataProducts->setSortingEnabled(false);
	this->blockSignals(true);
	QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
	for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setExpanded(true);
	}
	this->blockSignals(false);
	ui.treeWidgetDataProducts->setSortingEnabled(true);
}

void CEPCleanMainWindow::checkItems(void){
	this->blockSignals(true);
	QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
	for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setCheckState(0, Qt::Checked);
	}
	this->blockSignals(false);
}

void CEPCleanMainWindow::unCheckItems(void){
	this->blockSignals(true);
	QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
	for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		(*it)->setCheckState(0, Qt::Unchecked);
	}
	this->blockSignals(false);
}

void CEPCleanMainWindow::checkVICtree(void){
	this->blockSignals(true);
	ui.treeWidgetDataProducts->blockSignals(true);
	QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
	int itemType;
	for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		itemType = (*it)->data(0,ITEM_TYPE_ROLE).toInt();
		if (itemType == TASK_ITEM) {
			if ((*it)->checkState(0) == Qt::Checked) {
				(*it)->setCheckState(1, Qt::Checked);
			}
		}
		else if (itemType == DATA_PRODUCT_ITEM) {
			if ((*it)->parent()->checkState(0) == Qt::Checked) {
				(*it)->parent()->setCheckState(1, Qt::Checked);
			}
		}
	}
	ui.treeWidgetDataProducts->blockSignals(false);
	this->blockSignals(false);
}

void CEPCleanMainWindow::showTotals(void) {
    QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
    quint64 totalProjectSize(0), totalTaskSize(0);
    int itemType;
    for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
        itemType = (*it)->data(0,ITEM_TYPE_ROLE).toInt();
        if (itemType == PROJECT_ITEM) {
            totalProjectSize += static_cast<DataTreeWidgetItem *>(*it)->dataValue();
        }
        else if (itemType == TASK_ITEM) {
            totalTaskSize += static_cast<DataTreeWidgetItem *>(*it)->dataValue();
        }
    }
    QMessageBox::information(this, "Total size of selection", QString("The total size of the selected projects is:") + humanReadableUnits(totalProjectSize).c_str()
                             + "\nThe total size of the selected tasks is:" + humanReadableUnits(totalTaskSize).c_str());
}

void CEPCleanMainWindow::parsetTreeView(const QString &parset, const OTDBtree &otdb_tree) {
    itsTreeViewer->view(parset, otdb_tree);
}

void CEPCleanMainWindow::unCheckVICtree(void){
	this->blockSignals(true);
	ui.treeWidgetDataProducts->blockSignals(true);
	QList<QTreeWidgetItem*> items(ui.treeWidgetDataProducts->selectedItems());
	int itemType;
	for (QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
		itemType = (*it)->data(0,ITEM_TYPE_ROLE).toInt();
		if (itemType == TASK_ITEM) {
			(*it)->setCheckState(1, Qt::Unchecked);
		}
		else if (itemType == DATA_PRODUCT_ITEM) {
			(*it)->parent()->setCheckState(1, Qt::Unchecked);
		}
	}
	ui.treeWidgetDataProducts->blockSignals(false);
	this->blockSignals(false);
}

void CEPCleanMainWindow::updateAndShow(bool updateFileInfo) {
	if (updateFileInfo) {
        itsController->refreshStorageNodesInfo();
		doUpdateFiles();
	}
	DataTreeWidgetItem *projectItem, *taskItem, *dataProductItem;

	QString sasTreeIDstr, nodesToolTip;
	QStringList taskUniqueNodes;
	ui.treeWidgetDataProducts->clear();

	// calculate all totals in itsProjectData
	for (std::map<QString, ProjectDataProducts>::iterator it = itsProjectsData.begin(); it != itsProjectsData.end(); ++ it) {
		std::map<unsigned, TaskDataProducts> &taskDataProducts(it->second.taskDataProductsForChange());
		for (std::map<unsigned, TaskDataProducts>::iterator tit = taskDataProducts.begin(); tit != taskDataProducts.end(); ++tit) {
			std::map<dataProductTypes, DataProductInfo> &dataproducts(tit->second.dataProductsForChange());
			for (std::map<dataProductTypes, DataProductInfo>::iterator dpit = dataproducts.begin(); dpit != dataproducts.end(); ++dpit) {
				dpit->second.calculateTotalSize();
			}
		}
	}

	for (std::map<QString, ProjectDataProducts>::const_iterator projectit = itsProjectsData.begin(); projectit != itsProjectsData.end(); ++projectit) {
		projectItem = new DataTreeWidgetItem((QTreeWidget*)0, QStringList(projectit->first));
		projectItem->setData(0, ITEM_TYPE_ROLE, PROJECT_ITEM);
		for (int col=0; col <  ui.treeWidgetDataProducts->columnCount(); ++col) {
			projectItem->setTextColor(col,Qt::black);
		}
		quint64 totalSize(projectit->second.totalSizeOnDisk());
		projectItem->setText(10, QString(humanReadableUnits(totalSize, SIZE_UNITS).c_str()));
		projectItem->setDataValue(totalSize);
		const std::map<unsigned, TaskDataProducts> &taskDataProducts(projectit->second.taskDataProducts());
		for (std::map<unsigned, TaskDataProducts>::const_iterator taskit = taskDataProducts.begin(); taskit != taskDataProducts.end(); ++taskit) {
			taskUniqueNodes.clear();
			const OTDBtree &SAStree(taskit->second.SAStree());
			sasTreeIDstr = QString::number(taskit->first);

			const std::map<dataProductTypes, DataProductInfo> &dataProducts(taskit->second.dataProducts());
			taskItem = new DataTreeWidgetItem(projectItem, QStringList(taskit->second.taskName()));
			taskItem->setData(0, ITEM_TYPE_ROLE, TASK_ITEM);
			for (int col=0; col <  ui.treeWidgetDataProducts->columnCount(); ++col) {
				taskItem->setTextColor(col,Qt::darkBlue);
			}

			taskItem->setData(11, Qt::UserRole,QString("/data/L") + sasTreeIDstr);
			taskItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
			taskItem->setCheckState(0,Qt::Unchecked); // for deletion of the task's data products
			taskItem->setCheckState(1,Qt::Unchecked); // for deletion of the VIC tree
			// SAS ID
			taskItem->setText(1, sasTreeIDstr); // the task's SAS ID
			taskItem->setData(1, Qt::UserRole, taskit->first); // the SAS ID as an int
			// run date
			taskItem->setText(2, SAStree.startTime().getDate().toString(1).c_str());
			taskItem->setData(2, Qt::UserRole, SAStree.startTime().getDate().toQDate());
			// duration
			if (SAStree.stopTime().isSet() && SAStree.startTime().isSet()) {
				taskItem->setText(3, SAStree.startTime().timeDifference(SAStree.stopTime()).toString().c_str());
			}
			else {
				taskItem->setText(3, "duration unknown");
			}

			// total size of all data for this task
			taskItem->setDataValue(taskit->second.totalSizeOnDisk());
			taskItem->setText(10, taskItem->humanReadible());

			taskItem->setText(13, taskit->second.SASDBName());

			for (std::map<dataProductTypes, DataProductInfo>::const_iterator dpit = dataProducts.begin(); dpit != dataProducts.end(); ++dpit) {
				dataProductItem = new DataTreeWidgetItem(taskItem, QStringList(QString(DATA_PRODUCTS[dpit->first])));
				dataProductItem->setData(0, ITEM_TYPE_ROLE, DATA_PRODUCT_ITEM);
				dataProductItem->setData(0, DATA_TYPE_ROLE, (int)dpit->first);

				for (int col=0; col <  ui.treeWidgetDataProducts->columnCount(); ++col) {
	//				dataProductItem->setBackgroundColor(Qt::black);
					dataProductItem->setTextColor(col,Qt::darkMagenta);
				}


				// expire date
				dataProductItem->setText(4,dpit->second.expiryDate.toString(Qt::ISODate));
				// expired?
				if (dpit->second.expired) {
					dataProductItem->setText(5,"yes");
				}
				else {
					dataProductItem->setText(5,"no");
				}
				// deleted?
				if (dpit->second.deleted) {
					dataProductItem->setText(6,"yes");
				}
				else {
					dataProductItem->setText(6,"no");
				}
				// file size difference
				bool dif_size(false);
				quint64 max(0);
				if (!dpit->second.fileSizes.empty()) {
					max = *std::max_element(dpit->second.fileSizes.begin(), dpit->second.fileSizes.end());
				}
				if (std::find(dpit->second.exists.begin(), dpit->second.exists.end(), false) == dpit->second.exists.end()) { // if all files exist
					quint64 pct5 = max/20;
					for (std::vector<quint64>::const_iterator fsit = dpit->second.fileSizes.begin(); fsit != dpit->second.fileSizes.end(); ++fsit) {
						if (max - *fsit > pct5) {
							dataProductItem->setText(7,"diff size");
							dif_size = true;
							break;
						}
					}
					if (!dif_size) dataProductItem->setText(7,"yes"); // all files exist and are (approximately) equal size
				}
				else { // contains at least one non existing file
					if (std::find(dpit->second.exists.begin(), dpit->second.exists.end(), true) == dpit->second.exists.end()) {
						dataProductItem->setText(7,"no"); // no files where found at all
					}
					else dataProductItem->setText(7,"partial"); // some files where found
				}
				// exists
//				dataProductItem->setData(7, Qt::UserRole,dpit->second.exists);

				// # files
				dataProductItem->setText(8,QString::number(dpit->second.fileNames.size()));
				dataProductItem->setData(8, Qt::UserRole,dpit->second.fileNames);

				// file size
				dataProductItem->setText(9, humanReadableUnits(max, SIZE_UNITS).c_str());

				// store file sizes as text for detailed info
				QStringList fileSizes;
				for (unsigned i=0; i < dpit->second.exists.size(); ++i) {
					if (dpit->second.exists.at(i)) {
						fileSizes.append(humanReadableUnits(dpit->second.fileSizes.at(i), SIZE_UNITS).c_str());
					}
					else {
						fileSizes.append("-");
					}
				}
				dataProductItem->setData(9, Qt::UserRole,fileSizes);

				// total size
				dataProductItem->setDataValue(dpit->second.totalSize);
				dataProductItem->setText(10, humanReadableUnits(dpit->second.totalSize, SIZE_UNITS).c_str());

				// local path
				QStringList paths = dpit->second.localPaths;
				dataProductItem->setData(11, Qt::UserRole, dpit->second.localPaths);
				paths.removeDuplicates();
				if (paths.size() == 1) {
					// will the path be deleted by deleting the parent task path? which is also selected for deletion. So check if the path starts the same as the parent path.
					if (paths.at(0).startsWith(dataProductItem->parent()->data(11,Qt::UserRole).toString())) {
						dataProductItem->setData(11,VARIOUS_PATHS_ROLE, 0);
						dataProductItem->setText(11, paths.at(0));
					}
					else { // single data product path but different from general path
						dataProductItem->setData(11,VARIOUS_PATHS_ROLE, 1);
						dataProductItem->setText(11, paths.at(0));
					}
				}
				else { // various paths
					dataProductItem->setData(11,VARIOUS_PATHS_ROLE, 2);
					dataProductItem->setText(11, "various locations");
				}

				// nodes tooltip
				// calculate totals etc in itsProjectData

				QStringList dpUniqueNodes(dpit->second.nodes);
				dpUniqueNodes.removeDuplicates();
				dataProductItem->setText(12, QString::number(dpUniqueNodes.size()));
				int sz(dpUniqueNodes.size());
				if (sz > 0) {
					nodesToolTip = "";
					int ni(0);
					for (QStringList::const_iterator nsit = dpUniqueNodes.begin(); nsit != dpUniqueNodes.end(); ++nsit) {
						nodesToolTip += *nsit;
						if (++ni != sz) {
							nodesToolTip += ",";
						}
						if ((ni % 4 == 0) & (ni != sz)) nodesToolTip += "\n";
					}
					dataProductItem->setToolTip(12, nodesToolTip);
				}

				// nodes user data
//				nodes = dpit->second.nodes.join(",");
				dataProductItem->setData(12, Qt::UserRole,dpit->second.nodes); // store the node names QStringList in the data product item column 1 user data
				taskUniqueNodes += dpUniqueNodes;

				dataProductItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
				dataProductItem->setCheckState(0,Qt::Unchecked);
			}
			taskUniqueNodes.removeDuplicates();
			taskItem->setData(12, Qt::UserRole, taskUniqueNodes);
		}
		projectItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		projectItem->setCheckState(0,Qt::Unchecked);
		ui.treeWidgetDataProducts->addTopLevelItem(projectItem);
	}
	ui.treeWidgetDataProducts->setSortingEnabled(true);
	ui.treeWidgetDataProducts->header()->resizeSections(QHeaderView::ResizeToContents);
	if (!isVisible()) {
		show();
	}
	connect(ui.treeWidgetDataProducts,SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(checkItemChange(QTreeWidgetItem *, int)));

	updateProjectPieView();
}

// checkExcludeStrings returns false if the path+filename in chkStr should be excluded, true if it should be deleted
bool CEPCleanMainWindow::checkExcludeStrings(const QString &chkStr) {
    for (QMap<QString, bool>::iterator it = itsFoundExcludedStrings.begin(); it != itsFoundExcludedStrings.end(); ++it) {
        if (chkStr.contains(it.key())) {
            itsFoundExcludedStrings[it.key()] = true;
            return false;
        }
    }
    return true;
}

void CEPCleanMainWindow::doDelete(void) {
	// check which project/tasks have been selected
	// if a project is selected do a looping 'rm -R data/Lxxxxx' over all tasks for that project
	// if a task (observation/pipeline) is selected do a single 'rm -R data/Lxxxxx'
	// if a single data product type is selected add the individual files of that data product to the remove list

    // initialize the found excluded strings bookkeeping
    itsFoundExcludedStrings.clear();
    QStringList exStrings = Controller::theSchedulerSettings.getExludedStrings(); // get all excluded strings from settings
    for (QStringList::const_iterator it = exStrings.begin(); it != exStrings.end(); ++it) {
        itsFoundExcludedStrings[*it] = false;
    }

	itsVICtreesToDelete.clear();
	itsNodesDeleteCommands.clear();

	CEPdeleteDialog *cepDlg = new CEPdeleteDialog(this);
	connect(cepDlg, SIGNAL(rejected()), this, SLOT(enableDeleteButton()));
	ui.actionDelete->setEnabled(false);

	std::vector<QStringList> logInfo;
	QStringList nodes, localPaths, files, confirmInfo;
	QTreeWidgetItem *taskItem, *projectItem;
	QTreeWidgetItemIterator treeIt(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::Checked);
	int itemType;
	Qt::CheckState parentCheckState;
	while (*treeIt) {
		itemType = (*treeIt)->data(0, ITEM_TYPE_ROLE).toInt();
		if (itemType != PROJECT_ITEM) { // skip project items, they cannot be deleted
			parentCheckState = (*treeIt)->parent()->checkState(0);
			confirmInfo.clear();
			// determine if this is a task or a data product item
			if (itemType == DATA_PRODUCT_ITEM) {
				// this is a single data product item of which the parent task is not checked for removal
				// determine the parents obs Lxxxxx dir and add the individual files to the remove list
				// sas_id = (*it)->parent()->data(0,Qt::UserRole).toInt();
				taskItem = 0;
				if (parentCheckState != Qt::Checked) {
					nodes = (*treeIt)->data(12, Qt::UserRole).toStringList(); // the nodes string list for this data product
					if (!nodes.isEmpty()) {
						localPaths = (*treeIt)->data(11, Qt::UserRole).toStringList(); // the local path
						if (!localPaths.isEmpty()) {
							files = (*treeIt)->data(8, Qt::UserRole).toStringList(); // the individual files
							if (!files.isEmpty()) {
								if (localPaths.size() == 1) {
                                    for (int i = 0; i < files.size(); ++i) {
                                        if (checkExcludeStrings(localPaths.at(0)))
                                            itsNodesDeleteCommands[nodes.at(i)].append(localPaths.at(0) + files.at(i));
                                    }
                                }
								else {
									for (int i = 0; i < files.size(); ++i) {
                                        if (checkExcludeStrings(localPaths.at(i)))
                                            itsNodesDeleteCommands[nodes.at(i)].append(localPaths.at(i) + files.at(i));
									}
								}

								// add the information to the confirm delete dialog
								// arguments: project, task, sas ID, run date, duration, #files, #nodes
								projectItem = (*treeIt)->parent()->parent();
								taskItem = (*treeIt)->parent();
								confirmInfo << projectItem->text(0) // project name
							    			<< taskItem->text(0)    // task name
							    			<< taskItem->text(1)    // sas ID
							    			<< taskItem->text(2)    // run date
							    			<< taskItem->text(3)    // duration
							    			<< (*treeIt)->text(0)   // data type
							    			<< (*treeIt)->text(8)   // #files
							    			<< (*treeIt)->text(12)  // #nodes
							    			<< (*treeIt)->text(10); // total size
								cepDlg->addTask(confirmInfo);
								logInfo.push_back(confirmInfo);
							}
						}
					}
				}
				// add the data product item to the list of data products that need to be marked deleted in OTDB
				if (taskItem) {
					if (taskItem->checkState(1) != Qt::Checked) { // VIC tree is not marked for deletion?
						dataProductTypes dptype(static_cast<dataProductTypes>((*treeIt)->data(0, DATA_TYPE_ROLE).toInt()));
						if (dptype < DP_UNKNOWN_TYPE) {
							cepDlg->addMarkedDeleted(taskItem->text(13), taskItem->data(1,Qt::UserRole).toInt(), dptype);
						}
					}
				}
			}
			else if ((itemType == TASK_ITEM) && ((*treeIt)->checkState(0) == Qt::Checked)) { // TASK_ITEM and fully (not partial) checked
				// this is a task item, determine the obs Lxxxxx dir and add the directory to the remove list
				nodes = (*treeIt)->data(12, Qt::UserRole).toStringList(); // the nodes string list for this data product
                bool mayBeDeleted(true);
				if (!nodes.isEmpty()) {
					QString localPath((*treeIt)->data(11, Qt::UserRole).toString());
                    mayBeDeleted = checkExcludeStrings(localPath);
                    if (!localPath.isEmpty() && mayBeDeleted) {
						for (int i = 0; i < nodes.size(); ++i) {
                            itsNodesDeleteCommands[nodes.at(i)].append(localPath);
						}

						QTreeWidgetItem *child(0);
						for (int chi = 0; chi < (*treeIt)->childCount(); ++chi) {
							child = (*treeIt)->child(chi);
							int pathType(child->data(11,VARIOUS_PATHS_ROLE).toInt());
							if (pathType == 1) { // singular path but different from general path
								QStringList childPaths(child->data(11, Qt::UserRole).toStringList());
								QStringList childNodes(child->data(12, Qt::UserRole).toStringList());
								QStringList childFiles(child->data(8, Qt::UserRole).toStringList());
								for (int i = 0; i < childNodes.size(); ++i) {
                                    if (checkExcludeStrings(childPaths.at(0)))
                                        itsNodesDeleteCommands[childNodes.at(i)].append(childPaths.at(0) + childFiles.at(i));
								}
							}
							else if (pathType == 0) { // equal to general path, don't add because it is already added by the deletion of the task item itself

							}
							else if (pathType == 2) { // various paths, add each one of them
								QStringList childPaths(child->data(11, Qt::UserRole).toStringList());
								QStringList childNodes(child->data(12, Qt::UserRole).toStringList());
								QStringList childFiles(child->data(8, Qt::UserRole).toStringList());
								for (int i = 0; i < childNodes.size(); ++i) {
                                    if (checkExcludeStrings(childPaths.at(i)))
                                        itsNodesDeleteCommands[childNodes.at(i)].append(childPaths.at(i) + childFiles.at(i));
								}
							}
						}

						// add the information to the confirm delete dialog
						// arguments: project, task, sas ID, run date, duration, which data, #files, #nodes, size
						projectItem = (*treeIt)->parent();
						taskItem = (*treeIt);
						// project name
						if (projectItem) {
							confirmInfo << projectItem->text(0);
						}
						else confirmInfo << "unknown";
						confirmInfo << taskItem->text(0)     // task name
								    << taskItem->text(1)     // sas ID
								    << taskItem->text(2)     // run date
								    << taskItem->text(3)     // duration
								    << "all"                 // which data
								    << (*treeIt)->text(8)    // #files
								    << (*treeIt)->text(12)   // #nodes
								    << (*treeIt)->text(10);  // total size of all data of this task
						cepDlg->addTask(confirmInfo);
						logInfo.push_back(confirmInfo);
					}
				}
                if (mayBeDeleted && ((*treeIt)->checkState(0) != Qt::PartiallyChecked) && ((*treeIt)->checkState(1) == Qt::Checked)) { // VIC tree deletion as well?
					itsVICtreesToDelete[(*treeIt)->text(13)].append((*treeIt)->text(1));
				}
			}
		}
		++treeIt;
	}


	// merge the delete items to a single command for each node
	QString cmd;
	std::map<QString, QString> cmdList;
	for (std::map<QString, QStringList>::const_iterator it = itsNodesDeleteCommands.begin(); it != itsNodesDeleteCommands.end(); ++it) {
		if (!it->second.isEmpty()) {
			cmd = "rm -rf " + it->second.join(" ");
			cmdList[it->first] = cmd;
		}
	}

	cepDlg->setNodesCommandsInfo(cmdList);
	cepDlg->setVICtreesToDelete(itsVICtreesToDelete);

	if (!(cmdList.empty() && itsVICtreesToDelete.empty())) {
		writeLog("\n### New deletion request ###");

        bool permanentFilterActived(false);
        for (QMap<QString, bool>::const_iterator fit = itsFoundExcludedStrings.begin(); fit != itsFoundExcludedStrings.end(); ++fit) {
            if (fit.value()) {
                permanentFilterActived = true;
                break;
            }
        }
        if (permanentFilterActived) {
            writeLog("\n--------------------------------------------------------------------------------");
            writeLog("** Permanent Deletion Filter activated **\nThe following directory strings were found by the permanent deletion filter:");
            QString fndStrings;
            for (QMap<QString, bool>::const_iterator fit = itsFoundExcludedStrings.begin(); fit != itsFoundExcludedStrings.end(); ++fit) {
                if (fit.value()) {
                    writeLog(fit.key());
                    fndStrings += fit.key() + "\n";
                }
            }
            writeLog("Directories that contain these strings have been excluded from the deletion request.");
            writeLog("--------------------------------------------------------------------------------");
            if (!fndStrings.isEmpty()) {
                if (QMessageBox::question(this, "Permanent filter activated", "Found the following excluded strings in the deletion request:\n" + fndStrings +
                                          "Directories containing those strings are excluded from the deletion.\nContinue?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
                    writeLog("User Cancel request.\nNo data has been deleted.");
                    if (itsLogFile) itsLogFile->close();
                    QApplication::restoreOverrideCursor();
                    ui.actionDelete->setEnabled(true);
                    return;
                }
            }
        }

		// write info to log file
        writeLog("\nSelected deletion tasks:\nProject; Task; obsID; run-date; duration; which data; #files; #nodes; size\n");
		for (std::vector<QStringList>::const_iterator lit = logInfo.begin(); lit != logInfo.end(); ++lit) {
			writeLog(lit->join(";"));
		}
		int retVal(cepDlg->exec());
		if (retVal == 0) { // everything went ok
			itsKeepLog = true;
			updateTreeInfoAfterDelete(true);
			if (itsLogFile) itsLogFile->close();
		}
		else if (retVal == 2) {
			itsKeepLog = true;
			if (itsLogFile) itsLogFile->close();
			// data was deleted successfully but the connection to the SAS database was not ok, so could not mark the data as deleted nor delete the vic tree
			QMessageBox::warning(this, "SAS database not updated", "Although data was deleted the SAS database could not be updated.\nThe trees will still be shown but will have updated file sizes");
			updateTreeInfoAfterDelete(false);
		}
		else if (retVal == 3) { // user clicked cancel
			writeLog("User Cancel request.\nNo data has been deleted.");
			if (itsLogFile) itsLogFile->close();
		}
		else { // retVal = 1
			itsKeepLog = true;
			writeLog("There were some errors trying to delete data.");
			// something went wrong deleting the data, it cannot be assumed to be deleted, so don't mark it
		}
	}
	else {
		QMessageBox::warning(0, tr("Nothing to delete"),
				tr("There is no data selected for deletion."));
	}
	QApplication::restoreOverrideCursor();
	ui.actionDelete->setEnabled(true);
}

void CEPCleanMainWindow::updateTreeInfoAfterDelete(bool SASupdated) {
	ui.treeWidgetDataProducts->blockSignals(true);
	// remove the trees that have been deleted
	if (SASupdated) {
		QList<QTreeWidgetItem*> items;
		for (deleteVICmap::const_iterator it = itsVICtreesToDelete.begin(); it != itsVICtreesToDelete.end(); ++it) {
			for (QStringList::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
				items = ui.treeWidgetDataProducts->findItems(*sit,Qt::MatchExactly|Qt::MatchRecursive,1);
				if (!items.empty()) {
					items.first()->parent()->removeChild(items.first());
					delete items.first();
				}
			}
		}
	}

	// now update the properties in the tree
	QTreeWidgetItemIterator treeIt(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::Checked);
	QString markDeletedText = SASupdated ? "yes" : "no";
	int itemType;
	while (*treeIt) {
		itemType = (*treeIt)->data(0, ITEM_TYPE_ROLE).toInt();
		if (itemType == DATA_PRODUCT_ITEM) {
			std::map<unsigned, TaskDataProducts> &taskData(itsProjectsData[(*treeIt)->parent()->parent()->text(0)].taskDataProductsForChange());
			std::map<dataProductTypes, DataProductInfo> &dataProducts(taskData[(*treeIt)->parent()->text(1).toInt()].dataProductsForChange());
			DataProductInfo &dp(dataProducts[((dataProductTypes)(*treeIt)->data(0, DATA_TYPE_ROLE).toInt())]);
			dp.exists.assign(dp.fileNames.size(), false);
			dp.totalSize = 0;
			dp.deleted = true;
			if (SASupdated) {
				(*treeIt)->setText(6,markDeletedText); // mark data as deleted 'yes'/'no'
			}

			(*treeIt)->setCheckState(0,Qt::Unchecked);
			(*treeIt)->setText(7,"no");  // mark exists 'no'
			// set total data size to 0
			static_cast<DataTreeWidgetItem *>(*treeIt)->setDataValue(0); // set the total data size to 0
			(*treeIt)->setText(10, "0 kB");
		}
		else if (itemType == TASK_ITEM) {
			(*treeIt)->setCheckState(0,Qt::Unchecked);
		}
		++treeIt;
	}

	// now walk through the checked items again and update their project and task item's sizes
	QTreeWidgetItemIterator treeIt2(ui.treeWidgetDataProducts, QTreeWidgetItemIterator::Checked);
	while (*treeIt2) {
		if ((*treeIt2)->data(0, ITEM_TYPE_ROLE).toInt() == PROJECT_ITEM) {
			updateProjectItemSizes(*treeIt2);
			(*treeIt2)->setCheckState(0,Qt::Unchecked);
		}
		++treeIt2;
	}

	updateProjectPieView();

	ui.treeWidgetDataProducts->blockSignals(false);
}


void CEPCleanMainWindow::resetFileExistanceInfo(void) {
	for (std::map<QString, ProjectDataProducts>::iterator it = itsProjectsData.begin(); it != itsProjectsData.end(); ++ it) {
		std::map<unsigned, TaskDataProducts> &taskDataProducts(it->second.taskDataProductsForChange());
		for (std::map<unsigned, TaskDataProducts>::iterator tit = taskDataProducts.begin(); tit != taskDataProducts.end(); ++tit) {
			std::map<dataProductTypes, DataProductInfo> &dataProducts(tit->second.dataProductsForChange());
			for (std::map<dataProductTypes, DataProductInfo>::iterator dit = dataProducts.begin(); dit != dataProducts.end(); ++dit) {
				dit->second.exists.assign(dit->second.exists.size(),false);
				dit->second.fileSizes.assign(dit->second.fileSizes.size(),0);
			}
		}
	}
}

void CEPCleanMainWindow::doUpdateFiles(void) {
	if (fetchNodesData()) {
		resetFileExistanceInfo();
		parseDuResult(0,QProcess::NormalExit);
//		updateAndShow(false);
	}
	else {
		QMessageBox::critical(this, tr("Updating the nodes file info went wrong"),
					tr("Could not update the current files info from the locus nodes. The file size/exist info might be out dated"));
	}
}


bool CEPCleanMainWindow::getNodeLogFile(void) {
	QProcess fetch;
#ifdef Q_OS_UNIX
	fetch.start("scp -p lofarsys@lhn001.cep2.lofar:/tmp/calc_dir_size.log calc_dir_size.log");
#elif defined Q_OS_WIN
	fetch.start("pscp -p lofarsys@lhn001.cep2.lofar:/tmp/calc_dir_size.log calc_dir_size.log");
#else
	std::cerr << "ERROR, Unknown operating system. Don't know how to fetch the remote file calc_dir_size.log from lhn001" << std::endl;
	return false;
#endif

	if (!fetch.waitForStarted()) {
		std::cerr << "ERROR:fetch node log file process could not be started" << std::endl;
		return false;
	}
	if (!fetch.waitForFinished(180000)) {
		std::cerr << "ERROR:fetch node log file process could not be completed" << std::endl;
		return false;
	}
	if (fetch.exitStatus() == QProcess::NormalExit && fetch.exitCode() == 0) {
		return true;
	}
	else return false;
}

bool CEPCleanMainWindow::fetchNodesData(void) {
	// check date of log file. If of today then don't fetch it
	QFileInfo logfile("calc_dir_size.log");
	QMessageBox msgBox;
	msgBox.setWindowTitle("Fetching nodes file info");
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setModal(false);
	QCoreApplication::processEvents();

	if (logfile.exists()) {
		if (logfile.lastModified().date() != QDateTime::currentDateTime().date()) {
			msgBox.setText("file info is out of date, now fetching current file info for all locus nodes.\nPlease wait...");
			msgBox.open();
			QApplication::setOverrideCursor(Qt::WaitCursor);
			std::cout << "calc_dir_size.log is out of date. fetching the current from lhn001" << std::endl;
			if (getNodeLogFile()) {
				QApplication::restoreOverrideCursor();
				msgBox.close();
				return true;
			}
			else {
				QApplication::restoreOverrideCursor();
				msgBox.close();
				return false;
			}
		}
		else {
			std::cout << "calc_dir_size.log is current. skipping fetching the file from lhn001" << std::endl;
			return true;
		}
	}
	else {
		msgBox.setText("file info not yet fetched, now fetching current file info for all locus nodes.\nPlease wait...");
		msgBox.open();
		QApplication::setOverrideCursor(Qt::WaitCursor);
		std::cout << "calc_dir_size.log doesn't exist. fetching the current from lhn001" << std::endl;
		if (getNodeLogFile()) {
			QApplication::restoreOverrideCursor();
			msgBox.close();
			return true;
		}
		else {
			QApplication::restoreOverrideCursor();
			msgBox.close();
			return false;
		}
	}
}



void CEPCleanMainWindow::parseDuResult(int exitCode, QProcess::ExitStatus exitState) {
	if (exitCode == 0 && exitState == QProcess::NormalExit) {

		char buf[256];

		// now parse the file
		QMessageBox msgBox;
		msgBox.setWindowTitle("Updating nodes file info");
		msgBox.setText("now updating file info for all locus nodes.\nPlease wait...");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setModal(false);
		msgBox.open();
		QApplication::setOverrideCursor(Qt::WaitCursor);
		QCoreApplication::processEvents();

		std::cout << "now processing locus inventory log" << std::endl;
		QFile parseFile("calc_dir_size.log");
		parseFile.open(QIODevice::ReadOnly);

		parseFile.size();

        int nodeID(0);
        int pos(0);
		file f;
        QString nodeName;
		QStringList strlist;
		QRegExp whitespsplit("\\s+"), MS_subdirs("_uv.MS[./]"), SAS_ID("L[0-9]{5,}");
		quint64 totalNodeSpace(0);
		while (parseFile.readLine(buf, sizeof(buf)) != -1) {
			QString line(buf);
			if (line.startsWith("--------- locus")) {
				if (nodeID) {
					itsUsedNodesSpace[nodeID] = totalNodeSpace;
					totalNodeSpace = 0;
				}
				nodeID = itsController->getStorageNodeID(line.remove('-').trimmed().toStdString());
				nodeName = itsController->getStorageNodeName(nodeID).c_str();
			}
			else if (line.contains(MS_subdirs)) // this line contains a sub directory of the measurement set directory structure, skip it
			{ }
			else if (nodeID && line[0].isDigit()) {
				strlist = line.split(whitespsplit);
				f.fileSize = strlist.at(0).toLongLong();
                if (strlist.size() < 1) {
                    std::cerr << "problem reading line: "<< line.toStdString() << std::endl;
                    break;
                }
                else {
                    pos = strlist.at(1).lastIndexOf('/') +1;
                    f.path = strlist.at(1).left(pos);
                    f.fileName = strlist.at(1).mid(pos);
                }
				// try to guess SAS ID
				pos = f.path.indexOf(SAS_ID);
				if (pos != -1) {
					f.sasID = f.path.mid(pos+1, SAS_ID.matchedLength()-1).toInt();
				}
				else {
					pos = f.fileName.indexOf(SAS_ID);
					f.sasID = f.fileName.mid(pos+1, SAS_ID.matchedLength()-1).toInt();
				}
				updateFileInfo(f, nodeName);
				totalNodeSpace += f.fileSize;
			}
		}
		parseFile.close();
		msgBox.close();
        // also log the used node space for the last node (does not get triggered by a new '-------- locus101' line )
        if (nodeID) {
            if (itsUsedNodesSpace.find(nodeID) == itsUsedNodesSpace.end()) {
                itsUsedNodesSpace[nodeID] = totalNodeSpace;
            }
        }

		QApplication::restoreOverrideCursor();
	}
	else {
		std::cerr << "process did not exit normally" << std::endl;
	}
}

void CEPCleanMainWindow::writeFileInfo(const file &f, const QString &nodeName, std::map<dataProductTypes, DataProductInfo> &dataproducts) {
	if (f.fileName.endsWith(".MS")) { // correlated (e.g. _uv.MS and _uv.dppp.MS)
		DataProductInfo &dp(dataproducts[DP_CORRELATED_UV]);
		int idx = dp.fileNames.indexOf(f.fileName);
		if (idx != -1) {
			dp.exists[idx] = true;
			dp.fileSizes[idx] = f.fileSize;
			dp.nodes[idx] = nodeName;
		}
		else {
			dp.fileNames.append(f.fileName);
			dp.exists.push_back(true);
			dp.fileSizes.push_back(f.fileSize);
			dp.localPaths.push_back(f.path);
			dp.nodes.push_back(nodeName);
		}
	}
	else if (f.fileName.endsWith(".INST")) { // Instrument models
		DataProductInfo &dp(dataproducts[DP_INSTRUMENT_MODEL]);
		int idx = dp.fileNames.indexOf(f.fileName);
		if (idx != -1) {
			dp.exists.at(idx) = true;
			dp.fileSizes.at(idx) = f.fileSize;
			dp.nodes[idx] = nodeName;
		}
		else {
			dp.fileNames.append(f.fileName);
			dp.exists.push_back(true);
			dp.fileSizes.push_back(f.fileSize);
			dp.localPaths.push_back(f.path);
			dp.nodes.push_back(nodeName);
		}
	}
    else if (f.fileName.endsWith("_bf.tar.gz")) { // Pulsar data product
        DataProductInfo &dp(dataproducts[DP_PULSAR]);
        int idx = dp.fileNames.indexOf(f.fileName);
        if (idx != -1) {
            dp.exists.at(idx) = true;
            dp.fileSizes.at(idx) = f.fileSize;
            dp.nodes[idx] = nodeName;
        }
        else {
            dp.fileNames.append(f.fileName);
            dp.exists.push_back(true);
            dp.fileSizes.push_back(f.fileSize);
            dp.localPaths.push_back(f.path);
            dp.nodes.push_back(nodeName);
        }
    }
    /*
    else if (f.fileName.endsWith("_bf.h5") || f.fileName.endsWith("_bf.raw")) { // Coherent Stokes / Incoherent Stokes (new naming convention for incoherent is the same as for coherent stokes)
		bool foundFile(false);
		std::map<dataProductTypes, DataProductInfo>::iterator ddit(dataproducts.find(DP_COHERENT_STOKES));
		if (ddit != dataproducts.end()) { // found data product type?
            int idx = ddit->second.fileNames.indexOf(f.fileName);
            if (idx != -1) {
				foundFile = true;
				ddit->second.exists.at(idx) = true;
				ddit->second.fileSizes.at(idx) = f.fileSize;
				ddit->second.nodes[idx] = nodeName;
			}
		}
        if (!foundFile) {
            ddit = dataproducts.find(DP_INCOHERENT_STOKES);
            if (ddit != dataproducts.end()) { // found data product type?
                int idx = ddit->second.fileNames.indexOf(f.fileName);
                if (idx != -1) {
                    foundFile = true;
                    ddit->second.exists.at(idx) = true;
                    ddit->second.fileSizes.at(idx) = f.fileSize;
                    ddit->second.nodes[idx] = nodeName;
                }
            }
        }
        if (!foundFile) {
            DataProductInfo &dp = dataproducts[DP_UNKNOWN_TYPE]; // creates if if it doesn't exist yet
            int idx = dp.fileNames.indexOf(f.fileName);
            if (idx != -1) { // file found, update the file info
                dp.exists[idx] = true;
                dp.fileSizes[idx] = f.fileSize;
                dp.localPaths[idx] = f.path;
                dp.nodes[idx] = nodeName;
            }
            else { // unknown file was not cataloged before, append the file
                dp.fileNames.push_back(f.fileName);
                dp.fileSizes.push_back(f.fileSize);
                dp.localPaths.push_back(f.path);
                dp.exists.push_back(true);
                dp.nodes.push_back(nodeName);
            }
        }
	}
    */
    else if (f.fileName.endsWith("_bf.h5") || f.fileName.endsWith("_bf.raw")) { // coherent or incoherent h5 file. It is not an official data produkt so don't search for the full filename but instead search the base-name
        bool foundFile(false);
        QString findFile(f.fileName);
        findFile = findFile.remove("_bf.h5");
        findFile = findFile.remove("_bf.raw");
        std::map<dataProductTypes, DataProductInfo>::iterator ddit(dataproducts.find(DP_COHERENT_STOKES));
        if (ddit != dataproducts.end()) { // found data product type?
            QStringList::iterator sit = ddit->second.fileNames.begin();
            while (sit < ddit->second.fileNames.end()) {
                if (sit->startsWith(findFile)) { // found the base name without the extension
                    int i = ddit->second.fileNames.indexOf(f.fileName); // search for the full base name incl extension
                    if (i == -1) { // the file was not yet added, add it now
                        foundFile = true;
                        ddit->second.fileNames.append(f.fileName);
                        ddit->second.exists.push_back(true);
                        ddit->second.fileSizes.push_back(f.fileSize);
                        ddit->second.localPaths.push_back(f.path);
                        ddit->second.nodes.push_back(nodeName);
                        break;
                    }
                    else { // h5 file was already added, don't add it again just update
                        DataProductInfo &dp = dataproducts[DP_COHERENT_STOKES];
                        dp.exists[i] = true;
                        dp.fileSizes[i] = f.fileSize;
                        dp.localPaths[i] = f.path;
                        dp.nodes[i] = nodeName;
                        foundFile = true;
                        break;
                    }
                }
                ++sit;
            }
        }
        if (!foundFile) {
            ddit = dataproducts.find(DP_INCOHERENT_STOKES);
            if (ddit != dataproducts.end()) { // found data product type?
                QStringList::iterator sit = ddit->second.fileNames.begin();
                while (sit < ddit->second.fileNames.end()) {
                    if (sit->startsWith(findFile)) {
                        int i = ddit->second.fileNames.indexOf(f.fileName);
                        if (i == -1) { // the h5 file was not yet added, add it now
                            foundFile = true;
                            ddit->second.fileNames.append(f.fileName);
                            ddit->second.exists.push_back(true);
                            ddit->second.fileSizes.push_back(f.fileSize);
                            ddit->second.localPaths.push_back(f.path);
                            ddit->second.nodes.push_back(nodeName);
                            break;
                        }
                        else { // file was already added, don't add it again
                            DataProductInfo &dp = dataproducts[DP_INCOHERENT_STOKES];
                            dp.exists[i] = true;
                            dp.fileSizes[i] = f.fileSize;
                            dp.localPaths[i] = f.path;
                            dp.nodes[i] = nodeName;
                            foundFile = true;
                            break;
                        }
                    }
                    ++sit;
                }
            }
        }
        if (!foundFile) { // file not found in both coherent and incoherent dataprodukt filenames arrays, add it to unknown type files
            DataProductInfo &dp = dataproducts[DP_UNKNOWN_TYPE]; // creates if if it doesn't exist yet
            int idx = dp.fileNames.indexOf(f.fileName);
            if (idx != -1) { // file found, update the file info
                dp.exists[idx] = true;
                dp.fileSizes[idx] = f.fileSize;
                dp.localPaths[idx] = f.path;
                dp.nodes[idx] = nodeName;
            }
            else { // unknown file was not cataloged before, append the file
                dp.fileNames.push_back(f.fileName);
                dp.fileSizes.push_back(f.fileSize);
                dp.localPaths.push_back(f.path);
                dp.exists.push_back(true);
                dp.nodes.push_back(nodeName);
            }
        }
    }
	else if (f.fileName.endsWith("_sky.h5")) { // Sky Images in hdf5 format
		DataProductInfo &dp(dataproducts[DP_SKY_IMAGE]);
		int idx = dp.fileNames.indexOf(f.fileName);
		if (idx != -1) {
			dp.exists.at(idx) = true;
			dp.fileSizes.at(idx) = f.fileSize;
			dp.nodes[idx] = nodeName;
		}
		else {
			dp.fileNames.append(f.fileName);
			dp.exists.push_back(true);
			dp.fileSizes.push_back(f.fileSize);
			dp.localPaths.push_back(f.path);
			dp.nodes.push_back(nodeName);
		}
	}
	else if (f.fileName.endsWith("_bf.incoherentstokes")) { // incoherent Stokes
		DataProductInfo &dp(dataproducts[DP_INCOHERENT_STOKES]);
		int idx = dp.fileNames.indexOf(f.fileName);
		if (idx != -1) {
			dp.exists.at(idx) = true;
			dp.fileSizes.at(idx) = f.fileSize;
			dp.nodes[idx] = nodeName;
		}
		else {
			dp.fileNames.append(f.fileName);
			dp.exists.push_back(true);
			dp.fileSizes.push_back(f.fileSize);
			dp.localPaths.push_back(f.path);
			dp.nodes.push_back(nodeName);
		}
	}
	else { // adds the files with unknown file type extension
		DataProductInfo &dp = dataproducts[DP_UNKNOWN_TYPE]; // creates if if it doesn't exist yet
		int idx = dp.fileNames.indexOf(f.fileName);
		if (idx != -1) { // file found, update the file info
			dp.exists[idx] = true;
			dp.fileSizes[idx] = f.fileSize;
			dp.localPaths[idx] = f.path;
			dp.nodes[idx] = nodeName;
		}
		else { // unknown file was not cataloged before, append the file
			dp.fileNames.push_back(f.fileName);
			dp.fileSizes.push_back(f.fileSize);
			dp.localPaths.push_back(f.path);
			dp.exists.push_back(true);
			dp.nodes.push_back(nodeName);
		}
	}
}

void CEPCleanMainWindow::updateFileInfo(const file &fileInfo, const QString &nodeName) {
	if (!fileInfo.fileName.isEmpty()) {
		if (fileInfo.sasID) {
			for (std::map<QString, ProjectDataProducts>::iterator it = itsProjectsData.begin(); it != itsProjectsData.end(); ++ it) {
				std::map<unsigned, TaskDataProducts> &taskDataProducts(it->second.taskDataProductsForChange());
				std::map<unsigned, TaskDataProducts>::iterator fit(taskDataProducts.find(fileInfo.sasID));
				if (fit != taskDataProducts.end()) {
					std::map<dataProductTypes, DataProductInfo> &dataproducts(fit->second.dataProductsForChange());
					writeFileInfo(fileInfo, nodeName, dataproducts);
					return;
				}
			}
			// TODO: check the following regexp that it does not include any LOFAR_PULSAR_ARCHIVE data in the Unidentified (Lxxxxx)
			// eg: locus093: rm -rf /data/L54025 /data/LOFAR_PULSAR_ARCHIVE_locus093/L54025_CVplots
			if (fileInfo.path.contains(QRegExp("/data/L[0-9]{5,}"))) {
                std::map<unsigned, TaskDataProducts> &taskDataProducts(itsProjectsData["Unidentified (in /data/Lxxxxx)"].taskDataProductsForChange());
				taskDataProducts[fileInfo.sasID].setTaskName(QString::number(fileInfo.sasID));
				writeFileInfo(fileInfo, nodeName, taskDataProducts[fileInfo.sasID].dataProductsForChange());
			}
			else {
                std::map<unsigned, TaskDataProducts> &taskDataProducts(itsProjectsData["Unidentified (in other directories)"].taskDataProductsForChange());
				taskDataProducts[fileInfo.sasID].setTaskName(QString::number(fileInfo.sasID));
				writeFileInfo(fileInfo, nodeName, taskDataProducts[fileInfo.sasID].dataProductsForChange());
			}
		}
	}
}

void CEPCleanMainWindow::checkItemChange(QTreeWidgetItem *item, int column) {
	ui.treeWidgetDataProducts->blockSignals(true);
	disconnect(ui.treeWidgetDataProducts,SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(checkItemChange(QTreeWidgetItem *, int)));

	if (column == 0) { // only the first column has check boxes
		// The isExpanded trick is to speed up selection of a complete tree which was incredibly slow
		// but if you collapse the tree first then do the selection and then expand it to the previous levels
		// the selection is instantaneous!
		bool isexpand(item->isExpanded());

		if (isexpand)
			item->setExpanded(false);

		int itemType(item->data(0,ITEM_TYPE_ROLE).toInt());
		Qt::CheckState checkState(item->checkState(0));

		// place checkmarks downwards
		if (itemType == TASK_ITEM) {
			// if one of the item's childs was already checked then the parent should now be unchecked
			for (int i = 0; i < item->childCount(); ++i) {
				if (item->child(i)->checkState(0) == Qt::Checked) {
					item->setCheckState(0, Qt::Unchecked);
					checkState = Qt::Unchecked;
				}
			}
			bool partial(false);
			for (int i = 0; i < item->childCount(); ++i) {
				QTreeWidgetItem *childTask(item->child(i));
				if (!childTask->isHidden()) {
					childTask->setCheckState(0,checkState);
				}
				else partial = true;
			}
			if (checkState == Qt::Checked) {
				if (partial) {
					item->setCheckState(0, Qt::PartiallyChecked);
					item->setCheckState(1, Qt::Unchecked);
				}
			}
			else { // also disable the VIC tree deletion if not all the task's data is selected for deletion
				item->setCheckState(1, Qt::Unchecked);
			}
		}
		else if (itemType == PROJECT_ITEM) {
			for (int i = 0; i < item->childCount(); ++i) {
				QTreeWidgetItem *childTask(item->child(i));
				if (!childTask->isHidden()) {
					childTask->setCheckState(0,checkState);
					for (int ii = 0; ii < childTask->childCount(); ++ii) {
						childTask->child(ii)->setCheckState(0,checkState);
					}
				}
			}
		}
		// place checkmarks upwards
		if (itemType == TASK_ITEM) {
			if (checkState == Qt::Checked) {
				bool partiallyChecked(false);
				QTreeWidgetItem *parentItem(item->parent());
				for (int i = 0; i < parentItem->childCount(); ++i) {
					if (parentItem->child(i)->checkState(0) == Qt::Unchecked) {
						partiallyChecked = true;
						break;
					}
				}
				// now set the checkstate of the parent item
				if (partiallyChecked) {
					parentItem->setCheckState(0,Qt::PartiallyChecked);
				}
				else {
					parentItem->setCheckState(0,Qt::Checked);
				}
			}
			else { // user unchecked the task item
				item->setCheckState(1,Qt::Unchecked); // uncheck vic tree deletion check mark because not all data of task is selected for deletion
				QTreeWidgetItem *parentItem(item->parent());
				bool hasCheckedChilds(false);
				for (int i = 0; i < parentItem->childCount(); ++i) {
					if (parentItem->child(i)->checkState(0) == Qt::Checked) {
						hasCheckedChilds = true;
						break;
					}
				}
				if (hasCheckedChilds) {
					parentItem->setCheckState(0,Qt::PartiallyChecked);
				}
				else {
					parentItem->setCheckState(0,Qt::Unchecked);
				}
			}
		}
		else if (itemType == DATA_PRODUCT_ITEM) {
			if (checkState == Qt::Checked) {
				bool partiallyChecked(false);
				QTreeWidgetItem *parentTaskItem(item->parent());
				for (int i = 0; i < parentTaskItem->childCount(); ++i) {
					if (parentTaskItem->child(i)->checkState(0) == Qt::Unchecked) {
						partiallyChecked = true;
						break;
					}
				}
				// now set the checkstate of the parent Task item
				if (partiallyChecked) {
					parentTaskItem->setCheckState(0,Qt::PartiallyChecked);
					parentTaskItem->parent()->setCheckState(0,Qt::PartiallyChecked); // also marks the project item as partially checked
				}
				else {
					parentTaskItem->setCheckState(0,Qt::Checked);
					// we have to check the project item if it needs partial or full checkmark
					partiallyChecked = false;
					QTreeWidgetItem *parentProjectItem(parentTaskItem->parent());
					for (int i = 0; i < parentProjectItem->childCount(); ++i) {
						if (parentProjectItem->child(i)->checkState(0) == Qt::Unchecked) {
							partiallyChecked = true;
							break;
						}
					}
					if (partiallyChecked) {
						parentProjectItem->setCheckState(0,Qt::PartiallyChecked);
					}
					else {
						parentProjectItem->setCheckState(0,Qt::Checked);
					}
				}
			}
			else { // user unchecked the data product item
				QTreeWidgetItem *parentTaskItem(item->parent());
				parentTaskItem->setCheckState(1,Qt::Unchecked); // uncheck vic tree deletion check mark because not all data of task is selected for deletion
				bool hasCheckedChilds(false);
				for (int i = 0; i < parentTaskItem->childCount(); ++i) {
					if (parentTaskItem->child(i)->checkState(0) != Qt::Unchecked) {
						hasCheckedChilds = true;
						break;
					}
				}
				if (hasCheckedChilds) {
					parentTaskItem->setCheckState(0,Qt::PartiallyChecked);
					parentTaskItem->parent()->setCheckState(0,Qt::PartiallyChecked); // also marks the project item as partially checked
				}
				else {
					parentTaskItem->setCheckState(0,Qt::Unchecked);
					// we have to check the project item if it needs partial or no checkmark
					bool partiallyChecked(false);
					QTreeWidgetItem *parentProjectItem(parentTaskItem->parent());
					for (int i = 0; i < parentProjectItem->childCount(); ++i) {
						if (parentProjectItem->child(i)->checkState(0) != Qt::Unchecked) {
							partiallyChecked = true;
							break;
						}
					}
					if (partiallyChecked) {
						parentProjectItem->setCheckState(0,Qt::PartiallyChecked);
					}
					else {
						parentProjectItem->setCheckState(0,Qt::Unchecked);
					}
				}
			}
		}

		if (isexpand)
			item->setExpanded(true);
	}
	else if (column == 1) {
		if (item->checkState(0) != Qt::Checked) { // also disable the VIC tree deletion if not all the task's data is selected for deletion
			item->setCheckState(1,Qt::Unchecked);
		}
	}
	connect(ui.treeWidgetDataProducts,SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(checkItemChange(QTreeWidgetItem *, int)));

	ui.treeWidgetDataProducts->blockSignals(false);
}

void CEPCleanMainWindow::createStorageNodesTab(int rows, int colums) {
    QFont boldFont;
    boldFont.setBold(true);
    boldFont.setWeight(75);

	QGridLayout *grid = new QGridLayout(ui.tabNodes);

	for (int col = 0; col < colums; ++col) {
		// create header labels
		QLabel *label_free = new QLabel(ui.tabNodes);
		label_free->setText("Free:");
		label_free->setFont(boldFont);
		label_free->setAlignment(Qt::AlignCenter);
		grid->addWidget(label_free, 0, col*6 + 1, 1, 1);
		QLabel *label_full = new QLabel(ui.tabNodes);
		label_full->setText("% Used:");
		label_full->setFont(boldFont);
		label_full->setAlignment(Qt::AlignCenter);
		grid->addWidget(label_full, 0, col*6 + 2, 1, 1);
		QLabel *label_unknown = new QLabel(ui.tabNodes);
		label_unknown->setText("Unknown data:");
		label_unknown->setFont(boldFont);
		label_unknown->setAlignment(Qt::AlignCenter);
		grid->addWidget(label_unknown, 0, col*6 + 3, 1, 2);
		if (colums > 1) {
			QFrame *line = new QFrame(ui.tabNodes);
	        line->setFrameShape(QFrame::VLine);
	        line->setFrameShadow(QFrame::Sunken);
	        grid->addWidget(line, 0, col*6+5, rows+1, 1);
		}
		for (int row = 0; row < rows; ++row) {
			QLabel *label_locus = new QLabel(ui.tabNodes);
			label_locus->setText("locusxxx");
			itsNodes[row + col*rows] = label_locus;
	        grid->addWidget(label_locus, row+1, col*6, 1, 1);
			QLabel *label_free_space = new QLabel(ui.tabNodes);
			label_free_space->setText("0kB");
	        grid->addWidget(label_free_space, row+1, col*6+1, 1, 1);
	        itsFreeSpaceLabels[row + col*rows] = label_free_space;
	        // used space progress bars
	        QProgressBar * progressBarUsedSpace = new QProgressBar(ui.tabNodes);
	        progressBarUsedSpace->setInvertedAppearance(true);
	        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	        sizePolicy.setHeightForWidth(progressBarUsedSpace->sizePolicy().hasHeightForWidth());
	        progressBarUsedSpace->setSizePolicy(sizePolicy);
	        progressBarUsedSpace->setMinimumSize(QSize(0, 0));
	        progressBarUsedSpace->setMaximumSize(QSize(16777215, 21));
	        grid->addWidget(progressBarUsedSpace, row+1, col*6+2, 1, 1);
	        itsUsedSpaceBars[row + col*rows] = progressBarUsedSpace;
	        // unknown data progress bars
            QProgressBar * progressBarUnknown = new QProgressBar(ui.tabNodes);
            progressBarUnknown->setSizePolicy(sizePolicy);
            progressBarUnknown->setMinimumSize(QSize(0, 0));
            progressBarUnknown->setMaximumSize(QSize(16777215, 21));
            grid->addWidget(progressBarUnknown, row+1, col*6+4, 1, 1);
            itsUnknownSpaceBars[row + col*rows] = progressBarUnknown;
		}
	}
	QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    grid->addItem(verticalSpacer, rows+2, 0, 1, 1);
	// add refresh button
	itsRefreshStorageUsageButton = new QPushButton("Refresh");
	connect(itsRefreshStorageUsageButton, SIGNAL(clicked()), this, SLOT(updateStorageNodesTab(void)));
	grid->addWidget(itsRefreshStorageUsageButton, rows+1, 0, 1, 2);
}

void CEPCleanMainWindow::setNodesSpace(unsigned idx, const quint64 &totalSize, const quint64 &usedSize) {
	int percentage(0);
	if (totalSize > 0) {
		percentage = static_cast<int>(round((double)usedSize / totalSize * 100));
	}
	setNodePercentageUsed(idx, percentage);
	itsFreeSpaceLabels[idx]->setText(humanReadableUnits(totalSize - usedSize).c_str());
}

void CEPCleanMainWindow::setNodePercentageUsed(unsigned idx, int percentage) {
	QProgressBar *bar = itsUsedSpaceBars[idx];
	bar->setValue(percentage);
	QString style("QProgressBar {border: 1px solid grey; border-radius: 3px; background-color: lightgreen;}");
	QString chunkRed("QProgressBar::chunk {background-color: red;}");
	QString chunkOrange("QProgressBar::chunk {background-color: orange;}");
	QString chunkBlue("QProgressBar::chunk {background-color: lightblue;}");
	if (percentage > 80) {
		style += chunkRed;
	}
	else if (percentage > 60) {
		 style += chunkOrange;
	}
	else {
		style += chunkBlue;
	}
	bar->setStyleSheet(style);
	bar->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

void CEPCleanMainWindow::setNodePercentageUnknown(unsigned idx, int percentage) {
	QProgressBar *bar = itsUnknownSpaceBars[idx];
	bar->setValue(percentage);
	QString style("QProgressBar {border: 1px solid grey; border-radius: 3px; background-color: lightgray;}");
	QString chunkRed("QProgressBar::chunk {background-color: red;}");
//	QString chunkOrange("QProgressBar::chunk {background-color: orange;}");
//	QString chunkBlue("QProgressBar::chunk {background-color: lightblue;}");
	if (percentage > 15) {
		style += chunkRed;
	}
//	else if (percentage > 60) {
//		 style += chunkOrange;
//	}
//	else {
//		style += chunkBlue;
//	}
	bar->setStyleSheet(style);
	bar->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

void CEPCleanMainWindow::updateStorageNodesTab(void) {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	itsRefreshStorageUsageButton->setEnabled(false);
	itsRefreshStorageUsageButton->setText("Updating...");
	itsController->connectToDataMonitor();
	const storageHostsMap &nodes = Controller::theSchedulerSettings.getStorageNodes();
	const hostPartitionsMap &hostPartitions = Controller::theSchedulerSettings.getStoragePartitions();
	const statesMap &states = Controller::theSchedulerSettings.getStorageNodesStates();
	QString hostName;
	statesMap::const_iterator stateIt;
	storageHostsMap::const_iterator hit = nodes.begin();
	hostPartitionsMap::const_iterator pit;
	usedNodeSpaceMap::const_iterator uit;
	for (unsigned idx = 0; idx < nodes.size(); ++idx) {
		hostName = hit->second.itsName.c_str();
		QLabel *nodeLabel(itsNodes[idx]);
		nodeLabel->setText(hostName);
		if (hit->second.itsStatus == 1) {
			nodeLabel->setStyleSheet("QLabel { color:black; font:bold;}");
		}
		else {
			nodeLabel->setStyleSheet("QLabel { background-color:red; color:black; font:bold; border: 2px solid grey; border-radius: 3px; }");
		}
		stateIt = states.find(hit->second.itsStatus);
		if (stateIt != states.end()) {
			nodeLabel->setToolTip(stateIt->second.c_str());
		}

		pit = hostPartitions.find(hit->second.itsID);
		if (pit != hostPartitions.end()) {
			quint64 usedTotal(0), diskSizeTotal(0);
			for (dataPathsMap::const_iterator dpit = pit->second.begin(); dpit != pit->second.end(); ++dpit) { // iterate over all partitions for this host
				setNodesSpace(idx, dpit->second.second[0], dpit->second.second[1]);
				usedTotal += dpit->second.second[1];
				diskSizeTotal += dpit->second.second[0];
			}
			uit = itsUsedNodesSpace.find(hit->second.itsID);
			if (uit != itsUsedNodesSpace.end()) {
				double perc(((double)usedTotal - uit->second) / diskSizeTotal * 100);
				setNodePercentageUnknown(idx, (int)round(perc));
			}
		}
		++hit;
	}

	itsRefreshStorageUsageButton->setEnabled(true);
	itsRefreshStorageUsageButton->setText("Refresh");
	QApplication::restoreOverrideCursor();
}
