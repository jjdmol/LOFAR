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
//# $Id: ModelImageFFT.h 20029 2012-02-20 15:50:23Z duscha $


#include <coordinates/Coordinates/DirectionCoordinate.h>    // DirectionCoordinate needed for patch direction
#include <casa/OS/PrecTimer.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarConvolutionFunction.h>
// etc. need more

// Options for the ModelImageFFT
typedef struct ModelImageOptions
{
    casa::String ConvType;                  // convolution type
    bool Wprojection;                       // should W-projection be used?
    bool Aprojection;                       // should A-projection be used?
    casa::Int Nwplanes;                     // number of w-projection planes (default: 0)
    double wmax;                            // maximum w value to use
    casa::MPosition Mlocation;              // location of array on earth
    casa::MDirection PhaseDir;              // phase direction of uvw data
    casa::Float padding;                    // padding used in gridding
    casa::Int verbose;                      // verbosity level
    casa::Int maxSupport;                   // maximum support
    casa::uInt oversample;                  // FFT oversampling
    casa::String imageName;                 // name of ModelImage
    casa::Matrix<Bool> gridMuellerMask;     // grid Mueller mask
    casa::Matrix<Bool> degridMuellerMask;   // degridding Mueller mask, LofarFTMachine needs this
    unsigned int NThread;                   // number of threads used
};

class ModelImageFft
{
  public:
    ModelImageFft();
    ModelImageFft(const casa::String &name,
                  const casa::MDirection phasedir=0,
                  double wmax=0, 
                  unsigned int nwplanes=1, 
                  bool aprojection=false);
//    void ModelImageFft(const casa::String &name, double nwmax=0);
    ~ModelImageFft();

    void setNWplanes(uInt n);               // manually set No. of wplanes
    void computeConvolutionFunctions();     // compute w-plane convolution functions

    // Access to options
    void setoptions(const ModelImageOptions &options); // set all options for LofarFTMachine parameters
    ModelImageOptions getOptions();         // get all options for LofarFTMachine parameters
    void setDefaults();                     // set all options to default values
    
    // Setter functions for individual options
    void setConvType(const casa::String type="SF");
    void setWprojection(bool mode=true);
    void setWmax(double wmax);
    void setAprojection(bool mode=true);
    void setNwplanes(casa::uInt nwplanes);
    void setMlocation(casa::MPosition &location);
    void setPhaseDir(const casa::MDirection &phasedir);
    void setPadding(casa::Float padding);
    void setVerbose(casa::uInt verbose);
    void setMaxSupport(casa::Int maxsupport);
    void setOversample(casa::uInt oversample);
    void setImageName(const casa::String &imagename);
    void setGridMuellerMask(casa::Matrix<Bool> muellerMask);
    void setDegridMuellerMask(casa::Matrix<Bool> muellerMask);
    void setNThread(unsigned int nthread);

    // Getter functions for individual options
    inline casa::String    getConvType() const { return itsOptions.ConvType; }
    bool                   getWprojection() const { return itsOptions.Wprojection; }
    bool                   getAprojection() const { return itsOptions.Aprojection; }
    inline casa::Int       getNwplanes() const { return itsOptions.Nwplanes; }
    inline double          getWmax() const { return itsOptions.wmax; }
    inline casa::MPosition getMlocation() const { return itsOptions.Mlocation; }
    inline casa::MDirection getPhaseDir() const { return itsOptions.PhaseDir; }
    inline casa::Float     getPadding() const { return itsOptions.padding; }
    inline casa::uInt      getVerbose() const { return itsOptions.verbose; }
    inline casa::uInt      getMaxSupport() const { return itsOptions.maxSupport; }
    inline casa::uInt      getOversample() const { return itsOptions.oversample; }
    inline casa::String    getImageName() const { return itsOptions.imageName; }
    inline unsigned int    getNThread() const { return itsOptions.NThread; }

    // Show the relative timings of the various steps.
    void showTimings (std::ostream&, double duration) const;
    static casa::Int determineNWplanes();      // determine No. of wplanes from baselines   
    
    bool wprojection() const { return itsOptions.Wprojection; }     // return if it uses w-projection
    bool aprojection() const { return itsOptions.Aprojection; }     // return if it uses A-projection
    
  private:
    casa::ImageInterface<Complex> *image;
    casa::LatticeCache<Complex> *imageCache;        // Image / FFT of image (ffted in place)

    void init();                  // initialize LofarFTMachine (might change to all-in-one init)
    void initMaps(void);          // init polarization and channel maps

    // Pre-computed convolution functions for w-projection (indexed by w-plane number)
    LOFAR::LofarConvolutionFunction * itsConvFunc;
    Vector<LOFAR::LofarConvolutionFunction *> itsConvolutionFunctions;
//    Vector<casa::PagedImage *> itsConvolutionFunctionsImages;

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
    ModelImageOptions itsOptions;                   // struct containing all options
    double itsGriddingTime;                         // variable to keep track of timing of steps

    //-----------------------------------------------------------------  
    // Image functions
    void fftImage();                               // perform FFT on image
    void getImagePhaseDirection();                 // get the phase direction of the image

    //-----------------------------------------------------------------  
    // FTmachine functions
    void initializeToVis( ImageInterface<Complex>& iimage);

    // Gridder functions
//    LofarVisResampler visResamplers_p;
//    CountedPtr<LOFAR::VisibilityResamplerBase> visResampler_p;         // visibility resampler
    LOFAR::LofarVisResampler visResamplers_p;
    LOFAR::ConvolveGridder<casa::Double, casa::Complex>* gridder;      // Gridder
    CountedPtr<casa::Lattice<casa::Complex> > arrayLattice;            // Array lattice
    // Lattice. For non-tiled gridding, this will point to arrayLattice,
    //  whereas for tiled gridding, this points to the image (TODO: tidy up, we don't tile at all)
    CountedPtr<Lattice<Complex> > lattice;
    IPosition centerLoc, offsetLoc;         // Useful IPositions
    Vector<Double> uvScale, uvOffset;       // Image Scaling and offset

    // Arrays for non-tiled gridding (one per thread).
    Array<Complex>  itsGriddedData;
    Array<DComplex> itsGriddedData2;
    IPosition padded_shape;                // Shape of the padded image

    Bool useDoubleGrid_p;                  // use double precision grid
    Bool canComputeResiduals_p;
    Bool noPadding_p;                      //force no padding
    Float padding_p;                       // Padding in FFT

    Vector<Int> chanMap_p, polMap_p;       // channel map, polarization map
    Vector<Int> ConjCFMap_p, CFMap_p;
    Int nx,ny;                             // where else does this come from?
    Int nchan, npol;                       // where else does this come from?
    
    CFStore cfs_p, cfwts_p;
};
