// Copyright notice should go here

// $ID$

// Includes
#include <UVPAxis.h>

#if(DEBUG_MODE)
#include <cassert>
#endif



//====================>>>  UVPAxis::UVPAxis  <<<====================

UVPAxis::UVPAxis(double             scale,
                 double             offset,
                 const std::string& type,
                 const std::string& unit)
  : itsScale(scale),
    itsOffset(offset),
    itsType(type),
    itsUnit(unit)
{
  #if(DEBUG_MODE)
  assert(scale != 0.0);
  #endif
}





//===================>>>  UVPAxis::setTransferFunction  <<<===================

void UVPAxis::setTransferFunction(double scale,
                                  double offset)
{
  #if(DEBUG_MODE)
  assert(scale != 0.0);
  #endif

  itsScale  = scale;
  itsOffset = offset;
}





//===================>>>  UVPAxis::calcTransferFunction  <<<===================

void UVPAxis::calcTransferFunction(double worldMin, 
                                   double worldMax,
                                   double axisMin,
                                   double axisMax)
{
  #if(DEBUG_MODE)
  assert(axisMin != axisMax);
  assert(worldMin != worldMax);
  #endif

  itsScale  = (worldMax-worldMin)/(axisMax-axisMin);
  itsOffset = worldMax - itsScale*axisMax;
}






//====================>>>  UVPAxis::getScale  <<<====================

double UVPAxis::getScale() const
{
  return itsScale;
}





//====================>>>  UVPAxis::getOffset  <<<====================

double UVPAxis::getOffset() const
{
  return itsOffset;
}




//====================>>>  UVPAxis::getType  <<<====================

std::string UVPAxis::getType() const
{
  return itsType;
}





//====================>>>  UVPAxis::getUnit  <<<====================

std::string UVPAxis::getUnit() const
{
  return itsUnit;
}
