// Copyright notice should go here

#include <uvplot/UVPGraphSettings.h>


//====================>>>  UVPGraphSettings::UVPGraphSettings  <<<====================

UVPGraphSettings::UVPGraphSettings(unsigned int antenna1,
                                   unsigned int antenna2,
                                   unsigned int polarizationIndex,
                                   ValueType    valueType,
                                   PlotType     plotType,
                                   const std::vector<bool> fieldsToPlot)
  : itsAntenna1(antenna1),
    itsAntenna2(antenna2),
    itsPolarizationIndex(polarizationIndex),
    itsValueType(valueType),
    itsPlotType(plotType),
    itsPlotFields(fieldsToPlot)
{
}
 





//====================>>>  UVPGraphSettings::getAntenna1  <<<====================

unsigned int UVPGraphSettings::getAntenna1() const
{
  return itsAntenna1;//(itsAntenna1 > itsAntenna2 ? itsAntenna2 : itsAntenna1);
}




//====================>>>  UVPGraphSettings::getAntenna2  <<<====================

unsigned int UVPGraphSettings::getAntenna2() const
{
  return itsAntenna2;//(itsAntenna1 > itsAntenna2 ? itsAntenna1 : itsAntenna2);
}




//===============>>>  UVPGraphSettings::getPolarizationIndex  <<<===============

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




//====================>>>  UVPGraphSettings::setAntenna1  <<<====================

void UVPGraphSettings::setAntenna1(unsigned int antenna1)
{
  itsAntenna1 = antenna1;
}




//====================>>>  UVPGraphSettings::setAntenna2  <<<====================

void UVPGraphSettings::setAntenna2(unsigned int antenna2)
{
  itsAntenna2 = antenna2;
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
