/*
 * ShiftTasksDialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : sept-2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/shifttasksdialog.h $
 *
 */

#ifndef SHIFTTASKSDIALOG_H
#define SHIFTTASKSDIALOG_H

enum moveType {
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_TO_CENTER,
	MOVE_TO_START
};

#include <QDialog>
#include "ui_shifttasksdialog.h"
#include "Controller.h"

class redistributeTasksDialog;

class ShiftTasksDialog : public QDialog
{
    Q_OBJECT

public:
    ShiftTasksDialog(QWidget *parent = 0, Controller *controller = 0);
    ~ShiftTasksDialog();

private slots:
	void accept(void);
	void applyPreview(void); // only applies to this dialog preview, doesn't close the dialog
	void calculateNow(void);
	void applyShift(void);
	void calculateLST(void);
	void setLeftMoveType(void) {itsMoveType = MOVE_LEFT;}
	void setRightMoveType(void) {itsMoveType = MOVE_RIGHT;}
	void doLSTCheck(void);
	void showTableContextMenu(const QPoint &);
	void redistributeSelectedTasks(void);
	void scheduleAfterPredecessor(void);
    void selectObservations(void) {selectTasks(SEL_OBSERVATIONS);}
	void selectAllPipelines(void);
    void selectCalibratorPipelines(void) {selectTasks(SEL_CALIBRATOR_PIPELINES);}
    void selectTargetPipelines(void) {selectTasks(SEL_TARGET_PIPELINES);}
    void selectPreProcessingPipelines(void) {selectTasks(SEL_PREPROCESSING_PIPELINES);}
    void selectLongBaselinePipelines(void) {selectTasks(SEL_LONGBASELINE_PIPELINES);}
    void selectImagingPipelines(void) {selectTasks(SEL_IMAGING_PIPELINES);}
    void selectPulsarPipelines(void) {selectTasks(SEL_PULSAR_PIPELINES);}
    void sortPreview(int, Qt::SortOrder);

private:
	void loadTargets(void);
	void loadTasks(void);
	void selectTasks(int type);
	QList<int> getSelectedRows(void) const;
	int getSelectedTasksTimeSpan(void) const;
	void insertTaskBefore(int rowA, int rowB);

private:
    Ui::ShiftTasksDialogClass ui;
    Controller *itsController;
    redistributeTasksDialog *itsRedistributeDialog;

    moveType itsMoveType;
};

#endif // SHIFTTASKSDIALOG_H
