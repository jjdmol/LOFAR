//# ModelImageFFT.h: 
//#
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: ModelImageFFT.cc 20029 2012-04-19 15:50:23Z duscha $

#include <lofar_config.h>

#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/VectorIter.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <coordinates/Coordinates/CoordinateSystem.h> //for spectral coord
#include <coordinates/Coordinates/Coordinate.h>
#include <images/Images/PagedImage.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelImageFFT.h>
#include <BBSKernel/ModelImageVisibilityResampler.h>

using namespace std;
using namespace LOFAR;
using namespace BBS;
using namespace casa;

//*********************************************
//
// Constructors and destructor
//
//*********************************************
/*
ModelImageFft::ModelImageFft( const casa::String &name,
                              const casa::Vector<casa::Double> &frequencies,
                              unsigned int oversampling,
                              double uvscaleX, double uvscaleY)
*/                            
ModelImageFft::ModelImageFft( const casa::String &name,
                              unsigned int oversampling,
                              double uvscaleX, double uvscaleY)
{
  // set options
  itsOptions.name=name;
  itsOptions.oversampling=oversampling;
  itsOptions.uvScale[0]=uvscaleX;
  itsOptions.uvScale[1]=uvscaleY;

  PagedImage<DComplex> *image=new PagedImage<DComplex>(name);   // Open image as paged image

  // Get image properties
  // TODO
  LatticeFFT::cfft2d(*image);     // FFT image in place 
  itsImage=image->get();          // store in array attribute

// OLD casarest stuff
//  initPolmap();                   // initialize polMap_p
//  initChanmap();                  // initialize chanMap_p
//  itsVisResampler.setMaps(chanMap_p, polMap_p);   // set them in VisResampler
//  itsVisResampler.setParams(itsOptions.uvScale, itsOptions.offset, dphase_p); 
}

ModelImageFft::~ModelImageFft(void)
{
  // don't have to do anything
}

//**********************************************
//
// Setter functions for attributes
//
//**********************************************

void ModelImageFft::setConvType(const casa::String type)
{
  if(type=="SF" || type=="BOX")
  {
    itsOptions.ConvType=type;
  }
  else
  {
    THROW(BBSKernelException, "Convolution type " << type << " is unknown.");
  }
}

void ModelImageFft::setVerbose(casa::uInt verbose)
{
  itsOptions.verbose=verbose;
}

void ModelImageFft::setUVScale(const Vector<Double> &uvscale)
{
  if(uvscale[0] < 0 || uvscale[1] < 0)
  {
    THROW(BBSKernelException, "uvscale values must be > 0");    
  }
  else
  {
    itsOptions.uvScale.resize(2);
    itsOptions.uvScale[0]=uvscale[0];
    itsOptions.uvScale[1]=uvscale[1];
  }
}

void ModelImageFft::setUVScale(double uvscaleX, double uvscaleY)
{
  if(uvscaleX < 0 || uvscaleY < 0)
  {
    THROW(BBSKernelException, "uvscale values must be > 0");    
  }
  else
  {
    itsOptions.uvScale.resize(2);
    itsOptions.uvScale[0]=uvscaleX;
    itsOptions.uvScale[1]=uvscaleY;
  }
}

void ModelImageFft::setOversampling(unsigned int oversampling)
{
  itsOptions.oversampling=oversampling;
}

void ModelImageFft::setDegridMuellerMask(const casa::Matrix<Bool> &muellerMask)
{
  if(muellerMask.nrow() != 4 && muellerMask.ncolumn() != 4)
  {
    setDegridMuellerMask(muellerMask);
  }
  else
  {
    setDegridMuellerMask(muellerMask);
  }
}

//**********************************************
//
// Image property functions
//
//**********************************************

void ModelImageFft::getImageProperties(PagedImage<DComplex> *image)
{
  CoordinateSystem coordSys=image->coordinates();
  uInt SpectralCoordInd=coordSys.findCoordinate(Coordinate::SPECTRAL);
  uInt StokesCoordInd=coordSys.findCoordinate(Coordinate::STOKES);
  
//  IPosition shape=image->shape();
//  nchan=shape(SpectralCoordInd);
//  npol=shape(StokesCoordInd);
  spectralCoord_p=image->coordinates().spectralCoordinate(0);  
}


//**********************************************
//
// Degridding functions
//
//**********************************************

void ModelImageFft::degrid( const double *uvwBaseline, 
                            size_t timeslots, size_t nchans, 
                            const double *frequencies,
                            casa::DComplex *XX , casa::DComplex *XY, 
                            casa::DComplex *YX , casa::DComplex *YY)
{
  Vector<Double> lamdbdas(nchans);        // vector with wavelengths
  for(unsigned int i=0; i<nchans; i++)
  {
    itsOptions.frequencies[i]=frequencies[i];
  }
  
  itsOptions.lambdas=convertToLambdas(itsOptions.frequencies);  // convert to lambdas

  // create VisResampler

  // set up chanMap

  // convert uvwBaseline to VisResampler format
 
  // call VisResampler
}

void ModelImageFft::degrid( const boost::multi_array<double, 3> &uvwBaseline, 
                            size_t timeslots, size_t nchans, 
                            const double *frequencies,
                            casa::DComplex *XX , casa::DComplex *XY, 
                            casa::DComplex *YX , casa::DComplex *YY)
{
  Vector<Double> lamdbdas(nchans);        // vector with wavelengths
  for(unsigned int i=0; i<nchans; i++)
  {
    itsOptions.frequencies[i]=frequencies[i];
  }
  
  itsOptions.lambdas=convertToLambdas(itsOptions.frequencies);  // convert to lambdas

  // create VisResampler

  // set up chanMap

  // convert uvwBaseline to VisResampler format
 
  // call VisResampler
}

//**********************************************
//
// Helper functions
//
//**********************************************

// Convert a Vector of frequencies to lambdas
Vector<Double> ModelImageFft::convertToLambdas(const Vector<Double> &frequencies)
{
  Vector<Double> lambdas(frequencies.size());
  for(uInt i=0; i<frequencies.size(); i++)
  {
    lambdas[i]=(casa::C::c)/frequencies[i];
  }
  return lambdas;
}


/* OLD casarest stuff
void ModelImageFft::initPolmap()
{
  polMap_p.resize(4);
  polMap_p[0]=1;
  polMap_p[1]=1;
  polMap_p[2]=1;
  polMap_p[3]=1;
}

//void ModelImageFft::initChanmap(const Vector<Double> &frequencies)
void ModelImageFft::initChanmap()
{
  chanMap_p.resize(itsOptions.frequencies.size());
  chanMap_p.set(-1);    // reset chanMap to -1 for all channels

  // Find nearest Image frequency for frequencies
  Vector<Double> c(1);    // channel frequency
  c=0.0;
  Vector<Double> f(1);    // vector of frequencies neede for spectral pixel conversion
  Int nFound=0;           // number of found channels?
  unsigned int nvischan=itsOptions.frequencies.size();

  // Check if with have only an one channel image
  if(nchan==1)
  {
    chanMap_p.set(0);   // set all channels to pixel 0
  }
  else
  {
    // otherwise match channels
    for(uInt chan=0;chan<nvischan;chan++)
    {
    //  f(0)=lsrFreq[chan];
      if(spectralCoord_p.toPixel(c, f))
      {
        Int pixel=uInt(floor(c(0)+0.5));  // round to chan freq at chan center 
        //cout << "spw " << spw << " f " << f(0) << " pixel "<< c(0) << "  " << pixel << endl;
        /////////////
        //c(0)=pixel;
        //spectralCoord_p.toWorld(f, c);
        // cout << "f1 " << f(0) << " pixel "<< c(0) << "  " << pixel << endl;
        ////////////////
        if(pixel>-1 && pixel<nchan)
        {
          chanMap_p(chan)=pixel;
          nFound++;
          if(nvischan>1&&(chan==0||chan==nvischan-1))
          {
            LOG_DEBUG_STR("Selected visibility channel : " << chan+1
                          << " has frequency " 
                          <<  MFrequency(Quantity(f(0), "Hz")).get("GHz").getValue()
                          << " GHz and maps to image pixel " << pixel+1);
          }
        }
      }
    }
  }
}
*/