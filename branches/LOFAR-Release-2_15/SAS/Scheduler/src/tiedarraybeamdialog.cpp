#include "tiedarraybeamdialog.h"
#include "ComboBox.h"
#include "LineEdit.h"

#define COHERENT 0
#define INCOHERENT 1

TiedArrayBeamDialog::TiedArrayBeamDialog(QWidget *parent)
    : QDialog(parent), itsParentTaskDialog(static_cast<TaskDialog *>(parent)), itsMultiEdit(false)
{
	ui.setupUi(this);

    // angle1
    itsLineEditAngle1 = new LineEdit(this);
    itsLineEditAngle1->setObjectName(QString::fromUtf8("itsLineEditAngle1"));
    ui.gridLayout->addWidget(itsLineEditAngle1, 1, 1, 1, 2);
    // angle2
    itsLineEditAngle2 = new LineEdit(this);
    itsLineEditAngle2->setObjectName(QString::fromUtf8("itsLineEditAngle2"));
    ui.gridLayout->addWidget(itsLineEditAngle2, 2, 1, 1, 2);
	// type
    itsComboBoxType = new ComboBox(this);
    itsComboBoxType->setObjectName(QString::fromUtf8("itsComboBoxType"));
    QStringList items;
    items << "coherent" << "incoherent";
    itsComboBoxType->addItems(items);
    ui.gridLayout->addWidget(itsComboBoxType, 3, 1, 1, 2);
    // dispersion measure
    itsLineEditDispersionMeasure = new LineEdit(this);
    itsLineEditDispersionMeasure->setObjectName(QString::fromUtf8("itsLineEditDispersionMeasure"));
    ui.gridLayout->addWidget(itsLineEditDispersionMeasure, 4, 1, 1, 2);

    // multi TAB edit off by default
    ui.labelNrOfBeams->hide();
    ui.spinBoxNrOfBeams->hide();
}

TiedArrayBeamDialog::~TiedArrayBeamDialog()
{

}

void TiedArrayBeamDialog::reset(void) {
	itsTABNrs.clear();
	ui.spinBoxNrOfBeams->setValue(1);
	itsTiedArrayBeam = TiedArrayBeam();
	itsLineEditAngle1->setText("0.0");
	itsLineEditAngle2->setText("0.0");
	itsLineEditDispersionMeasure->setText("0.0");
	setReadOnly(false);
}

bool TiedArrayBeamDialog::changesMade(void) {
	 itsTABchanges.angle1 = itsLineEditAngle1->hasBeenChanged();
	 itsTABchanges.angle2 = itsLineEditAngle2->hasBeenChanged();
	 itsTABchanges.dispersion_measure = itsLineEditDispersionMeasure->hasBeenChanged();
	 itsTABchanges.coherent = itsComboBoxType->hasBeenChanged();
	 return itsTABchanges.angle1 || itsTABchanges.angle2 || itsTABchanges.dispersion_measure || itsTABchanges.coherent;
}

void TiedArrayBeamDialog::accept(void) {
	if (itsAddMode) {
		itsTiedArrayBeam.setAngle1(itsLineEditAngle1->text().toDouble());
		itsTiedArrayBeam.setAngle2(itsLineEditAngle2->text().toDouble());
		itsTiedArrayBeam.setDispersionMeasure(itsLineEditDispersionMeasure->text().toDouble());
		itsTiedArrayBeam.setCoherent(itsComboBoxType->currentIndex() == COHERENT);
		itsParentTaskDialog->addNewTiedArrayBeams(ui.spinBoxNrOfBeams->value(), itsTiedArrayBeam);
	}
	else if (changesMade()) {
		if (itsTABchanges.angle1) itsTiedArrayBeam.setAngle1(itsLineEditAngle1->text().toDouble());
		if (itsTABchanges.angle2) itsTiedArrayBeam.setAngle2(itsLineEditAngle2->text().toDouble());
		if (itsTABchanges.dispersion_measure) itsTiedArrayBeam.setDispersionMeasure(itsLineEditDispersionMeasure->text().toDouble());
		if (itsTABchanges.coherent) itsTiedArrayBeam.setCoherent(itsComboBoxType->currentIndex() == COHERENT);
		itsParentTaskDialog->applyChangeToTiedArrayBeams(itsTABNrs, itsTiedArrayBeam, itsTABchanges);
	}
	QDialog::accept();
}


void TiedArrayBeamDialog::setMultiEdit(bool multi_edit) {
	itsMultiEdit = multi_edit;
	if (itsMultiEdit) {
		ui.spinBoxNrOfBeams->setValue(1);
		ui.labelNrOfBeams->show();
		ui.spinBoxNrOfBeams->show();
	}
	else {
		ui.labelNrOfBeams->hide();
		ui.spinBoxNrOfBeams->hide();
	}
}


void TiedArrayBeamDialog::setReadOnly(bool read_only) {
	itsLineEditAngle1->setReadOnly(read_only);
	itsLineEditAngle2->setReadOnly(read_only);
	itsLineEditDispersionMeasure->setReadOnly(read_only);
	itsComboBoxType->setEnabled(!read_only);
	if (read_only) {
		ui.pushButtonOk->hide();
		ui.pushButtonCancel->setText("Close");
	}
	else {
		ui.pushButtonOk->show();
		ui.pushButtonCancel->setText("Cancel");
	}
}


void TiedArrayBeamDialog::loadTiedArrayBeam(const std::map<unsigned, TiedArrayBeam> &TABs) {
	itsAddMode = false;
	itsTiedArrayBeam = TABs.begin()->second;
	if (TABs.size() > 1) {
		this->setWindowTitle("Tied Array Beams (multi)");
		setMultiEdit(true);
	}
	else {
		this->setWindowTitle(QString("Tied Array Beam ") + QString::number(TABs.begin()->first));
	}

	itsTABchanges.angle1 = false;
	itsTABchanges.angle2 = false;
	itsTABchanges.dispersion_measure = false;
	itsTABchanges.coherent = false;
	itsTABNrs.clear();
	if (TABs.size() > 1) {
		for (std::map<unsigned, TiedArrayBeam>::const_iterator it = TABs.begin(); it != TABs.end(); ++it) {
			itsTABNrs.push_back(it->first);
			if (!itsTABchanges.angle1) {
				if (itsTiedArrayBeam.angle1() != it->second.angle1()) {
					itsTABchanges.angle1 = true;
					itsLineEditAngle1->setUndefined(true);
				}
			}
			if (!itsTABchanges.angle2) {
				if (itsTiedArrayBeam.angle2() != it->second.angle2()) {
					itsTABchanges.angle2 = true;
					itsLineEditAngle2->setUndefined(true);
				}
			}
			if (!itsTABchanges.dispersion_measure) {
				if (itsTiedArrayBeam.dispersionMeasure() != it->second.dispersionMeasure()) {
					itsTABchanges.dispersion_measure = true;
					itsLineEditDispersionMeasure->setUndefined(true);
				}
			}
			if (!itsTABchanges.coherent) {
				if (itsTiedArrayBeam.isCoherent() != it->second.isCoherent()) {
					itsTABchanges.coherent = true;
					itsComboBoxType->setUndefined(true);
				}
			}
		}
	}
	else {
		itsTABNrs.push_back(TABs.begin()->first);
	}

	if (!itsTABchanges.angle1) {
		itsLineEditAngle1->setText(QString::number(itsTiedArrayBeam.angle1(),'g',16));
	}
	if (!itsTABchanges.angle2) {
		itsLineEditAngle2->setText(QString::number(itsTiedArrayBeam.angle2(),'g',16));
	}
	if (!itsTABchanges.coherent) {
		if (itsTiedArrayBeam.isCoherent()) {
			itsComboBoxType->setCurrentIndex(COHERENT);
		}
		else {
			itsComboBoxType->setCurrentIndex(INCOHERENT);
		}
	}
	if (!itsTABchanges.dispersion_measure) {
		itsLineEditDispersionMeasure->setText(QString::number(itsTiedArrayBeam.dispersionMeasure(),'g',16));
	}
}
