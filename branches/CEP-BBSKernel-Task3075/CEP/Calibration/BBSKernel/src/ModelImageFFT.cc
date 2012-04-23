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
#include <images/Images/PagedImage.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelImageFFT.h>

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
  itsOptions.uvscaleX=uvscaleX;
  itsOptions.uvscaleY=uvscaleY;

  PagedImage<DComplex> *image=new PagedImage<DComplex>(name);   // Open image as paged image
  LatticeFFT::cfft2d(*image);     // FFT image in place 
  itsImage=image->get();          // store in array attribute
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

void ModelImageFft::setUVscale(double uvscaleX, double uvscaleY)
{
  if(uvscaleX < 0 || uvscaleY < 0)
  {
    THROW(BBSKernelException, "uvscale values must be > 0");    
  }
  else
  {
    itsOptions.uvscaleX=uvscaleX;
    itsOptions.uvscaleY=uvscaleY;
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
// Degridding functions
//
//**********************************************

void ModelImageFft::degrid( const boost::multi_array<double, 3> &uvwBaseline, 
                            size_t timeslots, size_t nchans, 
                            double *frequencies,
                            casa::DComplex *XX , casa::DComplex *XY, 
                            casa::DComplex *YX , casa::DComplex *YY)
{
  Vector<Double> lamdbdas(nchans);        // vector with wavelengths
  for(unsigned int i=0; i<nchans; i++)
  {
    itsOptions.frequencies[i]=frequencies[i];
  }
  
  itsOptions.lambdas=convertToLambdas(itsOptions.frequencies);  // convert to lambdas
}

// Convert a Vector of frequencies to lambdas
Vector<Double> ModelImageFft::convertToLambdas(const Vector<Double> &frequencies)
{
  Vector<Double> lambdas(frequencies.size());
  for(uInt i=0; i<frequencies.size(); i++)
  {
    lambdas[i]=casa::C::c/frequencies[i];
  }

  return lambdas;
}