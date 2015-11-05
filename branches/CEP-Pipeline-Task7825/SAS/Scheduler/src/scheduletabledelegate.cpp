/*
 * scheduletabledelegate.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 2, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/scheduletabledelegate.cpp $
 *
 */

#include <QtGui>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <vector>
#include <string>
#include "Controller.h"
#include "lofar_utils.h"
#include "scheduletabledelegate.h"
#include "task.h"
#include "station.h"

ScheduleTableDelegate::ScheduleTableDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

ScheduleTableDelegate::~ScheduleTableDelegate() {
}

// create the widget and load it with possible choices/items
QWidget *ScheduleTableDelegate::createEditor(QWidget *parent,
		const QStyleOptionViewItem &/* option */,
		const QModelIndex &index) const
{
	QComboBox * comboBox = 0;
	QListWidget * listWidget = 0;
	QSpinBox * spinBox = 0;
	QTimeEdit * timeEdit = 0;
	QDateEdit * dateEdit = 0;
	QLineEdit * lineEdit = 0;
	QDateTimeEdit * dateTimeEdit = 0;
	QStringList items;

	int task_type = index.model()->data(index.model()->index(index.row(),TASK_TYPE),USERDATA_ROLE).toInt();
	int task_status = index.model()->data(index.model()->index(index.row(),TASK_STATUS),USERDATA_ROLE).toInt();

	if (task_status < Task::SCHEDULED) {
	if ((task_type == Task::RESERVATION) || (task_type == Task::MAINTENANCE)) {
		switch (index.column()) {
		case STATION_ID:
			listWidget = new QListWidget(parent);
			listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
			listWidget->setFixedHeight(100);
			listWidget->setViewMode(QListView::IconMode);
			listWidget->setMovement(QListView::Static);
			if (!Controller::theSchedulerSettings.getStationList().empty()) {
				for (stationDefinitionsMap::const_iterator it = Controller::theSchedulerSettings.getStationList().begin();
				it != Controller::theSchedulerSettings.getStationList().end() ; ++it) {
					items << it->first.c_str();
				}
			}
			else {
				items << "no stations defined";
			}
			listWidget->addItems(items);
			return listWidget;
		case TASK_DURATION: // duration
			lineEdit = new QLineEdit(parent);
			lineEdit->setToolTip("hhhh:mm:ss");
			lineEdit->setInputMask("0000:00:00");
			return lineEdit;
		case TASK_NAME:
		case PROJECT_ID:
		case CONTACT_NAME:
		case CONTACT_PHONE:
		case CONTACT_EMAIL:
		case TASK_DESCRIPTION:
			lineEdit = new QLineEdit(parent);
			return lineEdit;
		case FILTER_TYPE: // filter type
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_FILTER_TYPES ; ++i) {
				items << filter_types_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
		case ANTENNA_MODE: // antenna mode
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_ANTENNA_MODES ; ++i) {
				items << antenna_modes_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
		case CLOCK_FREQUENCY: // clock frequencies
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_CLOCK_FREQUENCIES; ++i) {
				items << clock_frequencies_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
		case PLANNED_START: // date-time edit
		case PLANNED_END:
			dateTimeEdit = new QDateTimeEdit(parent);
			dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
			dateTimeEdit->setCalendarPopup(true);
			return dateTimeEdit;
		case TASK_STATUS: // task status
			comboBox = new QComboBox(parent);
			items << task_states_str[Task::PRESCHEDULED] << task_states_str[Task::ON_HOLD];
			comboBox->addItems(items);
			return comboBox;
		default:
			return 0;
		}
	}
	else if (task_type == Task::PIPELINE) {
		switch (index.column()) {
		case PREDECESSORS: // predecessors IDs
			lineEdit = new QLineEdit(parent);
			return lineEdit;
		case WINDOW_MINIMUM_TIME:
		case WINDOW_MAXIMUM_TIME:
			timeEdit = new QTimeEdit(parent);
			timeEdit->setDisplayFormat("hh:mm:ss");
			return timeEdit;
		case PRED_MIN_TIME_DIF:
		case PRED_MAX_TIME_DIF:
		case TASK_DURATION: // duration
			lineEdit = new QLineEdit(parent);
			lineEdit->setToolTip("hhhh:mm:ss");
			lineEdit->setInputMask("0000:00:00");
			return lineEdit;
		case TASK_NAME:
		case PROJECT_ID:
		case CONTACT_NAME:
		case CONTACT_PHONE:
		case CONTACT_EMAIL:
		case PRIORITY: // priority
		case TASK_DESCRIPTION:
			lineEdit = new QLineEdit(parent);
			return lineEdit;
		case FIRST_POSSIBLE_DATE: // date edit
		case LAST_POSSIBLE_DATE:
			dateEdit = new QDateEdit(parent);
			dateEdit->setDisplayFormat("yyyy-MM-dd");
			dateEdit->setCalendarPopup(true);
			return dateEdit;
		case PLANNED_START: // date-time edit
		case PLANNED_END:
			dateTimeEdit = new QDateTimeEdit(parent);
			dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
			dateTimeEdit->setCalendarPopup(true);
			return dateTimeEdit;
		case FIXED_DAY: // fixed day and time (0/1)
		case FIXED_TIME:
			lineEdit = new QLineEdit(parent);
			lineEdit->setToolTip("0/1");
			lineEdit->setInputMask("9");
			return lineEdit;
		case TASK_STATUS: // task status
			comboBox = new QComboBox(parent);
			if (task_status == Task::PRESCHEDULED) {
				items << task_states_str[Task::ON_HOLD] << task_states_str[Task::UNSCHEDULED]
				      << task_states_str[Task::PRESCHEDULED] << task_states_str[Task::SCHEDULED];
			}
			else if (task_status == Task::ABORTED) {
				items << task_states_str[Task::UNSCHEDULED] << task_states_str[Task::ABORTED];
			}
			else {
				items << task_states_str[Task::ON_HOLD] << task_states_str[Task::UNSCHEDULED]
				      << task_states_str[Task::PRESCHEDULED];
			}
			comboBox->addItems(items);
			return comboBox;
			break;
		}
	}
	else { // regular tasks:
		switch (index.column()) {
/*
		case TASK_TYPE: // task type
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_TASK_TYPES - 1; ++i) {
				items << task_types_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
*/
		case PREDECESSORS: // predecessors IDs
			lineEdit = new QLineEdit(parent);
			return lineEdit;
		case STATION_ID: // station names
			listWidget = new QListWidget(parent);
			listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
			listWidget->setFixedHeight(100);
			listWidget->setViewMode(QListView::IconMode);
			listWidget->setMovement(QListView::Static);
			if (!Controller::theSchedulerSettings.getStationList().empty()) {
				for (stationDefinitionsMap::const_iterator it = Controller::theSchedulerSettings.getStationList().begin();
				it != Controller::theSchedulerSettings.getStationList().end() ; ++it) {
					items << it->first.c_str();
				}
			}
			else {
				items << "no stations defined";
			}
			listWidget->addItems(items);
			return listWidget;
		case WINDOW_MINIMUM_TIME:
		case WINDOW_MAXIMUM_TIME:
			timeEdit = new QTimeEdit(parent);
			timeEdit->setDisplayFormat("hh:mm:ss");
			return timeEdit;
		case PRED_MIN_TIME_DIF:
		case PRED_MAX_TIME_DIF:
		case TASK_DURATION: // duration
			lineEdit = new QLineEdit(parent);
			lineEdit->setToolTip("hhhh:mm:ss");
			lineEdit->setInputMask("0000:00:00");
			return lineEdit;
		case TASK_NAME:
		case PROJECT_ID:
		case CONTACT_NAME:
		case CONTACT_PHONE:
		case CONTACT_EMAIL:
		case PRIORITY: // priority
		case TASK_DESCRIPTION:
			lineEdit = new QLineEdit(parent);
			return lineEdit;
		case FILTER_TYPE: // filter type
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_FILTER_TYPES ; ++i) {
				items << filter_types_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
		case ANTENNA_MODE: // antenna mode
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_ANTENNA_MODES ; ++i) {
				items << antenna_modes_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
		case CLOCK_FREQUENCY: // clock frequencies
			comboBox = new QComboBox(parent);
			for (short int i=0; i < NR_CLOCK_FREQUENCIES; ++i) {
				items << clock_frequencies_str[i];
			}
			comboBox->addItems(items);
			return comboBox;
		case FIRST_POSSIBLE_DATE: // date edit
		case LAST_POSSIBLE_DATE:
			dateEdit = new QDateEdit(parent);
			dateEdit->setDisplayFormat("yyyy-MM-dd");
			dateEdit->setCalendarPopup(true);
			return dateEdit;
		case PLANNED_START: // date-time edit
		case PLANNED_END:
			dateTimeEdit = new QDateTimeEdit(parent);
			dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
			dateTimeEdit->setCalendarPopup(true);
			return dateTimeEdit;
		case FIXED_DAY: // fixed day and time (0/1)
		case FIXED_TIME:
			lineEdit = new QLineEdit(parent);
			lineEdit->setToolTip("0/1");
			lineEdit->setInputMask("9");
			return lineEdit;
		case TASK_STATUS: // task status
			comboBox = new QComboBox(parent);
			if (task_status == Task::PRESCHEDULED) {
				items << task_states_str[Task::ON_HOLD] << task_states_str[Task::UNSCHEDULED]
				      << task_states_str[Task::PRESCHEDULED] << task_states_str[Task::SCHEDULED];
			}
			else if (task_status == Task::ABORTED) {
				items << task_states_str[Task::UNSCHEDULED] << task_states_str[Task::ABORTED];
			}
			else if (task_status == Task::CONFLICT) {
				items << task_states_str[Task::ON_HOLD] << task_states_str[Task::UNSCHEDULED]
				      << task_states_str[Task::PRESCHEDULED];
			}
			else {
				items << task_states_str[Task::ON_HOLD] << task_states_str[Task::UNSCHEDULED]
				      << task_states_str[Task::PRESCHEDULED];
			}
			comboBox->addItems(items);
			return comboBox;
			break;
		case NIGHT_TIME_WEIGHT_FACTOR: // night time weight factor (spinbox)
			spinBox = new QSpinBox(parent);
			spinBox->setRange(0, 100);
			return spinBox;
		}
	}
	}
	else { // all tasks with status > SCHEDULED
		if (index.column() == TASK_STATUS) {
			comboBox = new QComboBox(parent);
			if (task_status == Task::SCHEDULED) {
				items << task_states_str[Task::ON_HOLD] << task_states_str[Task::UNSCHEDULED] << task_states_str[Task::PRESCHEDULED] << task_states_str[Task::SCHEDULED]; // we allow also aborting a task which in the scheduler has state SCHEDULED (it could already be running)
				comboBox->addItems(items);
				return comboBox;
			}
			else if (task_status == Task::ABORTED) { // from ABORTED we can go to UNSCHEDULED only
				items << task_states_str[Task::UNSCHEDULED] << task_states_str[Task::ABORTED];
				comboBox->addItems(items);
				return comboBox;
			}
			else if (task_status == Task::OBSOLETE) {
				items << task_states_str[Task::UNSCHEDULED];
				comboBox->addItems(items);
				return comboBox;
			}
		}
	}
	return 0;
}

// setEditorData sets the correct value when user wants to edit the table cell
void ScheduleTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString valueStr;
	unsigned int uValue;
	QSpinBox * spinBox = 0;
	QListWidget * listWidget = 0;
	QComboBox * comboBox = 0;
	QTimeEdit * timeEdit = 0;
	QDateEdit * dateEdit = 0;
	QLineEdit * lineEdit = 0;
	QTime time;
	QDateTimeEdit * dateTimeEdit = 0;
	QDateTime dateTimeValue;
	QStringList stringListItems;
	QList<QListWidgetItem *> listItems;
	int count;
	bool found;
	switch (index.column()) {
	case TASK_TYPE: // task type
		comboBox = static_cast<QComboBox *>(editor); // convert the editor into a combobox for the task type column
		valueStr = index.model()->data(index).toString();
		for (short int i=0; i < NR_TASK_TYPES; ++i) {
			if (valueStr.compare(task_types_str[i]) == 0) {
				comboBox->setCurrentIndex(i);
				break;
			}
		}
		break;
	case STATION_ID: // station names
		listWidget = static_cast<QListWidget *>(editor); // convert the editor into a listbox to select station (names)
		stringListItems = index.model()->data(index).toStringList();
		for (QStringList::const_iterator sit = stringListItems.begin(); sit != stringListItems.end(); ++sit) {
			listItems = listWidget->findItems(*sit, Qt::MatchExactly);
			if (!listItems.empty())
				listItems.first()->setSelected(true);
		}
		break;
	case TASK_NAME:
	case PROJECT_ID:
	case CONTACT_NAME:
	case CONTACT_PHONE:
	case CONTACT_EMAIL:
	case PREDECESSORS: // predecessor ID
	case PRIORITY: // station IDs
	case PRED_MIN_TIME_DIF:
	case PRED_MAX_TIME_DIF:
	case TASK_DURATION: // duration
	case TASK_DESCRIPTION:
	case STORAGE_SIZE: // storage units
		lineEdit = static_cast<QLineEdit *>(editor);
		valueStr = index.model()->data(index).toString();
		lineEdit->setText(valueStr);
		break;
	case WINDOW_MINIMUM_TIME:
	case WINDOW_MAXIMUM_TIME:
		timeEdit = static_cast<QTimeEdit *>(editor);
		time = index.model()->data(index).toTime();
		timeEdit->setDisplayFormat("hh:mm:ss");
		timeEdit->setTime(time);
		break;
	case FILTER_TYPE: // filter type
		comboBox = static_cast<QComboBox *>(editor);
		valueStr = index.model()->data(index).toString();
		for (short int i=0; i < NR_FILTER_TYPES; ++i) {
			if (valueStr.compare(filter_types_str[i]) == 0) {
				comboBox->setCurrentIndex(i);
				break;
			}
		}
		break;
	case ANTENNA_MODE: // antenna mode
		comboBox = static_cast<QComboBox *>(editor);
		valueStr = index.model()->data(index).toString();
		for (short int i=0; i < NR_ANTENNA_MODES ; ++i) {
			if (valueStr.compare( antenna_modes_str[i]) == 0) {
				comboBox->setCurrentIndex(i);
				break;
			}
		}
		break;
	case CLOCK_FREQUENCY: // clock frequencies
		comboBox = static_cast<QComboBox *>(editor); // convert the editor into a combobox for the task type column
		valueStr = index.model()->data(index).toString();
		for (short int i=0; i < NR_CLOCK_FREQUENCIES; ++i) {
			if (valueStr.compare(clock_frequencies_str[i]) == 0) {
				comboBox->setCurrentIndex(i);
				break;
			}
		}
		break;
	case FIRST_POSSIBLE_DATE: // date edit
	case LAST_POSSIBLE_DATE:
		dateEdit = static_cast<QDateEdit *>(editor);
		dateEdit->setDisplayFormat("yyyy-MM-dd");
		dateEdit->setDate(index.model()->data(index).toDate());
		break;
	case PLANNED_START: // date-time edit
	case PLANNED_END:
		dateTimeEdit = static_cast<QDateTimeEdit *>(editor);
		dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
		dateTimeValue = index.model()->data(index).toDateTime();
		if (dateTimeValue.isNull()) {
            QDateTime cDate(QDateTime::currentDateTimeUtc());
            dateTimeEdit->setDateTime(QDateTime(cDate.date(),QTime(cDate.time().hour(),cDate.time().minute(),0)));
		}
		else {
			dateTimeEdit->setDateTime(dateTimeValue);
		}
		break;
	case FIXED_DAY: // fixed day and time (1/0)
	case FIXED_TIME:
		lineEdit = static_cast<QLineEdit *>(editor);
		uValue = index.model()->data(index).toUInt();
		if ((uValue == 0) || (uValue == 1)) {
			lineEdit->setText(index.model()->data(index).toString());
		}
		break;
	case TASK_STATUS: // task status
		comboBox = static_cast<QComboBox *>(editor); // convert the editor into a combobox for the task type column
		valueStr = index.model()->data(index).toString();
		// check if task is finished
		// check task type
//		task_type = index.model()->data(index.model()->index(index.row(),TASK_TYPE),USERDATA_ROLE).toInt();
		count = 0;
		found = false;
		while (count < comboBox->count()) {
			if (valueStr.compare(comboBox->itemText(count)) == 0) {
				comboBox->setCurrentIndex(count);
				found = true;
				break;
			}
			++count;
		}
		if (!found) {
			for (count=0; count < NR_TASK_STATES ; ++count) {
				if (valueStr.compare(task_states_str[count]) == 0) {
					comboBox->addItem(task_states_str[count]);
					comboBox->setCurrentIndex(comboBox->count()-1);
					break;
				}
			}
		}
		break;
	case NIGHT_TIME_WEIGHT_FACTOR: // night time weight factor (spinbox)
		spinBox = static_cast<QSpinBox *>(editor);
		uValue = index.model()->data(index).toUInt();
		spinBox->setValue(uValue);
		break;
	}
}

void ScheduleTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	bool change = false;
	int pos=0;
	QString strValue, strValue2;
	QListWidget * listWidget = 0;
	QComboBox * comboBox = 0;
	QSpinBox * spinBox = 0;
	QTimeEdit * timeEdit;
	QDateEdit * dateEdit;
	QDateTimeEdit * dateTimeEdit = 0;
	QLineEdit * lineEdit = 0;
	QDoubleValidator dValidator(editor);
	QIntValidator iValidator(editor);
	QRegExpValidator regExpValidator(editor);
	QRegExp regExpEmail("[A-Z0-9._%+-]{1,64}@[A-Z0-9-]{1,253}.[A-Z]{2,4}");
	regExpEmail.setCaseSensitivity(Qt::CaseInsensitive);
	QList<QListWidgetItem *> selectedItems;
	QStringList stringListItems, prev_list;

	unsigned taskID = model->data(model->index(index.row(), TASK_ID, QModelIndex())).toUInt();
	QVariant newValue;

	switch (index.column()) {
	case TASK_TYPE: // task type
		comboBox = static_cast<QComboBox *>(editor);
		strValue = model->data(index).toString();
		if (strValue.compare(task_types_str[comboBox->currentIndex()]) != 0) {
			model->setData(index, QVariant(task_types_str[comboBox->currentIndex()]));
			newValue =  model->data(index).toString();
			change = true;
		}
		break;
	case TASK_NAME:
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		if (strValue.length() < 3) {
			QMessageBox::warning(editor, tr("Task name not valid"),
					tr("The task name must contain at least three characters"),
							QMessageBox::Ok, QMessageBox::Ok);
		}
		else {
			model->setData(index, strValue);
			newValue = strValue;
			change = true;
		}
		break;
	case PROJECT_ID:
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		if (strValue.length() < 3) {
			QMessageBox::warning(editor, tr("Project name not valid"),
					tr("The project name must contain at least three characters"),
							QMessageBox::Ok, QMessageBox::Ok);
		}
		else {
			model->setData(index, strValue);
			newValue = strValue;
			change = true;
		}
		break;
	case CONTACT_NAME:
	case CONTACT_PHONE:
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		model->setData(index, strValue);
		newValue = strValue;
		change = true;
		break;
	case CONTACT_EMAIL:
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		regExpValidator.setRegExp(regExpEmail);
		if ((regExpValidator.validate(strValue, pos) == QValidator::Acceptable) | (strValue == "")) {
			model->setData(index, strValue);
			newValue = strValue;
			change = true;
		}
		else {
			QMessageBox::warning(editor, tr("Not a valid e-mail address"),
					tr("This is not a valid e-mail address.\n"
							"Please enter a valid e-mail address"),
							QMessageBox::Ok, QMessageBox::Ok);
			//model->setData(index,strValue);
			editor->parentWidget()->setFocus(); // set focus on cell again so user doesn't have to reselect it again for editing
//			this->setEditorData(editor,index);
//			emit openEditor(index, strValue);
		}
		break;
	case TASK_DESCRIPTION:
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		model->setData(index, strValue);
		newValue = strValue;
		change = true;
		break;
		/*
	case PREDECESSOR: // predecessor ID
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		if ((iValidator.validate(strValue,pos) == QValidator::Acceptable) | (strValue == "")) {
			if (model->data(index).toString().compare(strValue)) {
				model->setData(index, QVariant(strValue));
				newValue = model->data(index).toUInt();
				change = true;
			}
		}
		else {
			QMessageBox::warning(editor, tr("Not a valid task ID value"),
					tr("This is not a valid task ID value.\n"
							"Valid task ID values are positive integral numbers greater than 0"),
							QMessageBox::Ok, QMessageBox::Ok);
			editor->parentWidget()->setFocus();
		}
		break;
		*/
	case STATION_ID: // station names
		listWidget = static_cast<QListWidget *>(editor);
		prev_list = model->data(index).toStringList(); // previous list of selected stations
		selectedItems = listWidget->selectedItems();
		if (!selectedItems.empty()) {
			if ((*selectedItems.begin())->text().compare("no stations defined") != 0) {
				for (QList<QListWidgetItem *>::const_iterator qit = selectedItems.begin(); qit != selectedItems.end(); ++qit) {
					stringListItems.append((*qit)->text());
				}
				stringListItems.sort();
				if (selectedItems.size() != prev_list.size()) {
					model->setData(index, stringListItems);
					change = true;
				}
				else { // same number of selected stations, see if they are the same stations
					for (QList<QListWidgetItem *>::const_iterator qit = selectedItems.begin(); qit != selectedItems.end(); ++qit) {
						if (!(prev_list.contains((*qit)->text()))) {
							model->setData(index, stringListItems);
							change = true;
							break;
						}
					}
				}
				if (change) {
					for (QStringList::const_iterator it = stringListItems.begin(); it != stringListItems.end()-1; ++it) {
						strValue += *it + ";";
					}
					strValue += stringListItems.back();
					newValue = strValue;
				}
			}
		}
		else { // no stations selected
			if (prev_list.size() != 0) {
				model->setData(index, QStringList(""));
				change = true;
			}
		}
		break;
	case PREDECESSORS:
	case PRED_MIN_TIME_DIF: // time edit
	case PRED_MAX_TIME_DIF:
	case TASK_DURATION: // duration
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		model->setData(index, strValue);
		newValue = strValue;
		change = true;
		break;
	case STORAGE_SIZE: // storage
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text().trimmed();
		if (model->data(index).toString().compare(strValue)) {
			model->setData(index, QVariant(strValue));
			newValue = model->data(index).toInt();
			change = true;
		}
		break;
	case WINDOW_MINIMUM_TIME:
	case WINDOW_MAXIMUM_TIME:
		timeEdit = static_cast<QTimeEdit *>(editor);
		if (model->data(index).toTime() != timeEdit->time()) {
			model->setData(index, QVariant(timeEdit->time()));
			newValue = timeEdit->text();
			change = true;
		}
		break;
	case PRIORITY: // priority (double)
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue =  lineEdit->text().trimmed();
		dValidator.setRange(0.0,10.0,20);
		if (dValidator.validate(strValue,pos) == QValidator::Acceptable) {
			if (model->data(index).toString().compare(strValue)) {
				model->setData(index, QVariant(strValue));
				newValue = lineEdit->text().toDouble();
				change = true;
			}
		}
		else {
			   QMessageBox::warning(editor, tr("Not a valid priority value"),
			                                  tr("This is not a valid priority value.\n"
			                                     "Valid priority values are positive floating point numbers up to 10.0"),
			                                  QMessageBox::Ok, QMessageBox::Ok);
				editor->parentWidget()->setFocus();
		}
		break;
	case FILTER_TYPE: // filter type
		comboBox = static_cast<QComboBox *>(editor);
		strValue = model->data(index).toString();
		if (strValue.compare(filter_types_str[comboBox->currentIndex()]) != 0) {
			model->setData(index, filter_types_str[comboBox->currentIndex()]);
			newValue = model->data(index).toString();
			change = true;
		}
		break;
	case ANTENNA_MODE: // antenna mode
		comboBox = static_cast<QComboBox *>(editor);
		strValue = model->data(index).toString();
		if (strValue.compare(antenna_modes_str[comboBox->currentIndex()]) != 0) {
			model->setData(index, antenna_modes_str[comboBox->currentIndex()]);
			newValue = model->data(index).toString();
			change = true;
		}
		break;
	case CLOCK_FREQUENCY: // clock frequencies
		comboBox = static_cast<QComboBox *>(editor);
		strValue = model->data(index).toString();
		if (strValue.compare(clock_frequencies_str[comboBox->currentIndex()]) != 0) {
			model->setData(index, clock_frequencies_str[comboBox->currentIndex()]);
			newValue = model->data(index).toString();
			change = true;
		}
		break;
	case FIRST_POSSIBLE_DATE: // date edit
	case LAST_POSSIBLE_DATE:
		dateEdit = static_cast<QDateEdit *>(editor);
		if (model->data(index).toDate() != dateEdit->date()) {
			model->setData(index, QVariant(dateEdit->date()));
			strValue = dateEdit->text();
			newValue = strValue;
			change = true;
		}
		break;
	case PLANNED_START: // date-time edit
	case PLANNED_END:
		dateTimeEdit = static_cast<QDateTimeEdit *>(editor);
		if (model->data(index).toDateTime() != dateTimeEdit->dateTime()) {
			model->setData(index, QVariant(dateTimeEdit->dateTime()));
			newValue = dateTimeEdit->text();
			change = true;
		}
		break;
	case FIXED_DAY: // fixed day and time (checkbox)
	case FIXED_TIME:
		lineEdit = static_cast<QLineEdit *>(editor);
		strValue = lineEdit->text();
		iValidator.setRange(0,1);
		if (iValidator.validate(strValue,pos) == QValidator::Acceptable) {
			if (model->data(index).toString().compare(strValue)) {
				model->setData(index, strValue);
				newValue = model->data(index).toBool();
				change =true;
			}
		}
		else {
			   QMessageBox::warning(editor, tr("Not a valid value"),
			                                  tr("This is not a valid value.\n"
			                                     "Valid values are 1 (true) or 0 (false)"),
			                                  QMessageBox::Ok, QMessageBox::Ok);
		}
		break;
	case TASK_STATUS: // task status
		comboBox = static_cast<QComboBox *>(editor);
		strValue = model->data(index).toString();
		strValue2 = comboBox->currentText();
		if (strValue.compare(strValue2) != 0) { // if the status was changed
			model->setData(index, strValue2);
			newValue = strValue2;
			change = true;
		}
//		task_type = index.model()->data(index.model()->index(index.row(),TASK_TYPE),USERDATA_ROLE).toInt();
//		if ((task_type == Task::RESERVATION) | (task_type == Task::MAINTENANCE)) {
//			if (comboBox->currentIndex() == 0) {
//				if (strValue.compare(task_states_str[Task::SCHEDULED]) != 0) { // did the status change?
//					model->setData(index, QVariant(QString(task_states_str[Task::SCHEDULED])));
//					newValue = QString(task_states_str[Task::SCHEDULED]);
//					change = true;
//				}
//			}
//			else {
//				if (strValue.compare(task_states_str[Task::ON_HOLD]) != 0) { // did the status change?
//					model->setData(index, QVariant(QString(task_states_str[Task::ON_HOLD])));
//					newValue = QString(task_states_str[Task::ON_HOLD]);
//					change = true;
//				}
//			}
//		}
//		else {
//			if (strValue.compare(task_states_str[comboBox->currentIndex()]) != 0) {
//				model->setData(index, QVariant(task_states_str[comboBox->currentIndex()]));
//				newValue = model->data(index).toString();
//				change = true;
//			}
//		}
		break;
	case NIGHT_TIME_WEIGHT_FACTOR: // night time weight factor (spinbox)
//	case CEP_PROCESSING_UNITS: // CEP  processing %
//	case OFFLINE_PROCESSING_UNITS: // offline processing %
		spinBox = static_cast<QSpinBox *>(editor);
		if (model->data(index).toInt() != spinBox->value()) {
			model->setData(index, QVariant(spinBox->value()));
			newValue = model->data(index).toUInt();
			change = true;
		}
		break;
	}
	if (change) {
		emit tableItemChanged(taskID, static_cast<data_headers>(index.column()), newValue, index); // sends the new value
	}
}

void ScheduleTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    drawText(painter, option, index);
    if (index.column() == PLANNED_END && !index.model()->data(index, Qt::ToolTipRole).toString().isEmpty()) {
        QPolygon polygonTriangle(3);
        polygonTriangle.setPoint(0, QPoint(option.rect.x()+8, option.rect.y()));
        polygonTriangle.setPoint(1, QPoint(option.rect.x(), option.rect.y()));
        polygonTriangle.setPoint(2, QPoint(option.rect.x(), option.rect.y()+8));
        painter->save();
        QColor color((Qt::GlobalColor)index.model()->data(index, Qt::UserRole+1).toInt());
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QBrush(color));
        painter->setPen(QPen(color));
        painter->drawPolygon(polygonTriangle);
        painter->restore();
    }
}

void ScheduleTableDelegate::writeTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QString str = index.model()->data(index).toString();
	if (str.compare("::") == 0) { // if empty string specified (delete)
		str = "";
	}
	int m = str.indexOf(':'); // first ':'
	if (m == 0) { // if ':' is first character
		str = "0" + str;
	}
	else { // trim first zeros except last one before ':'
		for (short n=0; n < m-1; ++n) {
			if (str[n] == '0') {
				str = str.right(str.length()-1);
				--n;
				--m;
			}
			else break;
		}
	}
	// minutes cannot be left empty ::
	if (str.contains("::")) {
		str.insert(str.indexOf(':')+1,'0'); // insert a zero for the minutes
	}
	// take care of seconds part
	if ((str.length() > 1)) {
		if (str.right(2) == "::") {
			str = str.left(str.length()-1) + "0:0";
		}
		else if (str.right(1) == ":") {
			str = str + '0';
		}
	}
	QRect rect_inner = option.rect;
	rect_inner.adjust(5,5,-5,-5);
	painter->drawText(rect_inner , Qt::AlignLeft | Qt::AlignVCenter, str);
}

void ScheduleTableDelegate::writeStations(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QString str;
	QStringList stringItems = index.model()->data(index).toStringList();
	if (!(stringItems.empty())) {
		for (QStringList::const_iterator it = stringItems.begin(); it != stringItems.end()-1; ++it) {
			str += *it;
			str += ";";
		}
		str += *(stringItems.end()-1);
		QRect rect_inner = option.rect;
		rect_inner.adjust(5,5,-5,-5);
		painter->drawText(rect_inner , Qt::AlignLeft | Qt::AlignVCenter, str);
	}
}


void ScheduleTableDelegate::drawText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QRect rect_text = option.rect;
	rect_text.adjust(5,0,0,0); // make sure to keep a left text margin in the cell of 5 pixels
	if (option.state & QStyle::State_Selected) {
		painter->setPen(Qt::SolidLine);
		QRect rect_inner = option.rect;
		rect_inner.adjust(2,2,-2,-2);
		painter->fillRect(rect_inner, QBrush(0x005577aa/*Qt::blue*/));
		painter->setPen(Qt::white);
	}
	else {
		painter->setPen(index.model()->data(index,Qt::ForegroundRole).toString());
		painter->fillRect(option.rect, QBrush(index.model()->data(index,Qt::BackgroundRole).toString(),Qt::SolidPattern));
	}
	// now draw the text
	int task_type = index.model()->data(index.model()->index(index.row(),TASK_TYPE),USERDATA_ROLE).toInt();
	if (task_type == Task::RESERVATION) {
		switch (index.column()) {
		case PREDECESSORS:
		case PRIORITY:
		case PRED_MIN_TIME_DIF:
		case PRED_MAX_TIME_DIF:
		case FIRST_POSSIBLE_DATE:
		case LAST_POSSIBLE_DATE:
		case WINDOW_MINIMUM_TIME:
		case WINDOW_MAXIMUM_TIME:
//		case FIXED_DAY:
//		case FIXED_TIME:
		case NR_OF_SUBBANDS:
		case NIGHT_TIME_WEIGHT_FACTOR:
//		case STORAGE_SIZE:
			// don't write these for reservations or maintenance tasks
			break;
		case TASK_DURATION:
			writeTime(painter, option, index);
			break;
		case PLANNED_START:
		case PLANNED_END:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			break;
		case STATION_ID:
			writeStations(painter, option, index);
			break;
		case UNSCHEDULED_REASON:
			painter->setPen(Qt::black);
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
			break;
		default:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
			break;
		}
	}
	else if (task_type == Task::MAINTENANCE) {
		switch (index.column()) {
		case PREDECESSORS:
		case PRED_MIN_TIME_DIF:
		case PRED_MAX_TIME_DIF:
		case PRIORITY:
		case ANTENNA_MODE:
		case CLOCK_FREQUENCY:
		case FILTER_TYPE:
		case FIRST_POSSIBLE_DATE:
		case LAST_POSSIBLE_DATE:
		case WINDOW_MINIMUM_TIME:
		case WINDOW_MAXIMUM_TIME:
//		case FIXED_DAY:
//		case FIXED_TIME:
		case NR_OF_SUBBANDS:
		case NIGHT_TIME_WEIGHT_FACTOR:
		case STORAGE_SIZE:
			// don't write these for maintenance tasks
			break;
		case TASK_DURATION:
			writeTime(painter, option, index);
			break;
		case PLANNED_START:
		case PLANNED_END:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			break;
		case STATION_ID:
			writeStations(painter, option, index);
			break;
		case UNSCHEDULED_REASON:
			painter->setPen(Qt::black);
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
			break;
		default:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
			break;
		}
	}
	else if (task_type == Task::PIPELINE) {
		switch (index.column()) {
		case ANTENNA_MODE:
		case CLOCK_FREQUENCY:
		case FILTER_TYPE:
		case FIRST_POSSIBLE_DATE:
		case LAST_POSSIBLE_DATE:
		case WINDOW_MINIMUM_TIME:
		case WINDOW_MAXIMUM_TIME:
		case FIXED_DAY:
		case FIXED_TIME:
		case NR_OF_SUBBANDS:
		case NIGHT_TIME_WEIGHT_FACTOR:
		case STATION_ID:
//		case STORAGE_SIZE:
			// don't write these for maintenance tasks
			break;
		case TASK_DURATION:
			writeTime(painter, option, index);
			break;
		case PLANNED_START:
		case PLANNED_END:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			break;
		default:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
			break;
		}
	}
	else { // regular tasks
		switch (index.column()) {
		case PRED_MIN_TIME_DIF:
		case PRED_MAX_TIME_DIF:
		case TASK_DURATION:
			writeTime(painter, option, index);
			break;
		case FIRST_POSSIBLE_DATE:
		case LAST_POSSIBLE_DATE:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toDate().toString("yyyy-MM-dd"));
			break;
		case PLANNED_START:
		case PLANNED_END:
			painter->drawText(rect_text, Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			break;
		case STATION_ID:
			writeStations(painter, option, index);
			break;
		default:
			painter->drawText(rect_text,Qt::AlignLeft | Qt::AlignVCenter, index.model()->data(index).toString());
			break;
		}
	}
}

void ScheduleTableDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
