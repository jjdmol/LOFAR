//# LofarFTMachineSplitBeamWStackSingleGrid.cc: Gridder for LOFAR data correcting for DD effects
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

#include <LofarFT/FTMachineWsplit.h>

#include <time.h>

#include <Common/OpenMP.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/PrecTimer.h>
#include <casa/OS/HostInfo.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/sstream.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/CompositeNumber.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/UVWMachine.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisSet.h>

#include <LofarFT/FFTCMatrix.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarVBStore.h>

#define DORES True

using namespace casa;

namespace 
{
  void sumGridsOMP(casa::Array<Complex>& grid, const vector< casa::Array<Complex> >& gridToAdd0 );
  void sumGridsOMP(casa::Array<DComplex>& grid, const vector< casa::Array<DComplex> >& gridToAdd0 );
  void sumGridsOMP(casa::Array<Complex>& grid, const casa::Array<Complex>& gridToAdd);
  void sumGridsOMP(casa::Array<DComplex>& grid, const casa::Array<DComplex>& gridToAdd);
}

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN

FTMachineWsplit::FTMachineWsplit(
  const MeasurementSet& ms, 
  Int nwPlanes,
  MPosition mLocation, 
  Float padding, 
  Bool useDoublePrec, 
  
  Double refFreq, //
  const Record& parameters)//,
  : FTMachine( ms, nwPlanes, mLocation, padding, useDoublePrec, parameters),
    machineName_p("LofarFTMachineWsplit"), 
    thisterm_p(0)
{
  cout << "=======LofarFTMachineWSplit==============================" << endl;
  cout << itsParameters << endl;
  cout << "=========================================================" << endl;

  logIO() << LogOrigin(this-name(), this-name())  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  itsNThread = OpenMP::maxThreads();
  AlwaysAssert (itsNThread>0, AipsError);
  itsNumCycle=0;
  its_makeDirtyCorr=MakeDirtyCorr;
  its_TimeWindow=itsParameters.asDouble("timewindow");
  its_UseWSplit=itsParameters.asBool("UseWSplit");
  its_SingleGridMode=itsParameters.asBool("SingleGridMode");
  its_NGrids=itsNThread;
  if(its_SingleGridMode){its_NGrids=1;}
  its_t0=-1.;
  its_tSel0=itsParameters.asDouble("t0");
  its_tSel1=itsParameters.asDouble("t1");

  itsRefFreq=RefFreq;
  itsNamePsfOnDisk=PsfOnDisk;
  its_Use_Linear_Interp_Gridder=Use_Linear_Interp_Gridder;
  its_Use_EJones=Use_EJones;
  its_UseMasksDegrid=UseMasksDegrid;
  its_PBCut=PBCut;
  its_reallyDoPSF=reallyDoPSF;
  //its_FillFactor=FillFactor;
  itsStepApplyElement=StepApplyElement;
  its_Apply_Element=false;
  its_ApplyBeamCode=ApplyBeamCode;
  itsPredictFT=PredictFT;
  itsUVmin=UVmin;
  itsUVmax=UVmax;
  itsSeconds.mark();
  itsSupport_Speroidal = FFTCMatrix::optimalOddFFTSize (itsParameters.asDouble("SpheSupport"));
  
  its_tot_time_grid=0.;
  its_tot_time_cf=0.;
  its_tot_time_w=0.;
  its_tot_time_el=0.;
  its_tot_time_tot=0;


  if(itsStepApplyElement>0){its_Apply_Element=true;}

  // if(its_Use_Linear_Interp_Gridder){
  //   cout<<"Gridding using oversampling of 1 only"<<endl;
  //   itsOversample=1;
  // };
  //cout<<"FTMahin: itsRefFreq "<<itsRefFreq<<endl;

  ROMSSpWindowColumns window(ms.spectralWindow());
  itsListFreq.resize(window.nrow());
  for(uInt i=0; i<window.nrow();++i){
    itsListFreq[i]=window.refFrequency()(i);
    cout<<"SPW="<<i<<", freq="<<itsListFreq[i]<<endl;
    if (itsVerbose > 0) {
      for(uInt j=0; j<(window.chanFreq()(i)).shape()[0];++j){
        cout<<"chan"<<(window.chanFreq()(i))[j]<<endl;
      }
    }
  }
  its_Already_Initialized=false;
}


//----------------------------------------------------------------------
FTMachineWsplit& FTMachineWsplit::operator=(const FTMachineWsplit& other)
{
  if(this!=&other) {
    //Do the base parameters
    FTMachine::operator=(other);

    //private params
    imageCache=other.imageCache;
    cachesize=other.cachesize;
    tilesize=other.tilesize;
    convType=other.convType;
    uvScale.resize();
    uvOffset.resize();
    uvScale=other.uvScale;
    uvOffset=other.uvOffset;
    if(other.gridder==0)
      gridder=0;
    else{
      gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						     uvScale, uvOffset,
						     convType);
    }
    isTiled=other.isTiled;
    //lattice=other.lattice;
    lattice=0;
    tilesize=other.tilesize;
    arrayLattice=0;
    maxAbsData=other.maxAbsData;
    centerLoc=other.centerLoc;
    offsetLoc=other.offsetLoc;
    padding_p=other.padding_p;
    usezero_p=other.usezero_p;
    noPadding_p=other.noPadding_p;
    itsMS = other.itsMS;
    itsNWPlanes = other.itsNWPlanes;
    itsWMax = other.itsWMax;
    itsConvFunc = other.itsConvFunc;
    //cyrr: mfs
    itsRefFreq=other.itsRefFreq;
    thisterm_p=other.thisterm_p;
    its_Use_Linear_Interp_Gridder= other.its_Use_Linear_Interp_Gridder;
    its_Use_EJones= other.its_Use_EJones;
    its_UseMasksDegrid=other.its_UseMasksDegrid;
    its_Apply_Element= other.its_Apply_Element;
    its_ApplyBeamCode=other.its_ApplyBeamCode;
    itsStepApplyElement=other.itsStepApplyElement;
    its_SingleGridMode=other.its_SingleGridMode;
    its_Already_Initialized= other.its_Already_Initialized;
    its_reallyDoPSF = other.its_reallyDoPSF;
    its_PBCut= other.its_PBCut;
    its_NGrids=other.its_NGrids;
    itsStackMuellerNew=other.itsStackMuellerNew;
    itsDonePB=other.itsDonePB;
    itsUVmin=other.itsUVmin;
    itsUVmax=other.itsUVmax;
    itsListFreq=other.itsListFreq;
    itsNumCycle=other.itsNumCycle;
    its_makeDirtyCorr=other.its_makeDirtyCorr;
    its_UseWSplit=other.its_UseWSplit;
    //its_FillFactor=other.its_FillFactor;
     //cyrr: mfs
    its_t0=other.its_t0;
    its_tSel0=other.its_tSel0;
    its_tSel1=other.its_tSel1;
    its_tot_time_grid =other.its_tot_time_grid ;
    its_tot_time_cf   =other.its_tot_time_cf   ;
    its_tot_time_w    =other.its_tot_time_w    ;
    its_tot_time_el   =other.its_tot_time_el   ;
    its_tot_time_tot  =other.its_tot_time_tot ;

    its_TimeWindow=other.its_TimeWindow;

    ConjCFMap_p = other.ConjCFMap_p;
    CFMap_p = other.CFMap_p;
    itsNThread = other.itsNThread;
    itsGriddedData=other.itsGriddedData;
    itsGriddedData2=other.itsGriddedData2;//.resize (itsNThread);
    itsSumPB.resize (itsNThread);
    itsSumCFWeight.resize (itsNThread);
    itsSumWeight.resize (itsNThread);
    itsVerbose = other.itsVerbose;
    itsMaxSupport = other.itsMaxSupport;
    itsOversample = other.itsOversample;
    itsPredictFT = other.itsPredictFT;
    itsImgName = other.itsImgName;
    itsGridMuellerMask = other.itsGridMuellerMask;
    itsDegridMuellerMask = other.itsDegridMuellerMask;
    itsGriddingTime = other.itsGriddingTime;
    itsDegriddingTime = other.itsDegriddingTime;
    itsCFTime = other.itsCFTime;
    itsParameters = other.itsParameters;
    itsSupport_Speroidal=other.itsSupport_Speroidal;
  }
  return *this;
}

//----------------------------------------------------------------------
  FTMachineWsplit::FTMachineWsplit(const FTMachineWsplit& other) : FTMachine(), machineName_p("LofarFTMachineWsplit")
  {
    operator=(other);
  }

  FTMachineWsplit* LofarFTMachineWsplit::clone() const
  {
    FTMachineWsplit* newftm = new FTMachineWsplit(*this);
    return newftm;
  }

//----------------------------------------------------------------------
void FTMachineWsplit::init() {

  logIO() << LogOrigin(this-name(), "init")  << LogIO::NORMAL;
  canComputeResiduals_p = DORES;
  ok();

  isTiled=False;
  nx = image->shape()(0);
  ny = image->shape()(1);
  if (!noPadding_p) {
    // Make sure padding is always even.
    nx += int((padding_p-1)*nx/2.)*2;
    ny += int((padding_p-1)*ny/2.)*2;
  }
  npol  = image->shape()(2);
  nchan = image->shape()(3);

  uvScale.resize(3);
  uvScale=0.0;
  uvScale(0)=Float(nx)*image->coordinates().increment()(0);
  uvScale(1)=Float(ny)*image->coordinates().increment()(1);
  uvScale(2)=Float(1)*abs(image->coordinates().increment()(0));

  uvOffset.resize(3);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;
  uvOffset(2)=0;

  padded_shape = image->shape();
  padded_shape(0) = nx;
  padded_shape(1) = ny;
  cout << "Original shape " << image->shape()(0) << ","
        << image->shape()(1) << endl;
  cout << "Padded shape " << padded_shape(0) << ","
        << padded_shape(1) << endl;
  fftw_init_threads();
  fftw_plan_with_nthreads(1);
  itsConvFunc = new ConvolutionFunction(
    padded_shape,
    image->shape(),
    image->coordinates().directionCoordinate 
    (image->coordinates().findCoordinate(Coordinate::DIRECTION)),
    itsMS, 
    itsNWPlanes, 
    itsWMax,
    itsOversample,
    itsVerbose, 
    itsMaxSupport,
    itsImgName+String::toString(thisterm_p),
    itsUseEJones,
    itsApplyElement,
    itsApplyBeamCode,
    itsParameters,
    itsStackMuellerNew);
  
  itsNWPlanes = itsConvFunc->m_nWPlanes;

  VecFFTMachine.resize(image->shape()(2));
  Matrix<Complex> dummy(IPosition(2,nx,nx),0.);
  cout<<" initialise plans"<<endl;
  for(Int pol=0; pol<npol; ++pol)
  {
      VecFFTMachine[pol].normalized_forward(dummy.nrow(),dummy.data(),OpenMP::maxThreads()/npol , FFTW_MEASURE);
      VecFFTMachine[pol].normalized_backward(dummy.nrow(),dummy.data(),OpenMP::maxThreads()/npol , FFTW_MEASURE);
  }
  cout<<" done..."<<endl;

  itsMasksGridAllDone=false;
  itsAllAtermsDone=false;
  its_Already_Initialized=true;

  itsCyrilTimer.start();
  itsTStartObs=1.e30;
  itsDeltaTime=0.;
  itsNextApplyTime=0.;;
  itsCounterTimes=0;
}

// This is nasty, we should use CountedPointers here.
FTMachineWsplit::~FTMachineWsplit()
{
}

void FTMachineSplitBeamWStackSingleGrid::put(
  const VisBuffer& vb, 
  Int row, 
  Bool dopsf,
  FTMachine::Type type)
{
  itsCyrilTimer.stop();
  logIO() << LogOrigin(this-name(), "put") << LogIO::NORMAL
          << "I am gridding " << vb.nRow() << " row(s)."  << LogIO::POST;
  logIO() << LogIO::NORMAL << "Padding is " << padding_p  << LogIO::POST;


  //Check if ms has changed then cache new spw and chan selection
  if (vb.newMS()) 
  {
    matchAllSpwChans(vb);
  }

  //Here we redo the match or use previous match

  //Channel matching for the actual spectral window of buffer
  if (doConversion_p[vb.spectralWindow()]) 
  {
    matchChannel(vb.spectralWindow(), vb);
  } 
  else 
  {
    chanMap.resize();
    chanMap = multiChanMap_p[vb.spectralWindow()];
  }

  uInt spw(vb.spectralWindow());

  //No point in reading data if it's not matching in frequency
  if(max(chanMap)==-1) return;

  const Matrix<Float> *imagingweight;
  imagingweight=&(vb.imagingWeight());

  if (its_reallyDoPSF) 
  {
    dopsf=true;
  }
  if (dopsf) 
  {
    type = FTMachine::PSF;
  }
  Cube<Complex> data;
  //Fortran gridder need the flag as ints
  Cube<Int> flags;
  Matrix<Float> elWeight;
  interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);

  Int startRow, endRow, nRow;
  if (row==-1) 
  { 
    nRow=vb.nRow(); 
    startRow = 0; 
    endRow = nRow-1; 
  }
  else
  {
    nRow=1; 
    startRow = row;
    endRow=row;
  }

  // Get the uvws in a form that Fortran can use and do that
  // necessary phase rotation. On a Pentium Pro 200 MHz
  // when null, this step takes about 50us per uvw point. This
  // is just barely noticeable for Stokes I continuum and
  // irrelevant for other cases.
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;

  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) 
  {
    for (Int idim=0;idim<2;idim++) 
    {
      uvw(idim,i) = -vb.uvw()(i)(idim);
    }
    uvw(2,i)=vb.uvw()(i)(2);
  }

  // Set up VBStore object to point to the relevant info of the VB.
  LofarVBStore vbs;
  makeCFPolMap(vb,cfStokes_p,CFMap_p);
  makeConjPolMap(vb,CFMap_p,ConjCFMap_p);

  cout<<"do mapping"<<endl;
  vector<vector<vector<vector<uInt> > > > MapBlTimesW_grid = LofarFTMachine::make_mapping_time_W_grid(vb, spw);

  cout<<"done"<<endl;

  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double timeChunk = 0.5 * (times[times.size()-1] + times[0]);

  vbs.nRow_p = vb.nRow();
  vbs.uvw_p.reference(uvw);
  vbs.imagingWeight_p.reference(elWeight);
  vbs.visCube_p.reference(data);
  vbs.freq_p.reference(interpVisFreq_p);
  vbs.rowFlag_p.reference(vb.flagRow());

  // Really nice way of converting a Cube<Int> to Cube<Bool>.
  // However the VBS objects should ultimately be references
  // directly to bool cubes.
  //**************


  vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;
  //  vbs.flagCube_p.reference(vb.flagCube());
  //**************

   // Determine the terms of the Mueller matrix that should be calculated
  visResamplers_p.setParams(uvScale,uvOffset,dphase);
  visResamplers_p.setMaps(chanMap, polMap);

  // First compute the A-terms for all stations (if needed).
  PrecTimer CyrilTimer2Aterm;
  CyrilTimer2Aterm.start();

  if(!itsAllAtermsDone)
  {
    cout<<"Calc Aterm"<<endl;
    itsConvFunc->computeAterm (timeChunk);
    itsConvFunc->computeVecAterm(times[0],times[times.size()-1],its_TimeWindow);
    cout<<"... done"<<endl;
  }
  CyrilTimer2Aterm.stop();
  double Taterm=CyrilTimer2Aterm.getReal();

  uInt Nchannels = vb.nChannel();
  
  itsTotalTimer.start();

  Int doagain(0);

  PrecTimer CyrilElement;
  PrecTimer CyrilW;
  PrecTimer CyrilTimer2gridconv;
  CyrilTimer2gridconv.start();

  Array<Complex> tmp_stacked_GriddedData;
  tmp_stacked_GriddedData.resize ((*itsGriddedData)[0].shape());
  tmp_stacked_GriddedData=Complex();
  Array<Complex> tmp_stacked_stacked_GriddedData;
  tmp_stacked_stacked_GriddedData.resize ((*itsGriddedData)[0].shape());
  tmp_stacked_stacked_GriddedData=Complex();

  Vector<uInt> MapChanBlock((itsConvFunc->map_chan_Block_buffer).copy());
  visResamplers_p.setChanCFMaps(MapChanBlock);
  Vector<uInt> BlockCF((itsConvFunc->map_spw_chanBlock[spw]).copy());

  vector<double> time_grid;
  vector<double> time_conv;
  vector<uInt> nvis;
  double time_w(0);
  vector<uInt> nchunks;
  Int countw(0);
  time_grid.resize(itsNThread);
  time_conv.resize(itsNThread);
  nchunks.resize(itsNThread);
  nvis.resize(itsNThread);
  for(Int iii = 0; iii<itsNThread; ++iii)
  {
    time_grid[iii] = 0;
    time_conv[iii] = 0;
    nchunks[iii] = 0;
    nvis[iii] = 0;
  }

  for(uInt iwplane = 0; iwplane<MapBlTimesW_grid.size(); ++iwplane)
  {
    Int w_index=WIndexMap[iwplane];
    if (itsVerbose > 0) 
    {
      cout<<"  put::plane "<<iwplane<<" with windex= "<<w_index<<endl;
      cout<<"  put::grid ..."<<endl;
    }
    countw+=1;
    for(uInt igrid=0; igrid<MapBlTimesW_grid[iwplane].size(); ++igrid)
    {
      vector<vector<uInt > > MapBlTimes=MapBlTimesW_grid[iwplane][igrid];
      vector< Bool> done;
      done.resize(int(MapBlTimesW_grid[iwplane][igrid].size()));
      for(int i=0; i<int(MapBlTimesW_grid[iwplane][igrid].size()); ++i) 
      {
        done[i]=false;
        
      };
      Bool all_done(false);
      
      while(!all_done){
      #pragma omp parallel
      {
        // Thread-private variables.
        PrecTimer gridTimer;
        PrecTimer cfTimer;
        // The for loop can be parallellized. This must be done dynamically,
        // because the execution times of iterations can vary greatly.

        #pragma omp for schedule(dynamic)
        for (int i=0; i<int(MapBlTimes.size()); ++i) 
        {
          if((done[i]==true)|(MapBlTimes[i].size()==0))
          {
            continue;
          };
          Int ChunkSize(MapBlTimes[i].size());
          Int ist  = MapBlTimes[i][0];
          Int iend = MapBlTimes[i][ChunkSize-1];
          
          try
          {
            double average_weight=0.;
            if (itsDonePB==false)
            {
              uInt Nvis=0;
              for(Int j=0; j<ChunkSize; ++j)
              {
                uInt row=MapBlTimes[i][j];
                if(!vbs.rowFlag()[row])
                {
                  Nvis+=1;
                  for(uInt k=0; k<Nchannels; ++k) 
                  {
                    average_weight=average_weight+vbs.imagingWeight()(k,row);
                  }
                }
              }
              if (Nvis>0) 
              {
                average_weight=average_weight/Nvis;
              } 
              else 
              {
                average_weight=0.;
              }
              if (itsVerbose > 1) 
              {
                cout<<"average weights= "<<average_weight<<", Nvis="<<Nvis<<endl;
              }
            }

            int threadNum = OpenMP::threadNum();
            nchunks[threadNum]+=1;
            nvis[threadNum]+=(MapBlTimes[i]).size();
            PrecTimer CyrilTimer2grid;
            PrecTimer CyrilTimer2conv;
            CyrilTimer2conv.start();
            cfTimer.start();
            Double Wmean=0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend));
            Double TimeMean=0.5*(times[ist] + times[iend]);

            Vector<uInt> BlockCFlala;
            BlockCFlala.resize(BlockCF.size());
            for(uInt cc=0; cc<BlockCF.size(); ++cc)
            {
              BlockCFlala[cc]=BlockCF[cc];
            }
            

            LofarCFStore cfStore = itsConvFunc->makeConvolutionFunction (
              ant1[ist], 
              ant2[ist], 
              TimeMean,
              Wmean,
              itsGridMuellerMask, 
              false,
              average_weight,
              itsSumPB[threadNum],
              itsSumCFWeight[threadNum],
              BlockCFlala,
              thisterm_p,
              itsRefFreq,
              itsStackMuellerNew[threadNum],
              0, 
              false);

              CyrilTimer2conv.stop();
              time_conv[threadNum]+=CyrilTimer2conv.getReal();
              
              Int nConvX = (*(cfStore.vdata))[0][0][0].shape()[0];
              double cfstep=CyrilTimer2conv.getReal();
              CyrilTimer2grid.start();
              gridTimer.start();
              visResamplers_p.lofarDataToGrid_interp((*itsGriddedData)[0], vbs, MapBlTimes[i], itsSumWeight[threadNum], dopsf, cfStore);
              gridTimer.stop();

              CyrilTimer2grid.stop();
              time_grid[threadNum]+=CyrilTimer2grid.getReal();
              CyrilTimer2grid.reset();
              CyrilTimer2conv.reset();
              
              done[i]=true;
            } 
            catch (std::bad_alloc &)
            {
              cout << "-----------------------------------------"<< endl;
              cout << "!!!!!!! GRIDDING: Skipping baseline: "<< ant1[ist] << " | " << ant2[ist] << endl;
              cout << "memoryUsed() " << HostInfo::memoryUsed() << ", Free: "<< HostInfo::memoryFree() << endl;
              cout << "-----------------------------------------" << endl;
            };
          } // end omp for
          double cftime = cfTimer.getReal();
          #pragma omp atomic
          itsCFTime += cftime;
          double gtime = gridTimer.getReal();
          #pragma omp atomic
          itsGriddingTime += gtime;
        } // end omp parallel
        all_done=true;
        int number_missed(0);
        for (int i=0; i<int(blStart.size()); ++i) 
        {
          if (done[i]==false)
          {
            all_done = false;
            number_missed+=1;
          };
        };
        if (all_done==false) {
          doagain += 1;
        };

      }//end While loop

    }//end grids loop
    tmp_stacked_GriddedData = Complex();
    SumGridsOMP(tmp_stacked_GriddedData, (*itsGriddedData));

    //FFTConvolution
      
    cout<<"  put::apply-W"<<endl;
    CyrilW.start();
    
    #pragma omp parallel
    {
      #pragma omp for
      for(Int pol=0; pol<npol; ++pol)
      {
        Complex* ptr = tmp_stacked_GriddedData.data()+pol*nx*nx;
        Matrix<Complex> arr(IPosition(2,nx,nx), ptr, SHARE);
        dofftVec(arr,false,itsNThread/npol,pol);
      }
    }
    fftw_plan_with_nthreads(1);
      
    itsConvFunc->ApplyWterm_Image(tmp_stacked_GriddedData, tmp_stacked_GriddedData, spw, false, w_index);
    CyrilW.stop();
    cout << "  Wtime = " << CyrilW.getReal() << endl;
    time_w+=CyrilW.getReal();
    CyrilW.reset();
    SumGridsOMP(tmp_stacked_stacked_GriddedData, tmp_stacked_GriddedData);
    
    #pragma omp parallel for schedule(dynamic)
    for (uInt i=0; i<its_NGrids; ++i) 
    {
      (*itsGriddedData)[i]=Complex();
    }
  }//end for w-planes

  CyrilElement.start();
  if(its_Apply_Element){
    tmp_stacked_stacked_GriddedData.reference(itsConvFunc->ApplyElementBeam_Image(tmp_stacked_stacked_GriddedData, timeChunk, spw,
                                                                                  itsGridMuellerMask, false));
  }
      
  #pragma omp parallel
  {
    #pragma omp for
    for(Int pol=0; pol<npol; ++pol)
    {
      Complex* ptr = tmp_stacked_stacked_GriddedData.data()+pol*nx*nx;
      Matrix<Complex> arr(IPosition(2,nx,nx), ptr, SHARE);
      dofftVec(arr,true,itsNThread/npol,pol);
    }
  }
  fftw_plan_with_nthreads(1);
  CyrilElement.stop();

  double tot_time_grid(0);
  double tot_time_cf(0);
  double tot_chunk(0);
  double tot_nvis(0);

  for(Int iii=0; iii<itsNThread; ++iii)
  {
    tot_time_grid+=time_grid[iii];
    tot_time_cf+=time_conv[iii];
    tot_chunk+=nchunks[iii];
    tot_nvis+=nvis[iii];
  }

  cout<<"Nvis_El= "<<tot_nvis<<endl;
  cout<<"Nchunk = "<<tot_chunk<<endl;
  cout<<"Tcf    = "<<tot_time_cf<<endl;
  cout<<"Tgrid  = "<<tot_time_grid<<endl;
  cout<<"Tel    = "<<CyrilElement.getReal()<<endl;
  cout<<"Tw     = "<<time_w<<endl;
  cout<<"wmax   = "<<itsWMax<<endl;
  cout<<"Diam   = "<<abs(uvScale(0))<<endl;
  cout<<"Ns     = "<<itsSupport_Speroidal<<endl;
  cout<<"Nw     = "<<itsConvFunc->m_nWPlanes<<endl;

  its_tot_time_grid += tot_time_grid/itsNThread;
  its_tot_time_cf   += tot_time_cf/itsNThread;
  its_tot_time_w    += time_w;
  its_tot_time_el   += CyrilElement.getReal();
  its_tot_time_tot  = its_tot_time_grid+its_tot_time_cf+its_tot_time_w+its_tot_time_el;

  cout.precision(5);
  cout<<"cf="<<100*its_tot_time_cf/its_tot_time_tot<<"%, grid="<<100*its_tot_time_grid/its_tot_time_tot<<"%, el="<<100*its_tot_time_el/its_tot_time_tot<<"%, w="<<100*its_tot_time_w/its_tot_time_tot<<"%, tot="<<its_tot_time_tot<<endl;


  if (itsVerbose > 0) 
  {
    ofstream outFile("outputTimes.txt");
    outFile<<tot_nvis<<endl;
    outFile<<tot_chunk<<endl;
    outFile<<tot_time_cf<<endl;
    outFile<<tot_time_grid<<endl;
    outFile<<CyrilElement.getReal()<<endl;
    outFile<<time_w<<endl;
    outFile<<itsWMax<<endl;
    outFile<<abs(uvScale(0))<<endl;
    outFile<<itsSupport_Speroidal<<endl;
  }
  CyrilElement.reset();

  cout << "  put::apply-W ...done"<<endl;
  cout << endl;
  SumGridsOMP(its_stacked_GriddedData, tmp_stacked_stacked_GriddedData);
  itsTotalStepsGrid+=1;
}

void LofarFTMachine::get(VisBuffer& vb, Int row)
{
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) 
  { 
    nRow=vb.nRow(); 
    startRow=0; 
    endRow=nRow-1;
  }
  else
  {
    nRow=1; 
    startRow=row; 
    endRow=row; 
  }

  // Get the uvws in a form that Fortran can use
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) 
  {
    for (Int idim=0;idim<2;idim++) 
    {
      uvw(idim,i)=-vb.uvw()(i)(idim);
    }
    uvw(2,i)=vb.uvw()(i)(2);
  }

  //Check if ms has changed then cache new spw and chan selection
  if (vb.newMS())
  {
    matchAllSpwChans(vb);
  }
  
  uInt spw(vb.spectralWindow());

  logIO() << LogOrigin(this-name(), "get") << LogIO::NORMAL
          << "I am degridding " << vb.nRow() << " row(s)."  << LogIO::POST;

  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()])
  {
    matchChannel(vb.spectralWindow(), vb);
  }
  else
  {
    chanMap.resize();
    chanMap=multiChanMap_p[vb.spectralWindow()];
  }

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1) 
  {
    return;
  }

  Cube<Complex> data;
  Cube<Int> flags;
  getInterpolateArrays(vb, data, flags);

  Vector<uInt> MapChanBlock((itsConvFunc->map_chan_Block_buffer).copy());
  visResamplers_p.setChanCFMaps(MapChanBlock);
  Vector<uInt> BlockCF((itsConvFunc->map_spw_chanBlock[spw]).copy());

  LofarVBStore vbs;
  vbs.nRow_p = vb.nRow();
  vbs.beginRow_p = 0;
  vbs.endRow_p = vbs.nRow_p;

  vbs.uvw_p.reference(uvw);
  vbs.visCube_p.reference(data);

  vbs.freq_p.reference(interpVisFreq_p);
  vbs.rowFlag_p.resize(0); 
  vbs.rowFlag_p = vb.flagRow();
  
  // Really nice way of converting a Cube<Int> to Cube<Bool>.
  // However these should ultimately be references directly to bool
  // cubes.
  vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;

  // Determine the terms of the Mueller matrix that should be calculated
  visResamplers_p.setParams(uvScale,uvOffset,dphase);
  visResamplers_p.setMaps(chanMap, polMap);

  //LofarFTMachine::make_mapping(vb);
  vector<vector<vector<uInt> > > MapBlTimesW=LofarFTMachine::make_mapping_time_W(vb, spw);

  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double timeChunk = 0.5 * (times[times.size()-1] + times[0]);
  double MinTime = times[0];
  double MaxTime = times[times.size()-1];
  if (!itsAllAtermsDone)
  {
    itsConvFunc->computeAterm (timeChunk);
    itsConvFunc->computeVecAterm(times[0],times[times.size()-1],its_TimeWindow);
  }

  if (times[0]<itsTStartObs)
  {
    itsTStartObs=times[0];
  }

  // First compute the A-terms for all stations (if needed).

  itsTotalTimer.start();

  Array<Complex> GridToDegridElement;

  if (itsApplyElement)
  {
    GridToDegridElement.reference(itsConvFunc->ApplyElementBeam_Image (its_stacked_GriddedData, timeChunk, spw, itsGridMuellerMask, true));
  }
  else
  {
    GridToDegridElement = its_stacked_GriddedData.copy();
  }

  PrecTimer CyrilConv;
  PrecTimer CyrilGrid;

  itsGridToDegrid.resize(GridToDegridElement.shape());

  for (uInt iwplane=0; iwplane<MapBlTimesW.size(); ++iwplane)
  {
    vector<vector<uInt > > MapBlTimes=MapBlTimesW[iwplane];
    vector< Bool> done;
    done.resize(int(MapBlTimesW[iwplane].size()));
    for (int i=0; i<int(MapBlTimesW[iwplane].size()); ++i)
    {
      done[i]=false;
    }
    Bool all_done(false);
    Int w_index = WIndexMap[iwplane];
    cout << "  get:: apply-w " << iwplane << " " << w_index << endl;
    itsConvFunc->ApplyWterm_Image(GridToDegridElement, itsGridToDegrid, spw, true, w_index);

    #pragma omp parallel
    {
      #pragma omp for
      for(Int pol=0; pol<npol; ++pol)
      {
        Complex* ptr = itsGridToDegrid.data()+pol*nx*nx;
        Matrix<Complex> arr(IPosition(2,nx,nx), ptr, SHARE);
        VecFFTMachine[pol].forward(arr.nrow(),arr.data(),itsNThread/npol, FFTW_MEASURE);
      }
    }
    fftw_plan_with_nthreads(1);

    cout<<"  get:: apply-w .... done"<<endl;
    cout<<"  get:: degrid"<<endl;

    while(!all_done)
    {
      #pragma omp parallel
      {
        // Thread-private variables.
        PrecTimer degridTimer;
        PrecTimer cfTimer;
        // The for loop can be parallellized. This must be done dynamically,
        // because the execution times of iterations can vary greatly.
        #pragma omp for schedule(dynamic)
        for (int i=0; i<int(MapBlTimes.size()); ++i) 
        {
          if ((done[i]==true)|(MapBlTimes[i].size()==0)) 
          {
            continue;
          };
          Int ChunkSize(MapBlTimes[i].size());
          Int ist  = MapBlTimes[i][0];
          Int iend = MapBlTimes[i][ChunkSize-1];

          try 
          {
            int threadNum = OpenMP::threadNum();
            // Get the convolution function for degridding.
            if (itsVerbose > 1) {
              cout<<"ANTENNA "<<ant1[ist]<<" "<<ant2[ist]<<endl;
            }
            cfTimer.start();
            CyrilConv.start();
            Double TimeMean=0.5*(times[ist] + times[iend]);

            LofarCFStore cfStore;
            Vector<uInt> BlockCFlala;
            BlockCFlala.resize(BlockCF.size());
            for(uInt cc=0; cc<BlockCF.size(); ++cc)
            {
              BlockCFlala[cc]=BlockCF[cc];
            }

            cfStore = itsConvFunc->makeConvolutionFunction (
              ant1[ist], 
              ant2[ist], 
              TimeMean,
              0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend)),
              itsDegridMuellerMask,
              true,
              0.0,
              itsSumPB[threadNum],
              itsSumCFWeight[threadNum],
              BlockCFlala,thisterm_p,
              itsRefFreq,
              itsStackMuellerNew[threadNum],
              0,
              false);
          
            visResamplers_p.lofarGridToData_interp(
              vbs, 
              itsGridToDegrid,
              MapBlTimes[i], cfStore);
          
            cfTimer.stop();

            CyrilConv.stop();
            CyrilGrid.start();

            degridTimer.start();
            CyrilGrid.stop();

            degridTimer.stop();
            done[i]=true;

          } 
          catch (std::bad_alloc &)
          {
            cout << "-----------------------------------------" << endl;
            cout << "!!!!!!! DE-GRIDDING: Skipping baseline: " < <ant1[ist] << " | " <<ant2[ist] << endl;
            cout << "memoryUsed() "<< HostInfo::memoryUsed()<< ", Free: " << HostInfo::memoryFree() << endl;
            cout << "-----------------------------------------" << endl;
          }
        } // end omp for
        double cftime = cfTimer.getReal();
        #pragma omp atomic
        itsCFTime += cftime;
        double gtime = degridTimer.getReal();
        #pragma omp atomic
        itsGriddingTime += gtime;
      } // end omp parallel

      all_done = true;
      int number_missed(0);
      for (int i=0; i<int(blStart.size()); ++i) 
      {
        if (done[i]==false)
        {
          all_done = false;
          number_missed += 1;
        };
      };

    }//end While loop
    cout<<"  get:: done"<<endl;
    cout<<endl;
  }//end for wplanes loop

  itsTotalStepsDeGrid+=1;
  itsTotalTimer.stop();
  interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);

}

void FTMachineWsplit::initGridThreads(
  vector< casa::Array<casa::Complex> >&  otherGriddedData, 
  vector< casa::Array<casa::DComplex> >&  otherGriddedData2)
{
  itsGriddedData=&otherGriddedData;
  itsGriddedData2=&otherGriddedData2;
  (*itsGriddedData).resize (1);
  (*itsGriddedData2).resize (1);
}

} //# end namespace LOFAR

namespace 
{

  template <class T>
  void sumGridsOMP(casa::Array<T>& grid, const vector< casa::Array<T> >& gridToAdd0 )
  {
    for(casa::uInt vv=0; vv<gridToAdd0.size(); vv++)
    {
      casa::Array<T> gridToAdd(gridToAdd0[vv]);
      int y,ch,pol;
      int gridSize(grid.shape()[0]);
      int nPol(grid.shape()[2]);
      int nChan(grid.shape()[3]);
      T* gridPtr;
      const T* gridToAddPtr;
      
      #pragma omp parallel for private(y,ch,pol,gridPtr,gridToAddPtr)
      for(int x=0 ; x<grid.shape()[0] ; ++x)
      {
        for(ch=0 ; ch<nChan ; ++ch)
        {
          for(pol=0 ; pol<nPol ; ++pol)
          {
            gridPtr = grid.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
            gridToAddPtr = gridToAdd.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
            for(y=0 ; y<grid.shape()[1] ; ++y)
            {
               (*gridPtr++) += *gridToAddPtr++;
            }
          }
        }
      }
    }
  }
  
  //Sum Grids
  template <class T>
  void sumGridsOMP(casa::Array<T>& grid, const casa::Array<T>& gridToAdd){
    int y,ch,pol;
    int gridSize(grid.shape()[0]);
    int nPol(grid.shape()[2]);
    int nChan(grid.shape()[3]);
    T* gridPtr;
    const T* gridToAddPtr;
    
    #pragma omp parallel for private(y,ch,pol,gridPtr,GridToAddPtr)
    for(int x=0 ; x<grid.shape()[0] ; ++x)
    {
      for(ch=0 ; ch<nChan ; ++ch)
      {
        for(pol=0 ; pol<nPol ; ++pol)
        {
          gridPtr = grid.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
          gridToAddPtr = gridToAdd.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
          for(y=0 ; y<grid.shape()[1] ; ++y)
          {
            (*gridPtr++) += *gridToAddPtr++;
          }
        }
      }
    }
    
  }
  
} // end unnamed namespace

