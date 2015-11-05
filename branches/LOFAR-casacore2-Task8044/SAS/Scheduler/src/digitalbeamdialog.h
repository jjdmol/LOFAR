/*
 * digitalbeamdialog.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 8-june-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/digitalbeamdialog.h $
 *
 */

#ifndef DIGITALBEAMDIALOG_H
#define DIGITALBEAMDIALOG_H

#include <QtGui/QDialog>
#include "ui_digitalbeamdialog.h"
#include "task.h"
#include "Angle.h"
#include "DigitalBeam.h"

class TaskDialog;

class DigitalBeamDialog : public QDialog
{
    Q_OBJECT

public:
    DigitalBeamDialog(QWidget *parent = 0);
    ~DigitalBeamDialog();

    void reset(void);
    void loadBeamSettings(unsigned beamNr, const DigitalBeam &beam);
    void setReadOnly(bool read_only);
    void setBeamNr(unsigned beamNr) {itsBeamNr = beamNr;}
    void setDuration(const AstroTime &time);

private:
	void setAngleCoordinateSystem(const beamDirectionType &coordinateSystem);
	void setAngles(const anglePairs &angleUnits, const Angle &angle1, const Angle &angle2);


private slots:
	void switchToNewAngleUnits(const QString &newUnits);
	void switchToNewCoordinates(int newCoordinates);
	void getAngle1(QString);
	void getAngle2(QString);
	void accept(void);
	void updateSubbandLineEdit(const QString &);

private:
    Ui::DigitalBeamDialogClass ui;
    TaskDialog *itsParentTaskDialog;
    DigitalBeam itsBeam;
    unsigned itsBeamNr;
    bool change;
    QString itsSubbandString;
 //   anglePairs itsBeam.units;
 //   Angle itsAngle1, itsAngle2;
};

#endif // DIGITALBEAMDIALOG_H
