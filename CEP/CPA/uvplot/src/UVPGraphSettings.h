// Copyright notice should go here

#if !defined(UVPGRAPHSETTING_H)
#define UVPGRAPHSETTINGS_H

// $Id$

#include <vector>

//! 
class UVPGraphSettings
{
public:
  
  enum ValueType {eReal, eImag, eAbs, ePhase};
  enum PlotType {e2D, e3D};


  UVPGraphSettings(unsigned int antenna1                = 0,
                   unsigned int antenna2                = 0,
                   unsigned int polarizationIndex       = 0,
                   ValueType    valueType               = eAbs,
                   PlotType     plotType                = e2D,
                   const std::vector<bool> fieldsToPlot = std::vector<bool>(0));

  unsigned int getAntenna1() const;
  unsigned int getAntenna2() const;
  unsigned int getPolarizationIndex() const;
  ValueType    getValueType() const;
  PlotType     getPlotType() const;
  bool         mustPlotField(unsigned int fieldIndex) const;


  void         setAntenna1(unsigned int antenna1);
  void         setAntenna2(unsigned int antenna2);
  void         setPolarizationIndex(unsigned int polarizationIndex);
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
  
  unsigned int      itsAntenna1;
  unsigned int      itsAntenna2;
  unsigned int      itsPolarizationIndex;
  ValueType         itsValueType;
  PlotType          itsPlotType;
  std::vector<bool> itsPlotFields;
};

#endif //UVPGRAPHSETTINGS_H
