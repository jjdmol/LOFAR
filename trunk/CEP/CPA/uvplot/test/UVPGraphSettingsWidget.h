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

#if !defined(UVPGRAPHSETTINGSWIDGET_H)
#define UVPGRAPHSETTINGSWIDGET_H

// $Id$

#include <uvplot/UVPGraphSettings.h>

#include <qwidget.h>
class QLineEdit;
class QLabel;
class QSlider;
class QPushButton;
class QComboBox;
class QVBoxLayout;
class QHBoxLayout;

/*
#include <qlineedit.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qcombobox.h>
*/


class UVPGraphSettingsWidget: public QWidget
{
  Q_OBJECT

public:

  UVPGraphSettingsWidget(unsigned int numOfAntennae = 30,
                         QWidget *    parent = 0,
                         const char * name = 0,
                         WFlags       f = 0);
    
  ~UVPGraphSettingsWidget();
  

  void setNumberOfAntennae(unsigned int numberOfAntennae);

  void setNumberOfFields(unsigned int numberOfFields);

  const UVPGraphSettings &getSettings() const;

public slots:

  /*! One based antenna indices
   */
  void slot_antenna1Changed(int antenna1);
  void slot_antenna2Changed(int antenna2);
  void slot_correlationChanged(int corr);
  void slot_columnChanged     (int column);
  void slot_fieldChanged      ();
  void slot_spectralWindowChanged();


signals:

  //! antenna1 is zero based
  void signalAntenna1Changed(unsigned int antenna1);

  //! antenna2 is zero based
  void signalAntenna2Changed(unsigned int antenna2);

  void signalCorrelationChanged(UVPDataAtomHeader::Correlation corr);
  
  //! MS column changed
  void signalColumnChanged(const std::string& columnName);
  
  void signalLoadButtonClicked();

  void signalFieldsChanged();

  
protected:
private:
  unsigned int     itsNumberOfAntennae;

  QVBoxLayout*     itsVLayout;
  QHBoxLayout*     itsFieldLayout;

  QLabel*          itsFieldLabel;
  std::vector<QCheckBox*> itsFieldSelections;
 

  QSlider*         itsAntenna1Slider;
  QLabel*          itsAntenna1Label;

  QPushButton*     itsLoadButton;


  QSlider*         itsAntenna2Slider;
  QLabel*          itsAntenna2Label;

  QLabel*          itsColumnLabel;
  QComboBox*       itsColumnCombo;

  QComboBox*       itsCorrelationCombo;
  QLabel*          itsCorrelationLabel;

  UVPGraphSettings itsSettings;
};


#endif // UVPGRAPHSETTINGSWIDGET_H
