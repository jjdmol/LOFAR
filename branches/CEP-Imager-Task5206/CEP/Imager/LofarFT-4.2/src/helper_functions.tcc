//kate: syntax C++

#include <casa/BasicSL/Constants.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <images/Images/PagedImage.h>

namespace LOFAR {
namespace LofarFT {
  
inline casa::Double spheroidal(casa::Double nu);

// Apply a spheroidal taper to the input function.
template <typename T>
void taper (casa::Matrix<T> &function)
{
  ASSERT(function.shape()[0] == function.shape()[1]);
  casa::uInt size = function.shape()[0];
  casa::Double halfSize = (size-1) / 2.0;
  casa::Vector<casa::Double> x(size);
  for (casa::uInt i=0; i<size; ++i) {
    x[i] = spheroidal(abs(i - halfSize) / halfSize);
  }
  for (casa::uInt i=0; i<size; ++i) {
    for (casa::uInt j=0; j<size; ++j) {
      function(j, i) *= x[i] * x[j];
    }
  }
}

template <typename T>
casa::uInt findSupport(casa::Matrix<T> &function, casa::Double threshold)
{
  ///      Double peak = abs(max(abs(function)));
  casa::Double peak = max(amplitude(function));
  threshold *= peak;
  casa::uInt halfSize = function.shape()[0] / 2;
  casa::uInt x = 0;
  while (x < halfSize && abs(function(x, halfSize)) < threshold) {
    ++x;
  }
  return 2 * (halfSize - x);
}

//=================================================

casa::Double spheroidal(casa::Double nu)
{
  static casa::Double P[2][5] = {{ 8.203343e-2, -3.644705e-1, 6.278660e-1,
                            -5.335581e-1,  2.312756e-1},
                            { 4.028559e-3, -3.697768e-2, 1.021332e-1,
                            -1.201436e-1, 6.412774e-2}};
  static casa::Double Q[2][3] = {{1.0000000e0, 8.212018e-1, 2.078043e-1},
                            {1.0000000e0, 9.599102e-1, 2.918724e-1}};
  casa::uInt part = 0;
  casa::Double end = 0.0;
  if (nu >= 0.0 && nu < 0.75) 
  {
    part = 0;
    end = 0.75;
  } 
  else if (nu >= 0.75 && nu <= 1.00) 
  {
    part = 1;
    end = 1.00;
  }
  else 
  {
    return 0.0;
  }
  casa::Double nusq = nu * nu;
  casa::Double delnusq = nusq - end * end;
  casa::Double delnusqPow = delnusq;
  casa::Double top = P[part][0];
  for (casa::uInt k=1; k<5; ++k) 
  {
    top += P[part][k] * delnusqPow;
    delnusqPow *= delnusq;
  }

  casa::Double bot = Q[part][0];
  delnusqPow = delnusq;
  for (casa::uInt k=1; k<3; ++k) 
  {
    bot += Q[part][k] * delnusqPow;
    delnusqPow *= delnusq;
  }
      
  double result = (bot == 0  ?  0 : (1.0 - nusq) * (top / bot));
  //if(result<1.e-3){result=1.e-3;}
  return result;
}

  //# =================================================
  template <class T>
  void store (const casa::DirectionCoordinate &dir, const casa::Matrix<T> &data,
              const string &name);
  
  template <class T>
  void store(const casa::Matrix<T> &data, const string &name)
  {
    casa::Matrix<casa::Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    casa::Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> refLatLon(45.0 * casa::C::pi / 180.0, "rad");
    casa::DirectionCoordinate dir(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store (const casa::DirectionCoordinate &dir, const casa::Matrix<T> &data,
              const string &name)
  {
    //cout<<"Saving... "<<name<<endl;
    casa::Vector<casa::Int> stokes(1);
    stokes(0) = casa::Stokes::I;
    casa::CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(casa::StokesCoordinate(stokes));
    csys.addCoordinate(casa::SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    casa::PagedImage<T> im(casa::TiledShape(casa::IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, casa::IPosition(4, 0, 0, 0, 0));
  }

  template <class T>
  void store(const casa::Cube<T> &data, const string &name)
  {
    casa::Matrix<casa::Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    casa::Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> refLatLon(45.0 * casa::C::pi / 180.0, "rad");
    casa::DirectionCoordinate dir(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const casa::DirectionCoordinate &dir, const casa::Cube<T> &data,
             const string &name)
  {
//     AlwaysAssert(data.shape()(2) == 4, SynthesisError);
    //cout<<"Saving... "<<name<<endl;
    casa::Vector<casa::Int> stokes(4);
    stokes(0) = casa::Stokes::XX;
    stokes(1) = casa::Stokes::XY;
    stokes(2) = casa::Stokes::YX;
    stokes(3) = casa::Stokes::YY;
    casa::CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(casa::StokesCoordinate(stokes));
    csys.addCoordinate(casa::SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    casa::PagedImage<T>
      im(casa::TiledShape(casa::IPosition(4, data.shape()(0), data.shape()(1), 4, 1)),
         csys, name);
    im.putSlice(data, casa::IPosition(4, 0, 0, 0, 0));
  }
  
} // end namespace LofarFT
} // end namespace LOFAR