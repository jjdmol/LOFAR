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
    store(itsAveragePB, itsImageName + ".avgpb");
    
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
FTMachineIDG::VisibilityMap FTMachineIDG::make_mapping(
  const VisBuffer& vb, 
  double dtime
)
{
  
  VisibilityMap v;
  
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
        chunk.w = 0.5 * (abs(uvw(start_idx)(2)) + abs(uvw(end_idx)(2)));
        
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
    itsCompilerFlags = "-O3 -xAVX -openmp -mkl -lmkl_vml_avx -lmkl_avx -DUSE_VML=1";
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
  
  itsNGrid = itsNThread;
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedData.resize (itsNGrid);
  itsGriddedData2.resize (itsNGrid);
  itsSumPB.resize (itsNGrid);
  itsSumCFWeight.resize (itsNGrid);
  itsSumWeight.resize (itsNGrid);
  itsGriddedDataDomain = UV;
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

void FTMachineIDG::put(const casa::VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type) 
{
  put( *static_cast<const VisBuffer*>(&vb), row, dopsf, type);
}

void FTMachineIDG::put(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type) 
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

  // TODO
  
  
  // find the maximum uv baseline length on grid
  double max_u = 0;
  double max_u_;
  double max_v = 0;
  double max_v_;
  #pragma omp parallel private(max_u_, max_v_)
  {
    max_u_ = 0;
    max_v_ = 0;
    #pragma omp for nowait
    for(int i = 0; i < vb.nRow(); i++)
    {
      double u = abs(vb.uvw()(i)(0));
      double v = abs(vb.uvw()(i)(1));
      if (u < max_u_) max_u_ = u;
      if (v < max_v_) max_v_ = v;
    }
    #pragma omp critical
    {
      if (max_u < max_u_) max_u = max_u_;
      if (max_v < max_v_) max_v = max_v_;
    }
  }
  
  double wavelength = casa::C::c / vb.frequency()(0);
  double max_u_grid = itsPaddedNX/(2 * itsUVScale(0)) * wavelength;
  double max_v_grid = itsPaddedNY/(2 * itsUVScale(1)) * wavelength;
  
  max_u = min(max_u, max_u_grid);
  max_v = min(max_v, max_v_grid);
  
  #define MAX_SUPPORT_FREQ 16
  
  // find the maximum frequency window to stay within maximum support
  double max_bw_u = MAX_SUPPORT_FREQ / (max_u / casa::C::c * itsUVScale(0));
  double max_bw_v = MAX_SUPPORT_FREQ / (max_v / casa::C::c * itsUVScale(1));
  double max_bw = min(max_bw_u, max_bw_v);
  
    
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
  int N_chan_group = channel_map.size();
  
  LOG_DEBUG_STR("N_chan: " << N_chan);
  
  
  
  
  
  // find the maximum timewindow to stay within maximum support
  // max time = max support / (max baseline (pixels) * earth_rotation_rate (rad/s))
  // min(itsTimeWindow, max time)
  
  
  
  
  
  
  // W stack 
  

  // TODO 
  // this mapping should include a range of possible w values to which the block can be projected
  
  VisibilityMap v = make_mapping(vb, itsTimeWindow);
  
  // TODO
  // now find the lowest w_max, and make a w-layer there
  // grid all blocks within range to this layer
  
  
  Int N_time = 0;
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    N_time = max(N_time, chunk->end - chunk->start + 1);
  }

  Int N_chunks = 100000;
  Int N_stations = 1 + max(max(vb.antenna1()), max(vb.antenna2()));
  Int blocksize = 48;

  LOG_DEBUG_STR("Creating proxy...");
  itsProxy = new Xeon (itsCompiler.c_str(), itsCompilerFlags.c_str(),  N_stations, N_chunks, N_time, N_chan,
      itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  LOG_DEBUG_STR("done.");
  
  Array<Complex> visibilities(IPosition(4, itsNPol, N_chan, N_time, N_chunks), Complex(0.0,0.0));
  Cube<Float> uvw(3, N_time, N_chunks, 0.0); 
  Matrix<Float> offsets(3, N_chunks, 0.0);
  Matrix<Int> coordinates(2, N_chunks, 0.0);
  Vector<Float> wavenumbers(N_chan, 0.0);
  Array<Complex> aterm(IPosition(4, blocksize, blocksize, itsNPol, N_stations), 0.0);
  aterm(Slicer(IPosition(4,0,0,0,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  aterm(Slicer(IPosition(4,0,0,3,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  Matrix<Float> spheroidal(blocksize,blocksize, 1.0);
  Array<Complex> subgrids(IPosition(4, blocksize, blocksize, itsNPol, N_chunks), Complex(0.0, 0.0));
  
  Cube<Complex> grid(itsPaddedNX, itsPaddedNY, itsNPol, Complex(0.0, 0.0));
  
  Matrix<Int> baselines(2, N_chunks, 0);
  
  // spheroidal
  taper(spheroidal);
  
  //iterate over channel groups
  
  for(std::vector<vector<int> >::iterator chan_group = channel_map.begin(); chan_group != channel_map.end(); ++chan_group)
  {
    // set wavenumbers for this chan_group
    int chan;
    vector<int>::iterator chan_it;
    for(chan_it = chan_group->begin(), chan=0; chan_it != chan_group->end(); ++chan_it, ++chan)
    {
      double wavelength = casa::C::c / vb.frequency()(*chan_it);
      wavenumbers(chan) = 2 * casa::C::pi / wavelength;
    }

    int i = 0;    
    for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
    {
      int idx_start = v.baseline_index_map[chunk->start];
      int idx_end = v.baseline_index_map[chunk->end];
      
      double f = (vb.frequency()(chan_group->front()) + vb.frequency()(chan_group->back())) / 2;
      double wavelength = casa::C::c / f;
      
      double center_u = floor((vb.uvw()(idx_start)(0) + vb.uvw()(idx_end)(0))/2/wavelength * itsUVScale(0));
      double center_v = floor((vb.uvw()(idx_start)(1) + vb.uvw()(idx_end)(1))/2/wavelength * itsUVScale(1));
      
      coordinates(0,i) = center_u + itsUVOffset(0) - blocksize/2;
      coordinates(1,i) = center_v + itsUVOffset(1) - blocksize/2;
      
      //check whether block falls within grid
      if ((coordinates(0,i) < 0) ||
        (coordinates(1,i) < 0) ||
        ((coordinates(0,i)+blocksize) >= itsPaddedNX) ||
        ((coordinates(1,i)+blocksize) >= itsPaddedNY))
      {
        continue;
      }
      
      offsets (0, i) = center_u / itsUVScale(0) * 2 * casa::C::pi;
      offsets (1, i) = center_v / itsUVScale(1) * 2 * casa::C::pi;
      offsets (2, i) = 0;


      // copy data to buffer
      
      for (int j = chunk->start; j<=chunk->end; j++)
      {
        int k = j - chunk->start;
        
        int idx = v.baseline_index_map[j];

        uvw (0, k, i) = vb.uvw()(idx)(0);
        uvw (1, k, i) = vb.uvw()(idx)(1);
        uvw (2, k, i) = vb.uvw()(idx)(2);
      }
      
      
  //     double w_support = abs(max(uvw(2, idx_start),uvw(2, idx_end))/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0)*itsUVScale(0));
  //     
  //     double min_u = uvw(0, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
  //     min_u = min(min_u, uvw(0, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  //     min_u = min(min_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
  //     min_u = min(min_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  // 
  //     double min_v = uvw(1, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
  //     min_v = min(min_v, uvw(1, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  //     min_v = min(min_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
  //     min_v = min(min_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  //     
  //     double max_u = uvw(0, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
  //     max_u = max(max_u, uvw(0, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  //     max_u = max(max_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
  //     max_u = max(max_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  // 
  //     double max_v = uvw(1, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
  //     max_v = max(max_v, uvw(1, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  //     max_v = max(max_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
  //     max_v = max(max_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
  //     
  //     double uv_support = max(abs(max_u-min_u), abs(max_v-min_v));
      
  //     if ((uv_support + w_support) > 30)
  //     {
  //       cout << "w_support: " << w_support << endl;
  //       cout << "uv_support: " << uv_support << endl;
  //     }
      
      
      // baselines
      int idx = v.baseline_index_map[chunk->start];
      int ant1 = vb.antenna1()[idx];
      int ant2 = vb.antenna2()[idx];
      baselines(0,i) = ant1;
      baselines(1,i) = ant2;
      
      
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
                visibilities( IPosition(4, idx_pol, chan, k, i)) = imagingWeightCube(idx_pol, *chan_it, idx) * ((idx_pol==0) ||( idx_pol==3));
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
                visibilities( IPosition(4, idx_pol, chan, k, i)) = data(idx_pol, *chan_it, idx)*imagingWeightCube(idx_pol, *chan_it, idx);
                itsSumWeight[0](idx_pol,0) += imagingWeightCube(idx_pol, *chan_it, idx)*blocksize*blocksize*2;
              }
            }
          }
        }
      }
      
      i++;
      
      // if buffer is full or no more chunks then process buffer
      
      if ((i == N_chunks) || ((chunk+1) == v.chunks.end()))
      {
  //       cout << "N_chunks: " << N_chunks << endl;      
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
        itsProxy->adder(N_chunks, coordinates.data(), subgrids.data(), itsGriddedData[0].data());
        LOG_DEBUG_STR("done.");
        i = 0;
        visibilities = Complex(0);
      }
    }
  } 
}

void FTMachineIDG::get(casa::VisBuffer& vb, Int row)
{
  get(*static_cast<VisBuffer*>(&vb), row);
}

// Degrid
void FTMachineIDG::get(VisBuffer& vb, Int row)
{
  // within this scope redirect messages written to clog to LofarLogger with loglevel DEBUG
  ScopedRedirect<LogDebugSink> scoped_redirect(std::clog); 
  
//   gridOk(itsGridder->cSupport()(0));
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;}
  else         { nRow=1; startRow=row; endRow=row; }

  Vector<Int> chan_map(vb.frequency().size(), 0);
  
  Vector<Double> lsr_frequency;
  
  Int spwid = 0;
  Bool convert = True;
  vb.lsrFrequency(spwid, lsr_frequency, convert);

  Cube<Complex> data(vb.modelVisCube());
  
  VisibilityMap v = make_mapping(vb, itsTimeWindow);
  Int N_time = 0;
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    N_time = max(N_time, chunk->end - chunk->start + 1);
  }

  Int N_chan = vb.nChannel();
  
  Int N_chunks = 100000;
  Int N_stations = 1 + max(max(vb.antenna1()), max(vb.antenna2()));
  Int blocksize = 48;

  LOG_DEBUG_STR("Creating proxy...");
  itsProxy = new Xeon (itsCompiler.c_str(), itsCompilerFlags.c_str(),  N_stations, N_chunks, N_time, N_chan,
      itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  LOG_DEBUG_STR("done.");

  Array<Complex> visibilities(IPosition(4, itsNPol, N_chan, N_time, N_chunks), Complex(0.0,0.0));
  Cube<Float> uvw(3, N_time, N_chunks, 0.0); 
  Matrix<Int> idx1(N_time, N_chunks, -1); 
  Matrix<Float> offsets(3, N_chunks, 0.0);
  Matrix<Int> coordinates(2, N_chunks, 0.0);
  Vector<Float> wavenumbers(N_chan, 0.0);
  Array<Complex> aterm(IPosition(4, blocksize, blocksize, itsNPol, N_stations), 0.0);
  aterm(Slicer(IPosition(4,0,0,0,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  aterm(Slicer(IPosition(4,0,0,3,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;

  Matrix<Float> spheroidal(blocksize,blocksize, 1.0);
  Array<Complex> subgrids(IPosition(4, blocksize, blocksize, itsNPol, N_chunks), Complex(0.0, 0.0));
  
  Cube<Complex> grid(itsNPol, itsPaddedNX, itsPaddedNY, Complex(0.0, 0.0));
  
  Matrix<Int> baselines(2, N_chunks, 0);
  
  // spheroidal
  taper(spheroidal);

  // wavenumbers
  for(int i = 0; i<N_chan; i++)
  {
    double wavelength = casa::C::c / vb.frequency()(i);
    wavenumbers(i) = 2 * casa::C::pi / wavelength;
  }

  int i = 0;    
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    for (int j = chunk->start; j<=chunk->end; j++)
    {
      int k = j - chunk->start;
      
      int idx = v.baseline_index_map[j];

      uvw (0, k, i) = vb.uvw()(idx)(0);
      uvw (1, k, i) = vb.uvw()(idx)(1);
      uvw (2, k, i) = vb.uvw()(idx)(2);
      
      idx1(k,i) = idx;
      
    }
    
    int idx_start = v.baseline_index_map[chunk->start];
    int idx_end = v.baseline_index_map[chunk->end];
    
    double wavelength = casa::C::c / vb.frequency()(N_chan/2);
    
    offsets (0, i) = double(floor((vb.uvw()(idx_start)(0) + vb.uvw()(idx_end)(0))/2/wavelength * itsUVScale(0))) / itsUVScale(0) * 2 * casa::C::pi;
    offsets (1, i) = double(floor((vb.uvw()(idx_start)(1) + vb.uvw()(idx_end)(1))/2/wavelength * itsUVScale(1))) / itsUVScale(1) * 2 * casa::C::pi;
    offsets (2, i) = 0;
    
//     double w_support = abs(max(vb.uvw()(idx_start)(2),vb.uvw()(idx_start)(2))/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0)*itsUVScale(0));
//     
//     double min_u = vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
//     min_u = min(min_u, vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
//     min_u = min(min_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
//     min_u = min(min_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// 
//     double min_v = vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
//     min_v = min(min_v, vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
//     min_v = min(min_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
//     min_v = min(min_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
//     
//     double max_u = vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
//     max_u = max(max_u, vb.uvw()(idx_start)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
//     max_u = max(max_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
//     max_u = max(max_u, vb.uvw()(idx_end)(0)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
// 
//     double max_v = vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
//     max_v = max(max_v, vb.uvw()(idx_start)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
//     max_v = max(max_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
//     max_v = max(max_v, vb.uvw()(idx_end)(1)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
//     
//     double uv_support = max(abs(max_u-min_u), abs(max_v-min_v));
//     
//     if ((uv_support + w_support) > 30)
//     {
//       cout << "w_support: " << w_support << endl;
//       cout << "uv_support: " << uv_support << endl;
//     }
    
    coordinates(0,i) = round(offsets(0,i)/2/casa::C::pi*itsUVScale(0)) + itsUVOffset(0) - blocksize/2;
    coordinates(1,i) = round(offsets(1,i)/2/casa::C::pi*itsUVScale(1)) + itsUVOffset(1) - blocksize/2;
    
    // baselines
    int idx = v.baseline_index_map[chunk->start];
    int ant1 = vb.antenna1()[idx];
    int ant2 = vb.antenna2()[idx];
    baselines(0,i) = ant1;
    baselines(1,i) = ant2;
    
    if ((coordinates(0,i) < 0) ||
      (coordinates(1,i) < 0) ||
      ((coordinates(0,i)+blocksize) >= itsPaddedNX) ||
      ((coordinates(1,i)+blocksize) >= itsPaddedNY))
    {
      continue;
    }
    
    // subgrids
    
    #pragma omp parallel for collapse(3)
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
    
    i++;
    
    if ((i == N_chunks) || ((chunk+1) == v.chunks.end()))
    {
      itsProxy->degridder(
        N_chunks,
        offsets.data(), //      void    *offset,
        wavenumbers.data(), //      void    *wavenumbers,
        aterm.data(),                 
        baselines.data(),
        visibilities.data(), //      void    *visibilities
        uvw.data(), //      void    *uvw,
        spheroidal.data(),
        subgrids.data() //      void    *uvgrid,
      );

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
              for(int chan = 0; chan < N_chan; chan++)
              {
                data(pol, chan, idx) = conj(visibilities(IPosition(4,pol, chan, k,j)))/(blocksize*blocksize)/2;
              }
            }
          }
        }
      }

      idx1 = -1;      
      i = 0;
    }
  }
  
  
}
  
} //# end namespace LofarFT
} //# end namespace LOFAR

