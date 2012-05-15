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

#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <Common/SystemUtil.h>    // needed for basename
#include <Common/Exception.h>     // THROW macro for exceptions

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
                              unsigned int nwplanes,
                              unsigned int oversampling,
                              double uvscaleX, double uvscaleY)
{
  bool valid=false;

  // set options
  itsOptions.name=name;
  itsOptions.nwplanes=nwplanes;
  itsOptions.oversampling=oversampling;
  itsOptions.uvScale[0]=uvscaleX;
  itsOptions.uvScale[1]=uvscaleY;

  valid=validImage(name);
  if(!valid)
  {
    THROW(BBSKernelException, "Invalid image.");
  }

  PagedImage<DComplex> *image=new PagedImage<DComplex>(name);   // Open image as paged image
  this->getImageProperties(image);      // get image properties

  // TODO
  LatticeFFT::cfft2d(*image);     // FFT image in place 
  itsImage=image->get();          // store FFT'ed image in array attribute
  delete image;                   // don't need the intermediate image anymore


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

void ModelImageFft::setNwplanes(unsigned int nwplanes)
{
  itsOptions.nwplanes=nwplanes;
}

//**********************************************
//
// Image property functions
//
//**********************************************

void ModelImageFft::getImageProperties(PagedImage<DComplex> *image)
{
  CoordinateSystem coordSys=image->coordinates();   // get coordinate system of image

  uInt nPixelAxes=coordSys.nPixelAxes();
  if(nPixelAxes != 2)
  {
    THROW(BBSKernelException, "Model image does not have required 2 pixel axes, but "
          << nPixelAxes << " instead.");
  }
  IPosition shape=image->shape();
  Int nCoord=coordSys.nCoordinates();
  Int XCoordInd=coordSys.findCoordinate(Coordinate::DIRECTION);
  Int YCoordInd=coordSys.findCoordinate(Coordinate::DIRECTION, XCoordInd);

  if(XCoordInd != -1 && YCoordInd!=-1)
  {
    nx=shape(XCoordInd);
    ny=shape(YCoordInd);
    LOG_INFO_STR("Image " << itsOptions.name << " has dimensions nx = " << nx << " ny = " << ny << ".");
//    LOG_INFO_STR("Image has dimensions nx = " << nx << " ny = " << ny << ".");
  }
  else
  {
    THROW(BBSKernelException, "No DIRECTION coordinate in image " << itsOptions.name);
  }
  Int SpectralCoordInd=coordSys.findCoordinate(Coordinate::SPECTRAL);
  if(SpectralCoordInd != -1)
  {
    nchan=shape(SpectralCoordInd);    
    LOG_INFO_STR("Image has " << nchan << " frequency channels.");
  }
  else
  {
    nchan=1;
  }
  Int StokesCoordInd=coordSys.findCoordinate(Coordinate::STOKES);
  if(StokesCoordInd != -1)
  {
    npol=shape(StokesCoordInd);
    LOG_INFO_STR("Image has " << npol << " polarizations.");

    if(npol!=1 || npol!=4)
    {
      THROW(BBSKernelException, "Number of polarizations must be 1 or 4.");    
    }
  }
  else
  {
    npol=1;
  }

  // DEBUG
  cout << "nCoord = " << nCoord << endl;
  cout << "XCoordInd = " << XCoordInd << endl;
  cout << "YCoordInd = " << YCoordInd << endl;
  cout << "SpectralCoordInd = " << SpectralCoordInd << endl;
  cout << "StokesCoordInd = " << StokesCoordInd << endl;
  
  spectralCoord_p=image->coordinates().spectralCoordinate(0);  
}

// Check that input model image has Jy/pixel flux
//
bool ModelImageFft::validImage(const casa::String &imageName)
{
  bool valid=false;

  // Look for Jy/beam entry in file table keywords "unit"
  Table image(imageName);                                       // open image-table as read-only
  const TableRecord &imageKeywords(image.keywordSet());
  RecordFieldId unitsId("units");
  string units=imageKeywords.asString(unitsId);
  if(units=="Jy/pixel")
  {
    valid=true;
  }
  else
  {
    valid=false;
  }
  return valid;
}


//**********************************************
//
// Degridding functions
//
//**********************************************

void ModelImageFft::degrid( const double *uvwBaselines, 
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

  // write baseline uvw into vector
  vector<double> uswBaselinesVec(timeslots);
  for(unsigned int i=0; i<timeslots; i++)
  {
    uswBaselinesVec[i]=*(uvwBaselines+i);
  }

  // call Cornwell degrid
  
  // assign return arrays to vector addresses
}

void ModelImageFft::degrid( const boost::multi_array<double, 3> &uvwBaselines, 
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

  // convert uvwBaseline to Cornwell degrid format, vector
  
  // assign output array parameter to vector
 
}

void ModelImageFft::degrid( const boost::multi_array<double, 3> &uvwBaselines, 
                            const vector<double> &frequencies,
                            Vector<casa::DComplex> &XX , Vector<casa::DComplex> &XY, 
                            Vector<casa::DComplex> &YX , Vector<casa::DComplex> &YY)
{
  unsigned int nchans=frequencies.size();     // get number of channels
  Vector<Double> lamdbdas(nchans);            // vector for wavelengths conversion
  for(unsigned int i=0; i<nchans; i++)
  {
    itsOptions.frequencies[i]=frequencies[i];
  }
  
  itsOptions.lambdas=convertToLambdas(itsOptions.frequencies);  // convert to lambdas

  // convert uvwBaseline to ConvolveBlas format
 
  // call degrid
  
  // Distribute output to correlation vectors
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

//***************************************************************************
//
// Gridding and degridding functions J.Romein/T.Cornwell: tConvolveBLAS.cc
//
//***************************************************************************

/////////////////////////////////////////////////////////////////////////////////
// The next two functions are the kernel of the gridding/degridding.
// The data are presented as a vector. Offsets for the convolution function
// and for the grid location are precalculated so that the kernel does
// not need to know anything about world coordinates or the shape of
// the convolution function. The ordering of cOffset and iu, iv is
// random - some presorting might be advantageous.
//
// Perform gridding
//
// data - values to be gridded in a 1D vector
// support - Total width of convolution function=2*support+1
// C - convolution function shape: (2*support+1, 2*support+1, *)
// cOffset - offset into convolution function per data point
// iu, iv - integer locations of grid points
// grid - Output grid: shape (gSize, *)
// gSize - size of one axis of grid
//
void ModelImageFft::gridKernel(const std::vector<std::complex<float> >& data, 
    const int support,  const std::vector<std::complex<float> >& C, 
    const std::vector<unsigned int>& cOffset, const std::vector<unsigned int>& iu, 
    const std::vector<unsigned int>& iv,  std::vector<std::complex<float> >& grid, 
    const int gSize)
{
  int sSize=2*support+1;

  cout << "sSize = " << sSize << ", support = " << support << " dataSize = " << data.size() << endl;

  for (unsigned int dind=0; dind<data.size(); dind++)
  {
    // Nearly all the L2 cache misses originate here in the next
    // two statements
    // The actual grid point
    int gind=iu[dind]+gSize*iv[dind]-support;
    // The Convoluton function point from which we offset
    int cind=cOffset[dind];

//    cout << "gind: " << gind << ", cind: " << cind << endl;

    for (int suppv=0; suppv<sSize; suppv++)
    {
#ifdef USEBLAS
      cblas_caxpy(sSize, &data[dind], &C[cind], 1, &grid[gind], 1);
#else
      for (int suppu=0; suppu<sSize; suppu++)
      {
        grid[gind+suppu] += data[dind] * C[cind+suppu];
//	printf("C[%d] = (%f, %f)\n", cind + suppu, real(C[cind+suppu]), imag(C[cind+suppu]));
      }
#endif
      gind+=gSize;
      cind+=sSize;
    }
  }
/*
    for(size_t i=0; i< gSize; i++) {
	for(size_t j=0; j<gSize; j++) {
	    size_t index = i * gSize + j;
	    printf("grid[%ld][%ld] = (%f, %f)\n", i, j, real(grid[index]), imag(grid[index]));
	}
	printf("\n");
    }
*/
}

// Perform degridding
void ModelImageFft::degridKernel(const std::vector<std::complex<float> >& grid, 
                                const int gSize, const int support,
                                const std::vector<std::complex<float> >& C,
                                const std::vector<unsigned int>& cOffset,
                                const std::vector<unsigned int>& iu, 
                                const std::vector<unsigned int>& iv,
                                std::vector<std::complex<float> >& data)
{
  int sSize=2*support+1;

  for (unsigned int dind=0; dind<data.size(); dind++)
  {
   data[dind]=0.0;

    // Nearly all the L2 cache misses originate here in the next
    // two statements
    // The actual grid point from which we offset
    int gind=iu[dind]+gSize*iv[dind]-support;
    // The Convoluton function point from which we offset
    int cind=cOffset[dind];
    
    for (int suppv=0; suppv<sSize; suppv++)
    {
#ifdef USEBLAS
      std::complex<float>  dot;
      cblas_cdotu_sub(sSize, &grid[gind], 1, &C[cind], 1, &dot);
      data[dind]+=dot;
#else
      for (int suppu=0; suppu<sSize; suppu++)
      {
        data[dind]+=grid[gind+suppu]*C[cind+suppu];
      }
#endif
      gind+=gSize;
      cind+=sSize;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////

// Initialize W project convolution function 
// - This is application specific and should not need any changes.
//
// nSamples - number of visibility samples
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// gSize - size of grid in pixels (per axis)
// support - Total width of convolution function=2*support+1
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w
void ModelImageFft::initC(const int nSamples, const std::vector<double>& w,
    const std::vector<double>& freq, const double cellSize, 
    const double baseline,
    const int wSize, const int gSize, int& support, int& overSample,
    double& wCellSize, std::vector<std::complex<float> >& C)
{
  cout << "Initializing W projection convolution function" << endl;
  support=static_cast<int>(1.5*sqrt(abs(baseline) *static_cast<double>(cellSize)
      *freq[0])/cellSize);
  overSample=8;
  cout << "Support = " << support << " pixels" << endl;
  wCellSize=2*baseline*freq[0]/wSize;
  cout << "W cellsize = " << wCellSize << " wavelengths" << endl;

  // Convolution function. This should be the convolution of the
  // w projection kernel (the Fresnel term) with the convolution
  // function used in the standard case. The latter is needed to
  // suppress aliasing. In practice, we calculate entire function
  // by Fourier transformation. Here we take an approximation that
  // is good enough.
  int sSize=2*support+1;
  int cCenter=(sSize-1)/2;

  C.resize(sSize*sSize*overSample*overSample*wSize);
  cout << "Size of convolution function = " << sSize*sSize*overSample
      *overSample*wSize*8/(1024*1024) << " MB" << std::endl;
  cout << "Shape of convolution function = [" << sSize << ", " << sSize << ", "
      << overSample << ", " << overSample << ", " << wSize << "]" << std::endl;

  for (int k=0; k<wSize; k++)
  {
    double w=double(k-wSize/2);
    double fScale=sqrt(abs(w)*wCellSize*freq[0])/cellSize;

//    printf("wsize = %d, w = %f, fScale = %f, wCellSize = %f, freq[0] = %f, cellSize= %f\n", 
//	   wSize, w, fScale, wCellSize, freq[0], cellSize);

    for (int osj=0; osj<overSample; osj++)
    {
      for (int osi=0; osi<overSample; osi++)
      {
        for (int j=0; j<sSize; j++)
        {
          double j2=std::pow((double(j-cCenter)+double(osj)/double(overSample)), 2);
          for (int i=0; i<sSize; i++)
          {
            double r2=j2+std::pow((double(i-cCenter)+double(osi)/double(overSample)), 2);
            long int cind=i+sSize*(j+sSize*(osi+overSample*(osj+overSample*k)));
            if (w!=0.0)
            {
              C[cind]=static_cast<std::complex<float> >(std::cos(r2/(w*fScale)));
            }
            else
            {
              C[cind]=static_cast<std::complex<float> >(std::exp(-r2));
            }
          }
        }
      }
    }
  }

  // Now normalise the convolution function
  float sumC=0.0;
  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    sumC+=abs(C[i]);
  }

  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    C[i]*=std::complex<float> (wSize*overSample*overSample/sumC);
  }
/*
  for (int i = 0; i < sSize*sSize*overSample*overSample*wSize; i++) {
      printf("C[%d] = (%f, %f)\n", i, real(C[i]), imag(C[i]));
  }
*/
}

// Initialize Lookup function
// - This is application specific and should not need any changes.
//
// nSamples - number of visibility samples
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// gSize - size of grid in pixels (per axis)
// support - Total width of convolution function=2*support+1
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w
void ModelImageFft::initCOffset(const std::vector<double>& u, const std::vector<double>& v,
    const std::vector<double>& w, const std::vector<double>& freq,
    const double cellSize, const double wCellSize, const double baseline,
    const int wSize, const int gSize, const int support, const int overSample,
    std::vector<unsigned int>& cOffset, std::vector<unsigned int>& iu,
    std::vector<unsigned int>& iv)
{
  const int nSamples = u.size();
  const int nChan = freq.size();
  int sSize=2*support+1;

  // Now calculate the offset for each visibility point
  cOffset.resize(nSamples*nChan);
  iu.resize(nSamples*nChan);
  iv.resize(nSamples*nChan);
  for (int i=0; i<nSamples; i++)
  {
    for (int chan=0; chan<nChan; chan++)
    {
      int dind=i*nChan+chan;

      // Replacements by Ana:
      double uScaled=freq[chan]*u[i]/cellSize;
      int tmp = (int)uScaled;
      if (uScaled<(double)tmp) {
        tmp-=1; // btw, it goes here pretty often
      }
      // the overSample cast to Coord is also important, 'cause otherwise the subtraction
      // results is truncated to 0 before the multiplication. 
      int fracu=int((double)overSample*(uScaled-double(tmp)));

      iu[dind]=tmp+gSize/2;

      double vScaled=freq[chan]*v[i]/cellSize;
      tmp = (int)vScaled;
      if (vScaled<(double)tmp)
      {
        tmp-=1;
      }
      int fracv=int((double)overSample*(vScaled-double(tmp)));
      iv[dind]=tmp+gSize/2;

      // The beginning of the convolution function for this point
      double wScaled=freq[chan]*w[i]/wCellSize;
      int woff=wSize/2+int(wScaled);
      cOffset[dind]=sSize*sSize*(fracu+overSample*(fracv+overSample*woff));
    }
  }
}
