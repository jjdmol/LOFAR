// Copyright notice should go here

#include <UVPGraphSettings.h>


//====================>>>  UVPGraphSettings::UVPGraphSettings  <<<====================

UVPGraphSettings::UVPGraphSettings(unsigned int baselineCode,
                                   unsigned int polarizationIndex,
                                   ValueType    valueType,
                                   PlotType     plotType,
                                   const std::vector<bool> fieldsToPlot)
  : itsBaselineCode(baselineCode),
    itsPolarizationIndex(polarizationIndex),
    itsValueType(valueType),
    itsPlotType(plotType),
    itsPlotFields(fieldsToPlot)
{
}
 





//====================>>>  UVPGraphSettings::getBaselineCode  <<<====================

unsigned int UVPGraphSettings::getBaselineCode() const
{
  return itsBaselineCode;
}




//====================>>>  UVPGraphSettings::getPolarizationIndex  <<<====================

unsigned int UVPGraphSettings::getPolarizationIndex() const
{
  return itsPolarizationIndex;
}





//====================>>>  UVPGraphSettings::getValueType  <<<====================

UVPGraphSettings::ValueType UVPGraphSettings::getValueType() const
{
  return itsValueType;
}




//====================>>>  UVPGraphSettings::getPlotType  <<<====================

UVPGraphSettings::PlotType UVPGraphSettings::getPlotType() const
{
  return itsPlotType;
}




//====================>>>  UVPGraphSettings::mustPlotField  <<<====================

bool UVPGraphSettings::mustPlotField(unsigned int fieldIndex) const
{
  return itsPlotFields[fieldIndex];
}


//====================>>>  UVPGraphSettings::setBaselineCode  <<<====================

void UVPGraphSettings::setBaselineCode(unsigned int baselineCode)
{
  itsBaselineCode = baselineCode;
}




//====================>>>  UVPGraphSettings::setPolarizationIndex  <<<====================

void UVPGraphSettings::setPolarizationIndex(unsigned int polarizationIndex)
{
  itsPolarizationIndex = polarizationIndex;
}




//====================>>>  UVPGraphSettings::setValueType  <<<====================

void UVPGraphSettings::setValueType(ValueType valueType)
{
  itsValueType = valueType;
}




//====================>>>  UVPGraphSettings::setPlotType  <<<====================

void UVPGraphSettings::setPlotType(PlotType plotType)
{
  itsPlotType = plotType;
}




//====================>>>  UVPGraphSettings::setPlotField  <<<====================

void UVPGraphSettings::setPlotField(unsigned int fieldIndex,
                                    bool         mustPlotField)
{
  itsPlotFields[fieldIndex] = mustPlotField;
}


//====================>>>  UVPGraphSettings::getNumberOfFields  <<<====================

unsigned int UVPGraphSettings::getNumberOfFields() const
{
  return itsPlotFields.size();
}




//====================>>>  UVPGraphSettings::setNumberOfFields

void UVPGraphSettings::setNumberOfFields(unsigned int numberOfFields)
{
  itsPlotFields.resize(numberOfFields);
}
