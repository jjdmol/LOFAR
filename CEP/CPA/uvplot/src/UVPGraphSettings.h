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

#if !defined(UVPGRAPHSETTING_H)
#define UVPGRAPHSETTINGS_H

// $Id$

#include <vector>
#include <string>
#include <uvplot/UVPDataAtomHeader.h>


//! 
class UVPGraphSettings
{
public:
  
  enum ValueType {eReal, eImag, eAbs, ePhase};
  enum PlotType {e2D, e3D};


  UVPGraphSettings(unsigned int       antenna1           = 0,
                   unsigned int       antenna2           = 0,
                   UVPDataAtomHeader::Correlation  corr  = UVPDataAtomHeader::None,
                   const std::string& columnName         = "",
                   ValueType          valueType          = eAbs,
                   PlotType           plotType           = e2D,
                   const std::vector<bool>& fieldsToPlot = std::vector<bool>(0));

  unsigned int                    getAntenna1() const;
  unsigned int                    getAntenna2() const;
  UVPDataAtomHeader::Correlation  getCorrelation() const;
  std::string                     getColumnName() const;
  ValueType                       getValueType() const;
  PlotType                        getPlotType() const;
  bool                            mustPlotField(unsigned int fieldIndex) const;


  void         setAntenna1(unsigned int antenna1);
  void         setAntenna2(unsigned int antenna2);
  void         setCorrelation(UVPDataAtomHeader::Correlation corr);
  void         setColumnName(const std::string& columnName);
  void         setValueType(ValueType valueType);
  void         setPlotType(PlotType plotType);
  void         setPlotField(unsigned int fieldIndex,
                            bool         mustPlotField);

  unsigned int getNumberOfFields() const;
  //  unsigned int getNumberOfBaselines() const;
  //  unsigned int getNumberOfPolarizations() const;

  void         setNumberOfFields(unsigned int numberOfFields);
  //  void         setNumberOfBaselines(unsigned int numberOfBaselines);
  //  void         setNumberOfPolarizations(unsigned int numberOfPolarizations);
  
protected:
private:
  
  unsigned int                   itsAntenna1;
  unsigned int                   itsAntenna2;
  UVPDataAtomHeader::Correlation itsCorrelation;
  std::string                    itsColumnName;
  ValueType                      itsValueType;
  PlotType                       itsPlotType;
  std::vector<bool>              itsPlotFields;
};

#endif //UVPGRAPHSETTINGS_H
