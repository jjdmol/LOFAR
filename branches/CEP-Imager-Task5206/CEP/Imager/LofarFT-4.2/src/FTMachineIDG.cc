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

#include <LofarFT/FTMachineIDG.h>
#include <LofarFT/VBStore.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/VisBuffer.h>
#include <LofarFT/ConvolutionFunction.h>
#include <LofarFT/VisResamplerMatrixWB.h>
#include <Common/OpenMP.h>
#include <lattices/Lattices/LatticeFFT.h>
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

  
const Matrix<Float>& FTMachineIDG::getAveragePB() const
{
  if (itsAveragePB.empty()) 
  {
    itsAveragePB.resize(itsPaddedNX, itsPaddedNY);
    itsAveragePB = 1.0;
  }
  return itsAveragePB;
}
  

  // Partition the visbuffer according to baseline number and time
FTMachineIDG::VisibilityMap FTMachineIDG::make_mapping(
  const VisBuffer& vb, 
  const casa::Vector< casa::Double > &frequency_list_CF,
  double dtime,
  double w_step
)
{
  
  cout << "make map..." << flush;
  
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
        
        int N_CF_chan = frequency_list_CF.nelements();
        chunk.wplane_map.resize(N_CF_chan);
        for (int ch=0; ch<N_CF_chan; ch++)
        {
          double freq = frequency_list_CF(ch);
          double wavelength = casa::C::c / freq;
          double w_lambda = chunk.w/wavelength;
          int w_plane = floor(abs(w_lambda)/w_step);
          chunk.wplane_map[ch] = w_plane;
          if (w_plane>v.max_w_plane) v.max_w_plane = w_plane;
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
  cout << "done." << endl;
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
  itsNGrid = itsNThread;
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedData.resize (itsNGrid);
  itsGriddedData2.resize (itsNGrid);
  itsSumPB.resize (itsNGrid);
  itsSumCFWeight.resize (itsNGrid);
  itsSumWeight.resize (itsNGrid);
  itsVisResampler = new VisResamplerMatrixWB();
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
  if (itsVerbose > 0) {
    logIO() << LogOrigin(theirName, "put") << LogIO::NORMAL
            << "I am gridding " << vb.nRow() << " row(s)."  << LogIO::POST;
    logIO() << LogIO::NORMAL << "Padding is " << itsPadding  << LogIO::POST;
  }
  
  // Match data channels to images channels
  // chan_map is filled by match_channel with a mapping of the data channels
  // to the image channels. This mapping changes over time because of changing
  // Doppler shift due to earth rotation around its axis and the sun.
  // The channel mapping is determined for the first time sample in vb
  // It is assumed that within the vb the change in Dopplershift is small .
  
  // Wide band imager has multiple terms but only a single channel
  // All visibility channels map to image channel 0
  Vector<Int> chan_map(vb.frequency().size(), 0);;
  itsVisResampler->set_chan_map(chan_map);
  
  Vector<Double> lsr_frequency;
  
  Int spwid = 0;
  Bool convert = True;
  vb.lsrFrequency(spwid, lsr_frequency, convert);
  
  // Set the frequencies for which the convolution function will be evaluated.
  // set_frequency groups the frequncies found in vb according to the number of
  // data channels in a convolution function channel.
  // chan_map_CF is a mapping of the data channels to the 
  // convolution function channels
  Vector<Int> chan_map_CF;
  chan_map_CF = itsConvFunc->set_frequency(vb.frequency());
  itsVisResampler->set_chan_map_CF(chan_map_CF);
  
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

  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  
  for (Int i=startRow;i<=endRow;i++) 
  {
    for (Int idim = 0; idim<3; idim++) 
    {
      uvw(idim,i) = vb.uvw()(i)(idim);
    }
  }
  
//   rotateUVW(uvw, dphase, vb);
//   refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // Set up VBStore object to point to the relevant info of the VB.

  itsVisResampler->setParams(itsUVScale, itsUVOffset, dphase);

  const casa::Vector< casa::Double > &frequency_list_CF = itsConvFunc->get_frequency_list();

  double w_step = itsConvFunc->get_w_from_support();
  cout << "w_step: " << w_step << endl;

  VisibilityMap v = make_mapping(vb, frequency_list_CF, itsTimeWindow, w_step);
  
  Int N_time = 0;
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    N_time = max(N_time, chunk->end - chunk->start + 1);
  }

  Int N_chan = vb.nChannel();
  
  Int N_chunks = 100000;
  Int N_stations = 1 + max(max(vb.antenna1()), max(vb.antenna2()));
  Int blocksize = 32;

//   itsProxy = new Xeon ("g++", "-fopenmp -march=native",  N_stations, N_chunks, N_time, N_chan,
//       itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  itsProxy = new Xeon ("icpc", "-fopenmp -march=native -g",  N_stations, N_chunks, N_time, N_chan,
      itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  
  Array<Complex> visibilities(IPosition(4, itsNPol, N_chan, N_time, N_chunks), Complex(0.0,0.0));
  Cube<Float> uvw1(3, N_time, N_chunks, 0.0); 
  Matrix<Float> offsets(3, N_chunks, 0.0);
  Matrix<Int> coordinates(2, N_chunks, 0.0);
  Vector<Float> wavenumbers(N_chan, 0.0);
  Array<Complex> aterm(IPosition(4, blocksize, blocksize, itsNPol, N_stations), 0.0);
  aterm(Slicer(IPosition(4,0,0,0,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  aterm(Slicer(IPosition(4,0,0,3,0), IPosition(4,blocksize,blocksize,1,N_stations))) = 1.0;
  Matrix<Float> spheroidal(blocksize,blocksize, 1.0);
  Array<Complex> subgrids(IPosition(4, itsNPol, blocksize, blocksize, N_chunks), Complex(0.0, 0.0));
  
  Cube<Complex> grid(itsPaddedNX, itsPaddedNY, itsNPol, Complex(0.0, 0.0));
  
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

      uvw1 (0, k, i) = uvw(0, idx);
      uvw1 (1, k, i) = uvw(1, idx);
      uvw1 (2, k, i) = uvw(2, idx);
      
      for (int idx_pol = 0; idx_pol < itsNPol; idx_pol++)
      {
        for (int idx_chan = 0; idx_chan < N_chan; idx_chan++)
        {
          if (vb.flagCube()(idx_pol, idx_chan, idx))
          {
            visibilities( IPosition(4, idx_pol, idx_chan, k, i)) = Complex(0);
          }
          else
          {
            visibilities( IPosition(4, idx_pol, idx_chan, k, i)) = data(idx_pol, idx_chan, idx)*imagingWeightCube(idx_pol, idx_chan, idx);
            itsSumWeight[0](idx_pol,0) += imagingWeightCube(idx_pol, idx_chan, idx);
          }
        }
      }
    }
    
    int idx_start = v.baseline_index_map[chunk->start];
    int idx_end = v.baseline_index_map[chunk->end];
    
    double wavelength = casa::C::c / vb.frequency()(N_chan/2);
    
    offsets (0, i) = int((uvw(0, idx_start) + uvw(0, idx_end))/2/wavelength * itsUVScale(0)) / itsUVScale(0) * 2 * casa::C::pi;
    offsets (1, i) = int((uvw(1, idx_start) + uvw(1, idx_end))/2/wavelength * itsUVScale(1)) / itsUVScale(1) * 2 * casa::C::pi;
    offsets (2, i) = 0;
    
    double w_support = abs(max(uvw(2, idx_start),uvw(2, idx_end))/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0)*itsUVScale(0));
    
    double min_u = uvw(0, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    min_u = min(min_u, uvw(0, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    min_u = min(min_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    min_u = min(min_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));

    double min_v = uvw(1, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    min_v = min(min_v, uvw(1, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    min_v = min(min_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    min_v = min(min_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    
    double max_u = uvw(0, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    max_u = max(max_u, uvw(0, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    max_u = max(max_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    max_u = max(max_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));

    double max_v = uvw(1, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    max_v = max(max_v, uvw(1, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    max_v = max(max_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    max_v = max(max_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    
    double uv_support = max(abs(max_u-min_u), abs(max_v-min_v));
    
    if ((uv_support + w_support) > 30)
    {
      cout << "w_support: " << w_support << endl;
      cout << "uv_support: " << uv_support << endl;
    }
    
    coordinates(0,i) = offsets(0,i)/2/casa::C::pi*itsUVScale(0) + itsUVOffset(0) - blocksize/2;
    coordinates(1,i) = offsets(1,i)/2/casa::C::pi*itsUVScale(1) + itsUVOffset(1) - blocksize/2;
    
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
    else
    {
      i++;
    }
      
    if ((i == N_chunks) || ((chunk+1) == v.chunks.end()))
    {
      cout << "N_chunks: " << N_chunks << endl;      
      itsProxy->gridder(
        N_chunks,
        visibilities.data(), //      void    *visibilities
        uvw1.data(), //      void    *uvw,
        offsets.data(), //      void    *offset,
        wavenumbers.data(), //      void    *wavenumbers,
        aterm.data(),                 
        spheroidal.data(),
        baselines.data(),
        subgrids.data() //      void    *uvgrid,
      );
      
      itsProxy->adder(N_chunks, coordinates.data(), subgrids.data(), itsGriddedData[0].data());

      i = 0;
      visibilities = Complex(0);
    }
  }
  
  // add grid to itsGriddedData
//   for(int i=0; i < itsPaddedNX; i++)
//   {
//     for(int j=0; j < itsPaddedNX; j++)
//     {
//       for(int k=0; k < itsNPol; k++)
//       {
//         itsGriddedData[0](IPosition(4,i,j,k,0)) += grid(i,j,k)/(blocksize*blocksize);
//       }
//     }
//   }
//   
//   store(Matrix<Complex>(itsGriddedData[0][0][0]), "grid");
}

bool FTMachineIDG::put_on_w_plane(
  const VisBuffer &vb,
  const VBStore &vbs,
  const Vector<Double> &lsr_frequency,
  vector< casa::Array<casa::Complex> >  &w_plane_grids,
  const VisibilityMap &v,
  int w_plane,
  double w_offset,
  bool dopsf)
{
//       Get the convolution function.

  bool any_match = false;
  int i = 0;
  for (std::vector<Chunk>::const_iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    cout << "\r" << i++ << "/" << v.chunks.size() << flush;
    bool any_channel_match = false;
    vector<bool> channel_selection;
    channel_selection.reserve(chunk->wplane_map.size());
    for (std::vector<int>::const_iterator w_idx = chunk->wplane_map.begin() ; w_idx != chunk->wplane_map.end(); ++w_idx)
    {
      bool match = (*w_idx == w_plane);
      channel_selection.push_back(match);
      any_channel_match = any_channel_match || match;
    }
    if (any_channel_match)
    {
      any_match = true;
      int idx = v.baseline_index_map[chunk->start];
      int ant1 = vb.antenna1()[idx];
      int ant2 = vb.antenna2()[idx];
      
      itsConvFunc->computeAterm (chunk->time);
      CFStore cfStore = itsConvFunc->makeConvolutionFunction (
        ant1, 
        ant2, 
        chunk->time,
        chunk->w,
        chunk->sum_weight,
        channel_selection, 
        w_offset);
      
      if (itsUseDoubleGrid) 
      {
    //       TODO: support for double precision grids
    //       itsVisResampler->DataToGrid(
    //         itsGriddedData2[threadNum], 
    //         vbs, 
    //         blIndex,
    //         blStart[i], 
    //         blEnd[i],
    //         itsSumWeight[threadNum], 
    //         dopsf, 
    //         cfStore);
      } 
      else 
      {
        #pragma omp parallel for num_threads(itsNGrid)
        for (int taylor_idx = 0; taylor_idx<itsNGrid; taylor_idx++)
        {
          Vector<Double> taylor_weights(lsr_frequency.nelements(), 1.0);
          for (int j=0; j<lsr_frequency.nelements(); j++)
          {
            taylor_weights(j) = pow((lsr_frequency(j) - itsRefFreq)/itsRefFreq, taylor_idx);
          }
          itsVisResampler->DataToGrid(
            w_plane_grids[taylor_idx], 
            vbs, 
            v.baseline_index_map, 
            chunk->start,
            chunk->end, 
            itsSumWeight[taylor_idx], 
            dopsf, 
            cfStore,
            taylor_weights);
        }
      }
    }
  }
  return any_match;
}



void FTMachineIDG::get(casa::VisBuffer& vb, Int row)
{
  get(*static_cast<VisBuffer*>(&vb), row);
}

// Degrid
void FTMachineIDG::get(VisBuffer& vb, Int row)
{
//   gridOk(itsGridder->cSupport()(0));
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;}
  else         { nRow=1; startRow=row; endRow=row; }

  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  
  for (Int i=startRow;i<=endRow;i++) 
  {
    for (Int idim=0;idim<3;idim++) 
    {
      uvw(idim,i) = vb.uvw()(i)(idim);
    }
  }

  Vector<Int> chan_map(vb.frequency().size(), 0);
  itsVisResampler->set_chan_map(chan_map);
  
  // Set the frequencies for which the convolution function will be evaluated.
  // set_frequency groups the frequncies found in vb according to the number of
  // data channels in a convolution function channel.
  // chan_map_CF is a mapping of the data channels to the 
  // convolution function channels
  Vector<Int> chan_map_CF;
  chan_map_CF = itsConvFunc->set_frequency(vb.frequency());
  itsVisResampler->set_chan_map_CF(chan_map_CF);
  
  Vector<Double> lsr_frequency;
  
  Int spwid = 0;
  Bool convert = True;
  vb.lsrFrequency(spwid, lsr_frequency, convert);

  Cube<Complex> data(vb.modelVisCube());
  
  itsVisResampler->setParams(itsUVScale, itsUVOffset, dphase);
  itsVisResampler->setMaps(chanMap, polMap);

  const casa::Vector< casa::Double > &frequency_list_CF = itsConvFunc->get_frequency_list();

  double w_step = itsConvFunc->get_w_from_support();
  cout << "w_step: " << w_step << endl;

  VisibilityMap v = make_mapping(vb, frequency_list_CF, itsTimeWindow, w_step);
  Int N_time = 0;
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    N_time = max(N_time, chunk->end - chunk->start + 1);
  }

  Int N_chan = vb.nChannel();
  
  Int N_chunks = 100000;
  Int N_stations = 1 + max(max(vb.antenna1()), max(vb.antenna2()));
  Int blocksize = 32;

//   itsProxy = new Xeon ("g++", "-fopenmp -march=native",  N_stations, N_chunks, N_time, N_chan,
//       itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  itsProxy = new Xeon ("icpc", "-fopenmp -march=native -g",  N_stations, N_chunks, N_time, N_chan,
      itsNPol, blocksize, itsPaddedNX, itsUVScale(0));
  
  Array<Complex> visibilities(IPosition(4, itsNPol, N_chan, N_time, N_chunks), Complex(0.0,0.0));
  Cube<Float> uvw1(3, N_time, N_chunks, 0.0); 
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

      uvw1 (0, k, i) = uvw(0, idx);
      uvw1 (1, k, i) = uvw(1, idx);
      uvw1 (2, k, i) = uvw(2, idx);
      
      idx1(k,i) = idx;
      
    }
    
    int idx_start = v.baseline_index_map[chunk->start];
    int idx_end = v.baseline_index_map[chunk->end];
    
    double wavelength = casa::C::c / vb.frequency()(N_chan/2);
    
    offsets (0, i) = int((uvw(0, idx_start) + uvw(0, idx_end))/2/wavelength * itsUVScale(0)) / itsUVScale(0) * 2 * casa::C::pi;
    offsets (1, i) = int((uvw(1, idx_start) + uvw(1, idx_end))/2/wavelength * itsUVScale(1)) / itsUVScale(1) * 2 * casa::C::pi;
    offsets (2, i) = 0;
    
    double w_support = abs(max(uvw(2, idx_start),uvw(2, idx_end))/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0)*itsUVScale(0));
    
    double min_u = uvw(0, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    min_u = min(min_u, uvw(0, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    min_u = min(min_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    min_u = min(min_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));

    double min_v = uvw(1, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    min_v = min(min_v, uvw(1, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    min_v = min(min_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    min_v = min(min_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    
    double max_u = uvw(0, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    max_u = max(max_u, uvw(0, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    max_u = max(max_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    max_u = max(max_u, uvw(0, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));

    double max_v = uvw(1, idx_start)/casa::C::c*vb.frequency()(0)*itsUVScale(0);
    max_v = max(max_v, uvw(1, idx_start)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    max_v = max(max_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(0)*itsUVScale(0));
    max_v = max(max_v, uvw(1, idx_end)/casa::C::c*vb.frequency()(N_chan-1)*itsUVScale(0));
    
    double uv_support = max(abs(max_u-min_u), abs(max_v-min_v));
    
    if ((uv_support + w_support) > 30)
    {
      cout << "w_support: " << w_support << endl;
      cout << "uv_support: " << uv_support << endl;
    }
    
    coordinates(0,i) = offsets(0,i)/2/casa::C::pi*itsUVScale(0) + itsUVOffset(0) - blocksize/2;
    coordinates(1,i) = offsets(1,i)/2/casa::C::pi*itsUVScale(1) + itsUVOffset(1) - blocksize/2;
    
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
    for(int pol=0; pol<itsNPol; pol++) 
    {
      for(int j=0; j<blocksize; j++) 
      {
        for(int k=0; k<blocksize; k++) 
        {
          subgrids(IPosition(4, j, k, pol, i)) = conj(itsModelGrids[0](IPosition(4,j+coordinates(0,i),k+coordinates(1,i),pol,0)));
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
        uvw1.data(), //      void    *uvw,
        spheroidal.data(),
        subgrids.data() //      void    *uvgrid,
      );
      
      // move predicted data to data colun

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
                data(pol, chan, idx) = visibilities(IPosition(4,pol, chan, k,j));
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

