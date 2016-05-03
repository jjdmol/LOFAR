/*
 * qlofardatamodel.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jun 5, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/qlofardatamodel.cpp $
 *
 */

//#include <vector>
//#include <algorithm>
#include "qlofardatamodel.h"
#include "lofar_scheduler.h"
#include "task.h"

QLofarDataModel::QLofarDataModel(QObject *parent)
: QStandardItemModel(parent) /*, parentGUI(0) */
{
}

QLofarDataModel::QLofarDataModel(int rows, int columns, QObject *parent/*, SchedulerGUI * gui*/)
: QStandardItemModel(rows, columns, parent)
{
//	parentGUI = gui;
}

QLofarDataModel::~QLofarDataModel() {
}

int QLofarDataModel::findTaskRow(unsigned taskID) const {
	int nrRows = this->rowCount(QModelIndex());
	for (int row = 0; row <= nrRows; ++row) {
		if (this->data(this->index(row,TASK_ID)).toUInt() == taskID) return row; // if this is the row with the right task ID
	}
	return -1;
}


QVariant QLofarDataModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::BackgroundRole) {
		if (isErrorIndex(index)) // error
            return QColor(Qt::red);
		else {
			int task_status = index.model()->data(index.model()->index(index.row(),TASK_STATUS),USERDATA_ROLE).toInt();
			if (task_status == Task::FINISHED) {
				return QColor(145,255,145); // this task has finished, should not be editable (greenish background)
			}
			if (task_status == Task::COMPLETING) {
				return QColor(170,170,255); // this task is completing, should not be editable (purple background)
			}
			else if (task_status == Task::ABORTED) {
				return QColor(254,151,39);// 255,173,119); // orange-ish
			}
			else if (task_status == Task::ACTIVE) {
				return QColor(255,255,0);
			}
			else {
				int task_type = index.model()->data(index.model()->index(index.row(),TASK_TYPE),USERDATA_ROLE).toInt();
				if (task_type == Task::RESERVATION) {
					return QColor(254,108,108); // for non-editable cells
//					switch (static_cast<data_headers>(index.column())) {
//					case TASK_ID:
//					case SAS_ID:
//					case PROJECT_ID:
//					case TASK_NAME:
//					case TASK_DESCRIPTION:
//					case CONTACT_NAME:
//					case CONTACT_PHONE:
//					case CONTACT_EMAIL:
//					case STATION_ID:
//					case ANTENNA_MODE:
//					case CLOCK_FREQUENCY:
//					case FILTER_TYPE:
//					case TASK_DURATION:
//					case PLANNED_START:
//					case PLANNED_END:
//					case TASK_STATUS:
//						return Qt::white;
//						break;
//					default:
//						return QColor(220,220,220); // for non-editable cells
//						break;
//					}
				}
				else if (task_type == Task::MAINTENANCE) {
					return QColor(127,199,254); // for editable cells (blue-ish)
//					switch (static_cast<data_headers>(index.column())) {
//					case TASK_ID:
//					case SAS_ID:
//					case PROJECT_ID:
//					case TASK_NAME:
//					case TASK_DESCRIPTION:
//					case CONTACT_NAME:
//					case CONTACT_PHONE:
//					case CONTACT_EMAIL:
//					case STATION_ID:
//					case TASK_DURATION:
//					case PLANNED_START:
//					case PLANNED_END:
//					case TASK_STATUS:
//						return QColor(127,199,254); // for editable cells (blue-ish)
//						//return Qt::white; // for editable cells
//						break;
//					default:
//						return QColor(220,220,220); // for non-editable cells
//						break;
//					}
				}
				else { // regular tasks
                    return QColor(Qt::white);
				}
			}
		}
	}
	else if (role == Qt::ForegroundRole ) {
		if (isErrorIndex(index)) // error
            return QColor(Qt::white);
		else {
			int task_status = index.model()->data(index.model()->index(index.row(),TASK_STATUS),USERDATA_ROLE).toInt();
			if ((task_status >= Task::COMPLETING) && (task_status <= Task::ABORTED)) {
                return QColor(Qt::black);
			}
			else {
				int task_type = index.model()->data(index.model()->index(index.row(),TASK_TYPE),USERDATA_ROLE).toInt();
				if (task_type == Task::RESERVATION) {
					switch (static_cast<data_headers>(index.column())) {
					case TASK_ID:
					case SAS_ID:
					case PROJECT_ID:
					case TASK_NAME:
					case TASK_DESCRIPTION:
					case CONTACT_NAME:
					case CONTACT_PHONE:
					case CONTACT_EMAIL:
					case STATION_ID:
					case ANTENNA_MODE:
					case CLOCK_FREQUENCY:
					case FILTER_TYPE:
					case TASK_DURATION:
					case PLANNED_START:
					case PLANNED_END:
					case TASK_STATUS:
                        return QColor(Qt::black); // for editable cells
						break;
					default:
                        return QColor(Qt::darkGray); // for non-editable cells
						break;
					}
				}
				else if (task_type == Task::MAINTENANCE) {
					switch (static_cast<data_headers>(index.column())) {
					case TASK_ID:
					case SAS_ID:
					case PROJECT_ID:
					case TASK_NAME:
					case TASK_DESCRIPTION:
					case CONTACT_NAME:
					case CONTACT_PHONE:
					case CONTACT_EMAIL:
					case STATION_ID:
					case TASK_DURATION:
					case PLANNED_START:
					case PLANNED_END:
					case TASK_STATUS:
                        return QColor(Qt::black); // for editable cells
						break;
					default:
                        return QColor(Qt::darkGray); // for non-editable cells
						break;
					}
				}
				else { // regular tasks
                    return QColor(Qt::black);
				}
			}
		}
	}
//	else if (role == Qt::DisplayRole) {
////		QVariant value = QStandardItemModel::data(index, role);
//		return QStandardItemModel::data(index, role);
//	}
	else {
//		QVariant value = QStandardItemModel::data(index, role);
		return QStandardItemModel::data(index, role);
	}

	return QVariant();
}

/*
void QLofarDataModel::clearErrorIndices(void) {
	errorIndices.clear();
}
*/
void QLofarDataModel::clearErrorIndex(const QModelIndex &index) {
	for (std::vector<QModelIndex>::iterator it = errorIndices.begin(); it != errorIndices.end(); ++it) {
		if (it->row() == index.row() && it->column() == index.column()) {
			errorIndices.erase(it);
			return;
		}
	}
}

void QLofarDataModel::clearErrorCell(unsigned int taskID, data_headers header) {
	// Remove the error from the internal error-cell list
	int nrRows = this->rowCount(QModelIndex());
	for (int row = 0; row <= nrRows; ++row) {
		if (this->data(this->index(row,TASK_ID)).toUInt() == taskID) { // if this is the row with the right task ID
			for (std::vector<QModelIndex>::iterator it = errorIndices.begin(); it != errorIndices.end(); ++it) {
				if (it->row() == row && it->column() == header) {
					// Remove the erroneous cell from the internal data model (which is used to write table)
					setData(*it, QString(""));
					// now also clear the error from the internal error-cell list
					errorIndices.erase(it);
					return;
				}
			}
			return; // if index not found no need to continue searching
		}
	}
}

void QLofarDataModel::addErrorIndex(const QModelIndex &index) {
	if (!isErrorIndex(index)) // check if not already in error list
		errorIndices.push_back(index);
}

bool QLofarDataModel::isErrorIndex(const QModelIndex &index) const {
	for (std::vector<QModelIndex>::const_iterator it = errorIndices.begin(); it != errorIndices.end(); ++it) {
		if (it->row() == index.row() && it->column() == index.column())
			return true;
	}
	return false;
}

void QLofarDataModel::setErrorCells(const errorTasksMap & errorTasks) {
	int nrRows = this->rowCount(QModelIndex());
	clearErrorIndices();
	for (errorTasksMap::const_iterator it = errorTasks.begin(); it != errorTasks.end(); ++it) {
		// find the row containing the error tasks according to task ID
		for (int row = 0; row <= nrRows; ++row) {
			if (this->data(this->index(row, TASK_ID)).toUInt() == it->first) { // if this is the row with the right task ID
				for (std::vector<data_headers>::const_iterator dit = it->second.begin(); dit != it->second.end(); ++dit) {
					addErrorIndex(this->index(row, *dit));
				}
				break;
			}
		}
	}
}


