// Copyright notice should go here

#if !defined(UVPGRAPHSETTINGSWIDGET_H)
#define UVPGRAPHSETTINGSWIDGET_H

// $Id$

#include <UVPGraphSettings.h>

#include <qlineedit.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>


class UVPGraphSettingsWidget: public QWidget
{
  Q_OBJECT

public:

  UVPGraphSettingsWidget(unsigned int numOfAntennae = 30,
                         QWidget *    parent = 0,
                         const char * name = 0,
                         WFlags       f = 0);
    
  ~UVPGraphSettingsWidget();
  

  void setNumberOfBaselines(unsigned int numberOfBaselines);

  const UVPGraphSettings &getSettings() const;

public slots:

  /*! One based antenna indices
   */
  void slot_antenna1Changed(int antenna1);
  void slot_antenna2Changed(int antenna2);


signals:

  //! antenna1 is zero based
  void signalAntenna1Changed(unsigned int antenna1);

  //! antenna2 is zero based
  void signalAntenna2Changed(unsigned int antenna2);
  void signalStartButtonClicked();
  
protected:
private:
  unsigned int     itsNumberOfBaselines;

  QSlider*         itsAntenna1Slider;
  QLabel*          itsAntenna1Label;

  QSlider*         itsAntenna2Slider;
  QLabel*          itsAntenna2Label;
  
  QPushButton*     itsStartButton;

  UVPGraphSettings itsSettings;
};


#endif // UVPGRAPHSETTINGSWIDGET_H
