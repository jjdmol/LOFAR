/*
 * taskdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Nov 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/taskdialog.h $
 *
 */

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QtGui/QDialog>
#include <vector>
#include <string>
#include "lofar_scheduler.h"
#include "ui_taskdialog.h"
#include "task.h"
#include "observation.h"
#include "Angle.h"
#include "digitalbeamdialog.h"
#include "tiedarraybeamdialog.h"
#include "dataslotdialog.h"
#include "DigitalBeam.h"

class Task;
class StationTreeWidget;
class StationListWidget;
class Controller;
class DateEdit;
class TimeEdit;
class LineEdit;
class SpinBox;
class ListWidget;
class DateTimeEdit;
class ComboBox;
class QSpacerItem;
class SpinBox;
class DoubleSpinBox;
class TiedArrayBeamDialog;
class Pipeline;
class CalibrationPipeline;
class ImagingPipeline;
class PulsarPipeline;
class LongBaselinePipeline;
class DemixingSettings;

enum tabIndex {
	TAB_SCHEDULE,
	TAB_STATION_SETTINGS,
	TAB_STATION_BEAMS,
	TAB_PROCESSING,
	TAB_STORAGE,
	TAB_PIPELINE,
    TAB_PULSAR_PIPELINE,
    TAB_LONGBASELINE_PIPELINE,
	TAB_EXTRA_INFO,
	NR_TABS
};

extern const char *tab_names[NR_TABS];

#define STORE true
#define NOSTORE false

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    TaskDialog(QWidget *parentGUI, Controller *controller);
    ~TaskDialog();

//    bool stationsLoaded(void) const {return itsStationsLoaded;}
    void show(const Task *task = 0, tabIndex tab = TAB_SCHEDULE);
    void showMultiEdit(std::vector<Task *> &tasks);
    void update(const Task *task);
    void updateStatus(Task::task_status status);
    void addTask(unsigned taskID);
//    void addReservation(unsigned taskID);
//    void addPipeline(unsigned taskID);
	bool commitChanges(bool storeValues);
	bool commitReservation(bool storeValues);
	bool commitPipeline(void);
	bool commitMultiTasks(void);
//	void updatePredecessor(bool isValid) const;
    void loadAvailableStations(void);
    void setDigitalBeam(int beamNr, const DigitalBeam &beam, bool change = false);
//    void setPencilBeam(int pencilNr, const std::pair<Angle, Angle> &angles);
    void addNewTiedArrayBeams(int nrBeams, const TiedArrayBeam &TAB);
    void applyChangeToTiedArrayBeams(const std::vector<unsigned>& TABnrs,const TiedArrayBeam &, const tabProps &applyTABprop);
    void setExistingProjects(const campaignMap &projects);
    // load the existing process types in the processType ComboBox
    void loadProcessTypes(void);
//    void loadStorageNodesTable(void);

private:
    void apply(bool close);
    void resetChangeDetection(void);
    void enableTabs(void); // enables/disables the correct tabs depending on itsTask type
    void enablePredecessorSettings(bool enable);
    void StoreValues(void);
    void loadTask(const Task *task); // will load the task properties in my embedded task object
    void setReservationTaskMode(void);
    void setNormalTaskMode(void);
    void setFinishedTaskMode(void);
    void setActiveTaskMode(void);
    void setScheduledTaskMode(void);
    void setScheduledStart(const QDateTime &start); // used to set the scheduled start without triggering an end update
    void setScheduledEnd(const QDateTime &end); // used to set the scheduled end without triggering a duration update
    void setAnalogBeamAnglePair(const anglePairs &anglepair);
    void setAnalogBeamUnitsComboBox(void);
    Observation::analogBeamSettings getAnalogBeamSettings(void) const; // returns the beamsettings from the dialog
    void setAnalogBeamSettings(const Observation::analogBeamSettings &);
    void getDigitalBeamSettings(void); // obtains the current digital beam settings from the dialog and stores them in isTmpDigitalBeams
    void setDigitalBeamSettings(const Observation *obs); // sets the current digital beam settings in the dialog
//    void setPencilBeamSettings(const Task *task); // sets the current pencils beam settings in the dialog
    void updateTiedArrayBeams(const DigitalBeam &digiBeam);
    void setRTCPSettings(const Observation::RTCPsettings &rtcp); // sets the RTCP settings in the dialog
    void setComboBoxAnalogBeamUnits(void) const;
    std::vector<std::string> getAssignedStationNames(void) const;
    unsigned countStations(void) const;
    superStationMap getSuperStations(const Observation *task) const;
    void setStations(const StationTask *task);
    std::map<int, QStringList> getSuperStationChildNames(void) const;
    Observation::RTCPsettings getRTCPSettings(void);
    // update the comboBoxStorageDataType for selecting the displayed storage tree
    // and checks if the storage tree itself needsd to be updated as well.
    void setStorageSettings(bool forceUpdate = false);
    void storeStorageSettings(void); // stores the storage locations from the dialog into itsTask
    void setStorageTreeMixed(bool enableOverride);
    void countDigitalBeamSubbands(void); // count the current total nr of subbands and sets them in the dialog
//	void showStorageConflict(const QString &conflictText);
    // returns the currently selected data product type in the combox datatype on the storage tab
    dataProductTypes getSelectedStorageDataProduct(void);
    // get the displayed storage location for the data product currently selected by comboBoxStorageDataType and store in tmpStorage
    void getDisplayedStorageLocations(void);
    void checkStorageSettingsEnable(void); // checks multiple storage requirements and enables storage settings if these requirements are fulfilled
	bool askForApplyChanges(void);
	void checkEnableBeamButtons(void);
	void checkEnableDataTypeSettings(void);
	void updateStorageTab(void);
    void checkForRemovalOfDataProducts(const TaskStorage::enableDataProdukts &edp);
	void setInputDataProductsTree(const Task &task);
	std::map<dataProductTypes, std::vector<bool> > getInputDataFilesCheckState(void);
    void generateFileList(const Task *task, bool noWarnings);
    void loadReservations(void);
    void enableApplyButtons(bool enable);
    void setBitsPerSample(unsigned short bitsPerSample);
    void setMaxNrDataslotsPerRSPboard(unsigned short bitsPerSample);
    void disableAnalogBeamSettings(void);
    void setCurrentTab(tabIndex tab);
    void setDemixSettings(const DemixingSettings &demixing);
//    void setAveragingSettings(const Pipeline *pTask = 0);
    void disableDemixSettings(void);
//    void disableAveragingSettings(void);
    void disableCalibrationPipelineSettings(void);
    void disableImagingPipelineSettings(void);
    void setCalibrationPipelineSettings(const CalibrationPipeline *);
    void setImagingPipelineSettings(const ImagingPipeline *);
    void setPulsarPipelineSettings(const PulsarPipeline *);
    void setLongBaselinePipelineSettings(const LongBaselinePipeline *);
    void updateOriginalTreeID(void);
    void setProcessSubProcessStrategy(const Task *);
    void updateEnabledInputDataTypes(void);
    void clearMultiTasks(void);
    bool ifSettingsChanged(void) const {
        return changeBeams || changeExtraInfo || changeProcessing  || changeSchedule
                || changeStorage || changeStations || changePipeline || changePulsar || changeLongBaseline
                || changeEnabledInputFiles;
    }
    void updateProcessSubtypes(const QString &processType);
    void setPipelineProperties(void);
    void setTabModified(tabIndex tabIdx, bool modified);
    void setAllTabsUnmodified(void);
    void setVisibleTabs(const QVector<tabIndex> &tabs);
signals:
	void abortTask(unsigned int) const;
	void addNewTask(const Task &task) const;
	void checkPredecessor(unsigned int predecessorID) const;

private slots:
    void statusChanged(void);
	void showTiedArrayBeams(int);
	void setAnalogBeamAngle1(void);
	void setAnalogBeamAngle2(void);
	void detectChanges(void);
	void ApplyFirstPossibleDateChange(void);
	void ApplyFirstPossibleTimeChange(void);
	void ApplyLastPossibleTimeChange(void);
    void applyProjectChange(const QString &);
	void updateScheduledEnd(void);
	void updateDuration(void);
	void applyClicked(void);
	void cancelClicked(void);
	void okClicked(void);
	void enableFOVedit(int);
	bool checkEmail();
//	void checkForSpecialType(int);
	void AntennaModeChanged(int);
	void FilterTypeChanged(int);
	void StationClockModeChanged(int);
	void AnalogBeamAngleUnitChanged(const QString &unitStr);
	void AnalogBeamDirectionTypeChanged(void);
	void analogBeamStartTimeChanged(const QTime &starttime);
	void analogBeamDurationChanged(const QString &duration);
	void addDigitalBeam(void);
	void deleteDigitalBeam(void);
	void editDigitalBeam(void);
	void clearAllDigitalBeams(void);
	void addTiedArrayBeam(void);
	void deleteTiedArrayBeam(void);
	void editTiedArrayBeam(void);
	void setTiedArrayBeam(int row, const TiedArrayBeam &TAB);
	void clearAllTiedArrayBeams(void);
	void updateMaxDataslotsPerRSP(void);
    void setPipelineType(void);
	void setCorrelatorIntegrationTime(void);
	void addSuperStation(void);
	void addAvailableStations(const QStringList &stations);
	void checkIfStationChanged();
	void applyStationsRemoved(void);
	void addStationsToUsedStations(const QStringList &stations);
	void showDataSlots(void) {itsDataSlotDialog.exec();}
	void setStorageEditable(bool);
	void doManualStorageOverride(void);
	void doCheckSelectedStorage(void);
	void doUnCheckSelectedStorage(void);
	void countSelectedStorageLocations(void);
	void doTabChangeUpdate(int);
	void outputDataTypesChanged();
	void detectStorageLocationChanges(void);
    void strategyChanged(void);
    void updateProjects(void); // fetch the list of campaigns from SAS and update itsTaskDialog accordingly
    void updateStorageTree(void); // sets the storage locations in the dialog according to itsTask
    void displayDigitalBeamContextMenu(const QPoint &pos);
    void bindToReservation(int);
    void unbindReservation(void);
    void updateStorageSelectionMode(int);
    void processTypeChanged(int ptype);
    void updateStrategiesComboBox(void);
    void detectInputFilesEnabledChanges(void);
    void clearDemixAlways(void);
    void clearDemixIfNeeded(void);

private:
    Ui::TaskDialogClass ui;
    Controller *itsController;
    Task *itsTask; // contains an exact copy of the task currently shown and will contain the changes when user chooses to commit the task changes
    std::vector<Task *> itsMultiTasks; // only used for multi-edit of tasks
    QVector<QWidget *> itsTabs; // pointer to all tabs (for enabling/disabling)
    TaskStorage::enableDataProdukts itsOutputDataTypes;
    bool itsStorageOverflow;
    bool changeSchedule, changeStations, changeBeams, changeProcessing, changeStorage, changePipeline, changePulsar, changeLongBaseline, changeExtraInfo,
        changeEnabledInputFiles, itsNoUnschedule;
    Observation::analogBeamSettings itsAnalogBeamSettings, itsTempAnalogBeamSettings;
    std::map<unsigned, DigitalBeam> itsDigitalBeams, itsTempDigitalBeams;
	DigitalBeamDialog *itsDigitalBeamDialog;
	TiedArrayBeamDialog *itsTiedArrayBeamDialog;
    QString itsAnalogBeamAngle1Str, itsAnalogBeamAngle2Str;
    anglePairs itsAnalogBeamAnglePair; // the current units displayed for the analog beam angles
    int itsStationClockIdx, itsStationAntennaMode, itsStationFilterType;
	bool itsStationsLoaded;
    bool addingTask, addingPipeline, addingReservation, isMultiTasks, blockChangeDetection;
	DataSlotDialog itsDataSlotDialog;
    storageMap itsTmpStorage; // where to store for this task (temporary settings to compare for changes)
	QList<QTreeWidgetItem *> itsStorageTreeLocationsItems, itsStorageTreeNodeItems;

	QAction * itsActionStorageOverride, *itsActionStorageCheckSelected, *itsActionStorageUncheckSelected;
	bool itsStorageOverride;
	std::map<dataProductTypes, int> itsMinNrOfRequiredNodes;
};

#endif // TASKDIALOG_H
