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

// $ID$

// Includes
#include <uvplot/UVPAxis.h>

#if(DEBUG_MODE)
#include <cassert>
#endif



//====================>>>  UVPAxis::UVPAxis  <<<====================

UVPAxis::UVPAxis(double             scale,
                 double             offset,
                 const std::string& type,
                 const std::string& unit)
  : itsScale(scale),
    itsInverseScale(1.0/scale),
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

  itsScale        = scale;
  itsInverseScale = 1.0/scale;
  itsOffset       = offset;
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

  itsScale        = (worldMax-worldMin)/(axisMax-axisMin);
  itsInverseScale = 1.0/itsScale;
  itsOffset       = worldMax - itsScale*axisMax;
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
