// Copyright notice should go here

#if !defined(UVPIMAGECUBE_H)
#define UVPIMAGECUBE_H

// $ID$

// Includes
#include <UVPPixel.h>
#include <UVPAxis.h>


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
