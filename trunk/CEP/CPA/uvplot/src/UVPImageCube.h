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

#if !defined(UVPIMAGECUBE_H)
#define UVPIMAGECUBE_H

// $Id$

// Includes
#include <uvplot/UVPPixel.h>
#include <uvplot/UVPAxis.h>


#include <vector>
#include <string>


// Forward declarations




class UVPImageCube
{
 public:

  // Simple labelling of the three axes.
  enum Coordinate {
    X = 0,
    Y = 1,
    Z = 2
  };

  
  // Sets default transfer functions scale = 1.0, offset = 0. nx, ny
  // and nz MUST be >= 1
               UVPImageCube(unsigned int nx=1,
                            unsigned int ny=1,
                            unsigned int nz=1);
               

               ~UVPImageCube();


  // Returns a pointer to a specific UVPPixel object. One may use this
  // pointer to modify or query the pixel. The indices x, y and z are
  // zero-relative.
  inline UVPPixel*    getPixel(unsigned int x=0, 
                               unsigned int y=0, 
                               unsigned int z=0);
  
  inline const UVPPixel* getPixel(unsigned int x=0, 
                                  unsigned int y=0, 
                                  unsigned int z=0) const;
  

  // getN returns the number of pixels along the axis specified by
  // coordinate. coordinate may be X, Y or Z. It returns 0 on error.
  unsigned int getN(Coordinate coordinate) const;

  
  void         setTransferFunction(Coordinate coordinate,
                                   double     scale,
                                   double     offset);

  void         setAxis(Coordinate     coordinate,
                       const UVPAxis& axis);

  UVPAxis      getAxis(Coordinate     coordinate) const;


 private:
  
  // Addressing: itsCube[x][y][z]...
  //  std::vector<std::vector<std::vector<UVPPixel> > > itsCube;
  UVPPixel***   itsCube;
  unsigned int  itsNx;
  unsigned int  itsNy;
  unsigned int  itsNz;
  
  // The three coordinate axes.
  std::vector<UVPAxis>                              itsAxis;
};





//====================>>>  UVPImageCube::getPixel  <<<====================

inline UVPPixel *UVPImageCube::getPixel(unsigned int x,
                                        unsigned int y,
                                        unsigned int z)
{
#if(DEBUG_MODE)
  assert(x < itsNx);
  assert(y < itsNy);
  assert(z < itsNz);
#endif

  return &(itsCube[x][y][z]);
}


//====================>>>  UVPImageCube::getPixel  <<<====================

inline const UVPPixel *UVPImageCube::getPixel(unsigned int x,
                                              unsigned int y,
                                              unsigned int z) const
{
#if(DEBUG_MODE)
  assert(x < itsNx);
  assert(y < itsNy);
  assert(z < itsNz);
#endif

  return &(itsCube[x][y][z]);
}

#endif // UVPIMAGECUBE_H
