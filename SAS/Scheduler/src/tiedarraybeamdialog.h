/*
 * tiedarraybeamdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : feb-2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/tiedarraybeamdialog.h $
 *
 */

#ifndef TIEDARRAYBEAMDIALOG_H
#define TIEDARRAYBEAMDIALOG_H

struct tabProps {
	bool angle1, angle2, dispersion_measure, coherent;
};

#include <QtGui/QDialog>
#include "ui_tiedarraybeamdialog.h"
#include "TiedArrayBeam.h"
#include "taskdialog.h"

class ComboBox;
class LineEdit;

class TiedArrayBeamDialog : public QDialog
{
    Q_OBJECT

public:
    TiedArrayBeamDialog(QWidget *parent = 0);
    ~TiedArrayBeamDialog();

    void reset(void);
    void setAddMode(bool add_mode = true) {reset(); itsAddMode = add_mode;}
    void setMultiEdit(bool multi_edit);
    void setReadOnly(bool read_only = true);
    void loadTiedArrayBeam(const std::map<unsigned, TiedArrayBeam> &TABs);

private:
    bool changesMade(void);

private slots:
	void accept(void);

private:
    Ui::TiedArrayBeamDialogClass ui;

    TaskDialog *itsParentTaskDialog;
    ComboBox *itsComboBoxType;
    LineEdit *itsLineEditAngle1, *itsLineEditAngle2, *itsLineEditDispersionMeasure;
    std::vector<unsigned> itsTABNrs;
    TiedArrayBeam itsTiedArrayBeam;
    bool itsAddMode, itsMultiEdit;
    tabProps itsTABchanges;
};

#endif // TIEDARRAYBEAMDIALOG_H
