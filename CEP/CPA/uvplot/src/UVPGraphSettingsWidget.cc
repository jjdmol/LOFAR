// Copyright notice should go here



#include <UVPGraphSettingsWidget.h>

#include <qlayout.h>



//=================>>>  UVPGraphSettingsWidget::UVPGraphSettingsWidget  <<<=================

UVPGraphSettingsWidget::UVPGraphSettingsWidget(QWidget *    parent,
                                               const char * name,
                                               WFlags       f)
  : QWidget(parent, name, f),
    itsNumberOfBaselines(0)
{
  itsBaselineLabel = new QLabel("Baseline index:", this);
  itsBaselineEdit  = new QLineEdit(this);

  QVBoxLayout *BaselineVLayout = new QVBoxLayout(this);
  BaselineVLayout->addWidget(itsBaselineLabel);
  BaselineVLayout->addWidget(itsBaselineEdit);
  BaselineVLayout->addStretch();
  BaselineVLayout->activate();
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
