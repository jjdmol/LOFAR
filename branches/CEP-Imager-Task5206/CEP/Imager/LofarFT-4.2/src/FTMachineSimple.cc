//# FTMachineSimple.cc: Gridder for LOFAR data correcting for DD effects
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

#include <lofar_config.h>

#include <LofarFT/FTMachineSimple.h>
#include <LofarFT/VBStore.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/VisBuffer.h>
#include <LofarFT/ConvolutionFunction.h>

// #include <Common/LofarLogger.h>
// #include <Common/Exception.h>
#include <Common/OpenMP.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <measures/Measures/Stokes.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <scimath/Mathematics/RigidVector.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <measures/Measures/UVWMachine.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/OS/PrecTimer.h>
#include <casa/sstream.h>

using namespace casa;

const String LOFAR::LofarFT::FTMachineSimple::theirName("FTMachineSimple");

namespace
{
  bool dummy = LOFAR::LofarFT::FTMachineFactory::instance().
    registerClass<LOFAR::LofarFT::FTMachineSimple>(
      LOFAR::LofarFT::FTMachineSimple::theirName);
}

namespace LOFAR {
namespace LofarFT {

FTMachineSimple::FTMachineSimple(
  const MeasurementSet& ms,
//   Int nwPlanes,
//   MPosition mLocation, 
//   Float padding, 
//   Bool useDoublePrec,
  const Record& parameters)
  : FTMachine( ms, parameters),
    itsNThread(OpenMP::maxThreads())
{
  cout << "Constructing FTMachineSimple..." << endl;
  itsMachineName = "LofarFTMachineSimple";
  itsNGrid = itsNThread;
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedData.resize (itsNGrid);
  itsGriddedData2.resize (itsNGrid);
  itsSumPB.resize (itsNGrid);
  itsSumCFWeight.resize (itsNGrid);
  itsSumWeight.resize (itsNGrid);
}

FTMachineSimple::~FTMachineSimple()
{
  cout << "Destructing FTMachineSimple" << endl;
}

  //----------------------------------------------------------------------
FTMachineSimple& FTMachineSimple::operator=(const FTMachineSimple& other)
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
  FTMachineSimple::FTMachineSimple(const FTMachineSimple& other) : 
    FTMachine(other)
  {
    operator=(other);
  }

//----------------------------------------------------------------------
  FTMachineSimple* FTMachineSimple::clone() const
  {
    FTMachineSimple* newftm = new FTMachineSimple(*this);
    return newftm;
  }

//----------------------------------------------------------------------

void FTMachineSimple::put(const casa::VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type) 
{
  put( *static_cast<const VisBuffer*>(&vb), row, dopsf, type);
}

void FTMachineSimple::put(const VisBuffer& vb, Int row, Bool dopsf,
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
  Vector<Int> chan_map;
//   chan_map = match_channel(vb, itsImage);
  chan_map.resize();
  chan_map = Vector<Int>(vb.frequency().size(), 0);
  
  //No point in reading data if it's not matching in frequency
  if(max(chanMap)==-1) return;
  
  itsVisResampler->set_chan_map(chan_map);
  
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
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }

  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // Set up VBStore object to point to the relevant info of the VB.
  VBStore vbs;

  // Determine the baselines in the VisBuffer.
  const Vector<Int>& ant1 = vb.antenna1();
  const Vector<Int>& ant2 = vb.antenna2();
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  Vector<uInt> blIndex;
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
  vector<int> blStart, blEnd;
  blStart.reserve (nrant*(nrant+1)/2);
  blEnd.reserve   (nrant*(nrant+1)/2);
  Int  lastbl     = -1;
  Int  lastIndex  = 0;
  bool usebl      = false;
  bool allFlagged = true;
  const Vector<Bool>& flagRow = vb.flagRow();
  for (uint i=0; i<blnr.size(); ++i) 
  {
    Int inx = blIndex[i];
    Int bl = blnr[inx];
    if (bl != lastbl) 
    {
      // New baseline. Write the previous end index if applicable.
      if (usebl  &&  !allFlagged) 
      {
        double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[i-1]](2)));
        if (abs(Wmean) <= itsWMax) 
        {
          if (itsVerbose > 1) 
          {
            cout<<"using w="<<Wmean<<endl;
          }
          blStart.push_back (lastIndex);
          blEnd.push_back (i-1);
        }
      }
      // Skip auto-correlations and high W-values.
      // All w values are close, so if first w is too high, skip baseline.
      usebl = false;

      if (ant1[inx] != ant2[inx]) 
      {
	usebl = true;
      }
      lastbl=bl;
      lastIndex=i;
    }
    // Test if the row is flagged.
    if (! flagRow[inx]) {
      allFlagged = false;
    }
  }
  // Write the last end index if applicable.
  if (usebl  &&  !allFlagged) {
    double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[blnr.size()-1]](2)));
    if (abs(Wmean) <= itsWMax) {
      if (itsVerbose > 1) {
	cout<<"...using w="<<Wmean<<endl;
      }
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size()-1);
    }
  }
  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double time = 0.5 * (times[times.size()-1] + times[0]);

  vbs.nRow(vb.nRow());
  vbs.uvw(uvw);
  vbs.imagingWeightCube(imagingWeightCube);
  vbs.visCube(data);
  vbs.freq(vb.frequency());
  vbs.rowFlag(vb.flagRow());
  vbs.flagCube(vb.flagCube());

  itsVisResampler->setParams(itsUVScale, itsUVOffset, dphase);

  // First compute the A-terms for all stations (if needed).
  itsConvFunc->computeAterm (time);

  uInt Nchannels = vb.nChannel();

  itsTotalTimer.start();
  #pragma omp parallel 
  {
    // Thread-private variables.
    PrecTimer gridTimer;
    PrecTimer cfTimer;
    // The for loop can be parallellized. This must be done dynamically,
    // because the execution times of iterations can vary greatly.
//     #pragma omp for schedule(dynamic)
    
    for (int i=0; i<int(blStart.size()); ++i) 
    {
      int threadNum = OpenMP::threadNum();

      Int ist  = blIndex[blStart[i]];
      Int iend = blIndex[blEnd[i]];

//       compute average weight for baseline for CF averaging
      double average_weight(0.);
      uInt Nvis(0);
      for(Int j=ist; j<iend; ++j)
      {
        uInt row=blIndex[j];
        if(!vbs.rowFlag()[row])
        {
          Nvis+=1;
          for(uint k=0; k<Nchannels; ++k) 
          {
            // Temporary hack: should compute weight per polarization
            average_weight = average_weight + vbs.imagingWeightCube()(0,k,row);
          }
        }
      }
      average_weight=average_weight/Nvis;
      ///        itsSumWeight += average_weight * average_weight;
      if (itsVerbose > 1) 
      {
        cout<<"average weights= "<<average_weight<<", Nvis="<<Nvis<<endl;
      }


//       Get the convolution function.
      if (itsVerbose > 1) 
      {
        cout.precision(20);
        cout<<"A1="<<ant1[ist]<<", A2="<<ant2[ist]<<", time="<<fixed<<time<<endl;
      }
      CFStore cfStore;
      cfTimer.start();
      cfStore = itsConvFunc->makeConvolutionFunction (
        ant1[ist], 
        ant2[ist], 
        time,
        0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend)),
        itsGridMuellerMask, 
        false,
        average_weight,
        itsSumPB[threadNum],
        itsSumCFWeight[threadNum]);
      cfTimer.stop();

      if (itsUseDoubleGrid) 
      {
        itsVisResampler->DataToGrid(
          itsGriddedData2[threadNum], 
          vbs, 
          blIndex,
          blStart[i], 
          blEnd[i],
          itsSumWeight[threadNum], 
          dopsf, 
          cfStore);
      } 
      else 
      {
        if (itsVerbose > 1) 
        {
          cout<<"  gridding"<<" thread="<<threadNum<<'('<<itsNThread<<"), A1="<<ant1[ist]<<", A2="<<ant2[ist]<<", time=" <<time<<endl;
        }
        gridTimer.start();
        itsVisResampler->DataToGrid(
          itsGriddedData[threadNum], 
          vbs, 
          blIndex, 
          blStart[i],
          blEnd[i], 
          itsSumWeight[threadNum], 
          dopsf, 
          cfStore);
        gridTimer.stop();
      }
    } // end omp for
    double cftime = cfTimer.getReal();
#pragma omp atomic
    itsCFTime += cftime;
    double gtime = gridTimer.getReal();
#pragma omp atomic
    itsGriddingTime += gtime;
  } // end omp parallel
  itsTotalTimer.stop();
}


void FTMachineSimple::get(casa::VisBuffer& vb, Int row)
{
  get(*static_cast<VisBuffer*>(&vb), row);
}

// Degrid
void FTMachineSimple::get(VisBuffer& vb, Int row)
{
  if (itsVerbose > 0) {
    cout<<"///////////////////// GET!!!!!!!!!!!!!!!!!!"<<endl;
  }
//   gridOk(itsGridder->cSupport()(0));
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;}
  else         { nRow=1; startRow=row; endRow=row; }

  // Get the uvws in a form that Fortran can use
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())  matchAllSpwChans(vb);


  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()])
    matchChannel(vb.spectralWindow(), vb);
  else
    {
      chanMap.resize();
      chanMap=multiChanMap_p[vb.spectralWindow()];
    }

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1)    return;

  Cube<Complex> data;
  Cube<Bool> flags;
  getInterpolateArrays(vb, data, flags);

  VBStore vbs;
  vbs.nRow(vb.nRow());
  vbs.beginRow(0);
  vbs.endRow(vbs.nRow());

  vbs.uvw(uvw);
  //    vbs.imagingWeight.reference(elWeight);
  vbs.visCube(data);
  
  vbs.freq(interpVisFreq_p);
  vbs.rowFlag(vb.flagRow());
  vbs.flagCube(flags);

  // Determine the terms of the Mueller matrix that should be calculated
  itsVisResampler->setParams(itsUVScale, itsUVOffset, dphase);
  itsVisResampler->setMaps(chanMap, polMap);

  // Determine the baselines in the VisBuffer.
  const Vector<Int>& ant1 = vb.antenna1();
  const Vector<Int>& ant2 = vb.antenna2();
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  Vector<uInt> blIndex;
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
  vector<int> blStart, blEnd;
  blStart.reserve (nrant*(nrant+1)/2);
  blEnd.reserve   (nrant*(nrant+1)/2);
  Int  lastbl     = -1;
  Int  lastIndex  = 0;
  bool usebl      = false;
  bool allFlagged = true;
  const Vector<Bool>& flagRow = vb.flagRow();
  for (uint i=0; i<blnr.size(); ++i) {
    Int inx = blIndex[i];
    Int bl = blnr[inx];
    if (bl != lastbl) {
      // New baseline. Write the previous end index if applicable.
      if (usebl  &&  !allFlagged) {
        double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[i-1]](2)));
        if (abs(Wmean) <= itsWMax) {
	  if (itsVerbose > 1) {
	    cout<<"using w="<<Wmean<<endl;
	  }
	  blStart.push_back (lastIndex);
	  blEnd.push_back (i-1);
        }
      }
      // Skip auto-correlations and high W-values.
      // All w values are close, so if first w is too high, skip baseline.
      usebl = false;

      if (ant1[inx] != ant2[inx]) {
        usebl = true;
      }
      lastbl=bl;
      lastIndex=i;
    }
    // Test if the row is flagged.
    if (! flagRow[inx]) 
    {
      allFlagged = false;
    }
  }
  // Write the last end index if applicable.
  if (usebl  &&  !allFlagged) 
  {
    double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[blnr.size()-1]](2)));
    if (abs(Wmean) <= itsWMax) {
      if (itsVerbose > 1) 
      {
        cout<<"...using w="<<Wmean<<endl;
      }
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size()-1);
    }
  }

  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double time = 0.5 * (times[times.size()-1] + times[0]);

  // First compute the A-terms for all stations (if needed).
  itsConvFunc->computeAterm (time);

  itsTotalTimer.start();
  #pragma omp parallel
  {
    // Thread-private variables.
    PrecTimer degridTimer;
    PrecTimer cfTimer;
    // The for loop can be parallellized. This must be done dynamically,
    // because the execution times of iterations can vary greatly.
    #pragma omp for schedule(dynamic)
    for (int i=0; i<int(blStart.size()); ++i) {
      // #pragma omp critical(FTMachineSimple_lofarGridToData)
      // {
      Int ist  = blIndex[blStart[i]];
      Int iend = blIndex[blEnd[i]];
      int threadNum = OpenMP::threadNum();
      // Get the convolution function for degridding.
      if (itsVerbose > 1) {
	cout<<"ANTENNA "<<ant1[ist]<<" "<<ant2[ist]<<endl;
      }
      cfTimer.start();
      CFStore cfStore = itsConvFunc->makeConvolutionFunction (
        ant1[ist], 
        ant2[ist], 
        time,
        0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend)),
        itsDegridMuellerMask,
        true,
        0.0,
        itsSumPB[threadNum],
        itsSumCFWeight[threadNum]);
      cfTimer.stop();

      //Double or single precision gridding.
      //      cout<<"GRID "<<ant1[ist]<<" "<<ant2[ist]<<endl;
      degridTimer.start();
      itsVisResampler->GridToData(vbs, itsGriddedData[0],
                                      blIndex, blStart[i], blEnd[i], cfStore);
      degridTimer.stop();
    } // end omp for
    double cftime = cfTimer.getReal();
    #pragma omp atomic
    itsCFTime += cftime;
    double gtime = degridTimer.getReal();
    #pragma omp atomic
    itsGriddingTime += gtime;
  } // end omp parallel
  itsTotalTimer.stop();
  this->interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);
}

} //# end namespace LofarFT
} //# end namespace LOFAR

