//# LofarConvolutionFunction.h: Compute LOFAR convolution functions on demand.
//#
//# Copyright (C) 2011
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
//# $Id$

#ifndef LOFARFT_LOFARCONVOLUTIONFUNCTION_H
#define LOFARFT_LOFARCONVOLUTIONFUNCTION_H

#include <LofarFT/LofarATerm.h>
#include <LofarFT/LofarWTerm.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/FFTCMatrix.h>
#include <Common/Timer.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/PagedImage.h>
#include <casa/Utilities/Assert.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/OS/PrecTimer.h>


using namespace casa;

namespace LOFAR
{

  // Functions to store a 2D or 3D array in an PagedImage file.
  template <class T>
  void store(const DirectionCoordinate &dir, const Matrix<T> &data, const string &name);

  template <class T>
  void store(const DirectionCoordinate &dir, const Cube<T> &data, const string &name);

  template <class T>
  void store(const Matrix<T> &data, const string &name);

  template <class T>
  void store(const Cube<T> &data, const string &name);


  class LofarConvolutionFunction
  {

  public:
    LofarConvolutionFunction(const IPosition& shape,
                             const DirectionCoordinate& coordinates,
                             const MeasurementSet& ms,
                             uInt nW, double Wmax,
                             uInt oversample,
                             const String& beamElementPath,
                             const String& save_image_beam_directory="",
			     Int verbose=0,
			     Int maxsupport=1024);

//      ~LofarConvolutionFunction ()
//      {
//      }

    // Show the relative timings of the various steps.
    void showTimings (std::ostream&, double duration, double timeCF) const;

    // Show percentage of value in total with 1 decimal.
    static void showPerc1 (std::ostream& os, double value, double total);

    // Compute and store W-terms and A-terms in the fourier domain
    void store_all_W_images();

    // Get the spheroidal cut.
    const Matrix<Float>& getSpheroidCut() const
      { return Spheroid_cut_im; }

    // Compute the fft of the beam at the minimal resolution for all antennas,
    // and append it to a map object with a (double time) key.
    void computeAterm(Double time);

    // Compute the convolution function for all channel, for the polarisations
    // specified in the Mueller_mask matrix
    // Also specify weither to compute the Mueller matrix for the forward or
    // the backward step. A dirty way to calculate the average beam has been
    // implemented, by specifying the beam correcting to the given baseline
    // and timeslot.
    // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]
    LofarCFStore makeConvolutionFunction(uInt stationA, uInt stationB,
                                         Double time, Double w,
                                         const Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         Matrix<Complex>& Stack_PB_CF,
                                         double& sum_weight_square);

    // Returns the average Primary Beam from the disk
    Matrix<float> Give_avg_pb();

    // Compute the average Primary Beam from the Stack of convolution functions
    Matrix<Float> Compute_avg_pb(Matrix<Complex> &Sum_Stack_PB_CF,
                                 double sum_weight_square);

    // Zero padding of a Cube
    Cube<Complex> zero_padding(const Cube<Complex>& Image, int Npixel_Out);

    // Zero padding of a Matrix
    Matrix<Complex> zero_padding(const Matrix<Complex>& Image, int Npixel_Out);

    // Get the W scale.
    const WScale& wScale() const
      { return m_wScale; }

  private:
    void normalized_fft (Matrix<Complex>&, bool toFreq=true);
    void normalized_fft (PrecTimer& timer, Matrix<Complex>&, bool toFreq=true);

    MEpoch observationStartTime (const MeasurementSet &ms,
                                 uInt idObservation) const;

    Double observationReferenceFreq (const MeasurementSet &ms,
                                     uInt idDataDescription);

    // Estime spheroidal convolution function from the support of the fft
    // of the spheroidal in the image plane
    Double estimateSpheroidalResolution(const IPosition &shape,
                                        const DirectionCoordinate &coordinates);

    // Return the angular resolution required for making the image of the
    // angular size determined by coordinates and shape.
    // The resolution is assumed to be the same on both direction axes.
    Double estimateWResolution(const IPosition &shape,
                               const DirectionCoordinate &coordinates,
                               Double w) const;

    // Return the angular resolution required for making the image of the
    // angular size determined by coordinates and shape.
    // The resolution is assumed to be the same on both direction axes.
    Double estimateAResolution(const IPosition &shape,
                               const DirectionCoordinate &coordinates) const;

    // Apply a spheroidal taper to the input function.
    template <typename T>
    void taper (Matrix<T> &function) const
    {
      AlwaysAssert(function.shape()(0) == function.shape()(1), SynthesisError);
      //cout<<"function.shape()(0) "<<function.shape()(0)<<endl;
      uInt size = function.shape()(0);
      Double halfSize = (size-1) / 2.0;
      Vector<Double> x(size);
      for (uInt i = 0; i < size; ++i) {
        x(i) = spheroidal(abs(Double(i) - halfSize) / halfSize);
      }
      for(uInt i = 0; i < size; ++i) {
        for(uInt j = 0; j < size; ++j) {
          function(j, i) *= x(i) * x(j);
        }
      }
    }
    
    Double spheroidal(Double nu) const;

    template <typename T>
    uInt findSupport(Matrix<T> &function, Double threshold) const
    {
      Double peak = abs(max(abs(function)));
      threshold *= peak;
      uInt halfSize = function.shape()(0) / 2;
      uInt x = 0;
      while (x < halfSize && abs(function(x, halfSize)) < threshold) {
        ++x;
      }
      return 2 * (halfSize - x);
    }


    //# Data members.
    IPosition           m_shape;
    DirectionCoordinate m_coordinates;
    WScale              m_wScale;
    LofarWTerm          m_wTerm;
    LofarATerm          m_aTerm;
    Double              m_maxW;
    Double              Pixel_Size_Spheroidal;
    uInt                m_nWPlanes;
    uInt                m_nStations;
    uInt                m_oversampling;
    uInt                m_nChannel;
    Double              m_refFrequency;
    uInt                m_maxCFSupport;
    //# Stack of the convolution functions for the average PB calculation
    Matrix<Complex>     Spheroid_cut;
    //# Stack of the convolution functions for the average PB calculation
    Matrix<float>       Spheroid_cut_im;
    DirectionCoordinate coordinates_Conv_Func_image;
    string              save_image_Aterm_dir;
    //# List of the ferquencies the CF have to be caluclated for
    Vector< Double >    list_freq;
    vector< Matrix<Complex> > m_WplanesStore;
    //# Aterm_store[double time][antenna][channel]=Cube[Npix,Npix,4]
    map<Double, vector< vector< Cube<Complex> > > > m_AtermStore;
    Int                 itsVerbose;
    Int                 itsMaxSupport;
    vector<FFTCMatrix>  itsFFTMachines;
    Double              itsTimeW;
    Double              itsTimeWpar;
    Double              itsTimeWfft;
    unsigned long long  itsTimeWcnt;
    Double              itsTimeA;
    Double              itsTimeApar;
    Double              itsTimeAfft;
    unsigned long long  itsTimeAcnt;
    Double              itsTimeCF;
    Double              itsTimeCFpar;
    Double              itsTimeCFfft;
    unsigned long long  itsTimeCFcnt;
  };



  //# =================================================
  template <class T>
  void store(const Matrix<T> &data, const string &name)
  {
    Matrix<Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
    Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
    Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
    DirectionCoordinate dir(MDirection::J2000, Projection(Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store (const DirectionCoordinate &dir, const Matrix<T> &data,
              const string &name)
  {
    cout<<"Saving... "<<name<<endl;
    Vector<Int> stokes(1);
    stokes(0) = Stokes::I;
    CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
  }

  template <class T>
  void store(const Cube<T> &data, const string &name)
  {
    Matrix<Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
    Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
    Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
    DirectionCoordinate dir(MDirection::J2000, Projection(Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const DirectionCoordinate &dir, const Cube<T> &data,
             const string &name)
  {
    AlwaysAssert(data.shape()(2) == 4, SynthesisError);
    cout<<"Saving... "<<name<<endl;
    Vector<Int> stokes(4);
    stokes(0) = Stokes::XX;
    stokes(1) = Stokes::XY;
    stokes(2) = Stokes::YX;
    stokes(3) = Stokes::YY;
    CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    PagedImage<T>
      im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)),
         csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
  }

} //# end namespace casa

#endif
