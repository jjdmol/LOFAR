//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//



#include <uvplot/UVPGraphSettingsWidget.h>

#include <qlayout.h>

#include <sstream>


//=================>>>  UVPGraphSettingsWidget::UVPGraphSettingsWidget  <<<=================

UVPGraphSettingsWidget::UVPGraphSettingsWidget(unsigned int numOfAntennae,
                                               QWidget *    parent,
                                               const char * name,
                                               WFlags       f)
  : QWidget(parent, name, f),
    itsNumberOfBaselines(0)
{
  itsAntenna1Slider = new QSlider(1, numOfAntennae, 1, 1,
                                  QSlider::Horizontal, this, "Antenna 1");
  itsAntenna2Slider = new QSlider(1, numOfAntennae, 1, numOfAntennae,
                                  QSlider::Horizontal, this, "Antenna 2");
  itsAntenna1Label  = new QLabel("Antenna 1: ", this);
  itsAntenna2Label  = new QLabel("Antenna 2: ", this);


  itsCorrelationCombo = new QComboBox(false, this);
  itsCorrelationLabel = new QLabel("Correlation:", this);

  itsSettings.setAntenna1(itsAntenna1Slider->value()-1);
  itsSettings.setAntenna2(itsAntenna2Slider->value()-1);
  
  itsStartButton    = new QPushButton("Update plot", this);

  QVBoxLayout *BaselineVLayout = new QVBoxLayout(this);
  BaselineVLayout->addWidget(itsAntenna1Label);
  BaselineVLayout->addWidget(itsAntenna1Slider);
  BaselineVLayout->addWidget(itsAntenna2Label);
  BaselineVLayout->addWidget(itsAntenna2Slider);
  BaselineVLayout->addWidget(itsCorrelationLabel);
  BaselineVLayout->addWidget(itsCorrelationCombo);
  BaselineVLayout->addWidget(itsStartButton);
  BaselineVLayout->addStretch();
  BaselineVLayout->activate();

  itsCorrelationCombo->insertItem("Not selected", 0);
  itsCorrelationCombo->insertItem("XX", 1);
  itsCorrelationCombo->insertItem("XY", 2);
  itsCorrelationCombo->insertItem("YX", 3);
  itsCorrelationCombo->insertItem("YY", 4);
  itsCorrelationCombo->insertItem("RR", 5);
  itsCorrelationCombo->insertItem("RL", 6);
  itsCorrelationCombo->insertItem("LR", 7);
  itsCorrelationCombo->insertItem("LL", 8);

  QObject::connect(itsAntenna1Slider, SIGNAL(valueChanged(int)),
                   this, SLOT(slot_antenna1Changed(int)) );

  QObject::connect(itsAntenna2Slider, SIGNAL(valueChanged(int)),
                   this, SLOT(slot_antenna2Changed(int)) );

  QObject::connect(itsStartButton, SIGNAL(clicked()),
                   this, SIGNAL(signalStartButtonClicked()));

  QObject::connect(itsCorrelationCombo, SIGNAL(activated(int)),
                   this, SLOT(slot_correlationChanged(int)) );

  slot_antenna1Changed(itsAntenna1Slider->value());
  slot_antenna2Changed(itsAntenna2Slider->value());
}







//=================>>>  UVPGraphSettingsWidget::~UVPGraphSettingsWidget  <<<=================

UVPGraphSettingsWidget::~UVPGraphSettingsWidget()
{
}





//=================>>>  UVPGraphSettingsWidget::setNumberOfBaselines  <<<=================

void UVPGraphSettingsWidget::setNumberOfBaselines(unsigned int numberOfBaselines)
{
  itsNumberOfBaselines = numberOfBaselines;
}






//===============>>>  UVPGraphSettingsWidget::slot_antenna1Changed  <<<===============

void UVPGraphSettingsWidget::slot_antenna1Changed(int antenna1)
{
  std::ostringstream out;
  out << "Antenna 1: " << antenna1;
  itsAntenna1Label->setText(out.str().c_str());

  itsSettings.setAntenna1(antenna1-1);
  emit signalAntenna1Changed(antenna1-1);
}








//===============>>>  UVPGraphSettingsWidget::slot_antenna2Changed  <<<===============

void UVPGraphSettingsWidget::slot_antenna2Changed(int antenna2)
{
  std::ostringstream out;
  out << "Antenna 2: " << antenna2;
  itsAntenna2Label->setText(out.str().c_str());

  itsSettings.setAntenna2(antenna2-1);
  emit signalAntenna2Changed(antenna2-1);
}





//============>>>  UVPGraphSettingsWidget::slot_correlationChanged  <<<============

void UVPGraphSettingsWidget::slot_correlationChanged(int corr)
{
  itsSettings.setCorrelation(UVPDataAtomHeader::Correlation(corr));
  emit signalCorrelationChanged(UVPDataAtomHeader::Correlation(corr));
}





//===============>>>  UVPGraphSettingsWidget::getSettings  <<<===============

const UVPGraphSettings &UVPGraphSettingsWidget::getSettings() const
{
  return itsSettings;
}
