/*
 * digitalbeamdialog.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 8-june-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/digitalbeamdialog.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "digitalbeamdialog.h"
#include "lofar_utils.h"
#include "taskdialog.h"
#include <QMessageBox>
#include <limits>

DigitalBeamDialog::DigitalBeamDialog(QWidget *parent)
    : QDialog(parent), itsParentTaskDialog(static_cast<TaskDialog *>(parent)), itsBeamNr(-1), change(false)
{
	this->blockSignals(true);
	ui.setupUi(this);
	// load coordinate system combo box
	QStringList items;
	for (short i = _BEGIN_DIRECTION_TYPES; i < _END_DIRECTION_TYPES; ++i) {
		items << BEAM_DIRECTION_TYPES[i];
	}
	ui.comboBoxCoordinateSystem->addItems(items);
	ui.comboBoxCoordinateSystem->setCurrentIndex(0);

	// load angle units combo box
	items.clear();
	items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
	      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
	      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
	      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
	ui.comboBoxAngleUnits->addItems(items);
	ui.comboBoxAngleUnits->setCurrentIndex(0);
	ui.labelAngle1->setText("Right Asc.:");
	ui.labelAngle2->setText("Declination:");

	ui.lineEditAngle1->setToolTip("hh:mm:ss.s (hours:minutes:seconds)");
	ui.lineEditAngle2->setToolTip("+dd:mm:ss.s (degrees:minutes:seconds)");
	ui.lineEditAngle1->setInputMask("00:00:00.0000000");
	ui.lineEditAngle2->setInputMask("#00:00:00.0000000");

	ui.lineEditTabRingSize->setInputMask("0.000000000000000");

	connect(ui.comboBoxAngleUnits, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(switchToNewAngleUnits(const QString &)));
	connect(ui.comboBoxCoordinateSystem, SIGNAL(currentIndexChanged(int)), this, SLOT(switchToNewCoordinates(int)));
	connect(ui.lineEditSubbands, SIGNAL(textChanged(const QString &)), this, SLOT(updateSubbandLineEdit(const QString &)));
	connect(ui.buttonBoxCancelOK, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBoxCancelOK, SIGNAL(rejected()), this, SLOT(reject()));
	ui.lineEditTarget->setFocus();
	this->blockSignals(false);
}

DigitalBeamDialog::~DigitalBeamDialog() {
}

void DigitalBeamDialog::setReadOnly(bool read_only) {
	ui.comboBoxCoordinateSystem->setEnabled(!read_only);
	ui.lineEditTarget->setReadOnly(read_only);
	ui.lineEditAngle1->setReadOnly(read_only);
	ui.lineEditAngle2->setReadOnly(read_only);
	ui.lineEditSubbands->setReadOnly(read_only);
	ui.timeEditStartTime->setReadOnly(read_only);
	ui.lineEditDuration->setReadOnly(read_only);
	ui.spinBoxNrTabRings->setReadOnly(read_only);
	ui.lineEditTabRingSize->setReadOnly(read_only);
	ui.pushButtonSelectSource->setEnabled(!read_only);
	ui.buttonBoxCancelOK->clear();
	if (read_only) {
		ui.buttonBoxCancelOK->addButton(QDialogButtonBox::Close);
	}
	else {
		ui.buttonBoxCancelOK->addButton(QDialogButtonBox::Ok);
		ui.buttonBoxCancelOK->addButton(QDialogButtonBox::Cancel);
	}
}

void DigitalBeamDialog::accept(void) {
	// target name
	std::string newTarget = ui.lineEditTarget->text().toStdString();
	if (newTarget != itsBeam.target()) {
		itsBeam.setTarget(newTarget);
		change = true;
	}

	// subband list
	if (itsBeam.subbandsStr() != ui.lineEditSubbands->text()) {
		change = true;
		bool error(false);
		std::vector<unsigned> newSubbands = StringList2VectorOfUint(ui.lineEditSubbands->text(), error);
		if (!newSubbands.empty()) {
			itsBeam.setSubbandList(newSubbands);
		}
		else {
			QMessageBox::critical(this,tr("Error in subband list"), tr("There is an error in the subband list, please correct it"));
			return;
		}
	}

	// start time and duration
	AstroTime newStartTime(ui.timeEditStartTime->time().hour(), ui.timeEditStartTime->time().minute(), ui.timeEditStartTime->time().second());
	if (newStartTime != itsBeam.startTime()) {
		itsBeam.setStartTime(newStartTime);
		change = true;
	}
	AstroTime newDuration(ui.lineEditDuration->text());
	if (newDuration != itsBeam.duration()) {
		itsBeam.setDuration(newDuration);
		change = true;
	}

	// nr TAB rings
	int nrTabRings(ui.spinBoxNrTabRings->value());
	if (nrTabRings != itsBeam.nrTabRings()) {
		itsBeam.setNrTabRings(nrTabRings);
		change = true;
	}

	// ring Tab size
	const double &tabRingSize(ui.lineEditTabRingSize->text().toDouble());
	if (fabs(tabRingSize - itsBeam.tabRingSize()) > std::numeric_limits<double>::epsilon()) {
		itsBeam.setTabRingSize(tabRingSize);
		change = true;
	}

	if (change) {
		itsParentTaskDialog->setDigitalBeam(itsBeamNr, itsBeam, change);
	}

	QDialog::accept();
}

void DigitalBeamDialog::reset(void) {
	this->blockSignals(true);
	itsBeamNr = -1;
	this->setWindowTitle("Digital Beam");
	ui.comboBoxCoordinateSystem->setCurrentIndex(0);
	ui.comboBoxAngleUnits->setCurrentIndex(0);
	ui.lineEditTarget->setText("");
	ui.lineEditAngle1->setText("00:00:00.0");
	ui.lineEditAngle2->setText("000:00:00.0");
	ui.timeEditStartTime->setTime(QTime(0,0,0));
//	ui.lineEditDuration->setTime(QTime(0,0,0));
	ui.lineEditDuration->setInputMask("0000:00:00");
	ui.lineEditSubbands->setText("[]");
	ui.lineEditTabRingSize->setText("0.0");
	ui.spinBoxNrTabRings->setValue(0);
	itsBeam.clear();
	change = false;
	ui.lineEditTarget->setFocus();
	this->blockSignals(false);
}


void DigitalBeamDialog::setDuration(const AstroTime &time) {
	ui.lineEditDuration->setText(time.toString().c_str());
}

void DigitalBeamDialog::updateSubbandLineEdit(const QString &text) {
	QString newText(text);
	// get current length, needed when a user typed a non permitted character which will be removed, to update to correct cursorposition afterwards
	int oldLength = newText.length();
	// get current cursor position
	int cursorIdx = ui.lineEditSubbands->cursorPosition();
	// replace all single or multiple dots by double dots
	if (itsSubbandString.length() > newText.length()) { // user deleted something possibly a dot?
		for (int i = newText.length()-1; i > 0; --i) { // remove single dots
			if (newText[i] == '.' && newText[i-1] != '.' && newText[i+1] != '.') {
				newText.remove(i,1);
			}
		}
	}
	else { // convert single dots to double dots
		newText.replace(QRegExp("\\.+"),"..");
	}
	// check for ..d(d).. which is an error
	int errIdx = newText.indexOf(QRegExp("\\.{2,2}\\d+\\.{2,2}")); // ..number..
	if (errIdx != -1) {
		newText.remove(newText.indexOf("..",errIdx+2),2);
		QApplication::beep();
	}
	//check for ,..
	errIdx = newText.indexOf(QRegExp(",\\.{2,2}")); // ,..
	if (errIdx != -1) {
		newText.remove(newText.indexOf("..",errIdx),2);
		QApplication::beep();
	}
	//check for ..,
	errIdx = newText.indexOf(QRegExp("\\.{2,2},")); // ..,
	if (errIdx != -1) {
		newText.remove(newText.indexOf(',',errIdx),1);
		QApplication::beep();
	}
	errIdx = newText.indexOf(QRegExp(",{2}")); // more than one comma
	if (errIdx != -1) {
		newText.remove(newText.indexOf(',',errIdx),1);
		QApplication::beep();
	}
	// check brackets
	newText.remove(QRegExp("[^.,0-9]"));
	newText.prepend('[');
	newText.append(']');
	// remove . and , from index 1 and length-1
	while ((newText.at(1) == QChar(',')) | (newText.at(1) == QChar('.'))) newText.remove(1,1);

	// now update with new text
	itsSubbandString = newText;
//	ui.lineEditSubbands->blockSignals(true);
	ui.lineEditSubbands->setText(newText);
	if (oldLength == newText.length()) {
		ui.lineEditSubbands->setCursorPosition(cursorIdx); // return to previous cursorposition
	}
	else if (oldLength < newText.length()) { // '.' (dot) was automatically inserted
		ui.lineEditSubbands->setCursorPosition(cursorIdx+1); // user typed an invalid character that was removed
	}
	else {
		ui.lineEditSubbands->setCursorPosition(cursorIdx-1);
	}
//	ui.lineEditSubbands->blockSignals(false);
}

void DigitalBeamDialog::loadBeamSettings(unsigned beamNr, const DigitalBeam &beam) {
	itsBeamNr = beamNr;
	itsBeam = beam;
	this->setWindowTitle(QString("Digital Beam ") + QString::number(beamNr+1));
	ui.lineEditTarget->setText(beam.target().c_str());
	const AstroTime &beamStartTime(beam.startTime());
	ui.timeEditStartTime->setTime(QTime(beamStartTime.getHours(),beamStartTime.getMinutes(),beamStartTime.getSeconds()));
	ui.lineEditDuration->setText(beam.duration().toString().c_str());
	ui.lineEditSubbands->setText(beam.subbandsStr());
	ui.comboBoxCoordinateSystem->setCurrentIndex(beam.directionType());
	setAngleCoordinateSystem(beam.directionType());
	// set the correct option in comboboxAngleUnits
	for (short i =0; i < ui.comboBoxAngleUnits->count(); ++i) {
		if (ui.comboBoxAngleUnits->itemText(i).compare(ANGLE_PAIRS[beam.units()]) == 0) {
			ui.comboBoxAngleUnits->setCurrentIndex(i);
			break;
		}
	}
	setAngles(beam.units(), itsBeam.angle1(), itsBeam.angle2());
	ui.spinBoxNrTabRings->setValue(itsBeam.nrTabRings());
	ui.lineEditTabRingSize->setText(QString::number(itsBeam.tabRingSize(),'g',15));

	ui.lineEditTarget->setFocus();
	change = false;
}

void DigitalBeamDialog::setAngleCoordinateSystem(const beamDirectionType &coordinateSystem) {
	QStringList items;
	ui.comboBoxAngleUnits->clear();
	switch (coordinateSystem) {
	default:
	case DIR_TYPE_J2000: // Right ascension & declination
	case DIR_TYPE_B1950:
	case DIR_TYPE_ICRS:
	case DIR_TYPE_ITRF:
	case DIR_TYPE_TOPO:
	case DIR_TYPE_APP:
		ui.labelAngle1->setText("Right Asc.:");
		ui.labelAngle2->setText("Declination:");
		items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
	case DIR_TYPE_HADEC:
		ui.labelAngle1->setText("Hour angle:");
		ui.labelAngle2->setText("Declination:");
		items << ANGLE_PAIRS[ANGLE_PAIRS_HMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
	case DIR_TYPE_AZELGEO:
		ui.labelAngle1->setText("Azimuth:");
		ui.labelAngle2->setText("Elevation:");
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
		ui.labelAngle1->setText("Angle 1:");
		ui.labelAngle2->setText("Angle 2:");
		items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
		      << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
		break;
	case DIR_TYPE_GALACTIC:
	case DIR_TYPE_ECLIPTIC:
	case DIR_TYPE_COMET:
		ui.labelAngle1->setText("Longitude:");
		ui.labelAngle2->setText("Latitude:");
		items << ANGLE_PAIRS[ANGLE_PAIRS_DMS_DMS]
			  << ANGLE_PAIRS[ANGLE_PAIRS_DECIMAL_DEGREES]
		      << ANGLE_PAIRS[ANGLE_PAIRS_RADIANS];
		break;
	}

	ui.comboBoxAngleUnits->addItems(items);
}

void DigitalBeamDialog::setAngles(const anglePairs &angleUnits, const Angle &angle1, const Angle &angle2) {
	switch (angleUnits) {
	case ANGLE_PAIRS_HMS_DMS:
		ui.lineEditAngle1->setToolTip("hh:mm:ss.s (hours:minutes:seconds)");
		ui.lineEditAngle2->setToolTip("+dd:mm:ss.s (degrees:minutes:seconds)");
		ui.lineEditAngle1->setInputMask("00:00:00.0000000");
		ui.lineEditAngle2->setInputMask("#00:00:00.0000000");
		ui.lineEditAngle1->setText(angle1.HMSstring().c_str());
		ui.lineEditAngle2->setText(angle2.DMSstring().c_str());
		break;
	case ANGLE_PAIRS_DMS_DMS:
		ui.lineEditAngle1->setToolTip("ddd:mm:ss.s (degrees:minutes:seconds)");
		ui.lineEditAngle2->setToolTip("+dd:mm:ss.s (degrees:minutes:seconds)");
		ui.lineEditAngle1->setInputMask("000:00:00.000000");
		ui.lineEditAngle2->setInputMask("#00:00:00.000000");
		ui.lineEditAngle1->setText(angle1.DMSstring().c_str());
		ui.lineEditAngle2->setText(angle2.DMSstring().c_str());
		break;
	case ANGLE_PAIRS_RADIANS:
		ui.lineEditAngle1->setToolTip("radians");
		ui.lineEditAngle2->setToolTip("radians");
		ui.lineEditAngle1->setInputMask("0.000000000000000");
		ui.lineEditAngle2->setInputMask("#0.000000000000000");
		ui.lineEditAngle1->setText(QString::number(angle1.radian(),'g',15));
		ui.lineEditAngle2->setText(QString::number(angle2.radian(),'g',15));
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
		ui.lineEditAngle1->setToolTip("degrees");
		ui.lineEditAngle2->setToolTip("degrees");
		ui.lineEditAngle1->setInputMask("000.000000000000000");
		ui.lineEditAngle2->setInputMask("#00.000000000000000");
		ui.lineEditAngle1->setText(QString::number(angle1.degree(),'g',15));
		ui.lineEditAngle2->setText(QString::number(angle2.degree(),'g',15));
		break;
	default:
		break;
	}
}

void DigitalBeamDialog::switchToNewAngleUnits(const QString &newUnits) {
	ui.lineEditAngle1->blockSignals(true);
	ui.lineEditAngle2->blockSignals(true);
	for (short i = 0; i < END_ANGLE_PAIRS; ++i) {
		if (newUnits.compare(ANGLE_PAIRS[i]) == 0) {
			itsBeam.setUnits(static_cast<anglePairs>(i));
			break;
		}
	}
	setAngles(itsBeam.units(), itsBeam.angle1(), itsBeam.angle2());
	ui.lineEditAngle1->blockSignals(false);
	ui.lineEditAngle2->blockSignals(false);
	change = true;
}

void DigitalBeamDialog::switchToNewCoordinates(int newCoordinates) {
	ui.lineEditAngle1->blockSignals(true);
	ui.lineEditAngle2->blockSignals(true);
	itsBeam.setDirectionType(static_cast<beamDirectionType>(newCoordinates));
	setAngleCoordinateSystem(itsBeam.directionType());
	switch (itsBeam.directionType()) {
	default:
	case DIR_TYPE_J2000: // Right ascension & declination
	case DIR_TYPE_B1950:
	case DIR_TYPE_ICRS:
	case DIR_TYPE_ITRF:
	case DIR_TYPE_TOPO:
	case DIR_TYPE_APP:
		itsBeam.setUnits(ANGLE_PAIRS_HMS_DMS);
		break;
	case DIR_TYPE_HADEC:
		itsBeam.setUnits(ANGLE_PAIRS_HMS_DMS);
		break;
	case DIR_TYPE_AZELGEO:
		itsBeam.setUnits(ANGLE_PAIRS_DMS_DMS);
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
		itsBeam.setUnits(ANGLE_PAIRS_DMS_DMS);
		break;
	case DIR_TYPE_GALACTIC:
	case DIR_TYPE_ECLIPTIC:
	case DIR_TYPE_COMET:
		itsBeam.setUnits(ANGLE_PAIRS_DMS_DMS);
		break;
	}
	setAngles(itsBeam.units(), itsBeam.angle1(), itsBeam.angle2());
	ui.lineEditAngle1->blockSignals(false);
	ui.lineEditAngle2->blockSignals(false);
	change = true;
}

void DigitalBeamDialog::getAngle1(QString newValue) {
	Angle newAngle;
	switch(itsBeam.units()) {
	case ANGLE_PAIRS_HMS_DMS:
		newAngle.setHMSangleStr(newValue.toStdString());
		break;
	case ANGLE_PAIRS_DMS_DMS:
		newAngle.setDMSangleStr(newValue.toStdString());
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
		newAngle.setDegreeAngle(newValue.toDouble());
		break;
	case ANGLE_PAIRS_RADIANS:
		newAngle.setRadianAngle(newValue.toDouble());
		break;
	default:
		break;
	}
	if (newAngle != itsBeam.angle1()) {
		itsBeam.setAngle1(newAngle);
		change = true;
	}
}

void DigitalBeamDialog::getAngle2(QString newValue) {
	Angle newAngle;
	switch(itsBeam.units()) {
	case ANGLE_PAIRS_HMS_DMS:
		newAngle.setDMSangleStr(newValue.toStdString());
		break;
	case ANGLE_PAIRS_DMS_DMS:
		newAngle.setDMSangleStr(newValue.toStdString());
		break;
	case ANGLE_PAIRS_DECIMAL_DEGREES:
		newAngle.setDegreeAngle(newValue.toDouble());
		break;
	case ANGLE_PAIRS_RADIANS:
		newAngle.setRadianAngle(newValue.toDouble());
		break;
	default:
		break;
	}
	if (newAngle != itsBeam.angle2()) {
		itsBeam.setAngle2(newAngle);
		change = true;
	}
}
