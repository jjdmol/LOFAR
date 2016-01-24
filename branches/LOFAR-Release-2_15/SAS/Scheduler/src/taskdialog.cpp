/*
 * taskdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Nov 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/taskdialog.cpp $
 *
 */

#include "taskdialog.h"
#include "task.h"
#include "lofar_utils.h"
#include "lofar_scheduler.h"
#include "astrodatetime.h"
#include "DateEdit.h"
#include "DateTimeEdit.h"
#include "TimeEdit.h"
#include "LineEdit.h"
#include "CheckBox.h"
#include "SpinBox.h"
#include "doublespinbox.h"
#include "ListWidget.h"
#include "ComboBox.h"
#include "Controller.h"
#include "station.h"
#include "stationtreewidget.h"
#include "stationlistwidget.h"
#include "demixingsettings.h"
#include <QSpacerItem>
#include <QLabel>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QGraphicsOpacityEffect>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

#define STORAGE_NODE_ID_ROLE 200
#define STORAGE_RAID_ID_ROLE 300
#define NO_WARNING true

const char *tab_names[NR_TABS] = {"Schedule","Station settings","Station beams","Processing","Storage","Imaging Pipelines","Pulsar Pipeline","Long-Baseline Pipeline","Extra info"};

TaskDialog::TaskDialog(QWidget *parentGUI, Controller *controller)
    : QDialog(parentGUI), itsController(controller), itsTask(0), itsStorageOverflow(false), changeSchedule(false),
      changeStations(false), changeBeams(false), changeProcessing(false),
      changeStorage(false), changePipeline(false), changePulsar(false), changeLongBaseline(false),
      changeExtraInfo(false), changeEnabledInputFiles(false), itsStationsLoaded(false), addingTask(false), addingPipeline(false),
      addingReservation(false), blockChangeDetection(true)
{
    this->blockSignals(true); // to avoid early changes
    ui.setupUi(this);

	itsOutputDataTypes.correlated = false;
	itsOutputDataTypes.coherentStokes = false;
	itsOutputDataTypes.incoherentStokes = false;
	itsOutputDataTypes.instrumentModel = false;
	itsOutputDataTypes.skyImage = false;

    ui.comboBoxProcessType->blockSignals(true);
    for (short i = 0; i < NR_TASK_TYPES; ++i) {
        ui.comboBoxProcessType->addItem(task_types_str[i]);
    }
    ui.comboBoxProcessType->blockSignals(false);

    ui.lineEditDuration->setDefaultText("00:00:00");
    ui.lineEditPriority->setDefaultText("1.0");

    const AstroDate &earliest(Controller::theSchedulerSettings.getEarliestSchedulingDay());
    QDate earliestDate(earliest.getYear(), earliest.getMonth(), earliest.getDay());
    ui.dateTimeEditScheduledStart->setMinimumDate(earliestDate);
    ui.dateTimeEditScheduledStart->setDefaultDateTime(earliest);
    ui.dateTimeEditScheduledEnd->setMinimumDate(earliestDate);
    ui.dateTimeEditScheduledEnd->setDefaultDateTime(Controller::theSchedulerSettings.getLatestSchedulingDay());

    // ************** END frameScheduleGeneral ******************************************************************************

    ui.spinBoxDataslotsPerRSPboard->setMinimum(1);
    ui.spinBoxDataslotsPerRSPboard->setMaximum(MAX_DATASLOT_PER_RSP_16_BITS + 1);

    // Station beams tab

	QStringList items;
	// analog beam direction types
	items.clear();
	for (short int i=0; i < DIR_TYPE_UNDEFINED ; ++i) {
		items << BEAM_DIRECTION_TYPES[i];
	}
    ui.comboBoxAnalogBeamCoordinates->addItems(items);

	// analog beam units (defaults to J2000 coordinates (HMS,DMS)
	items.clear();
	for (short int i=0; i < END_ANGLE_PAIRS; ++i) {
		items << ANGLE_PAIRS[i];
	}
    ui.comboBoxAnalogBeamUnits->addItems(items);

    // fill antenna modes comboBox
    items.clear();
	for (short int i=0; i < NR_ANTENNA_MODES ; ++i) {
		items << antenna_modes_str[i];
	}
    ui.comboBoxStationAntennaMode->blockSignals(true);
    ui.comboBoxStationAntennaMode->addItems(items);
    ui.comboBoxStationAntennaMode->blockSignals(false);

    // fill station filter types comboBox
    items.clear();
	for (short int i=0; i < NR_FILTER_TYPES ; ++i) {
		items << filter_types_str[i];
	}
    ui.comboBoxStationFilter->blockSignals(true);
    ui.comboBoxStationFilter->addItems(items);
    ui.comboBoxStationFilter->blockSignals(false);

    // fill clock frequencies combobox
    items.clear();
	for (short int i=0; i < NR_CLOCK_FREQUENCIES; ++i) {
		items << clock_frequencies_str[i];
	}
    ui.comboBoxStationClock->blockSignals(true);
    ui.comboBoxStationClock->addItems(items);
    ui.comboBoxStationClock->blockSignals(false);

	// storage node selection method combo box
	items.clear();
	for (short int i=0; i < NR_STORAGE_SELECTION_MODES; ++i) {
		items << storage_select_mode_str[i];
	}
    ui.comboBoxStorageSelectionMode->blockSignals(true);
    ui.comboBoxStorageSelectionMode->addItems(items);
    ui.comboBoxStorageSelectionMode->blockSignals(false);


	// digital beam table header
	ui.tableWidgetDigitalBeams->setColumnCount(11);
	QStringList header;
	header << "Target" << "Coord." << "Units" << "Angle 1" << "Angle 2"
		   << "subband list" << "# subbands" << "start time" << "duration" << "# TAB rings" << "TAB ring size";
	ui.tableWidgetDigitalBeams->setHorizontalHeaderLabels(header);

	ui.tableWidgetDigitalBeams->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableWidgetDigitalBeams, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(displayDigitalBeamContextMenu(const QPoint &)));


	// Tied array beams table
	ui.tableWidgetTiedArrayBeams->setColumnCount(4);
	ui.tableWidgetTiedArrayBeams->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetTiedArrayBeams->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.tableWidgetTiedArrayBeams->setEditTriggers(QAbstractItemView::NoEditTriggers);
	header.clear();
	header << "Angle 1 (rad)" << "Angle 2 (rad)" << "Type" << "Dispersion measure";
	ui.tableWidgetTiedArrayBeams->setHorizontalHeaderLabels(header);
	ui.tableWidgetTiedArrayBeams->setColumnWidth(0,150);
	ui.tableWidgetTiedArrayBeams->setColumnWidth(1,150);
	ui.tableWidgetTiedArrayBeams->setColumnWidth(2,130);
	ui.tableWidgetTiedArrayBeams->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetTiedArrayBeams->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

	// enable default output data type
	ui.checkBoxCorrelatedData->blockSignals(true);
	ui.checkBoxCorrelatedData->setChecked(true);
	ui.checkBoxCorrelatedData->blockSignals(false);

	// storage nodes tree header
	header.clear();
	ui.treeWidgetStorageNodes->setColumnCount(4);
	header << "Storage node" << "Partition" << "Total Size" << "Free Space";
	ui.treeWidgetStorageNodes->setHeaderLabels(header);
	ui.treeWidgetStorageNodes->header()->resizeSection(0, 150);
	ui.treeWidgetStorageNodes->header()->resizeSection(1, 70);
	ui.treeWidgetStorageNodes->header()->resizeSection(2, 55);
	ui.treeWidgetStorageNodes->header()->resizeSection(3, 55);
	ui.treeWidgetStorageNodes->setSelectionMode(QAbstractItemView::ExtendedSelection);
	itsActionStorageOverride = new QAction("&Manual override", ui.treeWidgetStorageNodes);
	itsActionStorageCheckSelected = new QAction("&Check selected", ui.treeWidgetStorageNodes);
	itsActionStorageUncheckSelected = new QAction("&Uncheck selected", ui.treeWidgetStorageNodes);
	connect(itsActionStorageCheckSelected, SIGNAL(triggered()), this, SLOT(doCheckSelectedStorage()));
	connect(itsActionStorageUncheckSelected, SIGNAL(triggered()), this, SLOT(doUnCheckSelectedStorage()));
    connect(ui.treeWidgetStorageNodes, SIGNAL(clicked(QModelIndex)), this, SLOT(detectStorageLocationChanges(void)));
	connect(ui.treeWidgetInputDataProducts, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(detectInputFilesEnabledChanges(void)));
	ui.treeWidgetStorageNodes->addAction(itsActionStorageCheckSelected);
	ui.treeWidgetStorageNodes->addAction(itsActionStorageUncheckSelected);
	ui.treeWidgetStorageNodes->setContextMenuPolicy(Qt::ActionsContextMenu);

	// dataproducts tree header
	ui.treeWidgetOutputDataProducts->setColumnCount(1);
	ui.treeWidgetOutputDataProducts->header()->hide();
	ui.treeWidgetInputDataProducts->setColumnCount(1);
	ui.treeWidgetInputDataProducts->header()->hide();

	// Stokes types
	items.clear();
	for (short int i = 0; i < DATA_TYPE_XXYY; ++i) {
		items << DATA_TYPES[i];
	}
    ui.comboBoxIncoherentStokesType->addItems(items);

	items << DATA_TYPES[DATA_TYPE_XXYY]; // add XXYY to the coherent stokes type (i.e. for Complex Voltages)
    ui.comboBoxCoherentStokesType->addItems(items);

	// bits per sample
	items.clear();
	items /*<< "2"*/ << "4" << "8" << "16";
    ui.comboBoxBitsPerSample->addItems(items);

	QPalette palet = ( ui.lineEditStorageConflict->palette() );
	palet.setColor( QPalette::Base, Qt::red );
	palet.setColor( QPalette::Text, Qt::white );
	ui.lineEditStorageConflict->setPalette(palet);
	ui.lineEditStorageConflict->hide();

    enableFOVedit(1);

	// connect signals and slots
    connect(ui.comboBoxTaskStatus, SIGNAL(currentIndexChanged(int)), this, SLOT(statusChanged()));
    connect(ui.lineEditAnalogBeamAngle1, SIGNAL(editingFinished()), this, SLOT(setAnalogBeamAngle1()));
    connect(ui.lineEditAnalogBeamAngle2, SIGNAL(editingFinished()), this, SLOT(setAnalogBeamAngle2()));
	connect(ui.pushButtonAddSuperStation, SIGNAL(clicked()), this, SLOT(addSuperStation()));
    connect(ui.treeWidgetUsedStations, SIGNAL(stationsDeleted(const QStringList &)), this, SLOT(addAvailableStations(const QStringList &)));
    connect(ui.treeWidgetUsedStations, SIGNAL(checkForStationChanges()), this, SLOT(checkIfStationChanged()));
    connect(ui.listWidgetAvailableStations, SIGNAL(stationsRemoved()), this, SLOT(applyStationsRemoved()));
    connect(ui.listWidgetAvailableStations, SIGNAL(addStationsToUse(const QStringList &)), this, SLOT(addStationsToUsedStations(const QStringList &)));
	connect(ui.pushButtonAddBeam, SIGNAL(clicked()), this, SLOT(addDigitalBeam(void)));
	connect(ui.pushButtonDeleteBeams, SIGNAL(clicked()), this, SLOT(deleteDigitalBeam(void)));
	connect(ui.pushButtonEditBeam, SIGNAL(clicked()), this, SLOT(editDigitalBeam(void)));
	connect(ui.pushButtonClearAllBeams, SIGNAL(clicked()), this, SLOT(clearAllDigitalBeams(void)));
	connect(ui.tableWidgetDigitalBeams, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(editDigitalBeam(void)));
	connect(ui.tableWidgetDigitalBeams, SIGNAL(cellClicked(int,int)), this, SLOT(showTiedArrayBeams(int)));
	connect(ui.tableWidgetTiedArrayBeams, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(editTiedArrayBeam(void)));
	connect(ui.pushButtonAddTiedArrayBeam, SIGNAL(clicked()), this, SLOT(addTiedArrayBeam(void)));
	connect(ui.pushButtonDeleteTiedArrayBeam, SIGNAL(clicked()), this, SLOT(deleteTiedArrayBeam(void)));
	connect(ui.pushButtonEditTiedArrayBeam, SIGNAL(clicked()), this, SLOT(editTiedArrayBeam(void)));
	connect(ui.pushButtonClearAllTiedArrayBeam, SIGNAL(clicked()), this, SLOT(clearAllTiedArrayBeams(void)));

	connect(ui.tabWidgetMain, SIGNAL(currentChanged(int)), this, SLOT(doTabChangeUpdate(int)));
	connect(ui.pushButtonUpdateCampaigns, SIGNAL(clicked()), this, SLOT(updateProjects()));
    connect(ui.lineEditAnalogBeamDuration, SIGNAL(textEdited(const QString &)), this, SLOT(analogBeamDurationChanged(const QString &)));
    connect(ui.comboBoxAnalogBeamCoordinates, SIGNAL(currentIndexChanged(int)), this, SLOT(AnalogBeamDirectionTypeChanged(void)));
    connect(ui.comboBoxAnalogBeamUnits, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(AnalogBeamAngleUnitChanged(const QString &)));
    connect(ui.comboBoxProjectID, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(applyProjectChange(const QString &)));
    connect(ui.comboBoxStationAntennaMode, SIGNAL(currentIndexChanged(int)), this, SLOT(AntennaModeChanged(int)));
    connect(ui.comboBoxStationFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(FilterTypeChanged(int)));
    connect(ui.comboBoxStationClock, SIGNAL(currentIndexChanged(int)), this, SLOT(StationClockModeChanged(int)));
    connect(ui.comboBoxStorageSelectionMode, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStorageSelectionMode(int)));

    connect(ui.checkBoxSpecifyFOV,SIGNAL(stateChanged(int)), this, SLOT(enableFOVedit(int)));
    connect(ui.pushButtonClearDemixAlways, SIGNAL(clicked()), this, SLOT(clearDemixAlways()));
    connect(ui.pushButtonClearDemixIfNeeded, SIGNAL(clicked()), this, SLOT(clearDemixIfNeeded()));
    connect(ui.comboBoxBitsPerSample, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMaxDataslotsPerRSP()));
    connect(ui.checkBoxCorrelatedData, SIGNAL(stateChanged(int)), this, SLOT(outputDataTypesChanged()));
    connect(ui.checkBoxCoherentStokes, SIGNAL(stateChanged(int)), this, SLOT(outputDataTypesChanged()));
    connect(ui.checkBoxIncoherentStokes, SIGNAL(stateChanged(int)), this, SLOT(outputDataTypesChanged()));
    connect(ui.comboBoxCoherentStokesType, SIGNAL(currentIndexChanged(int)), this, SLOT(outputDataTypesChanged()));
    connect(ui.comboBoxIncoherentStokesType, SIGNAL(currentIndexChanged(int)), this, SLOT(outputDataTypesChanged()));

	// set the default button
	ui.pushButtonOk->setDefault(true);
	this->blockSignals(false); // to avoid early changes

	itsDigitalBeamDialog = new DigitalBeamDialog(this);
	itsTiedArrayBeamDialog= new TiedArrayBeamDialog(this);
}

TaskDialog::~TaskDialog()
{
    delete itsTask;
	delete itsDigitalBeamDialog;
	delete itsTiedArrayBeamDialog;
}

void TaskDialog::statusChanged(void) {
    Task::task_status state(taskStatusFromString(ui.comboBoxTaskStatus->currentText().toStdString()));
    if (state <= Task::PRESCHEDULED) setNormalTaskMode(); // again enables all widgets that might have been disabled
    else if (state == Task::SCHEDULED) setScheduledTaskMode();
    else if (state > Task::SCHEDULED) setFinishedTaskMode();
    detectChanges();
}

void TaskDialog::enableFOVedit(int state) {
    ui.lineEditFieldOfView->setEnabled(state);
	ui.labelFOV->setEnabled(state);
    ui.lineEditCellSize->setEnabled(!state);
	ui.labelCellSize->setEnabled(!state);
    ui.spinBoxNumberOfPixels->setEnabled(!state);
	ui.labelNpix->setEnabled(!state);
	detectChanges();
}

void TaskDialog::strategyChanged(void) {
	updateOriginalTreeID();
	detectChanges();
}

void TaskDialog::updateOriginalTreeID(void) {
    ui.lineEditOriginalTreeID->setText(QString::number(Controller::theSchedulerSettings.getSASDefaultTreeID(ui.comboBoxProcessType->currentText(), ui.comboBoxProcessSubType->currentText(), ui.comboBoxStrategies->currentText())));
}

void TaskDialog::loadProcessTypes(void) {
    ui.comboBoxProcessType->blockSignals(true);
    ui.comboBoxProcessSubType->blockSignals(true);
	const QStringList &processTypes(Controller::theSchedulerSettings.getAllProcessTypes());
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.comboBoxProcessType->model());
	if (model) {
		QStandardItem* item;
		// check if OBSERVATION is in processTypes and set it as the default
		item = model->item(0);
        if (processTypes.indexOf(task_types_str[Task::OBSERVATION]) == -1) { // check if a default template of type OBSERVATION exists
			// disable OBSERVATION process type in combobox
			item->setEnabled(false);
			item->setToolTip("No OBSERVATION default template found in SAS database.\nCannot add OBSERVATION tasks.");
            processTypeChanged(ui.comboBoxProcessType->currentIndex());
		}
		else {
			item->setEnabled(true);
			item->setToolTip("");
            ui.comboBoxProcessType->setCurrentIndex(0);
			// fetch processSubTypes for processType = OBSERVATION
            updateProcessSubtypes(task_types_str[Task::OBSERVATION]);
//            processTypeChanged(0);
			// set the default processSubtype to BEAM OBSERVATION if it exists
            int idx(ui.comboBoxProcessSubType->findText("BEAM OBSERVATION"));
			if (idx != -1) {
                ui.comboBoxProcessSubType->setCurrentIndex(idx);
			}
		}

		item = model->item(1);
		if (processTypes.indexOf("RESERVATION") == -1) { // check if RESERVATION is in processTypes if not disable it
			item->setEnabled(false);
			item->setToolTip("No RESERVATION default templates found in SAS database.\nCannot add RESERVATION tasks.");
		}
		else {
			item->setEnabled(true);
			item->setToolTip("");
		}

		item = model->item(2);
		if (processTypes.indexOf("MAINTENANCE") == -1) { // check if MAINTENANCE is in processTypes if not disable it
			item->setEnabled(false);
			item->setToolTip("No MAINTENANCE default templates found in SAS database.\nCannot add MAINTENANCE tasks.");
		}
		else {
			item->setEnabled(true);
			item->setToolTip("");
		}

		item = model->item(3);
		if (processTypes.indexOf("PIPELINE") == -1) { // check if PIPELINE is in processTypes if not disable it
			item->setEnabled(false);
			item->setToolTip("No PIPELINE default templates found in SAS database.\nCannot add PIPELINE tasks.");
		}
		else {
			item->setEnabled(true);
			item->setToolTip("");
		}

		item = model->item(4);
		if (processTypes.indexOf("SYSTEM") == -1) { // check if PIPELINE is in processTypes if not disable it
			item->setEnabled(false);
			item->setToolTip("No SYSTEM default templates found in SAS database.\nCannot add SYSTEM tasks.");
		}
		else {
			item->setEnabled(true);
			item->setToolTip("");
		}
	}
    ui.comboBoxProcessType->setEnabled(true);
    ui.comboBoxProcessSubType->setEnabled(true);
    ui.comboBoxStrategies->setEnabled(true);
    ui.comboBoxProcessType->setToolTip("");
    ui.comboBoxProcessSubType->setToolTip("");
    ui.comboBoxStrategies->setToolTip("");

    ui.comboBoxProcessType->blockSignals(false);
    ui.comboBoxProcessSubType->blockSignals(false);
}

void TaskDialog::setProcessSubProcessStrategy(const Task *pTask) {
    ui.comboBoxProcessType->blockSignals(true);
    ui.comboBoxProcessSubType->blockSignals(true);
    ui.comboBoxStrategies->blockSignals(true);
	// set processType according to task type
	Task::task_type type = pTask->getType();
    ui.comboBoxProcessType->setCurrentIndex(type);

	QString pstype(pTask->getProcessSubtypeStr());
	const QString &ptype(pTask->getProcessType()), strategy(pTask->getStrategy());

	// set process subtype
    ui.comboBoxProcessSubType->clear();
    ui.comboBoxProcessSubType->addItems(Controller::theSchedulerSettings.getAllProcessSubTypes(ptype));
    int idx(ui.comboBoxProcessSubType->findText(pstype));
	if (idx != -1) {
        ui.comboBoxProcessSubType->setCurrentIndex(idx);
	}
	else {
        ui.comboBoxProcessSubType->addItem(pstype);
        ui.comboBoxProcessSubType->setCurrentIndex(ui.comboBoxProcessSubType->count()-1);
	}

	// set strategy
    ui.comboBoxStrategies->clear();
    ui.comboBoxStrategies->addItems(Controller::theSchedulerSettings.getAllStrategies(ptype, pstype));
    idx = ui.comboBoxStrategies->findText(strategy);
	if (idx != -1) {
        ui.comboBoxStrategies->setCurrentIndex(idx);
	}
	else {
        ui.comboBoxStrategies->addItem(strategy);
        ui.comboBoxStrategies->setCurrentIndex(ui.comboBoxStrategies->count()-1);
	}

    ui.comboBoxProcessType->blockSignals(false);
    ui.comboBoxProcessSubType->blockSignals(false);
    ui.comboBoxStrategies->blockSignals(false);
}

void TaskDialog::processTypeChanged(int ptype) {
    ui.comboBoxProcessSubType->blockSignals(true);
    ui.comboBoxProcessSubType->clear();
    ui.comboBoxProcessSubType->addItems(Controller::theSchedulerSettings.getAllProcessSubTypes(task_types_str[ptype]));
    updateStrategiesComboBox();
    ui.comboBoxProcessSubType->blockSignals(false);

    enableApplyButtons(true);
    changeSchedule = true;

    int task_id(itsTask->getID());
    Task::task_type type = static_cast<Task::task_type>(ptype);
    switch (type) {
    case Task::OBSERVATION:
    default:
        delete itsTask;
        itsTask = new Observation(task_id);
        setNormalTaskMode();
        break;
    case Task::RESERVATION:
    case Task::MAINTENANCE:
        delete itsTask;
        itsTask = new StationTask(task_id, type);
        if (addingTask) {
            addingReservation = true;
        }
        setReservationTaskMode();
        if (!itsStationsLoaded) {
            loadAvailableStations();
        }
    case Task::PIPELINE:
        if (addingTask) {
            addingPipeline = true;
            this->setWindowTitle(QString("Add pipeline ") + QString::number(task_id));
            setPipelineType();
        }
        break;
    case Task::SYSTEM:
        delete itsTask;
        itsTask = new Task(task_id);
        setWindowTitle("Add System task");
        break;
    }
    enableTabs();
    detectChanges();
}

void TaskDialog::updateProcessSubtypes(const QString &processType) {
    ui.comboBoxProcessSubType->blockSignals(true);
    ui.comboBoxProcessSubType->clear();
    ui.comboBoxProcessSubType->addItems(Controller::theSchedulerSettings.getAllProcessSubTypes(processType));
    ui.comboBoxProcessSubType->setCurrentIndex(0);
    updateStrategiesComboBox();
    ui.comboBoxProcessSubType->blockSignals(false);
}

void TaskDialog::updateStrategiesComboBox(void) {
    ui.comboBoxStrategies->blockSignals(true);
    ui.comboBoxStrategies->clear();
    ui.comboBoxStrategies->addItems(Controller::theSchedulerSettings.getAllStrategies(ui.comboBoxProcessType->currentText(), ui.comboBoxProcessSubType->currentText()));
	updateOriginalTreeID();
    setPipelineType(); // if this is a pipeline
    ui.comboBoxStrategies->blockSignals(false);
}


void TaskDialog::setPipelineProperties(void) {
    if (itsTask->isPipeline()) {
        const CalibrationPipeline *calpipe(dynamic_cast<const CalibrationPipeline *>(itsTask));
        const ImagingPipeline *impipe(0);
        const PulsarPipeline *pulsepipe(0);
        const LongBaselinePipeline *lbpipe(0);

        if (calpipe) {
            setDemixSettings(calpipe->demixingSettings());
            setCalibrationPipelineSettings(calpipe);
            disableImagingPipelineSettings();
        }
        else if ((impipe = dynamic_cast<const ImagingPipeline *>(itsTask))) {
            setImagingPipelineSettings(impipe);
            disableCalibrationPipelineSettings();
            disableDemixSettings();
        }
        else if ((pulsepipe = dynamic_cast<const PulsarPipeline *>(itsTask))) {
            setPulsarPipelineSettings(pulsepipe);
        }
        else if ((lbpipe = dynamic_cast<const LongBaselinePipeline *>(itsTask))) {
            setLongBaselinePipelineSettings(lbpipe);
        }
        else {
            disableDemixSettings();
            disableCalibrationPipelineSettings();
            disableImagingPipelineSettings();
        }

        setInputDataProductsTree(*itsTask);
    }
}

void TaskDialog::setPipelineType(void) {
    if (ui.comboBoxProcessType->currentIndex() == Task::PIPELINE) {
        int task_id(itsTask->getID());
        delete itsTask;
        const QString &pst(ui.comboBoxProcessSubType->currentText());
        if (pst == PROCESS_SUBTYPES[PST_CALIBRATION_PIPELINE] || pst == PROCESS_SUBTYPES[PST_AVERAGING_PIPELINE]) {
            itsTask = new CalibrationPipeline(task_id);
//            setDemixSettings(static_cast<CalibrationPipeline *>(itsTask)->demixingSettings());
        }
        else if (pst == PROCESS_SUBTYPES[PST_PULSAR_PIPELINE]) {
            itsTask = new PulsarPipeline(task_id);
        }
        else if (pst == PROCESS_SUBTYPES[PST_IMAGING_PIPELINE] || pst == PROCESS_SUBTYPES[PST_MSSS_IMAGING_PIPELINE]) {
            itsTask = new ImagingPipeline(task_id);
//            setImagingPipelineSettings(static_cast<ImagingPipeline *>(itsTask));
        }
        else if (pst == PROCESS_SUBTYPES[PST_LONG_BASELINE_PIPELINE]) {
            itsTask = new LongBaselinePipeline(task_id);
        }
        else {
            itsTask = new Pipeline(task_id);
        }
//        setPipelineProperties();
        enableTabs();
    }
}

void TaskDialog::enableApplyButtons(bool enable) {
	if (enable) {
		ui.pushButtonCancelClose->setText("Cancel");
		ui.pushButtonApply->setEnabled(true);
		ui.pushButtonOk->setEnabled(true);
	}
	else {
		ui.pushButtonCancelClose->setText("Close");
		ui.pushButtonApply->setEnabled(false);
		ui.pushButtonOk->setEnabled(false);
    }
}

void TaskDialog::updateStorageSelectionMode(int newMode) {
	if (isMultiTasks) {
		// check if set to manual if so enable manual override
		if (static_cast<storage_selection_mode>(newMode) == STORAGE_MODE_MANUAL) {
			doManualStorageOverride();
		}
		else {
			setStorageEditable(false);
		}
	}
	else { // single task
		if (static_cast<storage_selection_mode>(newMode) == STORAGE_MODE_MANUAL) {
			setStorageEditable(true);
		}
		else {
			setStorageEditable(false);
		}
	}
	detectChanges();
}

void TaskDialog::displayDigitalBeamContextMenu(const QPoint &pos) {
	if (!isMultiTasks) {
    Task::task_status status = itsTask->getStatus();
		QMenu menu(this);
		QAction *copy_angles(0), *copy_all(0), *reset_duration(0), *action(0), *edit(0), *show(0);
		if (ui.groupBoxAnalogBeam->isEnabled()) {
			if (status <= Task::SCHEDULED) {
				copy_angles = menu.addAction("copy angles to analog beam");
				copy_all = menu.addAction("copy all setting to analog beam");
				reset_duration = menu.addAction("synchronize times with observation");
				edit =  menu.addAction("edit");
			}
			else {
				show =  menu.addAction("show");
			}
		}
		else {
			if (status <= Task::SCHEDULED) {
				reset_duration = menu.addAction("synchronize times with observation");
				edit =  menu.addAction("edit");
			}
			else {
				show =  menu.addAction("show");
			}
		}
		action = menu.exec(QCursor::pos());
		if (action == copy_angles) {
			QTableWidgetItem *item = ui.tableWidgetDigitalBeams->itemAt(pos);
			if (item) {
				std::map<unsigned, DigitalBeam>::const_iterator bit = itsDigitalBeams.find(item->row());
				if (bit != itsDigitalBeams.end()) {
					itsAnalogBeamSettings.angle1.setRadianAngle(bit->second.angle1().radian());
					itsAnalogBeamSettings.angle2.setRadianAngle(bit->second.angle2().radian());
					itsAnalogBeamSettings.directionType = bit->second.directionType();
                    ui.comboBoxAnalogBeamCoordinates->setCurrentIndex(itsAnalogBeamSettings.directionType);
					setComboBoxAnalogBeamUnits();
					itsAnalogBeamAnglePair = bit->second.units();
					setAnalogBeamAnglePair(itsAnalogBeamAnglePair);
					enableApplyButtons(true);
					changeBeams = true;
				}
			}
		}
		else if (action == copy_all) {
			QTableWidgetItem *item = ui.tableWidgetDigitalBeams->itemAt(pos);
			if (item) {
				// copy angles
				std::map<unsigned, DigitalBeam>::const_iterator bit = itsDigitalBeams.find(item->row());
				if (bit != itsDigitalBeams.end()) {
					itsAnalogBeamSettings.angle1.setRadianAngle(bit->second.angle1().radian());
					itsAnalogBeamSettings.angle2.setRadianAngle(bit->second.angle2().radian());
					itsAnalogBeamSettings.directionType = bit->second.directionType();
					itsAnalogBeamSettings.duration = bit->second.duration();
					itsAnalogBeamSettings.startTime = bit->second.startTime();
                    ui.comboBoxAnalogBeamCoordinates->setCurrentIndex(itsAnalogBeamSettings.directionType);
					setComboBoxAnalogBeamUnits();
					itsAnalogBeamAnglePair = bit->second.units();
					setAnalogBeamAnglePair(itsAnalogBeamAnglePair);
					ui.timeEditAnalogBeamStartTime->setTime(QTime(itsAnalogBeamSettings.startTime.getHours(),
							itsAnalogBeamSettings.startTime.getMinutes(),itsAnalogBeamSettings.startTime.getSeconds()));
                    ui.lineEditAnalogBeamDuration->setText(itsAnalogBeamSettings.duration.toString());
					enableApplyButtons(true);
					changeBeams = true;
				}
			}
		}
		else if (action == reset_duration) {
			QTableWidgetItem *item = ui.tableWidgetDigitalBeams->itemAt(pos);
			if (item) {
				std::map<unsigned, DigitalBeam>::iterator bit = itsDigitalBeams.find(item->row());
                bit->second.setDuration(ui.lineEditDuration->text());
				bit->second.zeroStartTime();
				setDigitalBeam(item->row(),bit->second,true);
				changeBeams = true;
			}
		}
		else if (action == edit) {
			QTableWidgetItem *item = ui.tableWidgetDigitalBeams->itemAt(pos);
			if (item) {
				itsDigitalBeamDialog->setReadOnly(false);
				itsDigitalBeamDialog->loadBeamSettings(item->row(), itsDigitalBeams.at(item->row()));
				itsDigitalBeamDialog->exec();
			}
		}
		else if (action == show) {
			QTableWidgetItem *item = ui.tableWidgetDigitalBeams->itemAt(pos);
			if (item) {
				itsDigitalBeamDialog->setReadOnly(true);
				itsDigitalBeamDialog->loadBeamSettings(item->row(), itsDigitalBeams.at(item->row()));
				itsDigitalBeamDialog->exec();
			}
		}
	}
}


void TaskDialog::setInputDataProductsTree(const Task &task) {
    ui.treeWidgetInputDataProducts->clear();
    const TaskStorage *task_storage(task.storage());
    if (task_storage) {
        const std::map<dataProductTypes, TaskStorage::inputDataProduct> &inputData(task_storage->getInputDataProducts());
        QTreeWidgetItem *dataProductTypeItem, *fileItem;
        QList<QTreeWidgetItem *> dataProductTypeList;
        bool inputDisabled;
        for (std::map<dataProductTypes, TaskStorage::inputDataProduct>::const_iterator it = inputData.begin(); it != inputData.end(); ++it) {
            QStringList itemValues;
            itemValues << QString(DATA_PRODUCTS[it->first]) + " data products";
            dataProductTypeItem = new QTreeWidgetItem((QTreeWidget*)0, itemValues);
            dataProductTypeItem->setData(0, Qt::UserRole, static_cast<int>(it->first));
            if (task_storage->isInputDataProduktEnabled(it->first)) {
                dataProductTypeItem->setCheckState(0, Qt::Checked);
                inputDisabled = false;
            }
            else {
                dataProductTypeItem->setCheckState(0, Qt::Unchecked);
                inputDisabled = true;
            }
            unsigned nrFiles(it->second.filenames.size());
            bool useSkip(it->second.skip.size() == nrFiles);
            for (unsigned idx = 0; idx < nrFiles; ++idx) {
                itemValues.clear();
                itemValues << it->second.locations.at(idx) + it->second.filenames.at(idx);
                fileItem = new QTreeWidgetItem(dataProductTypeItem, itemValues);
                fileItem->setData(0, Qt::UserRole, static_cast<int>(it->first)); // data product type
                if (useSkip) {
                    if (!it->second.skip.at(idx)) {
                        fileItem->setCheckState(0,Qt::Checked);
                    }
                    else {
                        fileItem->setCheckState(0,Qt::Unchecked);
                    }
                }
                else {
                    fileItem->setCheckState(0,Qt::Checked);
                }
                fileItem->setDisabled(inputDisabled);
            }
            dataProductTypeList.append(dataProductTypeItem);
        }
        ui.treeWidgetInputDataProducts->insertTopLevelItems(0, dataProductTypeList);
        ui.treeWidgetInputDataProducts->expandAll();
        ui.treeWidgetInputDataProducts->resizeColumnToContents(0);
    }
}

// update the enabled input data product types in itsTask according to the current enabled types in the inputDataTreeWidget
void TaskDialog::updateEnabledInputDataTypes(void) {
    TaskStorage *task_storage(itsTask->storage());
    if (task_storage) {
        QTreeWidgetItem *dpItem(0);
        dataProductTypes dpType;
        for (short i = 0; i < ui.treeWidgetInputDataProducts->topLevelItemCount(); ++i) {
            dpItem = ui.treeWidgetInputDataProducts->topLevelItem(i);
            // which data product type?
            dpType = static_cast<dataProductTypes>(dpItem->data(0, Qt::UserRole).toInt());
            task_storage->setInputDataProductEnabled(dpType, dpItem->checkState(0) == Qt::Checked);
        }
    }
}

// get the enabled input files which are currently selected in the input data product tree
std::map<dataProductTypes, std::vector<bool> > TaskDialog::getInputDataFilesCheckState(void) {
	std::map<dataProductTypes, std::vector<bool> > retMap;
	QTreeWidgetItemIterator it(ui.treeWidgetInputDataProducts, QTreeWidgetItemIterator::NoChildren);
	dataProductTypes type, prevType(DP_UNKNOWN_TYPE);
	std::vector<bool> vec;
	if (*it) {
		prevType = static_cast<dataProductTypes>((*it)->data(0,Qt::UserRole).toInt()); // file data product type
	}
	while (*it) {
		type = static_cast<dataProductTypes>((*it)->data(0,Qt::UserRole).toInt()); // file data product type
		if (prevType != type) {
			retMap[prevType] = vec; // store current data product type items
			vec.clear();
			prevType = type;
		}
		if ((*it)->checkState(0) == Qt::Checked) {
			vec.push_back(false);
		}
		else {
			vec.push_back(true);
		}
		++it;
	}
	if (!vec.empty()) {
		retMap[prevType] = vec; // store last data product type items
	}

	return retMap;
}

void TaskDialog::detectInputFilesEnabledChanges(void) {
    const TaskStorage *task_storage(itsTask->storage());
    changeEnabledInputFiles = false;
    if (task_storage) {
        // detect if an input data product is completely disabled
        QTreeWidgetItem *dpItem(0);
        bool disable(false);
        ui.treeWidgetInputDataProducts->blockSignals(true);
        for (short i = 0; i < ui.treeWidgetInputDataProducts->topLevelItemCount(); ++i) {
            dpItem = ui.treeWidgetInputDataProducts->topLevelItem(i);
            disable = dpItem->checkState(0) == Qt::Checked ? false : true;
            for (int i = 0; i < dpItem->childCount(); ++i) {
                dpItem->child(i)->setDisabled(disable);
            }
            // detect changes
            dataProductTypes dpType = static_cast<dataProductTypes>(dpItem->data(0,Qt::UserRole).toInt());
            if (task_storage->isInputDataProduktEnabled(dpType) == disable) {
                changeEnabledInputFiles = true;
            }
        }
        ui.treeWidgetInputDataProducts->blockSignals(false);

        // detect individual input file enabled/disabled
        if (!changeEnabledInputFiles) {
            std::map<dataProductTypes, std::vector<bool> > files(getInputDataFilesCheckState());
            const std::map<dataProductTypes, TaskStorage::inputDataProduct> &inputDataProducts(task_storage->getInputDataProducts());
            std::map<dataProductTypes, TaskStorage::inputDataProduct>::const_iterator init;
            for (std::map<dataProductTypes, std::vector<bool> >::const_iterator it = files.begin(); it != files.end(); ++it) {
                init = inputDataProducts.find(it->first);
                if (init != inputDataProducts.end()) {
                    if (init->second.skip != it->second) {
                        changeEnabledInputFiles = true;
                        return;
                    }
                }
            }
        }

        enableApplyButtons(ifSettingsChanged());
    }
}


void TaskDialog::generateFileList(const Task *task, bool noWarnings = false) {
    const TaskStorage *task_storage(task->storage());
    ui.treeWidgetOutputDataProducts->clear();

    if (task_storage) {
        const std::map<dataProductTypes, TaskStorage::outputDataProduct> &files = task_storage->getOutputDataProducts();
        QStringList itemValues;
        bool storage_assigned(false);
        if (files.size() > 0) {
            if (!noWarnings) {
                if (task->getSASTreeID() == 0) {
                    QMessageBox::warning(this, tr("New task does not have SAS ID yet"),
                                         tr("This appears to be a new task which does not have an observation ID (SAS ID) yet.\nThe shown locations and filenames will not contain the correct observation IDs"));
                }
            }
            QList<QTreeWidgetItem *> dataProductTypeList;
            QTreeWidgetItem *dataProductTypeItem, *fileItem;
            std::map<dataProductTypes, TaskStorage::outputDataProduct>::const_iterator it = files.begin();
            unsigned nrFiles;
            while (it != files.end()) {
                itemValues.clear();
                nrFiles = it->second.filenames.size();
                if (nrFiles > 0) {
                    storage_assigned = true;
                    itemValues << QString(DATA_PRODUCTS[it->first]) + " data products";
                    dataProductTypeItem = new QTreeWidgetItem((QTreeWidget*)0, itemValues);
                    dataProductTypeList.append(dataProductTypeItem);
                    for (unsigned idx = 0; idx < nrFiles; ++idx) {
                        itemValues.clear();
                        itemValues << it->second.locations.at(idx) + it->second.filenames.at(idx);
                        fileItem = new QTreeWidgetItem(dataProductTypeItem, itemValues);
                        dataProductTypeList.append(fileItem);
                    }
                }
                ++it;
            }
            ui.treeWidgetOutputDataProducts->insertTopLevelItems(0, dataProductTypeList);
            ui.treeWidgetOutputDataProducts->expandAll();
        }
        if (!storage_assigned) {
            if (task->isPipeline() && task->getConflicts().non_integer_nr_output_files) { // TODO: pragmatic, but not really pretty...
                ui.treeWidgetOutputDataProducts->insertTopLevelItem(0,new QTreeWidgetItem(QStringList(TASK_CONFLICTS[CONFLICT_NON_INTEGER_OUTPUT_FILES])));
            }
            else {
                ui.treeWidgetOutputDataProducts->insertTopLevelItem(0,new QTreeWidgetItem(QStringList("No storage assigned yet")));
            }
        }
        ui.treeWidgetOutputDataProducts->resizeColumnToContents(0);
    }
}


void TaskDialog::updateProjects(void) {
	itsController->updateProjects();
}


void TaskDialog::setExistingProjects(const campaignMap &projects) {
	if (!projects.empty()) {
        ui.comboBoxProjectID->blockSignals(true);
        ui.comboBoxProjectID->clear();
		QStringList items;
		int sidx(0);
		bool found(false);
		for (campaignMap::const_iterator cit = projects.begin(); cit != projects.end(); ++cit) {
			items << cit->second.name.c_str();
            if (itsTask && !(found) && (cit->first == itsTask->getProjectID())) {
				found = true;
			}
			else if (!found) ++sidx;
		}
        ui.comboBoxProjectID->addItems(items);
        ui.comboBoxProjectID->setCurrentIndex(sidx);
        ui.comboBoxProjectID->blockSignals(false);
	}
}

dataProductTypes TaskDialog::getSelectedStorageDataProduct(void) {
	QString dpstr(ui.comboBoxStorageDataType->currentText());
	if (dpstr.compare(DATA_PRODUCTS[DP_CORRELATED_UV]) == 0) return DP_CORRELATED_UV;
	else if (dpstr.compare(DATA_PRODUCTS[DP_COHERENT_STOKES]) == 0) return DP_COHERENT_STOKES;
	else if (dpstr.compare(DATA_PRODUCTS[DP_INCOHERENT_STOKES]) == 0) return DP_INCOHERENT_STOKES;
	else if (dpstr.compare(DATA_PRODUCTS[DP_INSTRUMENT_MODEL]) == 0) return DP_INSTRUMENT_MODEL;
    else if (dpstr.compare(DATA_PRODUCTS[DP_PULSAR]) == 0) return DP_PULSAR;
    else if (dpstr.compare(DATA_PRODUCTS[DP_SKY_IMAGE]) == 0) return DP_SKY_IMAGE;
	else return DP_UNKNOWN_TYPE;
}


void TaskDialog::detectStorageLocationChanges(void) {
    const TaskStorage *task_storage(itsTask->storage());
    if (task_storage) {
        getDisplayedStorageLocations(); // get the current choices from the dialog and store in itsTmpStorage
        if (itsTmpStorage != task_storage->getStorageLocations()) {
            enableApplyButtons(true);
            return;
        }
        if (!changeBeams && !changeExtraInfo && !changeProcessing && !changeSchedule && !changeStorage
                && !changeStations && !changeEnabledInputFiles && !changePipeline) { // nothing has changed disable 'apply' buttons
            enableApplyButtons(false);
        }
    }
}

void TaskDialog::updateStorageTab(void) {
	if (isMultiTasks) {
		ui.treeWidgetOutputDataProducts->clear();
		ui.treeWidgetOutputDataProducts->insertTopLevelItem(0,new QTreeWidgetItem(QStringList(MULTIPLE_VALUE_TEXT)));
	}
    else if (itsTask->getStatus() <= Task::PRESCHEDULED) {
		// copy the current dialog settings to a temporary task to be able to calculate storage properties according to latest changes
		bool incomplete(false);
        Task *tmpTask = cloneTask(itsTask);
        tmpTask->setType(static_cast<Task::task_type>(ui.comboBoxProcessType->currentIndex()));
        tmpTask->setDuration(ui.lineEditDuration->text());

        if (tmpTask->isObservation()) {
            Observation *tmpObs = static_cast<Observation *>(tmpTask);
            tmpObs->setRTCPsettings(getRTCPSettings());
            tmpObs->setDigitalBeams(itsDigitalBeams);
            if (ui.comboBoxStationAntennaMode->currentText().compare(antenna_modes_str[UNSPECIFIED_ANTENNA_MODE]) != 0) {
                tmpObs->setAntennaMode(antenna_modes_str[ui.comboBoxStationAntennaMode->currentIndex()]);
			}
			else incomplete = true;

            if (ui.comboBoxStationClock->currentText().compare(clock_frequencies_str[UNSPECIFIED_CLOCK]) != 0) {
                tmpObs->setStationClock(stringToStationClockType(ui.comboBoxStationClock->currentText().toStdString()));
			}
			else incomplete = true;

			const std::vector<std::string> &stations = getAssignedStationNames();
			if (!stations.empty()) {
                tmpObs->setStations(stations);
			}
			else incomplete = true;

		}

        TaskStorage *task_storage(tmpTask->storage());
        if (task_storage) {
            task_storage->setEnabledOutputDataProducts(itsOutputDataTypes);
            tmpTask->calculateDataFiles();
            task_storage->generateFileList();
            generateFileList(tmpTask, true);
            updateStorageTree();
            QPalette palet;
            if (!task_storage->hasStorageLocations()) {
                ui.lineEditStorageAssigned->setText("No storage set");
                ui.lineEditStorageAssigned->setPalette(palette());
            }
            else if (!task_storage->checkStorageAssigned()) {
                ui.lineEditStorageAssigned->setText("Storage not assigned");
                palet.setColor( QPalette::Base, Qt::red );
                ui.lineEditStorageAssigned->setPalette(palet);
            }
            else {
                ui.lineEditStorageAssigned->setText("Storage is assigned");
                palet.setColor( QPalette::Base, Qt::green );
                ui.lineEditStorageAssigned->setPalette(palet);
            }

            unsigned minNrOfNodes(0);
            itsMinNrOfRequiredNodes = task_storage->getMinimumNrOfStorageNodes();
            for (std::map<dataProductTypes, int>::const_iterator it = itsMinNrOfRequiredNodes.begin(); it != itsMinNrOfRequiredNodes.end(); ++it) {
                if (it->second == -1) { // single file storage node network bandwidth exceeded for this data product
                    minNrOfNodes = 0;
                    itsStorageOverflow = true;
                    ui.lineEditStorageConflict->setText("Current settings exceed storage node network bandwidth. The task cannot be scheduled.");
                    ui.lineEditStorageConflict->setToolTip(QString("The individual files for the ") + DATA_PRODUCTS[it->first] + " output will exceed the bandwidth of a single storage node.\nThe data size for this data type needs to reduced.");
                    ui.lineEditStorageConflict->show();
                }
                else {
                    minNrOfNodes = std::max(minNrOfNodes, (unsigned)it->second);
                }
            }
            if ((!itsStorageOverflow) && (itsController->isDataMonitorConnected())) { // following checks only when there is a data monitor connection
                size_t nrAvailableNodes = itsController->getNrOfStorageNodesAvailable();
                if (nrAvailableNodes > 0) {
                    if (minNrOfNodes > nrAvailableNodes) {
                        itsStorageOverflow = true;
                        ui.lineEditStorageConflict->setText("Too many storage nodes required. The task cannot be scheduled.");
                        ui.lineEditStorageConflict->setToolTip("The minimum number of storage nodes required to obtain the required network bandwidth exceeds the number of active storage nodes.");
                        ui.lineEditStorageConflict->show();
                    }
                    else {
                        ui.lineEditStorageConflict->hide();
                    }
                }
                else {
                    itsStorageOverflow = true;
                    ui.lineEditStorageConflict->setText("No storage nodes available. The task cannot be scheduled.");
                    ui.lineEditStorageConflict->setToolTip("There are no storage nodes available. The task cannot be scheduled");
                    ui.lineEditStorageConflict->show();
                }
            }
            else {
                ui.lineEditStorageConflict->hide();
            }
            checkStorageSettingsEnable();

            int totalSeconds = tmpTask->getDuration().totalSeconds();
            if (totalSeconds > 0 && !incomplete) {

                // set the newly calculated total data size in the appropriate lineEdit
                long double kbps = task_storage->getTotalStoragekBytes() / totalSeconds * 8;
                ui.lineEditTotalStorageSize->setText(humanReadableUnits((long double)task_storage->getTotalStoragekBytes(), SIZE_UNITS).c_str());
                ui.lineEditTotalBandwidth->setText(humanReadableUnits(kbps, BANDWIDTH_UNITS).c_str());
                ui.lineEditNrOfDataFiles->setText(QString::number(task_storage->getNrFiles()));
            }
            else {
                ui.lineEditTotalStorageSize->setText("0.0 kB");
                ui.lineEditTotalBandwidth->setText("0.0 kbit/s");
                ui.lineEditNrOfDataFiles->setText("0");
                ui.lineEditStorageConflict->hide();
            }

            std::map<dataProductTypes, int>::const_iterator minNodesIt(itsMinNrOfRequiredNodes.find(getSelectedStorageDataProduct()));
            if (minNodesIt != itsMinNrOfRequiredNodes.end()) {
                ui.lineEditMinNrStorageNodes->setText(QString::number(minNodesIt->second));
            }
            else {
                ui.lineEditMinNrStorageNodes->setText("0");
            }
        }
        delete tmpTask;
	}
    else { // status >= SCHEDULED  show the current task storage (no need to check all changes, because storage cannot be changed in the >= SCHEDULED states
        itsTask->calculateDataFiles();
        TaskStorage *task_storage(itsTask->storage());
        if (task_storage) {
            generateFileList(itsTask, true);
            updateStorageTree();
            // set the calculated total data size in the appropriate lineEdit
            long double kbps = task_storage->getTotalStoragekBytes() / itsTask->getDuration().totalSeconds() * 8;
            ui.lineEditTotalStorageSize->setText(humanReadableUnits((long double)task_storage->getTotalStoragekBytes(), SIZE_UNITS).c_str());
            ui.lineEditTotalBandwidth->setText(humanReadableUnits(kbps, BANDWIDTH_UNITS).c_str());
            ui.lineEditNrOfDataFiles->setText(QString::number(task_storage->getNrFiles()));
            // minimum number of storage nodes
            itsMinNrOfRequiredNodes = task_storage->getMinimumNrOfStorageNodes();
            std::map<dataProductTypes, int>::const_iterator minNodesIt(itsMinNrOfRequiredNodes.find(getSelectedStorageDataProduct()));
            if (minNodesIt != itsMinNrOfRequiredNodes.end()) {
                ui.lineEditMinNrStorageNodes->setText(QString::number(minNodesIt->second));
            }
            else {
                ui.lineEditMinNrStorageNodes->setText("0");
            }
            QPalette palet;
            if (!task_storage->hasStorageLocations()) {
                ui.lineEditStorageAssigned->setText("No storage set");
                ui.lineEditStorageAssigned->setPalette(palette());
            }
            else if (!task_storage->checkStorageAssigned()) {
                ui.lineEditStorageAssigned->setText("Storage not assigned");
                palet.setColor( QPalette::Base, Qt::red );
                ui.lineEditStorageAssigned->setPalette(palet);
            }
            else {
                ui.lineEditStorageAssigned->setText("Storage is assigned");
                palet.setColor( QPalette::Base, Qt::green );
                ui.lineEditStorageAssigned->setPalette(palet);
            }
        }
	}
}


void TaskDialog::doTabChangeUpdate(int /*currentTab*/) {
	if (ui.tabWidgetMain->currentWidget() == ui.tab_Storage) { // Storage tab
		updateStorageTab();
	}
}


void TaskDialog::addSuperStation(void) {
    ui.treeWidgetUsedStations->addSuperStationAndChilds();
	ui.labelAssignedStations->setText("Assigned stations (" + QString::number(countStations()) + ")");
}

void TaskDialog::addAvailableStations(const QStringList &stations) {
    ui.listWidgetAvailableStations->addStations(stations);
}

void TaskDialog::setCorrelatorIntegrationTime(void) {
    double integrationTime = ui.lineEditCorrelatorIntegrationTime->text().toDouble();
	if (integrationTime < MINIMUM_CORRELATOR_INTEGRATION_TIME) {
        QPalette palet = ( ui.lineEditCorrelatorIntegrationTime->palette() );
		palet.setColor( QPalette::Base, Qt::red );
        ui.lineEditCorrelatorIntegrationTime->setPalette(palet);
		QMessageBox::warning(this, tr("Wrong integration time value"),
				tr("The entered correlator integration time is too small\n The minimum value is ") +
				QString::number(MINIMUM_CORRELATOR_INTEGRATION_TIME) + tr(" sec"));
		QApplication::beep();
	}
	else {
        ui.lineEditCorrelatorIntegrationTime->setPalette(QPalette());
//		itsTask->setCorrelatorIntegrationTime(integrationTime);
	}
}

bool TaskDialog::checkEmail() {
	QRegExp regExpEmail("[A-Z0-9._%+-]+@[A-Z0-9.-]+.[A-Z]{2,4}");
	regExpEmail.setCaseSensitivity(Qt::CaseInsensitive);
	QRegExpValidator regExpValidator(regExpEmail, this);
	QString strValue = ui.lineEdit_ContactEmail->text();
	int pos;
	if ((regExpValidator.validate(strValue, pos) == QValidator::Acceptable) | (strValue == "")) {
		return true;
	}
	else {
		QMessageBox::warning(this, tr("Not a valid e-mail contact address"),
				tr("The contact's e-mail address in not a valid.\nPlease enter a valid e-mail address"));
		ui.tabWidgetMain->setCurrentWidget(ui.tab_ExtraInfo);
		ui.lineEdit_ContactEmail->setFocus();
		ui.lineEdit_ContactEmail->selectAll();
		return false;
	}
}

void TaskDialog::enablePredecessorSettings(bool enable) {
    ui.lineEditMinPredDistance->setEnabled(enable);
    ui.lineEditMaxPredDistance->setEnabled(enable);
	ui.labelMinPredDistance->setEnabled(enable);
	ui.labelMaxPredDistance->setEnabled(enable);
}

void TaskDialog::loadAvailableStations(void) {
    ui.listWidgetAvailableStations->clear();
	stationDefinitionsMap stations = Controller::theSchedulerSettings.getStationList();
	for (stationDefinitionsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
        ui.listWidgetAvailableStations->addItem(it->first.c_str());
	}
	itsStationsLoaded = true;
}

void TaskDialog::updateDuration(void) { // called when user fiddles with the scheduled end time box
    if ((!ui.dateTimeEditScheduledStart->isUndefined()) & (!ui.lineEditDuration->isUndefined())) {
        AstroTime duration = AstroDateTime(ui.dateTimeEditScheduledEnd->dateTime() - ui.dateTimeEditScheduledStart->dateTime());
        ui.lineEditDuration->blockSignals(true);
        ui.lineEditDuration->setText(duration.toString());
        ui.lineEditDuration->blockSignals(false);
		detectChanges();
	}
}

void TaskDialog::updateScheduledEnd(void) { // called when user changes duration or scheduled start time box
    if (ui.lineEditDuration->isUndefined()) { // if duration edit is not set
        ui.dateTimeEditScheduledEnd->setUndefined(); // set end time back on undefined when duration is undefined (=MULTIPLE values)
	}
    else if (!ui.dateTimeEditScheduledStart->isUndefined()) {
        AstroDateTime end = AstroDateTime(ui.dateTimeEditScheduledStart->dateTime()) + AstroTime(ui.lineEditDuration->text());
		setScheduledEnd(QDateTime(QDate(end.getYear(), end.getMonth(), end.getDay()), QTime(end.getHours(), end.getMinutes(), end.getSeconds())));
	}
	detectChanges();
}

void TaskDialog::setScheduledStart(const QDateTime &start) {
    ui.dateTimeEditScheduledStart->blockSignals(true); // prevents automatic update
    ui.dateTimeEditScheduledStart->setDateTime(start);
    ui.dateTimeEditScheduledStart->blockSignals(false);
}


void TaskDialog::setScheduledEnd(const QDateTime &end) {
    ui.dateTimeEditScheduledEnd->blockSignals(true); // prevents automatic update
    ui.dateTimeEditScheduledEnd->setDateTime(end);
    ui.dateTimeEditScheduledEnd->blockSignals(false);
}


void TaskDialog::detectChanges(void) {
    if (!blockChangeDetection) {

        QString tabName = ui.tabWidgetMain->tabText(ui.tabWidgetMain->currentIndex());

        if(tabName.contains(tab_names[TAB_SCHEDULE])) { // Schedule tab
            changeSchedule = false;
            if (ui.lineEditPredecessors->hasBeenChanged()) changeSchedule = true;
            else if (ui.lineEditGroupID->hasBeenChanged()) changeSchedule = true;
            else if (ui.lineEditMaxPredDistance->hasBeenChanged()) changeSchedule = true;
            else if (ui.lineEditMinPredDistance->hasBeenChanged()) changeSchedule = true;
            else if (ui.lineEditPriority->hasBeenChanged()) changeSchedule = true;
            else if (ui.checkBoxFixedDate->hasBeenChanged()) changeSchedule = true;
            else if (ui.checkBoxFixedTime->hasBeenChanged()) changeSchedule = true;
            else if (ui.timeEditFirstPossibleTime->hasBeenChanged()) changeSchedule = true;
            else if (ui.timeEditLastPossibleTime->hasBeenChanged()) changeSchedule = true;
            else if (ui.dateEditFirstPossibleDate->hasBeenChanged()) changeSchedule = true;
            else if (ui.dateEditLastPossibleDate->hasBeenChanged()) changeSchedule = true;
            else if (ui.lineEditTaskName->hasBeenChanged()) changeSchedule = true;
            else if (ui.comboBoxProjectID->hasBeenChanged()) changeSchedule = true;
            else if (ui.lineEditDuration->hasBeenChanged()) changeSchedule = true;
            else if (ui.dateTimeEditScheduledStart->hasBeenChanged()) changeSchedule = true;
            else if (ui.dateTimeEditScheduledEnd->hasBeenChanged()) changeSchedule = true;
            else if (ui.comboBoxTaskStatus->hasBeenChanged()) changeSchedule = true;
            else if (ui.comboBoxProcessType->hasBeenChanged()) changeSchedule = true;
            else if (ui.comboBoxProcessSubType->hasBeenChanged()) changeSchedule = true;
            else if (ui.comboBoxStrategies->hasBeenChanged()) changeSchedule = true;
            setTabModified(TAB_SCHEDULE, changeSchedule);
        }

        if (tabName.contains(tab_names[TAB_STATION_SETTINGS])) { // Station settings tab
            changeStations = false;
            if (ui.checkBoxTBBPiggybackAllowed->hasBeenChanged()) changeStations = true;
            else if (ui.checkBoxAartfaacPiggybackAllowed->hasBeenChanged()) changeStations = true;
            setTabModified(TAB_STATION_SETTINGS, changeStations);
        }

        else if (tabName.contains(tab_names[TAB_STATION_BEAMS])) { // beams tab
            changeBeams = false;
            if (ui.lineEditAnalogBeamAngle1->hasBeenChanged()) changeBeams = true;
            else if (ui.lineEditAnalogBeamAngle2->hasBeenChanged()) changeBeams = true;
            else if (ui.timeEditAnalogBeamStartTime->hasBeenChanged()) changeBeams = true;
            else if (ui.lineEditAnalogBeamDuration->hasBeenChanged()) changeBeams = true;
            else if (itsTempAnalogBeamSettings.directionType != itsAnalogBeamSettings.directionType) changeBeams = true;
            else if (itsTempAnalogBeamSettings.startTime != itsAnalogBeamSettings.startTime) changeBeams = true;
            else if (itsTempAnalogBeamSettings.duration != itsAnalogBeamSettings.duration) changeBeams = true;
            // digital beams (including tied array beam) differences
            else if (itsTempDigitalBeams != itsDigitalBeams) changeBeams = true;
            setTabModified(TAB_STATION_BEAMS, changeBeams);
        }

        else if (tabName.contains(tab_names[TAB_PROCESSING])) { // processing
            changeProcessing = false;
            if (ui.checkBoxCorrelatedData->hasBeenChanged()) changeProcessing = true;
            else if (ui.checkBoxCoherentStokes->hasBeenChanged()) changeProcessing = true;
            else if (ui.checkBoxIncoherentStokes->hasBeenChanged()) changeProcessing = true;
            else if (ui.checkBox_BandpassCorrection->hasBeenChanged()) changeProcessing = true;
            else if (ui.checkBox_CoherentDedispersion->hasBeenChanged()) changeProcessing = true;
            else if (ui.checkBox_DelayCompensation->hasBeenChanged()) changeProcessing = true;
            else if (ui.checkBoxPencilFlysEye->hasBeenChanged()) changeProcessing = true;
            else if (ui.comboBoxBitsPerSample->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxChannelsPerSubband->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxCoherentTimeIntegration->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxIncoherentTimeIntegration->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxCoherentChannelsPerSubband->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxIncoherentChannelsPerSubband->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxCoherentSubbandsPerFile->hasBeenChanged()) changeProcessing = true;
            else if (ui.spinBoxIncoherentSubbandsPerFile->hasBeenChanged()) changeProcessing = true;
            else if (ui.comboBoxCoherentStokesType->hasBeenChanged()) changeProcessing = true;
            else if (ui.comboBoxIncoherentStokesType->hasBeenChanged()) changeProcessing = true;
            else if (ui.lineEditCorrelatorIntegrationTime->hasBeenChanged()) changeProcessing = true;
            setTabModified(TAB_PROCESSING, changeProcessing);
        }

        else if (tabName.contains(tab_names[TAB_STORAGE])) { // storage
            changeStorage = false;
            if (ui.comboBoxStorageSelectionMode->hasBeenChanged()) changeStorage = true;
            setTabModified(TAB_STORAGE, changeStorage);
        }

        else if (tabName.contains(tab_names[TAB_PIPELINE])) {
            changePipeline = false;
            if (ui.listWidgetDemixAlways->hasBeenChanged()) changePipeline = true;
            else if (ui.listWidgetDemixIfNeeded->hasBeenChanged()) changePipeline = true;
            else if (ui.spinBoxDemixFreqStep->hasBeenChanged()) changePipeline = true;
            else if (ui.spinBoxDemixTimeStep->hasBeenChanged()) changePipeline = true;
            else if (ui.spinBoxAveragingFreqStep->hasBeenChanged()) changePipeline = true;
            else if (ui.spinBoxAveragingTimeStep->hasBeenChanged()) changePipeline = true;
            else if (ui.lineEditDemixSkyModel->hasBeenChanged()) changePipeline = true;
            else if (ui.lineEditCalibrationSkyModel->hasBeenChanged()) changePipeline = true;

            if (!changePipeline && ui.groupBoxImaging->isEnabled()) {
                if (ui.checkBoxSpecifyFOV->hasBeenChanged()) changeProcessing = true;
                else if (ui.lineEditFieldOfView->hasBeenChanged()) changePipeline = true;
                else if (ui.lineEditCellSize->hasBeenChanged()) changePipeline = true;
                else if (ui.spinBoxSlicesPerImage->hasBeenChanged()) changePipeline = true;
                else if (ui.spinBoxSubbandsPerImage->hasBeenChanged()) changePipeline = true;
                else if (ui.spinBoxNumberOfPixels->hasBeenChanged()) changePipeline = true;
            }
            setTabModified(TAB_PIPELINE, changePipeline);
        }

        else if (tabName.contains(tab_names[TAB_PULSAR_PIPELINE])) {
            changePulsar = false;
            if (ui.lineEditPulsarName->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEditDSPSRextraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEditPrepdataExtraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEdit2bf2fitsExtraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEditRfiFindExtraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEditPrepfoldExtraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEditPrepsubbandExtraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.lineEditDigifilExtraOptions->hasBeenChanged()) changePulsar = true;
            else if (ui.spinBoxTsubint->hasBeenChanged()) changePulsar = true;
            else if (ui.spinBoxDecodeSigma->hasBeenChanged()) changePulsar = true;
            else if (ui.spinBoxDecodeNblocks->hasBeenChanged()) changePulsar = true;
            else if (ui.doubleSpinBox8BitConversionSigma->hasBeenChanged()) changePulsar = true;
            else if (ui.doubleSpinBoxDynamicSpectrumTimeAverage->hasBeenChanged()) changePulsar = true;
            else if (ui.doubleSpinBoxRratsDmRange->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_NoRFI->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_No_DSPSR->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_No_fold->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_No_pdmp->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_RawTo8Bit->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_rrats->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_Single_pulse->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_Skip_dynamic_spectrum->hasBeenChanged()) changePulsar = true;
            else if (ui.checkBox_Skip_prepfold->hasBeenChanged()) changePulsar = true;
            setTabModified(TAB_PULSAR_PIPELINE, changePulsar);
        }

        else if (tabName.contains(tab_names[TAB_LONGBASELINE_PIPELINE])) {
            changeLongBaseline = false;
            if (ui.spinBoxSubbandGroupsPerMS->hasBeenChanged()) changeLongBaseline = true;
            else if (ui.spinBoxSubbandsPerSubbandGroup->hasBeenChanged()) changeLongBaseline = true;
            setTabModified(TAB_LONGBASELINE_PIPELINE, changeLongBaseline);
        }

        else if (tabName.contains(tab_names[TAB_EXTRA_INFO])) { // Extra info tab
            changeExtraInfo = false;
            if (ui.lineEdit_ContactName->text().compare(itsTask->getContactName())) changeExtraInfo = true;
            else if (ui.lineEdit_ContactPhone->text().compare(itsTask->getContactPhone())) changeExtraInfo = true;
            else if (ui.lineEdit_ContactEmail->text().compare(itsTask->getContactEmail())) changeExtraInfo = true;
            else if (ui.lineEditTaskDescription->text().compare(itsTask->SASTree().description().c_str())) changeExtraInfo = true;
            setTabModified(TAB_EXTRA_INFO, changeExtraInfo);
        }

        enableApplyButtons(ifSettingsChanged());
    }
}

void TaskDialog::setAllTabsUnmodified(void) {
    for (int i = 0; i < NR_TABS; ++i) {
        ui.tabWidgetMain->setTabText(i, ui.tabWidgetMain->tabText(i).remove('*'));
    }
}

void TaskDialog::setTabModified(tabIndex tabIdx, bool modified) {
    for (int i = 0; i < NR_TABS; ++i) {

        if (!ui.tabWidgetMain->tabText(i).contains(tab_names[tabIdx]))
            continue;

        // found the right tab:
        if (modified) {
            ui.tabWidgetMain->setTabText(i, QString("*") + tab_names[tabIdx]);
        }
        else {
            ui.tabWidgetMain->setTabText(i, ui.tabWidgetMain->tabText(i).remove('*'));
        }
        break;

    }
}

void TaskDialog::checkEnableDataTypeSettings(void) {
	bool coherentEnabled(ui.checkBoxCoherentStokes->isChecked());
    ui.comboBoxCoherentStokesType->setEnabled(coherentEnabled);
	ui.label_Coherent->setEnabled(coherentEnabled);
    if (ui.comboBoxCoherentStokesType->currentIndex() != DATA_TYPE_XXYY) {
        ui.spinBoxCoherentTimeIntegration->setEnabled(coherentEnabled);
        ui.spinBoxCoherentSubbandsPerFile->setEnabled(coherentEnabled);
        ui.spinBoxCoherentChannelsPerSubband->setEnabled(coherentEnabled);
	}
    else { // Complex Voltages (XXYY)
        ui.spinBoxCoherentTimeIntegration->setValue(1);
        ui.spinBoxCoherentSubbandsPerFile->setEnabled(coherentEnabled);
        ui.spinBoxCoherentChannelsPerSubband->setEnabled(coherentEnabled);
        ui.spinBoxCoherentTimeIntegration->setEnabled(coherentEnabled);
    }
    bool incoherentEnabled(ui.checkBoxIncoherentStokes->isChecked());
	ui.label_Incoherent->setEnabled(incoherentEnabled);
    ui.spinBoxIncoherentTimeIntegration->setEnabled(incoherentEnabled);
    ui.spinBoxIncoherentSubbandsPerFile->setEnabled(incoherentEnabled);
    ui.comboBoxIncoherentStokesType->setEnabled(incoherentEnabled);
    ui.spinBoxIncoherentChannelsPerSubband->setEnabled(incoherentEnabled);
	ui.checkBoxPencilFlysEye->setEnabled(coherentEnabled);
	ui.label_StokesType->setEnabled(coherentEnabled || incoherentEnabled);
	ui.checkBox_CoherentDedispersion->setEnabled(coherentEnabled || incoherentEnabled);
	ui.label_StokesChannelsPerSubband->setEnabled(coherentEnabled || incoherentEnabled);
	ui.label_StokesSubbandsPerFile->setEnabled(coherentEnabled || incoherentEnabled);
	ui.label_StokesTimeIntegration->setEnabled(coherentEnabled || incoherentEnabled);
}

void TaskDialog::outputDataTypesChanged() {
    itsOutputDataTypes.correlated = ui.checkBoxCorrelatedData->isChecked();
    itsOutputDataTypes.coherentStokes = ui.checkBoxCoherentStokes->isChecked();
    itsOutputDataTypes.incoherentStokes = ui.checkBoxIncoherentStokes->isChecked();

    if (ui.checkBoxCorrelatedData->isChecked()) {
        ui.label_CorrelatorIntTime->setEnabled(true);
        ui.lineEditCorrelatorIntegrationTime->setEnabled(true);
        ui.labelChannelsPerSubband->setEnabled(true);
        ui.spinBoxChannelsPerSubband->setEnabled(true);
    }
    else {
        ui.label_CorrelatorIntTime->setEnabled(false);
        ui.lineEditCorrelatorIntegrationTime->setEnabled(false);
        ui.labelChannelsPerSubband->setEnabled(false);
        ui.spinBoxChannelsPerSubband->setEnabled(false);
    }

    checkEnableDataTypeSettings();

    // check if pencil buttons (add/edit/delete) should be enabled
    checkEnableBeamButtons();

    setStorageSettings();

    checkStorageSettingsEnable();
    detectChanges();
}

// update the comboBoxStorageDataType for selecting the displayed storage tree
// and checks if the storage tree itself needs to be updated as well.
void TaskDialog::setStorageSettings(bool forceUpdate) {
	// add choices to comboBoxStorageDataTypes
	QString curDataProduct = ui.comboBoxStorageDataType->currentText();
	ui.comboBoxStorageDataType->blockSignals(true);
	ui.comboBoxStorageDataType->clear();
	QStringList items;
	if (itsOutputDataTypes.correlated) items << DATA_PRODUCTS[DP_CORRELATED_UV];
	if (itsOutputDataTypes.coherentStokes) items << DATA_PRODUCTS[DP_COHERENT_STOKES];
	if (itsOutputDataTypes.incoherentStokes) items << DATA_PRODUCTS[DP_INCOHERENT_STOKES];
    if (itsOutputDataTypes.instrumentModel) items << DATA_PRODUCTS[DP_INSTRUMENT_MODEL];
    if (itsOutputDataTypes.pulsar) items << DATA_PRODUCTS[DP_PULSAR];
    if (itsOutputDataTypes.skyImage) items << DATA_PRODUCTS[DP_SKY_IMAGE];

	ui.comboBoxStorageDataType->addItems(items);
	// check if the currently displayed storage tree needs to be updated.
	// only update if the current storage tree is displaying the storage settings for a data product that has now been disabled
	bool update_storage_tree(true);
	if (!forceUpdate) {
		for (short i = 0; i < ui.comboBoxStorageDataType->count(); ++i) {
			if (ui.comboBoxStorageDataType->itemText(i).compare(curDataProduct) == 0) {
				ui.comboBoxStorageDataType->setCurrentIndex(i);
				update_storage_tree = false;
				break;
			}
		}
	}
	ui.comboBoxStorageDataType->blockSignals(false);
	if (forceUpdate || update_storage_tree) {
		updateStorageTree();
	}
}

// checks if data products have been switched off by the user and removes them from itsTmpStorage if needed
void TaskDialog::checkForRemovalOfDataProducts(const TaskStorage::enableDataProdukts &edp) {
	if (!edp.correlated) {
		itsTmpStorage.erase(DP_CORRELATED_UV);
	}
	if (!edp.coherentStokes) {
		itsTmpStorage.erase(DP_COHERENT_STOKES);
	}
	if (!edp.incoherentStokes) {
		itsTmpStorage.erase(DP_INCOHERENT_STOKES);
	}
	if (!edp.instrumentModel) {
		itsTmpStorage.erase(DP_INSTRUMENT_MODEL);
	}
    if (!edp.pulsar) {
        itsTmpStorage.erase(DP_PULSAR);
    }
    if (!edp.skyImage) {
		itsTmpStorage.erase(DP_SKY_IMAGE);
	}
}


// enable/disable beam & pencilbeam add/delete/edit buttons according to current dialog settings
void TaskDialog::checkEnableBeamButtons(void) {
	bool enableBeamDelAdd = false;
    if (addingTask || ((itsTask->getSASTreeID() == 0) && itsTask->getStatus() <= Task::PRESCHEDULED)) enableBeamDelAdd = true;
	ui.pushButtonAddBeam->setEnabled(enableBeamDelAdd);
    const Task::task_status &status = itsTask->getStatus(); // get the current status from the dialog
	if (!itsDigitalBeams.empty()) { // if beams are defined
		ui.pushButtonEditBeam->setEnabled(true); // edit is also show button and should be enabled
		if (status >= Task::SCHEDULED) {
			itsDigitalBeamDialog->setReadOnly(true);
			ui.pushButtonEditBeam->setText("Show");
			ui.pushButtonDeleteBeams->setEnabled(false);
			ui.pushButtonClearAllBeams->setEnabled(false);
		}
		else {
			itsDigitalBeamDialog->setReadOnly(false);
			ui.pushButtonEditBeam->setText("Edit");
			ui.pushButtonDeleteBeams->setEnabled(enableBeamDelAdd); // depends on VIC tree or template tree
			ui.pushButtonClearAllBeams->setEnabled(enableBeamDelAdd);
		}
	}
	else { // if no beams have been defined yet
		if (status >= Task::SCHEDULED) {
			itsDigitalBeamDialog->setReadOnly(true);
			ui.pushButtonEditBeam->setText("Show");
		}
		else {
			itsDigitalBeamDialog->setReadOnly(false);
			ui.pushButtonEditBeam->setText("Edit");
		}
		ui.pushButtonEditBeam->setEnabled(false);
		ui.pushButtonDeleteBeams->setEnabled(false);
		ui.pushButtonClearAllBeams->setEnabled(false);
	}

	if (itsDigitalBeams.empty()) {
		ui.pushButtonAddTiedArrayBeam->setEnabled(false);
		ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
		ui.pushButtonEditTiedArrayBeam->setEnabled(false);
		ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
	}
	else {
		int digiBeamNr(ui.tableWidgetDigitalBeams->currentRow());
		if (digiBeamNr >= 0) {
			bool TABsLeft(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()].nrManualTABs() > 0);
			// enable/disable tied array add and delete buttons
			if (enableBeamDelAdd) {
				ui.pushButtonAddTiedArrayBeam->setEnabled(true);
				if (TABsLeft) {
					ui.pushButtonClearAllTiedArrayBeam->setEnabled(true);
					ui.pushButtonDeleteTiedArrayBeam->setEnabled(true);
				}
				else { // no TABs left
					ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
					ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
				}
			}
			else {
				ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
				ui.pushButtonAddTiedArrayBeam->setEnabled(false);
				ui.pushButtonDeleteTiedArrayBeam->setEnabled(false); // depends on VIC tree or template tree
			}
			// edit tied array beams?
			if ((status >= Task::SCHEDULED) || !TABsLeft) {
				ui.pushButtonEditTiedArrayBeam->setEnabled(false);
				itsTiedArrayBeamDialog->setReadOnly(true);
			}
			else {
				ui.pushButtonEditTiedArrayBeam->setEnabled(true); // show or edit button
				itsTiedArrayBeamDialog->setReadOnly(false);
			}
		}
	}
}

void TaskDialog::checkStorageSettingsEnable(void) {
    Task::task_status status = itsTask->getStatus();
	if (ui.comboBoxStorageDataType->count() == 0) {
		ui.treeWidgetStorageNodes->setToolTip("No output data type (tab processing) is selected.\nNo data will be generated. Cannot set storage");
	}
	else if (status != Task::PRESCHEDULED) { // only enable storage settings if task has PRESCHEDULED status
		ui.treeWidgetStorageNodes->setToolTip("This task is not in PRESCHEDULED state. It is not possible to change it's storage resources");
	}
	else if (!itsStorageOverflow) {
		if (isMultiTasks) {
			ui.treeWidgetStorageNodes->setToolTip("These storage nodes will be assigned to all multi-edited tasks");
		}
		else {
			ui.treeWidgetStorageNodes->setToolTip("The storage nodes and raid sets used by this task");
		}
	}
}

void TaskDialog::applyStationsRemoved(void) {
	enableApplyButtons(true);
    ui.labelAssignedStations->setText("Assigned stations (" + QString::number(countStations() - ui.treeWidgetUsedStations->getNrStationsRemoved()) + ")");

}

void TaskDialog::addStationsToUsedStations(const QStringList &stations) {
	if (!stations.empty()) {
        ui.treeWidgetUsedStations->addStations(stations);
		checkIfStationChanged();
	}
}

void TaskDialog::checkIfStationChanged() {
	// for now we just set as if it changed, check is to difficult to implement
	enableApplyButtons(true);
	ui.labelAssignedStations->setText("Assigned stations (" + QString::number(countStations()) + ")");
}


void TaskDialog::ApplyFirstPossibleDateChange(void) {
    QDate fpd = ui.dateEditFirstPossibleDate->date();
    ui.dateTimeEditScheduledStart->setMinimumDate(fpd);
}

void TaskDialog::ApplyFirstPossibleTimeChange(void) {
    QTime fpt = ui.timeEditFirstPossibleTime->time();
    ui.dateTimeEditScheduledStart->setMinimumTime(fpt);
}

void TaskDialog::ApplyLastPossibleTimeChange(void) {
    QTime lpt = ui.timeEditLastPossibleTime->time();
    ui.dateTimeEditScheduledEnd->setMaximumTime(lpt);
}

void TaskDialog::cancelClicked(void) {
	addingTask = false;
	addingReservation = false;
    blockChangeDetection = false;
	isMultiTasks = false;
    clearMultiTasks();
	ui.tabWidgetMain->setCurrentIndex(0);
	hide();
}

void TaskDialog::applyClicked(void) {
	apply(false);
}

void TaskDialog::okClicked(void) {
	apply(true);
}

void TaskDialog::apply(bool close) {
    blockChangeDetection = false;
	bool notCommitted(false);
	if (isMultiTasks) {
		if (!commitMultiTasks()) {
			raise();
			notCommitted = true;
			close = false;
		}
	}
    else if (itsTask->isReservation() || itsTask->isMaintenance()) {
		if (!commitReservation(!close)) {
			raise();
			notCommitted = true;
			close = false;
		}
	}
    else if (itsTask->isPipeline()) {
		if (!commitPipeline()) {
			raise();
			notCommitted = true;
			close = false;
		}
	}
	else if (!commitChanges(!close)) {
		raise();
		notCommitted = true;
		close = false;
	}
	if (close) {
		this->close();
	}
	else {
		if (ui.tabWidgetMain->currentWidget() == ui.tab_Storage) {
			updateStorageTab();
		}
        ui.pushButtonCancelClose->setFocus(); // change focus away from apply button to cancel button

        if (!notCommitted) {
			resetChangeDetection();
            setAllTabsUnmodified();
		}
	}
}

void TaskDialog::resetChangeDetection(void) {
	changeSchedule = false;
	changeStations = false;
	changeBeams = false;
    changeExtraInfo = false;
	changeStorage = false;
	changePipeline = false;
    changePulsar = false;
    changeLongBaseline = false;
	changeProcessing = false;
	changeEnabledInputFiles = false;
    ui.lineEditTaskName->resetChangeDetect();
    ui.lineEditMinPredDistance->resetChangeDetect();
    ui.lineEditMaxPredDistance->resetChangeDetect();
    ui.lineEditAnalogBeamDuration->resetChangeDetect();
    ui.lineEditGroupID->resetChangeDetect();
    ui.lineEditCorrelatorIntegrationTime->resetChangeDetect();
    ui.lineEditPredecessors->resetChangeDetect();
    ui.lineEditAnalogBeamAngle1->resetChangeDetect();
    ui.lineEditAnalogBeamAngle2->resetChangeDetect();
    ui.dateEditFirstPossibleDate->resetChangeDetect();
    ui.dateEditLastPossibleDate->resetChangeDetect();
    ui.timeEditFirstPossibleTime->resetChangeDetect();
    ui.timeEditLastPossibleTime->resetChangeDetect();
    ui.dateTimeEditScheduledStart->resetChangeDetect();
    ui.dateTimeEditScheduledEnd->resetChangeDetect();
    ui.lineEditDuration->resetChangeDetect();
    ui.lineEditPriority->resetChangeDetect();
    ui.comboBoxTaskStatus->resetChangeDetect();
    ui.comboBoxProjectID->resetChangeDetect();
    ui.comboBoxProcessType->resetChangeDetect();
    ui.comboBoxProcessSubType->resetChangeDetect();
    ui.comboBoxStrategies->resetChangeDetect();
    ui.comboBoxStationAntennaMode->resetChangeDetect();
    ui.comboBoxStationFilter->resetChangeDetect();
    ui.comboBoxReservation->resetChangeDetect();
    ui.comboBoxStationClock->resetChangeDetect();
    ui.comboBoxBitsPerSample->resetChangeDetect();
    ui.spinBoxChannelsPerSubband->resetChangeDetect();
    ui.comboBoxStorageSelectionMode->resetChangeDetect();
    ui.comboBoxAnalogBeamCoordinates->resetChangeDetect();
    ui.comboBoxAnalogBeamUnits->resetChangeDetect();
    ui.comboBoxCoherentStokesType->resetChangeDetect();
    ui.spinBoxCoherentTimeIntegration->resetChangeDetect();
    ui.spinBoxCoherentChannelsPerSubband->resetChangeDetect();
    ui.spinBoxCoherentSubbandsPerFile->resetChangeDetect();
    ui.comboBoxIncoherentStokesType->resetChangeDetect();
    ui.spinBoxIncoherentTimeIntegration->resetChangeDetect();
    ui.spinBoxIncoherentChannelsPerSubband->resetChangeDetect();
    ui.spinBoxIncoherentSubbandsPerFile->resetChangeDetect();
    ui.listWidgetDemixAlways->resetChangeDetect();
    ui.listWidgetDemixIfNeeded->resetChangeDetect();
    ui.spinBoxDemixFreqStep->resetChangeDetect();
    ui.spinBoxDemixTimeStep->resetChangeDetect();
    ui.spinBoxAveragingFreqStep->resetChangeDetect();
    ui.spinBoxAveragingTimeStep->resetChangeDetect();
    ui.lineEditDemixSkyModel->resetChangeDetect();
    ui.lineEditFieldOfView->resetChangeDetect();
    ui.lineEditCellSize->resetChangeDetect();
    ui.spinBoxSlicesPerImage->resetChangeDetect();
    ui.spinBoxSubbandsPerImage->resetChangeDetect();
    ui.spinBoxSubbandGroupsPerMS->resetChangeDetect();
    ui.spinBoxSubbandsPerSubbandGroup->resetChangeDetect();
    ui.spinBoxNumberOfPixels->resetChangeDetect();
    ui.lineEditCalibrationSkyModel->resetChangeDetect();
    ui.checkBoxCorrelatedData->resetChangeDetect();
    ui.checkBoxCoherentStokes->resetChangeDetect();
    ui.checkBoxIncoherentStokes->resetChangeDetect();
    ui.checkBox_BandpassCorrection->resetChangeDetect();
    ui.checkBox_CoherentDedispersion->resetChangeDetect();
    ui.checkBox_DelayCompensation->resetChangeDetect();
    ui.checkBoxPencilFlysEye->resetChangeDetect();
    ui.checkBoxCoherentStokes->resetChangeDetect();

}

std::vector<std::string> TaskDialog::getAssignedStationNames(void) const {
	std::vector<std::string> stations;
	std::string stationName;
    QTreeWidgetItemIterator it(ui.treeWidgetUsedStations, QTreeWidgetItemIterator::NoChildren);
	while (*it) {
		stationName = (*it++)->text(0).toStdString();
		stations.push_back(stationName);
	}
	return stations;
}

unsigned TaskDialog::countStations(void) const {
	unsigned nrOfStations(0), superStationNrChilds(0);
    QTreeWidgetItemIterator it(ui.treeWidgetUsedStations, QTreeWidgetItemIterator::All);
	QString stationName;
	while (*it) {
		stationName = (*it)->text(0);
		if (!stationName.startsWith(">") && !stationName.startsWith("MIXED")) {
			++nrOfStations;
		}
		else {
			unsigned nrChilds = (*it)->childCount();
			if (nrChilds) {
				++nrOfStations;
				superStationNrChilds += nrChilds;
			}
		}
		++it;
	}
	nrOfStations -= superStationNrChilds;
	return nrOfStations;
}

std::map<int, QStringList> TaskDialog::getSuperStationChildNames(void) const {
	std::map<int, QStringList> superStations;
    QTreeWidgetItemIterator ssit(ui.treeWidgetUsedStations, QTreeWidgetItemIterator::HasChildren);
	QStringList childNames;
	int ssidx(0);
	while (*ssit) {
		++ssidx;
		childNames.clear();
		for (int cc = 0; cc < (*ssit)->childCount(); ++cc) {
			childNames.append((*ssit)->child(cc)->text(0));
		}
		superStations.insert(std::map<int, QStringList>::value_type(ssidx, childNames));
		++ssit; // next super station
	}
	return superStations;
}

Observation::RTCPsettings TaskDialog::getRTCPSettings(void) {
    Observation::RTCPsettings RTCPsettings;
	RTCPsettings.correctBandPass = ui.checkBox_BandpassCorrection->isChecked();
	RTCPsettings.delayCompensation = ui.checkBox_DelayCompensation->isChecked();
	RTCPsettings.flysEye = ui.checkBoxPencilFlysEye->isChecked();
    RTCPsettings.correlatorIntegrationTime = ui.lineEditCorrelatorIntegrationTime->text().toDouble();
    RTCPsettings.nrBitsPerSample = ui.comboBoxBitsPerSample->currentText().toUInt();
	RTCPsettings.coherentDedisperseChannels = ui.checkBox_CoherentDedispersion->isChecked();
    RTCPsettings.coherentType = stringToStokesType(ui.comboBoxCoherentStokesType->currentText().toStdString());
    RTCPsettings.incoherentType = stringToStokesType(ui.comboBoxIncoherentStokesType->currentText().toStdString());
    RTCPsettings.coherentTimeIntegrationFactor = ui.spinBoxCoherentTimeIntegration->value();
    RTCPsettings.incoherentTimeIntegrationFactor = ui.spinBoxIncoherentTimeIntegration->value();
    RTCPsettings.channelsPerSubband = ui.spinBoxChannelsPerSubband->value();
    RTCPsettings.coherentSubbandsPerFile = ui.spinBoxCoherentSubbandsPerFile->value();
    RTCPsettings.incoherentSubbandsPerFile = ui.spinBoxIncoherentSubbandsPerFile->value();
    RTCPsettings.coherentChannelsPerSubband = ui.spinBoxCoherentChannelsPerSubband->value();
    RTCPsettings.incoherentChannelsPerSubband = ui.spinBoxIncoherentChannelsPerSubband->value();
	return RTCPsettings;
}

superStationMap TaskDialog::getSuperStations(const Observation *task) const {
    superStationMap superstations;
    const taskStationsMap &stationMapping = task->getStations();
	taskStationsMap::const_iterator tsit;
	int i(0);
	std::vector<unsigned> childStations;
    QTreeWidgetItemIterator ssit(ui.treeWidgetUsedStations, QTreeWidgetItemIterator::HasChildren);
	std::string beamFormerName;
	QTreeWidgetItem *childItem;
	while (*ssit) {
		childStations.clear();
		for (int cc = 0; cc < (*ssit)->childCount(); ++cc) {
			childItem = (*ssit)->child(cc);
			tsit = stationMapping.find(childItem->text(0).toStdString());
			if (tsit != stationMapping.end()) {
				childStations.push_back(tsit->second); // push back the station id of this child
			}
		}
        beamFormerName = "Beamformer[" + int2String(i++) + "]";
        superstations.insert(superStationMap::value_type(beamFormerName, childStations));
		++ssit; // next super station
	}
	return superstations;
}

void TaskDialog::applyProjectChange(const QString &project_id) {
	const campaignInfo &campaign = Controller::theSchedulerSettings.getCampaignInfo(project_id.toStdString());
	if (!campaign.name.empty()) {
		ui.lineEdit_ProjectName->setText(campaign.title.c_str());
		ui.lineEdit_ProjectPI->setText(campaign.PriInvestigator.c_str());
		ui.lineEdit_ProjectCOI->setText(campaign.CoInvestigator.c_str());
		ui.lineEdit_ContactName->setText(campaign.contact.c_str());
	}
	else {
		ui.lineEdit_ProjectName->clear();
		ui.lineEdit_ProjectPI->clear();
		ui.lineEdit_ProjectCOI->clear();
		ui.lineEdit_ContactName->clear();
	}
	detectChanges();
}

bool TaskDialog::commitPipeline(void) {
    Pipeline *pipeline(static_cast<Pipeline *>(itsTask));
    QString strValue = ui.lineEditTaskName->text();
    pipeline->setTaskName(strValue.toStdString());
    pipeline->setProjectName(ui.lineEdit_ProjectName->text().toStdString());
    pipeline->setProjectCO_I(ui.lineEdit_ProjectCOI->text().toStdString());
    pipeline->setProjectID(ui.comboBoxProjectID->currentText().toStdString());
    pipeline->setProjectPI(ui.lineEdit_ProjectPI->text().toStdString());

    pipeline->setType(ui.comboBoxProcessType->currentText(),
            stringToProcessSubType(ui.comboBoxProcessSubType->currentText()),
            ui.comboBoxStrategies->currentText());
    pipeline->setOriginalTreeID(ui.lineEditOriginalTreeID->text().toInt());

    Task::task_status state = taskStatusFromString(ui.comboBoxTaskStatus->currentText().toStdString());
    pipeline->setStatus(state);

    pipeline->setContactEmail(ui.lineEdit_ContactEmail->text().toStdString());
    pipeline->setContactName(ui.lineEdit_ContactName->text().toStdString());
    pipeline->setContactPhone(ui.lineEdit_ContactPhone->text().toStdString());
    pipeline->setTaskDescription(ui.lineEditTaskDescription->text().toStdString());

    ImagingPipeline *impipe(0);
    PulsarPipeline *pulsepipe(0);
    LongBaselinePipeline *lbpipe(0);
    CalibrationPipeline *calpipe = dynamic_cast<CalibrationPipeline *>(itsTask);
    if (calpipe) {
        calpipe->setSkyModel(ui.lineEditCalibrationSkyModel->text());
        if (ui.groupBoxDemixing->isEnabled()) {
            DemixingSettings demix_settings;
            demix_settings.itsDemixingEnabled = true;
            demix_settings.itsDemixAlways = ui.listWidgetDemixAlways->checkedItemsAsString();
            demix_settings.itsDemixIfNeeded = ui.listWidgetDemixIfNeeded->checkedItemsAsString();
            demix_settings.itsDemixFreqStep = ui.spinBoxDemixFreqStep->value();
            demix_settings.itsDemixTimeStep = ui.spinBoxDemixTimeStep->value();
            demix_settings.itsFreqStep = ui.spinBoxAveragingFreqStep->value();
            demix_settings.itsTimeStep = ui.spinBoxAveragingTimeStep->value();
            demix_settings.itsSkyModel = ui.lineEditDemixSkyModel->text();
            calpipe->setDemixingSettings(demix_settings);
        }
    }
    else if ((impipe = dynamic_cast<ImagingPipeline *>(itsTask))) {
        impipe->setSpecifyFov(ui.checkBoxSpecifyFOV->isChecked());
        impipe->setFov(ui.lineEditFieldOfView->text().toDouble());
        impipe->setCellSize(ui.lineEditCellSize->text());
        impipe->setNrOfPixels(ui.spinBoxNumberOfPixels->value());
        impipe->setSlicesPerImage(ui.spinBoxSlicesPerImage->value());
        impipe->setSubbandsPerImage(ui.spinBoxSubbandsPerImage->value());
    }

    else if ((pulsepipe = dynamic_cast<PulsarPipeline *>(itsTask))) {
        pulsepipe->setNoRFI(ui.checkBox_NoRFI->isChecked());
        pulsepipe->setSkipDspsr(ui.checkBox_No_DSPSR->isChecked());
        pulsepipe->setNoFold(ui.checkBox_No_fold->isChecked());
        pulsepipe->setNoPdmp(ui.checkBox_No_pdmp->isChecked());
        pulsepipe->setRawTo8Bit(ui.checkBox_RawTo8Bit->isChecked());
        pulsepipe->setRRATS(ui.checkBox_rrats->isChecked());
        pulsepipe->setSinglePulse(ui.checkBox_Single_pulse->isChecked());
        pulsepipe->setSkipDynamicSpectrum(ui.checkBox_Skip_dynamic_spectrum->isChecked());
        pulsepipe->setSkipPrepfold(ui.checkBox_Skip_prepfold->isChecked());
        pulsepipe->setTwoBf2fitsExtra(ui.lineEdit2bf2fitsExtraOptions->text());
        pulsepipe->setDigifilExtra(ui.lineEditDigifilExtraOptions->text());
        pulsepipe->setDspsrExtra(ui.lineEditDSPSRextraOptions->text());
        pulsepipe->setPrepDataExtra(ui.lineEditPrepdataExtraOptions->text());
        pulsepipe->setPrepFoldExtra(ui.lineEditPrepfoldExtraOptions->text());
        pulsepipe->setPrepSubbandExtra(ui.lineEditPrepsubbandExtraOptions->text());
        pulsepipe->setPulsarName(ui.lineEditPulsarName->text());
        pulsepipe->setRFIfindExtra(ui.lineEditRfiFindExtraOptions->text());
        pulsepipe->setDecodeNblocks(ui.spinBoxDecodeNblocks->value());
        pulsepipe->setDecodeSigma(ui.spinBoxDecodeSigma->value());
        pulsepipe->setTsubInt(ui.spinBoxTsubint->value());
        pulsepipe->setEightBitConversionSigma(ui.doubleSpinBox8BitConversionSigma->value());
        pulsepipe->setDynamicSpectrumAvg(ui.doubleSpinBoxDynamicSpectrumTimeAverage->value());
        pulsepipe->setRratsDmRange(ui.doubleSpinBoxRratsDmRange->value());
    }
    else if ((lbpipe = dynamic_cast<LongBaselinePipeline *>(itsTask))) {
        lbpipe->setSubbandGroupsPerMS(ui.spinBoxSubbandGroupsPerMS->value());
        lbpipe->setSubbandsPerSubbandGroup(ui.spinBoxSubbandsPerSubbandGroup->value());
    }

	// scheduled start and stop times
	const AstroDate &latestDate = Controller::theSchedulerSettings.getLatestSchedulingDay();

	QDateTime currentTime = QDateTime::currentDateTimeUtc();
    QDateTime st(ui.dateTimeEditScheduledStart->dateTime());
	AstroDateTime now = AstroDateTime(currentTime.date().day(), currentTime.date().month(), currentTime.date().year(),
			currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second());
	AstroDateTime start = AstroDateTime(st.date().day(), st.date().month(), st.date().year(),
			st.time().hour(), st.time().minute(), st.time().second());

	if ((start >= now) || (state < Task::PRESCHEDULED)) {
        if (ui.dateTimeEditScheduledEnd->date() <= QDate(latestDate.getYear(), latestDate.getMonth(), latestDate.getDay())) {
            if (!ui.dateTimeEditScheduledStart->isUndefined()) {
                pipeline->setScheduledStart(ui.dateTimeEditScheduledStart->dateTime());
			}
            if (!ui.dateTimeEditScheduledEnd->isUndefined()) {
                pipeline->setScheduledEnd(ui.dateTimeEditScheduledEnd->dateTime());
			}
		}
		else {
			QMessageBox::warning(this, tr("Task planned outside schedule"), tr("The task cannot be planned outside of the schedule.\nPlease correct the scheduled start and/or end dates"));
			return false;
		}
	}
	else {
		QMessageBox::warning(this, tr("Task planned in the past"), tr("You cannot schedule a task in the past.\nPlease correct the scheduled start date"));
		return false;
	}

	// task duration
    if (!ui.lineEditDuration->isUndefined()) {
        pipeline->setDuration(ui.lineEditDuration->text());
	}

	//priority
    pipeline->setPriority(ui.lineEditPriority->text().toDouble());

	// group ID
    pipeline->setGroupID(ui.lineEditGroupID->text().toUInt());

	// predecessor
    if (!ui.lineEditPredecessors->text().isEmpty()) {
        pipeline->setPredecessors(ui.lineEditPredecessors->text());
        pipeline->setPredecessorMinTimeDif(ui.lineEditMinPredDistance->text().toStdString());
        pipeline->setPredecessorMaxTimeDif(ui.lineEditMaxPredDistance->text().toStdString());
	} else {
        pipeline->clearPredecessors();
	}

    pipeline->setWindowFirstDay(ui.dateEditFirstPossibleDate->date());
    pipeline->setWindowLastDay(ui.dateEditLastPossibleDate->date());

    pipeline->setWindowMinTime(ui.timeEditFirstPossibleTime->time());
    pipeline->setWindowMaxTime(ui.timeEditLastPossibleTime->time());

    pipeline->setFixDay(ui.checkBoxFixedDate->isChecked());
    pipeline->setFixTime(ui.checkBoxFixedTime->isChecked());

    TaskStorage *pipe_storage(pipeline->storage());

    updateEnabledInputDataTypes(); // update the enabled input data product types in itsTask according to the current enabled types in the inputDataTreeWidget
    pipe_storage->setInputFilesToBeProcessed(getInputDataFilesCheckState());

	checkForRemovalOfDataProducts(itsOutputDataTypes); // checks if data products have been switched off by the user and remove them from itsTmpStorage if needed
    storage_selection_mode newStorageSelectionMode(static_cast<storage_selection_mode>(ui.comboBoxStorageSelectionMode->currentIndex()));
    if (newStorageSelectionMode != pipe_storage->getStorageSelectionMode()) {
        itsTmpStorage.clear();
        pipe_storage->unAssignStorage();
		if (newStorageSelectionMode == STORAGE_MODE_MANUAL) {
            getDisplayedStorageLocations();
		}
	}
    pipe_storage->setStorageSelectionMode(newStorageSelectionMode);
    pipe_storage->setStorage(itsTmpStorage);
    checkStorageSettingsEnable(); // enable the storage tree according to the current status which might have been changed

    //	pipeline->setAutoSelectStorage(ui.checkBox_AutomaticSelectionStorageNodes->isChecked());

    // add the end of making changes to a task hat potentially change its resources always call recalculateCheck
//	pipeline->recalculateCheck();

    if (addingPipeline) {
        itsController->createPipeline(pipeline);
		addingPipeline = false;
	}
    else if (itsController->updatePipelineTask(pipeline, true)) {
		enableApplyButtons(false);
	}

	return true;
}

bool TaskDialog::commitReservation(bool storeValues = false) {
	QString strValue;
	addingTask = false;
    strValue = ui.lineEditTaskName->text();
/*
	if (strValue.length() < 3) {
		ui.tabWidgetMain->setCurrentWidget(ui.tab_TaskSettings);
        ui.lineEditTaskName->setFocus();
		QMessageBox::warning(this, tr("Task name not valid"), tr("The task name must contain at least three characters"));
		return false;
	}
*/
    itsTask->setTaskName(strValue.toStdString());
    itsTask->setProjectName(ui.lineEdit_ProjectName->text().toStdString());
    itsTask->setProjectCO_I(ui.lineEdit_ProjectCOI->text().toStdString());
    itsTask->setProjectID(ui.comboBoxProjectID->currentText().toStdString());
    itsTask->setProjectPI(ui.lineEdit_ProjectPI->text().toStdString());
    itsTask->setContactEmail(ui.lineEdit_ContactEmail->text().toStdString());
    itsTask->setContactName(ui.lineEdit_ContactName->text().toStdString());
    itsTask->setContactPhone(ui.lineEdit_ContactPhone->text().toStdString());
    itsTask->setTaskDescription(ui.lineEditTaskDescription->text().toStdString());

    itsTask->setType(ui.comboBoxProcessType->currentText(),
            stringToProcessSubType(ui.comboBoxProcessSubType->currentText()),
            ui.comboBoxStrategies->currentText());
    itsTask->setOriginalTreeID(ui.lineEditOriginalTreeID->text().toInt());
    itsTask->setStatus(ui.comboBoxTaskStatus->currentText().toStdString());

	// scheduled start and stop times
//	const AstroDate &earliestDate = Controller::theSchedulerSettings.getEarliestSchedulingDay();
//	const AstroDate &latestDate = Controller::theSchedulerSettings.getLatestSchedulingDay();

/*
	if (addingReservation) {
        if (!(ui.dateTimeEditScheduledStart->isUndefined()) & !(ui.dateTimeEditScheduledEnd->isUndefined())) {
            if (ui.dateTimeEditScheduledEnd->date() <= QDate(latestDate.getYear(), latestDate.getMonth(), latestDate.getDay())) {
                itsTask->setScheduledStart(ui.dateTimeEditScheduledStart->dateTime());
                itsTask->setScheduledEnd(ui.dateTimeEditScheduledEnd->dateTime());
			}
			else {
				QMessageBox::warning(this, tr("Task planned outside schedule"), tr("The task may not be planned outside of the schedule.\nPlease correct the scheduled start and/or end dates"));
				return false;
			}
		}
	}
    else if (ui.dateTimeEditScheduledStart->date() >= QDate(earliestDate.getYear(), earliestDate.getMonth(), earliestDate.getDay()) &&
            (ui.dateTimeEditScheduledEnd->date() <= QDate(latestDate.getYear(), latestDate.getMonth(), latestDate.getDay()))) {
        if (!ui.dateTimeEditScheduledStart->isUndefined()) {
            itsTask->setScheduledStart(ui.dateTimeEditScheduledStart->dateTime());
		}
        if (!ui.dateTimeEditScheduledEnd->isUndefined()) {
            itsTask->setScheduledEnd(ui.dateTimeEditScheduledEnd->dateTime());
		}
	}
	else {
		QMessageBox::warning(this, tr("Task planned outside schedule"), tr("The task may not be planned outside of the schedule.\nPlease correct the scheduled start and/or end dates"));
		return false;
	}
	*/
	// scheduled start
    if (!ui.dateTimeEditScheduledStart->isUndefined()) {
        itsTask->setScheduledStart(ui.dateTimeEditScheduledStart->dateTime());
	}
	// scheduled end
    if (!ui.dateTimeEditScheduledEnd->isUndefined()) {
        itsTask->setScheduledEnd(ui.dateTimeEditScheduledEnd->dateTime());
	}
	// task duration
    if (!ui.lineEditDuration->isUndefined()) {
        itsTask->setDuration(ui.lineEditDuration->text());
	}

    StationTask *stationTask(static_cast<StationTask *>(itsTask)); // a resevation is per definition a stationTask

	//stations
	const std::vector<std::string> &stations = getAssignedStationNames();
	if (!stations.empty()) {
        stationTask->setStations(stations);
	}
	else {
		ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
        ui.listWidgetAvailableStations->setFocus();
		QMessageBox::warning(this, tr("No stations selected"), tr("At least one station needs to be selected"));
		return false;
	}


    if (!itsTask->isMaintenance()) { // for maintenance no checking and no further properties are set
        stationTask->setAntennaMode(antenna_modes_str[ui.comboBoxStationAntennaMode->currentIndex()]);
        stationTask->setFilterType(filter_types_str[ui.comboBoxStationFilter->currentIndex()]);
        stationTask->setStationClock(clock_frequencies_str[ui.comboBoxStationClock->currentIndex()]);

//        obs->setSuperStations(getSuperStations(obs)); // the call to getSuperStations must be done after setting the task's stations

        if (!checkEmail()) {
			return false;
		}
	}

	if (addingReservation) {
		if (!itsController->createReservation(itsTask)) {
			return false;
		}
		else {
			addingReservation = false;
			return true;
		}
	}
	else {
		addingReservation = false;
		if (storeValues) {
			StoreValues();
		}
		enableApplyButtons(false);
		itsController->updateTask(itsTask, true);
	}
	resetChangeDetection();
	return true;
}


bool TaskDialog::commitChanges(bool storeValues = false) {
	int dummy(0);
	QString strValue;
	bool unAssignStorage(false);

	// first check if this is an ABORT task commit in which case only the status may be changed and no further changes are allowed
    if (ui.comboBoxTaskStatus->currentText().compare("ABORTED") != 0) {


        strValue = ui.lineEditTaskName->text();
        itsTask->setTaskName(strValue.toStdString());
        itsTask->setProjectName(ui.lineEdit_ProjectName->text().toStdString());
        itsTask->setProjectCO_I(ui.lineEdit_ProjectCOI->text().toStdString());
        itsTask->setProjectID(ui.comboBoxProjectID->currentText().toStdString());
        itsTask->setProjectPI(ui.lineEdit_ProjectPI->text().toStdString());

        itsTask->setType(ui.comboBoxProcessType->currentText(),
                stringToProcessSubType(ui.comboBoxProcessSubType->currentText()),
                ui.comboBoxStrategies->currentText());
        itsTask->setOriginalTreeID(ui.lineEditOriginalTreeID->text().toInt());

        Task::task_status  state = taskStatusFromString(ui.comboBoxTaskStatus->currentText().toStdString());
        itsTask->setStatus(state);

        itsTask->setContactEmail(ui.lineEdit_ContactEmail->text().toStdString());
        itsTask->setContactName(ui.lineEdit_ContactName->text().toStdString());
        itsTask->setContactPhone(ui.lineEdit_ContactPhone->text().toStdString());
        itsTask->setTaskDescription(ui.lineEditTaskDescription->text().toStdString());

		// scheduled start and stop times
		//	const AstroDate &earliestDate = Controller::theSchedulerSettings.getEarliestSchedulingDay();
		const AstroDate &latestDate = Controller::theSchedulerSettings.getLatestSchedulingDay();

		QDateTime currentTime = QDateTime::currentDateTimeUtc();
        QDateTime st(ui.dateTimeEditScheduledStart->dateTime());
		AstroDateTime now = AstroDateTime(currentTime.date().day(), currentTime.date().month(), currentTime.date().year(),
				currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second());
		AstroDateTime start = AstroDateTime(st.date().day(), st.date().month(), st.date().year(),
				st.time().hour(), st.time().minute(), st.time().second());

		if ((start >= now) || (state < Task::PRESCHEDULED)) {
            if (ui.dateTimeEditScheduledEnd->date() <= QDate(latestDate.getYear(), latestDate.getMonth(), latestDate.getDay())) {

                if (!ui.dateTimeEditScheduledStart->isUndefined()) {
                    AstroDateTime prevStart(itsTask->getScheduledStart());
                    itsTask->setScheduledStart(ui.dateTimeEditScheduledStart->dateTime());
                    if (prevStart != itsTask->getScheduledStart()) unAssignStorage = true;
				}
                if (!ui.dateTimeEditScheduledEnd->isUndefined()) {
                    AstroDateTime prevEnd(itsTask->getScheduledEnd());
                    itsTask->setScheduledEnd(ui.dateTimeEditScheduledEnd->dateTime());
                    if (prevEnd != itsTask->getScheduledEnd()) unAssignStorage = true;
				}
			}
			else {
                QMessageBox::critical(this, tr("Task scheduled outside schedule"), tr("The task cannot be scheduled outside of the schedule.\nPlease correct the scheduled start and/or end dates"));
				return false;
			}
		}
		else {
            QMessageBox::critical(this, tr("Task scheduled in the past"), tr("You cannot schedule a task in the past or too close to now.\nPlease correct the scheduled start date"));
			return false;
		}

		// task duration
        if (!ui.lineEditDuration->isUndefined()) {
            itsTask->setDuration(ui.lineEditDuration->text());
		}

        if (itsTask->isStationTask()) {
            StationTask *stationtask(static_cast<StationTask *>(itsTask));
            //stations
            const std::vector<std::string> &stations = getAssignedStationNames();
            if (!stations.empty()) {
                taskStationsMap prevStations(stationtask->getStations());
                stationtask->setStations(stations);
                if (prevStations != stationtask->getStations()) unAssignStorage = true;
            }
            else {
                ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
                ui.listWidgetAvailableStations->setFocus();
                QMessageBox::critical(this, tr("No stations selected"), tr("At least one station needs to be selected"));
                return false;
            }

            int mode = ui.comboBoxStationAntennaMode->currentIndex();
            if (mode) {
                QString modeStr = ui.comboBoxStationAntennaMode->currentText();
                if (modeStr.startsWith("HBA")) { // check if Analog beam is set
                    if ((fabs(itsAnalogBeamSettings.angle1.radian()) < std::numeric_limits<double>::epsilon()) &
                            (fabs(itsAnalogBeamSettings.angle2.radian()) < std::numeric_limits<double>::epsilon()) &
                            (!itsAnalogBeamSettings.startTime.isSet()) &
                            (!itsAnalogBeamSettings.duration.isSet())) {
                        QApplication::beep();
                        ui.tabWidgetMain->setCurrentWidget(ui.tab_StationBeams);
                        ui.lineEditAnalogBeamAngle1->setFocus();
                        if (QMessageBox::question(this, tr(""),
                                                  tr("For a HBA antenna mode the analog beam needs to be set. It appears not to be set.\nDo you want to continue?"),
                                                  QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) == QMessageBox::Cancel) {
                            return false;
                        }
                    }
                }
                station_antenna_mode prevAntMode(stationtask->getAntennaMode());
                stationtask->setAntennaMode(antenna_modes_str[mode]);
                station_antenna_mode antMode(stationtask->getAntennaMode());
                if (antMode != prevAntMode) unAssignStorage = true;
            }
            else {
                ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
                ui.comboBoxStationAntennaMode->setFocus();
                QMessageBox::critical(this, tr("Station mode not set"), tr("The stations mode must be specified."));
                return false;
            }
            int filter = ui.comboBoxStationFilter->currentIndex();
            if (filter) {
                station_filter_type prevFilter(stationtask->getFilterType());
                stationtask->setFilterType(filter_types_str[filter]);
                if (prevFilter != stationtask->getFilterType()) unAssignStorage = true;
            }
            else {
                ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
                ui.comboBoxStationFilter->setFocus();
                QMessageBox::critical(this, tr("Station filter not set"), tr("The stations filter type must be specified."));
                return false;
            }

            int clock = ui.comboBoxStationClock->currentIndex();
            if (clock) {
                station_clock prevClock(stationtask->getStationClock());
                stationtask->setStationClock(clock_frequencies_str[clock]);
                if (prevClock != stationtask->getStationClock()) unAssignStorage = true;
            }
            else {
                ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
                ui.comboBoxStationClock->setFocus();
                QMessageBox::critical(this, tr("Station clock not set"), tr("The stations clock frequency must be specified."));
                return false;
            }


            if (stationtask->isObservation()) {
                Observation *obs(static_cast<Observation *>(stationtask));

                superStationMap prevSuperStations(obs->getSuperStations());
                obs->setSuperStations(getSuperStations(obs)); // the call to getSuperStations must be done after setting the task's stations
                if (prevSuperStations != obs->getSuperStations()) unAssignStorage = true;

                obs->setNrOfDataslotsPerRSPboard(ui.spinBoxDataslotsPerRSPboard->value());
                obs->setTBBPiggybackAllowed(ui.checkBoxTBBPiggybackAllowed->isChecked());
                obs->setAartfaacPiggybackAllowed(ui.checkBoxAartfaacPiggybackAllowed->isChecked());

                // beam settings
                itsTempAnalogBeamSettings = getAnalogBeamSettings(); // itsTempAnalogBeamSettings is used for change detection
                obs->setAnalogBeamSettings(itsAnalogBeamSettings);

                itsTempDigitalBeams = itsDigitalBeams; // itsTempDigitalBeams is used for change detection
                obs->setDigitalBeams(itsDigitalBeams);

                Observation::RTCPsettings prevRTCP(obs->getRTCPsettings());
                if (ui.spinBoxCoherentChannelsPerSubband->value() == 0) {
                    QMessageBox::critical(this, tr("Wrong value"), tr("Coherent channels per subband setting may not be zero"));
                    return false;
                }
                if (ui.spinBoxIncoherentChannelsPerSubband->value() == 0) {
                    QMessageBox::critical(this, tr("Wrong value"), tr("Incoherent channels per subband setting may not be zero"));
                    return false;
                }
                obs->setRTCPsettings(getRTCPSettings());

                if (prevRTCP != obs->getRTCPsettings()) unAssignStorage = true;

            } // end if = Observation
        } // end if = stationTask

        TaskStorage *task_storage(itsTask->storage());
        if (task_storage) {
            if (task_storage->getOutputDataProductsEnabled() != itsOutputDataTypes) {
                unAssignStorage = true;
            }

            if (unAssignStorage) {
                itsOutputDataTypes.coherentStokesAssigned = false;
                itsOutputDataTypes.correlatedAssigned = false;
                itsOutputDataTypes.incoherentStokesAssigned = false;
                itsOutputDataTypes.instrumentModelAssigned = false;
                itsOutputDataTypes.complexVoltagesAssigned = false;
                itsOutputDataTypes.skyImageAssigned = false;
                task_storage->unAssignStorage();
            }
            task_storage->setEnabledOutputDataProducts(itsOutputDataTypes);
            checkForRemovalOfDataProducts(itsOutputDataTypes); // checks if data products have been switched off by the user and remove them from itsTmpStorage if needed

            storage_selection_mode newStorageSelectionMode(static_cast<storage_selection_mode>(ui.comboBoxStorageSelectionMode->currentIndex()));
            if (newStorageSelectionMode != task_storage->getStorageSelectionMode()) {
                itsTmpStorage.clear();
                unAssignStorage = true;
                if (newStorageSelectionMode == STORAGE_MODE_MANUAL) {
                    getDisplayedStorageLocations();
                }
            }
            task_storage->setStorageSelectionMode(newStorageSelectionMode);
            task_storage->setStorage(itsTmpStorage);

        }
        checkStorageSettingsEnable(); // enable the storage tree according to the current status which might have been changed

        itsTask->setGroupID(ui.lineEditGroupID->text().toUInt());

		//priority
        QDoubleValidator dValidator(ui.lineEditPriority);
        strValue = ui.lineEditPriority->text();
		dValidator.setRange(0.0,10.0,20);
		if (dValidator.validate(strValue, dummy) == QValidator::Acceptable) {
            itsTask->setPriority(string2Double(strValue.toStdString()));
		}
		else {
			ui.tabWidgetMain->setCurrentWidget(ui.tab_TaskSettings);
            ui.lineEditPriority->setFocus();
            QMessageBox::critical(this, tr("Wrong priority value"), tr("The priority value must be in the [0,10] range"));
			return false;
		}

		// predecessor
        if (!ui.lineEditPredecessors->text().isEmpty()) {
            itsTask->setPredecessors(ui.lineEditPredecessors->text());
            itsTask->setPredecessorMinTimeDif(ui.lineEditMinPredDistance->text().toStdString());
            itsTask->setPredecessorMaxTimeDif(ui.lineEditMaxPredDistance->text().toStdString());
		} else {
            itsTask->clearPredecessors();
		}

        itsTask->setWindowFirstDay(ui.dateEditFirstPossibleDate->date());
        itsTask->setWindowLastDay(ui.dateEditLastPossibleDate->date());

        itsTask->setWindowMinTime(ui.timeEditFirstPossibleTime->time());
        itsTask->setWindowMaxTime(ui.timeEditLastPossibleTime->time());

        itsTask->setFixDay(ui.checkBoxFixedDate->isChecked());
        itsTask->setFixTime(ui.checkBoxFixedTime->isChecked());

        if (!checkEmail()) {
			return false;
		}

		// add the end of making changes to a task hat potentially change its resources always call recalculateCheck
//		itsTask->recalculateCheck();

		if (addingTask) {
			resetChangeDetection();
            if (!itsController->createTask(*itsTask)) {
				return false;
			}
			else {
				addingTask = false;
				return true;
			}
		}
		else {
			if (storeValues) {
				StoreValues();
			}
		}
        if (itsController->updateTask(itsTask, true)) {
			enableApplyButtons(false);
			return true;
		}
		else return false;
	} // task status is set to ABORTED
	else { // user put task on ABORT no other changes are allowed here to the task
        if (itsTask->getStatus() == Task::ACTIVE) {
			enableApplyButtons(false);
            emit abortTask(itsTask->getID());
			return true;
		}
		else {
            QMessageBox::critical(this, tr("Task is not ACTIVE"), "Task:" + QString::number(itsTask->getID()) + " is not ACTIVE. Could not ABORT the task");
			return false;
		}
	}
}

bool TaskDialog::commitMultiTasks(void) {
	if (!itsMultiTasks.empty()) {
	QString strValue;

    bool applyTaskName(false), applyProjectID(false), applyPredecessorID(false),
            applyPredecessorMinTimeDif(false), applyPredecessorMaxTimeDif(false), applyAnalogAngle1(false), applyAnalogAngle2(false), applyAnalogCoordinateSystem(false), /*applyDigitalBeamSettings(false), clearDigitalBeams(false),*/
            applyFirstPossibleDate(false), applyLastPossibleDate(false), applyFirstPossibleTime(false), applyLastPossibleTime(false), applyStatus(false),
            applyStations(false), applyScheduledStart(false), applyScheduledEnd(false), applyDuration(false), applyDescription(false), applyAntennaMode(false), applyStationFilter(false), applyStationClock(false),
            applyFlysEye(false), applyChannelsPerSubband(false), applyCoherentChannelsPerSubband(false), applyIncoherentChannelsPerSubband(false), applyBitsPerSample(false),
            applyCoherentStokesType(false), applyCoherentIntegration(false), applyIncoherentIntegration(false), applyIncoherentStokesType(false),
            applyCoherentSubbandPerFile(false), applyIncoherentSubbandPerFile(false), applyNrDataSlotsPerRSPBoard(false), applyTBBpiggyback(false), applyAartfaacPiggyback(false),
            applyCorrelatorIntegrationTime(false), applyReservation(false), applyProcessType(false), applyProcessSubtype(false), applyStrategy(false), applyStorageSelectMode(false),
            applyCorrelated(false), applyCoherent(false), applyIncoherent(false), applyBandpassCorrection(false), applyDelayCompensation(false),
            applyCoherentDedispersion(false), applyGroupID(false), applyDemixAlways(false), applyDemixIfNeeded(false), applyDemixFreqStep(false), applyDemixTimeStep(false),
            applyAvgFreqStep(false), applyAvgTimeStep(false), applyDemixSkyModel(false), applyCalibrationSkyModel(false), applySpecifyFOV(false), applyFOV(false), applyCellSize(false),
            applyNrOfPixels(false), applySlicesPerImage(false), applySubbandsPerImage(false), applySubbandGroupsPerMS(false), applySubbandsPerSubbandGroup(false),
            applyPulsar_noRFI(false), applyPulsar_noDSPSR(false), applyPulsar_noFold(false), applyPulsar_noPDMP(false), applyPulsar_rawTo8Bit(false), applyPulsar_RRATS(false),
            applyPulsar_singlePulse(false), applyPulsar_skipDynamicSpectrum(false), applyPulsar_skipPrepfold(false), applyPulsar_twoBf2fitsExtra(false), applyPulsar_digifilExtra(false),
            applyPulsar_dspsrExtra(false), applyPulsar_prepDataExtra(false), applyPulsar_prepFoldExtra(false), applyPulsar_prepSubbandExtra(false), applyPulsar_PulsarName(false),
            applyPulsar_rfiFindExtra(false), applyPulsar_decodeNblocks(false), applyPulsar_decodeSigma(false), applyPulsar_tsubint(false), applyPulsar_8bitconvSigma(false),
            applyPulsar_dynamicSpectrumAvg(false), applyPulsar_rratsDMRange(false);


    strValue = ui.lineEditTaskName->text();

    if (ui.lineEditTaskDescription->hasBeenChanged()) applyDescription = true;
    if (ui.lineEditTaskName->hasBeenChanged()) applyTaskName = true;

	unsigned int reservationID(0), groupID(0);
    if (ui.comboBoxReservation->hasBeenChanged()) {
        reservationID = ui.comboBoxReservation->itemData(ui.comboBoxReservation->currentIndex()).toUInt();
		applyReservation = true;
	}

    if (ui.lineEditGroupID->hasBeenChanged()) {
        groupID = ui.lineEditGroupID->text().toUInt();
		applyGroupID = true;
	}

    applyPredecessorID = ui.lineEditPredecessors->hasBeenChanged();
    applyProjectID = ui.comboBoxProjectID->hasBeenChanged();
    applyDuration = ui.lineEditDuration->hasBeenChanged();
    applyScheduledStart = ui.dateTimeEditScheduledStart->hasBeenChanged();
    applyScheduledEnd = ui.dateTimeEditScheduledEnd->hasBeenChanged();

	AstroTime predMinTimeDif, predMaxTimeDif;
    if (ui.lineEditMinPredDistance->hasBeenChanged()) {
        predMinTimeDif = AstroTime(ui.lineEditMinPredDistance->text().toStdString());
		applyPredecessorMinTimeDif = true;
	}
    if (ui.lineEditMaxPredDistance->hasBeenChanged()) {
        predMaxTimeDif = AstroTime(ui.lineEditMaxPredDistance->text().toStdString());
		applyPredecessorMaxTimeDif = true;
	}

	AstroDate firstPossibleDate, lastPossibleDate;
	AstroTime firstPossibleTime, lastPossibleTime;
    if (ui.dateEditFirstPossibleDate->hasBeenChanged()) {
        firstPossibleDate = AstroDate(ui.dateEditFirstPossibleDate->text().toStdString());
		applyFirstPossibleDate = true;
	}
    if (ui.dateEditLastPossibleDate->hasBeenChanged()) {
        lastPossibleDate = AstroDate(ui.dateEditLastPossibleDate->text().toStdString());
		applyLastPossibleDate = true;
	}
    if (ui.timeEditFirstPossibleTime->hasBeenChanged()) {
        firstPossibleTime = AstroTime(ui.timeEditFirstPossibleTime->text().toStdString());
		applyFirstPossibleTime = true;
	}
    if (ui.timeEditLastPossibleTime->hasBeenChanged()) {
        lastPossibleTime = AstroTime(ui.timeEditLastPossibleTime->text().toStdString());
		applyLastPossibleTime = true;
	}

	std::string statusStr;
    if (ui.comboBoxTaskStatus->hasBeenChanged()) {
        statusStr = ui.comboBoxTaskStatus->currentText().toStdString();
		applyStatus = true;
	}

	QString processType, strategy;
	processSubTypes processSubtype(PST_UNKNOWN);
	Task::task_type type(Task::UNKNOWN);
    if (ui.comboBoxProcessType->hasBeenChanged()) {
        type = static_cast<Task::task_type>(ui.comboBoxProcessType->currentIndex());
        processType = ui.comboBoxProcessType->currentText();
		applyProcessType = true;
	}
    if (ui.comboBoxProcessSubType->hasBeenChanged()) {
        processSubtype = stringToProcessSubType(ui.comboBoxProcessSubType->currentText());
		applyProcessSubtype = true;
	}
    if (ui.comboBoxStrategies->hasBeenChanged()) {
        strategy = ui.comboBoxStrategies->currentText();
		applyStrategy = true;
	}

    applyTBBpiggyback = ui.checkBoxTBBPiggybackAllowed->hasBeenChanged();
    applyAartfaacPiggyback = ui.checkBoxAartfaacPiggybackAllowed->hasBeenChanged();

    // analog beam coordinate system
	beamDirectionType analogBeamDirectionType(DIR_TYPE_J2000);
    if (ui.comboBoxAnalogBeamCoordinates->hasBeenChanged()) {
        analogBeamDirectionType = stringToBeamDirectionType(ui.comboBoxAnalogBeamCoordinates->currentText().toStdString());
		applyAnalogCoordinateSystem = true;
	}

	// analog beam settings
    applyAnalogAngle1 = ui.lineEditAnalogBeamAngle1->hasBeenChanged();
    applyAnalogAngle2 = ui.lineEditAnalogBeamAngle2->hasBeenChanged();

	anglePairs analogUnits(ANGLE_PAIRS_HMS_DMS);
	if (applyAnalogAngle1 || applyAnalogAngle2) {
        const QString &unitStr(ui.comboBoxAnalogBeamUnits->currentText());
		if (unitStr.compare(ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]) == 0) analogUnits = ANGLE_PAIRS_HMS_DMS;
		else if (unitStr.compare(ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]) == 0) analogUnits = ANGLE_PAIRS_DMS_DMS;
		else if (unitStr.compare(ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]) == 0) analogUnits = ANGLE_PAIRS_DECIMAL_DEGREES;
		else analogUnits = ANGLE_PAIRS_RADIANS;
	}


	double correlatorIntegrationTime(1.0);
    if (ui.lineEditCorrelatorIntegrationTime->hasBeenChanged()) {
        correlatorIntegrationTime = ui.lineEditCorrelatorIntegrationTime->text().toDouble();
		applyCorrelatorIntegrationTime = true;
	}

    applyCorrelated = ui.checkBoxCorrelatedData->hasBeenChanged();
    applyCoherent = ui.checkBoxCoherentStokes->hasBeenChanged();
    applyIncoherent = ui.checkBoxIncoherentStokes->hasBeenChanged();
    applyFlysEye = ui.checkBoxPencilFlysEye->hasBeenChanged();
    applyCoherentDedispersion = ui.checkBox_CoherentDedispersion->hasBeenChanged();
    applyBandpassCorrection = ui.checkBox_BandpassCorrection->hasBeenChanged();
    applyDelayCompensation = ui.checkBox_DelayCompensation->hasBeenChanged();

    if (ui.comboBoxBitsPerSample->hasBeenChanged()) {
		applyBitsPerSample = true;
		applyNrDataSlotsPerRSPBoard = true;
	}

    applyChannelsPerSubband = ui.spinBoxChannelsPerSubband->hasBeenChanged();
    applyCoherentStokesType = ui.comboBoxCoherentStokesType->hasBeenChanged();
    applyIncoherentStokesType = ui.comboBoxIncoherentStokesType->hasBeenChanged();
    applyCoherentIntegration = ui.spinBoxCoherentTimeIntegration->hasBeenChanged();
    applyIncoherentIntegration = ui.spinBoxIncoherentTimeIntegration->hasBeenChanged();
    applyCoherentSubbandPerFile = ui.spinBoxCoherentSubbandsPerFile->hasBeenChanged();
    applyIncoherentSubbandPerFile = ui.spinBoxIncoherentSubbandsPerFile->hasBeenChanged();
    applyCoherentChannelsPerSubband = ui.spinBoxCoherentChannelsPerSubband->hasBeenChanged();
    applyIncoherentChannelsPerSubband = ui.spinBoxIncoherentChannelsPerSubband->hasBeenChanged();

	std::vector<std::string> stations;
    if (ui.treeWidgetUsedStations->hasChanged()) {
		stations = getAssignedStationNames();
		applyStations = true;
	}

	// antenna mode
	station_antenna_mode antenna_mode(UNSPECIFIED_ANTENNA_MODE);
    if (ui.comboBoxStationAntennaMode->hasBeenChanged()) {
        antenna_mode = static_cast<station_antenna_mode>(ui.comboBoxStationAntennaMode->currentIndex());
		applyAntennaMode = true;
	}

	// station filter
	int filter(0);
    if (ui.comboBoxStationFilter->hasBeenChanged()) {
        filter = ui.comboBoxStationFilter->currentIndex();
		applyStationFilter = true;
	}

	// station clock

	int clock(0);
    if (ui.comboBoxStationClock->hasBeenChanged()) {
        clock = ui.comboBoxStationClock->currentIndex();
		applyStationClock = true;
	}

    applyDemixAlways = ui.listWidgetDemixAlways->hasBeenChanged();
    applyDemixIfNeeded = ui.listWidgetDemixIfNeeded->hasBeenChanged();
    applyDemixFreqStep = ui.spinBoxDemixFreqStep->hasBeenChanged();
    applyDemixTimeStep = ui.spinBoxDemixTimeStep->hasBeenChanged();
    applyAvgFreqStep = ui.spinBoxAveragingFreqStep->hasBeenChanged();
    applyAvgTimeStep = ui.spinBoxAveragingTimeStep->hasBeenChanged();
    applyDemixSkyModel = ui.lineEditDemixSkyModel->hasBeenChanged();
    applyCalibrationSkyModel = ui.lineEditCalibrationSkyModel->hasBeenChanged();
    applySpecifyFOV = ui.checkBoxSpecifyFOV->hasBeenChanged();

    applyFOV = ui.lineEditFieldOfView->hasBeenChanged();
    applyCellSize = ui.lineEditCellSize->hasBeenChanged();
    applyNrOfPixels = ui.spinBoxNumberOfPixels->hasBeenChanged();
    applySlicesPerImage = ui.spinBoxSlicesPerImage->hasBeenChanged();
    applySubbandsPerImage = ui.spinBoxSubbandsPerImage->hasBeenChanged();
    applySubbandGroupsPerMS = ui.spinBoxSubbandGroupsPerMS->hasBeenChanged();
    applySubbandsPerSubbandGroup = ui.spinBoxSubbandsPerSubbandGroup->hasBeenChanged();

    applyPulsar_noRFI = ui.checkBox_NoRFI->hasBeenChanged();
    applyPulsar_noDSPSR = ui.checkBox_No_DSPSR->hasBeenChanged();
    applyPulsar_noFold = ui.checkBox_No_fold->hasBeenChanged();
    applyPulsar_noPDMP = ui.checkBox_No_pdmp->hasBeenChanged();
    applyPulsar_rawTo8Bit = ui.checkBox_RawTo8Bit->hasBeenChanged();
    applyPulsar_RRATS = ui.checkBox_rrats->hasBeenChanged();
    applyPulsar_singlePulse = ui.checkBox_Single_pulse->hasBeenChanged();
    applyPulsar_skipDynamicSpectrum = ui.checkBox_Skip_dynamic_spectrum->hasBeenChanged();
    applyPulsar_skipPrepfold = ui.checkBox_Skip_prepfold->hasBeenChanged();
    applyPulsar_twoBf2fitsExtra = ui.lineEdit2bf2fitsExtraOptions->hasBeenChanged();

    applyPulsar_digifilExtra = ui.lineEditDigifilExtraOptions->hasBeenChanged();
    applyPulsar_dspsrExtra = ui.lineEditDSPSRextraOptions->hasBeenChanged();
    applyPulsar_prepDataExtra = ui.lineEditPrepdataExtraOptions->hasBeenChanged();
    applyPulsar_prepFoldExtra = ui.lineEditPrepfoldExtraOptions->hasBeenChanged();
    applyPulsar_prepSubbandExtra = ui.lineEditPrepsubbandExtraOptions->hasBeenChanged();
    applyPulsar_PulsarName = ui.lineEditPulsarName->hasBeenChanged();
    applyPulsar_rfiFindExtra = ui.lineEditRfiFindExtraOptions->hasBeenChanged();
    applyPulsar_decodeNblocks = ui.spinBoxDecodeNblocks->hasBeenChanged();

    applyPulsar_decodeSigma = ui.spinBoxDecodeSigma->hasBeenChanged();
    applyPulsar_tsubint = ui.spinBoxTsubint->hasBeenChanged();
    applyPulsar_8bitconvSigma = ui.doubleSpinBox8BitConversionSigma->hasBeenChanged();
    applyPulsar_dynamicSpectrumAvg = ui.doubleSpinBoxDynamicSpectrumTimeAverage->hasBeenChanged();
    applyPulsar_rratsDMRange = ui.doubleSpinBoxRratsDmRange->hasBeenChanged();

    // storage settings
    if (ui.comboBoxStorageSelectionMode->hasBeenChanged() || itsStorageOverride) {
        applyStorageSelectMode = true;
    }

	// have the controller store an undo level of the complete schedule
	itsController->storeScheduleUndo("Multi-edit of tasks");

	// now loop through the tasks to apply these changes
	bool change;
	// prerequisites for applying changes

	QString tmpStr;
	bool changesMade(false);
    StationTask *statTask(0);
    Observation *obs(0);
    CalibrationPipeline *calpipe(0);
    ImagingPipeline *impipe(0);
    PulsarPipeline *pulspipe(0);
    LongBaselinePipeline *lbpipe(0);
    for (std::vector<Task *>::iterator it = itsMultiTasks.begin(); it != itsMultiTasks.end(); ++it) {

		change = false;
		if (applyDuration) {
            (*it)->setDuration(ui.lineEditDuration->text());
			change = true;
		}
		if (applyScheduledStart) {
            (*it)->setScheduledStart(ui.dateTimeEditScheduledStart->dateTime());
            change = true;
		}
		if (applyScheduledEnd) {
            (*it)->setScheduledEnd(ui.dateTimeEditScheduledEnd->dateTime());
            change = true;
		}
		if (applyTaskName) {
            (*it)->setTaskName(ui.lineEditTaskName->text().toStdString());
			change = true;
		}
		if (applyGroupID) {
            (*it)->setGroupID(groupID);
			change = true;
		}
		if (applyDescription) {
			tmpStr = ui.lineEditTaskDescription->text();
            tmpStr = tmpStr.replace(MULTIPLE_VALUE_TEXT,(*it)->getTaskDescription());
            (*it)->setTaskDescription(tmpStr.toStdString());
			change = true;
		}
		if (applyProjectID) {
            (*it)->setProjectID(ui.comboBoxProjectID->currentText().toStdString());
			change = true;
		}
		if (applyPredecessorID) {
			// TODO: IMPORTANT the predecessors line edit is not filled in multiedit causing it to be set empty when applying multi-edit
            (*it)->setPredecessors(ui.lineEditPredecessors->text());
			change = true;
		}
		if (applyPredecessorMinTimeDif) {
            (*it)->setPredecessorMinTimeDif(predMinTimeDif);
			change = true;
		}
		if (applyPredecessorMaxTimeDif) {
            (*it)->setPredecessorMaxTimeDif(predMaxTimeDif);
			change = true;
		}
		if (applyFirstPossibleDate) {
            (*it)->setWindowFirstDay(firstPossibleDate);
			change = true;
		}
		if (applyLastPossibleDate) {
            (*it)->setWindowLastDay(lastPossibleDate);
			change = true;
		}
		if (applyFirstPossibleTime) {
            (*it)->setWindowMinTime(firstPossibleTime);
			change = true;
		}
		if (applyLastPossibleTime) {
            (*it)->setWindowMaxTime(lastPossibleTime);
			change = true;
		}
		if (applyStatus) {
            (*it)->setStatus(statusStr);
			change = true;
		}
		if (applyProcessType) {
            (*it)->setType(type);
            (*it)->setProcessType(processType);
			change = true;
		}
		if (applyProcessSubtype) {
            (*it)->setProcessSubtype(processSubtype);
			change = true;
		}
		if (applyStrategy) {
            (*it)->setStrategy(strategy);
			change = true;
		}


        statTask = dynamic_cast<StationTask *>(*it);
        if (statTask) {
            if (applyStations) {
                if (!stations.empty()) {
                    statTask->setStations(stations);
                    change = true;
                }
                else {
                    ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
                    ui.listWidgetAvailableStations->setFocus();
                    QMessageBox::warning(this, tr("No stations selected"), tr("At least one station needs to be selected"));
                    return false;
                }
            }
            if (applyAntennaMode) {
                statTask->setAntennaMode(antenna_mode);
                change = true;
            }
            if (applyStationFilter) {
                statTask->setFilterType(filter_types_str[filter]);
                change = true;
            }
            if (applyStationClock) {
                statTask->setStationClock(clock_frequencies_str[clock]);
                change = true;
            }

            obs = dynamic_cast<Observation *>(*it);
            if (obs) {
                if (applyReservation) {
                    obs->setReservation(reservationID);
                    change = true;
                }
                if (applyTBBpiggyback) {
                    obs->setTBBPiggybackAllowed(ui.checkBoxTBBPiggybackAllowed->isChecked());
                    change = true;
                }
                if (applyAartfaacPiggyback) {
                    obs->setAartfaacPiggybackAllowed(ui.checkBoxAartfaacPiggybackAllowed->isChecked());
                    change = true;
                }
                if (applyAnalogCoordinateSystem) {
                    obs->setAnalogBeamDirectionType(analogBeamDirectionType);
                }

                // apply analog beam settings
                if (applyAnalogAngle1) {
                    Angle angle1;
                    switch (analogUnits) {
                    case ANGLE_PAIRS_HMS_DMS:
                        angle1.setHMSangleStr(ui.lineEditAnalogBeamAngle1->text().toStdString());
                        break;
                    case ANGLE_PAIRS_DMS_DMS:
                        angle1.setDMSangleStr(ui.lineEditAnalogBeamAngle1->text().toStdString());
                        break;
                    case ANGLE_PAIRS_RADIANS:
                        angle1.setRadianAngle(ui.lineEditAnalogBeamAngle1->text().toDouble());
                        break;
                    case ANGLE_PAIRS_DECIMAL_DEGREES:
                        angle1.setDegreeAngle(ui.lineEditAnalogBeamAngle1->text().toDouble());
                        break;
                    default:
                        break;
                    }
                    obs->setAnalogBeamAngle1(angle1);
                    change = true;
                }
                if (applyAnalogAngle2) {
                    Angle angle2;
                    switch (analogUnits) {
                    case ANGLE_PAIRS_HMS_DMS:
                        angle2.setHMSangleStr(ui.lineEditAnalogBeamAngle2->text().toStdString());
                        break;
                    case ANGLE_PAIRS_DMS_DMS:
                        angle2.setDMSangleStr(ui.lineEditAnalogBeamAngle2->text().toStdString());
                        break;
                    case ANGLE_PAIRS_RADIANS:
                        angle2.setRadianAngle(ui.lineEditAnalogBeamAngle2->text().toDouble());
                        break;
                    case ANGLE_PAIRS_DECIMAL_DEGREES:
                        angle2.setDegreeAngle(ui.lineEditAnalogBeamAngle2->text().toDouble());
                        break;
                    default:
                        break;
                    }
                    obs->setAnalogBeamAngle2(angle2);
                    change = true;
                }

                if (applyNrDataSlotsPerRSPBoard) {
                    obs->setNrOfDataslotsPerRSPboard(ui.spinBoxDataslotsPerRSPboard->value());
                    change = true;
                }

                if (applyFlysEye) {
                    obs->setFlysEye(ui.checkBoxPencilFlysEye->isChecked());
                    change = true;
                }

                if (applyCoherentDedispersion) {
                    obs->setCoherentDedispersion(ui.checkBox_CoherentDedispersion->isChecked());
                    change = true;
                }

                if (applyBandpassCorrection) {
                    obs->setBandPassCorrection(ui.checkBox_BandpassCorrection->isChecked());
                    change = true;
                }

                if (applyDelayCompensation) {
                    obs->setDelayCompensation(ui.checkBox_DelayCompensation->isChecked());
                    change = true;
                }

                if (applyCorrelatorIntegrationTime) {
                    obs->setCorrelatorIntegrationTime(correlatorIntegrationTime);
                    change = true;
                }

                if (applyChannelsPerSubband) {
                    obs->setChannelsPerSubband(ui.spinBoxChannelsPerSubband->value());
                    change = true;
                }

                if (applyBitsPerSample) {
                    obs->setBitsPerSample(ui.comboBoxBitsPerSample->currentText().toUInt());
                    change = true;
                }

                if (applyCoherentStokesType) {
                    obs->setCoherentDataType(stringToStokesType(ui.comboBoxCoherentStokesType->currentText().toStdString()));
                    change = true;
                }

                if (applyIncoherentStokesType) {
                    obs->setIncoherentDataType(stringToStokesType(ui.comboBoxIncoherentStokesType->currentText().toStdString()));
                    change = true;
                }

                if (applyCoherentIntegration) {
                    obs->setCoherentTimeIntegration(ui.spinBoxCoherentTimeIntegration->value());
                    change = true;
                }

                if (applyIncoherentIntegration) {
                    obs->setIncoherentTimeIntegration(ui.spinBoxIncoherentTimeIntegration->value());
                    change = true;
                }

                if (applyCoherentChannelsPerSubband) {
                    obs->setCoherentChannelsPerSubband(ui.spinBoxCoherentChannelsPerSubband->value());
                    change = true;
                }

                if (applyIncoherentChannelsPerSubband) {
                    obs->setIncoherentChannelsPerSubband(ui.spinBoxIncoherentChannelsPerSubband->value());
                    change = true;
                }

                if (applyCoherentSubbandPerFile) {
                    obs->setCoherentSubbandsPerFile(ui.spinBoxCoherentSubbandsPerFile->value());
                    change = true;
                }

                if (applyIncoherentSubbandPerFile) {
                    obs->setIncoherentSubbandsPerFile(ui.spinBoxIncoherentSubbandsPerFile->value());
                    change = true;
                }
            } // end if (*it)->isObservation()
        } // end if (*it)->isStationTask()
        else if ((*it)->isPipeline()) {
            if ((calpipe = dynamic_cast<CalibrationPipeline *>(*it))) {
                if (applyCalibrationSkyModel) {
                    calpipe->setSkyModel(ui.lineEditCalibrationSkyModel->text());
                    change = true;
                }

                DemixingSettings &demix(calpipe->demixingSettings());
                if (applyDemixAlways) {
                    demix.setDemixAlways(ui.listWidgetDemixAlways->checkedItemsAsString());
                    change = true;
                }

                if (applyDemixIfNeeded) {
                    demix.setDemixIfNeeded(ui.listWidgetDemixIfNeeded->checkedItemsAsString());
                    change = true;
                }

                if (applyDemixFreqStep) {
                    demix.setDemixFreqStep((quint16)ui.spinBoxDemixFreqStep->value());
                    change = true;
                }

                if (applyDemixTimeStep) {
                    demix.setDemixTimeStep((quint16)ui.spinBoxDemixTimeStep->value());
                    change = true;
                }

                if (applyAvgFreqStep) {
                    demix.setAvgFreqStep((quint16)ui.spinBoxAveragingFreqStep->value());
                    change = true;
                }

                if (applyAvgTimeStep) {
                    demix.setAvgTimeStep((quint16)ui.spinBoxAveragingTimeStep->value());
                    change = true;
                }

                if (applyDemixSkyModel) {
                    demix.setDemixSkyModel(ui.lineEditDemixSkyModel->text());
                    change = true;
                }
            }

            else if ((impipe = dynamic_cast<ImagingPipeline *>(*it))) {
                if (applySpecifyFOV) {
                    impipe->setSpecifyFov(ui.checkBoxSpecifyFOV->isChecked());
                    change = true;
                }

                if (applyFOV) {
                    impipe->setFov(ui.lineEditFieldOfView->text().toDouble());
                    change = true;
                }

                if (applyCellSize) {
                    impipe->setCellSize(ui.lineEditCellSize->text());
                    change = true;
                }

                if (applyNrOfPixels) {
                    impipe->setNrOfPixels((quint16)ui.spinBoxNumberOfPixels->value());
                    change = true;
                }

                if (applySlicesPerImage) {
                    impipe->setSlicesPerImage((quint16)ui.spinBoxSlicesPerImage->value());
                    change = true;
                }

                if (applySubbandsPerImage) {
                    impipe->setSubbandsPerImage((quint16)ui.spinBoxSubbandsPerImage->value());
                    change = true;
                }
            }

            else if ((pulspipe = dynamic_cast<PulsarPipeline *>(*it))) {
                if (applyPulsar_noRFI) {
                    pulspipe->setNoRFI(ui.checkBox_NoRFI->isChecked());
                    change = true;
                }
                if (applyPulsar_noDSPSR) {
                    pulspipe->setSkipDspsr(ui.checkBox_No_DSPSR->isChecked());
                    change = true;
                }
                if (applyPulsar_noFold) {
                    pulspipe->setNoFold(ui.checkBox_No_fold->isChecked());
                    change = true;
                }
                if (applyPulsar_noPDMP) {
                    pulspipe->setNoPdmp(ui.checkBox_No_pdmp->isChecked());
                    change = true;
                }
                if (applyPulsar_rawTo8Bit) {
                    pulspipe->setRawTo8Bit(ui.checkBox_RawTo8Bit->isChecked());
                    change = true;
                }
                if (applyPulsar_RRATS) {
                    pulspipe->setRRATS(ui.checkBox_rrats->isChecked());
                    change = true;
                }
                if (applyPulsar_singlePulse) {
                    pulspipe->setSinglePulse(ui.checkBox_Single_pulse->isChecked());
                    change = true;
                }
                if (applyPulsar_skipDynamicSpectrum) {
                    pulspipe->setSkipDynamicSpectrum(ui.checkBox_Skip_dynamic_spectrum->isChecked());
                    change = true;
                }
                if (applyPulsar_skipPrepfold) {
                    pulspipe->setSkipPrepfold(ui.checkBox_Skip_prepfold->isChecked());
                    change = true;
                }
                if (applyPulsar_twoBf2fitsExtra) {
                    pulspipe->setTwoBf2fitsExtra(ui.lineEdit2bf2fitsExtraOptions->text());
                }
                if (applyPulsar_digifilExtra) {
                    pulspipe->setDigifilExtra(ui.lineEditDigifilExtraOptions->text());
                }
                if (applyPulsar_dspsrExtra) {
                    pulspipe->setDspsrExtra(ui.lineEditDSPSRextraOptions->text());
                }
                if (applyPulsar_prepDataExtra) {
                    pulspipe->setPrepDataExtra(ui.lineEditPrepdataExtraOptions->text());
                }
                if (applyPulsar_prepFoldExtra) {
                    pulspipe->setPrepFoldExtra(ui.lineEditPrepfoldExtraOptions->text());
                }
                if (applyPulsar_prepSubbandExtra) {
                    pulspipe->setPrepSubbandExtra(ui.lineEditPrepsubbandExtraOptions->text());
                }
                if (applyPulsar_PulsarName) {
                    pulspipe->setPulsarName(ui.lineEditPulsarName->text());
                }
                if (applyPulsar_rfiFindExtra) {
                    pulspipe->setRFIfindExtra(ui.lineEditRfiFindExtraOptions->text());
                }
                if (applyPulsar_decodeNblocks) {
                    pulspipe->setDecodeNblocks(ui.spinBoxDecodeNblocks->value());
                }
                if (applyPulsar_decodeSigma) {
                    pulspipe->setDecodeSigma(ui.spinBoxDecodeSigma->value());
                }
                if (applyPulsar_tsubint) {
                    pulspipe->setTsubInt(ui.spinBoxTsubint->value());
                }
                if (applyPulsar_8bitconvSigma) {
                    pulspipe->setEightBitConversionSigma(ui.doubleSpinBox8BitConversionSigma->value());
                }
                if (applyPulsar_dynamicSpectrumAvg) {
                    pulspipe->setDynamicSpectrumAvg(ui.doubleSpinBoxDynamicSpectrumTimeAverage->value());
                }
                if (applyPulsar_rratsDMRange) {
                    pulspipe->setRratsDmRange(ui.doubleSpinBoxRratsDmRange->value());
                }
            }

            else if ((lbpipe = dynamic_cast<LongBaselinePipeline *>(*it))) {
                if (applySubbandGroupsPerMS) {
                    lbpipe->setSubbandGroupsPerMS(ui.spinBoxSubbandGroupsPerMS->value());
                }
                if (applySubbandsPerSubbandGroup) {
                    lbpipe->setSubbandsPerSubbandGroup(ui.spinBoxSubbandsPerSubbandGroup->value());
                }
            }
        }

        TaskStorage *task_storage((*it)->storage());
        if (task_storage) {
            if (applyCorrelated) {
                task_storage->setOutputDataProductEnabled(DP_CORRELATED_UV, ui.checkBoxCorrelatedData->isChecked());
                change = true;
            }
            if (applyCoherent) {
                task_storage->setOutputDataProductEnabled(DP_COHERENT_STOKES, ui.checkBoxCoherentStokes->isChecked());
                change = true;
            }
            if (applyIncoherent) {
                task_storage->setOutputDataProductEnabled(DP_INCOHERENT_STOKES, ui.checkBoxIncoherentStokes->isChecked());
                change = true;
            }
            if (applyStorageSelectMode) {
                task_storage->setStorageSelectionMode(static_cast<storage_selection_mode>(ui.comboBoxStorageSelectionMode->currentIndex()));
                change = true;
            }

            if (itsStorageOverride) { // manual override of storage nodes in multi-edit mode is active
                task_storage->setStorage(itsTmpStorage);
                change = true;
            }
            else if (static_cast<storage_selection_mode>(ui.comboBoxStorageSelectionMode->currentIndex()) == STORAGE_MODE_MANUAL) {
                getDisplayedStorageLocations();
                if (itsTmpStorage != task_storage->getStorageLocations()) {
                    task_storage->setStorage(itsTmpStorage);
                    change = true;
                }
            }
        }

		if (change) {
            changesMade |= itsController->updateTask(*it, false);
		}
	} // END for loop through tasks
	if (!changesMade) {
		itsController->deleteLastStoredUndo();
	}
	itsController->updateStatusBar();
	resetChangeDetection();
	enableApplyButtons(false);
	}
	return true;
}

void TaskDialog::setVisibleTabs(const QVector<tabIndex> &tabs) {
    ui.tabWidgetMain->clear();
    if (tabs.contains(TAB_SCHEDULE)) ui.tabWidgetMain->insertTab(TAB_SCHEDULE, ui.tab_TaskSettings, tab_names[TAB_SCHEDULE]);
    if (tabs.contains(TAB_STATION_SETTINGS)) ui.tabWidgetMain->insertTab(TAB_STATION_SETTINGS, ui.tab_StationSettings, tab_names[TAB_STATION_SETTINGS]);
    if (tabs.contains(TAB_STATION_BEAMS)) ui.tabWidgetMain->insertTab(TAB_STATION_BEAMS, ui.tab_StationBeams, tab_names[TAB_STATION_BEAMS]);
    if (tabs.contains(TAB_PROCESSING)) ui.tabWidgetMain->insertTab(TAB_PROCESSING, ui.tab_Processing, tab_names[TAB_PROCESSING]);
    if (tabs.contains(TAB_STORAGE)) ui.tabWidgetMain->insertTab(TAB_STORAGE, ui.tab_Storage, tab_names[TAB_STORAGE]);
    if (tabs.contains(TAB_PIPELINE)) ui.tabWidgetMain->insertTab(TAB_PIPELINE, ui.tab_Pipeline, tab_names[TAB_PIPELINE]);
    if (tabs.contains(TAB_PULSAR_PIPELINE)) ui.tabWidgetMain->insertTab(TAB_PULSAR_PIPELINE, ui.tab_PulsarPipeline, tab_names[TAB_PULSAR_PIPELINE]);
    if (tabs.contains(TAB_LONGBASELINE_PIPELINE)) ui.tabWidgetMain->insertTab(TAB_LONGBASELINE_PIPELINE, ui.tab_LongBaselinePipeline, tab_names[TAB_LONGBASELINE_PIPELINE]);
    if (tabs.contains(TAB_EXTRA_INFO)) ui.tabWidgetMain->insertTab(TAB_EXTRA_INFO, ui.tab_ExtraInfo, tab_names[TAB_EXTRA_INFO]);
}

void TaskDialog::enableTabs(void) {
	// tabs:
	// 0 Schedule
	// 1 Station settings
	// 2 Station beams
	// 3 Processing
	// 4 Storage
	// 5 Pipeline
    // 6 Pulsar Pipeline
    // 7 Extra Info
	setUpdatesEnabled(false);
	blockSignals(true);
    ui.tabWidgetMain->blockSignals(true);
    QVector<tabIndex> tabs;
    if (isMultiTasks) {
        tabs << TAB_SCHEDULE << TAB_STATION_SETTINGS << TAB_STATION_BEAMS << TAB_PROCESSING << TAB_STORAGE
             << TAB_PIPELINE << TAB_PULSAR_PIPELINE << TAB_LONGBASELINE_PIPELINE << TAB_EXTRA_INFO;
    }
	else {
        const Pipeline *pipeline(0);
        switch(itsTask->getType()) {
		case Task::OBSERVATION:
            tabs << TAB_SCHEDULE << TAB_STATION_SETTINGS << TAB_STATION_BEAMS << TAB_PROCESSING << TAB_STORAGE << TAB_EXTRA_INFO;
            break;
		case Task::RESERVATION:
            tabs << TAB_SCHEDULE << TAB_STATION_SETTINGS << TAB_STORAGE << TAB_EXTRA_INFO;
			break;
		case Task::MAINTENANCE:
            tabs << TAB_SCHEDULE << TAB_STATION_SETTINGS << TAB_EXTRA_INFO;
			break;
		case Task::PIPELINE:
            pipeline = dynamic_cast<const Pipeline *>(itsTask);
            if (pipeline) {
                tabs << TAB_SCHEDULE << TAB_STORAGE;
                if (pipeline->isPulsarPipeline()) {
                    tabs << TAB_PULSAR_PIPELINE;
                }
                else if (pipeline->isLongBaselinePipeline()) {
                    tabs << TAB_LONGBASELINE_PIPELINE;
                }
                else {
                    tabs << TAB_PIPELINE;
                }
            }
            tabs << TAB_EXTRA_INFO;
            break;
        case Task::SYSTEM:
            tabs << TAB_SCHEDULE << TAB_EXTRA_INFO;
            break;
        default: // unknown task type enable all tabs
            tabs << TAB_SCHEDULE << TAB_STATION_SETTINGS << TAB_STATION_BEAMS << TAB_PROCESSING << TAB_STORAGE
                 << TAB_PIPELINE << TAB_PULSAR_PIPELINE << TAB_LONGBASELINE_PIPELINE << TAB_EXTRA_INFO;

            break;
		}
    }
    setVisibleTabs(tabs);

    ui.tabWidgetMain->blockSignals(false);
	blockSignals(false);
	setUpdatesEnabled(true);
}

void TaskDialog::addTask(unsigned taskID) {
	addingTask = true;
    blockChangeDetection = true;
	addingReservation = false;
    delete itsTask;
    itsTask = new Observation(taskID); // initializes the internal task object with a clean observation
	itsOutputDataTypes.correlated = false;
	itsOutputDataTypes.coherentStokes = false;
	itsOutputDataTypes.incoherentStokes = false;
	itsOutputDataTypes.instrumentModel = false;
	itsOutputDataTypes.skyImage = false;
    processTypeChanged(0);
	ui.comboBoxStorageDataType->clear();
    itsTask->setStatus(Task::PRESCHEDULED); // set the status of newly added tasks in the scheduler directly to PRESCHEDULED
	isMultiTasks = false;
	ui.pushButtonOk->setEnabled(true);
	this->setWindowTitle(QString("Add task ") + QString::number(taskID));
	ui.pushButtonOk->setText("Add");

	ui.pushButtonApply->hide();

	setNormalTaskMode(); // enable/disable specific settings
	enableTabs();

	loadReservations();
	setExistingProjects(Controller::theSchedulerSettings.getCampaignList());

	ui.checkBoxCorrelatedData->setTristate(false);
	ui.checkBoxCoherentStokes->setTristate(false);
	ui.checkBoxIncoherentStokes->setTristate(false);
	ui.checkBox_DelayCompensation->setTristate(false);
	ui.checkBox_BandpassCorrection->setTristate(false);
	ui.checkBoxPencilFlysEye->setTristate(false);
	ui.checkBox_CoherentDedispersion->setTristate(false);

	ui.pushButtonShowDataSlots->setEnabled(false);

    ui.dateEditFirstPossibleDate->setUndefined(false);
    ui.timeEditFirstPossibleTime->setUndefined(false);
    ui.dateEditLastPossibleDate->setUndefined(false);
    ui.timeEditLastPossibleTime->setUndefined(false);
    ui.dateTimeEditScheduledStart->setUndefined(false);
    ui.dateTimeEditScheduledEnd->setUndefined(false);

    ui.comboBoxTaskStatus->blockSignals(true);
    ui.comboBoxTaskStatus->clear();
    ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
    ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
    ui.comboBoxTaskStatus->addItem(task_states_str[Task::SCHEDULED]);
    ui.comboBoxTaskStatus->setCurrentIndex(1); // PRESCHEDULED
    ui.comboBoxTaskStatus->blockSignals(false);

    ui.lineEditPredecessors->setText("");
    ui.lineEditGroupID->setText("");
	enablePredecessorSettings(false);

	// set project info to "no campaign"
	ui.lineEdit_ProjectName->clear();
	ui.lineEdit_ProjectCOI->clear();
	ui.lineEdit_ProjectPI->clear();
	ui.lineEdit_ContactEmail->clear();
	ui.lineEdit_ContactPhone->clear();
	ui.lineEdit_ContactName->clear();
    ui.comboBoxProjectID->setFromString("no campaign");
    ui.lineEditTaskName->clear();
	ui.lineEditTaskDescription->clear();
	ui.lineEditTotalSubbands->clear();
	ui.lineEditTaskID->setText(QString::number(taskID));

    ui.comboBoxStationClock->setCurrentIndex(0);

    ui.comboBoxStationAntennaMode->blockSignals(true);
    ui.comboBoxStationAntennaMode->setCurrentIndex(0);
    ui.comboBoxStationAntennaMode->blockSignals(false);

    ui.comboBoxStationFilter->setCurrentIndex(0);

    ui.spinBoxDataslotsPerRSPboard->setValue(MAX_DATASLOT_PER_RSP_16_BITS + 1);

	//stations
    loadAvailableStations();
    ui.labelAssignedStations->setText("Assigned stations (0)");
    ui.treeWidgetUsedStations->clear();

	// processType
    itsTask->setProcessType(task_types_str[Task::OBSERVATION]);
    itsTask->setProcessSubtype(PST_BEAM_OBSERVATION);
    itsTask->setStrategy("default");
    setProcessSubProcessStrategy(itsTask);

    ui.comboBoxProcessType->setEnabled(true);
    ui.comboBoxProcessSubType->setEnabled(true);
    ui.comboBoxStrategies->setEnabled(true);

    ui.comboBoxProcessType->setToolTip("");
    ui.comboBoxProcessSubType->setToolTip("");
    ui.comboBoxStrategies->setToolTip("");

	QDateTime startTime = QDateTime::currentDateTime().toUTC();
	startTime = startTime.addSecs(630); // add the task 10.5 minutes from now and round to start at complete minutes
	startTime.setTime(QTime(startTime.time().hour(), startTime.time().minute()));
	AstroDateTime stime(startTime);
	const AstroDate &earliestDay = Controller::theSchedulerSettings.getEarliestSchedulingDay();
	if ((stime < earliestDay) | (stime > Controller::theSchedulerSettings.getLatestSchedulingDay())) {
		startTime = QDateTime(QDate(earliestDay.getYear(), earliestDay.getMonth(), earliestDay.getDay()), QTime());
	}
	setScheduledStart(startTime);
    ui.lineEditDuration->setText("0001:00:00");
	setScheduledEnd(startTime.addSecs(3600));

	const AstroDate &fdate = Controller::theSchedulerSettings.getEarliestSchedulingDay();
    ui.dateEditFirstPossibleDate->setDate(QDate(fdate.getYear(), fdate.getMonth(), fdate.getDay()));
	const AstroDate &ldate = Controller::theSchedulerSettings.getLatestSchedulingDay();
    ui.dateEditLastPossibleDate->setDate(QDate(ldate.getYear(), ldate.getMonth(), ldate.getDay()));
    ui.timeEditFirstPossibleTime->setTime(QTime(0,0,0));
    ui.timeEditLastPossibleTime->setTime(QTime(23,59,59));

    ui.checkBoxFixedDate->setTristate(false);
    ui.checkBoxFixedDate->setChecked(false);
    ui.checkBoxFixedTime->setTristate(false);
    ui.checkBoxFixedTime->setChecked(false);

    ui.lineEditPriority->setText("1.0");

	// clear analog beam settings
	itsAnalogBeamSettings.angle1.setRadianAngle(0.0);
	itsAnalogBeamSettings.angle2.setRadianAngle(0.0);
	itsAnalogBeamSettings.directionType = DIR_TYPE_J2000;
	itsAnalogBeamAnglePair = ANGLE_PAIRS_HMS_DMS;
    itsAnalogBeamSettings.duration.clearTime();
    itsAnalogBeamSettings.startTime.clearTime();
	setAnalogBeamSettings(itsAnalogBeamSettings);

	// clear digital beam settings
	clearAllDigitalBeams();
	itsDigitalBeams.clear();
	ui.tableWidgetDigitalBeams->clearContents();
	ui.tableWidgetDigitalBeams->setRowCount(0);
	ui.pushButtonEditBeam->setText("Edit");
	ui.pushButtonEditBeam->setEnabled(false);
	ui.pushButtonAddBeam->setEnabled(true);
	ui.pushButtonDeleteBeams->setEnabled(false);
	ui.pushButtonClearAllBeams->setEnabled(false);
	itsDigitalBeamDialog->setReadOnly(false);

	// clear tied array beam setings
//	itsTiedArrayBeams.clear();
	ui.tableWidgetTiedArrayBeams->clearContents();
	ui.tableWidgetTiedArrayBeams->setRowCount(0);
	ui.pushButtonAddTiedArrayBeam->setEnabled(false);
	ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
	ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
	ui.pushButtonEditTiedArrayBeam->setEnabled(false);
	itsTiedArrayBeamDialog->setReadOnly(false);

	// processing tab default settings
    ui.checkBoxCorrelatedData->setChecked(false);
	ui.checkBoxCoherentStokes->setChecked(false);
	ui.checkBoxIncoherentStokes->setChecked(false);
	ui.checkBox_BandpassCorrection->setChecked(true);
	ui.checkBox_DelayCompensation->setChecked(true);
	ui.checkBox_CoherentDedispersion->setEnabled(false);
	ui.checkBox_CoherentDedispersion->setChecked(true);
	ui.checkBoxPencilFlysEye->setEnabled(false);
	ui.checkBoxPencilFlysEye->setChecked(false);
    ui.checkBoxTBBPiggybackAllowed->setChecked(true);
    ui.checkBoxAartfaacPiggybackAllowed->setChecked(true);
    ui.comboBoxBitsPerSample->setCurrentIndex(2); // 16 bits per sample
    ui.comboBoxCoherentStokesType->blockSignals(true);
    ui.comboBoxCoherentStokesType->setCurrentIndex(0); // I - type
    ui.comboBoxCoherentStokesType->setEnabled(false);
    ui.comboBoxCoherentStokesType->blockSignals(false);
    ui.comboBoxIncoherentStokesType->blockSignals(true);
    ui.comboBoxIncoherentStokesType->setCurrentIndex(0); // I - type
    ui.comboBoxIncoherentStokesType->setEnabled(false);
    ui.comboBoxIncoherentStokesType->blockSignals(false);
    ui.spinBoxCoherentTimeIntegration->blockSignals(true);
    ui.spinBoxCoherentTimeIntegration->setValue(1);
    ui.spinBoxCoherentTimeIntegration->setEnabled(false);
    ui.spinBoxCoherentTimeIntegration->blockSignals(false);
    ui.spinBoxIncoherentTimeIntegration->blockSignals(true);
    ui.spinBoxIncoherentTimeIntegration->setValue(1);
    ui.spinBoxIncoherentTimeIntegration->setEnabled(false);
    ui.spinBoxIncoherentTimeIntegration->blockSignals(false);
    ui.spinBoxCoherentChannelsPerSubband->blockSignals(true);
    ui.spinBoxCoherentChannelsPerSubband->setMinimum(1);
    ui.spinBoxCoherentChannelsPerSubband->setValue(1);
    ui.spinBoxCoherentChannelsPerSubband->setEnabled(false);
    ui.spinBoxCoherentChannelsPerSubband->blockSignals(false);
    ui.spinBoxIncoherentChannelsPerSubband->blockSignals(true);
    ui.spinBoxIncoherentChannelsPerSubband->setEnabled(false);
    ui.spinBoxIncoherentChannelsPerSubband->setMinimum(1);
    ui.spinBoxIncoherentChannelsPerSubband->setValue(1);
    ui.spinBoxIncoherentChannelsPerSubband->blockSignals(false);
    ui.spinBoxCoherentSubbandsPerFile->blockSignals(true);
    ui.spinBoxCoherentSubbandsPerFile->setValue(MAX_DATASLOTS_16_BITS);
    ui.spinBoxCoherentSubbandsPerFile->setEnabled(false);
    ui.spinBoxCoherentSubbandsPerFile->blockSignals(false);
    ui.spinBoxIncoherentSubbandsPerFile->blockSignals(true);
    ui.spinBoxIncoherentSubbandsPerFile->setValue(MAX_DATASLOTS_16_BITS);
    ui.spinBoxIncoherentSubbandsPerFile->setEnabled(false);
    ui.spinBoxIncoherentSubbandsPerFile->blockSignals(false);
    ui.lineEditCorrelatorIntegrationTime->setText("1.0");
    ui.spinBoxChannelsPerSubband->setValue(64); // defaults to 64 channels per subband
	ui.tabWidgetMain->setCurrentWidget(ui.tab_TaskSettings);

	ui.pushButtonOk->setEnabled(true);
	ui.pushButtonCancelClose->setText("Cancel");
	ui.pushButtonCancelClose->setEnabled(true);

	// storage
	itsTmpStorage.clear();
	updateStorageTree();
	ui.lineEditStorageAssigned->setText("No storage set");
	ui.lineEditStorageAssigned->setPalette(palette());
	ui.treeWidgetInputDataProducts->clear();
	ui.treeWidgetOutputDataProducts->clear();
    ui.comboBoxStorageSelectionMode->setCurrentIndex(1);
	this->hide();
	this->setModal(true);
	this->showNormal();
	this->raise();
	this->activateWindow();
    blockChangeDetection = true;
}

/*
void TaskDialog::addReservation(unsigned taskID) {
	addingReservation = true;
	setReservationTaskMode();

	if (!itsStationsLoaded) {
	    loadAvailableStations();
	}
	ui.tabWidgetMain->setCurrentWidget(ui.tab_TaskSettings);

	ui.pushButtonApply->hide();
	ui.pushButtonOk->setText("Add");
	ui.pushButtonOk->setEnabled(true);
	ui.pushButtonCancelClose->setText("Cancel");
	ui.pushButtonCancelClose->setEnabled(true);

    blockChangeDetection = true;
}
*/

void TaskDialog::setReservationTaskMode(void) {
	isMultiTasks = false;

	ui.groupBoxAnalogBeam->setEnabled(false);
	ui.tab_TaskSettings->setEnabled(true);
	ui.tab_ExtraInfo->setEnabled(true);

    ui.comboBoxTaskStatus->clear();
    ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
    ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
    ui.comboBoxTaskStatus->setCurrentIndex(0); // PRESCHEDULED

    ui.lineEditPriority->setEnabled(false);
	ui.labelPriority->setEnabled(false);
    ui.lineEditPredecessors->setEnabled(false);
	ui.labelPredecessor->setEnabled(false);
	enablePredecessorSettings(false);
    ui.dateEditFirstPossibleDate->setEnabled(false);
    ui.labelFirstPossibleDate->setEnabled(false);
    ui.dateEditLastPossibleDate->setEnabled(false);
    ui.labelLastPossibleDate->setEnabled(false);
    ui.timeEditFirstPossibleTime->setEnabled(false);
    ui.labelFirstPossibleTime->setEnabled(false);
    ui.timeEditLastPossibleTime->setEnabled(false);
    ui.labelLastPossibleTime->setEnabled(false);
    ui.checkBoxFixedDate->setEnabled(false);
    ui.checkBoxFixedTime->setEnabled(false);
    ui.comboBoxReservation->setEnabled(false);
	ui.labelBindToReservation->setEnabled(false);
	ui.pushButtonUnBindReservation->setEnabled(false);

    ui.comboBoxStorageSelectionMode->setEnabled(false);
	ui.comboBoxStorageDataType->setEnabled(false);

	// tab stations
    ui.checkBoxTBBPiggybackAllowed->setEnabled(false);
    ui.checkBoxAartfaacPiggybackAllowed->setEnabled(false);
	ui.tab_StationSettings->setEnabled(true);
    ui.comboBoxStationAntennaMode->setEnabled(true);
    ui.comboBoxStationClock->setEnabled(true);
    ui.comboBoxStationFilter->setEnabled(true);
    ui.listWidgetAvailableStations->setEnabled(true);
    ui.treeWidgetUsedStations->setEnabled(true);
	ui.pushButtonAddSuperStation->setEnabled(true);
}

void TaskDialog::setNormalTaskMode(void) {
	loadReservations();

	// tab Schedule
    ui.lineEditTaskName->setEnabled(true);
	ui.pushButtonUpdateCampaigns->setEnabled(true);
	ui.labelTaskName->setEnabled(true);
    ui.comboBoxProjectID->setEnabled(true);
	ui.labelProjectName->setEnabled(true);
    ui.labelDuration->setEnabled(true);
    ui.lineEditDuration->setEnabled(true);
    ui.dateTimeEditScheduledStart->setEnabled(true);
    ui.labelScheduledStart->setEnabled(true);
    ui.dateTimeEditScheduledEnd->setEnabled(true);
    ui.labelScheduledEnd->setEnabled(true);
    ui.lineEditPriority->setEnabled(true);
	ui.labelPriority->setEnabled(true);
    ui.lineEditGroupID->setEnabled(true);
    ui.lineEditPredecessors->setEnabled(true);
    ui.lineEditPredecessors->setPalette(QPalette()); // sets the default palette
	ui.labelPredecessor->setEnabled(true);
	enablePredecessorSettings(true);
    ui.dateEditFirstPossibleDate->setEnabled(true);
    ui.labelFirstPossibleDate->setEnabled(true);
    ui.dateEditLastPossibleDate->setEnabled(true);
    ui.labelLastPossibleDate->setEnabled(true);
    ui.timeEditFirstPossibleTime->setEnabled(true);
    ui.labelFirstPossibleTime->setEnabled(true);
    ui.timeEditLastPossibleTime->setEnabled(true);
    ui.labelLastPossibleTime->setEnabled(true);
    ui.checkBoxFixedDate->setEnabled(true);
    ui.checkBoxFixedTime->setEnabled(true);
    ui.comboBoxReservation->setEnabled(true);
	ui.labelBindToReservation->setEnabled(true);
	ui.tab_TaskSettings->setEnabled(true);
	ui.groupBoxAnalogBeam->setEnabled(true);
	ui.tableWidgetDigitalBeams->setEnabled(true);
    ui.comboBoxStorageSelectionMode->setEnabled(true);
	ui.comboBoxStorageDataType->setEnabled(true);
	// tab stations
    ui.checkBoxTBBPiggybackAllowed->setEnabled(true);
    ui.checkBoxAartfaacPiggybackAllowed->setEnabled(true);
	ui.tab_StationSettings->setEnabled(true);
    ui.comboBoxStationAntennaMode->setEnabled(true);
    ui.comboBoxStationClock->setEnabled(true);
    ui.comboBoxStationFilter->setEnabled(true);
    ui.listWidgetAvailableStations->setEnabled(true);
    ui.treeWidgetUsedStations->setEnabled(true);
	ui.pushButtonAddSuperStation->setEnabled(true);

	ui.tab_Processing->setEnabled(true);
	ui.tab_Pipeline->setEnabled(true);
}

void TaskDialog::setScheduledTaskMode(void) {
	// tab Schedule
	ui.tab_TaskSettings->setEnabled(true);
    ui.comboBoxTaskStatus->setEnabled(true);
    ui.lineEditTaskName->setEnabled(false);
	ui.pushButtonUpdateCampaigns->setEnabled(false);
	ui.labelTaskName->setEnabled(false);
    ui.comboBoxProjectID->setEnabled(false);
	ui.labelProjectName->setEnabled(false);
    ui.labelDuration->setEnabled(false);
    ui.lineEditDuration->setEnabled(false);
    ui.dateTimeEditScheduledStart->setEnabled(false);
    ui.labelScheduledStart->setEnabled(false);
    ui.dateTimeEditScheduledEnd->setEnabled(false);
    ui.labelScheduledEnd->setEnabled(false);
    ui.lineEditPriority->setEnabled(false);
	ui.labelPriority->setEnabled(false);
    ui.lineEditGroupID->setEnabled(false);
    ui.lineEditPredecessors->setEnabled(false);
    ui.lineEditPredecessors->setPalette(QPalette()); // sets the default palette
	ui.labelPredecessor->setEnabled(false);
	enablePredecessorSettings(false);
    ui.comboBoxProcessType->setEnabled(false);
    ui.comboBoxProcessSubType->setEnabled(false);
    ui.comboBoxStrategies->setEnabled(false);
    ui.dateEditFirstPossibleDate->setEnabled(false);
    ui.labelFirstPossibleDate->setEnabled(false);
    ui.dateEditLastPossibleDate->setEnabled(false);
    ui.labelLastPossibleDate->setEnabled(false);
    ui.timeEditFirstPossibleTime->setEnabled(false);
    ui.labelFirstPossibleTime->setEnabled(false);
    ui.timeEditLastPossibleTime->setEnabled(false);
    ui.labelLastPossibleTime->setEnabled(false);
    ui.checkBoxFixedDate->setEnabled(false);
    ui.checkBoxFixedTime->setEnabled(false);
    ui.comboBoxReservation->setEnabled(false);
	ui.labelBindToReservation->setEnabled(true);
	ui.pushButtonUnBindReservation->setEnabled(false);
	ui.groupBoxAnalogBeam->setEnabled(false);
	// tab Station Beams
	ui.groupBoxAnalogBeam->setEnabled(false);
	itsDigitalBeamDialog->setReadOnly(true);
	ui.pushButtonEditBeam->setText("Show");
	ui.pushButtonDeleteBeams->setEnabled(false);
	ui.pushButtonClearAllBeams->setEnabled(false);
    ui.comboBoxStorageSelectionMode->setEnabled(false);
	// tab stations
    ui.checkBoxTBBPiggybackAllowed->setEnabled(false);
    ui.checkBoxAartfaacPiggybackAllowed->setEnabled(false);

	ui.tab_StationSettings->setEnabled(true);
    ui.comboBoxStationAntennaMode->setEnabled(false);
    ui.comboBoxStationClock->setEnabled(false);
    ui.comboBoxStationFilter->setEnabled(false);
    ui.listWidgetAvailableStations->setEnabled(false);
    ui.treeWidgetUsedStations->setEnabled(false);
	ui.pushButtonAddSuperStation->setEnabled(false);
	ui.tab_Processing->setEnabled(false);
	ui.tab_Pipeline->setEnabled(false);
}

void TaskDialog::setActiveTaskMode(void) {
	// tab Schedule
	ui.tab_TaskSettings->setEnabled(true);
    ui.comboBoxTaskStatus->setEnabled(true);
    ui.lineEditTaskName->setEnabled(false);
	ui.pushButtonUpdateCampaigns->setEnabled(false);
	ui.labelTaskName->setEnabled(false);
    ui.comboBoxProjectID->setEnabled(false);
	ui.labelProjectName->setEnabled(false);
    ui.labelDuration->setEnabled(false);
    ui.lineEditDuration->setEnabled(false);
    ui.dateTimeEditScheduledStart->setEnabled(false);
    ui.labelScheduledStart->setEnabled(false);
    ui.dateTimeEditScheduledEnd->setEnabled(false);
    ui.labelScheduledEnd->setEnabled(false);
    ui.lineEditPriority->setEnabled(false);
	ui.labelPriority->setEnabled(false);
    ui.lineEditGroupID->setEnabled(false);
    ui.lineEditPredecessors->setEnabled(false);
    ui.lineEditPredecessors->setPalette(QPalette()); // sets the default palette
	ui.labelPredecessor->setEnabled(false);
	enablePredecessorSettings(false);
    ui.comboBoxProcessType->setEnabled(false);
    ui.comboBoxProcessSubType->setEnabled(false);
    ui.comboBoxStrategies->setEnabled(false);
    ui.dateEditFirstPossibleDate->setEnabled(false);
    ui.labelFirstPossibleDate->setEnabled(false);
    ui.dateEditLastPossibleDate->setEnabled(false);
    ui.labelLastPossibleDate->setEnabled(false);
    ui.timeEditFirstPossibleTime->setEnabled(false);
    ui.labelFirstPossibleTime->setEnabled(false);
    ui.timeEditLastPossibleTime->setEnabled(false);
    ui.labelLastPossibleTime->setEnabled(false);
    ui.checkBoxFixedDate->setEnabled(false);
    ui.checkBoxFixedTime->setEnabled(false);
    ui.comboBoxReservation->setEnabled(false);
	ui.labelBindToReservation->setEnabled(true);
	ui.groupBoxAnalogBeam->setEnabled(false);
	// tab Station Beams
	ui.groupBoxAnalogBeam->setEnabled(false);
	itsDigitalBeamDialog->setReadOnly(true);
	ui.pushButtonEditBeam->setText("Show");
	ui.pushButtonDeleteBeams->setEnabled(false);
	ui.pushButtonClearAllBeams->setEnabled(false);
    ui.comboBoxStorageSelectionMode->setEnabled(false);
	// tab stations
	ui.tab_StationSettings->setEnabled(false);
	ui.tab_Processing->setEnabled(false);
	ui.tab_Pipeline->setEnabled(false);
}

void TaskDialog::setFinishedTaskMode(void) {
    ui.lineEditPredecessors->setPalette(QPalette()); // sets the default palette
	ui.groupBoxAnalogBeam->setEnabled(false);
    ui.comboBoxStorageSelectionMode->setEnabled(false);
	ui.tab_TaskSettings->setEnabled(false);
	ui.tab_StationSettings->setEnabled(false);
	ui.tab_Processing->setEnabled(false);
	ui.tab_Pipeline->setEnabled(false);
}

void TaskDialog::countSelectedStorageLocations(void) {
	int count(0);
	for (QList<QTreeWidgetItem *>::const_iterator it = itsStorageTreeLocationsItems.begin(); it != itsStorageTreeLocationsItems.end(); ++it) {
		if ((*it)->checkState(0) == Qt::Checked) ++count;
	}
	ui.lineEditAssignedStorageNodes->setText(QString::number(count));
}

void TaskDialog::setStorageTreeMixed(bool enableOverride) {
	ui.treeWidgetStorageNodes->clear();
	itsStorageTreeLocationsItems.clear();
	QStringList treeText;
	ui.treeWidgetStorageNodes->removeAction(itsActionStorageCheckSelected);
	ui.treeWidgetStorageNodes->removeAction(itsActionStorageUncheckSelected);
	ui.treeWidgetStorageNodes->addAction(itsActionStorageOverride);

	if (enableOverride) {
		connect(itsActionStorageOverride, SIGNAL(triggered()), this, SLOT(doManualStorageOverride()));
		treeText << "MIXED\nRight-click and choose 'Manual override'\nto manual set storage for all selected tasks";
//		ui.treeWidgetStorageNodes->setToolTip("right-click and choose 'Manual override' to manual set storage for all tasks");
		itsActionStorageOverride->setEnabled(true);
	}
	else {
		treeText << "MIXED\nSome tasks are not in the PRESCHEDULED state\nManual override is not possible";
		itsActionStorageOverride->setEnabled(false);
//		itsActionStorageOverride->setToolTip("some tasks have status above PRESCHEDULED\nStorage node assignment may only be changed for tasks that have status lower or equal to PRESCHEDULED\nManual override is not possible");
	}
	new QTreeWidgetItem(ui.treeWidgetStorageNodes, treeText);
	ui.treeWidgetStorageNodes->resizeColumnToContents(0);
	ui.lineEditMinNrStorageNodes->clear();
	ui.lineEditAssignedStorageNodes->clear();
}

void TaskDialog::doManualStorageOverride(void) {
	if (itsActionStorageOverride) {
		disconnect(itsActionStorageOverride);
	}
	ui.treeWidgetStorageNodes->removeAction(itsActionStorageOverride);
	ui.treeWidgetStorageNodes->addAction(itsActionStorageCheckSelected);
	ui.treeWidgetStorageNodes->addAction(itsActionStorageUncheckSelected);
//	itsTmpStorage.clear();
    ui.comboBoxStorageSelectionMode->blockSignals(true);
    ui.comboBoxStorageSelectionMode->setCurrentIndex(STORAGE_MODE_MANUAL);
    ui.comboBoxStorageSelectionMode->blockSignals(false);
	setStorageSettings(true);
	enableApplyButtons(true);
	itsStorageOverride = true;
}

void TaskDialog::doCheckSelectedStorage(void) {
	for (QList<QTreeWidgetItem *>::iterator it = itsStorageTreeLocationsItems.begin(); it != itsStorageTreeLocationsItems.end(); ++it) {
		if ((*it)->isSelected()) {
			(*it)->setCheckState(0, Qt::Checked);
		}
	}
}

void TaskDialog::doUnCheckSelectedStorage(void) {
	for (QList<QTreeWidgetItem *>::iterator it = itsStorageTreeLocationsItems.begin(); it != itsStorageTreeLocationsItems.end(); ++it) {
		if ((*it)->isSelected()) {
			(*it)->setCheckState(0, Qt::Unchecked);
		}
	}
}

void TaskDialog::updateStorageTree(void) {
    // add the storage nodes info
    itsStorageTreeLocationsItems.clear();
    itsStorageTreeNodeItems.clear();
    ui.treeWidgetStorageNodes->clear();
    QTreeWidgetItem *nodeItem, *childItem;
    QStringList itemValues;
    const hostPartitionsMap &partitions = itsController->getStoragePartitions();
    const storageHostsMap &storageNodes = itsController->getStorageNodes(); // should get the actual status of the storage nodes from the datamonitor class through the controller
    const statesMap & states = itsController->getStorageNodesStates();
    hostPartitionsMap::const_iterator pit;
    checkStorageSettingsEnable();

    // **************************************
    // **** FILL THE STORAGE TREE WIDGET ****
    // **************************************
    std::string hostName, hostStatus(" (Status unknown)");
    int nodeStatus;
    bool enableStorageSelection, enableNode;
    storage_selection_mode mode = static_cast<storage_selection_mode>(ui.comboBoxStorageSelectionMode->currentIndex());

    (mode == STORAGE_MODE_MANUAL) ? enableStorageSelection = true : enableStorageSelection = false;

    statesMap::const_iterator statit;
    dataProductTypes displayedDataProduct(getSelectedStorageDataProduct());

    for (storageHostsMap::const_iterator sit = storageNodes.begin(); sit != storageNodes.end(); ++sit) {
        // node name and status
        hostName = sit->second.itsName;
        nodeStatus = sit->second.itsStatus;
        statit = states.find(nodeStatus);
        if (statit != states.end())
            hostStatus = " (" + statit->second + ")";
        // node name and status
        hostName += hostStatus;
        nodeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(hostName.c_str()));
        if (nodeStatus == 0) { // inactive status
            nodeItem->setTextColor(0,Qt::red);
        }
        if (sit->second.itsMayBeUsed) {
            enableNode = true;
            nodeItem->setFlags(Qt::ItemIsEnabled);
        }
        else {
            nodeItem->setFlags(Qt::NoItemFlags);
            enableNode = false;
        }

        pit = partitions.find(sit->first);
        if (pit != partitions.end()) {
            bool checked(false);
            for (dataPathsMap::const_iterator dpit = pit->second.begin(); dpit != pit->second.end(); ++dpit) { // loop through raid sets (partitions)
                checked = false;
                itemValues << "" << dpit->second.first.c_str() /*partition name */
                           << humanReadableUnits(dpit->second.second[0], SIZE_UNITS).c_str() /*total space*/ << humanReadableUnits(dpit->second.second[3], SIZE_UNITS).c_str() /*free space*/;
                childItem = new QTreeWidgetItem(nodeItem, itemValues);
                itemValues.clear();

                if (enableNode) {
                    childItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                }
                else {
                    childItem->setFlags(Qt::ItemIsSelectable);
                }
                childItem->setData(0,STORAGE_NODE_ID_ROLE,sit->first); // store the storage node ID in the child's data
                childItem->setData(1,STORAGE_RAID_ID_ROLE,dpit->first); // store the raid ID in the child's data
                itsStorageTreeLocationsItems.append(childItem);
                childItem->setDisabled(!enableStorageSelection);
                if (nodeStatus == 0) { // inactive status
                    childItem->setTextColor(1,Qt::red);
                    childItem->setTextColor(2,Qt::red);
                    childItem->setTextColor(3,Qt::red);
                }
                // now check if the current storage location (=childItem) is selected for the task shown
                // if so set the check state of the childItem
                storageMap::const_iterator stit = itsTmpStorage.find(displayedDataProduct);
                if (stit != itsTmpStorage.end()) {
                    for (storageVector::const_iterator vit = stit->second.begin(); vit != stit->second.end(); ++vit) {
                        if ((vit->first == sit->first) & (vit->second == dpit->first)) {
                            childItem->setCheckState(0,Qt::Checked);
                            checked = true;
                            break;
                        }
                    }
                }
                if (!checked) {
                    childItem->setCheckState(0,Qt::Unchecked);
                }

                // also go through the the storage check results to see if this partition needs to be marked as conflicting (not enough storage capacity/bandwidth)
                // WHY? but only mark the raid arrays as conflicting if the storage was manually chosen, otherwise the storage check results of the task should not be shown.
                const TaskStorage *task_storage(itsTask->storage());
                if (task_storage) {
                    const std::vector<storageResult> &storageCheck = task_storage->getStorageCheckResult();
                    for (std::vector<storageResult>::const_iterator scrit = storageCheck.begin(); scrit != storageCheck.end(); ++scrit) {
                        if (scrit->storageRaidID == sit->first) {
                            for (short i = 0; i<=3; ++i) {
                                childItem->setBackgroundColor(i,Qt::red);
                                childItem->setTextColor(i,Qt::white);
                            }
                            if (scrit->conflict == CONFLICT_STORAGE_NODE_SPACE){
                                for (short i = 0; i<=3; ++i) {
                                    childItem->setToolTip(i,"At the task's scheduled time there will not be enough free space on this raid array");
                                }
                            }
                            else if (scrit->conflict == CONFLICT_STORAGE_NODE_BANDWIDTH) {
                                for (short i = 0; i<=3; ++i) {
                                    childItem->setToolTip(i,"At the task's scheduled time there will not be enough network bandwidth to this raid array");
                                }
                            }
                            else if (scrit->conflict == CONFLICT_STORAGE_TIME_TOO_EARLY) {
                                for (short i = 0; i<=3; ++i) {
                                    childItem->setToolTip(i,"This task is scheduled too early (before now)");
                                }
                            }
                            else if (scrit->conflict == CONFLICT_STORAGE_WRITE_SPEED) {
                                for (short i = 0; i<=3; ++i) {
                                    childItem->setToolTip(i,"At the task's scheduled time the maximum write speed for this raid set is exceeded");
                                }
                            }
                        }
                        else if (sit->first == scrit->storageNodeID) { // the current storage node has a conflict
                            for (short i = 0; i<=3; ++i) {
                                nodeItem->setBackgroundColor(i,Qt::red);
                                nodeItem->setTextColor(i,Qt::white);
                            }
                            if (scrit->conflict == CONFLICT_STORAGE_NODE_BANDWIDTH) {
                                for (short i = 0; i<=3; ++i) {
                                    nodeItem->setToolTip(i,"The total required network bandwidth exceeds the network bandwidth of this storage node");
                                }
                            }
                        }
                    }
                }
            }
        }
        itsStorageTreeNodeItems.append(nodeItem);
    }
    ui.treeWidgetStorageNodes->insertTopLevelItems(0, itsStorageTreeNodeItems);

    for (QList<QTreeWidgetItem *>::const_iterator eit = itsStorageTreeNodeItems.begin(); eit != itsStorageTreeNodeItems.end(); ++eit) {
        if (!(*eit)->isDisabled()) {
            ui.treeWidgetStorageNodes->expandItem(*eit);
        }
    }

    ui.treeWidgetStorageNodes->header()->resizeSection(0, 150);
    ui.treeWidgetStorageNodes->header()->resizeSection(1, 70);
    ui.treeWidgetStorageNodes->header()->resizeSection(2, 55);
    ui.treeWidgetStorageNodes->header()->resizeSection(3, 55);

    countSelectedStorageLocations();

    std::map<dataProductTypes, int>::const_iterator minNodesIt(itsMinNrOfRequiredNodes.find(displayedDataProduct));
    if (minNodesIt != itsMinNrOfRequiredNodes.end()) {
        ui.lineEditMinNrStorageNodes->setText(QString::number(minNodesIt->second));
    }
    else {
        ui.lineEditMinNrStorageNodes->setText("0");
    }
}

// get the displayed storage locations for the data product currently selected by comboBoxStorageDataType and store in tmpStorage
void TaskDialog::getDisplayedStorageLocations(void) {
    itsTmpStorage.clear();
	dataProductTypes dp(getSelectedStorageDataProduct());
	countSelectedStorageLocations();
	for (QList<QTreeWidgetItem *>::const_iterator it = itsStorageTreeLocationsItems.begin(); it != itsStorageTreeLocationsItems.end(); ++it) {
		if ((*it)->checkState(0) == Qt::Checked) {
            itsTmpStorage[dp].push_back(std::pair<int,int>((*it)->data(0,STORAGE_NODE_ID_ROLE).toInt(),(*it)->data(1,STORAGE_RAID_ID_ROLE).toInt()));
		}
	}
}

void TaskDialog::disableAnalogBeamSettings(void) {
	ui.groupBoxAnalogBeam->setEnabled(false);
    ui.comboBoxAnalogBeamUnits->setCurrentIndex(0);
    ui.comboBoxAnalogBeamCoordinates->setCurrentIndex(0);
	ui.timeEditAnalogBeamStartTime->clear();
    ui.lineEditAnalogBeamDuration->clear();
    ui.lineEditAnalogBeamAngle1->setText("");
    ui.lineEditAnalogBeamAngle2->setText("");
}


// disables or enables the user possibility to check storage locations in the storage locations tree widget
void TaskDialog::setStorageEditable(bool enable) {
	if (enable) {
		// enable right click actions
		ui.treeWidgetStorageNodes->removeAction(itsActionStorageOverride); // remove the right click action from the storage view
		ui.treeWidgetStorageNodes->addAction(itsActionStorageCheckSelected);
		ui.treeWidgetStorageNodes->addAction(itsActionStorageUncheckSelected);
		for (QList<QTreeWidgetItem *>::iterator it = itsStorageTreeLocationsItems.begin(); it != itsStorageTreeLocationsItems.end(); ++it) {
			(*it)->setDisabled(false);
		}
	}
	else {
		// disable all right click actions
		ui.treeWidgetStorageNodes->removeAction(itsActionStorageOverride); // remove the right click action from the storage view
		ui.treeWidgetStorageNodes->removeAction(itsActionStorageCheckSelected);
		ui.treeWidgetStorageNodes->removeAction(itsActionStorageUncheckSelected);
		ui.treeWidgetStorageNodes->setBackgroundRole(QPalette::Dark);
		for (QList<QTreeWidgetItem *>::iterator it = itsStorageTreeLocationsItems.begin(); it != itsStorageTreeLocationsItems.end(); ++it) {
			(*it)->setDisabled(true);
		}
	}
}

void TaskDialog::setStations(const StationTask *task) {
    ui.listWidgetAvailableStations->clearSelection(); // also clear selection from available stations
    ui.treeWidgetUsedStations->clear();
    const taskStationsMap &stations = task->getStations();
    taskStationsMap::const_iterator stationMappingit;
    std::vector<unsigned>::const_iterator stationIDit;
    std::vector<unsigned> addedStations;

    if (task->isObservation()) {
        const superStationMap &superStations = static_cast<const Observation *>(task)->getSuperStations();
        // now iterate over the super stations that have to be added
        superStationMap::const_iterator it = superStations.begin();
        while (it != superStations.end()) {
            QTreeWidgetItem * superStation = ui.treeWidgetUsedStations->addSuperStation();
            stationIDit = it->second.begin(); // iterate over the station IDs that have to be added to this superstation
            while (stationIDit != it->second.end()) {
                stationMappingit = stations.begin();
                while (stationMappingit != stations.end()) {
                    if (stationMappingit->second == *stationIDit) { // found the station in the station name - id map
                        QTreeWidgetItem * stationItem = new QTreeWidgetItem(superStation, QStringList(stationMappingit->first.c_str())); // add the station item (by name) to this superstation
                        ui.listWidgetAvailableStations->removeStation(stationMappingit->first.c_str()); // also remove the station from the list of available stations
                        stationItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
                        addedStations.push_back(*stationIDit); // remember the stations that have been added
                        ++stationIDit; // goto next station that has to be added to this superstation
                        break;
                    }
                    ++stationMappingit; // increase station mapping search iterator
                }
            }
            ++it; // next superstation
        }
    }

	stationMappingit = stations.begin();
	QStringList usedStations;
    while (stationMappingit != stations.end()) { // add the stations that have not already been added as part of a superstation
		if (std::find(addedStations.begin(), addedStations.end(), stationMappingit->second) == addedStations.end()) {
			usedStations.append(stationMappingit->first.c_str());
			// remove the station from the available station list
            ui.listWidgetAvailableStations->removeStation(stationMappingit->first.c_str());
		}
		++stationMappingit;
	}
    ui.treeWidgetUsedStations->addStations(usedStations);
    ui.treeWidgetUsedStations->resetHasChangedFlag();
}

/*
void TaskDialog::showStorageConflict(const QString &conflictText) {
	ui.lineEditStorageConflict->setText(conflictText);
	ui.lineEditStorageConflict->show();
	ui.checkBox_AutomaticSelectionStorageNodes->setEnabled(false);
	ui.treeWidgetStorageNodes->setEnabled(false);
}
*/


void TaskDialog::clearDemixAlways(void) {
	const QStringList &demixSources(Controller::theSchedulerSettings.getDemixSources());
    ui.listWidgetDemixAlways->addItems(demixSources);
}

void TaskDialog::clearDemixIfNeeded(void) {
	const QStringList &demixSources(Controller::theSchedulerSettings.getDemixSources());
    ui.listWidgetDemixIfNeeded->addItems(demixSources);
}
/*
void TaskDialog::disableAveragingSettings(void) {
    ui.spinBoxAveragingFreqStep->clear();
    ui.spinBoxAveragingTimeStep->clear();
	ui.groupBoxAveraging->setEnabled(false);
}

void TaskDialog::setAveragingSettings(const Pipeline *pTask) {
	if (pTask) {
        const DemixingSettings &demixing(pTask->getDemixingSettings());
        ui.spinBoxAveragingFreqStep->setValue(demixing.freqStep);
        ui.spinBoxAveragingTimeStep->setValue(demixing.timeStep);
	}
	else {
        ui.spinBoxAveragingFreqStep->setValue(1);
        ui.spinBoxAveragingTimeStep->setValue(1);
	}
	ui.groupBoxAveraging->setEnabled(true);
}
*/

void TaskDialog::disableDemixSettings(void) {
    ui.listWidgetDemixAlways->clear();
    ui.listWidgetDemixIfNeeded->clear();
    ui.lineEditDemixSkyModel->clear();
    ui.spinBoxDemixFreqStep->clear();
    ui.spinBoxDemixTimeStep->clear();
    ui.spinBoxAveragingFreqStep->clear();
    ui.spinBoxAveragingTimeStep->clear();
	ui.groupBoxDemixing->setEnabled(false);
    ui.groupBoxAveraging->setEnabled(false);
}

void TaskDialog::setDemixSettings(const DemixingSettings &demixing) {
    ui.listWidgetDemixAlways->blockSignals(true);
    ui.listWidgetDemixIfNeeded->blockSignals(true);
    ui.listWidgetDemixIfNeeded->blockSignals(true);
    ui.lineEditDemixSkyModel->blockSignals(true);
    ui.spinBoxDemixFreqStep->blockSignals(true);
    ui.spinBoxDemixTimeStep->blockSignals(true);
    ui.spinBoxAveragingFreqStep->blockSignals(true);
    ui.spinBoxAveragingTimeStep->blockSignals(true);

    ui.listWidgetDemixAlways->clear();
    ui.listWidgetDemixIfNeeded->clear();
	const QStringList &demixSources(Controller::theSchedulerSettings.getDemixSources());
	QStringList demixAlwaysChecked, demixIfNeededChecked;
    demixAlwaysChecked = demixing.demixAlways().split(',');
    demixIfNeededChecked = demixing.demixIfNeeded().split(',');
    ui.lineEditDemixSkyModel->setText(demixing.skyModel());
    ui.spinBoxDemixFreqStep->setValue(demixing.demixFreqStep());
    ui.spinBoxDemixTimeStep->setValue(demixing.demixTimeStep());
    ui.spinBoxAveragingFreqStep->setValue(demixing.freqStep());
    ui.spinBoxAveragingTimeStep->setValue(demixing.timeStep());
    ui.listWidgetDemixAlways->addItems(demixSources, demixAlwaysChecked);
    ui.listWidgetDemixIfNeeded->addItems(demixSources, demixIfNeededChecked);
	ui.groupBoxDemixing->setEnabled(true);
    ui.groupBoxAveraging->setEnabled(true);

    ui.listWidgetDemixAlways->blockSignals(false);
    ui.listWidgetDemixIfNeeded->blockSignals(false);
    ui.listWidgetDemixIfNeeded->blockSignals(false);
    ui.lineEditDemixSkyModel->blockSignals(false);
    ui.spinBoxDemixFreqStep->blockSignals(false);
    ui.spinBoxDemixTimeStep->blockSignals(false);
    ui.spinBoxAveragingFreqStep->blockSignals(false);
    ui.spinBoxAveragingTimeStep->blockSignals(false);
}

void TaskDialog::setCalibrationPipelineSettings(const CalibrationPipeline *pTask) {
    ui.lineEditCalibrationSkyModel->setText(pTask->skyModel());
	ui.groupBoxCalibration->setEnabled(true);
}

void TaskDialog::disableCalibrationPipelineSettings(void) {
    ui.lineEditCalibrationSkyModel->clear();
	ui.groupBoxCalibration->setEnabled(false);
}

void TaskDialog::disableImagingPipelineSettings(void) {
    ui.spinBoxSubbandsPerImage->clear();
    ui.spinBoxSlicesPerImage->clear();
    ui.spinBoxNumberOfPixels->clear();
    ui.lineEditFieldOfView->clear();
    ui.lineEditCellSize->clear();
	ui.groupBoxImaging->setEnabled(false);
}

void TaskDialog::setImagingPipelineSettings(const ImagingPipeline *pPipe) {
    ui.spinBoxSubbandsPerImage->setValue(pPipe->subbandsPerImage());
    ui.spinBoxSlicesPerImage->setValue(pPipe->slicesPerImage());
    ui.checkBoxSpecifyFOV->setTristate(false);
    ui.checkBoxSpecifyFOV->setChecked(pPipe->specifyFov());
    ui.lineEditFieldOfView->setText(QString::number(pPipe->fov(),'g',10));
    ui.spinBoxNumberOfPixels->setValue(pPipe->nrOfPixels());
    ui.lineEditCellSize->setText(pPipe->cellSize());
    ui.groupBoxImaging->setEnabled(true);
}

void TaskDialog::setLongBaselinePipelineSettings(const LongBaselinePipeline *lbpipe) {
    ui.spinBoxSubbandGroupsPerMS->setValue(lbpipe->subbandGroupsPerMS());
    ui.spinBoxSubbandsPerSubbandGroup->setValue(lbpipe->subbandsPerSubbandGroup());
}

void TaskDialog::setPulsarPipelineSettings(const PulsarPipeline *pPipe) {
    ui.checkBox_NoRFI->setChecked(pPipe->noRFI());
    ui.checkBox_No_DSPSR->setChecked(pPipe->skipDspsr());
    ui.checkBox_No_fold->setChecked(pPipe->noFold());
    ui.checkBox_No_pdmp->setChecked(pPipe->noPdmp());
    ui.checkBox_RawTo8Bit->setChecked(pPipe->rawTo8Bit());
    ui.checkBox_rrats->setChecked(pPipe->rrats());
    ui.checkBox_Single_pulse->setChecked(pPipe->singlePulse());
    ui.checkBox_Skip_dynamic_spectrum->setChecked(pPipe->skipDynamicSpectrum());
    ui.checkBox_Skip_prepfold->setChecked(pPipe->skipPrepfold());
    ui.lineEdit2bf2fitsExtraOptions->setText(pPipe->twoBf2fitsExtra());
    ui.lineEditDigifilExtraOptions->setText(pPipe->digifilExtra());
    ui.lineEditDSPSRextraOptions->setText(pPipe->dspsrExtra());
    ui.lineEditPrepdataExtraOptions->setText(pPipe->prepDataExtra());
    ui.lineEditPrepfoldExtraOptions->setText(pPipe->prepFoldExtra());
    ui.lineEditPrepsubbandExtraOptions->setText(pPipe->prepSubbandExtra());
    ui.lineEditPulsarName->setText(pPipe->pulsarName());
    ui.lineEditRfiFindExtraOptions->setText(pPipe->rfiFindExtra());
    ui.spinBoxDecodeNblocks->setValue(pPipe->decodeNblocks());
    ui.spinBoxDecodeSigma->setValue(pPipe->decodeSigma());
    ui.spinBoxTsubint->setValue(pPipe->tsubInt());
    ui.doubleSpinBox8BitConversionSigma->setValue(pPipe->eightBitConversionSigma());
    ui.doubleSpinBoxDynamicSpectrumTimeAverage->setValue(pPipe->dynamicSpectrumAvg());
    ui.doubleSpinBoxRratsDmRange->setValue(pPipe->rratsDmRange());
}

bool TaskDialog::askForApplyChanges(void) {
	QApplication::beep();
	QMessageBox questionBox(this);
	questionBox.setWindowTitle(tr("Changes made to task"));
	questionBox.setText(tr("Changes were made to this task, do you want to apply these changes?"));
	questionBox.setIcon(QMessageBox::Question);
	QAbstractButton *applyButton = questionBox.addButton(tr("Apply"), QMessageBox::ApplyRole);
	QAbstractButton *discardButton = questionBox.addButton(tr("Discard"), QMessageBox::RejectRole);
	questionBox.addButton(tr("Cancel"), QMessageBox::NoRole);
	questionBox.exec();
	if (questionBox.clickedButton() == applyButton) {
        if (itsTask->isReservation() || itsTask->isMaintenance()) {
			if (!commitReservation(NOSTORE)) {
				return false;
			}
		}
		else {
			if (!commitChanges(NOSTORE)) {
				return false;
			}
		}
	}
	else if (questionBox.clickedButton() == discardButton) {
		resetChangeDetection();
	}
	else return false; // Cancel was clicked -> return false
	return true;
}

void TaskDialog::clearMultiTasks(void) {
    for (std::vector<Task *>::iterator it = itsMultiTasks.begin(); it != itsMultiTasks.end(); ++it) {
        delete *it;
    }
    itsMultiTasks.clear();
}

void TaskDialog::show(const Task *task, tabIndex tab) {
    const AstroDate &edate = Controller::theSchedulerSettings.getEarliestSchedulingDay();
    QDate earliestDate(edate.getYear(), edate.getMonth(), edate.getDay());
    const AstroDate &ldate = Controller::theSchedulerSettings.getLatestSchedulingDay();
    QDate latestDate(ldate.getYear(), ldate.getMonth(), ldate.getDay());
    QString multipleStr(MULTIPLE_VALUE_TEXT);

    isMultiTasks = false;
    itsDataSlotDialog.clear();
    clearMultiTasks(); // clear the vector of task pointers used for multi-edit of tasks. Clearing is done here to make sure no old pointers are left dangling

    if (this->isVisible() && ifSettingsChanged()) {
        if (!askForApplyChanges()) return;
    }
    this->blockSignals(true);
    blockChangeDetection = true;
    addingTask = false;
    addingReservation = false;
    addingPipeline = false;

    // common properties both for reservations as for regular tasks
    ui.pushButtonApply->show();
    enableApplyButtons(false);

    setExistingProjects(Controller::theSchedulerSettings.getCampaignList());

    ui.pushButtonOk->setText("Ok");

    if (task) { // here only properties that have to be set for both reservations and regular tasks!
        Task::task_status status = task->getStatus();
        delete itsTask;
        itsTask = cloneTask(task); // create an exact copy of the supplied task shown
        enableTabs(); // enable/disable the correct tabs depending on itsTask type
        const TaskStorage *task_storage(task->storage());
        if (task_storage) {
            itsOutputDataTypes = task_storage->getOutputDataProductsEnabled();
            itsTmpStorage = task_storage->getStorageLocations();
            ui.comboBoxStorageSelectionMode->setCurrentIndex(task_storage->getStorageSelectionMode());
        }
        ui.lineEdit_ProjectName->setText(task->getProjectName());
        ui.lineEdit_ProjectCOI->setText(task->getProjectCO_I());
        ui.comboBoxProjectID->setFromString(task->getProjectID());
        ui.lineEditSASID->setText(QString::number(task->getSASTreeID()));
        ui.lineEditGroupID->setText(QString::number(task->getGroupID()));

        ui.lineEditCreationDate->setText(task->SASTree().creationDate().toString().c_str());
        ui.lineEditModificationDate->setText(task->SASTree().modificationDate().toString().c_str());

        ui.lineEdit_ProjectPI->setText(task->getProjectPI());
        ui.lineEditTaskName->setText(task->getTaskName());
        // contact details
        ui.lineEdit_ContactName->setText(task->getContactName());
        ui.lineEdit_ContactPhone->setText(task->getContactPhone());
        ui.lineEdit_ContactEmail->setText(task->getContactEmail());

        ui.lineEditTaskDescription->setText(task->SASTree().description().c_str());

        ui.lineEditTaskID->setText(QString::number(task->getID()));

        // earliest and latest scheduling dates
        ui.dateEditFirstPossibleDate->blockSignals(true);
        ui.dateEditLastPossibleDate->blockSignals(true);
        ui.dateEditFirstPossibleDate->setMinimumDate(earliestDate);
        ui.dateEditFirstPossibleDate->setMaximumDate(latestDate);
        ui.dateEditLastPossibleDate->setMinimumDate(earliestDate);
        ui.dateEditLastPossibleDate->setMaximumDate(latestDate);
        ui.dateEditFirstPossibleDate->blockSignals(false);
        ui.dateEditLastPossibleDate->blockSignals(false);

        // scheduled start and end
        //		ui.dateTimeEditScheduledStart->setMaximumDate(latestDate);
        ui.dateTimeEditScheduledEnd->setMinimumDate(earliestDate);
        //		ui.dateTimeEditScheduledEnd->setMaximumDate(latestDate);
        const AstroDateTime &sd = task->getScheduledStart();
        ui.dateTimeEditScheduledStart->blockSignals(true); // prevents automatic update
        if (sd.isSet()) {
            QDate sdate(sd.getYear(), sd.getMonth(), sd.getDay());
            QTime stime(sd.getHours(), sd.getMinutes(), sd.getSeconds());
            if (task->isObservation()) {
                unsigned reservation_id(static_cast<const Observation *>(task)->getReservation());
                if (reservation_id) {
                    const Task* reservation(itsController->getTask(reservation_id));
                    if (reservation) {
                        AstroDateTime rd(reservation->getScheduledStart());
                        ui.dateTimeEditScheduledStart->setMinimumDateTime(QDateTime(QDate(rd.getYear(),rd.getMonth(),rd.getDay()), QTime(rd.getHours(),rd.getMinutes(),rd.getSeconds())));
                    }
                    else ui.dateTimeEditScheduledStart->setMinimumDate(earliestDate);
                }
                else {
                    ui.dateTimeEditScheduledStart->setMinimumDate(earliestDate);
                }
            }
            else {
                ui.dateTimeEditScheduledStart->setMinimumDate(earliestDate);
            }
            ui.dateTimeEditScheduledStart->setDateTime(QDateTime(sdate, stime));
        }
        else {
            ui.dateTimeEditScheduledStart->setNotSetValue();
        }
        ui.dateTimeEditScheduledStart->blockSignals(false);

        const AstroDateTime &enddate = task->getScheduledEnd();
        if (enddate.isSet()) {
            QDate ed(enddate.getYear(), enddate.getMonth(), enddate.getDay());
            QTime et(enddate.getHours(), enddate.getMinutes(), enddate.getSeconds());
            setScheduledEnd(QDateTime(ed, et));
        }
        else {
            ui.dateTimeEditScheduledEnd->blockSignals(true); // prevents automatic update
            ui.dateTimeEditScheduledEnd->setNotSetValue();
            ui.dateTimeEditScheduledEnd->blockSignals(false);
        }
        // duration
        ui.lineEditDuration->blockSignals(true); // have to block signals to prevent scheduled end from being updated. Scheduled end would be displayed incorrectly the first time
        ui.lineEditDuration->setText(task->getDuration().toString());
        ui.lineEditDuration->blockSignals(false);

        // fixed day and time
        ui.checkBoxFixedDate->setTristate(false);
        ui.checkBoxFixedDate->setChecked(task->getFixedDay());
        ui.checkBoxFixedTime->setTristate(false);
        ui.checkBoxFixedTime->setChecked(task->getFixedTime());

        setProcessSubProcessStrategy(task);

        ui.lineEditOriginalTreeID->setText(QString::number(task->getOriginalTreeID()));
        // disable the possibility to change the task its processType, processSubtype and strategy if it is already created in SAS
        if (task->getSASTreeID()) {
            ui.comboBoxProcessType->setEnabled(false);
            ui.comboBoxProcessSubType->setEnabled(false);
            ui.comboBoxStrategies->setEnabled(false);
            QString tt(tr("This task is already created in SAS.\nIts processType, processSubType and strategy cannot be changed anymore"));
            ui.comboBoxProcessType->setToolTip(tt);
            ui.comboBoxProcessSubType->setToolTip(tt);
            ui.comboBoxStrategies->setToolTip(tt);
        }
        else {
            ui.comboBoxProcessType->setEnabled(true);
            ui.comboBoxProcessSubType->setEnabled(true);
            ui.comboBoxStrategies->setEnabled(true);
            ui.comboBoxProcessType->setToolTip("");
            ui.comboBoxProcessSubType->setToolTip("");
            ui.comboBoxStrategies->setToolTip("");
        }

        // predecessor
        if (task->hasPredecessors()) {
            ui.lineEditPredecessors->setText(task->getPredecessorsString());
            ui.lineEditMinPredDistance->setText(task->getPredecessorMinTimeDif().toString());
            ui.lineEditMaxPredDistance->setText(task->getPredecessorMaxTimeDif().toString());
        }
        else {
            ui.lineEditPredecessors->setPalette(QPalette()); // sets the default palette
            ui.lineEditPredecessors->setText("");
            ui.lineEditMinPredDistance->setText("");
            ui.lineEditMaxPredDistance->setText("");
            enablePredecessorSettings(false);
        }

        const AstroDate &fdate = task->getWindowFirstDay();
        if (fdate.isSet()) {
            ui.dateEditFirstPossibleDate->setDate(QDate(fdate.getYear(), fdate.getMonth(), fdate.getDay()));
        }
        else {
            ui.dateEditFirstPossibleDate->setDate(earliestDate);
        }
        const AstroDate &ldate = task->getWindowLastDay();
        if (ldate.isSet()) {
            ui.dateEditLastPossibleDate->setDate(QDate(ldate.getYear(), ldate.getMonth(), ldate.getDay()));
        }
        else {
            ui.dateEditLastPossibleDate->setDate(latestDate);
        }
        const AstroTime &ftime = task->getWindowMinTime();
        ui.timeEditFirstPossibleTime->setTime(QTime(ftime.getHours(), ftime.getMinutes(), ftime.getSeconds()));
        const AstroTime &ltime = task->getWindowMaxTime();
        ui.timeEditLastPossibleTime->setTime(QTime(ltime.getHours(), ltime.getMinutes(), ltime.getSeconds()));
        // priority
        ui.lineEditPriority->setInputMask("00.00;");
        ui.lineEditPriority->setText(QString::number(task->getPriority()));

        // settings for tasks running on stations (i.e. StationTask)
        const StationTask *statTask = dynamic_cast<const StationTask *>(task);
        if (statTask) {
            // load available stations
            loadAvailableStations();
            ui.comboBoxStationAntennaMode->blockSignals(true);
            ui.comboBoxStationClock->blockSignals(true);
            ui.comboBoxStationFilter->blockSignals(true);
            ui.comboBoxStationAntennaMode->setCurrentIndex(statTask->getAntennaMode());

            if (ui.comboBoxStationClock->itemText(ui.comboBoxStationClock->count()-1).compare(multipleStr) == 0) {
                ui.comboBoxStationClock->removeItem(ui.comboBoxStationClock->count()-1);
            }
            ui.comboBoxStationClock->setCurrentIndex(statTask->getStationClock());

            if (ui.comboBoxStationFilter->itemText(ui.comboBoxStationFilter->count()-1).compare(multipleStr) == 0) {
                ui.comboBoxStationFilter->removeItem(ui.comboBoxStationFilter->count()-1);
            }
            ui.comboBoxStationFilter->setCurrentIndex(statTask->getFilterType());
            ui.comboBoxStationAntennaMode->blockSignals(false);
            ui.comboBoxStationClock->blockSignals(false);
            ui.comboBoxStationFilter->blockSignals(false);

            //stations
            setStations(statTask);
            ui.labelAssignedStations->setText("Assigned stations (" + QString::number(countStations()) + ")");

            const Observation *obs = dynamic_cast<const Observation *>(statTask);
            if (obs) {
                this->setWindowTitle(QString("Observation ") + QString::number(obs->getID()) + " '" + obs->getTaskName() + "'");

                loadReservations(); // loads all defined reservation tasks into the reservation selection puldown box

                itsDataSlotDialog.loadData(obs->getDataSlots());

                ui.checkBoxTBBPiggybackAllowed->blockSignals(true);
                ui.checkBoxTBBPiggybackAllowed->setChecked(obs->getTBBPiggybackAllowed());
                ui.checkBoxTBBPiggybackAllowed->blockSignals(false);
                ui.checkBoxAartfaacPiggybackAllowed->blockSignals(true);
                ui.checkBoxAartfaacPiggybackAllowed->setChecked(obs->getAartfaacPiggybackAllowed());
                ui.checkBoxAartfaacPiggybackAllowed->blockSignals(false);
                // RTCP settings (have to be set correctly before setStorage is called)
                setRTCPSettings(obs->getRTCPsettings());
                checkEnableDataTypeSettings();

                itsDataSlotDialog.loadData(obs->getDataSlots());
                ui.pushButtonShowDataSlots->setEnabled(true);

                // digital and pencil beams
                ui.tableWidgetDigitalBeams->setToolTip("This task's digital station beams");
                setDigitalBeamSettings(obs);
                itsTempDigitalBeams = obs->getDigitalBeams(); // for change detection
                checkEnableBeamButtons();
                // NrDataslotsPerRSPboard
                ui.spinBoxDataslotsPerRSPboard->setValue(obs->getNrOfDataslotsPerRSPboard());
                // analog beam settings
                if (QString(obs->getAntennaModeStr()).startsWith("LBA")) {
                    disableAnalogBeamSettings();
                }
                else {
                    if (status < Task::STARTING) {
                        ui.groupBoxAnalogBeam->setEnabled(true);
                    }
                    const Observation::analogBeamSettings &beamSettings = obs->getAnalogBeam();
                    if (beamSettings.directionType == DIR_TYPE_J2000)
                        itsAnalogBeamAnglePair = ANGLE_PAIRS_HMS_DMS;
                    else
                        itsAnalogBeamAnglePair = ANGLE_PAIRS_RADIANS;
                    setAnalogBeamSettings(beamSettings);
                }
                itsTempAnalogBeamSettings = getAnalogBeamSettings(); // itsTempAnalogBeamSettings is used for change detection

            }
            // properties specific for reservations
            else if (task->isReservation() || task->isMaintenance()) { // is this a reservation/maintenance task that has to be shown?
                if (task->isReservation()) {
                    this->setWindowTitle(QString("Reservation ") + QString::number(task->getID()) + " '" + task->getTaskName() + "'");
                }
                else { // Maintenance
                    this->setWindowTitle(QString("Maintenance ") + QString::number(task->getID()) + " '" + task->getTaskName() + "'");
                }
                setReservationTaskMode();
                // load status options for reservation and maintenance tasks
                ui.comboBoxTaskStatus->clear();
                ui.lineEditPredecessors->setPalette(QPalette()); // sets the default palette
                ui.lineEditPredecessors->setText("");
                ui.lineEditMinPredDistance->setText("");
                ui.lineEditMaxPredDistance->setText("");
                enablePredecessorSettings(false);
                if (status == Task::ERROR) {
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::ERROR]);
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                    ui.comboBoxTaskStatus->setCurrentIndex(0);
                }
                if (status == Task::CONFLICT) {
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::CONFLICT]);
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                    ui.comboBoxTaskStatus->setCurrentIndex(0);
                }
                else {
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                    ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                    if (status == Task::PRESCHEDULED) {
                        ui.comboBoxTaskStatus->setCurrentIndex(0);
                    }
                    else {
                        ui.comboBoxTaskStatus->setCurrentIndex(1);
                    }
                }
            }
            ui.treeWidgetInputDataProducts->clear();
        }
        else if (task->isPipeline()) {
            this->setWindowTitle(QString("Pipeline ") + QString::number(task->getID()) + " '" + task->getTaskName() + "'");
            setPipelineProperties();
        }

        // set the possible statusses for all task types (except reservations and maintenance)
        if (!(task->isReservation() || task->isMaintenance())) {
            // task status combo box
            ui.comboBoxTaskStatus->blockSignals(true);
            ui.comboBoxTaskStatus->clear();
            switch (status) {
            case Task::ERROR:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ERROR]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::SCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
                break;
            case Task::COMPLETING:
            case Task::FINISHED:
            case Task::ABORTED:
                setFinishedTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[status]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
                break;
            case Task::CONFLICT:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::CONFLICT]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
                break;
            case Task::UNSCHEDULED:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
                break;
            case Task::PRESCHEDULED:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::SCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(1);
                break;
            case Task::SCHEDULED:
                setScheduledTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::SCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(2);
                break;
            case Task::ACTIVE:
                setActiveTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ACTIVE]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ABORTED]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
                break;
            case Task::STARTING:
                setActiveTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::STARTING]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ABORTED]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
                break;
            case Task::ON_HOLD:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(2);
                break;
            case Task::OBSOLETE:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::OBSOLETE]);
                ui.comboBoxTaskStatus->setCurrentIndex(3);
                break;
            default:
                setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[status]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
            }
            ui.comboBoxTaskStatus->blockSignals(false);
        }

        // storage properties
        updateStorageTab();
        setStorageSettings(true);
        checkStorageSettingsEnable();
    }


    StoreValues();

    ui.treeWidgetStorageNodes->removeAction(itsActionStorageOverride); // remove the right click action from the storage view
    ui.treeWidgetStorageNodes->removeAction(itsActionStorageCheckSelected);
    ui.treeWidgetStorageNodes->removeAction(itsActionStorageUncheckSelected);
    if (static_cast<storage_selection_mode>(ui.comboBoxStorageSelectionMode->currentIndex()) == STORAGE_MODE_MANUAL) {
        ui.treeWidgetStorageNodes->addAction(itsActionStorageCheckSelected);
        ui.treeWidgetStorageNodes->addAction(itsActionStorageUncheckSelected);
    }

    setCurrentTab(tab);
    this->blockSignals(false);
    this->hide();
    this->setModal(false);
    this->showNormal();
    this->raise();
    this->activateWindow();
    blockChangeDetection = false;
}

void TaskDialog::setCurrentTab(tabIndex tab) {
	switch (tab) {
	default:
	case TAB_SCHEDULE:
		ui.tabWidgetMain->setCurrentWidget(ui.tab_TaskSettings);
		return;
	case TAB_STATION_SETTINGS:
		ui.tabWidgetMain->setCurrentWidget(ui.tab_StationSettings);
		return;
	case TAB_STATION_BEAMS:
		ui.tabWidgetMain->setCurrentWidget(ui.tab_StationBeams);
		return;
	case TAB_PROCESSING:
		ui.tabWidgetMain->setCurrentWidget(ui.tab_Processing);
		return;
	case TAB_STORAGE:
		ui.tabWidgetMain->setCurrentWidget(ui.tab_Storage);
		return;
	case TAB_PIPELINE:
		ui.tabWidgetMain->setCurrentWidget(ui.tab_Pipeline);
		return;
	}
}

// make copies of the original supplied tasks
// make changes in the copies
// then for each task notify the controller that the task has changed so it can take the necessary actions and update the GUI
// reservation tasks cannot be multi-edited, these are already filtered out by the controller
void TaskDialog::showMultiEdit(std::vector<Task *> &tasks) {
	itsStorageOverride = false;
    blockChangeDetection = true;
	blockSignals(true);
	ui.checkBoxCorrelatedData->blockSignals(true);
	ui.checkBoxCoherentStokes->blockSignals(true);
	ui.checkBoxIncoherentStokes->blockSignals(true);
	addingTask = false;
	addingReservation = false;
	isMultiTasks = true;
	enableTabs();

	itsDataSlotDialog.clear();

    ui.lineEditCreationDate->clear();
    ui.lineEditModificationDate->clear();

	ui.lineEditStorageConflict->hide();
	QString multipleStr(MULTIPLE_VALUE_TEXT);

    ui.groupBoxCalibration->setEnabled(true);
    ui.groupBoxDemixing->setEnabled(true);
    ui.groupBoxAveraging->setEnabled(true);
    ui.groupBoxImaging->setEnabled(true);
    ui.groupBoxDigitalBeams->setEnabled(false); // multi-edit of digital beams not possible
    ui.tableWidgetDigitalBeams->setToolTip("Digital beams cannot be multi-edited");
    ui.tableWidgetDigitalBeams->clearContents();
    ui.tableWidgetTiedArrayBeams->clearContents();

	this->setWindowTitle("Multi-edit tasks");
	if (!tasks.empty()) {
//		connect(this,SIGNAL(QApplication::focusChanged (QWidget *, QWidget *)),this,SLOT(focusCheck(QWidget *, QWidget *)));
		// make copies of the original tasks (used when changes are applied to these tasks)
		for (std::vector<Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
            itsMultiTasks.push_back(cloneTask(*it));
		}

        if (this->isVisible() && ifSettingsChanged()) {
			if (!askForApplyChanges()) return;
		}


		// load available stations
		loadAvailableStations();
		// update project combobox
        setExistingProjects(Controller::theSchedulerSettings.getCampaignList());

        bool difProcessType(false), difProcessSubtype(false), difStrategy(false), difStatus(false), difProjectID(false), difTaskName(false), difContactName(false),
                difGroupID(false), difContactPhone(false), difContactEmail(false), difTaskDescription(false),
                difFirstPossibleDate(false), difWindowMinTime(false), difLastPossibleDate(false), difWindowMaxTime(false),
                difScheduledStart(false), difScheduledEnd(false), difReservation(false),
                difDuration(false), difAntennaMode(false), difStationClock(false), difFilterType(false),
                difPriority(false), difNrDataSlotsPerRSP(false), difTBBpiggyback(false), difAartfaacPiggyback(false), difoutputCorrelated(false),
                difOutputCoherentStokes(false), difOutputIncoherentStokes(false),
                difFixedDay(false), difFixedTime(false), difPredecessor(false), difpredMinTimeDif(false), difpredMaxTimeDif(false),
                difStations(false), difDelayCompensation(false), difCorrectBandPass(false), /*difDigitalBeams(false),*/
                difAnalogAngle1(false), difAnalogAngle2(false), difAnalogDirectionType(false), difAnalogDuration(false),
                difAnalogStartTime(false), onlyLBA(true), allNewTasks(true),
                difFlysEye(false), difBitsPerSample(false), difChannelsPerSubband(false),
                difCorrelatorIntegrationTime(false), difStorageSelectionMode(false), difStorageLocations(false),
                difCoherentStokesType(false), difIncoherentStokesType(false), /*difTiedArrayBeams(false),*/
                difCoherentDedispersion(false), difCoherentTimeIntegration(false), difIncoherentTimeIntegration(false),
                difCoherentChannelsPerSubband(false), difIncoherentChannelsPerSubband(false), difCoherentSubbandsPerFile(false), difIncoherentSubbandsPerFile(false),
                difDemixAlways(false), difDemixIfNeeded(false), difDemixFreqStep(false), difDemixTimeStep(false), difAvgFreqStep(false),
                difAvgTimeStep(false), difCalibrationSkyModel(false), difDemixSkyModel(false), difSpecifyFOV(false), difFOV(false),
                difCellSize(false), difNrOfPixels(false), difSlicesPerImage(false), difSubbandsPerImage(false), difSubbandGroupsPerMS(false), difSubbandsPerSubbandGroup(false),
                difPulsar_noRFI(false), difPulsar_noDSPSR(false), difPulsar_noFold(false), difPulsar_noPDMP(false), difPulsar_rawTo8Bit(false), difPulsar_RRATS(false),
                difPulsar_singlePulse(false), difPulsar_skipDynamicSpectrum(false), difPulsar_skipPrepfold(false), difPulsar_twoBf2fitsExtra(false), difPulsar_digifilExtra(false),
                difPulsar_dsprExtra(false), difPulsar_prepDataExtra(false), difPulsar_prepFoldExtra(false), difPulsar_prepSubbandExtra(false), difPulsar_PulsarName(false),
                difPulsar_rfiFindExtra(false), difPulsar_decodeNblocks(false), difPulsar_decodeSigma(false), difPulsar_tsubint(false), difPulsar_8bitconvSigma(false),
                difPulsar_dynamicSpectrumAvg(false), difPulsar_rratsDMRange(false);
        delete itsTask;
        itsTask = cloneTask(tasks.front()); // used for change detection in commitMultiTask
        Task::task_type type = itsTask->getType();
        QString processType = itsTask->getProcessType();
        QString processSubtypeStr = itsTask->getProcessSubtypeStr();
        processSubTypes processSubtype = itsTask->getProcessSubtype();
        QString strategy = itsTask->getStrategy();
        const QString &predecessors(itsTask->getPredecessorsString());
        AstroTime predecessorMinTimeDif(itsTask->getPredecessorMinTimeDif());
        AstroTime predecessorMaxTimeDif(itsTask->getPredecessorMaxTimeDif());

        Task::task_status status = itsTask->getStatus(), status2;
        std::string projectID = itsTask->getProjectID();
        std::string taskName = itsTask->getTaskName();
        std::string contactName = itsTask->getContactName();
        std::string contactPhone = itsTask->getContactPhone();
        std::string contactEmail = itsTask->getContactEmail();
        std::string taskDescription = itsTask->SASTree().description();

        // for each specific task type save a pointer to the first task (in the multitasks) that will be used to
        // compare for differences between the mutlti tasks
        const StationTask *firstStationTask(0);
        const Observation *firstObservation(0);
        const ImagingPipeline *firstImagingPipeline(0);
        const PulsarPipeline *firstPulsarPipeline(0);
        const CalibrationPipeline *firstCalibrationPipeline(0);
        const LongBaselinePipeline *firstLongBaselinePipeline(0);
        const DemixingSettings *firstDemixingSettings(0);
        const TaskStorage *firstStorage(0);
        TaskStorage::enableDataProdukts outputDataTypes;

        for (std::vector<Task *>::const_iterator it = tasks.begin(); it != tasks.end(); ++it) {
            if (!firstStationTask && (*it)->isStationTask()) {
                firstStationTask = static_cast<const StationTask *>(*it);
                if (!firstObservation && (*it)->isObservation()) {
                    firstObservation = static_cast<const Observation *>(*it);
                }
            }
            else if ((*it)->isPipeline()) {
                const Pipeline *pipe(static_cast<Pipeline *>(*it));
                if (!firstCalibrationPipeline && pipe->isCalibrationPipeline()) {
                    firstCalibrationPipeline = static_cast<const CalibrationPipeline *>(pipe);
                    firstDemixingSettings = &firstCalibrationPipeline->demixingSettings();
                }
                else if (!firstImagingPipeline && pipe->isImagingPipeline()) {
                    firstImagingPipeline = static_cast<const ImagingPipeline *>(pipe);
                }
                else if (!firstPulsarPipeline && pipe->isPulsarPipeline()) {
                    firstPulsarPipeline = static_cast<const PulsarPipeline *>(pipe);
                }
                else if (!firstLongBaselinePipeline && pipe->isLongBaselinePipeline()) {
                    firstLongBaselinePipeline = static_cast<const LongBaselinePipeline *>(pipe);
                }
            }
            if (!firstStorage && (*it)->hasStorage()) {
                firstStorage = (*it)->storage();
                outputDataTypes = firstStorage->getOutputDataProductsEnabled();
                itsTmpStorage = firstStorage->getStorageLocations();
            }
        }

//        if (firstObservation) {
//            if (firstObservation->getDigitalBeams().empty()) difTiedArrayBeams = true; // no need to check tied array beams for differences if there are none (no digital beams, so no tied array beams)
//        }

        bool hasSAStreeID(itsTask->getSASTreeID());

		bool containsNonEditableTasks(false), statusNotPreScheduled(false);
		bool all_storage_assigned(true), all_storage_set(true), no_storage_set(true);

		for (std::vector<Task *>::const_iterator taskit = tasks.begin(); taskit != tasks.end(); ++taskit) {
            Task *pTask = *taskit;

			if (!difProcessType) { // not yet different task types?
				if (pTask->getType() != type) {
					difProcessType = true;
				}
			}
			if (!difProcessSubtype) {
				if (pTask->getProcessSubtype() != processSubtype) {
					difProcessSubtype = true;
				}
			}
			if (!difStrategy) {
				if (pTask->getStrategy() != strategy) {
					difStrategy = true;
				}
			}
			if (!difStatus) {
				status2 = pTask->getStatus();
				if (status2 >= Task::SCHEDULED) containsNonEditableTasks = true;
				if (status2 != status) {
					difStatus = true;
				}
				if (!statusNotPreScheduled) {
					if (status2 != Task::PRESCHEDULED) statusNotPreScheduled = true;
				}
			}
			if (!difProjectID) {
				if (pTask->getProjectID() != projectID) {
					difProjectID = true;
				}
			}
			if (!difGroupID) {
                if (pTask->getGroupID() != itsTask->getGroupID()) {
					difGroupID = true;
				}
			}
			if (!difTaskName) {
				if (pTask->getTaskName() != taskName) {
					difTaskName = true;
				}
			}
			if (!difContactName) {
				if (pTask->getContactName() != contactName) {
					difContactName = true;
				}
			}
			if (!difContactPhone) {
				if (pTask->getContactPhone() != contactPhone) {
					difContactPhone = true;
				}
			}
			if (!difContactEmail) {
				if (pTask->getContactEmail() != contactEmail) {
					difContactEmail = true;
				}
			}
			if (!difTaskDescription) {
				if (pTask->SASTree().description() != taskDescription) {
					difTaskDescription = true;
				}
			}
			if (!difFirstPossibleDate) {
                if (pTask->getWindowFirstDay() != itsTask->getWindowFirstDay()) {
					difFirstPossibleDate = true;
				}
			}
			if (!difWindowMinTime) {
                if (pTask->getWindowMinTime() != itsTask->getWindowMinTime()) {
					difWindowMinTime = true;
				}
			}
			if (!difLastPossibleDate) {
                if (pTask->getWindowLastDay() != itsTask->getWindowLastDay()) {
					difLastPossibleDate = true;
				}
			}
			if (!difWindowMaxTime) {
                if (pTask->getWindowMaxTime() != itsTask->getWindowMaxTime()) {
					difWindowMaxTime = true;
				}
			}
			if (!difScheduledStart) {
                if (pTask->getScheduledStart() != itsTask->getScheduledStart()) {
					difScheduledStart= true;
				}
			}
			if (!difScheduledEnd) {
                if (pTask->getScheduledEnd() != itsTask->getScheduledEnd()) {
					difScheduledEnd= true;
				}
			}
			if (!difDuration) {
                if (pTask->getDuration() != itsTask->getDuration()) {
					difDuration = true;
				}
			}
			if (!difPriority) {
                if (fabs(pTask->getPriority() - itsTask->getPriority()) > std::numeric_limits<double>::epsilon()) {
					difPriority = true;
				}
			}
			if (!difFixedDay) {
                if (pTask->getFixedDay() != itsTask->getFixedDay()) {
					difFixedDay = true;
				}
			}
			if (!difFixedTime) {
                if (pTask->getFixedTime() != itsTask->getFixedTime()) {
					difFixedTime = true;
				}
			}
			if (!difPredecessor) {
                if (pTask->getPredecessorsString() != predecessors) {
					difPredecessor = true;
				}
			}
			if (!difpredMinTimeDif) {
				if (pTask->getPredecessorMinTimeDif() != predecessorMinTimeDif) {
					difpredMinTimeDif = true;
				}
			}
			if (!difpredMaxTimeDif) {
				if (pTask->getPredecessorMaxTimeDif() != predecessorMaxTimeDif) {
					difpredMaxTimeDif = true;
				}
			}
			if (!hasSAStreeID) {
				if (pTask->SASTree().treeID()) {
					hasSAStreeID = true;
				}
			}

            if (firstStationTask && pTask->isStationTask()) {
                const StationTask *pStatTask(static_cast<const StationTask *>(pTask));
                //station settings
                if (!difStations) {
                    if (pStatTask->getStations() != firstStationTask->getStations()) {
                        difStations = true;
                    }
                }
                // antenna mode
                if (!difAntennaMode) {
                    if (pStatTask->getAntennaMode() != firstStationTask->getAntennaMode()) {
                        difAntennaMode = true;
                    }
                }
                // station clock
                if (!difStationClock) {
                    if (pStatTask->getStationClock() != firstStationTask->getStationClock()) {
                        difStationClock = true;
                    }
                }
                // filter type
                if (!difFilterType) {
                    if (pStatTask->getFilterType() != firstStationTask->getFilterType()) {
                        difFilterType = true;
                    }
                }

                if (firstObservation && pTask->isObservation()) {
                    const Observation *pObs(static_cast<const Observation *>(pTask));
                    if (!difReservation) {
                        if (pObs->getReservation() != firstObservation->getReservation()) {
                            difReservation = true;
                        }
                    }
                    if (!difNrDataSlotsPerRSP) {
                        if (pObs->getNrOfDataslotsPerRSPboard() != firstObservation->getNrOfDataslotsPerRSPboard()) {
                            difNrDataSlotsPerRSP = true;
                        }
                    }
                    if (!difTBBpiggyback) {
                        if (pObs->getTBBPiggybackAllowed() != firstObservation->getTBBPiggybackAllowed()) {
                            difTBBpiggyback = true;
                        }
                    }

                    if (!difAartfaacPiggyback) {
                        if (pObs->getAartfaacPiggybackAllowed() != firstObservation->getAartfaacPiggybackAllowed()) {
                            difAartfaacPiggyback = true;
                        }
                    }

                    // analog beam
                    if (onlyLBA) {
                        if (QString(pObs->getAntennaModeStr()).startsWith("HBA")) {
                            onlyLBA = false;
                        }
                    }
                    const Observation::analogBeamSettings &analogBeam(firstObservation->getAnalogBeam());
                    const Observation::analogBeamSettings &otherAnalogBeam(pObs->getAnalogBeam());
                    if (!difAnalogAngle1) {
                        if (analogBeam.angle1 != otherAnalogBeam.angle1) {
                            difAnalogAngle1 = true;
                        }
                    }
                    if (!difAnalogAngle2) {
                        if (analogBeam.angle2 != otherAnalogBeam.angle2) {
                            difAnalogAngle2 = true;
                        }
                    }
                    if (!difAnalogDirectionType) {
                        if (analogBeam.directionType != otherAnalogBeam.directionType) {
                            difAnalogDirectionType = true;
                        }
                    }
                    if (!difAnalogDuration) {
                        if (analogBeam.duration != otherAnalogBeam.duration) {
                            difAnalogDuration = true;
                        }
                    }
                    if (!difAnalogStartTime) {
                        if (analogBeam.startTime != otherAnalogBeam.startTime) {
                            difAnalogStartTime = true;
                        }
                    }

                    // RTCP settings
                    const Observation::RTCPsettings &rtcp(firstObservation->getRTCPsettings());
                    const Observation::RTCPsettings &rtcp2(pObs->getRTCPsettings());

                    const TaskStorage::enableDataProdukts &edp2(pObs->storage()->getOutputDataProductsEnabled());
                    if (!difOutputCoherentStokes) {
                        if (outputDataTypes.coherentStokes != edp2.coherentStokes) {
                            difOutputCoherentStokes = true;
                        }
                    }
                    if (!difoutputCorrelated) {
                        if (outputDataTypes.correlated != edp2.correlated) {
                            difoutputCorrelated = true;
                        }
                    }
                    if (!difOutputIncoherentStokes) {
                        if (outputDataTypes.incoherentStokes != edp2.incoherentStokes) {
                            difOutputIncoherentStokes = true;
                        }
                    }
                    if (!difDelayCompensation) {
                        if (rtcp.delayCompensation != rtcp2.delayCompensation) {
                            difDelayCompensation = true;
                        }
                    }
                    if (!difCorrectBandPass) {
                        if (rtcp.correctBandPass != rtcp2.correctBandPass) {
                            difCorrectBandPass = true;
                        }
                    }
                    if (!difFlysEye) {
                        if (rtcp.flysEye != rtcp2.flysEye) {
                            difFlysEye = true;
                        }
                    }
                    if (!difCoherentStokesType) {
                        if (rtcp.coherentType != rtcp2.coherentType) {
//                            itsTask->setCoherentDataType(DATA_TYPE_UNDEFINED);
                            difCoherentStokesType = true;
                        }
                    }
                    if (!difIncoherentStokesType) {
                        if (rtcp.incoherentType != rtcp2.incoherentType) {
//                            itsTask->setIncoherentDataType(DATA_TYPE_UNDEFINED);
                            difIncoherentStokesType = true;
                        }
                    }
                    if (!difCoherentDedispersion) {
                        if (rtcp.coherentDedisperseChannels != rtcp2.coherentDedisperseChannels) {
                            difCoherentDedispersion = true;
                        }
                    }

                    if (!difCoherentTimeIntegration) {
                        if (rtcp.coherentTimeIntegrationFactor != rtcp2.coherentTimeIntegrationFactor) {
                            difCoherentTimeIntegration = true;
                        }
                    }
                    if (!difIncoherentTimeIntegration) {
                        if (rtcp.incoherentTimeIntegrationFactor != rtcp2.incoherentTimeIntegrationFactor) {
                            difIncoherentTimeIntegration = true;
                        }
                    }
                    if (!difCoherentChannelsPerSubband) {
                        if (rtcp.coherentChannelsPerSubband != rtcp2.coherentChannelsPerSubband) {
                            difCoherentChannelsPerSubband = true;
                        }
                    }
                    if (!difIncoherentChannelsPerSubband) {
                        if (rtcp.incoherentChannelsPerSubband != rtcp2.incoherentChannelsPerSubband) {
                            difIncoherentChannelsPerSubband = true;
                        }
                    }
                    if (!difCoherentSubbandsPerFile) {
                        if (rtcp.coherentSubbandsPerFile != rtcp2.coherentSubbandsPerFile) {
                            difCoherentSubbandsPerFile = true;
                        }
                    }
                    if (!difIncoherentSubbandsPerFile) {
                        if (rtcp.incoherentSubbandsPerFile != rtcp2.incoherentSubbandsPerFile) {
                            difIncoherentSubbandsPerFile = true;
                        }
                    }
                    if (!difBitsPerSample) {
                        if (rtcp.nrBitsPerSample != rtcp2.nrBitsPerSample) {
//                            itsTask->setBitsPerSample(0);
                            difBitsPerSample = true;
                        }
                    }
                    if (!difChannelsPerSubband) {
                        if (rtcp.channelsPerSubband != rtcp2.channelsPerSubband) {
//                            itsTask->setChannelsPerSubband(0);
                            difChannelsPerSubband = true;
                        }
                    }

                    if (!difCorrelatorIntegrationTime) {
                        if (fabs(rtcp.correlatorIntegrationTime - rtcp2.correlatorIntegrationTime) > 0.01) {
//                            itsTask->setCorrelatorIntegrationTime(0.0);
                            difCorrelatorIntegrationTime = true;
                        }
                    }
                } // end if firstObservation
            } // end if firstStationTask
            else if (pTask->isPipeline()) {
                const Pipeline *pipe(static_cast<Pipeline *>(pTask));
                if (pipe->isCalibrationPipeline()) {
                    const CalibrationPipeline *calpipe(static_cast<const CalibrationPipeline *>(pipe));
                    const DemixingSettings &otherDemixSettings(calpipe->demixingSettings());
                    if (!difDemixAlways) {
                        if (otherDemixSettings.demixAlways() != firstDemixingSettings->demixAlways()) {
                            difDemixAlways = true;
                        }
                    }
                    if (!difDemixIfNeeded) {
                        if (otherDemixSettings.demixIfNeeded() != firstDemixingSettings->demixIfNeeded()) {
                            difDemixIfNeeded = true;
                        }
                    }
                    if (!difDemixFreqStep) {
                        if (otherDemixSettings.demixFreqStep() != firstDemixingSettings->demixFreqStep()) {
                            difDemixFreqStep = true;
                        }
                    }
                    if (!difDemixTimeStep) {
                        if (otherDemixSettings.demixTimeStep() != firstDemixingSettings->demixTimeStep()) {
                            difDemixTimeStep = true;
                        }
                    }
                    if (!difAvgFreqStep) {
                        if (otherDemixSettings.freqStep() != firstDemixingSettings->freqStep()) {
                            difAvgFreqStep = true;
                        }
                    }
                    if (!difAvgTimeStep) {
                        if (otherDemixSettings.timeStep() != firstDemixingSettings->timeStep()) {
                            difAvgTimeStep = true;
                        }
                    }
                    if (!difDemixSkyModel) {
                        if (otherDemixSettings.skyModel() != firstDemixingSettings->skyModel()) {
                            difDemixSkyModel = true;
                        }
                    }
                    if (!difCalibrationSkyModel) {
                        if (firstCalibrationPipeline->skyModel() != calpipe->skyModel()) {
                            difCalibrationSkyModel = true;
                        }
                    }
                }
                else if (pipe->isImagingPipeline()) {
                    const ImagingPipeline *impipe(static_cast<const ImagingPipeline *>(pipe));
                    if (!difSpecifyFOV) {
                        if (firstImagingPipeline->specifyFov() != impipe->specifyFov()) {
                            difSpecifyFOV = true;
                        }
                    }

                    if (!difFOV) {
                        if (firstImagingPipeline->fov() != impipe->fov()) {
                            difFOV = true;
                        }
                    }

                    if (!difCellSize) {
                        if (firstImagingPipeline->cellSize() != impipe->cellSize()) {
                            difCellSize = true;
                        }
                    }

                    if (!difNrOfPixels) {
                        if (firstImagingPipeline->nrOfPixels() != impipe->nrOfPixels()) {
                            difNrOfPixels = true;
                        }
                    }

                    if (!difSlicesPerImage) {
                        if (firstImagingPipeline->slicesPerImage() != impipe->slicesPerImage()) {
                            difSlicesPerImage = true;
                        }
                    }

                    if (!difSubbandsPerImage) {
                        if (firstImagingPipeline->subbandsPerImage() != impipe->subbandsPerImage()) {
                            difSubbandsPerImage = true;
                        }
                    }
                }
                else if (pipe->isPulsarPipeline()) {
                    const PulsarPipeline *pulspipe(static_cast<const PulsarPipeline *>(pipe));

                    if (!difPulsar_noRFI && (pulspipe->noRFI() != firstPulsarPipeline->noRFI())) {
                        difPulsar_noRFI = true;
                    }
                    if (!difPulsar_noDSPSR && (pulspipe->skipDspsr() != firstPulsarPipeline->skipDspsr())) {
                        difPulsar_noDSPSR = true;
                    }
                    if (!difPulsar_noFold && (pulspipe->noFold() != firstPulsarPipeline->noFold())) {
                        difPulsar_noFold = true;
                    }
                    if (!difPulsar_noPDMP && (pulspipe->noPdmp() != firstPulsarPipeline->noPdmp())) {
                        difPulsar_noPDMP = true;
                    }
                    if (!difPulsar_rawTo8Bit && (pulspipe->rawTo8Bit() != firstPulsarPipeline->rawTo8Bit())) {
                        difPulsar_rawTo8Bit = true;
                    }
                    if (!difPulsar_RRATS && (pulspipe->rrats() != firstPulsarPipeline->rrats())) {
                        difPulsar_RRATS = true;
                    }
                    if (!difPulsar_singlePulse && (pulspipe->singlePulse() != firstPulsarPipeline->singlePulse())) {
                        difPulsar_singlePulse = true;
                    }
                    if (!difPulsar_skipDynamicSpectrum && (pulspipe->skipDynamicSpectrum() != firstPulsarPipeline->skipDynamicSpectrum())) {
                        difPulsar_skipDynamicSpectrum = true;
                    }
                    if (!difPulsar_skipPrepfold && (pulspipe->skipPrepfold() != firstPulsarPipeline->skipPrepfold())) {
                        difPulsar_skipPrepfold = true;
                    }
                    if (!difPulsar_twoBf2fitsExtra && (pulspipe->twoBf2fitsExtra() != firstPulsarPipeline->twoBf2fitsExtra())) {
                        difPulsar_twoBf2fitsExtra = true;
                    }
                    if (!difPulsar_digifilExtra && (pulspipe->digifilExtra() != firstPulsarPipeline->digifilExtra())) {
                        difPulsar_digifilExtra = true;
                    }
                    if (!difPulsar_dsprExtra && (pulspipe->dspsrExtra() != firstPulsarPipeline->dspsrExtra())) {
                        difPulsar_dsprExtra = true;
                    }
                    if (!difPulsar_prepDataExtra && (pulspipe->prepDataExtra() != firstPulsarPipeline->prepDataExtra())) {
                        difPulsar_prepDataExtra = true;
                    }
                    if (!difPulsar_prepFoldExtra && (pulspipe->prepFoldExtra() != firstPulsarPipeline->prepFoldExtra())) {
                        difPulsar_prepFoldExtra = true;
                    }
                    if (!difPulsar_prepSubbandExtra && (pulspipe->prepSubbandExtra() != firstPulsarPipeline->prepSubbandExtra())) {
                        difPulsar_prepSubbandExtra = true;
                    }
                    if (!difPulsar_PulsarName && (pulspipe->pulsarName() != firstPulsarPipeline->pulsarName())) {
                        difPulsar_PulsarName = true;
                    }
                    if (!difPulsar_rfiFindExtra && (pulspipe->rfiFindExtra() != firstPulsarPipeline->rfiFindExtra())) {
                        difPulsar_rfiFindExtra = true;
                    }
                    if (!difPulsar_decodeNblocks && (pulspipe->decodeNblocks() != firstPulsarPipeline->decodeNblocks())) {
                        difPulsar_decodeNblocks = true;
                    }
                    if (!difPulsar_decodeSigma && (pulspipe->decodeSigma() != firstPulsarPipeline->decodeSigma())) {
                        difPulsar_decodeSigma = true;
                    }
                    if (!difPulsar_tsubint && (pulspipe->tsubInt() != firstPulsarPipeline->tsubInt())) {
                        difPulsar_tsubint = true;
                    }
                    if (!difPulsar_8bitconvSigma && (pulspipe->eightBitConversionSigma() != firstPulsarPipeline->eightBitConversionSigma())) {
                        difPulsar_8bitconvSigma = true;
                    }
                    if (!difPulsar_dynamicSpectrumAvg && (pulspipe->dynamicSpectrumAvg() != firstPulsarPipeline->dynamicSpectrumAvg())) {
                        difPulsar_dynamicSpectrumAvg = true;
                    }
                    if (!difPulsar_rratsDMRange && (pulspipe->rratsDmRange() != firstPulsarPipeline->rratsDmRange())) {
                        difPulsar_rratsDMRange = true;
                    }
                }
                else if (pipe->isLongBaselinePipeline()) {
                    const LongBaselinePipeline *lbpipe(static_cast<const LongBaselinePipeline *>(pipe));
                    if (!difSubbandGroupsPerMS && (lbpipe->subbandGroupsPerMS() != firstLongBaselinePipeline->subbandGroupsPerMS())) {
                        difSubbandGroupsPerMS = true;
                    }
                    if (!difSubbandsPerSubbandGroup && (lbpipe->subbandsPerSubbandGroup() != firstLongBaselinePipeline->subbandsPerSubbandGroup())) {
                        difSubbandsPerSubbandGroup = true;
                    }
                }
            }

            const TaskStorage *tStorage(pTask->storage());
            if (tStorage) {
                if (!difStorageSelectionMode) {
                    if (tStorage->getStorageSelectionMode() != firstStorage->getStorageSelectionMode()) {
                        difStorageSelectionMode = true;
                    }
                }

                if (!difStorageLocations) {
                    if (tStorage->getStorageLocations() != itsTmpStorage) {
                        difStorageLocations = true;
                    }
                }

                if (all_storage_assigned && !tStorage->checkStorageAssigned()) {
                    all_storage_assigned = false;
                }
                if (all_storage_set || no_storage_set) {
                    bool hasStorage(tStorage->hasStorageLocations());
                    if (all_storage_set && !hasStorage) {
                        all_storage_set = false;
                    }
                    if (no_storage_set && hasStorage) {
                        no_storage_set = false;
                    }
                }
            }

            if (allNewTasks) {
                if (pTask->getSASTreeID() != 0) allNewTasks = false;
            }

			// IF (all_storage_assigned) -> storage assigned
			// ELSEIF (all_storage_set) -> storage set
			// ELSEIF (no_storage_set) -> storage not set
			// ELSE -> MIXED
		}

		this->blockSignals(true);


		if (all_storage_assigned) {
			ui.lineEditStorageAssigned->setText("Storage is assigned");
			QPalette palet;
			palet.setColor( QPalette::Base, Qt::green );
			ui.lineEditStorageAssigned->setPalette(palet);
		}
		else if (all_storage_set) {
			ui.lineEditStorageAssigned->setText("Storage not assigned");
			QPalette palet;
			palet.setColor( QPalette::Base, Qt::red );
			ui.lineEditStorageAssigned->setPalette(palet);
		}
		else if (no_storage_set) {
			ui.lineEditStorageAssigned->setText("No storage set");
			ui.lineEditStorageAssigned->setPalette(palette());
		}
		else {
			ui.lineEditStorageAssigned->setText("MIXED");
			ui.lineEditStorageAssigned->setPalette(palette());
		}

		ui.lineEditTaskID->setText("multiple");
		ui.lineEditSASID->setText("multiple");

		// fill taskDialog properties
		// *** processType, processSubtype, strategy ***

//        checkEnableDataTypeSettings();

        ui.comboBoxProcessType->blockSignals(true);
        ui.comboBoxProcessSubType->blockSignals(true);
        ui.comboBoxStrategies->blockSignals(true);

        ui.comboBoxProcessType->setEnabled(allNewTasks);
        ui.comboBoxProcessSubType->setEnabled(allNewTasks);
        ui.comboBoxStrategies->setEnabled(allNewTasks);

		if (!allNewTasks) {
			QString tt(tr("Some tasks are already created in SAS.\nThe processType, processSubType and strategy cannot be changed anymore"));
            ui.comboBoxProcessType->setToolTip(tt);
            ui.comboBoxProcessSubType->setToolTip(tt);
            ui.comboBoxStrategies->setToolTip(tt);
		}
		else {
            ui.comboBoxProcessType->setToolTip("");
            ui.comboBoxProcessSubType->setToolTip("");
            ui.comboBoxStrategies->setToolTip("");
		}

		if (difProcessType) { // tasks of different type
            ui.comboBoxProcessType->setUndefined(true);
            ui.comboBoxProcessSubType->setUndefined(true);
            ui.comboBoxStrategies->setUndefined(true);
		}
		else {
            ui.comboBoxProcessType->setCurrentIndex(type);
            updateProcessSubtypes(processType);
			if (difProcessSubtype) {
                ui.comboBoxProcessSubType->setUndefined(true);
                ui.comboBoxStrategies->setUndefined(true);
			}
			else {
                int idx(ui.comboBoxProcessSubType->findText(processSubtypeStr));
				if (idx != -1) {
                    ui.comboBoxProcessSubType->setCurrentIndex(idx);
				}
				else {
                    ui.comboBoxProcessSubType->addItem(processSubtypeStr);
                    ui.comboBoxProcessSubType->setCurrentIndex(ui.comboBoxProcessSubType->count()-1);
				}
				if (difStrategy) {
                    ui.comboBoxStrategies->setUndefined(true);
				}
				else {
                    idx = ui.comboBoxStrategies->findText(strategy);
					if (idx != -1) {
                        ui.comboBoxStrategies->setCurrentIndex(idx);
					}
					else {
                        ui.comboBoxStrategies->addItem(strategy);
                        ui.comboBoxStrategies->setCurrentIndex(ui.comboBoxStrategies->count()-1);
					}
				}
			}
		}
        ui.comboBoxProcessType->blockSignals(false);
        ui.comboBoxProcessSubType->blockSignals(false);
        ui.comboBoxStrategies->blockSignals(false);


		// *** task status ***
        ui.comboBoxTaskStatus->blockSignals(true);
        ui.comboBoxTaskStatus->clear();
		if (!difStatus) {
			if (status == Task::SCHEDULED) {
				setFinishedTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::SCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::OBSOLETE]);
                ui.comboBoxTaskStatus->setCurrentIndex(3);
			}
			else if (status == Task::ERROR) {
				setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ERROR]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
			}
			else if ((status >= Task::STARTING) && (status <= Task::FINISHED)) {
				setFinishedTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[status]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
			}
			else if (status == Task::ABORTED) {
				setFinishedTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ABORTED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->setCurrentIndex(0);
			}
			else if (status == Task::PRESCHEDULED) {
				setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::SCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::OBSOLETE]);
                ui.comboBoxTaskStatus->setCurrentIndex(1);
			}
			else {
				setNormalTaskMode();
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
                ui.comboBoxTaskStatus->addItem(task_states_str[Task::OBSOLETE]);
				if (status == Task::UNSCHEDULED) {
                    ui.comboBoxTaskStatus->setCurrentIndex(0);
				}
				else if (status == Task::PRESCHEDULED) {
                    ui.comboBoxTaskStatus->setCurrentIndex(1);
				}
				else if (status == Task::ON_HOLD) {
                    ui.comboBoxTaskStatus->setCurrentIndex(2);
				}
				else if (status == Task::OBSOLETE) {
                    ui.comboBoxTaskStatus->setCurrentIndex(3);
				}
			}
		}
		else { // tasks have different status
			if (containsNonEditableTasks) {
				QMessageBox::warning(0, tr("Selection contains non editable tasks"),
						tr("The selection contains finished/aborted/starting tasks which may not be edited.\nAny changes you make will not be applied to these finished tasks"));
			}
			setNormalTaskMode();
//			ui.comboBoxTaskStatus->addItem(task_states_str[Task::PREPARED]);
            ui.comboBoxTaskStatus->addItem(task_states_str[Task::UNSCHEDULED]);
            ui.comboBoxTaskStatus->addItem(task_states_str[Task::PRESCHEDULED]);
            ui.comboBoxTaskStatus->addItem(task_states_str[Task::OBSOLETE]);
            ui.comboBoxTaskStatus->addItem(task_states_str[Task::ON_HOLD]);
            ui.comboBoxTaskStatus->setUndefined(true);
		}
        ui.comboBoxTaskStatus->blockSignals(false);

        // *** stations ***
		if (difStations) {
			ui.labelAssignedStations->setText("Assigned stations " + multipleStr);
            ui.treeWidgetUsedStations->setMixed();
		}
        else if (firstStationTask) {
            setStations(firstStationTask);
			ui.labelAssignedStations->setText("Assigned stations (" + QString::number(countStations()) + ")");
		}

		// *** projectID, project name ***
		if (difProjectID) {
            ui.comboBoxProjectID->setUndefined(true);
			ui.lineEdit_ProjectName->setText(multipleStr);
			ui.lineEdit_ProjectCOI->setText(multipleStr);
			ui.lineEdit_ProjectPI->setText(multipleStr);
		}
		else {
            ui.comboBoxProjectID->setFromString(projectID.c_str());
            ui.lineEdit_ProjectName->setText(itsTask->getProjectName());
            ui.lineEdit_ProjectCOI->setText(itsTask->getProjectCO_I());
            ui.lineEdit_ProjectPI->setText(itsTask->getProjectPI());
		}

		if (difGroupID) {
            ui.lineEditGroupID->setUndefined(true);
		}
		else {
            ui.lineEditGroupID->setText(QString::number(itsTask->getGroupID()));
		}

		// *** taskName ***
		if (difTaskName) {
            ui.lineEditTaskName->setUndefined(true);
		}
		else {
            ui.lineEditTaskName->setText(taskName);
		}

		// *** contactName ***
		if (difContactName) {
			ui.lineEdit_ContactName->setText(multipleStr);
		}
		else {
			ui.lineEdit_ContactName->setText(contactName.c_str());
		}
        itsTask->setContactName(ui.lineEdit_ContactName->text().toStdString()); // change detection

		// *** contactPhone ***
		if (difContactPhone) {
			ui.lineEdit_ContactPhone->setText(multipleStr);
		}
		else {
			ui.lineEdit_ContactPhone->setText(contactPhone.c_str());
		}
        itsTask->setContactPhone(ui.lineEdit_ContactPhone->text().toStdString()); // change detection

		// *** contactEmail ***
		if (difContactEmail) {
			ui.lineEdit_ContactEmail->setText(multipleStr);
		}
		else {
			ui.lineEdit_ContactEmail->setText(contactEmail.c_str());
		}
        itsTask->setContactEmail(ui.lineEdit_ContactEmail->text().toStdString()); // change detection

		// *** taskDescription ***
		if (difTaskDescription) {
			ui.lineEditTaskDescription->setText(multipleStr);
		}
		else {
			ui.lineEditTaskDescription->setText(taskDescription.c_str());
		}
        itsTask->setTaskDescription(ui.lineEditTaskDescription->text().toStdString()); // change detection


		// reservation
		loadReservations();
		if (difReservation) {
            ui.comboBoxReservation->setUndefined();
            ui.comboBoxReservation->setEnabled(false);
		}
		else {
            ui.comboBoxReservation->setEnabled(true);
		}

		// *** firstPossibleDate ***
        ui.dateEditFirstPossibleDate->blockSignals(true);
		const AstroDate &fdate(Controller::theSchedulerSettings.getEarliestSchedulingDay());
        ui.dateEditFirstPossibleDate->setDefaultDate(fdate);
        ui.dateEditFirstPossibleDate->setMinimumDate(QDate(fdate.getYear(),fdate.getMonth(),fdate.getDay()));
		if (difFirstPossibleDate) {
            ui.dateEditFirstPossibleDate->setUndefined(true);
		}
		else {
            ui.dateEditFirstPossibleDate->setUndefined(false);
            const AstroDate &date = itsTask->getWindowFirstDay();
            ui.dateEditFirstPossibleDate->setDate(QDate(date.getYear(), date.getMonth(), date.getDay()));
		}
        ui.dateEditFirstPossibleDate->blockSignals(false);

		// *** lastPossibleDate ***
        ui.dateEditLastPossibleDate->blockSignals(true);
		const AstroDate &ldate(Controller::theSchedulerSettings.getLatestSchedulingDay());
        ui.dateEditLastPossibleDate->setDefaultDate(ldate);
        ui.dateEditLastPossibleDate->setMaximumDate(QDate(ldate.getYear(),ldate.getMonth(),ldate.getDay()));
		if (difLastPossibleDate) {
            ui.dateEditLastPossibleDate->setUndefined(true);
		}
		else {
            ui.dateEditLastPossibleDate->setUndefined(false);
            const AstroDate &date = itsTask->getWindowLastDay();
            ui.dateEditLastPossibleDate->setDate(QDate(date.getYear(), date.getMonth(), date.getDay()));
		}
        ui.dateEditLastPossibleDate->blockSignals(false);

		// *** firstPossibleTime ***
        ui.timeEditFirstPossibleTime->blockSignals(true);
		if (difWindowMinTime) {
            ui.timeEditFirstPossibleTime->setMultipleValue();
		}
		else {
            const AstroTime &time = itsTask->getWindowMinTime();
            ui.timeEditFirstPossibleTime->setTime(QTime(time.getHours(), time.getMinutes(), time.getSeconds()));
		}
        ui.timeEditFirstPossibleTime->blockSignals(false);

		// *** lastPossibleTime ***
        ui.timeEditLastPossibleTime->blockSignals(true);
		if (difWindowMaxTime) {
            ui.timeEditLastPossibleTime->setDefaultTime(AstroTime("23:59:59"));
            ui.timeEditLastPossibleTime->setMultipleValue();
		}
		else {
            const AstroTime &time = itsTask->getWindowMaxTime();
            ui.timeEditLastPossibleTime->setTime(QTime(time.getHours(), time.getMinutes(), time.getSeconds()));
		}
        ui.timeEditLastPossibleTime->blockSignals(false);

		// scheduledStart
        ui.dateTimeEditScheduledStart->blockSignals(true);
		if (difScheduledStart) {
            ui.dateTimeEditScheduledStart->setMultipleValue();
		}
		else {
			const AstroDate &date(Controller::theSchedulerSettings.getEarliestSchedulingDay());
            ui.dateTimeEditScheduledStart->setMinimumDate(QDate(date.getYear(),date.getMonth(),date.getDay()));
            const AstroDateTime &start = itsTask->getScheduledStart();
			setScheduledStart(QDateTime(QDate(start.getYear(), start.getMonth(), start.getDay()),
					QTime(start.getHours(), start.getMinutes(), start.getSeconds())));
		}
        ui.dateTimeEditScheduledStart->blockSignals(false);

		// scheduledEnd
        ui.dateTimeEditScheduledEnd->blockSignals(true);
		if (difScheduledEnd) {
            ui.dateTimeEditScheduledEnd->setMultipleValue();
		}
		else {
            const AstroDateTime &end = itsTask->getScheduledEnd();
			setScheduledEnd(QDateTime(QDate(end.getYear(), end.getMonth(), end.getDay()),
					QTime(end.getHours(), end.getMinutes(), end.getSeconds())));
        }
        ui.dateTimeEditScheduledEnd->blockSignals(false);

		// duration
        ui.lineEditDuration->blockSignals(true);
		if (difDuration) {
            ui.lineEditDuration->setUndefined(true);
		}
		else {
            ui.lineEditDuration->setText(itsTask->getDuration().toString());
		}
        ui.lineEditDuration->blockSignals(false);

		// priority
        ui.lineEditPriority->blockSignals(true);
		if (difPriority) {
            ui.lineEditPriority->setUndefined(true);
		}
		else {
            ui.lineEditPriority->setText(QString::number(itsTask->getPriority()));
		}
        ui.lineEditPriority->blockSignals(false);

		// predecessor
        ui.lineEditPredecessors->blockSignals(true);
        ui.lineEditMaxPredDistance->blockSignals(true);
        ui.lineEditMinPredDistance->blockSignals(true);

        if (predecessors.isEmpty()) {
			if (difPredecessor) {
                ui.lineEditPredecessors->setUndefined(true);
			}
			else {
                ui.lineEditPredecessors->setText(predecessors);
			}

			// predecessor min time dif
			if (difpredMinTimeDif) {
                ui.lineEditMinPredDistance->setUndefined(true);
			}
			else {
                ui.lineEditMinPredDistance->setText(predecessorMinTimeDif.toString());
			}

			// predecessor max time dif
			if (difpredMaxTimeDif) {
                ui.lineEditMaxPredDistance->setUndefined(true);
			}
			else {
                ui.lineEditMaxPredDistance->setText(predecessorMaxTimeDif.toString());
            }

        }
        else {
            ui.lineEditPredecessors->setPalette(QPalette());
            ui.lineEditPredecessors->setText("");
            ui.lineEditMinPredDistance->setText("");
            ui.lineEditMaxPredDistance->setText("");
        }
        ui.lineEditMaxPredDistance->blockSignals(false);
        ui.lineEditPredecessors->blockSignals(false);
        ui.lineEditMinPredDistance->blockSignals(false);

        // nrOFDataSlots
        ui.spinBoxDataslotsPerRSPboard->blockSignals(true);
		if (difNrDataSlotsPerRSP) {
            ui.spinBoxDataslotsPerRSPboard->setUndefined(true);
		}
        else if (firstStationTask) {
            ui.spinBoxDataslotsPerRSPboard->setValue(firstStationTask->getNrOfDataslotsPerRSPboard());
		}
        ui.spinBoxDataslotsPerRSPboard->blockSignals(false);

		// TBB piggyback allowed
        ui.checkBoxTBBPiggybackAllowed->blockSignals(true);
		if (difTBBpiggyback) {
            ui.checkBoxTBBPiggybackAllowed->setToolTip("Allow TBB to piggyback on the stations " + multipleStr);
            ui.checkBoxTBBPiggybackAllowed->setUndefined(true);
		}
		else {
            ui.checkBoxTBBPiggybackAllowed->setTristate(false);
            ui.checkBoxTBBPiggybackAllowed->setToolTip("Allow TBB to piggyback on the stations");
            if (firstObservation) {
                ui.checkBoxTBBPiggybackAllowed->setChecked(firstObservation->getTBBPiggybackAllowed());
            }
		}
        ui.checkBoxTBBPiggybackAllowed->blockSignals(false);

        // Aartfaac piggyback allowed
        ui.checkBoxAartfaacPiggybackAllowed->blockSignals(true);
        if (difAartfaacPiggyback) {
            ui.checkBoxAartfaacPiggybackAllowed->setToolTip("Allow AARTFAAC to piggyback on the stations " + multipleStr);
            ui.checkBoxAartfaacPiggybackAllowed->setUndefined(true);
        }
        else {
            ui.checkBoxAartfaacPiggybackAllowed->setTristate(false);
            ui.checkBoxAartfaacPiggybackAllowed->setToolTip("Allow AARTFAAC to piggyback on the stations");
            if (firstObservation) {
                ui.checkBoxAartfaacPiggybackAllowed->setChecked(firstObservation->getAartfaacPiggybackAllowed());
            }
        }
        ui.checkBoxAartfaacPiggybackAllowed->blockSignals(false);

		// antenna mode
		if (difAntennaMode) {
            ui.comboBoxStationAntennaMode->setUndefined(true);
		}
        else if (firstStationTask) {
            ui.comboBoxStationAntennaMode->setCurrentIndex(firstStationTask->getAntennaMode());
		}

		// station clock
		if (difStationClock) {
            ui.comboBoxStationClock->setUndefined(true);
		}
        else if (firstStationTask) {
            ui.comboBoxStationClock->setCurrentIndex(firstStationTask->getStationClock());
		}

		// filter type
		if (difFilterType) {
            ui.comboBoxStationFilter->setUndefined(true);
		}
        else if (firstStationTask) {
            ui.comboBoxStationFilter->setCurrentIndex(firstStationTask->getFilterType());
		}

		if (difOutputCoherentStokes) {
			ui.checkBoxCoherentStokes->setToolTip("Store coherent Stokes data " + multipleStr);
			ui.checkBoxCoherentStokes->setCheckState(Qt::PartiallyChecked);
		}
		else {
			ui.checkBoxCoherentStokes->setToolTip("Store coherent Stokes data");
            ui.checkBoxCoherentStokes->setChecked(outputDataTypes.coherentStokes/* || itsOutputDataTypes.complexVoltages*/);
		}

		if (difoutputCorrelated) {
			ui.checkBoxCorrelatedData->setToolTip("Store correlated visibilities " + multipleStr);
			ui.checkBoxCorrelatedData->setCheckState(Qt::PartiallyChecked);
		}
		else {
//			ui.checkBoxCorrelatedData->setTristate(false);
			ui.checkBoxCorrelatedData->setToolTip("Store correlated visibilities");
            ui.checkBoxCorrelatedData->setChecked(outputDataTypes.correlated);
		}

		if (difOutputIncoherentStokes) {
			ui.checkBoxIncoherentStokes->setToolTip("Store incoherent Stokes data " + multipleStr);
			ui.checkBoxIncoherentStokes->setCheckState(Qt::PartiallyChecked);
		}
		else {
			ui.checkBoxIncoherentStokes->setToolTip("Store incoherent Stokes data");
            ui.checkBoxIncoherentStokes->setChecked(outputDataTypes.incoherentStokes);
		}

        if (firstObservation) {
            // RTCP settings
            const Observation::RTCPsettings &rtcp(firstObservation->getRTCPsettings());
            if (difCorrelatorIntegrationTime) {
                ui.lineEditCorrelatorIntegrationTime->setUndefined(true);
            }
            else {
                ui.lineEditCorrelatorIntegrationTime->setText(QString::number(rtcp.correlatorIntegrationTime));
            }

            if (difDelayCompensation) {
                ui.checkBox_DelayCompensation->setToolTip("perform delay compensation between stations " + multipleStr);
                ui.checkBox_DelayCompensation->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_DelayCompensation->setToolTip("perform delay compensation between stations");
                ui.checkBox_DelayCompensation->setChecked(rtcp.delayCompensation);
            }

            if (difCorrectBandPass) {
                ui.checkBox_BandpassCorrection->setToolTip("apply the bandpass filter " + multipleStr);
                ui.checkBox_BandpassCorrection->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_BandpassCorrection->setToolTip("apply the bandpass filter");
                ui.checkBox_BandpassCorrection->setChecked(rtcp.correctBandPass);
            }

            if (difFlysEye) {
                ui.checkBoxPencilFlysEye->setToolTip("transforms every station into its own beam " + multipleStr);
                ui.checkBoxPencilFlysEye->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBoxPencilFlysEye->setToolTip("transforms every station into its own beam");
                ui.checkBoxPencilFlysEye->setChecked(rtcp.flysEye);
            }

            if (difCoherentDedispersion) {
                ui.checkBox_CoherentDedispersion->setToolTip("turn on online coherent dedispersion " + multipleStr);
                ui.checkBox_CoherentDedispersion->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_CoherentDedispersion->setToolTip("turn on online coherent dedispersion");
                ui.checkBox_CoherentDedispersion->setChecked(rtcp.coherentDedisperseChannels);
            }

            if (difCoherentStokesType) {
                ui.comboBoxCoherentStokesType->setUndefined(true);
            }
            else {
                ui.comboBoxCoherentStokesType->setCurrentIndex(static_cast<int>(rtcp.coherentType));
            }

            if (difIncoherentStokesType) {
                ui.comboBoxIncoherentStokesType->setUndefined(true);
            }
            else {
                ui.comboBoxIncoherentStokesType->setCurrentIndex(static_cast<int>(rtcp.incoherentType));
            }

            if (difCoherentTimeIntegration) {
                ui.spinBoxCoherentTimeIntegration->setUndefined(true);
            }
            else {
                ui.spinBoxCoherentTimeIntegration->setValue(rtcp.coherentTimeIntegrationFactor);
            }
            if (difIncoherentTimeIntegration) {
                ui.spinBoxIncoherentTimeIntegration->setUndefined(true);
            }
            else {
                ui.spinBoxIncoherentTimeIntegration->setValue(rtcp.incoherentTimeIntegrationFactor);
            }

            if (difCoherentChannelsPerSubband) {
                ui.spinBoxCoherentChannelsPerSubband->setUndefined(true);
            }
            else {
                ui.spinBoxCoherentChannelsPerSubband->setValue(rtcp.coherentChannelsPerSubband);
            }

            if (difIncoherentChannelsPerSubband) {
                ui.spinBoxIncoherentChannelsPerSubband->setUndefined(true);
            }
            else {
                ui.spinBoxIncoherentChannelsPerSubband->setValue(rtcp.incoherentChannelsPerSubband);
            }
            if (difCoherentSubbandsPerFile) {
                ui.spinBoxCoherentSubbandsPerFile->setUndefined(true);
            }
            else {
                ui.spinBoxCoherentSubbandsPerFile->setValue(rtcp.coherentSubbandsPerFile);
            }

            if (difIncoherentSubbandsPerFile) {
                ui.spinBoxIncoherentSubbandsPerFile->setUndefined(true);
            }
            else {
                ui.spinBoxIncoherentSubbandsPerFile->setValue(rtcp.incoherentSubbandsPerFile);
            }

            if (difBitsPerSample) {
                ui.comboBoxBitsPerSample->setUndefined(true);
            }
            else {
                setBitsPerSample(rtcp.nrBitsPerSample);
            }

            if (difChannelsPerSubband) {
                ui.spinBoxChannelsPerSubband->setUndefined(true);
            }
            else {
                ui.spinBoxChannelsPerSubband->setValue(rtcp.channelsPerSubband);
            }
        }

        // fixed day
		if (difFixedDay) {
            ui.checkBoxFixedDate->setToolTip("Fix scheduled date " + multipleStr);
            ui.checkBoxFixedDate->setCheckState(Qt::PartiallyChecked);
		}
		else {
            ui.checkBoxFixedDate->setTristate(false);
            ui.checkBoxFixedDate->setToolTip("Fix scheduled date");
            ui.checkBoxFixedDate->setChecked(itsTask->getFixedDay());
		}

		// fixed time
		if (difFixedTime) {
            ui.checkBoxFixedTime->setToolTip("Fix scheduled time " + multipleStr);
            ui.checkBoxFixedTime->setCheckState(Qt::PartiallyChecked);
		}
		else {
            ui.checkBoxFixedTime->setTristate(false);
            ui.checkBoxFixedTime->setToolTip("Fix scheduled time");
            ui.checkBoxFixedTime->setChecked(itsTask->getFixedTime());
		}

        /*
		// digital beams
		if (difDigitalBeams) {
			ui.tableWidgetDigitalBeams->clearContents();
			ui.tableWidgetDigitalBeams->setRowCount(1);
			QTableWidgetItem *newItem = new QTableWidgetItem(multipleStr);
			ui.tableWidgetDigitalBeams->setItem(0, 0, newItem);
			ui.lineEditTotalSubbands->clear();
			ui.pushButtonAddBeam->setEnabled(false);
			ui.pushButtonEditBeam->setEnabled(false);
			ui.pushButtonDeleteBeams->setEnabled(false);
			ui.pushButtonClearAllBeams->setEnabled(true);
			// disable tied array beam settings as long as digital beams are different
			ui.tableWidgetTiedArrayBeams->clearContents();
			ui.tableWidgetTiedArrayBeams->setRowCount(0);
			ui.pushButtonAddTiedArrayBeam->setEnabled(false);
			ui.pushButtonEditTiedArrayBeam->setEnabled(false);
			ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
			ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
		}
		else { // digital beams are the same or no beams have been defined yet
			itsDigitalBeams.clear();
			ui.tableWidgetDigitalBeams->clearContents();
			ui.tableWidgetDigitalBeams->setRowCount(digitalBeams.size());
			int row(0);
			for (std::map<unsigned, DigitalBeam>::const_iterator it = digitalBeams.begin(); it != digitalBeams.end(); ++it) {
				setDigitalBeam(row++, it->second);
			}
			// set appropriate column widths
			ui.tableWidgetDigitalBeams->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
			// set the total number of subbands
            ui.lineEditTotalSubbands->setText(QString::number(itsTask->getNrOfSubbands()));
			// enable/disable beam buttons depending on if the task has a SAS tree ID already (VIC tree)
			ui.pushButtonAddBeam->setEnabled(!hasSAStreeID);
			if (!digitalBeams.empty()) { // if beams are defined
				ui.pushButtonEditBeam->setEnabled(true); // edit is also show button and should be enabled
				if (containsNonEditableTasks) {
					itsDigitalBeamDialog->setReadOnly(true);
					ui.pushButtonEditBeam->setText("Show");
					ui.pushButtonDeleteBeams->setEnabled(false);
					ui.pushButtonClearAllBeams->setEnabled(false);
				}
				else { // digital beams are the same and editable
					itsDigitalBeamDialog->setReadOnly(false);
					ui.pushButtonEditBeam->setText("Edit");
					ui.pushButtonDeleteBeams->setEnabled(!hasSAStreeID); // depends on VIC tree or template tree
					ui.pushButtonClearAllBeams->setEnabled(!hasSAStreeID);
				}
			}
			else { // if no beams have been defined yet
				itsDigitalBeamDialog->setReadOnly(false);
				ui.pushButtonEditBeam->setText("Edit");
				ui.pushButtonEditBeam->setEnabled(false);
				ui.pushButtonDeleteBeams->setEnabled(false);
				ui.pushButtonClearAllBeams->setEnabled(false);
			}
		}

		// tied array beam (of the first digital beam)
		const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(digitalBeams.find(0)->second.tiedArrayBeams());
		if (difTiedArrayBeams) { // tells us if the tied array beams of the first digital beams are different or not
			if (tiedArrayBeams.empty()) {
				ui.tableWidgetTiedArrayBeams->clearContents();
				ui.tableWidgetTiedArrayBeams->setRowCount(0);
				ui.pushButtonAddTiedArrayBeam->setEnabled(false);
				ui.pushButtonEditTiedArrayBeam->setEnabled(false);
				ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
				ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
			}
			else { // tied array beams of the first digital beam are different
				ui.tableWidgetTiedArrayBeams->clearContents();
				ui.tableWidgetTiedArrayBeams->setRowCount(1);
				QTableWidgetItem *newItem = new QTableWidgetItem(multipleStr);
				ui.tableWidgetTiedArrayBeams->setItem(0, 0, newItem);
				ui.pushButtonAddTiedArrayBeam->setEnabled(false);
				ui.pushButtonEditTiedArrayBeam->setEnabled(false);
				ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
				ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
			}
		}
		else { // tied array beams of the first digital beam are the same, show it
			updateTiedArrayBeams(digitalBeams.find(0)->second);
			if (containsNonEditableTasks) {
				itsTiedArrayBeamDialog->setReadOnly(true);
				ui.pushButtonAddTiedArrayBeam->setEnabled(false);
				ui.pushButtonEditTiedArrayBeam->setEnabled(false);
				ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
				ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
			}
			else {
				itsTiedArrayBeamDialog->setReadOnly(false);
				ui.pushButtonAddTiedArrayBeam->setEnabled(!hasSAStreeID);
				ui.pushButtonDeleteTiedArrayBeam->setEnabled(!hasSAStreeID);
				ui.pushButtonEditTiedArrayBeam->setEnabled(true);
				ui.pushButtonClearAllTiedArrayBeam->setEnabled(!hasSAStreeID);
			}
		}
        */

		// analog beam
        ui.comboBoxAnalogBeamCoordinates->blockSignals(true);
		if (onlyLBA) { // disable analog beam editing
			disableAnalogBeamSettings();
		}
		else {
//			itsAnalogBeamSettings.directionType = DIR_TYPE_J2000; // for multi-edit set to J2000
			if (status < Task::SCHEDULED) {
				ui.groupBoxAnalogBeam->setEnabled(true);
			}
			if (difAnalogDirectionType) { // coordinate system different
                ui.comboBoxAnalogBeamCoordinates->setUndefined(true);
			}
			else { // coordinate system the same
                ui.comboBoxAnalogBeamCoordinates->setCurrentIndex(itsAnalogBeamSettings.directionType);
				setAnalogBeamUnitsComboBox();
			}

            ui.comboBoxAnalogBeamUnits->blockSignals(true);
            ui.comboBoxAnalogBeamUnits->setCurrentIndex(0);
            ui.comboBoxAnalogBeamUnits->blockSignals(false);

			bool setAngles(false);
			if (difAnalogAngle1) {
                ui.lineEditAnalogBeamAngle1->setUndefined(true);
			}
			else {
				setAngles = true;
			}

			if (difAnalogAngle2) {
                ui.lineEditAnalogBeamAngle2->setUndefined(true);
			}
			else {
				setAngles = true;
			}
			if (setAngles) {
                AnalogBeamAngleUnitChanged(ui.comboBoxAnalogBeamUnits->currentText());
			}

			if (difAnalogStartTime) {
				ui.timeEditAnalogBeamStartTime->setTime(QTime(0,0,0));

			}
			else {
				ui.timeEditAnalogBeamStartTime->setTime(QTime(itsAnalogBeamSettings.startTime.getHours(),
						itsAnalogBeamSettings.startTime.getMinutes(), itsAnalogBeamSettings.startTime.getSeconds()));
			}
			if (difAnalogDuration) {
                ui.lineEditAnalogBeamDuration->setToolTip("Active duration of this beam (hhhh:mm:ss) " + multipleStr);
                ui.lineEditAnalogBeamDuration->setUndefined(true);
			}
			else {
                ui.lineEditAnalogBeamDuration->setToolTip("Active duration of this beam (hh:mm:ss)");
                ui.lineEditAnalogBeamDuration->setText(itsAnalogBeamSettings.duration.toString());
			}
//			setAnalogBeamSettings(analogBeam);
		}
        ui.comboBoxAnalogBeamCoordinates->blockSignals(false);

        if (firstDemixingSettings) {
            const QStringList &demixSources(Controller::theSchedulerSettings.getDemixSources());
            if (difDemixAlways) {
                ui.listWidgetDemixAlways->setUndefined(true);
            }
            else {
                ui.listWidgetDemixAlways->addItems(demixSources, firstDemixingSettings->demixAlwaysList());
            }

            if (difDemixIfNeeded) {
                ui.listWidgetDemixIfNeeded->setUndefined(true);
            }
            else {
                ui.listWidgetDemixIfNeeded->addItems(demixSources, firstDemixingSettings->demixIfNeededList());
            }

            if (difDemixFreqStep) {
                ui.spinBoxDemixFreqStep->setUndefined(true);
            }
            else {
                ui.spinBoxDemixFreqStep->setValue(firstDemixingSettings->demixFreqStep());
            }

            if (difDemixTimeStep) {
                ui.spinBoxDemixTimeStep->setUndefined(true);
            }
            else {
                ui.spinBoxDemixTimeStep->setValue(firstDemixingSettings->demixTimeStep());
            }

            if (difAvgFreqStep) {
                ui.spinBoxAveragingFreqStep->setUndefined(true);
            }
            else {
                ui.spinBoxAveragingFreqStep->setValue(firstDemixingSettings->freqStep());
            }

            if (difAvgTimeStep) {
                ui.spinBoxAveragingTimeStep->setUndefined(true);
            }
            else {
                ui.spinBoxAveragingTimeStep->setValue(firstDemixingSettings->timeStep());
            }

            if (difDemixSkyModel) {
                ui.lineEditDemixSkyModel->setUndefined(true);
            }
            else {
                ui.lineEditDemixSkyModel->setText(firstDemixingSettings->skyModel());
            }
        }

        if (firstCalibrationPipeline) {
            if (difCalibrationSkyModel) {
                ui.lineEditCalibrationSkyModel->setUndefined(true);
            }
            else {
                ui.lineEditCalibrationSkyModel->setText(firstCalibrationPipeline->skyModel());
            }
        }

        if (firstImagingPipeline) {
            if (difSpecifyFOV) {
                ui.checkBoxSpecifyFOV->setToolTip("Specify Field Of View instead of Cell size & npix " + multipleStr);
                ui.checkBoxSpecifyFOV->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBoxSpecifyFOV->setTristate(false);
                ui.checkBoxSpecifyFOV->setToolTip("Specify Field Of View instead of Cell size & npix");
                ui.checkBoxSpecifyFOV->setChecked(firstImagingPipeline->specifyFov());
            }

            if (difFOV) {
                ui.lineEditFieldOfView->setUndefined(true);
            }
            else {
                ui.lineEditFieldOfView->setText(QString::number(firstImagingPipeline->fov(),'g',16));
            }

            if (difCellSize) {
                ui.lineEditCellSize->setUndefined(true);
            }
            else {
                ui.lineEditCellSize->setText(firstImagingPipeline->cellSize());
            }

            if (difNrOfPixels) {
                ui.spinBoxNumberOfPixels->setUndefined(true);
            }
            else {
                ui.spinBoxNumberOfPixels->setValue(firstImagingPipeline->nrOfPixels());
            }

            if (difSlicesPerImage) {
                ui.spinBoxSlicesPerImage->setUndefined(true);
            }
            else {
                ui.spinBoxSlicesPerImage->setValue(firstImagingPipeline->slicesPerImage());
            }

            if (difSubbandsPerImage) {
                ui.spinBoxSubbandsPerImage->setUndefined(true);
            }
            else {
                ui.spinBoxSubbandsPerImage->setValue(firstImagingPipeline->subbandsPerImage());
            }
        }

        if (firstPulsarPipeline) {
            if (difPulsar_noRFI) {
                ui.checkBox_NoRFI->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_NoRFI->setTristate(false);
                ui.checkBox_NoRFI->setChecked(firstPulsarPipeline->noRFI());
            }

            if (difPulsar_noDSPSR) {
                ui.checkBox_No_DSPSR->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_No_DSPSR->setTristate(false);
                ui.checkBox_No_DSPSR->setChecked(firstPulsarPipeline->skipDspsr());
            }

            if (difPulsar_noFold) {
                ui.checkBox_No_fold->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_No_fold->setTristate(false);
                ui.checkBox_No_fold->setChecked(firstPulsarPipeline->noFold());
            }

            if (difPulsar_noPDMP) {
                ui.checkBox_No_pdmp->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_No_pdmp->setTristate(false);
                ui.checkBox_No_pdmp->setChecked(firstPulsarPipeline->noPdmp());
            }

            if (difPulsar_rawTo8Bit) {
                ui.checkBox_RawTo8Bit->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_RawTo8Bit->setTristate(false);
                ui.checkBox_RawTo8Bit->setChecked(firstPulsarPipeline->rawTo8Bit());
            }

            if (difPulsar_RRATS) {
                ui.checkBox_rrats->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_rrats->setTristate(false);
                ui.checkBox_rrats->setChecked(firstPulsarPipeline->rrats());
            }
            if (difPulsar_singlePulse) {
                ui.checkBox_Single_pulse->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_Single_pulse->setTristate(false);
                ui.checkBox_Single_pulse->setChecked(firstPulsarPipeline->singlePulse());
            }

            if (difPulsar_skipDynamicSpectrum) {
                ui.checkBox_Skip_dynamic_spectrum->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_Skip_dynamic_spectrum->setTristate(false);
                ui.checkBox_Skip_dynamic_spectrum->setChecked(firstPulsarPipeline->skipDynamicSpectrum());
            }

            if (difPulsar_skipPrepfold) {
                ui.checkBox_Skip_prepfold->setCheckState(Qt::PartiallyChecked);
            }
            else {
                ui.checkBox_Skip_prepfold->setTristate(false);
                ui.checkBox_Skip_prepfold->setChecked(firstPulsarPipeline->skipPrepfold());
            }

            if (difPulsar_twoBf2fitsExtra) {
                ui.lineEdit2bf2fitsExtraOptions->setUndefined(true);
            }
            else {
                ui.lineEdit2bf2fitsExtraOptions->setText(firstPulsarPipeline->twoBf2fitsExtra());
            }

            if (difPulsar_digifilExtra) {
                ui.lineEditDigifilExtraOptions->setUndefined(true);
            }
            else {
                ui.lineEditDigifilExtraOptions->setText(firstPulsarPipeline->digifilExtra());
            }

            if (difPulsar_dsprExtra) {
                ui.lineEditDSPSRextraOptions->setUndefined(true);
            }
            else {
                ui.lineEditDSPSRextraOptions->setText(firstPulsarPipeline->dspsrExtra());
            }

            if (difPulsar_prepDataExtra) {
                ui.lineEditPrepdataExtraOptions->setUndefined(true);
            }
            else {
                ui.lineEditPrepdataExtraOptions->setText(firstPulsarPipeline->prepDataExtra());
            }

            if (difPulsar_prepFoldExtra) {
                ui.lineEditPrepfoldExtraOptions->setUndefined(true);
            }
            else {
                ui.lineEditPrepfoldExtraOptions->setText(firstPulsarPipeline->prepFoldExtra());
            }

            if (difPulsar_prepSubbandExtra) {
                ui.lineEditPrepsubbandExtraOptions->setUndefined(true);
            }
            else {
                ui.lineEditPrepsubbandExtraOptions->setText(firstPulsarPipeline->prepSubbandExtra());
            }

            if (difPulsar_PulsarName) {
                ui.lineEditPulsarName->setUndefined(true);
            }
            else {
                ui.lineEditPulsarName->setText(firstPulsarPipeline->pulsarName());
            }

            if (difPulsar_rfiFindExtra) {
                ui.lineEditRfiFindExtraOptions->setUndefined(true);
            }
            else {
                ui.lineEditRfiFindExtraOptions->setText(firstPulsarPipeline->rfiFindExtra());
            }

            if (difPulsar_decodeNblocks) {
                ui.spinBoxDecodeNblocks->setUndefined(true);
            }
            else {
                ui.spinBoxDecodeNblocks->setValue(firstPulsarPipeline->decodeNblocks());
            }

            if (difPulsar_decodeSigma) {
                ui.spinBoxDecodeSigma->setUndefined(true);
            }
            else {
                ui.spinBoxDecodeSigma->setValue(firstPulsarPipeline->decodeSigma());
            }

            if (difPulsar_tsubint) {
                ui.spinBoxTsubint->setUndefined(true);
            }
            else {
                ui.spinBoxTsubint->setValue(firstPulsarPipeline->tsubInt());
            }

            if (difPulsar_8bitconvSigma) {
                ui.doubleSpinBox8BitConversionSigma->setUndefined(true);
            }
            else {
                ui.doubleSpinBox8BitConversionSigma->setValue(firstPulsarPipeline->eightBitConversionSigma());
            }

            if (difPulsar_dynamicSpectrumAvg) {
                ui.doubleSpinBoxDynamicSpectrumTimeAverage->setUndefined(true);
            }
            else {
                ui.doubleSpinBoxDynamicSpectrumTimeAverage->setValue(firstPulsarPipeline->dynamicSpectrumAvg());
            }

            if (difPulsar_rratsDMRange) {
                ui.doubleSpinBoxRratsDmRange->setUndefined(true);
            }
            else {
                ui.doubleSpinBoxRratsDmRange->setValue(firstPulsarPipeline->rratsDmRange());
            }
        }
        if (firstLongBaselinePipeline) {
            if (difSubbandGroupsPerMS) {
                ui.spinBoxSubbandGroupsPerMS->setUndefined(true);
            }
            else {
                ui.spinBoxSubbandGroupsPerMS->setValue(firstLongBaselinePipeline->subbandGroupsPerMS());
            }

            if (difSubbandsPerSubbandGroup) {
                ui.spinBoxSubbandsPerSubbandGroup->setUndefined(true);
            }
            else {
                ui.spinBoxSubbandsPerSubbandGroup->setValue(firstLongBaselinePipeline->subbandsPerSubbandGroup());
            }
        }

        if (firstStorage) {
            if (difStorageSelectionMode) {
                ui.comboBoxStorageSelectionMode->setUndefined(true);
            }
            else {
                ui.comboBoxStorageSelectionMode->setCurrentIndex(static_cast<int>(firstStorage->getStorageSelectionMode()));
            }
        }

		if (difStorageLocations) {
			setStorageTreeMixed(!statusNotPreScheduled); // override will be enabled only if all tasks have status lower than PRESCHEDULED
		}
		else {
			updateStorageTree();
		}

	}

	// common properties both for reservations as for regular tasks
	ui.pushButtonApply->show();
	enableApplyButtons(false);
	ui.pushButtonOk->setText("Ok");

    blockChangeDetection = false;
	ui.checkBoxCorrelatedData->blockSignals(false);
	ui.checkBoxCoherentStokes->blockSignals(false);
	ui.checkBoxIncoherentStokes->blockSignals(false);
	this->blockSignals(false);
	this->setVisible(true);
}

void TaskDialog::updateStatus(Task::task_status status) {
    ui.comboBoxTaskStatus->blockSignals(true);

	QString stateStr(task_states_str[status]);
    for (int i = 0; i < ui.comboBoxTaskStatus->count(); ++i) {
        if (ui.comboBoxTaskStatus->itemText(i).compare(stateStr) == 0) {
            ui.comboBoxTaskStatus->setCurrentIndex(i);
			return;
		}
	}
	// status not found in combobox -> add it and set it as current
    ui.comboBoxTaskStatus->addItem(stateStr);
    ui.comboBoxTaskStatus->setCurrentIndex(ui.comboBoxTaskStatus->count()-1);

    ui.comboBoxTaskStatus->blockSignals(false);
}


void TaskDialog::update(const Task *task) { // dangerous function -> be aware it might change it task copies. This was a major and difficult to find problem when multi-editing
	if (this->isVisible() && (!isMultiTasks)) {
		if (task) {
            if (itsTask->getID() == task->getID()) { // only do the update if task is the task currently shown
				show(task);
			}
		}
        else show(itsController->getTask(itsTask->getID()));
	}
}

void TaskDialog::setRTCPSettings(const Observation::RTCPsettings &rtcp) {
	this->blockSignals(true);
	ui.checkBoxCorrelatedData->setTristate(false);
	ui.checkBoxCoherentStokes->setTristate(false);
	ui.checkBoxIncoherentStokes->setTristate(false);
	ui.checkBox_DelayCompensation->setTristate(false);
	ui.checkBox_BandpassCorrection->setTristate(false);
	ui.checkBoxPencilFlysEye->setTristate(false);
	ui.checkBox_CoherentDedispersion->setTristate(false);

	ui.checkBoxCorrelatedData->blockSignals(true);
	ui.checkBoxCoherentStokes->blockSignals(true);
	ui.checkBoxIncoherentStokes->blockSignals(true);
    ui.lineEditCorrelatorIntegrationTime->blockSignals(true);
    ui.spinBoxChannelsPerSubband->blockSignals(true);
	ui.checkBoxCorrelatedData->setChecked(itsOutputDataTypes.correlated);
	if (ui.checkBoxCorrelatedData->isChecked()) {
		ui.label_CorrelatorIntTime->setEnabled(true);
        ui.lineEditCorrelatorIntegrationTime->setEnabled(true);
        ui.labelChannelsPerSubband->setEnabled(true);
        ui.spinBoxChannelsPerSubband->setEnabled(true);
	}
	else {
		ui.label_CorrelatorIntTime->setEnabled(false);
        ui.lineEditCorrelatorIntegrationTime->setEnabled(false);
        ui.labelChannelsPerSubband->setEnabled(false);
        ui.spinBoxChannelsPerSubband->setEnabled(false);
    }
    ui.checkBoxCoherentStokes->setChecked(itsOutputDataTypes.coherentStokes);
	ui.checkBoxIncoherentStokes->setChecked(itsOutputDataTypes.incoherentStokes);
	ui.checkBoxCorrelatedData->blockSignals(false);
	ui.checkBoxCoherentStokes->blockSignals(false);
	ui.checkBoxIncoherentStokes->blockSignals(false);
    ui.lineEditCorrelatorIntegrationTime->blockSignals(false);
    ui.spinBoxChannelsPerSubband->blockSignals(false);

    ui.checkBox_CoherentDedispersion->blockSignals(true);
	ui.checkBox_CoherentDedispersion->setChecked(rtcp.coherentDedisperseChannels);
    ui.checkBox_CoherentDedispersion->blockSignals(false);
    ui.checkBox_BandpassCorrection->blockSignals(true);
	ui.checkBox_BandpassCorrection->setChecked(rtcp.correctBandPass);
    ui.checkBox_BandpassCorrection->blockSignals(false);
    ui.checkBox_DelayCompensation->blockSignals(true);
	ui.checkBox_DelayCompensation->setChecked(rtcp.delayCompensation);
    ui.checkBox_DelayCompensation->blockSignals(false);
    ui.comboBoxCoherentStokesType->blockSignals(true);
    ui.comboBoxCoherentStokesType->setCurrentIndex(rtcp.coherentType);
    ui.comboBoxCoherentStokesType->blockSignals(false);
    ui.comboBoxIncoherentStokesType->blockSignals(true);
    ui.comboBoxIncoherentStokesType->setCurrentIndex(rtcp.incoherentType);
    ui.comboBoxIncoherentStokesType->blockSignals(false);
    ui.spinBoxCoherentTimeIntegration->blockSignals(true);
    ui.spinBoxCoherentTimeIntegration->setValue(rtcp.coherentTimeIntegrationFactor);
    ui.spinBoxCoherentTimeIntegration->blockSignals(false);
    ui.spinBoxIncoherentTimeIntegration->blockSignals(true);
    ui.spinBoxIncoherentTimeIntegration->setValue(rtcp.incoherentTimeIntegrationFactor);
    ui.spinBoxIncoherentTimeIntegration->blockSignals(false);
    ui.spinBoxCoherentChannelsPerSubband->blockSignals(true);
    if (rtcp.coherentChannelsPerSubband == 0) {
        ui.spinBoxCoherentChannelsPerSubband->setMinimum(0); // this is not a valid value but if the task has it set to 0 then it should be shown as zero
    }
    else {
        ui.spinBoxCoherentChannelsPerSubband->setMinimum(1);
    }
    ui.spinBoxCoherentChannelsPerSubband->setValue(rtcp.coherentChannelsPerSubband);
    ui.spinBoxCoherentChannelsPerSubband->blockSignals(false);
    ui.spinBoxIncoherentChannelsPerSubband->blockSignals(true);
    if (rtcp.incoherentChannelsPerSubband == 0) {
        ui.spinBoxIncoherentChannelsPerSubband->setMinimum(0); // this is not a valid value but if the task has it set to 0 then it should be shown as zero
    }
    else {
        ui.spinBoxIncoherentChannelsPerSubband->setMinimum(1);
    }
    ui.spinBoxIncoherentChannelsPerSubband->setValue(rtcp.incoherentChannelsPerSubband);
    ui.spinBoxIncoherentChannelsPerSubband->blockSignals(false);
    ui.spinBoxCoherentSubbandsPerFile->blockSignals(true);
    ui.spinBoxCoherentSubbandsPerFile->setValue(rtcp.coherentSubbandsPerFile);
    ui.spinBoxCoherentSubbandsPerFile->blockSignals(false);
    ui.spinBoxIncoherentSubbandsPerFile->blockSignals(true);
    ui.spinBoxIncoherentSubbandsPerFile->setValue(rtcp.incoherentSubbandsPerFile);
    ui.spinBoxIncoherentSubbandsPerFile->blockSignals(false);

    ui.lineEditCorrelatorIntegrationTime->setText(QString::number(rtcp.correlatorIntegrationTime));
	// bits per sample 2,4,8,16 bits
	setBitsPerSample(rtcp.nrBitsPerSample);
    ui.spinBoxChannelsPerSubband->setValue(rtcp.channelsPerSubband);

	ui.checkBoxPencilFlysEye->setChecked(rtcp.flysEye);
	this->blockSignals(false);
}

void TaskDialog::setBitsPerSample(unsigned short bitsPerSample) {
	switch (bitsPerSample) {
	case 4:
        ui.comboBoxBitsPerSample->setCurrentIndex(0);
		break;
	case 8:
        ui.comboBoxBitsPerSample->setCurrentIndex(1);
		break;
	default: // default 16 bits
        ui.comboBoxBitsPerSample->setCurrentIndex(2);
		break;
	}

	// also set maximum nr of dataslots per RSP board
	setMaxNrDataslotsPerRSPboard(bitsPerSample);
}

void TaskDialog::setMaxNrDataslotsPerRSPboard(unsigned short bitsPerSample) {
    ui.spinBoxDataslotsPerRSPboard->blockSignals(true);
	switch (bitsPerSample) {
	case 4:
        ui.spinBoxDataslotsPerRSPboard->setMaximum(MAX_DATASLOT_PER_RSP_4_BITS + 1);
		break;
	case 8:
        ui.spinBoxDataslotsPerRSPboard->setMaximum(MAX_DATASLOT_PER_RSP_8_BITS + 1);
		break;
	default:
        ui.spinBoxDataslotsPerRSPboard->setMaximum(MAX_DATASLOT_PER_RSP_16_BITS + 1);
	break;
	}
    ui.spinBoxDataslotsPerRSPboard->blockSignals(false);
}

void TaskDialog::updateMaxDataslotsPerRSP(void) {
    setMaxNrDataslotsPerRSPboard(ui.comboBoxBitsPerSample->currentText().toUInt());
    ui.spinBoxDataslotsPerRSPboard->blockSignals(true);
    ui.spinBoxDataslotsPerRSPboard->setValue(ui.spinBoxDataslotsPerRSPboard->maximum());
    ui.spinBoxDataslotsPerRSPboard->blockSignals(false);
    detectChanges();
}

void TaskDialog::StoreValues(void) {
	// save current values for comparison to detect changes
    itsStationClockIdx = ui.comboBoxStationClock->currentIndex();
    itsStationAntennaMode = ui.comboBoxStationClock->currentIndex();
    itsStationFilterType = ui.comboBoxStationFilter->currentIndex();

    // beam settings
    itsAnalogBeamSettings = getAnalogBeamSettings();
}

void TaskDialog::addDigitalBeam(void) {
	itsDigitalBeamDialog->reset();
	itsDigitalBeamDialog->exec();
}

void TaskDialog::editDigitalBeam(void) {
	// load the currently selected beam in the digital beam dialog
	int row = ui.tableWidgetDigitalBeams->currentRow();
	if (row >= 0) {
		if (ui.tableWidgetDigitalBeams->item(row, 0)->text().toStdString().compare(MULTIPLE_VALUE_TEXT) != 0) {
			itsDigitalBeamDialog->loadBeamSettings(row, itsDigitalBeams.at(row));
			itsDigitalBeamDialog->exec();
		}
		else QApplication::beep();
	}
	else {
		itsDigitalBeamDialog->loadBeamSettings(0, itsDigitalBeams.at(0));
		itsDigitalBeamDialog->exec();
	}
}

void TaskDialog::deleteDigitalBeam(void) {
	int row = ui.tableWidgetDigitalBeams->currentRow();
	if (row >= 0) {
		ui.tableWidgetDigitalBeams->removeRow(row);
		itsDigitalBeams.erase(row);
		// important! renumber beam numbers in itsDigitalBeam map, to keep track of digital beams
		int i(0);
		std::map<unsigned, DigitalBeam> newBeams;
		for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
			newBeams[i++] = it->second;
		}
		itsDigitalBeams = newBeams;

		if (itsDigitalBeams.empty()) {
			ui.pushButtonClearAllBeams->setEnabled(false);
			ui.pushButtonDeleteBeams->setEnabled(false);
			ui.pushButtonEditBeam->setEnabled(false);
			// also remove the tied array beams from the tied array beam table
			// and update the tied array beam table and buttons
			ui.tableWidgetTiedArrayBeams->clearContents();
			ui.tableWidgetTiedArrayBeams->setRowCount(0);
			ui.pushButtonAddTiedArrayBeam->setEnabled(false);
			ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
			ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
			ui.pushButtonEditTiedArrayBeam->setEnabled(false);
		}
		else { // show the tied array beams of the now selected digital beam
			ui.pushButtonAddTiedArrayBeam->setEnabled(false);
			ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
			ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
			ui.pushButtonEditTiedArrayBeam->setEnabled(false);
//			updateTiedArrayBeams(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()]);
			checkEnableBeamButtons();

		}
		enableApplyButtons(true);
	}
	else {
		QMessageBox::warning(this, tr("No beam selected"),
				tr("No digital beam is selected for deletion."));
	}
	countDigitalBeamSubbands();
}

void TaskDialog::clearAllDigitalBeams(void) {
//	if (!itsDigitalBeams.empty()) {
		itsDigitalBeams.clear();
		ui.tableWidgetDigitalBeams->clearContents();
		ui.tableWidgetDigitalBeams->setRowCount(0);
		countDigitalBeamSubbands();
		changeBeams = true;
		ui.pushButtonClearAllBeams->setEnabled(false);
		ui.pushButtonDeleteBeams->setEnabled(false);
		ui.pushButtonEditBeam->setEnabled(false);
		// also clear the tied array beams (they are part of the digital beam)
//		itsTiedArrayBeams.clear();
		ui.tableWidgetTiedArrayBeams->clearContents();
		ui.tableWidgetTiedArrayBeams->setRowCount(0);
		ui.pushButtonAddTiedArrayBeam->setEnabled(false);
		ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
		ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
		ui.pushButtonEditTiedArrayBeam->setEnabled(false);
		enableApplyButtons(true);
//	}
}

// function used a.o. by DigitalBeamDialog to add or update a beam
void TaskDialog::setDigitalBeam(int beamNr, const DigitalBeam &beam, bool change) {
	// update the beam in the dialog's digital beam table

	if (beamNr == -1) { // -1 : add new beam
		beamNr = itsDigitalBeams.size();
		ui.tableWidgetDigitalBeams->insertRow(beamNr);
		ui.pushButtonEditBeam->setEnabled(true); // only enable these when a new beam is added
		ui.pushButtonDeleteBeams->setEnabled(true);
		ui.pushButtonClearAllBeams->setEnabled(true);
		ui.pushButtonAddTiedArrayBeam->setEnabled(true);
		ui.tableWidgetTiedArrayBeams->clearContents();
		ui.tableWidgetTiedArrayBeams->setRowCount(0);
	}

	QTableWidgetItem *newItem = new QTableWidgetItem(beam.target().c_str());
	ui.tableWidgetDigitalBeams->setItem(beamNr, 0, newItem);
	newItem = new QTableWidgetItem(BEAM_DIRECTION_TYPES[beam.directionType()]);
	ui.tableWidgetDigitalBeams->setItem(beamNr, 1, newItem);
	// angle units
	newItem = new QTableWidgetItem(ANGLE_PAIRS[beam.units()]);
	ui.tableWidgetDigitalBeams->setItem(beamNr, 2, newItem);
	// angle1
	switch (beam.units()) {
	case ANGLE_PAIRS_HMS_DMS:
		newItem = new QTableWidgetItem(beam.angle1().HMSstring().c_str());
		ui.tableWidgetDigitalBeams->setItem(beamNr, 3, newItem);
		// angle2
		newItem = new QTableWidgetItem(beam.angle2().DMSstring().c_str());
		ui.tableWidgetDigitalBeams->setItem(beamNr, 4, newItem);
		break;
	case ANGLE_PAIRS_DMS_DMS:
		newItem = new QTableWidgetItem(beam.angle1().DMSstring().c_str());
		ui.tableWidgetDigitalBeams->setItem(beamNr, 3, newItem);
		// angle2
		newItem = new QTableWidgetItem(beam.angle2().DMSstring().c_str());
		ui.tableWidgetDigitalBeams->setItem(beamNr, 4, newItem);
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
		newItem = new QTableWidgetItem(QString::number(beam.angle1().degree(),'g',16));
		ui.tableWidgetDigitalBeams->setItem(beamNr, 3, newItem);
		// angle2
		newItem = new QTableWidgetItem(QString::number(beam.angle2().degree(),'g',16));
		ui.tableWidgetDigitalBeams->setItem(beamNr, 4, newItem);
		break;
	case ANGLE_PAIRS_RADIANS:
		newItem = new QTableWidgetItem(QString::number(beam.angle1().radian(),'g',16));
		ui.tableWidgetDigitalBeams->setItem(beamNr, 3, newItem);
		// angle2
		newItem = new QTableWidgetItem(QString::number(beam.angle2().radian(),'g',16));
		ui.tableWidgetDigitalBeams->setItem(beamNr, 4, newItem);
		break;
	default:
		break;
	}
	// subband list
	newItem = new QTableWidgetItem(Vector2StringList(beam.subbandList()));
	ui.tableWidgetDigitalBeams->setItem(beamNr, 5, newItem);
	// number of subbands
	newItem = new QTableWidgetItem(QString::number(beam.subbandList().size()));
	ui.tableWidgetDigitalBeams->setItem(beamNr, 6, newItem);
	// start time
	newItem = new QTableWidgetItem(beam.startTime().toString().c_str());
	ui.tableWidgetDigitalBeams->setItem(beamNr, 7, newItem);
	// duration
	newItem = new QTableWidgetItem(beam.duration().toString().c_str());
	ui.tableWidgetDigitalBeams->setItem(beamNr, 8, newItem);
	// nr TAB rings
	newItem = new QTableWidgetItem(QString::number(beam.nrTabRings()));
	ui.tableWidgetDigitalBeams->setItem(beamNr, 9, newItem);
	// duration
	newItem = new QTableWidgetItem(QString::number(beam.tabRingSize(),'g',10));
	ui.tableWidgetDigitalBeams->setItem(beamNr, 10, newItem);

	// update the beam settings in itsDigitalBeams map
	if (change) {
		itsDigitalBeams[beamNr] = beam;
//		enableApplyButtons(true);
		detectChanges();
		countDigitalBeamSubbands();
	}
	// select the just edited or added beam (needed to be able to edit its tied array beams)
	ui.tableWidgetDigitalBeams->selectRow(beamNr);
}

void TaskDialog::countDigitalBeamSubbands(void) {
	unsigned totalSubbands(0);
	for (unsigned i = 0; i < itsDigitalBeams.size(); ++i) {
		totalSubbands += itsDigitalBeams[i].nrSubbands();
	}
	// set the total number of subbands
	ui.lineEditTotalSubbands->setText(QString::number(totalSubbands));
}

// sets the task's digital beam settings in the dialog and stores them in a local copy
void TaskDialog::setDigitalBeamSettings(const Observation *obs) {
	ui.tableWidgetDigitalBeams->blockSignals(true);
	ui.tableWidgetDigitalBeams->clearContents();
	// store the beam settings in a local copy
    itsDigitalBeams = obs->getDigitalBeams();
	ui.tableWidgetDigitalBeams->setRowCount(itsDigitalBeams.size());
	int row(0);
	for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
		setDigitalBeam(row++, it->second);
	}
	// set appropriate column widths
	ui.tableWidgetDigitalBeams->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	// set the total number of subbands
    ui.lineEditTotalSubbands->setText(QString::number(obs->getNrOfSubbands()));
	//also update the tied array beams
	if (!itsDigitalBeams.empty()) {
		ui.tableWidgetDigitalBeams->selectRow(0);
		updateTiedArrayBeams(itsDigitalBeams.begin()->second);
	}
	ui.tableWidgetDigitalBeams->blockSignals(false);
}

// obtains the current digital beam settings from the dialog and stores them in itsDigitalBeams
void TaskDialog::getDigitalBeamSettings(void) {
	itsDigitalBeams.clear();
	DigitalBeam beam;
	QTableWidgetItem * item;
	QString coordUnits;
	for (int row = 0; row < ui.tableWidgetDigitalBeams->rowCount(); ++row) {
		// target name
		item = ui.tableWidgetDigitalBeams->item(row, 0);
		beam.setTarget(item->text().toStdString());
		// coordinate types
		item = ui.tableWidgetDigitalBeams->item(row, 1);
		beam.setDirectionType(stringToBeamDirectionType(item->text().toStdString()));
		// angle units
		item = ui.tableWidgetDigitalBeams->item(row, 2);
		coordUnits = item->text();
		// now read angles according to current coordinate units
		if (coordUnits == ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]) {
			// angle 1
			item = ui.tableWidgetDigitalBeams->item(row, 3);
			beam.setAngle1HMS(item->text().toStdString());
			// angle 2
			item = ui.tableWidgetDigitalBeams->item(row, 4);
			beam.setAngle2DMS(item->text().toStdString());
			// units
			beam.setUnits(ANGLE_PAIRS_HMS_DMS);
		}
		else if (coordUnits == ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]) {
			// angle 1
			item = ui.tableWidgetDigitalBeams->item(row, 3);
			beam.setAngle1DMS(item->text().toStdString());
			// angle 2
			item = ui.tableWidgetDigitalBeams->item(row, 4);
			beam.setAngle2DMS(item->text().toStdString());
			// units
			beam.setUnits(ANGLE_PAIRS_DMS_DMS);
		}
		else if (coordUnits == ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]) {
			// angle 1
			item = ui.tableWidgetDigitalBeams->item(row, 3);
			beam.setAngle1Degree(item->text().toDouble());
			// angle 2
			item = ui.tableWidgetDigitalBeams->item(row, 4);
			beam.setAngle2Degree(item->text().toDouble());
			// units
			beam.setUnits(ANGLE_PAIRS_DECIMAL_DEGREES);
		}
		else if (coordUnits == ANGLE_PAIRS[ANGLE_PAIRS_RADIANS]) {
			// angle 1
			item = ui.tableWidgetDigitalBeams->item(row, 3);
			beam.setAngle1Radian(item->text().toDouble());
			// angle 2
			item = ui.tableWidgetDigitalBeams->item(row, 4);
			beam.setAngle2Radian(item->text().toDouble());
			// units
			beam.setUnits(ANGLE_PAIRS_RADIANS);
		}
		// subband list
		item = ui.tableWidgetDigitalBeams->item(row, 5);
		beam.setSubbandList(item->text());

		// data slots is on column 6 but cannot be changed by user, is determined automatically by the scheduler only shown for info

		// start time
		item = ui.tableWidgetDigitalBeams->item(row, 7);
		beam.setStartTime(AstroTime(item->text().toStdString()));

		// duration
		item = ui.tableWidgetDigitalBeams->item(row, 8);
		beam.setDuration(AstroTime(item->text().toStdString()));

		item = ui.tableWidgetDigitalBeams->item(row, 9);
		beam.setNrTabRings(item->text().toUInt());

		item = ui.tableWidgetDigitalBeams->item(row, 10);
		beam.setTabRingSize(item->text().toDouble());

		// store in itsDigitalBeams
		itsDigitalBeams[row] = beam;
	}
}

void TaskDialog::addTiedArrayBeam(void) {
	itsTiedArrayBeamDialog->setAddMode();
//	itsTiedArrayBeamDialog->setWindowTitle("Add Tied Array Beam " + QString::number(itsTiedArrayBeams.size()));
	itsTiedArrayBeamDialog->setMultiEdit(true);
	itsTiedArrayBeamDialog->exec();
}


void TaskDialog::addNewTiedArrayBeams(int nrBeams, const TiedArrayBeam &TAB) {
	int digiBeamNr(ui.tableWidgetDigitalBeams->currentRow());
	std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(itsDigitalBeams[digiBeamNr].getTiedArrayBeamsForChange());
	int newTABstartNr(tiedArrayBeams.size());
	for (int tabNr = newTABstartNr; tabNr < newTABstartNr + nrBeams; ++tabNr) {
		tiedArrayBeams[tabNr].setAngle1(TAB.angle1());
		tiedArrayBeams[tabNr].setAngle2(TAB.angle2());
		tiedArrayBeams[tabNr].setDispersionMeasure(TAB.dispersionMeasure());
		tiedArrayBeams[tabNr].setCoherent(TAB.isCoherent());
	}
	// update the tied array beams in itsDigitalBeams
//	itsDigitalBeams[digiBeamNr].setTiedArrayBeams(itsTiedArrayBeams);
	// update the tied array beam table to reflect the changes
	ui.tableWidgetTiedArrayBeams->clearContents();
	ui.tableWidgetTiedArrayBeams->setRowCount(tiedArrayBeams.size());
	int row(0);
	for (std::map<unsigned, TiedArrayBeam>::const_iterator it = tiedArrayBeams.begin(); it != tiedArrayBeams.end(); ++it) {
		setTiedArrayBeam(row++, it->second);
	}
	checkEnableBeamButtons();
	enableApplyButtons(true);
}

void TaskDialog::deleteTiedArrayBeam(void) {
	std::vector<unsigned> selectedRows;
	for (int row = 0; row <  ui.tableWidgetTiedArrayBeams->rowCount(); ++row) {
		if (ui.tableWidgetTiedArrayBeams->item(row,0)->isSelected()) {
			selectedRows.push_back(row);
		}
	}
	std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()].getTiedArrayBeamsForChange());
	if (selectedRows.size() != 0) {
		for (std::vector<unsigned>::const_iterator it = selectedRows.begin(); it != selectedRows.end(); ++it) {
			tiedArrayBeams.erase(*it);
		}
        // re-enumerate the tied array beams
        unsigned cnt(0);
        std::map<unsigned, TiedArrayBeam> newTABs;
        for(std::map<unsigned, TiedArrayBeam>::iterator it = tiedArrayBeams.begin(); it != tiedArrayBeams.end(); ++it ) {
            newTABs[cnt++] = it->second;
        }
        tiedArrayBeams = newTABs;

		// remove the selected tied array beams from the tied array beams table
		for (std::vector<unsigned>::reverse_iterator it = selectedRows.rbegin(); it != selectedRows.rend(); ++it) {
			ui.tableWidgetTiedArrayBeams->removeRow(*it);
		}
		checkEnableBeamButtons();
		enableApplyButtons(true);
	}
	else {
		QMessageBox::warning(this, tr("No tied array beam selected"),
				tr("To delete, select one or more tied array beams in the table first."));
	}
}

void TaskDialog::clearAllTiedArrayBeams(void) {
	std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()].getTiedArrayBeamsForChange());
	tiedArrayBeams.clear();
	ui.pushButtonDeleteTiedArrayBeam->setEnabled(false);
	ui.pushButtonClearAllTiedArrayBeam->setEnabled(false);
	ui.pushButtonEditTiedArrayBeam->setEnabled(false);
	ui.tableWidgetTiedArrayBeams->clearContents();
	ui.tableWidgetTiedArrayBeams->setRowCount(0);
    enableApplyButtons(true);
}

void TaskDialog::editTiedArrayBeam(void) {
	// load the currently selected beam in the tied array beam dialog
	std::vector<unsigned> selectedRows;
	for (int row = 0; row <  ui.tableWidgetTiedArrayBeams->rowCount(); ++row) {
		if (ui.tableWidgetTiedArrayBeams->item(row,0)->isSelected()) {
			selectedRows.push_back(row);
		}
	}
	unsigned nrSelectedRows(selectedRows.size());
	std::map<unsigned, TiedArrayBeam> tmpTiedArrays;
	const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()].tiedArrayBeams());
	if (nrSelectedRows == 1) { // single row edit
		itsTiedArrayBeamDialog->setMultiEdit(false);
		std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.find(selectedRows.front());
		if (tit != tiedArrayBeams.end()) {
			tmpTiedArrays[selectedRows.front()] = tit->second;
			itsTiedArrayBeamDialog->loadTiedArrayBeam(tmpTiedArrays);
			itsTiedArrayBeamDialog->exec();
		}
	}
	else if (nrSelectedRows > 1) { // multi row edit
		for (std::vector<unsigned>::const_iterator rit = selectedRows.begin(); rit != selectedRows.end(); ++rit) {
			std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.find(*rit);
			tmpTiedArrays[*rit] = tit->second;
		}
		itsTiedArrayBeamDialog->loadTiedArrayBeam(tmpTiedArrays);
		itsTiedArrayBeamDialog->exec();
	}
	else {
		QMessageBox::warning(this, tr("No tied array beam selected"),
						tr("To edit, select one or more tied array beams in the table first."));
	}
}

void TaskDialog::applyChangeToTiedArrayBeams(const std::vector<unsigned> &tabNrs, const TiedArrayBeam &TAB, const tabProps &applyTABprop) {
	bool change(false);
//	std::map<unsigned, TiedArrayBeam> tiedArrayBeams;
	std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()].getTiedArrayBeamsForChange());
	for (std::vector<unsigned>::const_iterator it = tabNrs.begin(); it != tabNrs.end(); ++it) {
		if (applyTABprop.angle1) { // apply settings for angle1 ?
			tiedArrayBeams[*it].setAngle1(TAB.angle1());
			change = true;
		}
		if (applyTABprop.angle2) {
			tiedArrayBeams[*it].setAngle2(TAB.angle2());
			change = true;
		}
		if (applyTABprop.dispersion_measure) {
			tiedArrayBeams[*it].setDispersionMeasure(TAB.dispersionMeasure());
			change = true;
		}
		if (applyTABprop.coherent) {
			tiedArrayBeams[*it].setCoherent(TAB.isCoherent());
			change = true;
		}
	}
	if (change) {
		// update the tied array beam table to reflect the changes
		ui.tableWidgetTiedArrayBeams->clearContents();
		ui.tableWidgetTiedArrayBeams->setRowCount(tiedArrayBeams.size());
		int row(0);
		for (std::map<unsigned, TiedArrayBeam>::const_iterator it = tiedArrayBeams.begin(); it != tiedArrayBeams.end(); ++it) {
			setTiedArrayBeam(row++, it->second);
		}
		detectChanges();
	}
}

// sets the task's tied array beam settings in the dialog and stores them in a local copy
void TaskDialog::updateTiedArrayBeams(const DigitalBeam &digiBeam) {
    ui.tableWidgetTiedArrayBeams->clearContents();
    ui.tableWidgetTiedArrayBeams->setRowCount(0);

    // store the beam settings in a local copy
    const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(digiBeam.tiedArrayBeams());
    ui.tableWidgetTiedArrayBeams->setRowCount(tiedArrayBeams.size());
    for (std::map<unsigned, TiedArrayBeam>::const_iterator it = tiedArrayBeams.begin(); it != tiedArrayBeams.end(); ++it) {
        setTiedArrayBeam(it->first, it->second);
    }
}

// SLOT showTiedArrayBeams used when clicking in the digital beam dialog to show the digital's beam its tied array beams
void TaskDialog::showTiedArrayBeams(int digitalBeamNr) {
	updateTiedArrayBeams(itsDigitalBeams[digitalBeamNr]);
}

void TaskDialog::setTiedArrayBeam(int TABnr, const TiedArrayBeam &TAB) {
	bool change(false), newTiedArrayBeam(false);
	std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(itsDigitalBeams[ui.tableWidgetDigitalBeams->currentRow()].getTiedArrayBeamsForChange());
	if (TABnr == -1) { // beam = -1 means add a new beam
		newTiedArrayBeam = true;
		TABnr = tiedArrayBeams.size();
		ui.tableWidgetTiedArrayBeams->insertRow(TABnr);
		ui.pushButtonEditTiedArrayBeam->setEnabled(true); // only enable these when a new beam is added
		ui.pushButtonDeleteTiedArrayBeam->setEnabled(true);
		ui.pushButtonClearAllTiedArrayBeam->setEnabled(true);
	}
	// angle1
	QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(TAB.angle1(),'g',16));
	ui.tableWidgetTiedArrayBeams->setItem(TABnr, 0, newItem);
	// angle2
	newItem = new QTableWidgetItem(QString::number(TAB.angle2(),'g',16));
	ui.tableWidgetTiedArrayBeams->setItem(TABnr, 1, newItem);
	// type (coherent/incoherent)
	QString coherent;
	TAB.isCoherent() ? coherent = "coherent" : coherent = "incoherent";
	newItem = new QTableWidgetItem(coherent);
	ui.tableWidgetTiedArrayBeams->setItem(TABnr, 2, newItem);
	// dispersion measure
	newItem = new QTableWidgetItem(QString::number(TAB.dispersionMeasure(),'g',16));
	ui.tableWidgetTiedArrayBeams->setItem(TABnr, 3, newItem);

	// update the TABs
	if (newTiedArrayBeam) {
		change = true;
	}
	else if (TAB != tiedArrayBeams[TABnr]) change = true; // TODO: itsTiedArrayBeams have not been set when called from show(), therefore change is always set true here
	// should this not be changed so that a compare is made with the tiedarraybeams from tsTask ? But of which digital beam then?

	if (change) {
		tiedArrayBeams[TABnr] = TAB;
		enableApplyButtons(true);
//		changeBeams = true;
	}
}


void TaskDialog::FilterTypeChanged(int newFilter) {
	switch (newFilter) {
	case 1:
	case 2:
	case 6:
        ui.comboBoxStationClock->setCurrentIndex(1); // 160 Mhz
		break;
	case 3:
	case 4:
	case 5:
	case 7:
        ui.comboBoxStationClock->setCurrentIndex(2); // 200 Mhz
		break;
	}
	if ((newFilter >= 1) & (newFilter <=4)) {
		ui.groupBoxAnalogBeam->setEnabled(false); // for LBA antenna modes the Analog beam settings are not relevant
        if (ui.comboBoxStationAntennaMode->currentText().startsWith("HBA")) { // discrepancy between new filter type choosen and antenna mode
            ui.comboBoxStationAntennaMode->setCurrentIndex(0, false); // UNSPECIFIED antenna mode
		}
	}
	else {
		ui.groupBoxAnalogBeam->setEnabled(true); // for HBA antenna modes the Analog beam settings need to be specified
		if (newFilter > 4) { // HBA filter type
            if (ui.comboBoxStationAntennaMode->currentText().startsWith("LBA")) { // discrepancy between new filter type choosen and antenna mode
                ui.comboBoxStationAntennaMode->setCurrentIndex(0, false); // UNSPECIFIED antenna mode
			}
		}
	}

	if (!addingReservation && !addingTask) { // disable/enable apply and cancel buttons only when changing a task not when adding a new task
		if (itsStationFilterType != newFilter) {
			enableApplyButtons(true);
			changeStations = true;
		}
		else {
			changeStations = false;
			detectChanges();
		}
	}
}

void TaskDialog::AntennaModeChanged(int newMode) {
	if ((newMode >= 1) & (newMode <=6)) { // LBA modes
		ui.groupBoxAnalogBeam->setEnabled(false); // for LBA antenna modes the Analog beam settings are not relevant
        if (ui.comboBoxStationFilter->currentText().startsWith("HBA")) { // discrepancy between new antenna mode choosen and filter type
            ui.comboBoxStationFilter->setCurrentIndex(0); // UNSPECIFIED filter type
		}
	}
	else {
		ui.groupBoxAnalogBeam->setEnabled(true); // for HBA antenna modes the Analog beam settings need to be specified
		if (newMode > 6) { // HBA antenna mode was chosen
            if (ui.comboBoxStationFilter->currentText().startsWith("LBA")) { // discrepancy between new antenna mode choosen and filter type
                ui.comboBoxStationFilter->setCurrentIndex(0); // UNSPECIFIED filter type
			}
		}
	}

	if (!addingReservation && !addingTask) { // disable/enable apply and cancel buttons only when changing a task not when adding a new task
		if (itsStationAntennaMode != newMode) {
			enableApplyButtons(true);
			changeStations = true;
		}
		else {
			changeStations = false;
			detectChanges();
		}
	}
}

void TaskDialog::StationClockModeChanged(int newClockMode) {
	if (newClockMode) { // 0 = UNSPECIFIED
        const QString &filter = ui.comboBoxStationFilter->currentText();
		if (newClockMode == clock_160Mhz) {
			if ((filter == filter_types_str[LBA_10_70]) |
					(filter == filter_types_str[LBA_30_70]) |	// 160MHz filter types
					(filter == filter_types_str[HBA_170_230])) { }
			else { // 200MHz filter types
				// clock mode chosen that is not compatible with the current station filter
                ui.comboBoxStationFilter->setCurrentIndex(0, false);
			}
		}
	}
	if (!addingReservation && !addingTask) { // disable/enable apply and cancel buttons only when changing a task not when adding a new task
		if (itsStationClockIdx != newClockMode) {
			enableApplyButtons(true);
			changeStations = true;
		}
		else {
			changeStations = false;
			detectChanges();
		}
	}
}

void TaskDialog::setAnalogBeamUnitsComboBox(void) {
	QStringList items;
    beamDirectionType newCoordinateSystem = static_cast<beamDirectionType>(ui.comboBoxAnalogBeamCoordinates->currentIndex());
		switch (newCoordinateSystem) {
		default:
		case DIR_TYPE_J2000: // Right ascension & declination
		case DIR_TYPE_B1950:
		case DIR_TYPE_ICRS:
		case DIR_TYPE_ITRF:
		case DIR_TYPE_TOPO:
		case DIR_TYPE_APP:
			ui.labelBeamAngle1->setText("Right Asc.:");
			ui.labelBeamAngle2->setText("Declination:");
			ui.labelAngleNotation->setText("Units (ra,dec):");
			items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
			      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
			break;
		case DIR_TYPE_HADEC:
			ui.labelBeamAngle1->setText("Hour angle:");
			ui.labelBeamAngle2->setText("Declination:");
			ui.labelAngleNotation->setText("Units (ha,dec):");
			items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
			      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
			break;
		case DIR_TYPE_AZELGEO:
			ui.labelBeamAngle1->setText("Azimuth:");
			ui.labelBeamAngle2->setText("Elevation:");
			ui.labelAngleNotation->setText("Units (az,el):");
			items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
			      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
			break;
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
			ui.labelBeamAngle1->setText("Angle 1:");
			ui.labelBeamAngle2->setText("Angle 2:");
			ui.labelAngleNotation->setText("Units (ang1,ang2):");
			items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
			      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
			break;
			break;
		case DIR_TYPE_GALACTIC:
		case DIR_TYPE_ECLIPTIC:
		case DIR_TYPE_COMET:
			ui.labelBeamAngle1->setText("Longitude:");
			ui.labelBeamAngle2->setText("Latitude:");
			ui.labelAngleNotation->setText("Units (long,lat):");
			items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
			      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
			break;
		}
    ui.comboBoxAnalogBeamUnits->blockSignals(true);
    ui.comboBoxAnalogBeamUnits->clear();
    ui.comboBoxAnalogBeamUnits->addItems(items);
    ui.comboBoxAnalogBeamUnits->blockSignals(false);
}

void TaskDialog::AnalogBeamDirectionTypeChanged(void) {
    beamDirectionType newCoordinateSystem = static_cast<beamDirectionType>(ui.comboBoxAnalogBeamCoordinates->currentIndex());

	if (newCoordinateSystem != itsAnalogBeamSettings.directionType) {

		setAnalogBeamUnitsComboBox();

		itsAnalogBeamSettings.directionType = newCoordinateSystem;

		changeBeams = true;
		enableApplyButtons(true);
	}
	else detectChanges();
}


void TaskDialog::setAnalogBeamAngle1(void) {
	switch (itsAnalogBeamAnglePair) {
	case ANGLE_PAIRS_HMS_DMS:
        if (!itsAnalogBeamSettings.angle1.setHMSangleStr(ui.lineEditAnalogBeamAngle1->text().toStdString())) {
            QPalette palet = ( ui.lineEditAnalogBeamAngle1->palette() );
			palet.setColor( QPalette::Base, Qt::red );
            ui.lineEditAnalogBeamAngle1->setPalette(palet);
			QMessageBox::warning(this, tr("Wrong HMS angle"), tr("The entered HMS angle is invalid"));
			QApplication::beep();
		}
		else {
            ui.lineEditAnalogBeamAngle1->setPalette(QPalette());
		}
		break;
	case ANGLE_PAIRS_DMS_DMS:
        if (!itsAnalogBeamSettings.angle1.setDMSangleStr(ui.lineEditAnalogBeamAngle1->text().toStdString())) {
            QPalette palet = ( ui.lineEditAnalogBeamAngle1->palette() );
			palet.setColor( QPalette::Base, Qt::red );
            ui.lineEditAnalogBeamAngle1->setPalette(palet);
			QMessageBox::warning(this, tr("Wrong DMS angle"), tr("The entered DMS angle is invalid"));
			QApplication::beep();
		}
		else {
            ui.lineEditAnalogBeamAngle1->setPalette(QPalette());
		}
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
        itsAnalogBeamSettings.angle1.setDegreeAngle(ui.lineEditAnalogBeamAngle1->text().toDouble());
        ui.lineEditAnalogBeamAngle1->setPalette(QPalette());
		break;
	case ANGLE_PAIRS_RADIANS:
        itsAnalogBeamSettings.angle1.setRadianAngle(ui.lineEditAnalogBeamAngle1->text().toDouble());
        ui.lineEditAnalogBeamAngle1->setPalette(QPalette());
		break;
	default:
		break;
	}
}

void TaskDialog::setAnalogBeamAngle2(void) {
	switch (itsAnalogBeamAnglePair) {
	case ANGLE_PAIRS_HMS_DMS:
        if (!itsAnalogBeamSettings.angle2.setDMSangleStr(ui.lineEditAnalogBeamAngle2->text().toStdString())) {
            QPalette palet = ( ui.lineEditAnalogBeamAngle2->palette() );
			palet.setColor( QPalette::Base, Qt::red );
            ui.lineEditAnalogBeamAngle2->setPalette(palet);
			QMessageBox::warning(this, tr("Wrong DMS angle"), tr("The entered DMS angle is invalid"));
			QApplication::beep();
		}
		else {
            ui.lineEditAnalogBeamAngle2->setPalette(QPalette());
		}
		break;
	case ANGLE_PAIRS_DMS_DMS:
        if (!itsAnalogBeamSettings.angle2.setDMSangleStr(ui.lineEditAnalogBeamAngle2->text().toStdString())) {
            QPalette palet = ( ui.lineEditAnalogBeamAngle2->palette() );
			palet.setColor( QPalette::Base, Qt::red );
            ui.lineEditAnalogBeamAngle2->setPalette(palet);
			QMessageBox::warning(this, tr("Wrong DMS angle"), tr("The entered DMS angle is invalid"));
			QApplication::beep();
		}
		else {
            ui.lineEditAnalogBeamAngle2->setPalette(QPalette());
		}
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
        itsAnalogBeamSettings.angle2.setDegreeAngle(ui.lineEditAnalogBeamAngle2->text().toDouble());
        ui.lineEditAnalogBeamAngle2->setPalette(QPalette());
		break;
	case ANGLE_PAIRS_RADIANS:
        itsAnalogBeamSettings.angle2.setRadianAngle(ui.lineEditAnalogBeamAngle2->text().toDouble());
        ui.lineEditAnalogBeamAngle2->setPalette(QPalette());
		break;
	default:
		break;
	}
}


void TaskDialog::AnalogBeamAngleUnitChanged(const QString &unitStr) {
	anglePairs newDisplayUnits;

	if (unitStr.compare(ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]) == 0) newDisplayUnits = ANGLE_PAIRS_HMS_DMS;
	else if (unitStr.compare(ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]) == 0) newDisplayUnits = ANGLE_PAIRS_DMS_DMS;
	else if (unitStr.compare(ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]) == 0) newDisplayUnits = ANGLE_PAIRS_DECIMAL_DEGREES;
	else newDisplayUnits = ANGLE_PAIRS_RADIANS;

	if (newDisplayUnits != itsAnalogBeamAnglePair) {
		setAnalogBeamAnglePair(newDisplayUnits);
		itsAnalogBeamAnglePair = newDisplayUnits;
	}
}

void TaskDialog::setComboBoxAnalogBeamUnits(void) const {
	QStringList items;
    ui.comboBoxAnalogBeamUnits->clear();
	switch (itsAnalogBeamSettings.directionType) {
	default:
	case DIR_TYPE_J2000: // Right ascension & declination
	case DIR_TYPE_B1950:
	case DIR_TYPE_ICRS:
	case DIR_TYPE_ITRF:
	case DIR_TYPE_TOPO:
	case DIR_TYPE_APP:
		ui.labelBeamAngle1->setText("Right Asc.:");
		ui.labelBeamAngle2->setText("Declination:");
		ui.labelAngleNotation->setText("Units (ra,dec):");
		items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
	case DIR_TYPE_HADEC:
		ui.labelBeamAngle1->setText("Hour angle:");
		ui.labelBeamAngle2->setText("Declination:");
		ui.labelAngleNotation->setText("Units (ha,dec):");
		items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
	case DIR_TYPE_AZELGEO:
		ui.labelBeamAngle1->setText("Azimuth:");
		ui.labelBeamAngle2->setText("Elevation:");
		ui.labelAngleNotation->setText("Units (az,el):");
		items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			  << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
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
		ui.labelBeamAngle1->setText("Angle 1:");
		ui.labelBeamAngle2->setText("Angle 2:");
		ui.labelAngleNotation->setText("Units (ang1,ang2):");
		items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
		break;
	case DIR_TYPE_GALACTIC:
	case DIR_TYPE_ECLIPTIC:
	case DIR_TYPE_COMET:
		ui.labelBeamAngle1->setText("Longitude:");
		ui.labelBeamAngle2->setText("Latitude:");
		ui.labelAngleNotation->setText("Units (long,lat):");
		items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			  << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
	}
    ui.comboBoxAnalogBeamUnits->addItems(items);
}

void TaskDialog::setAnalogBeamAnglePair(const anglePairs &anglepair) {
    ui.lineEditAnalogBeamAngle1->blockSignals(true); // to prevent false detection of changes
    ui.lineEditAnalogBeamAngle2->blockSignals(true);
	switch (anglepair) {
	case ANGLE_PAIRS_HMS_DMS:
        ui.lineEditAnalogBeamAngle1->setInputMask("00:00:00.000000");
        ui.lineEditAnalogBeamAngle2->setInputMask("#00:00:00.000000");
        ui.lineEditAnalogBeamAngle1->setText(itsAnalogBeamSettings.angle1.HMSstring().c_str());
        ui.lineEditAnalogBeamAngle2->setText(itsAnalogBeamSettings.angle2.DMSstring().c_str());
		break;
	case ANGLE_PAIRS_DMS_DMS:
        ui.lineEditAnalogBeamAngle1->setInputMask("000:00:00.000000");
        ui.lineEditAnalogBeamAngle2->setInputMask("#00:00:00.000000");
        ui.lineEditAnalogBeamAngle1->setText(itsAnalogBeamSettings.angle1.DMSstring().c_str());
        ui.lineEditAnalogBeamAngle2->setText(itsAnalogBeamSettings.angle2.DMSstring().c_str());
		break;
	case ANGLE_PAIRS_RADIANS:
        ui.lineEditAnalogBeamAngle1->setInputMask("0.0000000000");
        ui.lineEditAnalogBeamAngle2->setInputMask("#0.0000000000");
        ui.lineEditAnalogBeamAngle1->setText(QString::number(itsAnalogBeamSettings.angle1.radian(),'g',16));
        ui.lineEditAnalogBeamAngle2->setText(QString::number(itsAnalogBeamSettings.angle2.radian(),'g',16));
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
        ui.lineEditAnalogBeamAngle1->setInputMask("000.0000000000");
        ui.lineEditAnalogBeamAngle2->setInputMask("#00.0000000000");
        ui.lineEditAnalogBeamAngle1->setText(QString::number(itsAnalogBeamSettings.angle1.degree(),'g',16));
        ui.lineEditAnalogBeamAngle2->setText(QString::number(itsAnalogBeamSettings.angle2.degree(),'g',16));
		break;
	default:
		break;
	}
    itsAnalogBeamAngle1Str = ui.lineEditAnalogBeamAngle1->text(); // for change detection
    itsAnalogBeamAngle2Str = ui.lineEditAnalogBeamAngle2->text();
    ui.lineEditAnalogBeamAngle1->blockSignals(false); // to prevent false detection of changes
    ui.lineEditAnalogBeamAngle2->blockSignals(false); // to prevent false detection of changes
}

Observation::analogBeamSettings TaskDialog::getAnalogBeamSettings(void) const {
    Observation::analogBeamSettings beamSettings;
	switch (itsAnalogBeamAnglePair) {
	case ANGLE_PAIRS_HMS_DMS:
        beamSettings.angle1.setHMSangleStr(ui.lineEditAnalogBeamAngle1->text().toStdString());
        beamSettings.angle2.setDMSangleStr(ui.lineEditAnalogBeamAngle2->text().toStdString());
		break;
	case ANGLE_PAIRS_DMS_DMS:
        beamSettings.angle1.setDMSangleStr(ui.lineEditAnalogBeamAngle1->text().toStdString());
        beamSettings.angle2.setDMSangleStr(ui.lineEditAnalogBeamAngle2->text().toStdString());
		break;
	case ANGLE_PAIRS_RADIANS:
        beamSettings.angle1.setRadianAngle(ui.lineEditAnalogBeamAngle1->text().toDouble());
        beamSettings.angle2.setRadianAngle(ui.lineEditAnalogBeamAngle2->text().toDouble());
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
        beamSettings.angle1.setDegreeAngle(ui.lineEditAnalogBeamAngle1->text().toDouble());
        beamSettings.angle2.setDegreeAngle(ui.lineEditAnalogBeamAngle2->text().toDouble());
		break;
	default:
		break;
	}
    beamSettings.directionType = static_cast<beamDirectionType>(ui.comboBoxAnalogBeamCoordinates->currentIndex());
    beamSettings.duration = AstroTime(ui.lineEditAnalogBeamDuration->text().toStdString());
	QTime timeVal = ui.timeEditAnalogBeamStartTime->time();
	beamSettings.startTime = AstroTime(timeVal.hour(), timeVal.minute(), timeVal.second());
	return beamSettings;
}

void TaskDialog::setAnalogBeamSettings(const Observation::analogBeamSettings &beamSettings) {
	itsAnalogBeamSettings = beamSettings;
	itsAnalogBeamSettings = beamSettings;
    ui.comboBoxAnalogBeamCoordinates->blockSignals(true);
    ui.comboBoxAnalogBeamCoordinates->setCurrentIndex(itsAnalogBeamSettings.directionType);
    ui.comboBoxAnalogBeamCoordinates->blockSignals(false);
	setComboBoxAnalogBeamUnits();
	setAnalogBeamAnglePair(itsAnalogBeamAnglePair);
	ui.timeEditAnalogBeamStartTime->setTime(QTime(itsAnalogBeamSettings.startTime.getHours(),
			itsAnalogBeamSettings.startTime.getMinutes(),itsAnalogBeamSettings.startTime.getSeconds()));
    ui.lineEditAnalogBeamDuration->setText(itsAnalogBeamSettings.duration.toString());
}

void TaskDialog::analogBeamStartTimeChanged(const QTime &starttime) {
	AstroTime newStartTime(starttime.hour(), starttime.minute(), starttime.second());
	if (itsAnalogBeamSettings.startTime != newStartTime) {
		itsAnalogBeamSettings.startTime = AstroTime(starttime.hour(), starttime.minute(), starttime.second());
	}
	else detectChanges();
}

void TaskDialog::analogBeamDurationChanged(const QString &duration) {
	itsAnalogBeamSettings.duration = duration;
    if (itsTask->isObservation()) {
        if (itsAnalogBeamSettings.duration != static_cast<const Observation *>(itsTask)->getAnalogBeam().duration) {
            enableApplyButtons(true);
            changeBeams = true;
        }
        else detectChanges();
    }
}

void TaskDialog::loadReservations(void) {
    ui.comboBoxReservation->blockSignals(true);
    ui.comboBoxReservation->clear();
    ui.comboBoxReservation->addItem("no reservation", 0);
    ui.comboBoxReservation->setEnabled(true);
	bool selectReservation(false), foundReservation(false);
    unsigned reservationID(0);
    if (itsTask->isObservation()) {
        reservationID = static_cast<const Observation *>(itsTask)->getReservation();
    }

	if (reservationID) selectReservation = true;
	reservationsMap reservations = itsController->getReservations();
	reservationsMap::const_iterator rit = reservations.begin();
	int idx(1);
	while (rit != reservations.end()) {
		if (rit->second->isReservation()) {
            ui.comboBoxReservation->addItem(QString(rit->second->getTaskName()) + " (" + QString::number(rit->second->getSASTreeID()) + ")", rit->first);
			if (selectReservation) {
				if (rit->first == reservationID) {
                    ui.comboBoxReservation->setCurrentIndex(idx);
                    ui.comboBoxReservation->setEnabled(false);
					ui.pushButtonUnBindReservation->setEnabled(true);
					foundReservation = true;
					selectReservation = false;
				}
				else {
					++idx;
				}
			}
		}
		++rit;
	}
	if (!foundReservation) {
		ui.pushButtonUnBindReservation->setEnabled(false);
	}
    ui.comboBoxReservation->blockSignals(false);
}

void TaskDialog::bindToReservation(int currentIndex) {
    if (itsTask->isObservation()) {
        Observation *obs(static_cast<Observation *>(itsTask));
        unsigned reservationID = ui.comboBoxReservation->itemData(currentIndex).toUInt();
        if (obs->getReservation() != reservationID) {
            //do checks on task for compatibility with reservation and show needed changes if necessary
            std::pair<bool, std::pair<QString, Observation> > reservationCheck = itsController->doReservationChecks(obs, reservationID);
            if (reservationCheck.first) { // changes to the task are needed to be compatible with the reservation, warn user about changes

                if (QMessageBox::question(this, tr("Task changes needed"), reservationCheck.second.first + "\nDo you want to continue with these changes?",
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
                    obs->clone(&reservationCheck.second.second);
                    obs->setReservation(reservationID);
                    // update the dialog with the accepted changes
                    ui.lineEditDuration->blockSignals(true);
                    ui.dateTimeEditScheduledStart->blockSignals(true);
                    ui.dateTimeEditScheduledEnd->blockSignals(true);
                    ui.comboBoxStationClock->blockSignals(true);
                    ui.comboBoxStationFilter->blockSignals(true);
                    ui.lineEditDuration->setText(itsTask->getDuration().toString());
                    const AstroDate &fdate(itsTask->getWindowFirstDay());
                    const AstroTime &ftime(itsTask->getWindowMinTime());
                    const AstroDate &ldate(itsTask->getWindowLastDay());
                    const AstroTime &ltime(itsTask->getWindowMaxTime());
                    const AstroDateTime &start(itsTask->getScheduledStart());
                    const AstroDateTime &stop(itsTask->getScheduledEnd());
                    ui.dateTimeEditScheduledStart->setMinimumDateTime(QDateTime(QDate(fdate.getYear(), fdate.getMonth(), fdate.getDay()), QTime(ftime.getHours(), ftime.getMinutes(), ftime.getSeconds())));
                    //				ui.dateTimeEditScheduledEnd->setMaximumDateTime(QDateTime(QDate(ldate.getYear(), ldate.getMonth(), ldate.getDay()), QTime(ltime.getHours(), ltime.getMinutes(), ltime.getSeconds())));
                    setScheduledStart(QDateTime(QDate(start.getYear(), start.getMonth(), start.getDay()), QTime(start.getHours(), start.getMinutes(), start.getSeconds())));
                    setScheduledEnd(QDateTime(QDate(stop.getYear(), stop.getMonth(), stop.getDay()), QTime(stop.getHours(), stop.getMinutes(), stop.getSeconds())));
                    ui.dateEditFirstPossibleDate->setDate(QDate(fdate.getYear(), fdate.getMonth(), fdate.getDay()));
                    ui.timeEditFirstPossibleTime->setTime(QTime(ftime.getHours(), ftime.getMinutes(), ftime.getSeconds()));
                    ui.dateEditLastPossibleDate->setDate(QDate(ldate.getYear(), ldate.getMonth(), ldate.getDay()));
                    ui.timeEditLastPossibleTime->setTime(QTime(ltime.getHours(), ltime.getMinutes(), ltime.getSeconds()));
                    setStations(obs);
                    ui.comboBoxStationClock->setCurrentIndex(obs->getStationClock());
                    ui.comboBoxStationFilter->setCurrentIndex(obs->getFilterType());
                    ui.comboBoxStationAntennaMode->setCurrentIndex(obs->getAntennaMode());
                    ui.lineEditDuration->blockSignals(false);
                    ui.dateTimeEditScheduledStart->blockSignals(false);
                    ui.dateTimeEditScheduledEnd->blockSignals(false);
                    ui.comboBoxStationClock->blockSignals(false);
                    ui.comboBoxStationFilter->blockSignals(false);

                    changeSchedule = true;
                    enableApplyButtons(true);
                }
                else {
                    ui.comboBoxReservation->blockSignals(true);
                    ui.comboBoxReservation->setCurrentIndex(0);
                    ui.comboBoxReservation->blockSignals(false);
                }
            }
        }
    }
}

void TaskDialog::unbindReservation(void) {
    if (itsTask->isObservation()) {
        static_cast<Observation *>(itsTask)->setReservation(0);
        changeSchedule = true;
        ui.pushButtonUnBindReservation->blockSignals(true);
        ui.comboBoxReservation->blockSignals(true);
        ui.pushButtonUnBindReservation->setEnabled(false);
        ui.comboBoxReservation->setCurrentIndex(0);
        ui.comboBoxReservation->setEnabled(true);
        ui.pushButtonUnBindReservation->blockSignals(false);
        ui.comboBoxReservation->blockSignals(false);
        enableApplyButtons(true);
    }
}
