//# FTMachineIDG.cc: Gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2014
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
//# $Id: FTMachineIDG.cc 28512 2014-03-05 01:07:53Z vdtol $

#include <lofar_config.h>

#include <AWImager2/FTMachineIDG.h>
#include <AWImager2/VBStore.h>
#include <AWImager2/CFStore.h>
#include <AWImager2/VisBuffer.h>
#include <AWImager2/ConvolutionFunction.h>
#include <AWImager2/VisResamplerMatrixWB.h>
#include <AWImager2/ScopedTimer.h>
#include <Common/OpenMP.h>
#include <casacore/lattices/LatticeMath/LatticeFFT.h>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/concepts.hpp>  // sink
#include "helper_functions.tcc"

#define MAX_SUPPORT_FREQ 16
#define MAX_SUPPORT_TIME 16
#define MAX_SUPPORT_W 16
#define MAX_SUPPORT_A 1
#define SUPPORT_SPHEROIDAL 7
  

using namespace casa;

const String LOFAR::LofarFT::FTMachineIDG::theirName("FTMachineIDG");

namespace
{
  bool dummy = LOFAR::LofarFT::FTMachineFactory::instance().
    registerClass<LOFAR::LofarFT::FTMachineIDG>(
      LOFAR::LofarFT::FTMachineIDG::theirName);
}

namespace LOFAR {
namespace LofarFT {

class LogDebugSink : public boost::iostreams::sink {
public:
  
    std::streamsize write(const char* s, std::streamsize n)
    {
      int nn = n;
      if (s[nn-1] == '\n') nn--;
      LOG_DEBUG_STR(std::string(s,nn));
      return n;
    }
};
  
template <typename SinkClass> class  ScopedRedirect
{
public:
  
  ScopedRedirect(std::ostream &source) :
    itsSource(source)
  {
    itsLogDebugStream.open(itsSink);
    itsOldBuffer = clog.rdbuf(itsLogDebugStream.rdbuf());
  }
  
  ~ScopedRedirect() 
  {
    clog.rdbuf(itsOldBuffer);
  }
  
private:
  std::ostream &itsSource;
  SinkClass itsSink;
  boost::iostreams::stream<SinkClass> itsLogDebugStream;
  std::streambuf * itsOldBuffer;
};

bool compare_size(vector<int> a, vector<int> b) { return a.size()<b.size(); }
  
  
Matrix<Float> FTMachineIDG::getAveragePB()
{
  if (itsAveragePB.empty()) 
  {
    itsAveragePB.resize(itsNX, itsNY);
    itsAveragePB = 1.0;
    // Make it persistent.
    LOG_DEBUG_STR("Storing average beam...");
    
    //DEBUG store does not work anymore, somthing to do with coordinates and wcs
//     store(itsAveragePB, itsImageName + ".avgpb");
    
    LOG_DEBUG_STR("done.");
  }
  return itsAveragePB;
}
  
// Matrix<Float> FTMachineIDG::getSpheroidal()
// {
//   if (itsSpheroidal.empty()) 
//   {
//     itsSpheroidal.resize(itsNX, itsNY);
//     itsAveragePB = 1.0;
//   }
//   return itsSpheroidal;
// }

  // Partition the visbuffer according to baseline number and time
  
  
struct Chunk
{
  int start;
  int end;
  double time;
  std::vector<float> support;
  std::vector<float> w_min;
  std::vector<float> w_max;
  std::vector<bool> has_been_processed;
  casa::Matrix<casa::Float> sum_weight;
};

struct VisibilityMap
{
  vector<Chunk> chunks;
  casa::Vector<casa::uInt> baseline_index_map;
  int blocksize;
  float min_w_max;
};

VisibilityMap make_mapping(
  const VisBuffer& vb, 
  double dtime,
  vector<vector<int> > channel_map,
  casa::Vector<casa::Double> uvscale)
{
  VisibilityMap v;
  
  int N_chan_group = channel_map.size();
  
  // Determine the baselines in the VisBuffer.
  const Vector<Int>& ant1 = vb.antenna1();
  const Vector<Int>& ant2 = vb.antenna2();
  const Vector<Double>& times = vb.timeCentroid();
  const Vector<RigidVector<Double, 3> > &uvw = vb.uvw()  ;
  
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  Vector<uInt> &blIndex = v.baseline_index_map;
  GenSortIndirect<Int>::sort (blIndex, blnr);
  
  // Now determine nr of unique baselines and their start index.
  Int  lastbl     = -1;
  Int  lastIndex  = 0;
  bool allFlagged = true;
  const Vector<Bool>& flagRow = vb.flagRow();
  
  Double time0 = times[0];
  float max_time_frequency_support = 0.0;
  for (uint i=0; i<=blnr.size(); ++i) 
  {
    if ((i == blnr.size()) || (blnr[blIndex[i]] != lastbl) || ((times[blIndex[i]] - time0) > dtime))
    {
      // New baseline. Write the previous end index if applicable.
      if (!allFlagged) 
      {
        Chunk chunk;
        chunk.start = lastIndex;
        chunk.end = i-1;
        
        Int start_idx = blIndex[chunk.start];
        Int end_idx = blIndex[chunk.end];
        
        chunk.time = 0.5 * (times[start_idx] + times[end_idx]);
        chunk.support.reserve(N_chan_group);
        chunk.w_min.reserve(N_chan_group);
        chunk.w_max.reserve(N_chan_group);
        chunk.has_been_processed.resize(N_chan_group, false);
        // iterate over channel groups
        for(std::vector<vector<int> >::iterator chan_group = channel_map.begin(); chan_group != channel_map.end(); ++chan_group)
        {
          // in pixels, so scale with uvscale
          float min_u = float(uvw(start_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(0);
          min_u = min(min_u, float(uvw(start_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(0));
          min_u = min(min_u, float(uvw(end_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(0));
          min_u = min(min_u, float(uvw(end_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(0));
      
          // in pixels, so scale with uvscale
          float min_v = float(uvw(start_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(1);
          min_v = min(min_v, float(uvw(start_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(1));
          min_v = min(min_v, float(uvw(end_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(1));
          min_v = min(min_v, float(uvw(end_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(1));
          
          // in wavelengths, so no scaling with uvscale
          float min_w = float(uvw(start_idx)(2))/(float)casa::C::c*float(vb.frequency()(chan_group->front()));
          min_w = min(min_w, float(uvw(end_idx)(2))/(float)casa::C::c*float(vb.frequency()(chan_group->front())));
          
          float max_u = float(uvw(start_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(0);
          max_u = max(max_u, float(uvw(start_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(0));
          max_u = max(max_u, float(uvw(end_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(0));
          max_u = max(max_u, float(uvw(end_idx)(0))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(0));
      
          float max_v = float(uvw(start_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(1);
          max_v = max(max_v, float(uvw(start_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(1));
          max_v = max(max_v, float(uvw(end_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->front()))*(float)uvscale(1));
          max_v = max(max_v, float(uvw(end_idx)(1))/(float)casa::C::c*float(vb.frequency()(chan_group->back()))*(float)uvscale(1));
          
          float max_w = float(uvw(start_idx)(2))/(float)casa::C::c*float(vb.frequency()(chan_group->back()));
          max_w = max(max_w, float(uvw(end_idx)(2))/(float)casa::C::c*float(vb.frequency()(chan_group->back())));
          
          
//           LOG_DEBUG_STR("max_u-min_u: " << (max_u-min_u));
//           LOG_DEBUG_STR("max_v-min_v: " << (max_v-min_v));
          float time_frequency_support = max(max_u-min_u, max_v-min_v);
//           LOG_DEBUG_STR("time_frequency_support: " << time_frequency_support);

            
          chunk.support.push_back(time_frequency_support);
          chunk.w_min.push_back(min_w);
          chunk.w_max.push_back(max_w);
          
          max_time_frequency_support = max(max_time_frequency_support, time_frequency_support);
//           LOG_DEBUG_STR("max_time_frequency_support: " << max_time_frequency_support);
//           double w_support = abs(max(uvw(2, idx_start),uvw(2, idx_end))/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0)*itsUVScale(0));
        
          // find time and frequency support used
          // assign remaining support to w
          // compute w_min and w_max plane this chunk can be gridded to
        }
        
        v.chunks.push_back(chunk);
        
        allFlagged = true;
      }
      
      if (i == blnr.size()) break;
      
      lastbl = blnr[blIndex[i]];
      lastIndex = i;
      time0 = times[blIndex[lastIndex]];
    }
    
    // Test if the row is flagged.
    if (! flagRow[blIndex[i]]) {
      allFlagged = false;
    }
  }
  LOG_DEBUG_STR("Max time frequency support: " << max_time_frequency_support);
  v.blocksize = ceil((max_time_frequency_support + SUPPORT_SPHEROIDAL + MAX_SUPPORT_A + MAX_SUPPORT_W) / 8 ) * 8;
  LOG_DEBUG_STR("blocksize: " << v.blocksize);

  // iterate over all chunks and set w_min and w_max
  v.min_w_max = std::numeric_limits<float>::infinity();
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    for (int i = 0; i<N_chan_group; ++i)
    {
      float w_support = v.blocksize - SUPPORT_SPHEROIDAL - MAX_SUPPORT_A - chunk->support[i];
      //TODO find correct value of factor
      float factor = 2.0;
      float w_range = w_support/(uvscale(0)*uvscale(0))/factor;
      float w_max = chunk->w_min[i] + w_range;
      float w_min = chunk->w_max[i] - w_range;
      chunk->w_min[i] = w_min;
      chunk->w_max[i] = w_max;
      v.min_w_max = min(v.min_w_max, w_max);
    }
  }
  
  return v;  
}

 

FTMachineIDG::FTMachineIDG(
  const MeasurementSet& ms,
  const ParameterSet& parset)
  : FTMachine( ms, parset),
    itsNThread(OpenMP::maxThreads()),
    itsProxy(0)
{
  itsMachineName = theirName;
  
  // TODO use popen() to capture output stream
  // TODO check for VML library separately
  
//   if (!(system("exec &>/dev/null; icpc --version ")))
  if (!(system("exec &>/dev/null; icpc --version  >/dev/null 2>&1")))
  {
    itsCompiler = "icpc";
    itsCompilerFlags = "-O3 -xAVX -openmp -mkl -lmkl_vml_avx -lmkl_avx -DUSE_VML=1 -g";
  }
  else if (!(system("g++ --version >/dev/null 2>&1")))
  {
    itsCompiler = "g++";
    itsCompilerFlags = "-fopenmp -march=native -O3 -ffast-math -DUSE_VML=0";
  }
  else
  {
    throw(AipsError("No compiler found."));
  }
  
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedDataDomain = IMAGE;
  double msRefFreq=0; //TODO: put some useful reference frequency here
  itsRefFreq=parset.getDouble("image.refFreq",msRefFreq),
  itsTimeWindow=parset.getDouble("gridding.timewindow",300);
}

FTMachineIDG::~FTMachineIDG()
{
}

  //----------------------------------------------------------------------
FTMachineIDG& FTMachineIDG::operator=(const FTMachineIDG& other)
{
  if(this!=&other) 
  {
    //Do the base parameters
    FTMachine::operator=(other);

    itsLattice = 0;
    itsNThread = other.itsNThread;
    itsGriddedData.resize (itsNThread);
    itsGriddedData2.resize (itsNThread);
    itsSumPB.resize (itsNThread);
    itsSumCFWeight.resize (itsNThread);
    itsSumWeight.resize (itsNThread);
    itsProxy = other.itsProxy;
  }
  return *this;
}

//----------------------------------------------------------------------
FTMachineIDG::FTMachineIDG(const FTMachineIDG& other) : 
  FTMachine(other)
{
  operator=(other);
}

//----------------------------------------------------------------------
FTMachineIDG* FTMachineIDG::clone() const
{
  FTMachineIDG* newftm = new FTMachineIDG(*this);
  return newftm;
}

//----------------------------------------------------------------------


void FTMachineIDG::initialize_model_grids(Bool normalize_model)
{
  // Create model grids but do not initialize here
  // because that will happen in the loop over w-planes
    
  uInt nmodels = itsModelImages.nelements();
  
  itsNGrid = nmodels;

  itsComplexModelImages.resize(nmodels);
  itsModelGrids.resize(nmodels);

  CoordinateSystem coords = itsModelImages[0]->coordinates();

  IPosition gridShape(4, itsPaddedNX, itsPaddedNY, itsNPol, itsNChan);
  
  StokesCoordinate stokes_coordinate = get_stokes_coordinates();
  
  Int stokes_index = coords.findCoordinate(Coordinate::STOKES);
  coords.replaceCoordinate(stokes_coordinate, stokes_index);
  
  for (uInt model = 0; model<nmodels; model++)
  {
    // create complex model images
    // Force in memory, allow 1e6 MB memory usage
    itsComplexModelImages[model] = new TempImage<Complex> (gridShape, coords, 1e6); 
    itsModelGrids[model].reference(itsComplexModelImages[model]->get());

  }
}

void FTMachineIDG::residual(VisBuffer& vb, casa::Int row, casa::FTMachine::Type type) 
{
   getput(vb, row, True, True, False, type);
}


void FTMachineIDG::getput(
  VisBuffer& vb, 
  casa::Int row, 
  casa::Bool doget,
  casa::Bool doput,    
  casa::Bool dopsf,
  casa::FTMachine::Type type)
{
  // within this scope redirect messages written to clog to LofarLogger with loglevel DEBUG
  ScopedRedirect<LogDebugSink> scoped_redirect(std::clog); 
  
  // Match data channels to images channels
  // chan_map is filled by match_channel with a mapping of the data channels
  // to the image channels. This mapping changes over time because of changing
  // Doppler shift due to earth rotation around its axis and the sun.
  // The channel mapping is determined for the first time sample in vb
  // It is assumed that within the vb the change in Dopplershift is small .
  
  // Wide band imager has multiple terms but only a single channel
  // All visibility channels map to image channel 0
  Vector<Int> chan_map(vb.frequency().size(), 0);;
  
  Vector<Double> lsr_frequency;
  
  Int spwid = 0;
  Bool convert = True;
  vb.lsrFrequency(spwid, lsr_frequency, convert);
  
  if(dopsf) {type=FTMachine::PSF;}
  
  Cube<Complex> data;
  
  switch(type) 
  {
  case FTMachine::MODEL:
    data.reference(vb.modelVisCube());
    break;
  case FTMachine::CORRECTED:
    data.reference(vb.correctedVisCube());
    break;
  case FTMachine::OBSERVED:
    data.reference(vb.visCube());
  }
  
  Cube<Bool> flag(vb.flag());
  
  Cube<Float> imagingWeightCube(vb.imagingWeightCube());
  
  Int startRow, endRow, nRow;
  if (row==-1) 
  {
    nRow=vb.nRow(); 
    startRow=0; endRow=nRow-1; 
  }
  else
  { 
    nRow=1; 
    startRow=row; 
    endRow=row; 
  }

  Vector<RigidVector<Double, 3> > uvw_ = vb.uvw();
  
  
  Vector<Int> antenna1 = vb.antenna1();
  Vector<Int> antenna2 = vb.antenna2();
  
  // find negative w, swap antennas and conjugate data
  Vector<Bool> swapped(vb.nRow(), False);
  
  if (data.empty() || !doput)
  {
//     #pragma omp parallel
    for(int i = 0; i < vb.nRow(); i++)
    {
      if (uvw_(i)(2) < 0)
      {
        uvw_(i) = -uvw_(i);
        swap(antenna1(i), antenna2(i));
        swapped(i) = True;
      }
    }
  }
  else
  {
//     #pragma omp parallel
    for(int i = 0; i < vb.nRow(); i++)
    {
      if (uvw_(i)(2) < 0)
      {
        uvw_(i) = -uvw_(i);
        swap(antenna1(i), antenna2(i));
        //TODO transpose
        data[i] = conj(data[i]);
        swapped(i) = True;
      }
    }
  }
  
  // find the maximum uv baseline length on grid
  float max_u = 0;
  float max_u_;
  float max_v = 0;
  float max_v_;
  float max_l2 = 0;
  float max_l2_;
  
  
  #pragma omp parallel private(max_u_, max_v_, max_l2_)
  {
    max_u_ = 0;
    max_v_ = 0;
    max_l2_ = 0;
    #pragma omp for nowait
    for(int i = 0; i < vb.nRow(); i++)
    {
      float u = abs(uvw_(i)(0));
      float v = abs(uvw_(i)(1));
      float w = abs(uvw_(i)(2));
      float l2 = u*u + v*v + w*w;
      max_u_ = max(max_u_, u);
      max_v_ = max(max_v_, v);
      max_l2_ = max(max_l2_, l2);
    }
    #pragma omp critical
    {
      max_u = max(max_u, max_u_);
      max_v = max(max_v_, max_v_);
      max_l2 = max(max_l2, max_l2_);
    }
  }
  
  float wavelength = casa::C::c / vb.frequency()(0);
  float max_u_grid = itsPaddedNX/(2 * itsUVScale(0)) * wavelength;
  float max_v_grid = itsPaddedNY/(2 * itsUVScale(1)) * wavelength;
  
  max_u = min(max_u, max_u_grid);
  max_v = min(max_v, max_v_grid);
  
  // find the maximum frequency window to stay within maximum support
  float max_bw_u = MAX_SUPPORT_FREQ / (max_u / casa::C::c * itsUVScale(0));
  float max_bw_v = MAX_SUPPORT_FREQ / (max_v / casa::C::c * itsUVScale(1));
  float max_bw = min(max_bw_u, max_bw_v);
  
    
  // create a channel mapping
  // count number of channels per group
  vector<vector<int> > channel_map(1, vector<int>(1,0));
  for(int i = 1; i<vb.nChannel(); i++)
  {
    if ((vb.frequency()(i) - vb.frequency()(channel_map.back().front())) < max_bw)
    {
      channel_map.back().push_back(i);
    }
    else
    {
      channel_map.push_back(vector<int>(1, i));
    }
  }
  
  int N_chan = max_element(channel_map.begin(), channel_map.end(), compare_size)->size();
//   int N_chan_group = channel_map.size();
  
  LOG_DEBUG_STR("N_chan: " << N_chan);
  
  
  
  // find the maximum timewindow to stay within maximum support
  // max time = max support / (max baseline (pixels) * earth_rotation_rate (rad/s))
  // min(itsTimeWindow, max time)
  
  float min_wavelength = casa::C::c / vb.frequency()(vb.nChannel()-1);
  float max_l = sqrt(max_l2)/min_wavelength * abs(itsUVScale(0));
  float earth_rotation_rate = 7.2921150e-5; 
  float max_time = MAX_SUPPORT_TIME / (max_l * earth_rotation_rate);
  
  LOG_DEBUG_STR("min_wavelength: " << min_wavelength);
  LOG_DEBUG_STR("max_l2: " << max_l2);
  LOG_DEBUG_STR("max_l: " << max_l);
  LOG_DEBUG_STR("Max time: " << max_time);  // W stack 
  
  float timewindow = min(itsTimeWindow, max_time);
  
  VisibilityMap v = make_mapping(vb, timewindow, channel_map, itsUVScale);
  
  LOG_DEBUG_STR("min_w_max: " << v.min_w_max);
  
  Int N_time = 0;
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    N_time = max(N_time, chunk->end - chunk->start + 1);
  }

  Int N_chunks = 10000;
  Int N_stations = 1 + max(max(vb.antenna1()), max(vb.antenna2()));
  Int blocksize = v.blocksize;

  LOG_DEBUG_STR("Creating proxy...");
  itsProxy = new Xeon (itsCompiler.c_str(), itsCompilerFlags.c_str(),  N_stations, N_chunks, N_time, N_chan,
      itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  LOG_DEBUG_STR("done.");
  
  Array<Complex> visibilities(IPosition(4, itsNPol, N_chan, N_time, N_chunks), Complex(0.0,0.0));
  Matrix<Int> idx1(N_time, N_chunks, -1); 
  Cube<Float> uvw(3, N_time, N_chunks, 0.0); 
  Matrix<Float> offsets(3, N_chunks, 0.0);
  Matrix<Int> coordinates(2, N_chunks, 0);
  Vector<Float> wavenumbers(N_chan, 0.0);
  Array<Complex> aterm(IPosition(4, blocksize, blocksize, itsNPol, N_stations), 0.0);
  aterm(Slicer(IPosition(4,0,0,0,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  aterm(Slicer(IPosition(4,0,0,3,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  Matrix<Float> spheroidal(blocksize,blocksize, 1.0);
  Array<Complex> subgrids(IPosition(4, blocksize, blocksize, itsNPol, N_chunks), Complex(0.0, 0.0));
  
  Matrix<Int> baselines(2, N_chunks, 0);
  
  // spheroidal
  taper(spheroidal);
  
  float min_w_max = v.min_w_max;
  
  //DEBUG
//   min_w_max = 0.0;
  
  // iterate over w-planes
  while (min_w_max < std::numeric_limits<float>::infinity())
  {
    double w_offset = min_w_max;
    LOG_DEBUG_STR("W plane offset: " << w_offset);
    
    if (doget)
    {
      itsComplexModelImages[0]->set(Complex(0.0));

//             fill complex sub image from model image
      IPosition blc(
        4, 
        (itsPaddedNX - itsModelImages[0]->shape()(0) + (itsPaddedNX % 2 == 0)) / 2,
        (itsPaddedNY - itsModelImages[0]->shape()(1) + (itsPaddedNY % 2 == 0)) / 2,
        0, 
        0);
      IPosition shape(4, itsNX, itsNY, itsNPol, itsNChan);
      SubImage<Complex> complex_model_subimage(*itsComplexModelImages[0], Slicer(blc, shape), True);
      
      // convert float IQUV model image to complex image
      StokesImageUtil::From(complex_model_subimage, *itsModelImages[0]);
      
      normalize(complex_model_subimage, itsNormalizeModel, True);

      // Apply W term in image domain
      Array<Complex> complex_model_subimage_data;
      complex_model_subimage.get(complex_model_subimage_data);
      itsConvFunc->applyWterm(complex_model_subimage_data, -w_offset);
      
      // transform to uv domain
      
      itsModelGrids[0].reference(itsComplexModelImages[0]->get());
      
      FFTCMatrix f;
      for(Int pol=0; pol<itsNPol; ++pol)
      {
        Complex* ptr = itsModelGrids[0].data() + pol*itsPaddedNX*itsPaddedNY;
        f.forward(itsPaddedNX, ptr, OpenMP::maxThreads());
      }
    }
    
    min_w_max = std::numeric_limits<float>::infinity();
    casa::Array<casa::Complex>  w_plane_grid;
    if (doput)
    {
      w_plane_grid = casa::Array<casa::Complex>(itsGriddedData[0].shape(), casa::Complex(0));
    }
  
    //iterate over channel groups
    
    int chan_group_idx = 0;
    std::vector<vector<int> >::iterator chan_group;
    for(chan_group = channel_map.begin(); chan_group != channel_map.end(); ++chan_group, ++chan_group_idx)
    {
      // set wavenumbers for this chan_group
      int chan;
      vector<int>::iterator chan_it;
      for(chan_it = chan_group->begin(), chan=0; chan_it != chan_group->end(); ++chan_it, ++chan)
      {
        double wavelength = casa::C::c / vb.frequency()(*chan_it);
        wavenumbers(chan) = 2 * casa::C::pi / wavelength;
      }

      std::vector<Chunk>::iterator chunk = v.chunks.begin();
      while (chunk != v.chunks.end())
      {
        for(int i = 0; ((i < N_chunks) && (chunk != v.chunks.end())); ++chunk)
        {
          if (chunk->has_been_processed[chan_group_idx]) continue;
          if (chunk->w_min[chan_group_idx] > w_offset) 
          {
            min_w_max = min(min_w_max, chunk->w_max[chan_group_idx]);
            continue;
          }
          chunk->has_been_processed[chan_group_idx] = true;
          
          int idx_start = v.baseline_index_map[chunk->start];
          int idx_end = v.baseline_index_map[chunk->end];
          
          double f = (vb.frequency()(chan_group->front()) + vb.frequency()(chan_group->back())) / 2;
          double wavelength = casa::C::c / f;
          
          int center_u = floor((vb.uvw()(idx_start)(0) + vb.uvw()(idx_end)(0))/2/wavelength * itsUVScale(0));
          int center_v = floor((vb.uvw()(idx_start)(1) + vb.uvw()(idx_end)(1))/2/wavelength * itsUVScale(1));
          
          coordinates(0,i) = center_u + itsUVOffset(0) - blocksize/2;
          coordinates(1,i) = center_v + itsUVOffset(1) - blocksize/2;

          //check whether block falls within grid
          //TODO use extra margin when w_offset>0
          if ((coordinates(0,i) < 0) ||
            (coordinates(1,i) < 0) ||
            ((coordinates(0,i)+blocksize) >= itsPaddedNX) ||
            ((coordinates(1,i)+blocksize) >= itsPaddedNY))
          {
            continue;
          }
          
          offsets (0, i) = double(center_u) / itsUVScale(0) * 2 * casa::C::pi;
          offsets (1, i) = double(center_v) / itsUVScale(1) * 2 * casa::C::pi;
          offsets (2, i) = w_offset * 2 * casa::C::pi;

          for (int j = chunk->start; j<=chunk->end; j++)
          {
            int k = j - chunk->start;
            
            int idx = v.baseline_index_map[j];

            uvw (0, k, i) = vb.uvw()(idx)(0);
            uvw (1, k, i) = vb.uvw()(idx)(1);
            uvw (2, k, i) = vb.uvw()(idx)(2);
            idx1(k,i) = idx;
          }
          
          // baselines
          int idx = v.baseline_index_map[chunk->start];
          int ant1 = vb.antenna1()[idx];
          int ant2 = vb.antenna2()[idx];
          baselines(0,i) = ant1;
          baselines(1,i) = ant2;

          if (doput)
          {
            // copy data to buffer
            for (int j = chunk->start; j<=chunk->end; j++)
            {
              int k = j - chunk->start;
              int idx = v.baseline_index_map[j];
              if (dopsf)
              {
                for(chan_it = chan_group->begin(), chan=0; chan_it != chan_group->end(); ++chan_it, ++chan)
                {
                  for (int idx_pol = 0; idx_pol < itsNPol; idx_pol++)
                  {
                    if (vb.flagCube()(idx_pol, *chan_it, idx))
                    {
                      visibilities( IPosition(4, idx_pol, chan, k, i)) = Complex(0);
                    }
                    else
                    {
                      visibilities( IPosition(4, idx_pol, chan, k, i)) = -((idx_pol==0) ||( idx_pol==3));
                      itsSumWeight[0](idx_pol,0) += imagingWeightCube(idx_pol, *chan_it, idx)*blocksize*blocksize*2;
                    }
                  }
                }
              }
              else
              {
                for(chan_it = chan_group->begin(), chan=0; chan_it != chan_group->end(); ++chan_it, ++chan)
                {
                  for (int idx_pol = 0; idx_pol < itsNPol; idx_pol++)
                  {
                    if (vb.flagCube()(idx_pol, *chan_it, idx))
                    {
                      visibilities( IPosition(4, idx_pol, chan, k, i)) = Complex(0);
                    }
                    else
                    {
                      visibilities( IPosition(4, idx_pol, chan, k, i)) = -data(idx_pol, *chan_it, idx);
                      itsSumWeight[0](idx_pol,0) += imagingWeightCube(idx_pol, *chan_it, idx)*blocksize*blocksize*2;
                    }
                  }
                }
              }
            }
          }
          if (doget)
          {
            #pragma omp parallel for num_threads(4)
            for(int pol=0; pol<itsNPol; pol++) 
            {
              for(int j=0; j<blocksize; j++) 
              {
                for(int k=0; k<blocksize; k++) 
                {
                  subgrids(IPosition(4, j, k, pol, i)) = itsModelGrids[0](IPosition(4,j+coordinates(0,i),k+coordinates(1,i),pol,0));
                }
              }
            }
          }
          i++;
        }
        
        if (doget)
        {
          LOG_DEBUG_STR("degridder...");
          itsProxy->degridder(
            N_chunks,
            offsets.data(), //      void    *offset,
            wavenumbers.data(), //      void    *wavenumbers,
            aterm.data(),                 
            baselines.data(),
            visibilities.data(), //      void    *visibilities
            uvw.data(), //      void    *uvw,
            spheroidal.data(),
            subgrids.data()); //      void    *uvgrid,
          LOG_DEBUG_STR("done.");
        }
        // TODO make docommit a parameter of getput
        Bool docommit = !data.empty();
        if (docommit)
        {
          // move predicted data to data column

          for(int j = 0; j<N_chunks; j++)
          {
            for(int k = 0; k<N_time; k++)
            {
              int idx = idx1(k,j);
              if (idx > -1)
              {
                for(int pol = 0; pol < itsNPol; pol++)
                {
                  for(chan_it = chan_group->begin(), chan=0; chan_it != chan_group->end(); ++chan_it, ++chan)
                  {
                    if (swapped(idx))
                    {
                      data(pol, *chan_it, idx) = conj(visibilities(IPosition(4,pol, chan, k,j)));
                    }
                    else
                    {
                      data(pol, *chan_it, idx) = visibilities(IPosition(4,pol, chan, k,j));
                    }
                  }
                }
              }
            }
          }
        } 
        
        if (doput)
        {
          for(int j = 0; j<N_chunks; j++)
          {
            for(int k = 0; k<N_time; k++)
            {
              int idx = idx1(k,j);
              if (idx > -1)
              {
                for(int pol = 0; pol < itsNPol; pol++)
                {
                  for(chan_it = chan_group->begin(), chan=0; chan_it != chan_group->end(); ++chan_it, ++chan)
                  {
                    visibilities(IPosition(4,pol, chan, k,j)) *= 
                      -imagingWeightCube(pol, *chan_it, idx) * 
                      (!vb.flagCube()(pol, *chan_it, idx));
                  }
                }
              }
            }
          }
          
          LOG_DEBUG_STR("gridder...");
          itsProxy->gridder(
            N_chunks,
            visibilities.data(), //      void    *visibilities
            uvw.data(), //      void    *uvw,
            offsets.data(), //      void    *offset,
            wavenumbers.data(), //      void    *wavenumbers,
            aterm.data(),                 
            spheroidal.data(),
            baselines.data(),
            subgrids.data() //      void    *uvgrid,
          );
          LOG_DEBUG_STR("done.");
          
          LOG_DEBUG_STR("adder...");
          itsProxy->adder(N_chunks, coordinates.data(), subgrids.data(), w_plane_grid.data());
          LOG_DEBUG_STR("done.");
        }
        visibilities = Complex(0);
        subgrids = Complex(0);
        idx1 = -1;      
      }
    }
    if (doput)
    {
      // transform to image domain
      LOG_DEBUG_STR("W plane fft...");
      {
        ScopedTimer t("W plane fft");
        FFTCMatrix fft;
        for (int ii = 0; ii<w_plane_grid.shape()(3); ii++)
        {
          for (int jj = 0; jj<w_plane_grid.shape()(2); jj++)
          {
            Matrix<Complex> im(w_plane_grid[ii][jj]);
            fft.forward (im.nrow(), im.data(), OpenMP::maxThreads());
          }
        }
      }
      LOG_DEBUG_STR("done.");
      
      // Apply W term in image domain
      LOG_DEBUG_STR("W plane apply...");
      {
        ScopedTimer t("W plane apply");
        itsConvFunc->applyWterm(w_plane_grid, -w_offset);
      }
      LOG_DEBUG_STR("done.");
      
      // Add image to master grid
      itsGriddedData[0] += w_plane_grid;
    }
  }
}

void FTMachineIDG::put(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type) 
{
  getput(*const_cast<VisBuffer*>(&vb), row, False, True, dopsf, type);
}

void FTMachineIDG::get(VisBuffer& vb, Int row) 
{
  getput(vb, row, True, False, False, FTMachine::MODEL);
}

// Degrid
// void FTMachineIDG::get(VisBuffer& vb, Int row)
// {
//   // within this scope redirect messages written to clog to LofarLogger with loglevel DEBUG
//   ScopedRedirect<LogDebugSink> scoped_redirect(std::clog); 
//   
// //   gridOk(itsGridder->cSupport()(0));
//   // If row is -1 then we pass through all rows
//   Int startRow, endRow, nRow;
//   if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;}
//   else         { nRow=1; startRow=row; endRow=row; }
// 
//   Vector<Int> chan_map(vb.frequency().size(), 0);
//   
//   Vector<Double> lsr_frequency;
//   
//   Int spwid = 0;
//   Bool convert = True;
//   vb.lsrFrequency(spwid, lsr_frequency, convert);
// 
//   Cube<Complex> data(vb.modelVisCube());
//   
//   VisibilityMap v = make_mapping(vb, itsTimeWindow, std::vector<std::vector<int> >(), itsUVScale);
//   Int N_time = 0;
//   for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
//   {
//     N_time = max(N_time, chunk->end - chunk->start + 1);
//   }
// 
//   Int N_chan = vb.nChannel();
//   
//   Int N_chunks = 100000;
//   Int N_stations = 1 + max(max(vb.antenna1()), max(vb.antenna2()));
//   Int blocksize = 48;
// 
//   LOG_DEBUG_STR("Creating proxy...");
//   itsProxy = new Xeon (itsCompiler.c_str(), itsCompilerFlags.c_str(),  N_stations, N_chunks, N_time, N_chan,
//       itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
//   LOG_DEBUG_STR("done.");
// 
//   Array<Complex> visibilities(IPosition(4, itsNPol, N_chan, N_time, N_chunks), Complex(0.0,0.0));
//   Cube<Float> uvw(3, N_time, N_chunks, 0.0); 
//   Matrix<Int> idx1(N_time, N_chunks, -1); 
//   Matrix<Float> offsets(3, N_chunks, 0.0);
//   Matrix<Int> coordinates(2, N_chunks, 0.0);
//   Vector<Float> wavenumbers(N_chan, 0.0);
//   Array<Complex> aterm(IPosition(4, blocksize, blocksize, itsNPol, N_stations), 0.0);
//   aterm(Slicer(IPosition(4,0,0,0,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
//   aterm(Slicer(IPosition(4,0,0,3,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
// 
//   Matrix<Float> spheroidal(blocksize,blocksize, 1.0);
//   Array<Complex> subgrids(IPosition(4, blocksize, blocksize, itsNPol, N_chunks), Complex(0.0, 0.0));
//   
//   Cube<Complex> grid(itsNPol, itsPaddedNX, itsPaddedNY, Complex(0.0, 0.0));
//   
//   Matrix<Int> baselines(2, N_chunks, 0);
//   
//   // spheroidal
//   taper(spheroidal);
// 
//   // wavenumbers
//   for(int i = 0; i<N_chan; i++)
//   {
//     double wavelength = casa::C::c / vb.frequency()(i);
//     wavenumbers(i) = 2 * casa::C::pi / wavelength;
//   }
// 
//   int i = 0;    
//   for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
//   {
//     for (int j = chunk->start; j<=chunk->end; j++)
//     {
//       int k = j - chunk->start;
//       
//       int idx = v.baseline_index_map[j];
// 
//       uvw (0, k, i) = vb.uvw()(idx)(0);
//       uvw (1, k, i) = vb.uvw()(idx)(1);
//       uvw (2, k, i) = vb.uvw()(idx)(2);
//       
//       idx1(k,i) = idx;
//       
//     }
//     
//     int idx_start = v.baseline_index_map[chunk->start];
//     int idx_end = v.baseline_index_map[chunk->end];
//     
//     double wavelength = casa::C::c / vb.frequency()(N_chan/2);
//     
//     offsets (0, i) = double(floor((vb.uvw()(idx_start)(0) + vb.uvw()(idx_end)(0))/2/wavelength * itsUVScale(0))) / itsUVScale(0) * 2 * casa::C::pi;
//     offsets (1, i) = double(floor((vb.uvw()(idx_start)(1) + vb.uvw()(idx_end)(1))/2/wavelength * itsUVScale(1))) / itsUVScale(1) * 2 * casa::C::pi;
//     offsets (2, i) = 0;
//     
// //     double w_support = abs(max(vb.uvw()(idx_start)(2),vb.uvw()(idx_start)(2))/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0)*itsUVScale(0));
// //     
// //     double min_u = vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
// //     min_u = min(min_u, vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// //     min_u = min(min_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
// //     min_u = min(min_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// // 
// //     double min_v = vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
// //     min_v = min(min_v, vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// //     min_v = min(min_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
// //     min_v = min(min_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// //     
// //     double max_u = vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
// //     max_u = max(max_u, vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// //     max_u = max(max_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
// //     max_u = max(max_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// // 
// //     double max_v = vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
// //     max_v = max(max_v, vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// //     max_v = max(max_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
// //     max_v = max(max_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// //     
// //     double uv_support = max(abs(max_u-min_u), abs(max_v-min_v));
// //     
// //     if ((uv_support + w_support) > 30)
// //     {
// //       cout << "w_support: " << w_support << endl;
// //       cout << "uv_support: " << uv_support << endl;
// //     }
//     
//     coordinates(0,i) = round(offsets(0,i)/2/casa::C::pi*itsUVScale(0)) + itsUVOffset(0) - blocksize/2;
//     coordinates(1,i) = round(offsets(1,i)/2/casa::C::pi*itsUVScale(1)) + itsUVOffset(1) - blocksize/2;
//     
//     // baselines
//     int idx = v.baseline_index_map[chunk->start];
//     int ant1 = vb.antenna1()[idx];
//     int ant2 = vb.antenna2()[idx];
//     baselines(0,i) = ant1;
//     baselines(1,i) = ant2;
//     
//     if ((coordinates(0,i) < 0) ||
//       (coordinates(1,i) < 0) ||
//       ((coordinates(0,i)+blocksize) >= itsPaddedNX) ||
//       ((coordinates(1,i)+blocksize) >= itsPaddedNY))
//     {
//       continue;
//     }
//     
//     // subgrids
//     
//     #pragma omp parallel for collapse(3)
//     for(int pol=0; pol<itsNPol; pol++) 
//     {
//       for(int j=0; j<blocksize; j++) 
//       {
//         for(int k=0; k<blocksize; k++) 
//         {
//           subgrids(IPosition(4, j, k, pol, i)) = itsModelGrids[0](IPosition(4,j+coordinates(0,i),k+coordinates(1,i),pol,0));
//         }
//       }
//     }
//     
//     i++;
//     
//     if ((i == N_chunks) || ((chunk+1) == v.chunks.end()))
//     {
//       itsProxy->degridder(
//         N_chunks,
//         offsets.data(), //      void    *offset,
//         wavenumbers.data(), //      void    *wavenumbers,
//         aterm.data(),                 
//         baselines.data(),
//         visibilities.data(), //      void    *visibilities
//         uvw.data(), //      void    *uvw,
//         spheroidal.data(),
//         subgrids.data() //      void    *uvgrid,
//       );
// 
//       // move predicted data to data column
// 
//       for(int j = 0; j<N_chunks; j++)
//       {
//         for(int k = 0; k<N_time; k++)
//         {
//           int idx = idx1(k,j);
//           if (idx > -1)
//           {
//             for(int pol = 0; pol < itsNPol; pol++)
//             {
//               for(int chan = 0; chan < N_chan; chan++)
//               {
//                 data(pol, chan, idx) = conj(visibilities(IPosition(4,pol, chan, k,j)))/(blocksize*blocksize)/2;
//               }
//             }
//           }
//         }
//       }
// 
//       idx1 = -1;      
//       i = 0;
//     }
//   }
// }
  
} //# end namespace LofarFT
} //# end namespace LOFAR

