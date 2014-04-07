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
#include <tables/Tables/Table.h>                      // access image as a table
#include <measures/Measures/Stokes.h>                 // casa::Stokes::StokesTypes
#include <lattices/Lattices/LatticeFFT.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageFFT.h>
#include <coordinates/Coordinates/CoordinateSystem.h> //for spectral coord
#include <coordinates/Coordinates/Coordinate.h>


#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <Common/SystemUtil.h>    // needed for basename
#include <Common/Exception.h>     // THROW macro for exceptions

#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelImageFFT.h>

#include <iterator>

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
                              double uvscaleX, double uvscaleY, Int cacheSizeMB)
{
  // initialize Image properties
  itsImageProperties.I=false;
  itsImageProperties.Q=false;
  itsImageProperties.U=false;
  itsImageProperties.V=false;

  // set options
  itsOptions.name=name;
  itsOptions.nwplanes=nwplanes;
  itsOptions.oversampling=oversampling;
  setUVScale(uvscaleX, uvscaleY);

  LatticeBase *baseLattice=ImageOpener::openImage(name);
  DataType dtype;
  if(baseLattice!=NULL)
  {
    dtype=baseLattice->dataType();
  }
  else
  {
    THROW(BBSKernelException, "Error opening image: " << name);
  }
  if(dtype!=TpFloat)
  {
    THROW(BBSKernelException, "Unsupported data type: " << dtype);
  }
  
  ImageInterface<Float> *image=dynamic_cast<ImageInterface<Float>* >(baseLattice); 
  ASSERT(image);
  
  itsImageProperties.name=name;
  // Check if image is valid, i.e. has Jy/beam and I OR Q and U Stokes
  if(!validUnits(name))
  {
    THROW(BBSKernelException, "Invalid Model image: " << name << " invalid Units, must be Jy/beam");
  }

  this->getImageProperties(*image);     // get image properties
  this->getImageFrequencies();          // get image frequencies and save them in options
  if(!validStokes(itsImageProperties.stokes))
  {
    THROW(BBSKernelException, "Invalid Model image: " << name << " wrong Stokes components.");  
  }
  
  // allocate memory for itsImage (FFT'ed image) by copy-constructor from image
  itsImage=new TempImage<Complex>(image->shape(), image->coordinates(), cacheSizeMB);
  // 2D-FFT every image plane along their Direction axes
  ImageFFT imgFFT;
  imgFFT.fft(*image, getFourierAxes(*image));
  imgFFT.getComplex(*itsImage);
  delete image;                   // don't need the original image anymore
}

ModelImageFft::~ModelImageFft(void)
{
  free(itsImage);      // release memory of FFT'ed complex image hyper-cube
}

//**********************************************
//
// Setter functions for attributes
//
//**********************************************

void ModelImageFft::setPhaseDirection(const casa::MDirection &phaseDir)
{
  itsOptions.phaseDirection=phaseDir;
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

void ModelImageFft::setOversampling(int oversampling)
{
  itsOptions.oversampling=oversampling;
}

void ModelImageFft::setNwplanes(unsigned int nwplanes)
{
  itsOptions.nwplanes=nwplanes;
}

void ModelImageFft::setFrequencies(const vector<double> &frequencies)
{
  itsOptions.frequencies=frequencies;
}

void ModelImageFft::setFrequencies(const double *frequencies, size_t n)
{
  itsOptions.frequencies=vector<double>(frequencies, frequencies+n);
}

//**********************************************
//
// Image property functions
//
//**********************************************

template <class T> void ModelImageFft::getImageProperties(const ImageInterface<T> &image)
{
  CoordinateSystem coordSys=image.coordinates();   // get coordinate system of image
  
  IPosition shape=image.shape();
  itsImageProperties.nCoords=coordSys.nCoordinates();
  Int DirectionCoordInd=coordSys.findCoordinate(Coordinate::DIRECTION);

  // DIRECTION coordinates
  DirectionCoordinate directionCoord=coordSys.directionCoordinate(0);
  if(DirectionCoordInd == -1)
  {
    THROW(BBSKernelException, "No DIRECTION coordinates in image " << itsOptions.name);
  }
  itsImageProperties.nPixelAxes=directionCoord.nPixelAxes();
  if(itsImageProperties.nPixelAxes==2)    // check we have 2 directional axes
  {
    // Keep these values in imageProperties
    itsImageProperties.shape=shape;
    itsImageProperties.DirectionCoordInd=DirectionCoordInd;
    itsImageProperties.DirectionCoordAxes=coordSys.pixelAxes(DirectionCoordInd);

    // Direction shapes
    itsImageProperties.nx=shape(itsImageProperties.DirectionCoordAxes(0)); 
    itsImageProperties.ny=shape(itsImageProperties.DirectionCoordAxes(1));
    LOG_INFO_STR("Image " << itsOptions.name << " has dimensions nx = " << 
    itsImageProperties.nx << " ny = " << itsImageProperties.ny << ".");
  }
  else
  {
    THROW(BBSKernelException, "Incorrect number of pixel axes: " << coordSys.nPixelAxes());
  }

  // SPECTRAL coordinate
  Int SpectralCoordInd=coordSys.findCoordinate(Coordinate::SPECTRAL);
  if(SpectralCoordInd != -1)
  {
    itsImageProperties.nchan=shape(coordSys.pixelAxes(SpectralCoordInd)(0));
    LOG_INFO_STR("Image has " << itsImageProperties.nchan << " frequency channels.");

    SpectralCoordinate specCoord=image.coordinates().spectralCoordinate(SpectralCoordInd);
    itsImageProperties.SpectralCoordAxes=coordSys.pixelAxes(SpectralCoordInd);
  }
  else
  {
    itsImageProperties.nchan=1;
  }
  // STOKES coordinate
  Int StokesCoordInd=coordSys.findCoordinate(Coordinate::STOKES);
  if(StokesCoordInd != -1)
  {
    itsImageProperties.npol=shape(coordSys.pixelAxes(StokesCoordInd)(0));
    LOG_INFO_STR("Image " << itsOptions.name << " has " << itsImageProperties.npol 
    << " polarizations.");

    StokesCoordinate stokesCoord=image.coordinates().stokesCoordinate(StokesCoordInd);
    itsImageProperties.StokesCoordAxes=coordSys.pixelAxes(StokesCoordInd);
    itsImageProperties.stokes=image.coordinates().stokesCoordinate(StokesCoordInd).stokes(); 
  }
  else
  {
    itsImageProperties.npol=1;
  }
}

// Check that input model image has Jy/pixel flux
//
bool ModelImageFft::validUnits(const String &imageName)
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

template <class T> bool ModelImageFft::validUnits(const ImageInterface<T> &image)
{
  bool valid=false;
  if(image.units().getName()=="Jy/pixel")
  {
    valid=true;
  }
  else
  {
    valid=false;
  }
  return valid;
}


// Check if the image contains a valid selection of Stokes components
// Currently only supports: I, (later Q + U)
//
bool ModelImageFft::validStokes(casa::Vector<casa::Int> stokes)
{
  bool valid=false;       // overall judgement on valid Stokes parameters

  for(uInt i=0; i<stokes.size(); i++)
  {
    if(Stokes::type(stokes[i]) == Stokes::I)
    {
      itsImageProperties.I=true;
    }
    else if(Stokes::type(stokes[i]) == Stokes::Q)
    {
      itsImageProperties.Q=true;
    }
    else if(Stokes::type(stokes[i]) == Stokes::U)
    {
      itsImageProperties.U=true;
    }
  }
  
  // A valid model image has either I or Q,U and V Stokes
  if(itsImageProperties.I || (itsImageProperties.Q && itsImageProperties.U && itsImageProperties.V))
  {
    valid=true;
  }
  else if(itsImageProperties.I && itsImageProperties.Q && itsImageProperties.U && itsImageProperties.V)
  {
    valid=true;
  }
  else
  {
    valid=false;
  }
  return valid;
}

// Get the patch direction, i.e. RA/Dec of the central image pixel
//
//casa::MDirection ModelImageFft::getPatchDirection(const ImageInterface<Float> &image)
template <class T> casa::MDirection ModelImageFft::getPatchDirection(const ImageInterface<T> &image)
{
  casa::Vector<casa::Double> Pixel(2);                    // pixel coords vector of image centre
  casa::MDirection MDirWorld(casa::MDirection::J2000);    // astronomical direction in J2000
    
  Pixel[0]=floor(itsImageProperties.nx/2);                // get centre pixel
  Pixel[1]=floor(itsImageProperties.ny/2);
  // Determine DirectionCoordinate
  casa::DirectionCoordinate dir(image.coordinates().directionCoordinate (image.coordinates().findCoordinate(casa::Coordinate::DIRECTION)));
  dir.toWorld(MDirWorld, Pixel);

  return MDirWorld;
}

// Get channel frequencies from image
//
Vector<Double> ModelImageFft::getImageFrequencies()
{
  uInt nchan=itsImageProperties.nchan;
  // set up vector of number of image channels
  vector<double> frequencies(nchan);
  Vector<Double> frequenciesVec(nchan);
  Vector<Double> pixels(nchan);
  for(unsigned int i=0; i<nchan; i++)
  {
    pixels[i]=i;
  }
  // get frequencies from spectralCoord attribute
  itsImageProperties.spectralCoord.toWorld(frequenciesVec, pixels);
  frequenciesVec.tovector(frequencies); 
  itsImageProperties.frequencies=frequencies; // this is now kept in the ImageProperties
  
  return frequencies;
}

// Determine directional coordinate indices which are the axes which should be
// FFT-ed and set to True in Vector<Bool>
//
template<class T> Vector<Bool> ModelImageFft::getFourierAxes(const ImageInterface<T> &image)
{
  CoordinateSystem coordSys=image.coordinates();   // get coordinate system of image
  IPosition shape=image.shape();

  if(itsImageProperties.DirectionCoordAxes.size()!=2)
  {
    THROW(BBSKernelException, "getFourierAxes(): number of direction axes must be 2.");
  }

  // 2D-FFT the image per channel
  Vector<Bool> FourierAxes(image.shape().size());    // axes to Fourier transform
  for(uInt i; i < (uInt) FourierAxes.size(); i++)    // this nasty thing is to avoid a uInt/Int warning
  {
    // for the Direction axes set Fourier Transform to true
    if( i==(uInt) itsImageProperties.DirectionCoordAxes(0) || 
        i==(uInt) itsImageProperties.DirectionCoordAxes(1))
    {
      FourierAxes[i]=True;
    }
    else
    {
      FourierAxes[i]=False; // otherwise don't transform spectral and polarization axes
    }
  }
  return FourierAxes;
}

// Find nearest frequency to match requested channel with image frequency channels
// returns chanMap index vector into image frequencies
Vector<Int>  ModelImageFft::chanMap(const vector<double> &frequencies)
{
  unsigned int nfreqs=frequencies.size();

  vector<double> imageFreqs=imageFrequencies();
  Vector<Int> chanMap(nfreqs);      // channel map to return

  /*
  for(unsigned int i=0; i<nfreqs; i++)
  {
    double pixel=0;
    if(itsImageProperties.spectralCoord.toWorld(pixel, frequencies[i]))
    {
      cout << "pixel: " << pixel << endl;
      chanMap[i]=pixel;
    }
    else
    {
      LOG_WARN_STR("Mapping of frequency " << frequencies[i] << "to image failed.");
    }    
  }
  */
  for(unsigned int i=0; i<nfreqs; i++)
  {
    double lower=-1, upper=-1;
    double val=frequencies[i];     // search nearest neighbour for this frequency
  
    std::vector<double>::iterator it;
    it = lower_bound(imageFreqs.begin(), imageFreqs.end(), val);
    if (it == imageFreqs.begin())
    {
      upper = *it;              // no smaller value than val in vector
      chanMap[i]=upper;
    }
    else if (it == imageFreqs.end())
    {
      lower = *(it-1);          // no bigger value than val in vector
      chanMap[i]=lower;
    }
    else 
    {
      lower = *(it-1);    // lower neighbour in image channels
      upper = *it;        // upper neighbour in image channels
      if(abs(lower-val) < abs(val-upper))   // find nearest neighbour
      {
        chanMap[i]=lower;
      }
      else
      {
        chanMap[i]=upper;
      }
    }
  }
  return chanMap;
}

Vector<Int> ModelImageFft::chanMap(const double *frequencies, size_t nfreqs)
{
  vector<double> freqsvec(frequencies, frequencies + nfreqs);
  Vector<Int> chanMap=this->chanMap(freqsvec);
  return chanMap;
}

// Show image properties on std
//
void ModelImageFft::printImageProperties()
{
  cout << "ImageProperties: " << itsImageProperties.name << endl;
  cout << "Shape:     " << itsImageProperties.shape << endl;
  cout << "DirectionCoordInd: " << itsImageProperties.DirectionCoordInd << endl;
  cout << "SpectralCoordInd: " << itsImageProperties.SpectralCoordInd << endl;
  cout << "StokesCoordInd: " << itsImageProperties.StokesCoordInd << endl;
  cout << "nx:         " << itsImageProperties.nx << endl;
  cout << "ny:         " << itsImageProperties.ny << endl;
  cout << "nchan:      " << itsImageProperties.nchan << endl;
  cout << "npol:       " << itsImageProperties.npol << endl;
  cout << "Stokes:     ";
  for(uInt i=0; i<itsImageProperties.stokes.size(); i++)
  {
    cout << Stokes::name(Stokes::type(itsImageProperties.stokes[i])) << " ";
  }
  cout << endl;
  cout << "I:          " << itsImageProperties.I << endl;
  cout << "Q:          " << itsImageProperties.Q << endl;
  cout << "U:          " << itsImageProperties.U << endl;
  cout << "V:          " << itsImageProperties.V << endl;
}

//**********************************************
//
// Degridding functions
//
//**********************************************

/*
void ModelImageFft::degrid( const double *uvwBaselines, 
                            size_t nuvw, size_t nchans, 
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
  vector<double> uswBaselinesVec(nuvw);
  for(unsigned int i=0; i<nuvw; i++)
  {
    uswBaselinesVec[i]=*(uvwBaselines+i);
  }

  // call Cornwell degrid
  
  // assign return arrays to vector addresses
}
*/
void ModelImageFft::degrid( const double *uBl, const double *vBl, const double *wBl, 
                            size_t nuvw, size_t nfreqs, 
                            const double *frequencies,
                            casa::DComplex *XX , casa::DComplex *XY, 
                            casa::DComplex *YX , casa::DComplex *YY,
                            double maxBaseline)
{
  setFrequencies(frequencies, nfreqs);       // don't really need to do that...
  
  // convert uvwBaseline to Cornwell degrid format, 1-D data vector
  int nSamples=nuvw*nfreqs;       // number of samples
  LOG_INFO_STR("degridding " << nSamples << " samples.");

  //------------------------------------------------------------------------
  // Prepare uvw variables etc.
  //
  vector<complex<float> > data(nuvw*nfreqs);
  vector<double> u(uBl, uBl+nuvw);      // u coord of requested baselines
  vector<double> v(vBl, vBl+nuvw);      // v coord of requested baselines
  vector<double> w(wBl, wBl+nuvw);      // w coord of requested baselines
  // Don't change any of these numbers unless you know what you are doing!
  int gSize=itsImageProperties.nx;      // Size of output grid in pixels (only support square pixels)
  double cellSize=40.0;                 // Cellsize of output grid in wavelengths
  vector<std::complex<float> > grid(gSize*gSize);
  grid.assign(grid.size(), std::complex<float> (1.0));

  int wSize=itsOptions.nwplanes;        // Number of lookup planes in w projection
  
  // match frequencies to channels in image
  vector<double> freqsvec(frequencies, frequencies + nfreqs);
  itsOptions.chanMap=chanMap(freqsvec);

  // Measure frequency in inverse wavelengths
  std::vector<double> freq(nfreqs);
  itsOptions.lambdas.resize(nfreqs);
  for (unsigned int i=0; i<nfreqs; i++)
  {
    itsOptions.lambdas[i]=(casa::C::c)/frequencies[i];
  }

  // Initialize convolution function and offsets
  std::vector<std::complex<float> > C;
  //int support, overSample;
  int support;
  std::vector<unsigned int> cOffset;
  // Vectors of grid centers
  std::vector<unsigned int> iu;
  std::vector<unsigned int> iv;
  double wCellSize;

  //----------------------------------------------------------------------
  // Initialize Convolution function
  //
  initC(nSamples, w, itsOptions.lambdas, cellSize, maxBaseline, wSize, gSize, support, 
        itsOptions.oversampling, wCellSize, C);

  //------------------------------------------------------------------------
  // Degridding of FFT image planes for different Stokes components
  //
  // For now only support Stokes I component in image!
  //
  if(itsImageProperties.I)     // Do Stokes I
  {
    LOG_INFO_STR("degridding Stokes I: Freq " << itsOptions.frequencies[0]/1e6 << " - " 
    << itsOptions.frequencies[nfreqs-1]/1e6 << " MHz" );
    for(uInt freq=0; freq<nfreqs; freq++)
    {
      //----------------------------------------------------------------------
      // Get image plane
      //
      //Array<Complex> imagePlane;        // array to hold image plane
      IPosition planeShape(4);
      planeShape[itsImageProperties.DirectionCoordAxes(0)]=itsImageProperties.nx;
      planeShape[itsImageProperties.DirectionCoordAxes(1)]=itsImageProperties.ny;
      planeShape[itsImageProperties.StokesCoordAxes(0)]=1;       // 1 Stokes
      planeShape[itsImageProperties.SpectralCoordAxes(0)]=1;     // 1 channel

      Array<Complex> imagePlane(planeShape); 
      Slicer slicer=makeSlicer(freq);           // Stokes defaults to I

      if(itsImage->getSlice(imagePlane, slicer))
      {
        //ASSERT(imagePlane.contiguous);      // image slice is contiguous, use        
        imagePlane.tovector(grid);    // copy data into STL vector to conform to Cornwell interface
        // Convert baselines to lambda units
        u=convertMetresToWavelengths(u, itsOptions.frequencies[freq]);
        v=convertMetresToWavelengths(v, itsOptions.frequencies[freq]);
        w=convertMetresToWavelengths(w, itsOptions.frequencies[freq]);

        //----------------------------------------------------------------------
        // Initialize COffset
        //
        initCOffset(u, v, w, itsOptions.lambdas, cellSize, wCellSize, maxBaseline, wSize, gSize,
                    support, itsOptions.oversampling, cOffset, iu, iv);
        degridKernel(grid, gSize, support, C, cOffset, iu, iv, data);  // call Cornwell degridKernel
      }
      else
      {
        LOG_WARN_STR("Did not get image slice for frequency: " << itsOptions.frequencies[freq]);
      }
      // copy vector into correlation arrays: TODO: XX=0.5*I, YY=0.5*I
      if(XX && YY)    // only copy, if we have a valid pointer
      {
        computeICorr(data, XX, YY);
      }
      else
      {
        LOG_WARN_STR("degrid(): XX or YY parameter empty, no correlations output.");
      }
    }
  }
}

// Create a slicer for the requested image plane / polarization (default=I)
//
Slicer ModelImageFft::makeSlicer(Int chan, const String &Stokes)
{
  IPosition plane(itsImageProperties.shape);

//  cout << "itsOptions.chanMap[" << chan << "]: " << itsOptions.chanMap[chan] << endl;  // DEBUG

  plane[itsImageProperties.DirectionCoordAxes(0)]=itsImageProperties.nx;  // full x dimension
  plane[itsImageProperties.DirectionCoordAxes(1)]=itsImageProperties.ny;  // full y dimension
  plane[itsImageProperties.StokesCoordAxes(0)]=1;                          // 1 Stokes
  plane[itsImageProperties.SpectralCoordAxes(0)]=1;                        // 1 channel
  
  IPosition start(itsImageProperties.shape);    // start of slice
  IPosition end(itsImageProperties.shape);      // end of slice
  
  start[itsImageProperties.DirectionCoordAxes(0)]=0;
  start[itsImageProperties.DirectionCoordAxes(1)]=0;
  start[itsImageProperties.StokesCoordAxes(0)]=itsImage->coordinates().stokesPixelNumber(Stokes);
  start[itsImageProperties.SpectralCoordAxes(0)]=itsOptions.chanMap[chan];

  end[itsImageProperties.DirectionCoordAxes(0)]=itsImageProperties.nx-1;
  end[itsImageProperties.DirectionCoordAxes(1)]=itsImageProperties.ny-1;
  end[itsImageProperties.StokesCoordAxes(0)]=itsImage->coordinates().stokesPixelNumber(Stokes);
  end[itsImageProperties.SpectralCoordAxes(0)]=itsOptions.chanMap[chan];  
  
  Slicer slicer(start, end, Slicer::endIsLast);
  return slicer;
}

//**********************************************
//
// Correlation computation functions
//
//**********************************************

// Using std::vector datacontainers
void ModelImageFft::computeICorr( const vector<complex<float> > &data, 
                                  vector<complex<float> > &XX,
                                  vector<complex<float> > &YY)
{
  for(unsigned int i=0; i<data.size(); i++)
  {
    XX[i]=data[i];
    YY[i]=data[i];
  }
}

void ModelImageFft::computeICorr( const vector<complex<float> > &data, 
                                  DComplex *XX, DComplex *YY)
{
  for(unsigned int i=0; i<data.size(); i++)
  {
    XX[i]=data[i];
    YY[i]=data[i];
  }
}

// Using data stored in pointers
void ModelImageFft::computeICorr(const complex<float> *data, size_t nuvw,
                                 DComplex *XX, DComplex *YY)
{
  for(unsigned int i=0; i<nuvw; i++)
  {
    XX[i]=data[i];
    YY[i]=data[i];
  }
}

/*
void ModelImageFft::computePolCorr( const vector<complex<float> > &I
                                    const vector<complex<float> > &Q, 
                                    const vector<complex<float> > &U,
                                    const vector<complex<float> > &V,
                                    vector<complex<float> > &XX,
                                    vector<complex<float> > &XY,
                                    vector<complex<float> > &XY,
                                    vector<complex<float> > &YY)
{
//  for_each (myvector.begin(), myvector.end(), myfunction);
  for(unsigned int i=0; i<data.size(); i++)
  {
    XX[i] = I[i] + Q[i];                      // XX= I+Q
    XY[i] = U[i] + complex<float>(0,1)*V[i];  // XY = U+iV
    YX[i] = U[i] - complex<float>(0,1)*V[i];  // YX = U-iV
    YY[i] = I[i] - Q[i];                      // YY = I-Q
  }  
}

void ModelImageFft::computePolCorr( const complex<float> *Q, 
                                    const complex<float> *U,
                                    const complex<float> *V,  
                                    size_t nuvw,
                                    DComplex *XX, DComplex *XY,
                                    DComplex *YX, DComplex *YY)
{
  for(unsigned int i=0; i<data.size(); i++)
  {
    XX[i] = I[i] + Q[i];
    XY[i] = U[i] + complex<float>(0,1)*V[i];
    YX[i] = U[i] - complex<float>(0,1)*V[i];
    YY[i] = I[i] - Q[i];
  }
}
*/

//**********************************************
//
// Helper functions
//
//**********************************************

// Convert baselines from metres (m) to wavelengths (lambdas)
//
vector<double> ModelImageFft::convertMetresToWavelengths( const vector<double> &metres, 
                                                          double freq)
{
  double lambda=(casa::C::c)/freq;

  vector<double> lambdas(metres.size());
  for(unsigned int i=0; i<metres.size(); i++)
  {
    lambdas[i]=metres[i]/lambda;
  }

  return lambdas;
}

void ModelImageFft::convertMetresToWavelengths( const double *metres, double *lambdas,
                                                unsigned int nuvw, double freq)
{
  double lambda=(casa::C::c)/freq;
  for(unsigned int i=0; i<nuvw; i++)
  {
    lambdas[i]=metres[i]/lambda;
  }
}

// Convert baselines from wavelengths (lambdas) to metres (m)
//
vector<double> ModelImageFft::convertWavelengthsToMetres( const vector<double> &lambdas, 
                                                          double freq)
{
  vector<double> metres(lambdas.size());      // in metres

  double lambda=(casa::C::c)/freq;

  vector<double>::iterator itMetres;
  vector<double>::const_iterator itLambdas;
  for(itLambdas=lambdas.begin(), itMetres=metres.begin(); itLambdas!=lambdas.end(); 
      itLambdas++, itMetres++)
  {
    *itMetres=*itLambdas*lambda;
  }
  
  return metres;
}

void ModelImageFft::convertWavelengthsToMetres( const double *lambdas, double *metres, 
                                                unsigned int nuvw, double freq)
{
  double lambda=(casa::C::c)/freq;
  for(unsigned int i=0; i<nuvw; i++)
  {
    metres[i]=lambdas[i]*lambda;
  }
}

// Convert a Vector of frequencies to lambdas
//
Vector<Double> ModelImageFft::convertToLambdas(const Vector<Double> &frequencies)
{
  Vector<Double> lambdas(frequencies.size());
  for(uInt i=0; i<frequencies.size(); i++)
  {
    lambdas[i]=(casa::C::c)/frequencies[i];
  }
  return lambdas;
}

// Write an image out to file
void ModelImageFft::writeImage(const Array<Complex> &imagePlane, const String &filename)
{
  IPosition shape=imagePlane.shape();
  PagedImage<Complex> image(shape, itsImage->coordinates(), filename);

  image.put(imagePlane);
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

  LOG_INFO_STR("sSize = " << sSize << ", support = " << support << " dataSize = " << data.size());

  for (unsigned int dind=0; dind<data.size(); dind++)
  {
    // Nearly all the L2 cache misses originate here in the next
    // two statements
    // The actual grid point
    int gind=iu[dind]+gSize*iv[dind]-support;
    // The Convoluton function point from which we offset
    int cind=cOffset[dind];

//    cout << "gind: " << gind << ", cind: " << cind << endl;   // DEBUG

    for (int suppv=0; suppv<sSize; suppv++)
    {
#ifdef USE_CBLAS
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

// Perform degridding with Tim Cornwell's degridding
//
void ModelImageFft::degridKernel( const std::vector<std::complex<float> >& grid, 
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
#ifdef USE_CBLAS
      std::complex<double>  dot;
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
  LOG_INFO_STR("Initializing W projection convolution function");
  support=static_cast<int>(1.5*sqrt(abs(baseline) *static_cast<double>(cellSize)
      *freq[0])/cellSize);
  overSample=8;
  LOG_DEBUG_STR("Support = " << support << " pixels");
  wCellSize=2*baseline*freq[0]/wSize;
  LOG_DEBUG_STR("W cellsize = " << wCellSize << " wavelengths");

  // Convolution function. This should be the convolution of the
  // w projection kernel (the Fresnel term) with the convolution
  // function used in the standard case. The latter is needed to
  // suppress aliasing. In practice, we calculate entire function
  // by Fourier transformation. Here we take an approximation that
  // is good enough.
  int sSize=2*support+1;
  int cCenter=(sSize-1)/2;

  C.resize(sSize*sSize*overSample*overSample*wSize);
  LOG_DEBUG_STR("Size of convolution function = " << sSize*sSize*overSample
      *overSample*wSize*8/(1024*1024) << " MB");
  LOG_DEBUG_STR("Shape of convolution function = [" << sSize << ", " << sSize << ", "
      << overSample << ", " << overSample << ", " << wSize << "]");

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
              C[cind]=static_cast<std::complex<double> >(std::cos(r2/(w*fScale)));
            }
            else
            {
              C[cind]=static_cast<std::complex<double> >(std::exp(-r2));
            }
          }
        }
      }
    }
  }

  // Now normalise the convolution function
  double sumC=0.0;
  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    sumC+=abs(C[i]);
  }

  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    C[i]*=std::complex<double> (wSize*overSample*overSample/sumC);
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
