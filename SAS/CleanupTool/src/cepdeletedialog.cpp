/*
 * cepdeletedialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11934 $
 * Last change by : $Author: mol $
 * Change date	  : $Date: 2014-10-27 12:54:46 +0000 (Mon, 27 Oct 2014) $
 * First creation : Jan 5, 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/cepdeletedialog.cpp $
 *
 */

#include "cepdeletedialog.h"
#include <QProcess>
#include <QMessageBox>
#include <QProgressDialog>
#include "Controller.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <unistd.h>

CEPdeleteDialog::CEPdeleteDialog(QWidget *parent)
    : QDialog(parent), itsRow(0), itsExitCode(0), itsRetryCount(0), fp(0), itsState(NOT_STARTED),
      itsParentCleanupDialog(static_cast<CEPCleanMainWindow *>(parent))
{
	ui.setupUi(this);

	QStringList header;
	header << "project" << "task" << "sas ID" << "run date" << "duration" << "which data" << "# files" << "# nodes" << "size";
	ui.tableWidgetDelete->setColumnCount(header.size());
	ui.tableWidgetDelete->setHorizontalHeaderLabels(header);
	this->setWindowFlags(Qt::Window);
	this->setWindowTitle("Confirm delete");
	connect(ui.pushButtonDelete, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	ui.progressBar->setValue(0);

	#if defined Q_OS_WIN
	itsConnectCmd = "C:\\progra~1\\putty\\plink -agent lofarsys@";
#elif defined Q_OS_UNIX
	itsConnectCmd = "ssh lofarsys@lhn001.cep2.lofar ssh ";
#else
	std::cerr << "ERROR: Unknown operating system. Don't know how to connect with storage nodes" << std::endl;
	return;
#endif

}

CEPdeleteDialog::~CEPdeleteDialog()
{
	for (std::map<QString, std::pair<runState, QProcess *> >::iterator pit = itsCleanProcesses.begin(); pit != itsCleanProcesses.end(); ++pit) {
		delete pit->second.second;
	}
}

void CEPdeleteDialog::setNodesCommandsInfo(const std::map<QString, QString> &nci) {
	itsNodeCommands = nci;
	QString cmd;
	for (std::map<QString, QString>::const_iterator it = nci.begin(); it != nci.end(); ++it) {
		ui.textEditDelete->append(it->first + ": " + it->second);
	}
}


void CEPdeleteDialog::setVICtreesToDelete(const deleteVICmap &vics) {
	itsVICtreesToDelete = vics;
	QStringList vicString;
	for (deleteVICmap::const_iterator it = vics.begin(); it != vics.end(); ++it) {
		vicString.append(it->second.join(","));
	}
	ui.lineEditVICtrees->setText(vicString.join(","));
}

void CEPdeleteDialog::addTask(const QStringList &info) {
	if (info.size() == ui.tableWidgetDelete->columnCount()) {
		ui.tableWidgetDelete->setRowCount(itsRow+1);
		for (int i = 0; i < info.size(); ++i) {
			ui.tableWidgetDelete->setItem(itsRow, i, new QTableWidgetItem(info.at(i)));
		}
		ui.tableWidgetDelete->resizeColumnsToContents();
	}
	++itsRow;
}

void CEPdeleteDialog::cancelClicked(void) {
	if (itsState == SUCCESS) {
		done(0);
	}
	else if ((itsState == RUNNING) || (itsState == RETRYING)) {
	}
	else done(3); // cancel clicked before clean was started or clean did not fully work, return with status 3
}

void CEPdeleteDialog::okClicked(void) {
	if (ui.pushButtonDelete->text() == "Ok") {
		done(itsExitCode);
	}
	else deleteConfirmed();
}

void CEPdeleteDialog::nodeCleanFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/) {
	for (std::map<QString, std::pair<runState, QProcess *> >::iterator pit = itsCleanProcesses.begin(); pit != itsCleanProcesses.end(); ++pit) {
		if (pit->second.first == RUNNING) { // marked as running?
			if (pit->second.second->state() == QProcess::NotRunning) { // process finished?
				if (pit->second.second->exitCode() == 0 && pit->second.second->exitStatus() == QProcess::NormalExit) {
					ui.textEditDelete->setTextColor(Qt::black);
					ui.textEditDelete->append(pit->first + " clean ok");
					itsParentCleanupDialog->writeLog(pit->first + " clean ok");
					pit->second.first = SUCCESS;
				}
				else {
					ui.textEditDelete->setTextColor(Qt::red);
					QString errStr("WARNING could not clean " + pit->first + " (exitcode:" + QString::number(pit->second.second->exitCode()) + ", exitstate:" + QString::number(pit->second.second->exitStatus()) + ")");
					itsParentCleanupDialog->writeLog(errStr);
					ui.textEditDelete->append(errStr);
					std::cerr << "ERROR cleaning " << pit->first.toStdString() << ", exitcode:" << pit->second.second->exitCode()
							<< ", exitStatus:" << pit->second.second->exitStatus() << std::endl;
					pit->second.first = FAILED;
				}
				break;
			}
		}
	}

	// check if still running or a retry is needed or finish
	unsigned count(0);
	for (std::map<QString, std::pair<runState, QProcess *> >::iterator pit = itsCleanProcesses.begin(); pit != itsCleanProcesses.end(); ++pit) {
		if (pit->second.first == RUNNING) {
			itsState = RUNNING;
			break;
		}
		else if (++count >= itsCleanProcesses.size()) { // reached last process and did not find a running process
			// check if there are failed processes
			count = 0;
			for (std::map<QString, std::pair<runState, QProcess *> >::iterator pit = itsCleanProcesses.begin(); pit != itsCleanProcesses.end(); ++pit) {
				if (pit->second.first == FAILED) { // still have failed processes?
					if (++itsRetryCount >= 3) { // already retried 3 times?
						itsState = FAILED;
					}
					else {
						itsState = RETRYING;
					}
					break;
				}
				++count;
			}
			if (count >= itsCleanProcesses.size()) {
				if ((itsState == RUNNING) || (itsState == RETRYING)) { // not retrying anymore and checked that there are no FAILED processes
//					itsCleanIsRunning = false;
					itsState = SUCCESS;
				}
			}
		}
	}


	if (itsState == RETRYING) {
		ui.textEditDelete->setTextColor(Qt::black);
		ui.textEditDelete->append("retrying cleaning of failed nodes in 3 seconds");
#ifdef Q_OS_WIN
Sleep(3000);
#else
sleep(3);
#endif
		retryDelete();
	}

	else if ((itsState == SUCCESS) || (itsState == FAILED)) {
		QApplication::restoreOverrideCursor();
		cleanFinished(); // nodes cleaning finished, now clean database if necessary
		// we're done
		ui.pushButtonCancel->setText("Close");
	}
}

void CEPdeleteDialog::deleteConfirmed(void) {
	ui.progressBar->setValue(0);
	ui.pushButtonDelete->setEnabled(false);
	if (QMessageBox::question(0, tr("Final confirm delete"),
			tr("Are you sure you want to start the delete?"),
			QMessageBox::Ok,
			QMessageBox::Cancel) == QMessageBox::Ok) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		ui.textEditDelete->append("Now starting cleaning process...");
		itsParentCleanupDialog->writeLog("Now starting cleaning process...");
		ui.pushButtonCancel->setEnabled(false);
		QApplication::processEvents();
		// execute commands on storage nodes
		itsRetryCount = 0;
		QString cmd;
		if (!itsNodeCommands.empty()) { // if there is data marked for deletion
			itsState = RUNNING;
			for (std::map<QString, QString>::const_iterator nodeIt = itsNodeCommands.begin(); nodeIt != itsNodeCommands.end(); ++nodeIt) {
				QProcess *fp = new QProcess(this);
				itsCleanProcesses[nodeIt->first] = std::pair<runState, QProcess *>(RUNNING, fp);
				connect(fp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nodeCleanFinished(int, QProcess::ExitStatus)));
				cmd = itsConnectCmd + nodeIt->first + ".cep2.lofar " + nodeIt->second;
				itsParentCleanupDialog->writeLog("executing:" + cmd);
				fp->start(cmd);
			}
		}
		else if (!itsVICtreesToDelete.empty()) {
			if (!deleteVicTrees()) {
				done(2); // deletion of vic trees could not be completed
			}
		}
	}
	else { // user cancel
		done(3);
		return;
	}
	done(0);
}

void CEPdeleteDialog::retryDelete(void) {
	std::map<QString, std::pair<runState, QProcess *> > retryCleanProcesses;
	std::map<QString, std::pair<runState, QProcess *> >::const_iterator cleanIt;
	QString cmd;
	itsParentCleanupDialog->writeLog("Retrying cleaning of failed nodes, retry count=" + QString::number(itsRetryCount));
	for (std::map<QString, QString>::const_iterator nodeIt = itsNodeCommands.begin(); nodeIt != itsNodeCommands.end(); ++nodeIt) {
		cleanIt = itsCleanProcesses.find(nodeIt->first);
		if (cleanIt != itsCleanProcesses.end()) {
			if (cleanIt->second.first == FAILED) {
				QProcess *fp = new QProcess(this);
				retryCleanProcesses[nodeIt->first] = std::pair<runState, QProcess *>(RUNNING, fp);
				connect(fp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(nodeCleanFinished(int, QProcess::ExitStatus)));
				cmd = itsConnectCmd + nodeIt->first + ".cep2.lofar " + nodeIt->second;
				itsParentCleanupDialog->writeLog("retrying :" + cmd);
				fp->start(cmd);
			}
		}
	}
	// delete all old clean processes
	for (std::map<QString, std::pair<runState, QProcess *> >::iterator pit = itsCleanProcesses.begin(); pit != itsCleanProcesses.end(); ++pit) {
		pit->second.second->close();
//		delete pit->second.second;
	}

	itsCleanProcesses = retryCleanProcesses;
}

bool CEPdeleteDialog::isVICmarkedForDeletion(int treeID) const {
	QString vicidstr(QString::number(treeID));
	for (deleteVICmap::const_iterator it = itsVICtreesToDelete.begin(); it != itsVICtreesToDelete.end(); ++it) {
		if (it->second.contains(vicidstr)) return true;
	}
	return false;
}

bool CEPdeleteDialog::deleteVicTrees(void) {
	ui.textEditDelete->append("now deleting VIC trees in SAS database...");
	itsParentCleanupDialog->writeLog("VIC trees to be deleted in SAS database:");
	for (deleteVICmap::const_iterator it = itsVICtreesToDelete.begin(); it != itsVICtreesToDelete.end(); ++it) {
		itsParentCleanupDialog->writeLog(it->first + ":" + it->second.join(","));
	}
	if (itsParentCleanupDialog->deleteTreesCleanup(itsVICtreesToDelete)) {
		itsParentCleanupDialog->writeLog("VIC trees successfully deleted");
		return true;
	}
	else {
		ui.textEditDelete->setTextColor(Qt::red);
		itsParentCleanupDialog->writeLog("ERROR:Could not delete the VIC trees in the sas database!");
		ui.textEditDelete->append("Could not delete the VIC trees in the sas database!");
		ui.textEditDelete->setTextColor(Qt::black);
		return false;
	}
}


void CEPdeleteDialog::cleanFinished(void) {
	ui.pushButtonDelete->setText("Ok");
	ui.pushButtonDelete->setEnabled(true);
	if (itsState == SUCCESS) {
		ui.textEditDelete->setTextColor(Qt::black);
		ui.textEditDelete->append("Data deleted successfully");
		itsParentCleanupDialog->writeLog("Data deleted successfully");
			// mark data products as deleted in the VIC trees in the SAS database
			deletedDataMap finalTreesToMark;
			std::vector<std::pair<int, dataProductTypes> > tmpVec;
			for (deletedDataMap::const_iterator mit = itsTreesToMark.begin(); mit != itsTreesToMark.end(); ++mit) {
				for (std::vector<std::pair<int, dataProductTypes> >::const_iterator treeit = mit->second.begin(); treeit != mit->second.end(); ++treeit) {
					if (!isVICmarkedForDeletion(treeit->first)) {
						tmpVec.push_back(*treeit);
					}
				}
				finalTreesToMark.insert(deletedDataMap::value_type(mit->first, tmpVec));
				tmpVec.clear();
			}

			if (!finalTreesToMark.empty()) {
				ui.textEditDelete->append("now marking data deleted in SAS database...");
				itsParentCleanupDialog->writeLog("now marking data deleted in SAS database...");
				if (!itsParentCleanupDialog->markDataProductsDeleted(finalTreesToMark)) {
					ui.textEditDelete->setTextColor(Qt::red);
					itsParentCleanupDialog->writeLog("Could not mark data products as deleted in the sas database!");
					ui.textEditDelete->append("Could not mark data products as deleted in the sas database!");
					itsParentCleanupDialog->writeLog("WARNING: Could not mark data products as deleted in the sas database!");
					ui.textEditDelete->setTextColor(Qt::black);
					debugWarn("s","could not connect to SAS database while trying to mark data products as deleted");
					itsExitCode = 2;
					return;
				}
			}

			// now delete vic trees if needed
			if (!itsVICtreesToDelete.empty()) {
				if (!deleteVicTrees()) {
					itsExitCode = 2;
					return;
				}
			}

		ui.textEditDelete->append("All done.");
		itsParentCleanupDialog->writeLog("Cleaning finished successfully");
		itsExitCode = 0;
		return;
	}
	else {
		itsParentCleanupDialog->writeLog("ERROR: Something went wrong trying to delete the data. Not all data could be deleted.");
		QMessageBox::warning(this, "Data deletion problem", "Something went wrong trying to delete the data.\nNot all data could be deleted.\nThe file sizes and existence info in the display will not be updated!");
		itsExitCode = 1;
	}
}
