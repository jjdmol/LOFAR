//# FTMachineSimpleWB.h: Definition for FTMachineSimple
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id: FTMachineSimpleWB.h 28512 2014-03-05 01:07:53Z vdtol $

#ifndef LOFAR_LOFARFT_FTMACHINESIMPLEWB_H
#define LOFAR_LOFARFT_FTMACHINESIMPLEWB_H

#include <LofarFT/FTMachine.h>
#include <LofarFT/VisResamplerWB.h>

namespace LOFAR {
namespace LofarFT {

// <summary>  An FTMachine for Gridded Fourier transforms </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=FTMachine>FTMachine</linkto> module
//   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
//   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
// </prerequisite>
//
// <etymology>
// FTMachine is a Machine for Fourier Transforms. LofarFTMachine does
// Grid-based Fourier transforms.
// </etymology>
//
// <synopsis>
// The <linkto class=SkyEquation>SkyEquation</linkto> needs to be able
// to perform Fourier transforms on visibility data. LofarFTMachine
// allows efficient Fourier Transform processing using a
// <linkto class=VisBuffer>VisBuffer</linkto> which encapsulates
// a chunk of visibility (typically all baselines for one time)
// together with all the information needed for processing
// (e.g. UVW coordinates).
//
// Gridding and degridding in LofarFTMachine are performed using a
// novel sort-less algorithm. In this approach, the gridded plane is
// divided into small patches, a cache of which is maintained in memory
// using a general-purpose <linkto class=LatticeCache>LatticeCache</linkto> class. As the (time-sorted)
// visibility data move around slowly in the Fourier plane, patches are
// swapped in and out as necessary. Thus, optimally, one would keep at
// least one patch per baseline.
//
// A grid cache is defined on construction. If the gridded uv plane is smaller
// than this, it is kept entirely in memory and all gridding and
// degridding is done entirely in memory. Otherwise a cache of tiles is
// kept an paged in and out as necessary. Optimally the cache should be
// big enough to hold all polarizations and frequencies for all
// baselines. The paging rate will then be small. As the cache size is
// reduced below this critical value, paging increases. The algorithm will
// work for only one patch but it will be very slow!
//
// This scheme works well for arrays having a moderate number of
// antennas since the saving in space goes as the ratio of
// baselines to image size. For the ATCA, VLBA and WSRT, this ratio is
// quite favorable. For the VLA, one requires images of greater than
// about 200 pixels on a side to make it worthwhile.
//
// The FFT step is done plane by plane for images having less than
// 1024 * 1024 pixels on each plane, and line by line otherwise.
//
// The gridding and degridding steps are implemented in Fortran
// for speed. In gridding, the visibilities are added onto the
// grid points in the neighborhood using a weighting function.
// In degridding, the value is derived by a weight summ of the
// same points, using the same weighting function.
// </synopsis>
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// Define an interface to allow efficient processing of chunks of
// visibility data
// </motivation>
//
// <todo asof="97/10/01">
// <ul> Deal with large VLA spectral line case
// </todo>

class VisBuffer;  
  
class FTMachineSimpleWB : public FTMachine {
public:
  static const casa::String theirName;

  // Constructor: cachesize is the size of the cache in words
  // (e.g. a few million is a good number), tilesize is the
  // size of the tile used in gridding (cannot be less than
  // 12, 16 works in most cases), and convType is the type of
  // gridding used (SF is prolate spheriodal wavefunction,
  // and BOX is plain box-car summation). mLocation is
  // the position to be used in some phase rotations. If
  // mTangent is specified then the uvw rotation is done for
  // that location iso the image center.
  // <group>
//  LofarFTMachineOld(Long cachesize, Int tilesize, CountedPtr<VisibilityResamplerBase>& visResampler,
//	  String convType="SF", Float padding=1.0, Bool usezero=True, Bool useDoublePrec=False);
  FTMachineSimpleWB(
    const casa::MeasurementSet& ms, 
//     casa::Int nwPlanes,
//     casa::MPosition mLocation, 
//     casa::Float padding,
//     casa::Bool useDoublePrec, 
    LOFAR::ParameterSet& parset);

  virtual ~FTMachineSimpleWB();
  
  // Copy constructor
  FTMachineSimpleWB(const FTMachineSimpleWB &other);

  // Assignment operator
  FTMachineSimpleWB &operator=(const FTMachineSimpleWB &other);

  // Clone
  FTMachineSimpleWB* clone() const;

  virtual casa::String name() const { return theirName;}
  
  // Get actual coherence from grid by degridding
  virtual void get(casa::VisBuffer& vb, casa::Int row=-1);
  virtual void get(VisBuffer& vb, casa::Int row=-1);


  // Put coherence to grid by gridding.
  using casa::FTMachine::put;

  virtual void put(
    const casa::VisBuffer& vb, 
    casa::Int row = -1, 
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
  
  virtual void put(
    const VisBuffer& vb, 
    casa::Int row = -1, 
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
  
protected:

  // Get the appropriate data pointer
  casa::Array<casa::Complex>* getDataPointer(const casa::IPosition&, casa::Bool);

  // Gridder
  casa::String convType;

  casa::Float maxAbsData;

  // Useful IPositions
  casa::IPosition centerLoc;
  casa::IPosition offsetLoc;

  // Arrays for non-tiled gridding (one per thread).

  // Shape of the padded image
  casa::IPosition padded_shape;

  casa::Int convSampling;
  casa::Float pbLimit_p;
  // The average PB for sky image normalization
  //
  int itsNThread;
  casa::Int itsRefFreq;
  
  casa::CountedPtr<VisResamplerWB> itsVisResampler;
  virtual VisResampler* visresampler() {return &*itsVisResampler;}


};

} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
