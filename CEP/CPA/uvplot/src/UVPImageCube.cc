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
#include <uvplot/UVPImageCube.h>


#if(DEBUG_MODE)
#include <cassert>
#endif





#ifdef KRCS_USE_VECTOR
//====================>>>  UVPImageCube::UVPImageCube  <<<====================

UVPImageCube::UVPImageCube(unsigned int nx,
                           unsigned int ny,
                           unsigned int nz)
  : itsCube(nx, 
            std::vector<std::vector<UVPPixel> >(ny,
                                                std::vector<UVPPixel>(nz)
                                                )
            ),
    itsAxis(3)
{
}
#endif



//====================>>>  UVPImageCube::UVPImageCube  <<<====================

UVPImageCube::UVPImageCube(unsigned int nx,
                           unsigned int ny,
                           unsigned int nz)
  : itsCube(0),
    itsNx(nx),
    itsNy(ny),
    itsNz(nz),
    itsAxis(3)
{
  itsCube = new UVPPixel**[nx];
  for(unsigned int x = 0; x < nx; x++) {
    itsCube[x] = new UVPPixel*[ny];
    
    for(unsigned int y = 0; y < ny; y++) {
      itsCube[x][y] = new UVPPixel[nz];
    }
  }
}





//====================>>>  UVPImageCube::UVPImageCube  <<<====================

UVPImageCube::~UVPImageCube()  
{
  for(unsigned int x = 0; x < itsNx; x++) {
    for(unsigned int y = 0; y < itsNy; y++) {
      delete[] itsCube[x][y];
    }
    delete[] itsCube[x];
  }
  delete[] itsCube;
}




#if NOT_INLINE_GETPIXELasdaf
//====================>>>  UVPImageCube::getPixel  <<<====================

UVPPixel *UVPImageCube::getPixel(unsigned int x,
                                 unsigned int y,
                                 unsigned int z)
{
#if(DEBUG_MODE)
  assert( x >= 0 && x < itsNx);
  assert( y >= 0 && y < itsNy);
  assert( z >= 0 && z < itsNz);
#endif

  return &(itsCube[x][y][z]);
}






//====================>>>  UVPImageCube::getPixel  <<<====================

const UVPPixel *UVPImageCube::getPixel(unsigned int x,
                                       unsigned int y,
                                       unsigned int z) const
{
#if(DEBUG_MODE)
  assert( x >= 0 && x < itsNx);
  assert( y >= 0 && y < itsNy);
  assert( z >= 0 && z < itsNz);
#endif

  return &(itsCube[x][y][z]);
}
#endif





//====================>>>  UVPImageCube::getN  <<<====================

unsigned int UVPImageCube::getN(Coordinate coordinate) const
{
#if(DEBUG_MODE)
  assert(coordinate == X ||
         coordinate == Y ||
         coordinate == Z);
#endif

  unsigned int n(0);

  switch(coordinate) {
  case X:
    {
      n = itsNx;
    }
    break;

  case Y:
    {
      n = itsNy;
    }
    break;

  case Z:
    {
      n = itsNz;
    }
    break;

  default:
    {
      n = 0;                    // wrong coordinate
    }
  }

  return n;
}






//=================>>>  UVPImageCube::setTransferFunction  <<<================

void UVPImageCube::setTransferFunction(Coordinate coordinate,
                                       double     scale,
                                       double     offset)
{
#if(DEBUG_MODE)
  assert(coordinate == X ||
         coordinate == Y ||
         coordinate == Z);
#endif

  itsAxis[coordinate].setTransferFunction(scale, offset);
}




//====================>>>  UVPImageCube::setAxis  <<<====================

void UVPImageCube::setAxis(Coordinate     coordinate,
                           const UVPAxis& axis)
{
#if(DEBUG_MODE)
  assert(coordinate == X ||
         coordinate == Y ||
         coordinate == Z);
#endif

  itsAxis[coordinate] = axis;
}





//====================>>>  UVPImageCube::getAxis  <<<====================

UVPAxis UVPImageCube::getAxis(Coordinate coordinate) const
{
#if(DEBUG_MODE)
  assert(coordinate == X ||
         coordinate == Y ||
         coordinate == Z);
#endif

  return itsAxis[coordinate];
}
