// Copyright notice should go here

// $ID$

// Includes
#include <UVPImageCube.h>


#if(DEBUG_MODE)
#include <cassert>
#endif





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





//====================>>>  UVPImageCube::getPixel  <<<====================

UVPPixel *UVPImageCube::getPixel(unsigned int x,
                                 unsigned int y,
                                 unsigned int z)
{
#if(DEBUG_MODE)
  assert( x >= 0 && x < itsCube().size());
  assert( y >= 0 && y < itsCube()[0].size());
  assert( z >= 0 && z < itsCube()[0][0].size());
#endif

  return &(itsCube[x][y][z]);
}






//====================>>>  UVPImageCube::getPixel  <<<====================

const UVPPixel *UVPImageCube::getPixel(unsigned int x,
                                       unsigned int y,
                                       unsigned int z) const
{
#if(DEBUG_MODE)
  assert( x >= 0 && x < itsCube().size());
  assert( y >= 0 && y < itsCube()[0].size());
  assert( z >= 0 && z < itsCube()[0][0].size());
#endif

  return &(itsCube[x][y][z]);
}






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
      n = itsCube.size();
    }
    break;

  case Y:
    {
      n = itsCube[0].size();
    }
    break;

  case Z:
    {
      n = itsCube[0][0].size();
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
