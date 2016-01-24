/*
 * ShiftTasksDialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : sept-2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/shifttasksdialog.cpp $
 *
 */

#include "shifttasksdialog.h"
#include <QTime>
#include <QMessageBox>
#include <QDateTime>
#include <vector>
#include <map>
#include <string>
#include "astrotime.h"
#include "DigitalBeam.h"
#include "redistributetasksdialog.h"

ShiftTasksDialog::ShiftTasksDialog(QWidget *parent, Controller *controller)
    : QDialog(parent), itsController(controller), itsMoveType(MOVE_RIGHT)
{
	ui.setupUi(this);
	connect(ui.pushButtonShiftToNow, SIGNAL(clicked()), this, SLOT(calculateNow(void)));
	connect(ui.pushButtonCenterAtLST, SIGNAL(clicked()), this, SLOT(calculateLST(void)));
	connect(ui.radioButtonShiftLeft, SIGNAL(clicked()), this, SLOT(setLeftMoveType(void)));
	connect(ui.radioButtonShiftRight, SIGNAL(clicked()), this, SLOT(setRightMoveType(void)));
	connect(ui.pushButtonApplyAbsMove, SIGNAL(clicked()), this, SLOT(applyShift(void)));
	connect(ui.pushButtonApplyLSTnow, SIGNAL(clicked()), this, SLOT(applyPreview(void)));
	connect(ui.dateTimeEditNow, SIGNAL(dateChanged(const QDate &)), this, SLOT(doLSTCheck(void)));
	ui.tableWidgetTasks->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWidgetTasks, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showTableContextMenu(const QPoint &)));
	ui.treeWidgetAngles->setColumnCount(4);

	QStringList header;
	header << "SAS ID" << "task description" << "start time (UTC)" << "duration";
	ui.treeWidgetAngles->setHeaderLabels(header);
	ui.treeWidgetAngles->setSortingEnabled(true);

	header.clear();
	header << "SAS ID" << "task name" << "planned start (UTC)" << "planned end (UTC)" << "duration" << "task description" << "status" << "task type";
	ui.tableWidgetTasks->setColumnCount(header.size());
	ui.tableWidgetTasks->setHorizontalHeaderLabels(header);
	ui.tableWidgetTasks->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetTasks->setSelectionBehavior(QAbstractItemView::SelectRows);
	connect(ui.tableWidgetTasks->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(sortPreview(int, Qt::SortOrder)));

	loadTargets();
	loadTasks();

	itsRedistributeDialog = new redistributeTasksDialog(this);
	QTime step(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks().toQTime());
	itsRedistributeDialog->setTimeStep1(step);
	itsRedistributeDialog->setTimeStep2(step);
	sortPreview(2,Qt::AscendingOrder);
    // select all tasks that can be moved (i.e. have status below PRESCHEDULED
    ui.tableWidgetTasks->setSelectionMode(QTableView::MultiSelection);
    int nrRows(ui.tableWidgetTasks->rowCount());
    for (int row = 0; row < nrRows; ++row) {
        if (static_cast<Task::task_status>(ui.tableWidgetTasks->item(row, 6)->data(Qt::UserRole).toInt()) <= Task::PRESCHEDULED) {
            ui.tableWidgetTasks->selectRow(row);
        }
    }
    ui.tableWidgetTasks->setSelectionMode(QTableView::ExtendedSelection);

    exec();
}

ShiftTasksDialog::~ShiftTasksDialog()
{
}

// put rowB directly before rowA
void ShiftTasksDialog::insertTaskBefore(int rowA, int rowB) {
	std::vector<QTableWidgetItem *> rowItemsA, rowItemsB;
	for (int col = 0; col < ui.tableWidgetTasks->columnCount(); ++col) {
		rowItemsB.push_back(ui.tableWidgetTasks->takeItem(rowB, col));
	}
	// delete the row
	ui.tableWidgetTasks->removeRow(rowB);
	// insert a new row before rowA
	ui.tableWidgetTasks->insertRow(rowA);
	for (int col = 0; col < ui.tableWidgetTasks->columnCount(); ++col) {
		ui.tableWidgetTasks->setItem(rowA, col, rowItemsB.at(col));
	}
}

void ShiftTasksDialog::sortPreview(int col, Qt::SortOrder sortorder) {
	ui.tableWidgetTasks->blockSignals(true);
	ui.tableWidgetTasks->horizontalHeader()->blockSignals(true);
	// step1: sort normally
	ui.tableWidgetTasks->sortByColumn(col, sortorder);
	// step2: go through list and check that each item is below all its predecessors
	const Task *predTask(0);
	int rowIdx1(0);
	while (rowIdx1 < ui.tableWidgetTasks->rowCount()) {
		bool insertedTasks(false);
		// get the tasks predecessors if any
		const Task *pTask(itsController->getTask(ui.tableWidgetTasks->item(rowIdx1, 0)->data(Qt::UserRole).toInt()));
		// search for the last predecessor end time
		const IDvector &predecessors(pTask->getPredecessors());
		if (!predecessors.empty()) {
			for (IDvector::const_iterator prit = predecessors.begin(); prit != predecessors.end(); ++prit) {
				unsigned predID(0);
				if (prit->second != ID_SCHEDULER) {
					predTask = itsController->getTask(prit->second, prit->first);
					if (predTask) {
						predID = predTask->getID();
					}
				}
				else {
					predID = prit->second;
				}
				if (predID) {
					for (int rowIdx2 = rowIdx1+1; rowIdx2 < ui.tableWidgetTasks->rowCount(); ++rowIdx2) {
						if (predID == ui.tableWidgetTasks->item(rowIdx2, 0)->data(Qt::UserRole).toUInt()) {
							// insert this predecessor directly before current task
							insertTaskBefore(rowIdx1, rowIdx2);
							insertedTasks = true;
							break;
						}
					}
				}
			}
		}
		if (!insertedTasks) {
			++rowIdx1;
		}
	}

	ui.tableWidgetTasks->horizontalHeader()->setSortIndicator(col, sortorder);
	ui.tableWidgetTasks->horizontalHeader()->setSortIndicatorShown(true);
	ui.tableWidgetTasks->horizontalHeader()->blockSignals(false);
	ui.tableWidgetTasks->blockSignals(false);
}

void ShiftTasksDialog::showTableContextMenu(const QPoint &pos) {
	QMenu menu;
	if (!ui.tableWidgetTasks->selectedItems().isEmpty()) {
		QAction *action = menu.addAction("Select observations");
		action->setToolTip("select all observations in this list");
		connect(action, SIGNAL(triggered()), this, SLOT(selectObservations(void)));
		action = menu.addAction("Select all pipelines");
		action->setToolTip("select all pipelines in this list");
		connect(action, SIGNAL(triggered()), this, SLOT(selectAllPipelines(void)));
		action = menu.addAction("Select calibrator pipelines");
		action->setToolTip("select all calibrator pipelines in this list");
		connect(action, SIGNAL(triggered()), this, SLOT(selectCalibratorPipelines(void)));
		action = menu.addAction("Select target pipelines");
		action->setToolTip("select all target pipelines in this list");
		connect(action, SIGNAL(triggered()), this, SLOT(selectTargetPipelines(void)));
		action = menu.addAction("Select pre-processing pipelines");
		action->setToolTip("select all pre-processing pipelines in this list");
		connect(action, SIGNAL(triggered()), this, SLOT(selectPreProcessingPipelines(void)));
        action = menu.addAction("Select long-baseline pipelines");
        action->setToolTip("select all long-baseline pipelines in this list");
        connect(action, SIGNAL(triggered()), this, SLOT(selectLongBaselinePipelines(void)));
        action = menu.addAction("Select imaging pipelines");
		action->setToolTip("select all imaging pipelines in this list");
		connect(action, SIGNAL(triggered()), this, SLOT(selectImagingPipelines(void)));
        action = menu.addAction("Select pulsar pipelines");
        action->setToolTip("select all pulsar pipelines in this list");
        connect(action, SIGNAL(triggered()), this, SLOT(selectPulsarPipelines(void)));
        action = menu.addAction("Redistribute tasks");
		action->setToolTip("redistribute selected tasks given a start time and a time distance in the sequence such as shown here");
		connect(action, SIGNAL(triggered()), this, SLOT(redistributeSelectedTasks(void)));
		action = menu.addAction("Schedule directly after predecessor");
		action->setToolTip("schedule selected tasks directly after each task's last predecessor");
		connect(action, SIGNAL(triggered()), this, SLOT(scheduleAfterPredecessor(void)));
	}
	menu.exec(ui.tableWidgetTasks->viewport()->mapToGlobal(pos));
}

QList<int> ShiftTasksDialog::getSelectedRows(void) const {
	QList<int> rows, newRows;
	QList<QTableWidgetSelectionRange> ranges(ui.tableWidgetTasks->selectedRanges());

	foreach (QTableWidgetSelectionRange range, ranges) {
		int sr = range.bottomRow();
		int er = range.topRow();
		if (sr > er) {
			int tmp(er);
			er = sr;
			sr = tmp;
		}
		while (sr <= er) {
			rows.push_back(sr);
			++sr;
		}
	}
	qSort(rows);
	bool showWarning(false);
	const Task *pTask(0);
	for (QList<int>::iterator it = rows.begin(); it != rows.end(); ++it) {
		pTask = itsController->getTask(ui.tableWidgetTasks->item(*it, 0)->data(Qt::UserRole).toInt());
		if (pTask->getStatus() <= Task::PRESCHEDULED) {
			newRows.append(*it);
		}
		else {
			showWarning = true;
		}
	}

	if (showWarning) {
		ui.tableWidgetTasks->clearSelection();
		QTableView::SelectionMode prevMode(ui.tableWidgetTasks->selectionMode());
		ui.tableWidgetTasks->setSelectionMode(QTableView::MultiSelection);
		for (QList<int>::iterator it = newRows.begin(); it != newRows.end(); ++it) {
			ui.tableWidgetTasks->selectRow(*it);
		}
		QMessageBox::warning(0, tr("Some tasks could not be moved"),
				"Some tasks have a status above PRESCHEDULED. These tasks cannot not be moved and have been deselected.", tr("Close"));
		ui.tableWidgetTasks->setSelectionMode(prevMode);
	}

	return newRows;
}

void ShiftTasksDialog::selectTasks(int type) {
	ui.tableWidgetTasks->clearSelection();
	ui.tableWidgetTasks->setSelectionMode(QAbstractItemView::MultiSelection);
    if (type >= SEL_CALIBRATOR_PIPELINES && type <= SEL_PULSAR_PIPELINES) { // i.e. all pipeline types
		// pipelines cannot be moved to LST, so if the current move type is MOVE_TO_CENTER (= 'move to LST')
		// then change the move typ to MOVE_TO_START to update the redistribute dialog settings shown
		if (itsMoveType == MOVE_TO_CENTER) {
			itsMoveType = MOVE_TO_START;
		}
	}
	for (int row = 0; row < ui.tableWidgetTasks->rowCount(); ++row) {
		if (ui.tableWidgetTasks->item(row, 7)->data(Qt::UserRole).toInt() == type) {
			ui.tableWidgetTasks->selectRow(row);
		}
	}
	ui.tableWidgetTasks->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void ShiftTasksDialog::selectAllPipelines(void) {
	ui.tableWidgetTasks->clearSelection();
	ui.tableWidgetTasks->setSelectionMode(QAbstractItemView::MultiSelection);
	// pipelines cannot be moved to LST, so if the current move type is MOVE_TO_CENTER (= 'move to LST')
	// then change the move typ to MOVE_TO_START to update the redistribute dialog settings shown
	if (itsMoveType == MOVE_TO_CENTER) {
		itsMoveType = MOVE_TO_START;
	}
	for (int row = 0; row < ui.tableWidgetTasks->rowCount(); ++row) {
		int rowType(ui.tableWidgetTasks->item(row, 7)->data(Qt::UserRole).toInt());
        if (rowType >= SEL_CALIBRATOR_PIPELINES && rowType <= SEL_PULSAR_PIPELINES) {
			ui.tableWidgetTasks->selectRow(row);
		}
	}
	ui.tableWidgetTasks->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void ShiftTasksDialog::redistributeSelectedTasks(void) {
	QList<int> rows = getSelectedRows();

	if (!rows.isEmpty()) {
		itsRedistributeDialog->setRedistributeMode();
		// fetch the earliest start time from the selected tasks
		QDateTime earliestStart, cstart;
		for (QList<int>::const_iterator rowIt = rows.begin(); rowIt < rows.end(); ++rowIt) {
			if (earliestStart.isNull()) {
				if (!ui.tableWidgetTasks->item(*rowIt, 2)->text().isEmpty()) {
					earliestStart = ui.tableWidgetTasks->item(*rowIt, 2)->data(Qt::UserRole).toDateTime();
				}
			}
			else {
				if (!ui.tableWidgetTasks->item(*rowIt, 2)->text().isEmpty()) {
					cstart = ui.tableWidgetTasks->item(*rowIt, 2)->data(Qt::UserRole).toDateTime();
					if (cstart < earliestStart) {
						earliestStart = cstart;
					}
				}
			}
		}
		if (earliestStart.isNull()) {
			itsRedistributeDialog->setStartTime(QDateTime::currentDateTimeUtc());
		}
		else {
			itsRedistributeDialog->setStartTime(earliestStart);
		}
		itsMoveType == MOVE_TO_CENTER ? itsRedistributeDialog->setMoveToLSTType() : itsRedistributeDialog->setMoveToStartType();
		if (itsRedistributeDialog->exec()) {
			const QDateTime start(itsRedistributeDialog->getStartTime());
			const QTime timeStep1(itsRedistributeDialog->getTimeStep1());
			const QTime timeStep2(itsRedistributeDialog->getTimeStep2());
			int nrParallelTasks(itsRedistributeDialog->getNrParallelTasks());

			AstroTime gap1(timeStep1), gap2(timeStep2);
			AstroDateTime startTime(start);
			AstroDateTime endTime(startTime.addSeconds(ui.tableWidgetTasks->item(rows.first(), 4)->data(Qt::UserRole).toInt()));
			if (static_cast<Task::task_status>(ui.tableWidgetTasks->item(rows.first(), 6)->data(Qt::UserRole).toInt()) <= Task::PRESCHEDULED) {
				ui.tableWidgetTasks->item(rows.first(), 2)->setText(startTime.toString().c_str());
				ui.tableWidgetTasks->item(rows.first(), 2)->setData(Qt::UserRole, startTime.toQDateTime());
				ui.tableWidgetTasks->item(rows.first(), 3)->setText(endTime.toString().c_str());
				ui.tableWidgetTasks->item(rows.first(), 3)->setData(Qt::UserRole, endTime.toQDateTime());
			}
			int taskCounter = 0;
			AstroTime gap(gap1);
			bool gapAlternator(true);
			for (int idx = 1; idx < rows.size(); ++idx) {
				int row(rows.at(idx));
				Task::task_status state(static_cast<Task::task_status>(ui.tableWidgetTasks->item(row, 6)->data(Qt::UserRole).toInt()));
				int durationSec(ui.tableWidgetTasks->item(row, 4)->data(Qt::UserRole).toInt());
				if (state <= Task::PRESCHEDULED) {
					if (++taskCounter == nrParallelTasks) {
						taskCounter = 0;
						startTime = endTime + gap;	// use same start time for parallel task
						// alternate between gap1 and gap2
						if (gapAlternator) {
							gap = gap2;
							gapAlternator = false;
						}
						else {
							gap = gap1;
							gapAlternator = true;
						}
					}
					endTime = startTime.addSeconds(durationSec);

					ui.tableWidgetTasks->item(row, 2)->setText(startTime.toString().c_str());
					ui.tableWidgetTasks->item(row, 2)->setData(Qt::UserRole, startTime.toQDateTime());
					ui.tableWidgetTasks->item(row, 3)->setText(endTime.toString().c_str());
					ui.tableWidgetTasks->item(row, 3)->setData(Qt::UserRole, endTime.toQDateTime());
				}
			}

			if (itsMoveType == MOVE_TO_CENTER) { // now move them to LST if necessary
				applyPreview();
			}
		}
	}
}

void ShiftTasksDialog::scheduleAfterPredecessor(void) {
	QList<int> rows = getSelectedRows();
	if (!rows.isEmpty()) {
		itsRedistributeDialog->setAfterPredecessorMode();
		itsRedistributeDialog->setStartTime(ui.dateTimeEditNow->dateTime());
		if (itsRedistributeDialog->exec()) {
			AstroTime timeStep(itsRedistributeDialog->getTimeStep1());
			const Task *predTask(0);
			QString errStr;
			for (int idx = 0; idx < rows.size(); ++idx) {
				QDateTime lastPredEnd;
				int row(rows.at(idx));
				const Task *pTask(itsController->getTask(ui.tableWidgetTasks->item(row, 0)->data(Qt::UserRole).toInt()));
				if (pTask->getStatus() <= Task::PRESCHEDULED) {
					// search for the last predecessor end time
					const IDvector &predecessors(pTask->getPredecessors());
					if (!predecessors.empty()) {
						// now look for these predecessors in the preview list and use the specified times from there
						// (they might already be changed and thus be different from the times in the schedule)
						bool timeSet(false);
						for (IDvector::const_iterator prit = predecessors.begin(); prit != predecessors.end(); ++prit) {
							unsigned predID(0);
							if (prit->first != ID_SCHEDULER) {
								predTask = itsController->getTask(prit->second, prit->first);
								if (predTask) {
									predID = predTask->getID();
								}
							}
							else {
								predID = prit->second;
							}
							if (predID) {
								bool found(false);
								for (int rowIdx = 0; rowIdx < ui.tableWidgetTasks->rowCount(); ++rowIdx) {
									if (predID == ui.tableWidgetTasks->item(rowIdx, 0)->data(Qt::UserRole).toUInt()) {
										found = true;
										const QDateTime &predEndTime = ui.tableWidgetTasks->item(rowIdx, 3)->data(Qt::UserRole).toDateTime();
										if (predEndTime.isValid() && predEndTime > lastPredEnd) {
											lastPredEnd = predEndTime;
											timeSet = true;
										}
										break; // search for next predecessor
									}
								}
								if (!found) {
									errStr += QString::number(pTask->getSASTreeID()) + " predecessor:" + QString::number(prit->second) + " not found!\n";
								}
							}
						}

						if (timeSet) {
							AstroDateTime newStart = lastPredEnd + timeStep;
							AstroDateTime endTime = newStart.addSeconds(ui.tableWidgetTasks->item(row, 4)->data(Qt::UserRole).toInt());
							ui.tableWidgetTasks->item(row, 2)->setText(newStart.toString().c_str());
							ui.tableWidgetTasks->item(row, 2)->setData(Qt::UserRole, newStart.toQDateTime());
							ui.tableWidgetTasks->item(row, 3)->setText(endTime.toString().c_str());
							ui.tableWidgetTasks->item(row, 3)->setData(Qt::UserRole, endTime.toQDateTime());
						}
					}
					else {
						errStr += QString::number(pTask->getSASTreeID()) + " does not have predecessors defined\n";
					}
				}
			}
			if (!errStr.isEmpty()) {
				QMessageBox::warning(this, tr("Troubles"), errStr, tr("Close"));
			}
		}
	}
}


void ShiftTasksDialog::loadTargets(void) {
	bool beamsLoaded(false);
    const std::vector<unsigned> &tasks(itsController->selectedTasks());
    if (!tasks.empty()) {
        const Observation *pObs;
        for (std::vector<unsigned>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
            pObs = itsController->getObservation(*it);
            if (pObs) {
                QStringList props;
                const std::map<unsigned, DigitalBeam> &beams(pObs->getDigitalBeams());
                if (!beams.empty()) {
                    props << QString::number(pObs->getSASTreeID()) << pObs->getTaskDescription() << pObs->getScheduledStart().toString().c_str() << pObs->getDuration().toString(3).c_str();
                    QTreeWidgetItem *obsItem = new QTreeWidgetItem(ui.treeWidgetAngles, props);
                    obsItem->setFlags(Qt::ItemIsEnabled);

                    for (std::map<unsigned, DigitalBeam>::const_iterator beamIt = beams.begin(); beamIt != beams.end(); ++beamIt) {
                        props.clear();
                        props << "" << beamIt->second.angle1().HMSstring().c_str() << beamIt->second.target().c_str();
                        QTreeWidgetItem *beamItem = new QTreeWidgetItem(obsItem, props);
                        //							QTreeWidgetItem *item = new QTreeWidgetItem(str.c_str(), ui.treeWidgetAngles);
                        //							beamItem->setData(0, Qt::UserRole, beamIt->second.angle1().HMSstring().c_str());
                        std::string angleStr(beamIt->second.angle1().HMSstring());
                        beamItem->setData(0, Qt::UserRole, angleStr.substr(0, angleStr.find_first_of(".")).c_str());
                        beamItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
                        beamsLoaded = true;
                    }
                }
            }
        }
	}
	ui.treeWidgetAngles->expandAll();
	ui.treeWidgetAngles->resizeColumnToContents(0);
	ui.treeWidgetAngles->resizeColumnToContents(1);
	ui.treeWidgetAngles->resizeColumnToContents(2);
	ui.treeWidgetAngles->resizeColumnToContents(3);
	ui.pushButtonCenterAtLST->setEnabled(beamsLoaded);
}

void ShiftTasksDialog::loadTasks(void) {
	const std::vector<unsigned> &tasks(itsController->selectedTasks());
	if (!tasks.empty()) {
		ui.tableWidgetTasks->setRowCount(tasks.size());
		const Task *task;
		int row(0);
		for (std::vector<unsigned>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
			task = itsController->getTask(*it);
			if (task) {
				// SAS ID
				QTableWidgetItem *item = new QTableWidgetItem(QString::number(task->getSASTreeID()));
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				item->setData(Qt::UserRole, *it); // save the task id in the user data, for later reference
				ui.tableWidgetTasks->setItem(row, 0, item);
				// task name
				item = new QTableWidgetItem(task->getTaskName());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				ui.tableWidgetTasks->setItem(row, 1, item);
				// planned start
				item = new QTableWidgetItem(task->getScheduledStart().toString().c_str());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				item->setData(Qt::UserRole, task->getScheduledStart().toQDateTime());
				ui.tableWidgetTasks->setItem(row, 2, item);
				// planned end
				item = new QTableWidgetItem(task->getScheduledEnd().toString().c_str());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				item->setData(Qt::UserRole, task->getScheduledEnd().toQDateTime());
				ui.tableWidgetTasks->setItem(row, 3, item);
				// duration
				item = new QTableWidgetItem(task->getDuration().toString(3).c_str());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				item->setData(Qt::UserRole, task->getDuration().totalSeconds());
				ui.tableWidgetTasks->setItem(row, 4, item);
				// task description
				item = new QTableWidgetItem(task->SASTree().description().c_str());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				ui.tableWidgetTasks->setItem(row, 5, item);
				// status
				item = new QTableWidgetItem(task->getStatusStr());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				item->setData(Qt::UserRole, task->getStatus()); // save the task status
				ui.tableWidgetTasks->setItem(row, 6, item);
				// type
				item = new QTableWidgetItem(task->getTypeStr());
				item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled);
				// save the task type
				Task::task_type type(task->getType());
				if (type == Task::OBSERVATION) {
                    item->setData(Qt::UserRole, SEL_OBSERVATIONS); // observation
				}
				else if ((type == Task::PIPELINE) && (task->getStrategy().contains("calibrator", Qt::CaseInsensitive))) {
                    item->setData(Qt::UserRole, SEL_CALIBRATOR_PIPELINES); // calibrator pipeline
				}
				else if ((type == Task::PIPELINE) && (task->getStrategy().contains("target", Qt::CaseInsensitive))) {
                    item->setData(Qt::UserRole, SEL_TARGET_PIPELINES); // target pipeline
				}
				else if (task->getProcessSubtype() == PST_AVERAGING_PIPELINE) {
                    item->setData(Qt::UserRole, SEL_PREPROCESSING_PIPELINES);	// pre-processing pipeline
				}
                else if (task->getProcessSubtype() == PST_IMAGING_PIPELINE || task->getProcessSubtype() == PST_MSSS_IMAGING_PIPELINE) {
                    item->setData(Qt::UserRole, SEL_IMAGING_PIPELINES);	// imaging pipeline
				}
                else if (task->getProcessSubtype() == PST_LONG_BASELINE_PIPELINE) {
                    item->setData(Qt::UserRole, SEL_LONGBASELINE_PIPELINES);	// long-baseline pipeline
                }
                else if (type == Task::RESERVATION) {
                    item->setData(Qt::UserRole, SEL_RESERVATION);	// reservation
				}
				else if (type == Task::MAINTENANCE) {
                    item->setData(Qt::UserRole, SEL_MAINTENANCE);	// maintenance
				}
				else if (type == Task::SYSTEM) {
                    item->setData(Qt::UserRole, SEL_SYSTEM);	// system
				}
				else {
                    item->setData(Qt::UserRole, SEL_UNKNOWN);	// unknown type
				}
				ui.tableWidgetTasks->setItem(row, 7, item);
			}
			++row;
		}
		ui.tableWidgetTasks->resizeColumnsToContents();
	}
}

void ShiftTasksDialog::accept(void) {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QCoreApplication::processEvents(); // force update to paint dialog
	if (itsController) {
		std::map<unsigned, AstroDateTime> tasks;
		for (int row = 0; row < ui.tableWidgetTasks->rowCount(); ++row) {
			if (static_cast<Task::task_status>(ui.tableWidgetTasks->item(row,6)->data(Qt::UserRole).toInt()) <= Task::PRESCHEDULED) {
				if (!ui.tableWidgetTasks->item(row, 2)->text().isEmpty()) {
					tasks[ui.tableWidgetTasks->item(row, 0)->data(Qt::UserRole).toInt()] =
							AstroDateTime(ui.tableWidgetTasks->item(row, 2)->data(Qt::UserRole).toDateTime());
				}
			}
		}
        if (itsController->moveTasks(tasks)) {
            QDialog::accept();
        }
	}
    QApplication::restoreOverrideCursor();
    QCoreApplication::processEvents(); // force update to paint dialog
}

void ShiftTasksDialog::applyPreview(void) {
	QList<int> rows = getSelectedRows();
	std::map<unsigned, Task> shiftedTasks;
	if (!rows.isEmpty()) {
		bool showWarning(false);
		std::vector<int> taskRows;
		for (int idx = 0; idx < rows.size(); ++idx) {
			int row(rows.at(idx));
			if (static_cast<Task::task_status>(ui.tableWidgetTasks->item(row, 6)->data(Qt::UserRole).toInt()) <= Task::PRESCHEDULED) {
				taskRows.push_back(row);
			}
			else showWarning = true;
		}

		AstroDateTime new_start, new_end;
		if ((itsMoveType == MOVE_LEFT) || (itsMoveType == MOVE_RIGHT)) { // these are relative moves so interpret only the time part of date_time (as a relative time)
			for (std::vector<int>::iterator it = taskRows.begin(); it != taskRows.end(); ++it) {
				switch (itsMoveType) {
				case MOVE_LEFT:
					new_start = AstroDateTime(ui.tableWidgetTasks->item(*it, 2)->data(Qt::UserRole).toDateTime()) - AstroTime(ui.lineEditTimeShift->text());
					break;
				default: // MOVE_RIGHT
					new_start = AstroDateTime(ui.tableWidgetTasks->item(*it, 2)->data(Qt::UserRole).toDateTime()) + AstroTime(ui.lineEditTimeShift->text());
					break;
				}
				new_end = new_start.addSeconds(ui.tableWidgetTasks->item(*it, 4)->data(Qt::UserRole).toInt());

				// apply to preview
				ui.tableWidgetTasks->item(*it, 2)->setText(new_start.toString().c_str());
				ui.tableWidgetTasks->item(*it, 2)->setData(Qt::UserRole, new_start.toQDateTime()); // save the task id in the user data, for later reference
				ui.tableWidgetTasks->item(*it, 3)->setText(new_end.toString().c_str());
				ui.tableWidgetTasks->item(*it, 3)->setData(Qt::UserRole, new_end.toQDateTime());
			}
		}
		else if (itsMoveType == MOVE_TO_START) { // shift to absolute specified time
			if (!taskRows.empty()) {
				AstroDateTime earliestStartTime = AstroDateTime(ui.tableWidgetTasks->item(taskRows.front(), 2)->data(Qt::UserRole).toDateTime()), startTime;
				for (std::vector<int>::iterator it = taskRows.begin(); it != taskRows.end(); ++it) {
					startTime = AstroDateTime(ui.tableWidgetTasks->item(*it, 2)->data(Qt::UserRole).toDateTime());
					if (startTime < earliestStartTime) {
						earliestStartTime = startTime;
					}
				}

				AstroDateTime newFirstTime(ui.dateTimeEditNow->dateTime());
				AstroTime dif;
				bool left_right;
				if (earliestStartTime < newFirstTime) { // shift right
					dif = newFirstTime - earliestStartTime;
					left_right  = true;

				}
				else { // shift left
					dif = earliestStartTime - newFirstTime;
					left_right  = false;
				}

				for (std::vector<int>::iterator it = taskRows.begin(); it != taskRows.end(); ++it) {
					startTime = AstroDateTime(ui.tableWidgetTasks->item(*it, 2)->data(Qt::UserRole).toDateTime());
					if (left_right) {
						new_start = startTime + dif;
					}
					else {
						new_start = startTime - dif;
					}
					new_end = new_start.addSeconds(ui.tableWidgetTasks->item(*it, 4)->data(Qt::UserRole).toInt());

					// apply to preview
					ui.tableWidgetTasks->item(*it, 2)->setText(new_start.toString().c_str());
					ui.tableWidgetTasks->item(*it, 2)->setData(Qt::UserRole, new_start.toQDateTime()); // save the task id in the user data, for later reference
					ui.tableWidgetTasks->item(*it, 3)->setText(new_end.toString().c_str());
					ui.tableWidgetTasks->item(*it, 3)->setData(Qt::UserRole, new_end.toQDateTime());

				}
			}
		}
		else { // MOVE_TO_CENTER
			if (!taskRows.empty()) {
				AstroDateTime earliestStartTime = AstroDateTime(ui.tableWidgetTasks->item(taskRows.front(), 2)->data(Qt::UserRole).toDateTime()), startTime, endTime;
				AstroDateTime latestEndTime = AstroDateTime(ui.tableWidgetTasks->item(taskRows.front(), 3)->data(Qt::UserRole).toDateTime());
                selector_types type;
				for (std::vector<int>::iterator it = taskRows.begin(); it != taskRows.end(); ++it) {
                    type = static_cast<selector_types>(ui.tableWidgetTasks->item(*it, 7)->data(Qt::UserRole).toInt());
                    if (type == SEL_OBSERVATIONS) { // only look at observations to determine the shift for a CENTER = LST move
						startTime = AstroDateTime(ui.tableWidgetTasks->item(*it, 2)->data(Qt::UserRole).toDateTime());
						endTime = AstroDateTime(ui.tableWidgetTasks->item(*it, 3)->data(Qt::UserRole).toDateTime());
						if (startTime < earliestStartTime) {
							earliestStartTime = startTime;
						}
						if (endTime > latestEndTime) {
							latestEndTime = endTime;
						}
					}
				}

				AstroDateTime oldCenterTime((latestEndTime.toJulian() + earliestStartTime.toJulian()) / 2);
				AstroTime dif = AstroDateTime(ui.dateTimeEditNow->dateTime()) - oldCenterTime;

				for (std::vector<int>::iterator it = taskRows.begin(); it != taskRows.end(); ++it) {
					startTime = AstroDateTime(ui.tableWidgetTasks->item(*it, 2)->data(Qt::UserRole).toDateTime());
					new_start = startTime + dif;
					new_end = new_start.addSeconds(ui.tableWidgetTasks->item(*it, 4)->data(Qt::UserRole).toInt());

					ui.tableWidgetTasks->item(*it, 2)->setText(new_start.toString().c_str());
					ui.tableWidgetTasks->item(*it, 2)->setData(Qt::UserRole, new_start.toQDateTime()); // save the task id in the user data, for later reference
					ui.tableWidgetTasks->item(*it, 3)->setText(new_end.toString().c_str());
					ui.tableWidgetTasks->item(*it, 3)->setData(Qt::UserRole, new_end.toQDateTime());
				}
			}
		}


		if (showWarning) {
			QMessageBox::warning(this, tr("Some tasks could not be moved"),
					"Some tasks have a status above PRESCHEDULED. These tasks cannot not be moved.", tr("Close"));
		}

		// now apply the preview
		for (std::map<unsigned, Task>::const_iterator taskit = shiftedTasks.begin(); taskit != shiftedTasks.end(); ++taskit) {
			for (int row = 0; row < ui.tableWidgetTasks->rowCount(); ++row) {
				if (ui.tableWidgetTasks->item(row, 0)->data(Qt::UserRole).toUInt() == taskit->first) {
					ui.tableWidgetTasks->item(row, 2)->setText(taskit->second.getScheduledStart().toString().c_str());
					ui.tableWidgetTasks->item(row, 2)->setData(Qt::UserRole, taskit->second.getScheduledStart().toQDateTime()); // save the task id in the user data, for later reference
					ui.tableWidgetTasks->item(row, 3)->setText(taskit->second.getScheduledEnd().toString().c_str());
					ui.tableWidgetTasks->item(row, 3)->setData(Qt::UserRole, taskit->second.getScheduledEnd().toQDateTime());
					break;
				}
			}
		}
	}
}


void ShiftTasksDialog::doLSTCheck(void) {
	if (itsMoveType == MOVE_TO_CENTER) { // MOVE_TO_CENTER means LST move here
		calculateLST();
	}
	else {
		applyPreview();
	}
}


void ShiftTasksDialog::calculateNow(void) {
	itsMoveType = MOVE_TO_START;
	QDateTime nu;
	nu = QDateTime::currentDateTimeUtc();
	nu = nu.addSecs(300);
	ui.dateTimeEditNow->blockSignals(true);
	ui.dateTimeEditNow->setDateTime(nu);
	ui.dateTimeEditNow->blockSignals(false);
	ui.lineEditTimeShift->setText("0000:00:00");
//	applyPreview();
}

int ShiftTasksDialog::getSelectedTasksTimeSpan(void) const {
	QList<int> rows = getSelectedRows();
	QDateTime startTime, endTime, earliestStartTime, latestEndTime;
	bool isSet(false);
	if (!rows.isEmpty()) {
		for (int idx = 0; idx < rows.size(); ++idx) {
			int row(rows.at(idx));
			const QDateTime &startTime = ui.tableWidgetTasks->item(row, 2)->data(Qt::UserRole).toDateTime();
			const QDateTime &endTime = ui.tableWidgetTasks->item(row, 2)->data(Qt::UserRole).toDateTime();
			if (!isSet) {
				earliestStartTime = startTime;
				latestEndTime = endTime;
				isSet = true;
			}
			else {
				if (startTime < earliestStartTime) {
					earliestStartTime = startTime;
				}
				if (endTime > latestEndTime) {
					latestEndTime = endTime;
				}
			}
		}
	}
	if (isSet) {
		return (latestEndTime - earliestStartTime).totalSeconds();
	}
	else return 0;
}

void ShiftTasksDialog::applyShift(void) {
	itsMoveType = ui.radioButtonShiftLeft->isChecked() ? MOVE_LEFT : MOVE_RIGHT;
	applyPreview();
}

void ShiftTasksDialog::calculateLST(void) {
	itsMoveType = MOVE_TO_CENTER;
	QList<QTreeWidgetItem *> items(ui.treeWidgetAngles->selectedItems());
	// if the date(day) has already been set by the user then don't change the date
	// if it has not been set then set it to today
	AstroDate day;
	if (ui.dateTimeEditNow->date().toJulianDay() == J2000_EPOCH) { // by default the date equals the J2000_EPOCH
		day = AstroDate(QDateTime::currentDateTime().date());
		ui.dateTimeEditNow->blockSignals(true);
		ui.dateTimeEditNow->setDate(QDateTime::currentDateTime().date());
		ui.dateTimeEditNow->blockSignals(false);
	}
	else {
		day = AstroDate(ui.dateTimeEditNow->date());
	}
	if (!items.isEmpty()) {
		AstroTime time;
		double tmean(0.0);
		for (QList<QTreeWidgetItem *>::const_iterator lit = items.begin(); lit != items.end(); ++lit) {
			time = (*lit)->data(0, Qt::UserRole).toString(); // the angle right ascension interpreted as a time
			tmean += time.toJulian();
		}
		tmean = tmean / items.count();
		time = AstroTime(tmean);

        AstroDateTime UTC = AstroDateTime::UTfromLST(time, day);
		int halfTimeSpanSeconds = getSelectedTasksTimeSpan() / 2;

		while (UTC.subtractSeconds(halfTimeSpanSeconds).toQDateTime() < QDateTime::currentDateTime().toUTC()) {
			day = day.addDays(1);
			UTC = AstroDateTime::UTfromLST(time, day);
		}
		ui.dateTimeEditNow->blockSignals(true);
		ui.dateTimeEditNow->setDateTime(UTC.toQDateTime());
		ui.dateTimeEditNow->blockSignals(false);

		// now update the tasks table accordingly to show the preview of the results
//		applyPreview();

	}
	else {
		QMessageBox::warning(this, "Select a beam right ascension", "First select one or more beam right ascension(s) to center upon.\nSelecting multiple will calculate the mean LST for these right ascensions.");
	}
}
