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

#include <uvplot/UVPGraphSettings.h>


//====================>>>  UVPGraphSettings::UVPGraphSettings  <<<====================

UVPGraphSettings::UVPGraphSettings(unsigned int                   antenna1,
                                   unsigned int                   antenna2,
                                   UVPDataAtomHeader::Correlation corr,
                                   const std::string&             columnName,
                                   ValueType                      valueType,
                                   PlotType                       plotType,
                                   const std::vector<bool>&       fieldsToPlot)
  : itsAntenna1(antenna1),
    itsAntenna2(antenna2),
    itsCorrelation(corr),
    itsColumnName(columnName),
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




//===============>>>  UVPGraphSettings::getCorrelation  <<<===============

UVPDataAtomHeader::Correlation  UVPGraphSettings::getCorrelation() const
{
  return itsCorrelation;
}





//====================>>>  UVPGraphSettings::getValueType  <<<====================

UVPGraphSettings::ValueType UVPGraphSettings::getValueType() const
{
  return itsValueType;
}




//====================>>>  UVPGraphSettings::getColumnName  <<<====================

std::string UVPGraphSettings::getColumnName() const
{
  return itsColumnName;
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




//=================>>>  UVPGraphSettings::setCorrelation  <<<=================

void UVPGraphSettings::setCorrelation(UVPDataAtomHeader::Correlation corr)
{
  itsCorrelation = corr;
}




//=================>>>  UVPGraphSettings::setColumnName  <<<=================

void UVPGraphSettings::setColumnName(const std::string& columnName)
{
  itsColumnName = columnName;
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
