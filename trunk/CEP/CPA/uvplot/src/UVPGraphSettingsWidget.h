// Copyright notice should go here

#if !defined(UVPGRAPHSETTINGSWIDGET_H)
#define UVPGRAPHSETTINGSWIDGET_H

// $Id$

#include <UVPGraphSettings.h>

#include <qlineedit.h>
#include <qlabel.h>


class UVPGraphSettingsWidget: public QWidget
{
  Q_OBJECT

public:

  UVPGraphSettingsWidget(QWidget *    parent = 0,
                         const char * name = 0,
                         WFlags       f = 0);
    
  ~UVPGraphSettingsWidget();
  

  void setNumberOfBaselines(unsigned int numberOfBaselines);


signals:

  void signalBaselineChanged(unsigned int baseline);
  
protected:
private:
  unsigned int itsNumberOfBaselines;

  QLabel*      itsBaselineLabel;
  QLineEdit*   itsBaselineEdit;
};


#endif // UVPGRAPHSETTINGSWIDGET_H
