//# LofarConvolutionFunction.cc: Compute the LOFAR convolution function
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
#include <LofarFT/LofarConvolutionFunction.h>
#include <Common/LofarLogger.h>
#include <Common/OpenMP.h>

#include <BBSKernel/MeasurementAIPS.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/sstream.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MeasTable.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/OS/PrecTimer.h>
#include <casa/sstream.h>
#include <iomanip>

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>

namespace LOFAR
{

  LofarConvolutionFunction::LofarConvolutionFunction
  (const IPosition& shape,
   const DirectionCoordinate& coordinates,
   const MeasurementSet& ms,
   uInt nW, double Wmax,
   uInt oversample,
   Int verbose,
   Int maxsupport,
   const String& imgName,
   Bool Use_EJones,
   Bool Apply_Element,
   int ApplyBeamCode,
   const casa::Record& parameters,
   vector< vector< vector < Matrix<Complex> > > > & StackMuellerNew
  )
  // ,
  //Int TaylorTerm,
    //Double RefFreq
    : m_nWPlanes(nW),
      itsParameters(parameters),
      m_shape(shape),
      m_coordinates(coordinates),
      m_aTerm(ms, parameters),
      m_maxW(Wmax), //maximum W set by ft machine to flag the w>wmax
      m_oversampling(oversample),
      itsVerbose (verbose),
      itsMaxSupport(maxsupport),
      itsImgName(imgName),
      itsTimeW    (0),
      itsTimeWpar (0),
      itsTimeWfft (0),
      itsTimeWcnt (0),
      itsTimeA    (0),
      itsTimeApar (0),
      itsTimeAfft (0),
      itsTimeAcnt (0),
      itsTimeCFpar(0),
      itsTimeCFfft(0),
      itsTimeCFcnt(0)
  {
    if (itsVerbose > 0) {
      cout<<"LofarConvolutionFunction:shape  "<<shape<<endl;
    }
    itsFFTMachines.resize (OpenMP::maxThreads());
    initStoreMasksNew();
    
    if(Apply_Element){
      if (itsVerbose > 0) {
        cout<<"m_shape "<<m_shape<<endl;
      }
      its_output_grid_element.resize(m_shape);
      its_ArrMatrix_out_element.resize(IPosition(2,m_shape[0],m_shape[0]),0.);
    }

    //    m_maxCFSupport=0; //need this parameter to stack all the CF for average PB estimate

    //itsTaylorTerm=TaylorTerm;
    //itsRefFreq=RefFreq;
    //cout<<"itsTaylorTerm itsRefFreq "<<itsTaylorTerm<<" "<<itsRefFreq<<endl;
    MEpoch start = observationStartTime(ms, 0);
    its_NotApplyArray=(int(float(ApplyBeamCode)/2.)==1.);
    its_NotApplyElement=((float(ApplyBeamCode)/2.-int(float(ApplyBeamCode)/2.))*2.==1.);
    
    //cout<<ApplyBeamCode-float(ApplyBeamCode)/2<<" "<<ApplyBeamCode<<" "<<float(ApplyBeamCode)/2<<" "<<ApplyBeamCode<<endl;

    its_UseWSplit=parameters.asBool("UseWSplit");
    //cout<<"Not Apply Elements Beam: "<<its_NotApplyElement<<" "<<ApplyBeamCode/2<<" | Not Apply Array Beam: "<<its_NotApplyArray<<" "<<ApplyBeamCode-ApplyBeamCode/2<<endl;
    //cout<<"Not Apply Elements Beam: "<<its_NotApplyElement<<" "<<ApplyBeamCode/2<<" | Not Apply Array Beam: "<<its_NotApplyArray" "<<ApplyBeamCode-ApplyBeamCode/2<<endl;
    
    m_refFrequency = BBS::readFreqReference(ms, 0);
    its_Use_EJones=Use_EJones;
    its_Apply_Element=Apply_Element;
    its_count_time=0;
    //if(!its_Use_EJones){cout<<"Not using the beam in the calculation of the CFs...."<<endl;}
    if (m_oversampling%2 == 0) {
      // Make OverSampling an odd number
      m_oversampling++;
    }

    //list_freq   = Vector<Double>(1, m_refFrequency);
    ROMSSpWindowColumns window(ms.spectralWindow());
    list_freq_spw.resize(window.nrow());
    uInt NSPW(window.nrow());
    uInt NchanPerSPW((window.chanFreq()(0)).shape()[0]);
    its_ChanBlockSize=parameters.asInt("ChanBlockSize");
    if(its_ChanBlockSize==0){its_ChanBlockSize=NchanPerSPW;}
    uInt NBlocks(NchanPerSPW*NSPW/its_ChanBlockSize);
    uInt NBlocksPerSPW(NchanPerSPW/its_ChanBlockSize);
    list_freq_chanBlock.resize(NBlocks);
    map_chan_chanBlock.resize(NchanPerSPW*NSPW);
    map_chanBlock_spw.resize(NBlocks);
    //cout<<NBlocks<<" "<<NchanPerSPW<<" "<<NSPW<<endl;
    uInt map_chanBlock_spw_index(0);
    uInt list_freq_chanBlock_index(0);
    uInt map_chan_chanBlock_index(0);

    Int count_block=0;
    IPosition pos(1,1);
    pos(0)=0;
    for(uInt i=0; i<window.nrow();++i){
      list_freq_spw[i]=window.refFrequency()(i);
      Double freqMeanBlock(0.);
      Double freqChan(0.);
      for(uInt j=0; j<(window.chanFreq()(i)).shape()[0];++j){
	freqChan=((window.chanFreq()(i))[j])(pos);
	freqMeanBlock+=freqChan/its_ChanBlockSize;
	count_block+=1;
	map_chan_chanBlock[map_chan_chanBlock_index]=list_freq_chanBlock_index;
	map_chan_chanBlock_index+=1;
	if(count_block==its_ChanBlockSize){
	  list_freq_chanBlock[list_freq_chanBlock_index]=freqMeanBlock;
	  freqMeanBlock=0.;
	  count_block=0;
	  map_chanBlock_spw[map_chanBlock_spw_index]=i;
	  list_freq_chanBlock_index+=1;
	  map_chanBlock_spw_index+=1;
	}
      }
    };
    Vector<uInt> tmpVec;
    tmpVec.resize(NBlocksPerSPW);
    map_spw_chanBlock.resize(NSPW);
    uInt map_spw_chanBlock_index(0);
    for(uInt i=0; i<NSPW;++i){
      for(uInt j=0; j<NBlocksPerSPW;++j){
	tmpVec[j]=map_spw_chanBlock_index;
	map_spw_chanBlock_index+=1;
      }
      map_spw_chanBlock[i]=tmpVec.copy();
    }

    map_chan_Block_buffer.resize(NchanPerSPW);
    uInt iindex(0);
    for(uInt i=0; i<NchanPerSPW/its_ChanBlockSize;++i){
      for(Int j=0; j<its_ChanBlockSize;++j){
	map_chan_Block_buffer[iindex]=i;
	iindex+=1;
      }
    }

    if (itsVerbose > 0) {
      for(uInt i=0; i<list_freq_spw.size();++i){
        cout<<"SPW"<<i<<" f="<<list_freq_spw[i]/1.e6<<endl;
      }
      cout<<endl;

      for(uInt i=0; i<list_freq_chanBlock.size();++i){
        cout<<"block"<<i<<" f="<<list_freq_chanBlock[i]/1.e6<<endl;
        cout<<"  to spw="<<map_chanBlock_spw[i]<<endl; 
      }

      cout<<endl;
      for(uInt i=0; i<map_chan_chanBlock.size();++i){
        cout<<"  Chan "<<i<<" to ChanBlock="<<map_chan_chanBlock[i]<<endl; 
      }

      cout<<endl;
      for(uInt i=0; i<NSPW;++i){
        cout<<" spw="<<i<<"  Blocks=" <<map_spw_chanBlock[i]<<endl;
      }

      cout<<endl;
      for(uInt i=0; i<NchanPerSPW;++i){
        cout<<" chan="<<i<<"  BlocksBuffer=" <<map_chan_Block_buffer[i]<<endl;
      }
    }
    // assert(false);
    

  // Array<Complex> lala(IPosition(4,8192,8192,4,1),0.);
  // for(uInt pol=0; pol<4; ++pol){
  //   Matrix<Complex> slice = (lala(Slicer(IPosition(4, 0, 0,pol,0),
  // 					 IPosition(4, lala.shape()[0], lala.shape()[0],1,1)))).nonDegenerate();
  //   slice(200,200)=1.;
  //   itsFFTMachines[0].normalized_forward (slice.nrow(),slice.data(),6, FFTW_MEASURE);
  //  }
  // Cube<Complex> SaveCube;
  // SaveCube=lala.nonDegenerate();
  // store(SaveCube,"SaveCube.fft");
  // for(uInt pol=0; pol<4; ++pol){
  //   Matrix<Complex> slice = (lala(Slicer(IPosition(4, 0, 0,pol,0),
  // 					 IPosition(4, lala.shape()[0], lala.shape()[0],1,1)))).nonDegenerate();
  //   itsFFTMachines[0].normalized_backward (slice.nrow(),slice.data(),6, FFTW_MEASURE);
  //  }
  // SaveCube=lala.nonDegenerate();
  // store(SaveCube,"SaveCube.fft.fft");
  // assert(false);

    m_nChannelBlocks  = list_freq_chanBlock.size();
    ROMSAntennaColumns antenna(ms.antenna());
    m_nStations = antenna.nrow();

    m_pixelSizeSpheroidal = makeSpheroidCut();
    
    if(parameters.asBool("FindNWplanes")){
      m_nWPlanes=FindNWplanes();
    }
      m_wScale = WScale(m_maxW, m_nWPlanes);
    //Double PixelSize=abs(m_coordinates.increment()(0));
    //Double ImageDiameter=PixelSize * m_shape(0);
    //Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, m_maxW));
    //m_maxCFSupport= ImageDiameter / W_Pixel_Ang_Size;
    //Matrix<Complex> Stack_pb_cf0(IPosition(2,m_maxCFSupport,m_maxCFSupport),0.);
    //Matrix<Complex> Stack_pb_cf0(IPosition(2,m_shape(0),m_shape(0)),0.);
      //Matrix<float> Stack_pb_cf1(IPosition(2,m_shape(0),m_shape(0)),0.);

    //Stack_pb_cf0(256,300)=1.;
    //Matrix<Complex> Avg_PB_padded00(give_normalized_fft(Stack_pb_cf0,false));
    //store(Avg_PB_padded00,"Avg_PB_padded00.img");

    // Precalculate the Wtwerm fft for all w-planes.
    store_all_W_images();
    itsFilledVectorMasks=false;




    // // Build the cutted spheroidal for the element beam image
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    DirectionCoordinate coordinate = m_coordinates;
    Double aPixelAngSize = min(m_pixelSizeSpheroidal,
    			       estimateAResolution(m_shape, m_coordinates));
    //Double aPixelAngSize = estimateAResolution(m_shape, m_coordinates, 30);
    cout<<"    Aterm support: "<<imageDiameter /estimateAResolution(m_shape, m_coordinates)<<endl;
    Int nPixelsConv = imageDiameter / aPixelAngSize;
    nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
    // Matrix<Complex> spheroid_cut_element(IPosition(2,nPixelsConv,nPixelsConv),1.);
    // taper_parallel(spheroid_cut_element);
    // //Matrix<Complex> spheroid_cut_element_fft=give_normalized_fft_lapack(spheroid_cut_element, true);
    // normalized_fft_parallel(spheroid_cut_element, true);
    // spheroid_cut_element_fft=spheroid_cut_element;
    // Matrix<Complex> spheroid_cut_element_padfft(zero_padding(spheroid_cut_element_fft, m_shape(0)));
    // //Matrix<Complex> spheroid_cut_element_padfft_fft=give_normalized_fft_lapack (spheroid_cut_element_padfft, false);
    // normalized_fft (spheroid_cut_element_padfft, false);
    // Matrix<Complex> spheroid_cut_element_padfft_fft(spheroid_cut_element_padfft);
    // float threshold = 1.e-6;
    // for (Int jj=0; jj<m_shape[1]; ++jj) {
    //   for (Int ii=0; ii<m_shape[0]; ++ii) {
    // 	Float absVal = abs(spheroid_cut_element_padfft_fft(ii,jj));
    // 	spheroid_cut_element_padfft_fft(ii,jj) = std::max (absVal, threshold);
    //   }
    // }
    // Spheroid_cut_im_element.reference (real(spheroid_cut_element_padfft_fft));
    // store(m_coordinates,Spheroid_cut_im_element,"Spheroid_cut_im_element.img");

    // Stuff for image plane correction
    its_VectorThreadsSumWeights.resize(OpenMP::maxThreads());
    for (uInt t=0; t<OpenMP::maxThreads(); ++t) {
      its_VectorThreadsSumWeights[t]=0.;
      StackMuellerNew[t].resize(4);
      for (uInt i=0; i<4; ++i) {
	StackMuellerNew[t][i].resize(4);
	for (uInt j=0; j<4; ++j) {
	  StackMuellerNew[t][i][j].resize(IPosition(2,nPixelsConv,nPixelsConv));
	  StackMuellerNew[t][i][j]=Complex(0.);
	}
      }
    }
    m_NPixATerm=nPixelsConv;
    cout<<"  Initialising wTerm Interpolation..."<<endl;
    initMeanWStepsGridder();
    cout<<"  done"<<endl;
    //initStoreMasks();

  }

  //      ~LofarConvolutionFunction ()
  //      {
  //      }

  Int LofarConvolutionFunction::FindNWplanes()
  {
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    float NMeans(500);
    // Size the vector, but give each element its own default matrix,
    // so the vector can be safely filled in parallel.
    Double wPixelAngSize = min(m_pixelSizeSpheroidal, estimateAResolution(m_shape, m_coordinates));
    Int nPixelsConv = imageDiameter / wPixelAngSize;
    nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
    IPosition shape(2, nPixelsConv, nPixelsConv);
    //Careful with the sign of increment!!!! To check!!!!!!!
    Vector<Double> increment(2, wPixelAngSize);
    double wavelength(C::c / list_freq_chanBlock[0]);
    Double wmax_plane(m_maxW);
    {
      // Thread private variables.
      Float wStep(5000.);
      for (uInt i=0; i<NMeans; ++i) {
        Double w = wStep*float(i+0.5)/float(NMeans);

        // Make odd and optimal.
	Double W_Pixel_Ang_Size=estimateWResolution(m_shape, pixelSize, w/wavelength);
	
	Int nPixelsConvW = imageDiameter /  W_Pixel_Ang_Size;
	//cout<<endl;
	//cout<<w<<" "<<imageDiameter*180./3.14<<" "<<W_Pixel_Ang_Size*60.*180./(3.14)<<" "<<nPixelsConvW<<" "<<nPixelsConv<<endl;
	if(nPixelsConvW>nPixelsConv){
	  wmax_plane=w;
	  break;
	};
      }
      m_nWPlanes=int(m_maxW/wmax_plane)+2;
      cout<<" Number of w-planes set to: "<<m_nWPlanes<<endl;
      return m_nWPlanes;
      // Update the timing info.
    } // end omp parallel

  }

  void LofarConvolutionFunction::initMeanWStepsGridder()
  {
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    vector<Complex> wCorrGridder;
    float NMeans(200);
    wCorrGridder.resize(NMeans);
    its_wCorrGridder.resize(NMeans);
    its_wCorrGridderMatrix.resize(NMeans);
    // Size the vector, but give each element its own default matrix,
    // so the vector can be safely filled in parallel.
    Double wPixelAngSize = min(m_pixelSizeSpheroidal, estimateAResolution(m_shape, m_coordinates));
    Int nPixelsConv = imageDiameter / wPixelAngSize;
    nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
    IPosition shape(2, nPixelsConv, nPixelsConv);
    //Careful with the sign of increment!!!! To check!!!!!!!
    Vector<Double> increment(2, wPixelAngSize);
    double wavelength(C::c / list_freq_chanBlock[0]);
    {
      // Thread private variables.
      Float wStep(m_wScale.center(1)-m_wScale.center(0));
      for (uInt i=0; i<NMeans; ++i) {
        Double w = wStep*float(i)/float(NMeans);
        // Double wPixelAngSize = min(m_pixelSizeSpheroidal,
        //                            estimateWResolution(m_shape,
        //                                                pixelSize, w));

        // Make odd and optimal.
	//Double W_Pixel_Ang_Size=estimateWResolution(m_shape, pixelSize, w);
	//Int nPixelsConvW = imageDiameter /  W_Pixel_Ang_Size;
	//cout<<w<<" "<<nPixelsConvW<<" "<<nPixelsConv<<endl;
	// if(nPixelsConvW>nPixelsConv){
	//   cout<<" ... too little w-planes"<<endl;
	//   assert(false);
	// };
        Matrix<Complex> wTerm = m_wTerm.evaluate(shape, increment, w/wavelength);
	Complex ValMean(0.);
	for(uInt ii=0; ii<wTerm.shape()[0]; ++ii){
	  for(uInt jj=0; jj<wTerm.shape()[0]; ++jj){
	    ValMean+=wTerm(ii,jj);
	  }
	}
	ValMean/=wTerm.shape()[0]*wTerm.shape()[0];
	wCorrGridder[i]=ValMean/abs(ValMean);
	its_wCorrGridderMatrix[i].reference(wTerm);
	//cout<<w<<" "<<ValMean<<" "<<abs(ValMean)<<endl;
      }
      its_wCorrGridder=wCorrGridder;
      its_wStep=wStep/float(NMeans);
      // Update the timing info.
    } // end omp parallel

  }

  // Precalculate all W-terms in the fourier domain
  void LofarConvolutionFunction::store_all_W_images()
  {
    logIO()<<"LofarConvolutionFunction::store_all_W_images() "<<"Computing the Wterms..."<< LogIO::POST;//<<endl;
    PrecTimer wTimer;
    wTimer.start();
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    // Size the vector, but give each element its own default matrix,
    // so the vector can be safely filled in parallel.
    m_WplanesStore.reserve (m_nWPlanes);
    for (uInt i=0; i<m_nWPlanes; ++i) {
      m_WplanesStore.push_back (Matrix<Complex>());
    }

    double wavelength(C::c / list_freq_chanBlock[0]);

#pragma omp parallel
    {
      // Thread private variables.
      PrecTimer timerFFT;
      PrecTimer timerPar;
#pragma omp for schedule(dynamic)
      for (uInt i=0; i<m_nWPlanes; ++i) {
        timerPar.start();
        Double w = m_wScale.center(i);
	//cout<<"i="<<i<<", w="<<w<<endl;
        Double wPixelAngSize = min(m_pixelSizeSpheroidal,
                                   estimateWResolution(m_shape,
                                                       pixelSize, w/wavelength));
        Int nPixelsConv = imageDiameter / wPixelAngSize;
        if (itsVerbose > 0) {
          cout<<"Number of pixel in the "<<i<<"-wplane: "<<nPixelsConv
              <<"  (w="<<w<<")"<<endl;
	}
        if (nPixelsConv > itsMaxSupport) {
          nPixelsConv = itsMaxSupport;
        }
        // Make odd and optimal.
        nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
        wPixelAngSize = imageDiameter / nPixelsConv;
        IPosition shape(2, nPixelsConv, nPixelsConv);
        //Careful with the sign of increment!!!! To check!!!!!!!
        Vector<Double> increment(2, wPixelAngSize);
        Matrix<Complex> wTerm = m_wTerm.evaluate(shape, increment,
                                                 w/wavelength);
	//store(m_coordinates,wTerm ,"wTerm."+String::toString(i)+".img");
        //normalized_fft(timerFFT, wTerm);
        m_WplanesStore[i].reference (wTerm);
        timerPar.stop();
      }
      // Update the timing info.
      double ftime = timerFFT.getReal();
#pragma omp atomic
      itsTimeWfft += ftime;
      unsigned long long cnt = timerFFT.getCount();
#pragma omp atomic
      itsTimeWcnt += cnt;
      double ptime = timerPar.getReal();
#pragma omp atomic
      itsTimeWpar += ptime;
    } // end omp parallel

    its_MaxWSupport=0;
    for (uInt i=0; i<m_nWPlanes; ++i) {
      if(m_WplanesStore[i].shape()[0]>its_MaxWSupport){its_MaxWSupport=m_WplanesStore[i].shape()[0];};
    }



    wTimer.stop();
    itsTimeW = wTimer.getReal();
    logIO()<<"LofarConvolutionFunction::store_all_W_images() "<<"... Done!"<< LogIO::POST;//<<endl;
  }


  // Compute the fft of the beam at the minimal resolution for all antennas
  // if not done yet.
  // Put it in a map object with a (double time) key.
  void LofarConvolutionFunction::computeAterm (Double time)
  {
    //logIO()<<"LofarConvolutionFunction::computeAterm "<<"Computing the Aterms for t="<<time<<", and maximum Wupport="<<its_MaxWSupport<< LogIO::POST;//<<endl;
    PrecTimer timerCyril;
    timerCyril.start();
    if (m_AtermStore.find(time) != m_AtermStore.end()) {
      // Already done.
      return;
    }
    PrecTimer aTimer;
    aTimer.start();
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    // Try to avoid making copies when inserting elements in vector or map.
    // Therefore first create the elements and resize them.
    m_AtermStore[time] = vector< vector< Cube<Complex> > >();
    m_AtermStore_element[time] = vector< vector< Cube<Complex> > >();
    m_AtermStore_station[time] = vector< vector< Cube<Complex> > >();
    vector< vector< Cube<Complex> > >& aTermList = m_AtermStore[time];
    vector< vector< Cube<Complex> > >& aTermList_element = m_AtermStore_element[time];
    vector< vector< Cube<Complex> > >& aTermList_station = m_AtermStore_station[time];
    // Calculate the A-term and fill the vector for all stations.
    aTermList.resize (m_nStations);
    aTermList_element.resize (m_nStations);
    aTermList_station.resize (m_nStations);
    ///#pragma omp parallel
    {
      // Thread private variables.
      PrecTimer timerFFT;
      PrecTimer timerPar;

      DirectionCoordinate coordinate = m_coordinates;
      Double aPixelAngSize = min(m_pixelSizeSpheroidal, estimateAResolution(m_shape, m_coordinates));
      Int nPixelsConv = imageDiameter / aPixelAngSize;
      if (nPixelsConv > itsMaxSupport) {
	nPixelsConv = itsMaxSupport;
      }
      // Make odd and optimal.
      nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
      aPixelAngSize = imageDiameter / nPixelsConv;
      IPosition shape(2, nPixelsConv, nPixelsConv);
      Vector<Double> increment_old(coordinate.increment());
      Vector<Double> increment(2);
      increment[0] = aPixelAngSize*sign(increment_old[0]);
      increment[1] = aPixelAngSize*sign(increment_old[1]);
      coordinate.setIncrement(increment);
      Vector<Double> refpix(2, 0.5*(nPixelsConv-1));
      coordinate.setReferencePixel(refpix);
      
      DirectionCoordinate coordinate_element = m_coordinates;
      //Double aPixelAngSize_element = estimateAResolution(m_shape, m_coordinates, 30.);
      Double aPixelAngSize_element = min(m_pixelSizeSpheroidal, estimateAResolution(m_shape, m_coordinates));
      Int nPixelsConv_element = imageDiameter / aPixelAngSize_element;
      //cout<<"Element_beam size:"<<"1 "<<nPixelsConv_element<<", 2 "<<aPixelAngSize_element<<endl;
      nPixelsConv_element = FFTCMatrix::optimalOddFFTSize (nPixelsConv_element);
      aPixelAngSize_element = imageDiameter / nPixelsConv_element;
      //cout<<"Element_beam size:"<<"1 "<<nPixelsConv_element<<", 2 "<<aPixelAngSize_element<<endl;
      IPosition shape_element(2, nPixelsConv_element, nPixelsConv_element);
      Vector<Double> increment_element(2);
      increment_element[0] = aPixelAngSize_element*sign(increment_old[0]);
      increment_element[1] = aPixelAngSize_element*sign(increment_old[1]);
      coordinate_element.setIncrement(increment_element);
      Vector<Double> refpix_element(2, 0.5*(nPixelsConv_element-1));
      coordinate_element.setReferencePixel(refpix_element);
      
      //hier is het
      
      m_aTerm.setDirection(coordinate, shape);
      
      MEpoch binEpoch;
      binEpoch.set(Quantity(time, "s"));
      
      m_aTerm.setEpoch(binEpoch);
//       LofarATerm::ITRFDirectionMap dirMap = m_aTerm.makeDirectionMap(coordinate, shape, binEpoch);
      
      //#pragma omp for
      for (uInt i=0; i<m_nStations; ++i) {
        timerPar.start();

        //======================================
        // Separated element and station
        //======================================
	vector< Cube<Complex> > aTermA_element;
	vector< Cube<Complex> > aTermA_array;
	
	vector< Cube<Complex> > aTermA;
	//aTermA=m_aTerm.evaluate(i, list_freq_chanBlock , list_freq_chanBlock , true);

	Bool itsapplyIonosphere = itsParameters.asBool("applyIonosphere");
	Cube<DComplex> zTermA;
	//if(itsapplyIonosphere==true){zTermA=m_aTerm.evaluateIonosphere(i, list_freq_chanBlock);}
	if(itsapplyIonosphere==true){zTermA=m_aTerm.evaluateStationScalarFactor(i, list_freq_chanBlock, list_freq_chanBlock, true);}

	vector< Matrix<Complex> > aTermA_array_plane;
	if(its_NotApplyArray==false){
	  aTermA_array_plane=m_aTerm.evaluateArrayFactor(i, 0, list_freq_chanBlock , list_freq_chanBlock, true);
	} else{
	  aTermA_array_plane.resize(m_nChannelBlocks);
	  for (uInt ch=0; ch<m_nChannelBlocks; ++ch) {
	    aTermA_array_plane[ch].resize(IPosition(2,shape[0],shape[0]));
	    aTermA_array_plane[ch]=0.;
	  }
	}

	aTermA_array.resize(m_nChannelBlocks);
        for (uInt ch=0; ch<m_nChannelBlocks; ++ch) {
	  aTermA_array[ch].resize(IPosition(3,shape[0],shape[0],4));
	  aTermA_array[ch]=0.;
	}
        for (uInt ch=0; ch<m_nChannelBlocks; ++ch) {
	  Matrix<Complex> plane(aTermA_array[ch].xyPlane(0));
	  plane=aTermA_array_plane[ch].copy();
	  Matrix<Complex> plane2(aTermA_array[ch].xyPlane(3));
	  plane2=aTermA_array_plane[ch].copy();
	  if(itsapplyIonosphere==true){
	    Matrix<DComplex> plane3(zTermA.xyPlane(ch));
	    Matrix<Complex> plane3b(plane3.shape());
	    convertArray (plane3b, plane3);
	    //plane=plane*plane3b;
	    //plane2=plane2*plane3b;
	    plane=plane3b.copy();
	    plane2=plane3b.copy();
	  }
	}

	//store(zTermA[0],"Ion"+String::toString(i)+".img");

	if(its_NotApplyElement==false){
	  aTermA_element=m_aTerm.evaluateElementResponse(i, 0, list_freq_spw, true);
	} else{
	  aTermA_element.resize(m_nChannelBlocks);
	  for (uInt ch=0; ch<m_nChannelBlocks; ++ch) {
	    aTermA_element[ch].resize(IPosition(3,shape[0],shape[0],4));
	    aTermA_element[ch]=0.;
	  }
	}
	// Disable element beam and station beam
	  
	if(its_NotApplyElement==true){
	  for (uInt ch=0; ch<aTermA_element.size(); ++ch) {
	    for(uInt pl=0; pl<4; ++pl){
	      Matrix<Complex> plane(aTermA_element[ch].xyPlane(pl));
	      if((pl==0)|(pl==3))
		{
		  plane=1.;
		} 
	      else
		{
		  plane=0.;
		}
	    }
	  }
	}
	
	if(its_NotApplyArray==true){
	  for (uInt ch=0; ch<m_nChannelBlocks; ++ch) {
	    for(uInt pl=0; pl<4; ++pl){
	      Matrix<Complex> plane(aTermA_array[ch].xyPlane(pl));
	      if((pl==0)|(pl==3))
		{
		  plane=1.;
		} 
	      else
		{
		  plane=0.;
		}
	    }
	  }
	}
	// End Disable element beam and station beam


        // Compute the fft on the beam
        // for (uInt ch=0; ch<m_nChannel; ++ch) {
        //   for (uInt pol=0; pol<4; ++pol) {
        //     Matrix<Complex> plane1 (aTermA_array[ch].xyPlane(pol));
	//     //Matrix< Complex > plane0int = LinearInterpol2(plane1,200.);
        //     normalized_fft (timerFFT, plane1);
        //   }
        // }

	// store(coordinate,aTermA_array[0],"aTermA_array.fft."+String::toString(i)+".img");

        // Note that push_back uses the copy constructor, so for the Cubes
        // in the vector the copy constructor is called too (which is cheap).
        aTermList[i] = aTermA;
        aTermList_element[i] = aTermA_element;
        aTermList_station[i] = aTermA_array;
        timerPar.stop();
      } // end omp for
      // Update the timing info.
      double ftime = timerFFT.getReal();
      ///#pragma omp atomic
      itsTimeAfft += ftime;
      unsigned long long cnt = timerFFT.getCount();
      ///#pragma omp atomic
      itsTimeAcnt += cnt;
      double ptime = timerPar.getReal();
      ///#pragma omp atomic
      itsTimeApar += ptime;
    } // end omp parallel
    aTimer.stop();
    itsTimeA = aTimer.getReal();
    //logIO()<<"LofarConvolutionFunction::computeAterm "<<"...Done!"<< LogIO::POST;//<<endl;
    timerCyril.stop();
    //cout.precision(20);
    //cout<<"For time: "<<time<<endl;
    //assert(false);
    //timerCyril.show(cout,"LofarConvolutionFunction::computeAterm");
    //assert(false);
  }


  //==================================================================
  //==================================================================
  void LofarConvolutionFunction::ApplyWterm_Image(Array<Complex>& input_grid, Array<Complex>& output_grid, uInt spw, bool degridding_step, Int w_index)
  {

    Double res_ini = abs(m_coordinates.increment()(0));
    //    Double diam_image = res_ini*m_shape[0];
    Vector<Double> resolution(2, res_ini);
    double w;
    if(w_index<0){
      w=-m_wScale.center(-w_index);
    } else {
      w=m_wScale.center( w_index);
    }
    

    double wavelength(C::c / list_freq_spw[spw]);
    // Array<Complex> grid_out(input_grid.shape(),0.);
    // if (w_index>0.) {
    //   w=-w;
    // }
    if (!degridding_step) {
      w=-w;
    }
    // cout<<input_grid.shape()<<endl;

    if (input_grid.shape()[0]%2 == 1) {
      cout<<"image must have an odd number of pixels"<<endl;
      assert(false);
    }
    
    // cout<<"declare aMatrix"<<endl;
    // Matrix<Complex> aMatrix(IPosition(2,input_grid.shape()[0],input_grid.shape()[0]),0.);
    // cout<<"fft aMatrix"<<endl;
    // normalized_fft (aMatrix, false);
    // cout<<"...done fft aMatrix"<<endl;



    int nx(input_grid.shape()[0]);
    int ny(input_grid.shape()[0]);
    double radius[2] = {0.5 * (nx), 0.5 * (ny)};
    double twoPiW = 2.0 * C::pi * w/wavelength;


    IPosition pos(4,1,1,1,1);
    Complex pix;
    uInt jj;
    Complex wterm;
    double m, m2, l, lm2, phase;
      for(uInt ch=0; ch<input_grid.shape()[3]; ++ch){
#pragma omp parallel
    {
#pragma omp for private(pos,pix,jj,wterm,m,m2, l, lm2, phase) schedule(dynamic)
      	for(uInt ii=0; ii<input_grid.shape()[0]; ++ii){
      	  pos[0]=ii;
      	  for(jj=0; jj<input_grid.shape()[0]; ++jj){
      	    pos[1]=jj;
	    m = resolution[1] * (ii - radius[1]);
	    m2 = m * m;
	    l = resolution[0] * (jj - radius[0]);
	    lm2 = l * l + m2;
	    phase = twoPiW * (sqrt(1.0 - lm2) - 1.0);
	    wterm=Complex(cos(phase), sin(phase));
     	    //wterm=m_wTerm.evaluate_pixel(ii, jj, input_grid.shape()[0],input_grid.shape()[0], resolution, w/wavelength);
	    wterm=wterm*abs(Spheroid_cut_im(ii,jj));
	    //if(w_index<0){wterm=conj(wterm);}
	    //pix=Complex(input_grid(pos)*wterm);
	    for(uInt pol=0; pol<input_grid.shape()[2]; ++pol){
	      pos[3]=ch;
	      pos[2]=pol;
	      output_grid(pos)=input_grid(pos)*wterm;

	    //if(abs(input_grid(pos))>0.01){cout<<ii<<" "<<jj<<" "<<input_grid(pos)<<" "<<wterm<<" "<<pix<<" "<<endl;}
	    //input_grid(pos)=pix;
	    }
      	}
      }
    }
    }

      //return output_grid;//input_grid;//grid_out;
      
  }



  Array<Complex> LofarConvolutionFunction::ApplyWterm(Array<Complex>& input_grid, uInt /*spw*/, bool degridding_step, Int w_index, vector< Array<Complex> >& gridsparalel, Int TnumMask, Int WnumMask)
  {

    Matrix<Complex> wTerm;
    if(w_index<0){
      wTerm = (m_WplanesStore[-w_index]).copy();
    } else {
      wTerm = (m_WplanesStore[w_index]).copy();
    }

    if (w_index>0.) {
      wTerm=conj(wTerm);
    }
    if (degridding_step) {
      wTerm=conj(wTerm);
    }


    Array<Complex> grid_out(input_grid.shape(),0.);

    Matrix<Complex> Spheroid_WtermOrig(zero_padding(Spheroid_cut,wTerm.shape()[0]));
    normalized_fft (Spheroid_WtermOrig, false);
    //wTerm=1.;
    wTerm*=Spheroid_WtermOrig;
    normalized_fft (wTerm, true);
    // taper(wTerm);
    // normalized_fft (wTerm, true);


     Matrix<uShort> MaskInFull;
     MaskInFull.resize(IPosition(2,input_grid.shape()[0]*input_grid.shape()[0],2));
     uInt index(0);
     for(uShort i=0;i<input_grid.shape()[0];++i){
       for(uShort j=0;j<input_grid.shape()[0];++j){
     	 MaskInFull(index,0)=i;
     	 MaskInFull(index,1)=j;
     	 index+=1;
       }
     }

    Matrix<Complex> CF(wTerm);
    //cout<<" Wterm supprt: "<<CF.shape()[0]<<endl;
    if(!degridding_step){
      Matrix<uShort> MaskIn;
      MaskIn.reference(itsVecMasksNew[TnumMask][WnumMask+m_nWPlanes]);
      ConvolveArrayArrayParallel4(input_grid, grid_out, CF,gridsparalel,MaskIn);
    } else{
      Matrix<uShort> MaskIn;
      MaskIn.reference(itsVecMasksNewW[TnumMask][WnumMask+m_nWPlanes]);
      MaskIn.reference(itsVecMasksNew[TnumMask][WnumMask+m_nWPlanes]);
      ConvolveArrayArrayParallel4(input_grid, grid_out, CF,gridsparalel,MaskIn);
    }
    return grid_out;

  }

  //////////////////////////////////////////////////////////

  Array<Complex> LofarConvolutionFunction::ApplyElementBeam3(Array<Complex>& input_grid, Double timeIn, uInt spw, const Matrix<bool>& Mask_Mueller_in2, bool degridding_step, vector< Array<Complex> >& gridsparalel, Int /*UsedMask*/)
  {

    Double time(GiveClosestTimeAterm(timeIn));
    Matrix<bool> Mask_Mueller_in(Mask_Mueller_in2.copy());
    for(uInt i=0;i<4;++i){
      for(uInt j=0;j<4;++j){
    	Mask_Mueller_in(i,j)=true;
      }
    }
    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_element = m_AtermStore_element.find(time);
    AlwaysAssert (aiter_element!=m_AtermStore_element.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm_element = aiter_element->second;



    vector< vector< IPosition > > Mueller_Coordinates;
    Mueller_Coordinates.resize(4);
    for(uInt i=0;i<4;++i){
      Mueller_Coordinates[i].resize(4);
      IPosition pos(2,2,1);
      for(uInt j=0;j<4;++j){
	Mueller_Coordinates[i][j]=pos;
      }
    }

    {
    uInt ind0;
    uInt ind1;
    uInt ii = 0;
    IPosition cfShape;
    for (uInt row0=0; row0<=1; ++row0) {
      for (uInt col0=0; col0<=1; ++col0) {
	vector < Matrix<Complex> > Row(4);
	vector < Matrix<Complex> > Row_non_padded(4);
	uInt jj = 0;
	for (uInt row1=0; row1<=1; ++row1) {
	  for (uInt col1=0; col1<=1; ++col1) {
	    // This Mueller ordering is for polarisation given as XX,XY,YX YY
	    //ind0 = row0 + 2*row1;
	    //ind1 = col0 + 2*col1;
	    ind0 = 2.*row0 + row1;
	    ind1 = 2.*col0 + col1;
	    IPosition pos(2,2,1);
	    pos[0]=ind0;
	    pos[1]=ind1;
	    Mueller_Coordinates[ii][jj]=pos;
	    ++jj;
	  }
	}
	++ii;
      }
    }
    }

    if (!degridding_step) {
      for (uInt i=0; i<4; ++i) {
    	for (uInt j=i; j<4; ++j) {
    	  IPosition pos_tmp(Mueller_Coordinates[i][j]);
    	  Mueller_Coordinates[i][j]=Mueller_Coordinates[j][i];
    	  Mueller_Coordinates[j][i]=pos_tmp;
    	  Bool bool_tmp(Mask_Mueller_in(i,j));
    	  Mask_Mueller_in(i,j)=Mask_Mueller_in(j,i);
    	  Mask_Mueller_in(i,j)=bool_tmp;
    	}
      }
    }

    Cube<Complex> aTermA(aterm_element[0][spw].copy());
    Array<Complex> grid_out(input_grid.shape(),0.);

    vector< vector< Matrix<Complex> > > vec_plane_product;
    vec_plane_product.resize(4);


    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Calculate element beams"<< LogIO::POST;//<<endl;
    Matrix<Complex> Spheroid_AtermOrig(zero_padding(Spheroid_cut,aTermA.xyPlane(0).shape()[0]));
    normalized_fft (Spheroid_AtermOrig, false);
    for(uInt ii=0;ii<4;++ii){
      vec_plane_product[ii].resize(4);
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  vec_plane_product[ii][jj].resize(aTermA.xyPlane(0).shape());
	  vec_plane_product[ii][jj]=aTermA.xyPlane((Mueller_Coordinates[ii][jj])[0]) * conj(aTermA.xyPlane((Mueller_Coordinates[ii][jj])[1]));
	  //taper(vec_plane_product[ii][jj]);
	  vec_plane_product[ii][jj]*=Spheroid_AtermOrig;
	  if(!degridding_step){vec_plane_product[ii][jj]=conj(vec_plane_product[ii][jj]);};
	  //store(vec_plane_product[ii][jj],"Im_AH"+String::toString(ii)+"-"+String::toString(jj)+".img");
	  normalized_fft(vec_plane_product[ii][jj],true);
	}
      }
    }

     Matrix<uShort> MaskInFull;
     MaskInFull.resize(IPosition(2,input_grid.shape()[0]*input_grid.shape()[0],2));
     uInt index(0);
     for(uShort i=0;i<input_grid.shape()[0];++i){
       for(uShort j=0;j<input_grid.shape()[0];++j){
     	 MaskInFull(index,0)=i;
     	 MaskInFull(index,1)=j;
     	 index+=1;
       }
     }
     Array<Complex> ArrMatrix_out(input_grid.shape());
    for(uInt ii=0;ii<4;++ii){
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  Matrix<uShort> MaskIn;
	  MaskIn.reference(MaskInFull);//itsVecMasksNewElement[UsedMask]);
	  Matrix<Complex> ConvFunc(vec_plane_product[ii][jj]);
	  //store(ConvFunc,"Im_AH"+String::toString(ii)+"-"+String::toString(jj)+".img");
	  ArrMatrix_out=Complex(0.);
	  ConvolveArrayArrayParallel4(input_grid, ArrMatrix_out, jj, ConvFunc,gridsparalel,MaskIn);
	  SumGridsOMP(grid_out, ArrMatrix_out, jj, ii);
	}
      }
    }



    return grid_out;

  }

  //==================================================================
  //==================================================================

  //////////////////////////////////////////////////////////

  Array<Complex> LofarConvolutionFunction::ApplyElementBeam_Image(Array<Complex>& input_grid, Double timeIn, uInt spw, const Matrix<bool>& Mask_Mueller_in2, bool degridding_step)
  {

    PrecTimer TimeEl;
    Double time(GiveClosestTimeAterm(timeIn));

    //cout.precision(20);
    //cout<<time<<endl;

    Matrix<bool> Mask_Mueller_in(Mask_Mueller_in2.copy());
    for(uInt i=0;i<4;++i){
      for(uInt j=0;j<4;++j){
    	Mask_Mueller_in(i,j)=true;
      }
    }
    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_element = m_AtermStore_element.find(time);
    AlwaysAssert (aiter_element!=m_AtermStore_element.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm_element = aiter_element->second;
    

    vector< vector< IPosition > > Mueller_Coordinates;
    Mueller_Coordinates.resize(4);
    for(uInt i=0;i<4;++i){
      Mueller_Coordinates[i].resize(4);
      IPosition pos(2,2,1);
      for(uInt j=0;j<4;++j){
	Mueller_Coordinates[i][j]=pos;
      }
    }

    {
    uInt ind0;
    uInt ind1;
    uInt ii = 0;
    IPosition cfShape;
    for (uInt row0=0; row0<=1; ++row0) {
      for (uInt col0=0; col0<=1; ++col0) {
	vector < Matrix<Complex> > Row(4);
	vector < Matrix<Complex> > Row_non_padded(4);
	uInt jj = 0;
	for (uInt row1=0; row1<=1; ++row1) {
	  for (uInt col1=0; col1<=1; ++col1) {
	    // This Mueller ordering is for polarisation given as XX,XY,YX YY
	    ind0 = row0 + 2*row1;
	    ind1 = col0 + 2*col1;
	    // ind0 = 2.*row0 + row1;
	    // ind1 = 2.*col0 + col1;
	    IPosition pos(2,2,1);
	    pos[0]=ind0;
	    pos[1]=ind1;
	    Mueller_Coordinates[ii][jj]=pos;
	    ++jj;
	  }
	}
	++ii;
      }
    }
    }


    if (!degridding_step) {
      for (uInt i=0; i<4; ++i) {
    	for (uInt j=i; j<4; ++j) {
    	  IPosition pos_tmp(Mueller_Coordinates[i][j]);
    	  Mueller_Coordinates[i][j]=Mueller_Coordinates[j][i];
    	  Mueller_Coordinates[j][i]=pos_tmp;
    	  Bool bool_tmp(Mask_Mueller_in(i,j));
    	  Mask_Mueller_in(i,j)=Mask_Mueller_in(j,i);
    	  Mask_Mueller_in(i,j)=bool_tmp;
    	}
      }
    }


    Cube<Complex> aTermA(aterm_element[0][spw].copy());
    uInt npol(input_grid.shape()[2]);

    vector< vector< Matrix<Complex> > > vec_plane_product;
    vec_plane_product.resize(4);


    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Calculate element beams"<< LogIO::POST;//<<endl;
    Matrix<Complex> Spheroid_AtermOrig(Spheroid_cut.copy());//zero_padding(Spheroid_cut,aTermA.xyPlane(0).shape()[0]));
    normalized_fft (Spheroid_AtermOrig, false);
    

    for(uInt ii=0;ii<4;++ii){
      vec_plane_product[ii].resize(4);
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  vec_plane_product[ii][jj].resize(aTermA.xyPlane(0).shape());
	  vec_plane_product[ii][jj]=aTermA.xyPlane((Mueller_Coordinates[ii][jj])[0]) * conj(aTermA.xyPlane((Mueller_Coordinates[ii][jj])[1]));
	  vec_plane_product[ii][jj]*=Spheroid_AtermOrig;
	  if(!degridding_step){vec_plane_product[ii][jj]=conj(vec_plane_product[ii][jj]);};
	  //store(vec_plane_product[ii][jj],"Im_AH"+String::toString(ii)+"-"+String::toString(jj)+".img");
	  //normalized_fft(vec_plane_product[ii][jj],true);
	  itsFFTMachines[0].normalized_forward (vec_plane_product[ii][jj].nrow(),vec_plane_product[ii][jj].data(),OpenMP::maxThreads(), FFTW_MEASURE);

	}
      }
    }


    Array<Complex> output_grid;//(input_grid.shape(),0.);
    Matrix<Complex> ArrMatrix_out;//(IPosition(2,input_grid.shape()[0],input_grid.shape()[0]),0.);
    output_grid.reference(its_output_grid_element);
    output_grid=0.;
    ArrMatrix_out.reference(its_ArrMatrix_out_element);

    Matrix<Complex> Mueller_term;
    for(uInt ii=0;ii<4;++ii){
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  uInt iii,jjj;
	  Float factor;
	  if(npol==4){iii=ii;jjj=jj;factor=1.;}
	  if(npol==1){
	    iii=0;jjj=0;
	    Bool condii((ii/3-float(ii)/3.)==0.);
	    Bool condjj((jj/3-float(jj)/3.)==0.);
	    if(!((condii)&(condjj))){continue;}
	    factor=.5;
	  }
	  Matrix<Complex> ConvFunc(vec_plane_product[ii][jj]);
	  //store(ConvFunc,"Im_AH"+String::toString(ii)+"-"+String::toString(jj)+".img");
	  //Mueller_term.reference(zero_padding(ConvFunc,input_grid.shape()[0]));
	  Mueller_term.reference(zero_padding(ConvFunc, ArrMatrix_out, true));
	  //normalized_fft(Mueller_term,false);
	  itsFFTMachines[0].normalized_backward (Mueller_term.nrow(),Mueller_term.data(),OpenMP::maxThreads(), FFTW_MEASURE);


	  IPosition posIn(4,1,1,1,1);
	  IPosition posOut(4,1,1,1,1);
	  IPosition posEl(2,1,1);
	  Complex pix;
	  uInt jjpix,ch;
	  Complex ElementValue;
	  for(ch=0; ch<input_grid.shape()[3]; ++ch){
#pragma omp parallel
	    {
#pragma omp for private(posIn,posOut,posEl,ch,pix,jjpix,ElementValue)// schedule(dynamic)
	      for(uInt iipix=0; iipix<input_grid.shape()[0]; ++iipix){
		posIn[2]=iii;
		posOut[2]=jjj;
		posIn[3]=ch;
		posOut[3]=ch;
		posIn[0]=iipix;
		posOut[0]=iipix;
		posEl[0]=iipix;
		for(jjpix=0; jjpix<input_grid.shape()[0]; ++jjpix){
		  posIn[1]=jjpix;
		  posOut[1]=jjpix;
		  posEl[1]=jjpix;
		  ElementValue=Mueller_term(posEl)*factor;
		  output_grid(posOut)+=input_grid(posIn)*ElementValue;
		}
	      }
	    }
	  }


	}
      }
    }



    return output_grid;

  }





  //================================================
  // Compute the convolution function for all channel, for the polarisations specified in the Mueller_mask matrix
  // Also specify weither to compute the Mueller matrix for the forward or the backward step. A dirty way to calculate
  // the average beam has been implemented, by specifying the beam correcping to the given baseline and timeslot.
  // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]

  Int LofarConvolutionFunction::GiveWSupport(Double w,uInt spw){
    double ratio_freqs=list_freq_spw[0]/list_freq_spw[spw];
    uInt w_index = m_wScale.plane(w*ratio_freqs);
    Matrix<Complex> wTerm;
    wTerm = m_WplanesStore[w_index];
    return wTerm.shape()[0];
  }

  uInt LofarConvolutionFunction::GiveWindex(Double w,uInt spw){
    double ratio_freqs=list_freq_spw[0]/list_freq_spw[spw];
    uInt w_index = m_wScale.plane(w*ratio_freqs);
    return w_index;
  }

  Int LofarConvolutionFunction::GiveWindexIncludeNegative(Double w,uInt spw){
    double ratio_freqs=list_freq_spw[0]/list_freq_spw[spw];
    Int w_index = m_wScale.plane(w*ratio_freqs);
    if(w<0.){return -w_index;}
    return w_index;
  }

  LofarCFStore LofarConvolutionFunction::makeConvolutionFunction
  (uInt stationA, uInt stationB, Double timeIn, Double w,
   const Matrix<bool>& /*Mask_Mueller_in*/, bool degridding_step,
   double Append_average_PB_CF, Matrix<Complex>& Stack_PB_CF,
   double& sum_weight_square, Vector<uInt> ChanBlockIn, Int /*TaylorTerm*/, double /*RefFreq*/,
   vector< vector < Matrix<Complex> > > & StackMuellerNew, Int ImposeSupport, Bool UseWTerm)
  {
    // Initialize timers.
    PrecTimer timerFFT;
    PrecTimer timerPar;
    PrecTimer timerCyril;
    timerPar.start();

    Double time(GiveClosestTimeAterm(timeIn));
    Vector<uInt> ChanBlock(ChanBlockIn.copy());

    Matrix<bool> Mask_Mueller(IPosition(2,4,4),false);
    Mask_Mueller(0,0)=true;

    // Stack_PB_CF should be called Sum_PB_CF (it is a sum, no stack).
    CountedPtr<CFTypeVec> res (new vector< vector< vector < Matrix<Complex> > > >());
    CFTypeVec& result = *res;
    vector< vector< vector < Matrix<Complex> > > > result_non_padded;


    // Stack the convolution function if averagepb.img don't exist
    //Matrix<Complex> Stack_PB_CF_fft(IPosition(2,m_shape(0),m_shape(0)),0.);
    Bool Stack = (Append_average_PB_CF != 0.);



    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_station =
      m_AtermStore_station.find(time);
    AlwaysAssert (aiter_station!=m_AtermStore_station.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm_station = aiter_station->second;




    ///        if(m_AtermStore.find(time)==m_AtermStore.end()){computeAterm(time);}

    // Load the Wterm
    Matrix<Complex> wTerm;
    Matrix<Complex> wTerm_paddedf;
    Int Npix_out = 0;
    Int Npix_out2 = 0;
    
    for(uInt ch_block=0;ch_block<ChanBlock.size();++ch_block){
      //cout<<" Doing CF for "<<ch_block<<" Block="<<ChanBlock<<", BlockSel="<<ChanBlock[ch_block]<<endl;
    double ratio_freqs=list_freq_spw[0]/list_freq_chanBlock[ChanBlock[ch_block]];
    // Load the Aterm
    //const Cube<Complex> aTermA(aterm_station[stationA][ch].copy());
    Cube<Complex> aTermA(aterm_station[stationA][ChanBlock[ch_block]].copy());
    //const Cube<Complex>& aTermB(aterm_station[stationB][ch]);
    Cube<Complex> aTermB(aterm_station[stationB][ChanBlock[ch_block]].copy());


    if(UseWTerm){
      uInt w_index = m_wScale.plane(w*ratio_freqs);
      wTerm = m_WplanesStore[w_index];
      if (w > 0.) {
	wTerm.reference (conj(wTerm));
      }
      Npix_out = std::max(std::max(aTermA.shape()[0], aTermB.shape()[0]),
			  std::max(wTerm.shape()[0], Spheroid_cut.shape()[0]));
      if(ImposeSupport!=0){Npix_out =ImposeSupport;}
      //Adapt resolution of wterm
      Matrix<Complex> Spheroid_WtermOrig(zero_padding(Spheroid_cut,wTerm.shape()[0]));
      normalized_fft (Spheroid_WtermOrig, false);
      wTerm*=Spheroid_WtermOrig;
      normalized_fft (wTerm, true);
      wTerm_paddedf.reference((zero_padding(wTerm,Npix_out)));
      normalized_fft (wTerm_paddedf, false);
      Matrix<Complex> Spheroid_WtermOrig_Npix(zero_padding(Spheroid_cut,Npix_out));
      normalized_fft (Spheroid_WtermOrig_Npix, false);
      wTerm_paddedf/=Spheroid_WtermOrig_Npix;
    } else{
      Npix_out = std::max(std::max(aTermA.shape()[0], aTermB.shape()[0]),
			  Spheroid_cut.shape()[0]);
      if(ImposeSupport!=0){Npix_out =ImposeSupport;}
      uInt w_index(GiveWindex(w, map_chanBlock_spw[ChanBlock[ch_block]]));
      
      double wcf=m_wScale.center(w_index);
      double ratio_freqs_block=list_freq_chanBlock[0]/list_freq_chanBlock[ChanBlock[ch_block]];
      Float icorr=(abs(w)-wcf)/its_wStep;

      wTerm=its_wCorrGridderMatrix[round(abs(icorr)*ratio_freqs_block)].copy();

      //cout<<"w="<<w<<", cf=("<<wcf<<", "<<w_index<<"), dw="<<abs(w)-wcf<<", i="<<floor(abs(icorr))<<endl;

      if(w<0.){
      	if((abs(w)-wcf)>0.){wTerm=conj(wTerm);}
      } else {
      	if((abs(w)-wcf)<0.){wTerm=conj(wTerm);}
      }
      //wTerm=1.;
      wTerm_paddedf.reference(conj(wTerm));
      //wTerm_paddedf.reference(wTerm);

    }






      //==============================
      //==============================
      // Cyr: MFS
      //==============================
      //==============================
      // if( TaylorTerm > 0 )
      // 	{
      // 	  Float freq=0.0,mulfactor=1.0;
      // 	  freq = list_freq[ch];
      // 	  mulfactor = ((freq-RefFreq)/RefFreq);
      // 	  //cout<<"mulfactor "<<mulfactor<<endl;
      // 	  Cube<Complex> slice(aTermA);
      // 	  slice *= pow(mulfactor,TaylorTerm);//mulfactor;
      // 	}
      //==============================
      //==============================
      // Determine maximum support of A, W, and Spheroidal function for zero padding

      //store(Spheroid_cut,"Spheroid_cut.img");






      //cout << "CF Shapes, Wterm:" << wTerm.shape()[0] << ", Beam " << aTermA.shape()[0] << ", Spheroid: " << Spheroid_cut.shape()[0] << endl;

      // Zero pad to make the image planes of the A1, A2, and W term have the same resolution in the image plane
      Matrix<Complex> Spheroid_cut_paddedf(zero_padding(Spheroid_cut,Npix_out));
      //Matrix<Complex> wTerm_paddedf(zero_padding(wTerm, Npix_out));

      // FFT (backward) the A and W terms
      //normalized_fft (timerFFT, wTerm_paddedf, false);
      normalized_fft (timerFFT, Spheroid_cut_paddedf, false);

      Matrix<Complex> Spheroid_AtermOrig(zero_padding(Spheroid_cut,aTermA.shape()[0]));
      normalized_fft (Spheroid_AtermOrig, false);
      Matrix<Complex> Spheroid_AtermAdapted(zero_padding(Spheroid_cut,Npix_out));
      normalized_fft (Spheroid_AtermAdapted, false);
      for (uInt pol=0; pol<4; ++pol) {
	Matrix<Complex> plane1 (aTermA.xyPlane(pol));
	Matrix<Complex> plane2 (aTermB.xyPlane(pol));
	plane1*=Spheroid_AtermOrig;
	plane2*=Spheroid_AtermOrig;
	normalized_fft (timerFFT, plane1);
	normalized_fft (timerFFT, plane2);
      }

      Cube<Complex> aTermA_padded(zero_padding(aTermA, Npix_out));
      Cube<Complex> aTermB_padded(zero_padding(aTermB, Npix_out));
      //store(aTermA_padded,"aTermA.ft.padded.img");

      for (uInt i=0; i<4; ++i) {
        // Make a matrix referencing the data in the cube's plane.
        Matrix<Complex> planeAf(aTermA_padded.xyPlane(i));
        Matrix<Complex> planeBf(aTermB_padded.xyPlane(i));
        AlwaysAssert(planeAf.contiguousStorage(), AipsError);
        normalized_fft (timerFFT, planeAf, false);
        normalized_fft (timerFFT, planeBf, false);
	planeAf/=Spheroid_AtermAdapted;
	planeBf/=Spheroid_AtermAdapted;
      }
      // Create the vectors of Matrices giving the convolution functions
      // for each Mueller element.
      vector< vector < Matrix<Complex> > > Kron_Product;
      Kron_Product.reserve(4);




     // Something I still don't completely understand: for the average PB calculation.
      // The convolution functions padded with a higher value than the minimum one give a
      // better result in the end. If you try Npix_out2=Npix_out, then the average PB shows
      // structure like aliasing, producing high values in the devided disrty map... This
      // is likely to be due to the way fft works?...
      // FIX: I now do the average of the PB by stacking the CF, FFT the result and square
      // it in the end. This is not the way to do in principle but the result is almost the
      // same. It should pose no problem I think.
      Matrix<Complex> Spheroid_cut_padded2f;
      Matrix<Complex> Spheroid_cut_padded2ftot;
      Matrix<Complex> spheroid_cut_element_fft2;
      Matrix<Complex> spheroid_cut_element_fft2tot;
      Cube<Complex> aTermA_padded2;
      Cube<Complex> aTermB_padded2;
      Cube<Complex> aTermA_padded2tot;
      Cube<Complex> aTermB_padded2tot;

      // Keep the non-padded convolution functions for average PB calculation.
      vector< vector < Matrix<Complex> > > Kron_Product_non_padded;
      Kron_Product_non_padded.reserve(4);

      vector< vector < Matrix<Complex> > > StackMuellerNewTemp;
      Cube<Complex> aTermAtot;
      Cube<Complex> aTermBtot;
      map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_element = m_AtermStore_element.find(time);
      AlwaysAssert (aiter_element!=m_AtermStore_element.end(), AipsError);
      const vector< vector< Cube<Complex> > >& aterm_element = aiter_element->second;
      // map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_element =m_AtermStore.find(time);
      // AlwaysAssert (aiter_element!=m_AtermStore.end(), AipsError);
      // const vector< vector< Cube<Complex> > >& aterm_element = aiter_element->second;
      if (Stack) {
	
        Npix_out2 = Npix_out;//aTermA.shape()[0];//Npix_out;
    
        Spheroid_cut_padded2f = zero_padding(Spheroid_cut, Npix_out2);
      	spheroid_cut_element_fft2 = zero_padding(spheroid_cut_element_fft, Npix_out2);
      	normalized_fft (timerFFT, spheroid_cut_element_fft2, false);
        aTermA_padded2 = aTermA_padded;//zero_padding(aTermA, Npix_out2);
        aTermB_padded2 = aTermB_padded;//zero_padding(aTermB, Npix_out2);
        normalized_fft (timerFFT, Spheroid_cut_padded2f, false);

	
	Spheroid_cut_padded2ftot = zero_padding(Spheroid_cut, m_NPixATerm);
      	spheroid_cut_element_fft2tot = zero_padding(spheroid_cut_element_fft, m_NPixATerm);
      	aTermAtot=aterm_element[stationA][map_chanBlock_spw[ChanBlock[ch_block]]].copy();
      	aTermBtot=aterm_element[stationB][map_chanBlock_spw[ChanBlock[ch_block]]].copy();
        aTermA_padded2tot = zero_padding(aTermAtot, m_NPixATerm);
        aTermB_padded2tot = zero_padding(aTermBtot, m_NPixATerm);


      	StackMuellerNewTemp.resize(4);
      	for (uInt i=0; i<4; ++i) {
      	  StackMuellerNewTemp[i].resize(4);
      	  for (uInt j=0; j<4; ++j) {
      	    StackMuellerNewTemp[i][j].resize(IPosition(2,m_NPixATerm,m_NPixATerm));
      	  }
      	}
      }






      // Compute the Mueller matrix considering the Mueller Mask

      uInt ind0;
      uInt ind1;
      uInt ii = 0;
      IPosition cfShape;
      Bool allElem = True;
      for (uInt row0=0; row0<=1; ++row0) {
        for (uInt col0=0; col0<=1; ++col0) {
          vector < Matrix<Complex> > Row(4);
          vector < Matrix<Complex> > Row_non_padded(4);
          uInt jj = 0;
          for (uInt row1=0; row1<=1; ++row1) {
            for (uInt col1=0; col1<=1; ++col1) {
              // This Mueller ordering is for polarisation given as XX,XY,YX YY
              ind0 = row0 + 2*row1;
              ind1 = col0 + 2*col1;
              // Compute the convolution function for the given Mueller element
              if (Mask_Mueller(ii,jj)) {
                // Padded version for oversampling the convolution function
                Matrix<Complex> plane_product (aTermB_padded.xyPlane(ind0) *
                                               conj(aTermA_padded.xyPlane(ind1)));
		//plane_product=1.;
		//if(UseWTerm){
		  plane_product *= wTerm_paddedf;
		//}
                plane_product *= Spheroid_cut_paddedf;
		  for(uInt iii=0; iii<plane_product.shape()[0];++iii){
		    for(uInt jjj=0; jjj<plane_product.shape()[0];++jjj){
		      if(!(abs(plane_product(iii,jjj))<1e6)){plane_product(iii,jjj)=0.;};
		    }
		  }

                Matrix<Complex> plane_product_paddedf
                  (zero_padding(plane_product,
                                plane_product.shape()[0] * m_oversampling));
                normalized_fft (timerFFT, plane_product_paddedf);

                plane_product_paddedf *= static_cast<Float>(m_oversampling *
                                                            m_oversampling);
		if (itsVerbose>3 && row0==0 && col0==0 && row1==0 && col1==0) {
		  store (plane_product_paddedf, "awfft"+String::toString(stationA)+'-'+String::toString(stationB));
		}

                // Maybe to do:
                // Find circle (from outside to inside) until value > peak*1e-3.
                // Cut out that box to use as the convolution function.
                // See nPBWProjectFT.cc (findSupport).

                Row[jj].reference (plane_product_paddedf);
                cfShape = plane_product_paddedf.shape();
                // Non padded version for PB calculation (no W-term)
                if (Stack) {
                  Matrix<Complex> plane_productf(conj(aTermB_padded2.xyPlane(ind0))*
                                                 aTermA_padded2.xyPlane(ind1));
		  //Some Nans are created at the resolution adaptation above, for the very low values.
		  //Replace them ny zero here.
		  for(uInt iii=0; iii<plane_productf.shape()[0];++iii){
		    for(uInt jjj=0; jjj<plane_productf.shape()[0];++jjj){
		      if(!(abs(plane_productf(iii,jjj))<1e6)){plane_productf(iii,jjj)=0.;};
		    }
		  }
		  plane_productf*=conj(plane_productf);
                  plane_productf *= Spheroid_cut_padded2f;
		  ////if(its_Apply_Element){plane_productf *= spheroid_cut_element_fft2;}
                  normalized_fft (timerFFT, plane_productf);
                  Row_non_padded[jj].reference (plane_productf);

                }
              } else {
                allElem = False;
              }
              ++jj;
            }
          }
          ++ii;
          Kron_Product.push_back(Row);
          if (Stack) {
            // Keep non-padded for primary beam calculation.
            Kron_Product_non_padded.push_back(Row_non_padded);
          }
        }
      }

      if (Stack) {
	ii = 0;
	for (uInt row0=0; row0<=1; ++row0) {
	  for (uInt col0=0; col0<=1; ++col0) {
	    uInt jj = 0;
	    for (uInt row1=0; row1<=1; ++row1) {
	      for (uInt col1=0; col1<=1; ++col1) {
		// This Mueller ordering is for polarisation given as XX,XY,YX YY
		ind0 = row0 + 2*row1;
		ind1 = col0 + 2*col1;
		Matrix<Complex> plane_productftot(conj(aTermB_padded2tot.xyPlane(ind0))*
						  aTermA_padded2tot.xyPlane(ind1));
		
		//plane_productftot *= Spheroid_cut_padded2ftot;
		//if(its_Apply_Element){plane_productftot *= spheroid_cut_element_fft2tot;}
		StackMuellerNewTemp[ii][jj]=plane_productftot.copy();
		++jj;
	      }
	    }
	    ++ii;
	  }
	}
      }


      // When degridding, transpose and use conjugate.
      if (degridding_step) {
        for (uInt i=0; i<4; ++i) {
          for (uInt j=i; j<4; ++j) {
            //AlwaysAssert (Mask_Mueller(i,j) == Mask_Mueller(j,i), AipsError);
	    if ((Mask_Mueller(i,j)==false)&&(Mask_Mueller(j,i)==true)){
	      Matrix<Complex> a(Kron_Product[i][j].copy());
	      a=0.;
	      Kron_Product[i][j]=a.copy();
	    };
            if (Mask_Mueller(i,j)) {
              if (i!=j) {
                Matrix<Complex> conj_product(conj(Kron_Product[i][j]));
                Kron_Product[i][j].reference (conj(Kron_Product[j][i]));
                Kron_Product[j][i].reference (conj_product);
              } else {
                Kron_Product[i][j].reference (conj(Kron_Product[i][j]));
              }
            }
          }
        }
      }

      // Put similarly shaped matrix with zeroes for missing Mueller elements.
      if (!allElem) {
        Matrix<Complex> zeroCF(cfShape);
        for (uInt i=0; i<4; ++i) {
          for (uInt j=0; j<4; ++j) {
            if (! Mask_Mueller(i,j)) {
              Kron_Product[i][j].reference (zeroCF);
            }
          }
        }
      }
      // Add the conv.func. for this channel to the result.
      result.push_back(Kron_Product);
      if (Stack) {
        result_non_padded.push_back(Kron_Product_non_padded);
      }
      //}

    // Stacks the weighted quadratic sum of the convolution function of
    // average PB estimate (!!!!! done for channel 0 only!!!)
    if (Stack) {
      //	  cout<<"...Stack CF for PB estimate"<<endl;
      double weight_square = 4. * Append_average_PB_CF * Append_average_PB_CF;
      double weight_sqsq = weight_square * weight_square;
      for (uInt i=0; i<4; ++i) {
        //if((i==2)||(i==1)) break;
        for (uInt j=0; j<4; ++j) {
          // Only use diagonal terms for average primary beam.
          if (i==j  &&  Mask_Mueller(i,j)) {
            //Stack_PB_CF=0.;
            double istart = 0;//0.5 * (m_shape[0] - Npix_out2);
            //if (istart-floor(istart) != 0.) {
            //  istart += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
            //}
            for (Int jj=0; jj<Npix_out2; ++jj) {
              for (Int ii=0; ii<Npix_out2; ++ii) {
                Complex gain = result_non_padded[0][i][j](ii,jj);
                Stack_PB_CF(istart+ii,istart+jj) += gain;//*weight_sqsq;
              }
            }
            sum_weight_square += 1.;//weight_sqsq*weight_sqsq;
          }
        }
      }
      
      for (uInt jj=0; jj<m_NPixATerm; ++jj) {
      	for (uInt ii=0; ii<m_NPixATerm; ++ii) {
      	  for (uInt i=0; i<4; ++i) {
      	    for (uInt j=0; j<4; ++j) {
      	      for (uInt ind=0; ind<4; ++ind) {
      		(StackMuellerNew[i][j])(IPosition(2,ii,jj))=(StackMuellerNew[i][j])(IPosition(2,ii,jj))+conj((StackMuellerNewTemp[ind][i])(IPosition(2,ii,jj)))*(StackMuellerNewTemp[ind][j])(IPosition(2,ii,jj))*weight_sqsq;
      	      }
      	    }
      	  }
      	}
      }
      its_VectorThreadsSumWeights[OpenMP::threadNum()]+=weight_sqsq;
      


    }

    }

    
    // Put the resulting vec(vec(vec))) in a LofarCFStore object
    CoordinateSystem csys;
    Vector<Float> samp(2, m_oversampling);
    Vector<Int> xsup(1, Npix_out/2);
    Vector<Int> ysup(1, Npix_out/2);
    Int maxXSup(Npix_out);///2);
    Int maxYSup(Npix_out);///2);
    Quantity PA(0., "deg");
    Int mosPointing(0);

    // Update the timing info.
    timerPar.stop();
    double ftime = timerFFT.getReal();
#pragma omp atomic
    itsTimeCFfft += ftime;
    unsigned long long cnt = timerFFT.getCount();
#pragma omp atomic
    itsTimeCFcnt += cnt;
    double ptime = timerPar.getReal();
#pragma omp atomic
    itsTimeCFpar += ptime;


    return LofarCFStore (res, csys, samp,  xsup, ysup, maxXSup, maxYSup,
                         PA, mosPointing, Mask_Mueller);
  }


  //================================================

  // Returns the average Primary Beam from the disk
  void LofarConvolutionFunction::Make_MuellerAvgPB(vector< vector< vector < Matrix<Complex> > > > & StackMueller, double /*sum_weight_square*/)
  {
    cout<<"  Computing MuellerAvgPB"<<endl;
    cout<<"... stack the stacks"<<endl;
    vector< vector < Matrix< Complex > > > MeanTerm;
    vector< vector < Matrix<Complex> > >   StackMuellerStack;
    StackMuellerStack.resize(4);
    MeanTerm.resize(4);
    for (uInt i=0; i<4; ++i) {
      StackMuellerStack[i].resize(4);
      MeanTerm[i].resize(4);
      for (uInt j=0; j<4; ++j) {
  	StackMuellerStack[i][j].resize(IPosition(2,m_NPixATerm,m_NPixATerm));
	StackMuellerStack[i][j]=Complex(0.);
  	MeanTerm[i][j].resize(IPosition(2, m_shape[0], m_shape[1]));
	MeanTerm[i][j]=Complex(0.);
      }
    }
    
    Double tot_weights(0.);
    for (uInt t=0; t<StackMuellerStack.size(); ++t) {
      tot_weights+=its_VectorThreadsSumWeights[t];
    }



    for (uInt t=0; t<StackMuellerStack.size(); ++t) {
	for (uInt i=0; i<4; ++i) {
	  StackMuellerStack[i].resize(4);
	  for (uInt j=0; j<4; ++j) {
	    StackMuellerStack[i][j].resize(IPosition(2,m_NPixATerm,m_NPixATerm));
	    StackMuellerStack[i][j]+=StackMueller[t][i][j]/tot_weights;
	  }
	}
    }

    //////////////////////////////////////////////////////////////////////////
    // Matrix<Complex> SpheroidBeam(IPosition(2,m_NPixATerm,m_NPixATerm),1.);
    // taper(SpheroidBeam);
    // normalized_fft (SpheroidBeam);
    // Matrix<Complex> SpheroidBeam_big(zero_padding(SpheroidBeam,501));
    // normalized_fft (SpheroidBeam_big,false);
    
    // for (uInt ind0beam=0; ind0beam<4; ++ind0beam) {
    //   for (uInt ind1beam=0; ind1beam<4; ++ind1beam) {
    // 	    // This Mueller ordering is for polarisation given as XX,XY,YX YY
    // 	Matrix<Complex> plane_product(StackMuellerStack[ind0beam][ind1beam]);
    // 	taper(plane_product);
    // 	normalized_fft (plane_product);
    // 	Matrix<Complex> plane_product_big(zero_padding(plane_product,501));
    // 	normalized_fft (plane_product_big,false);
    // 	Matrix<Float> plane_product_big_float(IPosition(2,plane_product_big.shape()[0],plane_product_big.shape()[0]),0.);
	
    // 	plane_product_big/=SpheroidBeam_big;
    // 	for (uInt ipix=0; ipix<plane_product_big.shape()[0]; ++ipix){
    // 	  for (uInt jpix=0; jpix<plane_product_big.shape()[0]; ++jpix){
    // 	    plane_product_big_float(ipix,jpix)=abs(plane_product_big(ipix,jpix));
    // 	  };
    // 	}
    // 	//plane_product_big_float=abs(plane_product_big);
	
    // 	store (m_coordinates, plane_product_big_float, "Mueller"+String::toString(ind0beam)+'-'+String::toString(ind1beam));
    // 	//store (plane_product, "Beam"+String::toString(iii)+'-'+String::toString(jj));
    //   }
    // }
    // //make beam image
    // assert(false);
    ////////////////////////////////////////////////////////////////////////////






    Matrix< Complex > SpheMeanTermCut(IPosition(2, m_NPixATerm,m_NPixATerm),1.);
    taper(SpheMeanTermCut);
    normalized_fft (SpheMeanTermCut, true);
    Matrix<Complex> SpheMeanTerm(zero_padding(SpheMeanTermCut, m_shape[1]));
    normalized_fft (SpheMeanTerm, false);
    //store (SpheMeanTerm, "Sphetest");
    double istart = 0.5 * (m_shape[0] - m_NPixATerm);
    if (istart-floor(istart) != 0.) {
      istart += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
    }

    // for (uInt i=0; i<4; ++i) {
    //   for (uInt j=0; j<4; ++j) {
    // 	store (StackMuellerStack[i][j], "Bef"+String::toString(i)+'-'+String::toString(j));
    //   }
    // }

    cout<<"... adapt resolution"<<endl;
    for (uInt i=0; i<4; ++i) {
      for (uInt j=0; j<4; ++j) {
	taper(StackMuellerStack[i][j]);
	MeanTerm[i][j]=Complex(0.);
	normalized_fft (StackMuellerStack[i][j], true);
	for (uInt jj=0; jj<m_NPixATerm; ++jj) {
	  for (uInt ii=0; ii<m_NPixATerm; ++ii) {
	    Complex gain = StackMuellerStack[i][j](ii,jj);
	    MeanTerm[i][j](istart+ii,istart+jj) = gain;
	  }
	}
	normalized_fft (MeanTerm[i][j], false);
	MeanTerm[i][j]/=SpheMeanTerm;
	//MeanTerm[i][j]*=Spheroid_cut_im;
	//MeanTerm[i][j]*=Spheroid_cut_im_element;
	//MeanTerm[i][j]*=SpheMeanTerm;
	//store (MeanTerm[i][j], "Mean"+String::toString(i)+'-'+String::toString(j));
      }
    }

    cout<<"...invert"<<endl;

    //invert 4*4 matrix...
    
    vector< vector< Matrix < Complex > > > InvertMat;
    InvertMat.resize(m_shape[0]);
    for (uInt i=0; i<m_shape[0]; ++i) {
      InvertMat[i].resize(m_shape[0]);
      for (uInt j=0; j<m_shape[0]; ++j) {
  	InvertMat[i][j].resize(IPosition(2, 4, 4));
	InvertMat[i][j]=Complex(0.);
      }
    }

#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
    for (uInt i=0; i<m_shape[0]; ++i) {
      for (uInt j=0; j<m_shape[0]; ++j) {
	for (uInt ii=0; ii<4; ++ii) {
	  for (uInt jj=0; jj<4; ++jj) {
	    InvertMat[i][j](ii,jj)=MeanTerm[ii][jj](i,j);
	  }
	}
	InvertMat[i][j]=invert(InvertMat[i][j]);
      }
    }
    }

    Matrix<Complex> TempIm;
    TempIm.resize(IPosition(2, m_shape[0], m_shape[1]));
    //Matrix<Float> TempImFlt;
    //TempImFlt.resize(IPosition(2, m_shape[0], m_shape[1]));
    for (uInt ii=0; ii<4; ++ii) {
      for (uInt jj=0; jj<4; ++jj) {
	for (uInt i=0; i<m_shape[0]; ++i) {
	  for (uInt j=0; j<m_shape[0]; ++j) {
	    TempIm(i,j)=InvertMat[i][j](ii,jj);
	    //TempImFlt(i,j)=abs(InvertMat[i][j](ii,jj));
	  }
	}
	store (m_coordinates, TempIm, "JAWS_products/"+itsImgName + ".MuellerPB."+String::toString(ii)+'-'+String::toString(jj));
	//store (m_coordinates, TempImFlt, "JAWS_products/"+itsImgName + ".MuellerPB."+String::toString(ii)+'-'+String::toString(jj));
      }
    }
    
    



    cout<<"  ... done!"<<endl;
  }

  Array<Complex> LofarConvolutionFunction::Correct_CC(Array<Complex>& ModelImage)
  {
    cout<<"  Correcting CC image..."<<endl;
    Array<Complex> CorrectedModelImage;
    uInt nx=ModelImage.shape()[0];
    uInt npol=ModelImage.shape()[2];
    uInt nchan=ModelImage.shape()[3];
    
    IPosition gridShape(4, nx,nx,npol,nchan);
    CorrectedModelImage.resize(gridShape);
    CorrectedModelImage=Complex(0.);
    Matrix< Complex > data0S;
    data0S.resize(IPosition(2,nx,nx));

    
    Matrix<Float> AvgPB = getAveragePB(itsImgName);
    
      
    Float MaxPB(0.);
    for(uInt i=0; i<nx; ++i)
      {
	for(uInt j=0; j<nx; ++j)
	  {
	    if(AvgPB(i,j)>MaxPB){MaxPB=AvgPB(i,j);}
	  }
      }
      
    
    for(uInt k0=0;k0<npol;++k0){
      for(uInt k1=0;k1<npol;++k1){
	//cout<<"  ... doing "<<k0<<" "<<k1<<endl;
	String File_name("JAWS_products/"+itsImgName + ".MuellerPB."+String::toString(k0)+'-'+String::toString(k1));
	PagedImage<Complex> tmp(File_name);
	data0S=tmp.get (True);
	//cout<<"  ... ok "<<endl;
	

#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
	for(uInt i=0;i<nx;++i){
	  IPosition pos0(4,nx,nx,3,1);
	  pos0(0)=i;
	  pos0(2)=0;
	  pos0(3)=0;
	  pos0(2)=k0;
	  IPosition pos1(4,nx,nx,3,1);
	  pos1(0)=i;
	  pos1(2)=0;
	  pos1(3)=0;
	  pos1(2)=k1;
	  for(uInt j=0;j<nx;++j){
	      pos0(1)=j;
	      pos1(1)=j;
	      //if((i==nx/2-1)&(j==nx/2-1)){cout<<i<<" "<<j<<" "<<CorrectedModelImage(pos0)<<" "<<data0S(i,j)<<" "<<ModelImage(pos1);}
	      CorrectedModelImage(pos0)+=data0S(i,j)*ModelImage(pos1);///(Spheroid_cut_im_element(i,j)*Spheroid_cut_im(i,j)*sqrt(MaxPB)/sqrt(AvgPB(i,j)));
	    }
	  }
    }
      }
      }
      cout<<"  ... done ..."<<endl;
      return CorrectedModelImage;
  }

  Matrix<Float> LofarConvolutionFunction::Give_avg_pb()
  {
    // Only read if not available.
    if (Im_Stack_PB_CF0.empty()) {
      if (itsVerbose > 0) {
        cout<<"==============Give_avg_pb()"<<endl;
      }
      String PBFile_name(itsImgName + ".avgpb");
      File PBFile(PBFile_name);
      if (! PBFile.exists()) {
        throw SynthesisError (PBFile_name + " not found");
      }
      if (itsVerbose > 0) {
        cout<<"..... loading Primary Beam image from disk ....."<<endl;
      }
      PagedImage<Float> tmp(PBFile_name);
      IPosition shape(tmp.shape());
      AlwaysAssert (shape[0]==m_shape[0] && shape[1]==m_shape[1], AipsError);
      tmp.get (Im_Stack_PB_CF0, True);   // remove degenerate axes.
    }
    return Im_Stack_PB_CF0;
  }

  // Compute the average Primary Beam from the Stack of convolution functions
  Matrix<Float> LofarConvolutionFunction::Compute_avg_pb
  (Matrix<Complex>& Sum_Stack_PB_CF_small, double sum_weight_square)
  {
    // Only calculate if not done yet.
    if (Im_Stack_PB_CF0.empty()) {
      //if (itsVerbose > 0) {
        cout<<"..... Compute average PB"<<endl;
      //}
      Sum_Stack_PB_CF_small /= float(sum_weight_square);
      Matrix<Complex> Sum_Stack_PB_CF(zero_padding(Sum_Stack_PB_CF_small,m_shape[0]));

      //store(Sum_Stack_PB_CF,"Stack_PB_CF.img");

      normalized_fft_parallel(Sum_Stack_PB_CF, false);
      
      Sum_Stack_PB_CF/=Spheroid_cut_im;
      //store(Sum_Stack_PB_CF,"Im_Stack_PB_CF00.img");
      //store(Sum_Stack_PB_CF, itsImgName + ".before");
      Im_Stack_PB_CF0.resize (IPosition(2, m_shape[0], m_shape[1]));
      Im_Stack_PB_CF0=0.;


      // if(its_Apply_Element){
      // 	Sum_Stack_PB_CF*=Spheroid_cut_im_element;
      // 	Sum_Stack_PB_CF*=Spheroid_cut_im_element;
      // }
      // if(its_UseWSplit){
      // 	Sum_Stack_PB_CF*=Spheroid_cut_im;
      // 	Sum_Stack_PB_CF*=Spheroid_cut_im;
      // }

      float maxPB(0.);
      for(uInt i=0;i<m_shape[1];++i){
	for(uInt j=0;j<m_shape[1];++j){
	    Complex pixel(Sum_Stack_PB_CF(i,j));
	    if(abs(pixel)>maxPB){
	      maxPB=abs(pixel);
	      //maxPB_noabs=pixel;
	    };
	}
      }
      for (Int jj=0; jj<m_shape[1]; ++jj) {
        for (Int ii=0; ii<m_shape[0]; ++ii) {
          Float absVal = abs(Sum_Stack_PB_CF(ii,jj));
          //Im_Stack_PB_CF0(ii,jj) = std::max (absVal*absVal, threshold*maxPB);
          Im_Stack_PB_CF0(ii,jj) = absVal;//std::max (absVal, threshold*maxPB);
	  //Im_Stack_PB_CF0(ii,jj) = sqrt(Im_Stack_PB_CF0(ii,jj))*sign(maxPB_noabs);
        }
      }
      // Make it persistent.
      store(m_coordinates,Im_Stack_PB_CF0, itsImgName + ".avgpb");
    }
    return Im_Stack_PB_CF0;
  }

  //================================================
  // Does Zeros padding of a Cube
  Cube<Complex> LofarConvolutionFunction::zero_padding
  (const Cube<Complex>& Image, int Npixel_Out)
  {
    if (Image.shape()[0] == Npixel_Out) {
      return Image.copy();
    }

    // if ((Npixel_Out%2) != 1) {
    //   Npixel_Out++;
    // }

    Cube<Complex> Image_Enlarged(Npixel_Out,Npixel_Out,Image.shape()[2]);
    uInt Dii = Image.shape()(0)/2;
    uInt Start_image_enlarged=Npixel_Out/2-Dii; //Is an even number, Assume square image
    // if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) {
    //   Start_image_enlarged += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
    // }
    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;} */

    //double ratio(double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0)));
    //if(!toFrequency){ratio=1./ratio;}
    double ratio=1.;

    for (Int pol=0; pol<Image.shape()[2]; ++pol) {
      //cout<<"pol: "<<pol<<endl;
      for (Int jj=0; jj<Image.shape()[1]; ++jj) {
        for (Int ii=0; ii<Image.shape()[0]; ++ii) {
          Image_Enlarged(Start_image_enlarged+ii,
                         Start_image_enlarged+jj,pol) = ratio*Image(ii,jj,pol);
        }
      }
    }
    return Image_Enlarged;
  }

  //================================================
  // Zeros padding of a Matrix

  Matrix<Complex> LofarConvolutionFunction::zero_padding
  (const Matrix<Complex>& Image, int Npixel_Out)
  {
    if (Image.shape()[0] == Npixel_Out) {
      return Image.copy();
    }
    IPosition shape_im_out(2, Npixel_Out, Npixel_Out);
    Matrix<Complex> Image_Enlarged(shape_im_out, 0.);

    double ratio=1.;

    //if(!toFrequency){ratio=double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0));}

    uInt Dii = Image.shape()[0]/2;
    uInt Start_image_enlarged = shape_im_out[0]/2-Dii;
    //Is an even number, Assume square image
    //If number of pixel odd then 0th order at the center, shifted by one otherwise
    // if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) {
    //   Start_image_enlarged += 0.5;
    // }

    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;} */
    for (Int jj=0; jj<Image.shape()[1]; ++jj) {
      for (Int ii=0; ii<Image.shape()[0]; ++ii) {
        Image_Enlarged(Start_image_enlarged+ii,Start_image_enlarged+jj) =
          ratio*Image(ii,jj);
      }
    }
    return Image_Enlarged;
  }

  Matrix<Complex> LofarConvolutionFunction::zero_padding
  (const Matrix<Complex>& Image, Matrix<Complex>& Image_Enlarged, bool tozero)
  {
    int Npixel_Out(Image_Enlarged.shape()[0]);
    if (Image.shape()[0] == Npixel_Out) {
      return Image.copy();
    }
    IPosition shape_im_out(2, Npixel_Out, Npixel_Out);
    if(tozero){
#pragma omp parallel
      {
#pragma omp for schedule(dynamic)
	for (Int i=0; i<Npixel_Out; ++i) {
	  for (Int j=0; j<Npixel_Out; ++j) {
	    Image_Enlarged(i,j)=0.;
	  }
	}
      }
    }


    double ratio=1.;

    //if(!toFrequency){ratio=double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0));}

    uInt Dii = Image.shape()[0]/2;
    uInt Start_image_enlarged = shape_im_out[0]/2-Dii;
    //Is an even number, Assume square image
    //If number of pixel odd then 0th order at the center, shifted by one otherwise
    // if ((Start_image_enlarged-floor(Start_image_enlarged))!=0.) {
    //   Start_image_enlarged += 0.5;
    // }

    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;} */
    for (Int jj=0; jj<Image.shape()[1]; ++jj) {
      for (Int ii=0; ii<Image.shape()[0]; ++ii) {
        Image_Enlarged(Start_image_enlarged+ii,Start_image_enlarged+jj) =
          ratio*Image(ii,jj);
      }
    }
    return Image_Enlarged;
  }

  //================================================
  void LofarConvolutionFunction::normalized_fft
  (Matrix<Complex> &im, bool toFreq)
  {
    //cout<<" "<<im.ncolumn()<<" "<<im.nrow()<<" "<<im.size()<<" "<<im.contiguousStorage()<<endl;
    AlwaysAssert (im.ncolumn() == im.nrow()  &&  im.size() > 0  &&
                  im.contiguousStorage(), AipsError);
    int tnr = OpenMP::threadNum();
    if (toFreq) {
      itsFFTMachines[tnr].normalized_forward (im.nrow(), im.data());
    } else {
      itsFFTMachines[tnr].normalized_backward (im.nrow(), im.data());
    }
  }

  void LofarConvolutionFunction::normalized_fft_parallel
  (Matrix<Complex> &im, bool toFreq)
  {
    //cout<<" "<<im.ncolumn()<<" "<<im.nrow()<<" "<<im.size()<<" "<<im.contiguousStorage()<<endl;
    AlwaysAssert (im.ncolumn() == im.nrow()  &&  im.size() > 0  &&
                  im.contiguousStorage(), AipsError);
    if (toFreq) {
      itsFFTMachines[0].normalized_forward (im.nrow(), im.data(),OpenMP::maxThreads(), FFTW_MEASURE);
    } else {
      itsFFTMachines[0].normalized_backward (im.nrow(), im.data(),OpenMP::maxThreads(), FFTW_MEASURE);
    }
  }

  void LofarConvolutionFunction::normalized_fft
  (PrecTimer& timer, Matrix<Complex> &im, bool toFreq)
  {
    timer.start();
    normalized_fft (im, toFreq);
    timer.stop();
  }

  //=================================================
  MEpoch LofarConvolutionFunction::observationStartTime
  (const MeasurementSet &ms, uInt idObservation) const
  {
    // Get phase center as RA and DEC (J2000).
    ROMSObservationColumns observation(ms.observation());
    AlwaysAssert(observation.nrow() > idObservation, SynthesisError);
    AlwaysAssert(!observation.flagRow()(idObservation), SynthesisError);

    return observation.timeRangeMeas()(0)(IPosition(1, 0));
  }

  //=================================================
  // Estime spheroidal convolution function from the support of the fft of the spheroidal in the image plane

  Double LofarConvolutionFunction::makeSpheroidCut()
  {
    // Only calculate if not done yet.
    if (! Spheroid_cut_im.empty()) {
      return m_pixelSizeSpheroidal;
    }
    Matrix<Complex> spheroidal(m_shape[0], m_shape[1], 1.);
    taper_parallel(spheroidal);
    if (itsVerbose > 0) {
      store(spheroidal, itsImgName + ".spheroidal");
    }
    normalized_fft_parallel(spheroidal);
    Double Support_Speroidal = itsParameters.asDouble("SpheSupport");//11.;//11.;//findSupport(spheroidal, 0.0001);
    Support_Speroidal = FFTCMatrix::optimalOddFFTSize (Support_Speroidal);
    //if (itsVerbose > 0) {
    //      store(spheroidal, itsImgName + ".spheroidal_fft");
    //}

    Double res_ini = abs(m_coordinates.increment()(0));
    Double diam_image = res_ini*m_shape[0];
    Double Pixel_Size_Spheroidal = diam_image/Support_Speroidal;
    uInt Npix = floor(diam_image/Pixel_Size_Spheroidal);
    if (Npix%2 != 1) {
    // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
      ++Npix;
      Pixel_Size_Spheroidal = diam_image/Npix;
    }
    cout<<"    Spheroidal support: "<<Npix<<endl;
    
    Matrix<Complex> Spheroid_cut0(IPosition(2,Npix,Npix),1.);
    Spheroid_cut=Spheroid_cut0;
    double istart(m_shape[0]/2.-Npix/2.);
    if ((istart-floor(istart))!=0.) {
      //If number of pixel odd then 0th order at the center, shifted by one otherwise
      istart += 0.5;
    }
    for (uInt j=0; j<Npix; ++j) {
      for (uInt i=0; i<Npix; ++i) {
        Spheroid_cut(i,j) = spheroidal(istart+i,istart+j);
      }
    }
    //taper(Spheroid_cut);
    //store(Spheroid_cut, itsImgName + ".spheroid_cut");
    Matrix<Complex> Spheroid_cut_paddedf=zero_padding(Spheroid_cut,m_shape[0]);
    normalized_fft_parallel(Spheroid_cut_paddedf, false);
    //normalized_fft(Spheroid_cut,false);
    store(Spheroid_cut, itsImgName + ".spheroid_cut");
    Spheroid_cut_im.reference (real(Spheroid_cut_paddedf));
    // Only this one is really needed.

    store(Spheroid_cut_im, itsImgName + ".spheroid_cut_im");
    return Pixel_Size_Spheroidal;
  }

  const Matrix<Float>& LofarConvolutionFunction::getSpheroidCut()
  {
    if (Spheroid_cut_im.empty()) {
      makeSpheroidCut();
    }
    return Spheroid_cut_im;
  }

  Matrix<Float> LofarConvolutionFunction::getSpheroid(uInt npix)
  {
    Matrix<Float> ones(IPosition(2,npix,npix),1.);
    taper(ones);
    // Matrix<Float> spheratio;
    // spheratio=Spheroid_cut_im/ones;
    // spheratio*=spheratio*spheratio;
    // for(uInt ii=0;ii<spheratio.shape()[0];++ii){
    //   for(uInt jj=0;jj<spheratio.shape()[0];++jj){
    // 	spheratio(ii,jj)=log(abs(spheratio(ii,jj)));
    //   }
    // }
    store(ones,"Spherio");
    return ones;
  }

  Matrix<Float> LofarConvolutionFunction::getSpheroidCut (const String& imgName)
  {
    PagedImage<Float> im(imgName+".spheroid_cut_im");
    return im.get (True);
  }

  Matrix<Float> LofarConvolutionFunction::getAveragePB (const String& imgName)
  {
    PagedImage<Float> im(imgName+".avgpb");
    return im.get (True);
  }

  //=================================================
  // Return the angular resolution required for making the image of the angular size determined by
  // coordinates and shape. The resolution is assumed to be the same on both direction axes.
  Double LofarConvolutionFunction::estimateWResolution
  (const IPosition &shape, Double pixelSize,
   Double w) const
  {
    Double diam_image = pixelSize*shape[0];         // image diameter in radian
    if (w == 0.) {
      return diam_image;
    }
    // Get pixel size in W-term image in radian
    Double Res_w_image = .5/(sqrt(2.)*w*(shape[0]/2.)*pixelSize);
    // Get number of pixel size in W-term image
    uInt Npix=floor(diam_image/Res_w_image);
    Res_w_image = diam_image/Npix;
    if (Npix%2 != 1) {
      // Make the resulting image have an even number of pixel
      // (to make the zeros padding step easier)
      ++Npix;
      Res_w_image = diam_image/Npix;
    }
    return Res_w_image;
  }

  //=================================================
  // Return the angular resolution required for making the image of the angular size determined by
  // coordinates and shape. The resolution is assumed to be the same on both direction axes.
  Double LofarConvolutionFunction::estimateAResolution
  (const IPosition &shape, const DirectionCoordinate &coordinates, double station_diam) const
  {
    Double res_ini=abs(coordinates.increment()(0));                      // pixel size in image in radian
    Double diam_image=res_ini*shape(0);                                  // image diameter in radian
    station_diam = 70.;                                           // station diameter in meters: To be adapted to the individual station size.
    Double Res_beam_image= 0.5*((C::c/m_refFrequency)/station_diam)/2.;      // pixel size in A-term image in radian
    uInt Npix=itsParameters.asDouble("SpheSupport");//floor(diam_image/Res_beam_image);                         // Number of pixel size in A-term image
    Npix = FFTCMatrix::optimalOddFFTSize (Npix);
    Res_beam_image=diam_image/Npix;
    if (Npix%2 != 1) {
      // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
      ++Npix;
      Res_beam_image = diam_image/Npix;
    }
    return Res_beam_image;
  }

  //=================================================
  Double LofarConvolutionFunction::spheroidal(Double nu) const
  {
    static Double P[2][5] = {{ 8.203343e-2, -3.644705e-1, 6.278660e-1,
                              -5.335581e-1,  2.312756e-1},
                             { 4.028559e-3, -3.697768e-2, 1.021332e-1,
                              -1.201436e-1, 6.412774e-2}};
    static Double Q[2][3] = {{1.0000000e0, 8.212018e-1, 2.078043e-1},
                             {1.0000000e0, 9.599102e-1, 2.918724e-1}};
    uInt part = 0;
    Double end = 0.0;
    if (nu >= 0.0 && nu < 0.75) {
      part = 0;
      end = 0.75;
    } else if (nu >= 0.75 && nu <= 1.00) {
      part = 1;
      end = 1.00;
    } else {
      return 0.0;
    }
    Double nusq = nu * nu;
    Double delnusq = nusq - end * end;
    Double delnusqPow = delnusq;
    Double top = P[part][0];
    for (uInt k=1; k<5; ++k) {
      top += P[part][k] * delnusqPow;
      delnusqPow *= delnusq;
    }

    Double bot = Q[part][0];
    delnusqPow = delnusq;
    for (uInt k=1; k<3; ++k) {
      bot += Q[part][k] * delnusqPow;
      delnusqPow *= delnusq;
    }

    double result = (bot == 0  ?  0 : (1.0 - nusq) * (top / bot));
    //if(result<1.e-3){result=1.e-3;}
    return result;
  }

  void LofarConvolutionFunction::showTimings (ostream& os,
                                              double duration,
					      double timeCF) const
  {
    os << "  Wterm calculation ";
    showPerc1 (os, itsTimeW, duration);
    os << "    fft-part ";
    showPerc1 (os, itsTimeWfft, itsTimeW);
    os << "  (";
    showPerc1 (os, itsTimeWfft, duration);
    os << " of total;   #ffts=" << itsTimeWcnt << ')' << endl;
    os << "  Aterm calculation ";
    showPerc1 (os, itsTimeA, duration);
    os << "    fft-part ";
    showPerc1 (os, itsTimeAfft, itsTimeA);
    os << "  (";
    showPerc1 (os, itsTimeAfft, duration);
    os << " of total;   #ffts=" << itsTimeAcnt << ')' << endl;
    os << "  CFunc calculation ";
    showPerc1 (os, timeCF, duration);
    os << "    fft-part ";
    showPerc1 (os, itsTimeCFfft, timeCF);
    os << "  (";
    showPerc1 (os, itsTimeCFfft, duration);
    os << " of total;   #ffts=" << itsTimeCFcnt << ')' << endl;
  }

  void LofarConvolutionFunction::showPerc1 (ostream& os,
                                            double value, double total)
  {
    int perc = (total==0  ?  0 : int(1000. * value / total + 0.5));
    os << std::setw(3) << perc/10 << '.' << perc%10 << '%';
  }


  LofarCFStore LofarConvolutionFunction::makeConvolutionFunctionAterm
  (uInt stationA, uInt stationB, Double timeIn, Double /*w*/,
   const Matrix<bool>& /*Mask_Mueller_in*/, bool degridding_step,
   double Append_average_PB_CF, Matrix<Complex>& Stack_PB_CF,
   double& sum_weight_square, uInt spw, Int /*TaylorTerm*/, double /*RefFreq*/,
   vector< vector < Matrix<Complex> > > & /*StackMuellerNew*/, Int /*ImposeSupport*/)
  {
    // Initialize timers.
    PrecTimer timerFFT;
    PrecTimer timerPar;
    PrecTimer timerCyril;
    timerPar.start();
    Double time(GiveClosestTimeAterm(timeIn));



    Matrix<bool> Mask_Mueller(IPosition(2,4,4),false);
    Mask_Mueller(0,0)=true;

    // Stack_PB_CF should be called Sum_PB_CF (it is a sum, no stack).
    CountedPtr<CFTypeVec> res (new vector< vector< vector < Matrix<Complex> > > >());
    CFTypeVec& result = *res;
    vector< vector< vector < Matrix<Complex> > > > result_non_padded;


    // Stack the convolution function if averagepb.img don't exist
    //Matrix<Complex> Stack_PB_CF_fft(IPosition(2,m_shape(0),m_shape(0)),0.);
    Bool Stack = (Append_average_PB_CF != 0.);

    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_station =
      m_AtermStore_station.find(time);
    AlwaysAssert (aiter_station!=m_AtermStore_station.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm_station = aiter_station->second;




    ///        if(m_AtermStore.find(time)==m_AtermStore.end()){computeAterm(time);}

    // Load the Wterm
    vector< vector < Matrix<Complex> > > Kron_Product;
    Kron_Product.resize(4);
    for (uInt i=0; i<4; ++i) {
      Kron_Product[i].resize(4);
    }

    uInt ch(spw);
    Cube<Complex> aTermA(aterm_station[stationA][ch].copy());
    Cube<Complex> aTermB(aterm_station[stationB][ch].copy());
    Int Npix_out = aTermA.shape()[0];


      // Compute the Mueller matrix considering the Mueller Mask


    Matrix<Complex> plane_product (conj(aTermB.xyPlane(0)) * aTermA.xyPlane(0));
    Matrix<Complex> plane_nosphe(plane_product.copy());

    Matrix<Complex> Spheroid_cut_Apply(zero_padding(Spheroid_cut,plane_product.shape()[0]));
    normalized_fft (Spheroid_cut_Apply, false);
    plane_product*=Spheroid_cut_Apply;
    if (!degridding_step) {plane_product=conj(plane_product);}
    Matrix<Complex> plane_product_paddedf(zero_padding(plane_product,plane_product.shape()[0] * m_oversampling));
    normalized_fft (timerFFT, plane_product_paddedf);
    plane_product_paddedf *= static_cast<Float>(m_oversampling*m_oversampling);
    Kron_Product[0][0].reference(plane_product_paddedf);

    //store(plane_product_paddedf,"try.img");
    //assert(false);

      // Put similarly shaped matrix with zeroes for missing Mueller elements.
    Matrix<Complex> zeroCF(IPosition(2,plane_product_paddedf.shape()[0],plane_product_paddedf.shape()[0]));
    zeroCF=0.;
    for (uInt i=0; i<4; ++i) {
      for (uInt j=0; j<4; ++j) {
	if (! Mask_Mueller(i,j)) {
	  Kron_Product[i][j].reference (zeroCF);
	}
      }
    }
      // Add the conv.func. for this channel to the result.
    result.push_back(Kron_Product);
      //}

    

    if (Stack) {
      //taper(plane_product);
      Matrix<Complex> Spheroid_cut_ImPlane(zero_padding(Spheroid_cut,plane_nosphe.shape()[0]));
      normalized_fft (Spheroid_cut_ImPlane, false);
      plane_nosphe*=conj(plane_nosphe);
      plane_nosphe*=Spheroid_cut_ImPlane;
      normalized_fft (plane_nosphe, true);

      double istart = 0.5 * (m_shape[0] - Npix_out);
      if (istart-floor(istart) != 0.) {
	istart += 0.5; //If number of pixel odd then 0th order at the center, shifted by one otherwise
      }
      for (Int jj=0; jj<Npix_out; ++jj) {
	for (Int ii=0; ii<Npix_out; ++ii) {
	  Complex gain = plane_nosphe(ii,jj);
	  Stack_PB_CF(istart+ii,istart+jj) += gain;//*weight_sqsq;
	}
      }
      sum_weight_square += 1.;//weight_sqsq*weight_sqsq;
    }

    // Put the resulting vec(vec(vec))) in a LofarCFStore object
    CoordinateSystem csys;
    Vector<Float> samp(2, m_oversampling);
    Vector<Int> xsup(1, Npix_out/2);
    Vector<Int> ysup(1, Npix_out/2);
    Int maxXSup(Npix_out);///2);
    Int maxYSup(Npix_out);///2);
    Quantity PA(0., "deg");
    Int mosPointing(0);

    // Update the timing info.
    timerPar.stop();
    double ftime = timerFFT.getReal();
#pragma omp atomic
    itsTimeCFfft += ftime;
    unsigned long long cnt = timerFFT.getCount();
#pragma omp atomic
    itsTimeCFcnt += cnt;
    double ptime = timerPar.getReal();
#pragma omp atomic
    itsTimeCFpar += ptime;


    return LofarCFStore (res, csys, samp,  xsup, ysup, maxXSup, maxYSup,
                         PA, mosPointing, Mask_Mueller);
  }




  /////////////////////////////////////////////////////

  void LofarConvolutionFunction::ConvolveArrayArrayParallel( const Array<Complex>& gridin, Array<Complex>& gridout,
				     const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel)
    {

      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      //      uInt j, ii, jj;
      

      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }

      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	{
#pragma omp parallel for //private(threadNum, ConvPol)
	  for (Int i=0; i<GridSize-Support; ++i) {
	    int threadNum = OpenMP::threadNum();
	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    const Complex* VinPtr= gridin.data() + ConvPol*GridSize*GridSize + (off+i)*GridSize + off;
	    Array<Complex> outGrid(GridsParallel[threadNum]);
	    for (Int j=0; j<GridSize-Support; ++j) {
	      if (VinPtr->real() != 0  || VinPtr->imag() != 0) {
		const Complex* cfPtr = ConvFunc.data();
		for (Int ii=0; ii<Support; ++ii) {
		  Complex* outPtr = outGrid.data() + (i+ii)*GridSize + j +offPol;
		  for (Int jj=0; jj<Support; ++jj) {
		    outPtr[jj] += *cfPtr++ * *VinPtr;//VecinPtr[threadNum];
		  }
		}
	      }
	      VinPtr++;//VecinPtr[threadNum]++;
	    }
	    //VecinPtr[threadNum] += Support;
	  }
	}

      }
      
      SumGridsOMP(gridout, GridsParallel);

    }



  void LofarConvolutionFunction::ConvolveArrayArrayParallel2( const Array<Complex>& gridin, Array<Complex>& gridout,
				     const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      //      uInt j, ii, jj;
      

      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }

      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	{
	  Int i,j;
	  const Complex* VinPtr;
	  Array<Complex> outGrid;
	  const Complex* cfPtr;
	  Complex* outPtr;
#pragma omp parallel for schedule(dynamic) private(i,j,VinPtr,outGrid,cfPtr)
	  for (Int ij=0; ij<(GridSize-Support+1)*(GridSize-Support+1); ++ij) {
	    int threadNum = OpenMP::threadNum();
	    
	    i=floor(float(ij)/float(GridSize-Support+1));
	    j=floor(float(ij)-i*float(GridSize-Support+1));

	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    VinPtr= gridin.data() + offPol + (off+i)*GridSize + off+j;
	    //cout<<ConvPol<<" "<<ij<<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	    if (VinPtr->real() != 0  || VinPtr->imag() != 0) {
	      outGrid.reference(GridsParallel[threadNum]);
	      //cout<<ConvPol<<" "<<threadNum <<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	      cfPtr= ConvFunc.data();
	      for (Int ii=0; ii<Support; ++ii) {
		outPtr = outGrid.data() + offPol + (i+ii)*GridSize +  j ;
		for (Int jj=0; jj<Support; ++jj) {
		  outPtr[jj] += *cfPtr++ * *VinPtr;//VecinPtr[threadNum];
		}
	      }
	    }
	    //VinPtr++;//VecinPtr[threadNum]++;

	    //VecinPtr[threadNum] += Support;
	  }
	}

      }
      
      SumGridsOMP(gridout, GridsParallel);
    }


  void LofarConvolutionFunction::ConvolveArrayArrayParallel2( const Array<Complex>& gridin, Array<Complex>& gridout,
							      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<Bool> MaskIn)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      //      uInt j, ii, jj;
      

      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }

      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	{
	  Int i,j;
	  const Complex* VinPtr;
	  const Bool* MaskVinPtr;
	  Array<Complex> outGrid;
	  const Complex* cfPtr;
	  Complex* outPtr;
#pragma omp parallel for schedule(dynamic) private(i,j,VinPtr,outGrid,cfPtr)
	  for (Int ij=0; ij<(GridSize-Support+1)*(GridSize-Support+1); ++ij) {
	    int threadNum = OpenMP::threadNum();
	    
	    i=floor(float(ij)/float(GridSize-Support+1));
	    j=floor(float(ij)-i*float(GridSize-Support+1));

	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    VinPtr= gridin.data() + offPol + (off+i)*GridSize + off+j;
	    MaskVinPtr= MaskIn.data() + (off+i)*GridSize + off+j;
	    if (*MaskVinPtr) {
	      outGrid.reference(GridsParallel[threadNum]);
	      //cout<<ConvPol<<" "<<threadNum <<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	      cfPtr= ConvFunc.data();
	      for (Int ii=0; ii<Support; ++ii) {
		outPtr = outGrid.data() + offPol + (i+ii)*GridSize +  j ;
		for (Int jj=0; jj<Support; ++jj) {
		  outPtr[jj] += *cfPtr++ * *VinPtr;//VecinPtr[threadNum];
		}
	      }
	    }
	    //VinPtr++;//VecinPtr[threadNum]++;

	    //VecinPtr[threadNum] += Support;
	  }
	}

      }
      
      SumGridsOMP(gridout, GridsParallel);
    }

  void LofarConvolutionFunction::ConvolveArrayArrayParallel3( const Array<Complex>& gridin, Array<Complex>& gridout,
				     const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      //      uInt j, ii, jj;
      

      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }

      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	{
	  Int i,j;
	  Int ii0,ii1;
	  Int jj0,jj1;
	  const Complex* VinPtr;
	  Array<Complex> outGrid;
	  const Complex* cfPtr;
	  Complex* outPtr;
#pragma omp parallel for schedule(dynamic) private(i,j,VinPtr,outGrid,cfPtr,ii0,ii1,jj0,jj1)
	  for (Int ij=0; ij<(GridSize)*(GridSize); ++ij) {
	    int threadNum = OpenMP::threadNum();
	    
	    i=floor(float(ij)/float(GridSize));
	    j=floor(float(ij)-i*float(GridSize));

	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    VinPtr= gridin.data() + offPol + i*GridSize + j;
	    //cout<<ConvPol<<" "<<ij<<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	    if (VinPtr->real() != 0  || VinPtr->imag() != 0) {
	      outGrid.reference(GridsParallel[threadNum]);
	      //cout<<ConvPol<<" "<<threadNum <<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	      ii0=std::min(i,off);
	      ii1=std::min(GridSize-i-1,off);
	      jj0=std::min(j,off);
	      jj1=std::min(GridSize-j-1,off);
	      for (Int ii=-ii0; ii<=ii1; ++ii) {
		outPtr = outGrid.data() + offPol + (i+ii)*GridSize +  j-jj0 ;
		cfPtr= ConvFunc.data()+(off+ii)*Support-jj0+off;
		for (Int jj=0; jj<=jj1+jj0; ++jj) {
		  outPtr[jj] += *cfPtr++ * *VinPtr;
		}
	      }
	    }
	    //VinPtr++;//VecinPtr[threadNum]++;

	    //VecinPtr[threadNum] += Support;
	  }
	}

      }
      
      SumGridsOMP(gridout, GridsParallel);
    }

  void LofarConvolutionFunction::ConvolveArrayArrayParallel3( const Array<Complex>& gridin, Array<Complex>& gridout,
							      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<Bool> MaskIn)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      //      uInt j, ii, jj;
      

      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }

      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	{
	  Int i,j;
	  Int ii0,ii1;
	  Int jj0,jj1;
	  const Complex* VinPtr;
	  const Bool* MaskVinPtr;
	  Array<Complex> outGrid;
	  const Complex* cfPtr;
	  Complex* outPtr;
#pragma omp parallel for schedule(dynamic) private(i,j,VinPtr,outGrid,cfPtr,ii0,ii1,jj0,jj1)
	  for (Int ij=0; ij<(GridSize)*(GridSize); ++ij) {
	    int threadNum = OpenMP::threadNum();
	    
	    i=floor(float(ij)/float(GridSize));
	    j=floor(float(ij)-i*float(GridSize));

	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    VinPtr= gridin.data() + offPol + i*GridSize + j;
	    //cout<<ConvPol<<" "<<ij<<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	    MaskVinPtr= MaskIn.data() + i*GridSize + j;
	    if (*MaskVinPtr) {
	      outGrid.reference(GridsParallel[threadNum]);
	      //cout<<ConvPol<<" "<<threadNum <<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	      ii0=std::min(i,off);
	      ii1=std::min(GridSize-i-1,off);
	      jj0=std::min(j,off);
	      jj1=std::min(GridSize-j-1,off);
	      for (Int ii=-ii0; ii<=ii1; ++ii) {
		outPtr = outGrid.data() + offPol + (i+ii)*GridSize +  j-jj0 ;
		cfPtr= ConvFunc.data()+(off+ii)*Support-jj0+off;
		for (Int jj=0; jj<=jj1+jj0; ++jj) {
		  outPtr[jj] += *cfPtr++ * *VinPtr;
		}
	      }
	    }
	    //VinPtr++;//VecinPtr[threadNum]++;

	    //VecinPtr[threadNum] += Support;
	  }
	}

      }
      
      SumGridsOMP(gridout, GridsParallel);
    }

  void LofarConvolutionFunction::ConvolveArrayArrayParallel4( const Array<Complex>& gridin, Array<Complex>& gridout,
							      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<uShort> MaskIn)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int NChan(gridin.shape()[3]);
      int NPol(gridin.shape()[2]);
      int off(Support/2);
      AlwaysAssert (gridout.shape()[3]==1, AipsError);

      //      uInt j, ii, jj;
      

      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
#pragma omp parallel for schedule(dynamic)
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }

      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){
	//cout<<"  ConvPol="<<ConvPol<<endl;
	for(Int ch=0 ; ch<NChan ; ++ch){

	  uInt offChan = ch*NPol*GridSize*GridSize;

	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	{
	  Int i,j;
	  Int ii0,ii1;
	  Int jj0,jj1;
	  const Complex* VinPtr;
	  Array<Complex> outGrid;
	  const Complex* cfPtr;
	  Complex* outPtr;
#pragma omp parallel for schedule(dynamic) private(i,j,VinPtr,outGrid,cfPtr,ii0,ii1,jj0,jj1)
	  for (uInt ij=0; ij<MaskIn.shape()[0]; ++ij) {
	    int threadNum = OpenMP::threadNum();
	    IPosition pos(2,1,1);
	    i=MaskIn(ij,0);
	    j=MaskIn(ij,1);

	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    VinPtr= gridin.data() + offChan+offPol + i*GridSize + j;
	    //cout<<i<<" "<<j<<" "<<*VinPtr<<endl;
	    //cout<<ConvPol<<" "<<ij<<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	    //MaskVinPtr= MaskIn.data() + i*GridSize + j;
	    //if (*MaskVinPtr) {
	      outGrid.reference(GridsParallel[threadNum]);
	      //cout<<ConvPol<<" "<<threadNum <<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	      ii0=std::min(i,off);
	      ii1=std::min(GridSize-i-1,off);
	      jj0=std::min(j,off);
	      jj1=std::min(GridSize-j-1,off);
	      for (Int ii=-ii0; ii<=ii1; ++ii) {
		outPtr = outGrid.data() + offChan+offPol + (i+ii)*GridSize +  j-jj0 ;
		cfPtr= ConvFunc.data()+(off+ii)*Support-jj0+off;
		for (Int jj=0; jj<=jj1+jj0; ++jj) {
		  outPtr[jj] += *cfPtr++ * *VinPtr;
		}
	      }
	    //}
	    //VinPtr++;//VecinPtr[threadNum]++;

	    //VecinPtr[threadNum] += Support;
	  }
	}
	}
      }
      
      SumGridsOMP(gridout, GridsParallel);
    }

  void LofarConvolutionFunction::ConvolveArrayArrayParallel4( const Array<Complex>& gridin, Array<Complex>& gridout, uInt PolNum,
							      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<uShort> MaskIn)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int NPol(gridin.shape()[2]);
      int NChan(gridin.shape()[3]);
      int off(Support/2);


      //      uInt j, ii, jj;
      

      AlwaysAssert (gridout.shape()[3]==1, AipsError);
      vector< const Complex* > VecinPtr;
      VecinPtr.resize(OpenMP::maxThreads());
      
#pragma omp parallel for schedule(dynamic)
      for (uInt i=0; i<OpenMP::maxThreads(); ++i) {
	GridsParallel[i]=Complex();
      }


	uInt ConvPol(PolNum);
	Int offPol(ConvPol*GridSize*GridSize);
	//const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	// for(ii=0; ii<OpenMP::maxThreads(); ++ii){
	//   VecinPtr[ii] = ;
	// }
	for(Int ch=0 ; ch<NChan ; ++ch){
	{
	  uInt offChan = ch*NPol*GridSize*GridSize;
	  Int i,j;
	  Int ii0,ii1;
	  Int jj0,jj1;
	  const Complex* VinPtr;
	  Array<Complex> outGrid;
	  const Complex* cfPtr;
	  Complex* outPtr;
#pragma omp parallel for schedule(dynamic) private(i,j,VinPtr,outGrid,cfPtr,ii0,ii1,jj0,jj1)
	  for (uInt ij=0; ij<MaskIn.shape()[0]; ++ij) {
	    int threadNum = OpenMP::threadNum();
	    IPosition pos(2,1,1);
	    i=MaskIn(ij,0);
	    j=MaskIn(ij,1);

	    //VecinPtr[threadNum] = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off + i*GridSize;
	    VinPtr= gridin.data() + offChan+ offPol + i*GridSize + j;
	    //cout<<ConvPol<<" "<<i<<" "<<j<<" "<<*VinPtr<<endl;
	    //cout<<ConvPol<<" "<<ij<<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	    //MaskVinPtr= MaskIn.data() + i*GridSize + j;
	    //if (*MaskVinPtr) {
	      outGrid.reference(GridsParallel[threadNum]);
	      //cout<<ConvPol<<" "<<threadNum <<" "<<i<<" "<<j<<" "<<VinPtr->real()<<endl;
	      ii0=std::min(i,off);
	      ii1=std::min(GridSize-i-1,off);
	      jj0=std::min(j,off);
	      jj1=std::min(GridSize-j-1,off);
	      for (Int ii=-ii0; ii<=ii1; ++ii) {
		outPtr = outGrid.data() + offChan+ offPol +(i+ii)*GridSize +  j-jj0 ;
		cfPtr= ConvFunc.data()+(off+ii)*Support-jj0+off;
		for (Int jj=0; jj<=jj1+jj0; ++jj) {
		  outPtr[jj] += *cfPtr++ * *VinPtr;
		}
	      }
	    //}
	    //VinPtr++;//VecinPtr[threadNum]++;

	    //VecinPtr[threadNum] += Support;
	  }
	}
	}
	SumGridsOMP(gridout, GridsParallel,PolNum,PolNum);
    }




    
    void LofarConvolutionFunction::SumGridsOMP(Array<Complex>& grid, const vector< Array<Complex> >& GridToAdd0 ){

    for(uInt vv=0; vv<GridToAdd0.size();vv++){
      Array<Complex> GridToAdd(GridToAdd0[vv]);
      int y,ch,pol;
      int GridSize(grid.shape()[0]);
      int NPol(grid.shape()[2]);
      int NChan(grid.shape()[3]);
      Complex* gridPtr;
      const Complex* GridToAddPtr;
      
#pragma omp parallel for private(y,ch,pol,gridPtr,GridToAddPtr)
      for(int x=0 ; x<grid.shape()[0] ; ++x){
	for(ch=0 ; ch<NChan ; ++ch){
	  for(pol=0 ; pol<NPol ; ++pol){
	    gridPtr = grid.data() + ch*NPol*GridSize*GridSize + pol*GridSize*GridSize+x*GridSize;
	    GridToAddPtr = GridToAdd.data() + ch*NPol*GridSize*GridSize + pol*GridSize*GridSize+x*GridSize;
	    for(y=0 ; y<grid.shape()[1] ; ++y){
	      (*gridPtr++) += *GridToAddPtr++;
	      //gridPtr++;
	      //GridToAddPtr++;
	    }
	  }
	}
      }
    }

    }

  void LofarConvolutionFunction::SumGridsOMP(Array<Complex>& grid, const vector< Array<Complex> > & GridToAdd0 , uInt PolNumIn, uInt PolNumOut){
  
    for(uInt vv=0; vv<GridToAdd0.size();vv++){
      Array<Complex> GridToAdd(GridToAdd0[vv]);
      int y,ch,polIn,polOut;
      int GridSize(grid.shape()[0]);
      int NPol(grid.shape()[2]);
      int NChan(grid.shape()[3]);
      Complex* gridPtr;
      const Complex* GridToAddPtr;
      //cout<<"SumGridsOMP "<<PolNum<<endl;
      //cout<< "gridout: "<<grid.shape()<<" , gridin: "<<GridToAdd.shape()<<endl;

#pragma omp parallel for private(y,ch,polIn,polOut,gridPtr,GridToAddPtr)
      for(int x=0 ; x<grid.shape()[0] ; ++x){
	polIn=PolNumIn;
	polOut=PolNumOut;
	for(ch=0 ; ch<NChan ; ++ch){
	    
	  gridPtr = grid.data() + ch*NPol*GridSize*GridSize + polOut*GridSize*GridSize+x*GridSize;
	  GridToAddPtr = GridToAdd.data() + ch*NPol*GridSize*GridSize + polIn*GridSize*GridSize+x*GridSize;
	  
	  for(y=0 ; y<grid.shape()[1] ; ++y){
	    //if((*GridToAddPtr)!=Complex(0.)){cout<<"yo "<<ch<<" "<<x<<" "<<y<<" "<<*GridToAddPtr<<endl;}
	    //cout<<"yo "<<pol<<" "<<x<<" "<<y<<" "<<*GridToAddPtr<<endl;
	    (*gridPtr++) += *GridToAddPtr++;
	    //gridPtr++;
	    //GridToAddPtr++;
	  }
	}
      }
    }
    
  }

  void LofarConvolutionFunction::SumGridsOMP(Array<Complex>& grid, const Array<Complex> & GridToAdd , uInt PolNumIn, uInt PolNumOut){
  
    int y,ch,polIn,polOut;
      int GridSize(grid.shape()[0]);
      int NPol(grid.shape()[2]);
      int NChan(grid.shape()[3]);
      Complex* gridPtr;
      const Complex* GridToAddPtr;
      //cout<<"SumGridsOMP novec"<<PolNum<<endl;
      //cout<< "gridout: "<<grid.shape()<<" , gridin: "<<GridToAdd.shape()<<endl;


#pragma omp parallel for private(y,ch,polIn,polOut,gridPtr,GridToAddPtr)
      for(int x=0 ; x<grid.shape()[0] ; ++x){
	polIn=PolNumIn;
	polOut=PolNumOut;
	for(ch=0 ; ch<NChan ; ++ch){
	    
	  gridPtr = grid.data() + ch*NPol*GridSize*GridSize + polOut*GridSize*GridSize+x*GridSize;
	  GridToAddPtr = GridToAdd.data() + ch*NPol*GridSize*GridSize + polIn*GridSize*GridSize+x*GridSize;
	  for(y=0 ; y<grid.shape()[1] ; ++y){
	    //if((*GridToAddPtr)!=Complex(0.)){cout<<"yo "<<pol<<" "<<x<<" "<<y<<" "<<*GridToAddPtr<<endl;}
	    //cout<<"yo2 "<<pol<<" "<<x<<" "<<y<<" "<<*GridToAddPtr<<endl;
	    (*gridPtr++) += *GridToAddPtr++;
	    //gridPtr++;
	    //GridToAddPtr++;
	  }
	}
      }
    
  }



  //////////////////////////////////////////////////////////

  Array<Complex> LofarConvolutionFunction::ApplyElementBeam2(Array<Complex>& input_grid, Double time, uInt spw, const Matrix<bool>& Mask_Mueller_in2, bool degridding_step, Int UsedMask)
  {

    Matrix<bool> Mask_Mueller_in(Mask_Mueller_in2.copy());
    for(uInt i=0;i<4;++i){
      for(uInt j=0;j<4;++j){
    	Mask_Mueller_in(i,j)=true;
      }
    }
    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_element = m_AtermStore_element.find(time);
    AlwaysAssert (aiter_element!=m_AtermStore_element.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm_element = aiter_element->second;



    vector< vector< IPosition > > Mueller_Coordinates;
    Mueller_Coordinates.resize(4);
    for(uInt i=0;i<4;++i){
      Mueller_Coordinates[i].resize(4);
      IPosition pos(2,2,1);
      for(uInt j=0;j<4;++j){
	Mueller_Coordinates[i][j]=pos;
      }
    }

    {
    uInt ind0;
    uInt ind1;
    uInt ii = 0;
    IPosition cfShape;
    for (uInt row0=0; row0<=1; ++row0) {
      for (uInt col0=0; col0<=1; ++col0) {
	vector < Matrix<Complex> > Row(4);
	vector < Matrix<Complex> > Row_non_padded(4);
	uInt jj = 0;
	for (uInt row1=0; row1<=1; ++row1) {
	  for (uInt col1=0; col1<=1; ++col1) {
	    // This Mueller ordering is for polarisation given as XX,XY,YX YY
	    ind0 = row0 + 2*row1;
	    ind1 = col0 + 2*col1;
	    //ind0 = 2.*row0 + row1;
	    //ind1 = 2.*col0 + col1;
	    IPosition pos(2,2,1);
	    pos[0]=ind0;
	    pos[1]=ind1;
	    Mueller_Coordinates[ii][jj]=pos;
	    ++jj;
	  }
	}
	++ii;
      }
    }
    }

    if (!degridding_step) {
      for (uInt i=0; i<4; ++i) {
    	for (uInt j=i; j<4; ++j) {
    	  IPosition pos_tmp(Mueller_Coordinates[i][j]);
    	  Mueller_Coordinates[i][j]=Mueller_Coordinates[j][i];
    	  Mueller_Coordinates[j][i]=pos_tmp;
    	  Bool bool_tmp(Mask_Mueller_in(i,j));
    	  Mask_Mueller_in(i,j)=Mask_Mueller_in(j,i);
    	  Mask_Mueller_in(i,j)=bool_tmp;
    	}
      }
    }

    Cube<Complex> aTermA(aterm_element[0][spw].copy());
    Array<Complex> grid_out(input_grid.shape(),0.);
    Int nx(input_grid.shape()[0]);
    Int npol(input_grid.shape()[2]);

    vector< vector< Matrix<Complex> > > vec_plane_product;
    vec_plane_product.resize(4);


    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Calculate element beams"<< LogIO::POST;//<<endl;
    Matrix<Complex> Spheroid_AtermOrig(zero_padding(Spheroid_cut,aTermA.xyPlane(0).shape()[0]));
    normalized_fft (Spheroid_AtermOrig, false);
    for(uInt ii=0;ii<4;++ii){
      vec_plane_product[ii].resize(4);
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  vec_plane_product[ii][jj].resize(aTermA.xyPlane(0).shape());
	  vec_plane_product[ii][jj]=aTermA.xyPlane((Mueller_Coordinates[ii][jj])[0]) * conj(aTermA.xyPlane((Mueller_Coordinates[ii][jj])[1]));
	  //taper(vec_plane_product[ii][jj]);
	  vec_plane_product[ii][jj]*=Spheroid_AtermOrig;
	  if(!degridding_step){vec_plane_product[ii][jj]=conj(vec_plane_product[ii][jj]);};
	  //store(vec_plane_product[ii][jj],"Im_AH"+String::toString(ii)+"-"+String::toString(jj)+".img");
	  normalized_fft(vec_plane_product[ii][jj],true);
	}
      }
    }

    //assert(false);
    //#pragma omp parallel
    if(GridsMueller.size()==0){
      //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"...Declare GridsMueller Matrix"<< LogIO::POST;//<<endl;
      GridsMueller.resize(4);
      for(uInt ii=0;ii<4;++ii){
	GridsMueller[ii].resize(4);
	for(uInt jj=0;jj<4;++jj){
	  if(Mask_Mueller_in(ii,jj)==true){
	    GridsMueller[ii][jj].resize(IPosition(2,nx,nx));
	    GridsMueller[ii][jj]=Complex();
	  }
	}
      }
    } else {
      for(uInt ii=0;ii<4;++ii){
    	for(uInt jj=0;jj<4;++jj){
    	  if(Mask_Mueller_in(ii,jj)==true){
    	    GridsMueller[ii][jj]=Complex();
    	  }
    	}
      }
    }

    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Convolve..."<< LogIO::POST;//<<endl;
    {
      Int ii;
      Int jj;
#pragma omp parallel for private(ii,jj)
      for(uInt iii=0;iii<16;++iii){
	ii=floor(float(iii)/4.);
	jj=floor((float(iii)/4.-ii)*4.);
	//cout<<"iii"<<iii<<" "<<ii<<" "<<jj<<" M="<<Mask_Mueller_in(jj,ii)<<endl;
	if(Mask_Mueller_in(ii,jj)==true){
	  Matrix<Complex> ConvFunc(vec_plane_product[ii][jj]);

	  //ConvolveGerArray(input_grid, ii, GridsMueller[ii][jj], ConvFunc);

	  if(npol==1){
	    if(!(UsedMask>-1)){
	      ConvolveGerArray(input_grid, 0, GridsMueller[ii][jj], ConvFunc);
	    } else {
	      ConvolveGerArrayMask(input_grid, 0, GridsMueller[ii][jj], ConvFunc, UsedMask);
	    }
	  }
	  if(npol==4){
	    if(!(UsedMask>-1)){
	      ConvolveGerArray(input_grid, ii, GridsMueller[ii][jj], ConvFunc);
	    } else {
	      ConvolveGerArrayMask(input_grid, ii, GridsMueller[ii][jj], ConvFunc, UsedMask);
	    }
	  }


	}

      }
    }



    //    #pragma omp parallel
    if(npol==4)
      {
	int y=0;
	uInt ii=0;
	uInt jj=0;
	#pragma omp parallel for private(y,ii,jj)
	for(int x=0 ; x<nx ; ++x){
	  //cout<<"x="<<x<<endl;
	  for(y=0 ; y<nx ; ++y){

	    for(ii=0;ii<4;++ii){
	      for(jj=0;jj<4;++jj){
		//if(Mask_Mueller_in(ii,jj)==true){
		  grid_out(IPosition(4,x,y,jj,0)) += GridsMueller[ii][jj](x,y) ;///Spheroid_cut_im_element(x,y);

		//}
	      }
	    }
	  }
	}
      }


    if(npol==1)
      {
    	int y=0;
    	uInt ii=0;
    	#pragma omp parallel for private(y,ii)
    	for(int x=0 ; x<nx ; ++x){
    	  for(y=0 ; y<nx ; ++y){
    	    for(ii=0;ii<4;++ii){
    	      grid_out(IPosition(4,x,y,0,0)) += 0.5*(GridsMueller[0][ii](x,y) + GridsMueller[3][ii](x,y));///Spheroid_cut_im_element(x,y);

    	    }
    	  }
    	}
      }



    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Shapes InputGrid:"<<input_grid.shape()<<", Shapes OutputGrid:"<< LogIO::POST;//<<grid_out.shape()<<endl;


    return grid_out;

  }




} //# end namespace casa
