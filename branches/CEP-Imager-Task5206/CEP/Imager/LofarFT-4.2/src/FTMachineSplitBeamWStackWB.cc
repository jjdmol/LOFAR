//# FTMachineSplitBeamWStackWB.cc: Gridder for LOFAR data correcting for DD effects
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
//# $Id: FTMachineSplitBeamWStackWB.cc 28512 2014-03-05 01:07:53Z vdtol $

#include <lofar_config.h>

#include <LofarFT/FTMachineSplitBeamWStackWB.h>
#include <LofarFT/ScopedTimer.h>
#include <LofarFT/FFTCMatrix.h>
#include <LofarFT/VBStore.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/VisBuffer.h>
#include <LofarFT/ConvolutionFunction.h>
#include <LofarFT/ConvolutionFunctionDiagonal.h>
#include <LofarFT/VisResamplerMatrixWB.h>
#include <LofarFT/VisResamplerDiagonalWB.h>
#include <LofarFT/util.h>

#include <Common/OpenMP.h>
#include <lattices/Lattices/LatticeFFT.h>

#include "helper_functions.tcc"

using namespace casa;

const String LOFAR::LofarFT::FTMachineSplitBeamWStackWB::theirName("FTMachineSplitBeamWStackWB");

namespace
{
  bool dummy = LOFAR::LofarFT::FTMachineFactory::instance().
    registerClass<LOFAR::LofarFT::FTMachineSplitBeamWStackWB>(
      LOFAR::LofarFT::FTMachineSplitBeamWStackWB::theirName);
}

namespace LOFAR {
namespace LofarFT {

FTMachineSplitBeamWStackWB::FTMachineSplitBeamWStackWB(
  const MeasurementSet& ms,
  const ParameterSet& parset)
  : FTMachine( ms, parset),
    itsSplitBeam(parset.getBool("gridding.splitbeam", False)),
    itsNThread(OpenMP::maxThreads())
{
  //TODO 
  itsMachineName = theirName;
  itsNGrid = itsNThread;
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedData.resize (itsNGrid);
  itsGriddedData2.resize (itsNGrid);
  itsSumPB.resize (itsNGrid);
  itsSumCFWeight.resize (itsNGrid);
  itsSumWeight.resize (itsNGrid);
  itsGriddedDataDomain = IMAGE;
  double msRefFreq=0; //TODO: put some useful reference frequency here
  itsRefFreq = get_reference_frequency(itsParset, itsMS);
  itsTimeWindow=parset.getDouble("gridding.timewindow",300);
  if (itsSplitBeam)
  {
    itsConvFunc = new ConvolutionFunctionDiagonal(
      itsMS, 
      itsWMax,
      itsOversample, 
      itsVerbose, 
      itsMaxSupport,
      itsParset);
    itsVisResampler = new VisResamplerDiagonalWB();
  }
  else
  {
    itsVisResampler = new VisResamplerMatrixWB();
  }
  
}

FTMachineSplitBeamWStackWB::~FTMachineSplitBeamWStackWB()
{
}

  //----------------------------------------------------------------------
FTMachineSplitBeamWStackWB& FTMachineSplitBeamWStackWB::operator=(const FTMachineSplitBeamWStackWB& other)
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
  }
  return *this;
}

//----------------------------------------------------------------------
  FTMachineSplitBeamWStackWB::FTMachineSplitBeamWStackWB(const FTMachineSplitBeamWStackWB& other) : 
    FTMachine(other)
  {
    operator=(other);
  }

//----------------------------------------------------------------------
  FTMachineSplitBeamWStackWB* FTMachineSplitBeamWStackWB::clone() const
  {
    FTMachineSplitBeamWStackWB* newftm = new FTMachineSplitBeamWStackWB(*this);
    return newftm;
  }

//----------------------------------------------------------------------

  
void FTMachineSplitBeamWStackWB::initialize_model_grids(Bool normalize_model)
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
  
  // Partition the visbuffer according to baseline number and time
FTMachineSplitBeamWStackWB::VisibilityMap FTMachineSplitBeamWStackWB::make_mapping(
  const VisBuffer& vb, 
  const casa::Vector< casa::Double > &frequency_list_CF,
  double dtime,
  double w_step
)
{
  
  ScopedTimer t("make_mapping");
  
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
  
//   /////vvvvvvvvvvvvvvvvv///////////////////////
// 
//   int nrow = vb.nRow();
//   int nchan = vb.frequency().size();
//   Vector w(nrow * nchan);
//   
//   for (int i; i<nrow; i++)
//   {
//     for (int ch=0; ch<nchan; ch++)
//     {
//       float freq = vb.frequency()(ch);
//       float wavelength = casa::C::c / freq;
//       w(i*nchan+ch) = abs(uvw(i)(2))/wavelength;
//     }
//   }        
//   /////^^^^^^^^^^^^^^^^^////////////////////////
//           

  
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
        chunk.w = 0.5 * (uvw(start_idx)(2) + uvw(end_idx)(2));
        
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

void FTMachineSplitBeamWStackWB::put(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type) 
{
  
  ScopedTimer _t("FTMachineSplitBeamWStackWB::put");
  
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

  if (itsSplitBeam)
  {
    Double t = (vb.timeCentroid()(0) + vb.timeCentroid()(nRow-1)) / 2;
//     itsConvFunc->computeElementBeam(t);
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
  VBStore vbs;

  vbs.nRow(vb.nRow());
  vbs.uvw(uvw);
  vbs.imagingWeightCube(imagingWeightCube);
  vbs.visCube(data);
  vbs.freq(vb.frequency());
  vbs.rowFlag(vb.flagRow());
  vbs.flagCube(vb.flagCube());

  itsVisResampler->setParams(itsUVScale, itsUVOffset, dphase);

  const casa::Vector< casa::Double > &frequency_list_CF = itsConvFunc->get_frequency_list();

  double w_step = 2*itsConvFunc->get_w_from_support();
  cout << "w_step: " << w_step << endl;

  VisibilityMap v = make_mapping(vb, frequency_list_CF, itsTimeWindow, w_step);
  
  // sum weights per chunk
  // for average beam computation
  // TODO take care of flags
  for (std::vector<Chunk>::iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    chunk->sum_weight.resize(IPosition(2,4,chan_map_CF.size()));
    chunk->sum_weight = 0.0;
    // iterate over time in chunk
    for(int i=chunk->start; i<=chunk->end; i++)
    {
      int idx = v.baseline_index_map[i];
      // iterate over channels in data
      for(int ch=0; ch<vb.nChannel(); ch++)
      {
        // map to CF channels and sum weight
        chunk->sum_weight[chan_map_CF[ch]] = chunk->sum_weight[chan_map_CF[ch]] + imagingWeightCube[idx][ch];
      }
    }
  }
  

  // init the grids to zero
  vector< casa::Array<casa::Complex> >  w_plane_grids(itsNGrid);
  for (int i=0; i<itsNGrid; i++)
  {
    w_plane_grids[i].resize(itsGriddedData[0].shape());
    w_plane_grids[i] = 0;
  }
  
  for (int w_plane=0; w_plane <= v.max_w_plane; w_plane++)
  {
    double w_offset = (w_plane+0.5) * w_step;
    
    if (put_on_w_plane(vb, vbs, lsr_frequency, w_plane_grids, v, w_plane, w_offset, dopsf))
    // returns true if anything was put on this plane
    {
//       store(Matrix<Complex>(w_plane_grids[0][0][0]), "grid1");
//       throw AipsError("stop");
      
      cout << "w_plane: " << w_plane << endl;
      for (int i=0; i<itsNGrid; i++)
      {
        // transform to image domain
//         ArrayLattice<Complex> lattice(w_plane_grids[i]);
//         LatticeFFT::cfft2d(lattice, True);
        
        {
          ScopedTimer t("W plane fft");
          Array<Complex> w_plane_grid(w_plane_grids[i]);
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
        
        // Apply W term in image domain
        {
          ScopedTimer t("W plane apply");
          itsConvFunc->applyWterm(w_plane_grids[i], -w_offset);
        }
        
        
        if (itsSplitBeam)
        {
//            itsConvFunc->applyElementBeam(w_plane_grids[i], True);
        }
        
        // Add image to master grid
        itsGriddedData[i] += w_plane_grids[i];
        // reset the grid
        w_plane_grids[i].resize(itsGriddedData[0].shape());
        w_plane_grids[i] = 0;
      }
    }
  }
  
  
}

bool FTMachineSplitBeamWStackWB::put_on_w_plane(
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
  omp_set_nested(1);

  bool any_match = false;
  int i = 0;
  for (std::vector<Chunk>::const_iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
  {
    cout << "\r" << i++ << "/" << v.chunks.size() << flush;
    
//     // DEBUG
//     if (chunk->w > 0) continue;

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
      
      //DEBUG (added static to reduce runtime, results are incorrect)
//       static CFStore cfStore = itsConvFunc->makeConvolutionFunction (
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
//         #pragma omp parallel for num_threads(itsNGrid)
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
      //DEBUG
//       break;
    }
  }
  return any_match;
}



void FTMachineSplitBeamWStackWB::get(casa::VisBuffer& vb, Int row)
{
  get(*static_cast<VisBuffer*>(&vb), row);
}

// Degrid
void FTMachineSplitBeamWStackWB::get(VisBuffer& vb, Int row)
{
//   gridOk(itsGridder->cSupport()(0));
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;}
  else         { nRow=1; startRow=row; endRow=row; }

  // Get the uvws in a form that Fortran can use
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
  
  VBStore vbs;
  vbs.nRow(vb.nRow());
  vbs.beginRow(0);
  vbs.endRow(vbs.nRow());

  vbs.uvw(uvw);
  vbs.visCube(data);
  
  vbs.freq(vb.frequency());
  vbs.rowFlag(vb.flagRow());
  vbs.flagCube(vb.flagCube());
  
  itsVisResampler->setParams(itsUVScale, itsUVOffset, dphase);
  itsVisResampler->setMaps(chanMap, polMap);

  if (itsSplitBeam)
  {
    Double t = (vb.timeCentroid()(0) + vb.timeCentroid()(nRow-1)) / 2;
//     itsConvFunc->computeElementBeam(t);
  }


  const casa::Vector< casa::Double > &frequency_list_CF = itsConvFunc->get_frequency_list();

  double w_step = 2*itsConvFunc->get_w_from_support();
  cout << "w_step: " << w_step << endl;

  VisibilityMap v = make_mapping(vb, frequency_list_CF, itsTimeWindow, w_step);
  
  for (int w_plane=0; w_plane <= v.max_w_plane; w_plane++)
  {
    double w_offset = (w_plane+0.5) * w_step;
    
    bool model_grids_initialized = false;

    // disable w-stack
//     model_grids_initialized = true;
//     w_offset = 0;

    // get from w_plane
    
    for (std::vector<Chunk>::const_iterator chunk = v.chunks.begin() ; chunk != v.chunks.end(); ++chunk)
    {
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
        
        // Make sure the grid for this w-plane is initialized
        if (not model_grids_initialized)
        {
          cout << "w_plane: " << w_plane << endl;
          for (int i=0; i<itsNGrid; i++)
          {
            itsComplexModelImages[i]->set(Complex(0.0));

//             fill complex sub image from model image
            IPosition blc(
              4, 
              (itsPaddedNX - itsModelImages[i]->shape()(0) + (itsPaddedNX % 2 == 0)) / 2,
              (itsPaddedNY - itsModelImages[i]->shape()(1) + (itsPaddedNY % 2 == 0)) / 2,
              0, 
              0);
            IPosition shape(4, itsNX, itsNY, itsNPol, itsNChan);
            SubImage<Complex> complex_model_subimage(*itsComplexModelImages[i], Slicer(blc, shape), True);
            
            // convert float IQUV model image to complex image
            StokesImageUtil::From(complex_model_subimage, *itsModelImages[i]);
            
            normalize(complex_model_subimage, itsNormalizeModel, True);

            // Apply W term in image domain
            Array<Complex> complex_model_subimage_data;
            if (complex_model_subimage.get(complex_model_subimage_data))
            {
//               cout << "OK, it is a reference" << endl;
            }
            else
            {
//               cout << "Not OK, it is not a reference" << endl;
            }
              
            itsConvFunc->applyWterm(complex_model_subimage_data, w_offset);
            
            // transform to uv domain
//             LatticeFFT::cfft2d(*itsComplexModelImages[i], True);
            
            itsModelGrids[i].reference(itsComplexModelImages[i]->get());
            
            if (itsSplitBeam)
            {
//               itsConvFunc->applyElementBeam(itsModelGrids[i], False);
            }
            
            FFTCMatrix f;
            for(Int pol=0; pol<itsNPol; ++pol)
            {
              Complex* ptr = itsModelGrids[i].data() + pol*itsPaddedNX*itsPaddedNY;
              f.normalized_backward(itsPaddedNX, ptr, OpenMP::maxThreads());
            }
          }
          model_grids_initialized = true;
        }
          
        // get visibility from grid
        
        int idx = v.baseline_index_map[chunk->start];
        int ant1 = vb.antenna1()[idx];
        int ant2 = vb.antenna2()[idx];
        
        casa::Matrix<casa::Float> sum_weight;

        itsConvFunc->computeAterm (chunk->time);
        CFStore cfStore = itsConvFunc->makeConvolutionFunction (
          ant1, 
          ant2, 
          chunk->time,
          chunk->w,
          sum_weight,
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
          for (int taylor_idx = 0; taylor_idx<itsNGrid; taylor_idx++)
          {
            Vector<Double> taylor_weights(lsr_frequency.nelements());
            for (int j=0; j<lsr_frequency.nelements(); j++)
            {
              taylor_weights(j) = pow((lsr_frequency(j) - itsRefFreq)/itsRefFreq, taylor_idx);
            }
            itsVisResampler->GridToData(
              vbs, 
              itsModelGrids[taylor_idx], 
              v.baseline_index_map, 
              chunk->start,
              chunk->end, 
              cfStore,
              taylor_weights);
          }
        }
      }
    }
  }
}
  
} //# end namespace LofarFT
} //# end namespace LOFAR

