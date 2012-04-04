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

// SvdT: included for debugging purposes
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TiledColumnStMan.h>

// SvdT: to get rid of warnings for unused variables
#define UNUSED(expr) (void)(expr);

#define interpolation_kernel_width 2

namespace LOFAR
{
  
  void interpolate(const Matrix<Complex> &m1, Matrix<Complex> &m2);
  void interpolate(const Matrix<Float> &m1, Matrix<Float> &m2);
  Matrix<Complex> find_support( const Matrix<Complex> m );

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
   const casa::Record& parameters
  )
  // ,
  //Int TaylorTerm,
    //Double RefFreq
    : m_shape(shape),
      m_coordinates(coordinates),
      itsParameters(parameters),
      m_aTerm(ms, parameters),
      m_maxW(Wmax), //maximum W set by ft machine to flag the w>wmax
      m_nWPlanes(nW),
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

    //    m_maxCFSupport=0; //need this parameter to stack all the CF for average PB estimate

    //itsTaylorTerm=TaylorTerm;
    //itsRefFreq=RefFreq;
    //cout<<"itsTaylorTerm itsRefFreq "<<itsTaylorTerm<<" "<<itsRefFreq<<endl;
      m_wScale = WScale(m_maxW, m_nWPlanes);
    MEpoch start = observationStartTime(ms, 0);

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
    list_freq.resize(window.nrow());
    for(uInt i=0; i<window.nrow();++i){
      list_freq[i]=window.refFrequency()(i);
    };

    m_nChannel  = list_freq.size();
    ROMSAntennaColumns antenna(ms.antenna());
    m_nStations = antenna.nrow();

    m_pixelSizeSpheroidal = makeSpheroidCut();
    //Double PixelSize=abs(m_coordinates.increment()(0));
    //Double ImageDiameter=PixelSize * m_shape(0);
    //Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, m_maxW));
    //m_maxCFSupport= ImageDiameter / W_Pixel_Ang_Size;
    //Matrix<Complex> Stack_pb_cf0(IPosition(2,m_maxCFSupport,m_maxCFSupport),0.);
    Matrix<Complex> Stack_pb_cf0(IPosition(2,m_shape(0),m_shape(0)),0.);
    Matrix<float> Stack_pb_cf1(IPosition(2,m_shape(0),m_shape(0)),0.);

    //Stack_pb_cf0(256,300)=1.;
    //Matrix<Complex> Avg_PB_padded00(give_normalized_fft(Stack_pb_cf0,false));
    //store(Avg_PB_padded00,"Avg_PB_padded00.img");

    // Precalculate the Wtwerm fft for all w-planes.
    store_all_W_images();
    itsFilledVectorMasks=false;

    // Build the cutted spheroidal for the element beam image
    Double pixelSize = abs(m_coordinates.increment()[0]);
    Double imageDiameter = pixelSize * m_shape(0);
    DirectionCoordinate coordinate = m_coordinates;
    Double aPixelAngSize = estimateAResolution(m_shape, m_coordinates);
    Int nPixelsConv = imageDiameter / aPixelAngSize;

    itsSumPB = Array<Float>(IPosition(4, nPixelsConv + 2*interpolation_kernel_width, nPixelsConv + 2*interpolation_kernel_width,4,m_nChannel), 0.0);
    double itsSumWeight = 0.0;
    
    aPixelAngSize = min(m_pixelSizeSpheroidal,aPixelAngSize);
    
    Matrix<Complex> spheroid_cut_element(IPosition(2,nPixelsConv,nPixelsConv),1.);
    taper(spheroid_cut_element);
    //Matrix<Complex> spheroid_cut_element_fft=give_normalized_fft_lapack(spheroid_cut_element, true);
    normalized_fft(spheroid_cut_element, true);
    spheroid_cut_element_fft=spheroid_cut_element;
    Matrix<Complex> spheroid_cut_element_padfft(zero_padding(spheroid_cut_element_fft, m_shape(0)));
    //Matrix<Complex> spheroid_cut_element_padfft_fft=give_normalized_fft_lapack (spheroid_cut_element_padfft, false);
    normalized_fft (spheroid_cut_element_padfft, false);
    Matrix<Complex> spheroid_cut_element_padfft_fft(spheroid_cut_element_padfft);
    float threshold = 1.e-6;
    for (Int jj=0; jj<m_shape[1]; ++jj) {
      for (Int ii=0; ii<m_shape[0]; ++ii) {
	Float absVal = abs(spheroid_cut_element_padfft_fft(ii,jj));
	spheroid_cut_element_padfft_fft(ii,jj) = std::max (absVal, threshold);
      }
    }
    Spheroid_cut_im_element.reference (real(spheroid_cut_element_padfft_fft));
    store(m_coordinates,Spheroid_cut_im_element,"Spheroid_cut_im_element.img");

    // SvdT: for debugging puprposes
    TableDesc td;
    td.comment() = "logging table for debug info";
    td.addColumn (ScalarColumnDesc<Int> ("ANTENNA1"));
    td.addColumn (ScalarColumnDesc<Int> ("ANTENNA2"));
    td.addColumn (ArrayColumnDesc<Complex> ("DATA",0));
    
    SetupNewTable newtab("loggingtable.data", td, Table::New);
    itsLoggingTable = Table(newtab);

  }

  //      ~LofarConvolutionFunction ()
  //      {
  //      }
  

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
#pragma omp parallel
    {
      // Thread private variables.
      PrecTimer timerFFT;
      PrecTimer timerPar;
#pragma omp for schedule(dynamic)
      for (uInt i=0; i<m_nWPlanes; ++i) {
        timerPar.start();
        Double w = m_wScale.center(i);
        Double wPixelAngSize = min(m_pixelSizeSpheroidal,
                                   estimateWResolution(m_shape,
                                                       pixelSize, w));
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
        double wavelength(C::c / list_freq[0]);
        Matrix<Complex> wTerm = m_wTerm.evaluate(shape, increment,
                                                 w/wavelength);
        normalized_fft(timerFFT, wTerm);
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

      ///#pragma omp for
      DirectionCoordinate coordinate = m_coordinates;
      Double aPixelAngSize = estimateAResolution(m_shape, m_coordinates);
      Int nPixelsConv = imageDiameter / aPixelAngSize;
      if (nPixelsConv > itsMaxSupport) {
	nPixelsConv = itsMaxSupport;
      }
      // Make odd and optimal.
//      nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
      aPixelAngSize = imageDiameter / nPixelsConv;
      IPosition shape(2, nPixelsConv + 2*interpolation_kernel_width, nPixelsConv + 2*interpolation_kernel_width);
      Vector<Double> increment_old(coordinate.increment());
      Vector<Double> increment(2);
      increment[0] = aPixelAngSize*sign(increment_old[0]);
      increment[1] = aPixelAngSize*sign(increment_old[1]);
      coordinate.setIncrement(increment);
      Vector<Double> refpix(2, 0.5*(nPixelsConv-1) + interpolation_kernel_width);
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
      
      m_aTerm.setDirection(coordinate, shape);
      
      MEpoch binEpoch;
      binEpoch.set(Quantity(time, "s"));
      
      m_aTerm.setEpoch(binEpoch);

      for (uInt i=0; i<m_nStations; ++i) {
        timerPar.start();

        //======================================
        // Separated element and station
        //======================================
        vector< Cube<Complex> > aTermA_element;
        vector< Cube<Complex> > aTermA_array;

        vector< Matrix<Complex> > aTermA_array_plane(m_aTerm.evaluateStationScalarFactor(i, list_freq , list_freq , true));
        aTermA_array.resize(m_nChannel);
        for (uInt ch=0; ch<m_nChannel; ++ch) 
        {
          aTermA_array[ch].resize(IPosition(3,shape[0],shape[0],4));
          aTermA_array[ch]=0.;
        }
        for (uInt ch=0; ch<m_nChannel; ++ch) 
        {
          Matrix<Complex> plane(aTermA_array[ch].xyPlane(0));
          plane=aTermA_array_plane[ch].copy();
          Matrix<Complex> plane2(aTermA_array[ch].xyPlane(3));
          plane2=aTermA_array_plane[ch].copy();
        }

        aTermA_element=m_aTerm.evaluateElementResponse(i, 0, list_freq, true);

        // Compute the fft on the beam
//         for (uInt ch=0; ch<m_nChannel; ++ch) {
//           for (uInt pol=0; pol<4; ++pol) {
//             Matrix<Complex> plane1 (aTermA_array[ch].xyPlane(pol));
//             normalized_fft (timerFFT, plane1);
//           }
//         }
        // store(coordinate,aTermA_array[0],"aTermA_array.fft."+String::toString(i)+".img");

        // Note that push_back uses the copy constructor, so for the Cubes
        // in the vector the copy constructor is called too (which is cheap).
        //aTermList[i] = aTermA;
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
  }


  Array<Complex> LofarConvolutionFunction::ApplyElementBeam(Array<Complex> input_grid, Double time, uInt spw, const Matrix<bool>& Mask_Mueller_in, bool degridding_step)
  {

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


    uInt ind0;
    uInt ind1;
    uInt ii = 0;
    IPosition cfShape;
    Bool allElem = True;
    UNUSED(allElem);
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



    vector< vector< Matrix<Complex> > > vec_element_product;
    vec_element_product.resize(4);
    vector< vector< Matrix<Complex> > > vec_plane_product;
    vec_plane_product.resize(4);
    if (!degridding_step) {
      for (uInt i=0; i<4; ++i) {
    	for (uInt j=i; j<4; ++j) {
	  IPosition pos_tmp(Mueller_Coordinates[i][j]);
	  Mueller_Coordinates[i][j]=Mueller_Coordinates[j][i];
	  Mueller_Coordinates[j][i]=pos_tmp;
        }
      }
    }

    uInt nx(input_grid.shape()[0]);
    uInt ny(input_grid.shape()[1]);
    UNUSED(ny);
    uInt npol(input_grid.shape()[2]);

    Cube<Complex> aTermA(aterm_element[0][spw].copy());
    Array<Complex> grid_out(input_grid.shape(),0.);


    if(!degridding_step){

      logIO() <<"LofarConvolutionFunction::ApplyElementBeam "<<"FFT of the gridded data for this timeslot" << LogIO::POST;//<<endl;

      for(uInt channel=0;channel< input_grid.shape()[3];++channel){
	{
#pragma omp parallel for
	for(uInt jj=0;jj<npol;++jj){
	  //cout<<"jj="<<jj<<endl;
	  Matrix<Complex> plane_array_in  = input_grid(Slicer(IPosition(4, 0, 0, jj, 0),
							      IPosition(4, nx, nx, 1, 1))).nonDegenerate();
	  //store(plane_array_in,"plane_array_in"+String::toString(jj)+".img");
	  normalized_fft (plane_array_in, false);
	  //store(plane_array_in,"plane_array_in"+String::toString(jj)+".img");
	}
	}
      }
    }



    //cout<<"LofarConvolutionFunction::ApplyElementBeam "<<"Calculate element beams"<<endl;
    logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Calculate element beams"<< LogIO::POST;//<<endl;
    for(uInt ii=0;ii<4;++ii){
      vec_element_product[ii].resize(4);
      vec_plane_product[ii].resize(4);
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  vec_element_product[ii][jj].resize(IPosition(2, nx, nx));
	  vec_plane_product[ii][jj].resize(aTermA.xyPlane(0).shape());
	  vec_plane_product[ii][jj]=aTermA.xyPlane((Mueller_Coordinates[ii][jj])[0]) * aTermA.xyPlane((Mueller_Coordinates[ii][jj])[1]);
	  taper(vec_plane_product[ii][jj]);
	  if(!degridding_step){vec_plane_product[ii][jj]=conj(vec_plane_product[ii][jj]);};
	}
	//store(vec_plane_product[ii][jj],"vec_plane_product"+String::toString(ii)+String::toString(jj)+".img");
      }
    }


    logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"FFT - Zero Pad - IFFT"<< LogIO::POST;//<<endl;
    //#pragma omp parallel
    {
      Int ii;
      Int jj;
#pragma omp parallel for private(ii,jj)
      for(uInt iii=0;iii<16;++iii){
	jj=floor(float(iii)/4.);
	ii=floor((float(iii)/4.-jj)*4.);
	//cout<<"iii"<<iii<<" "<<ii<<" "<<jj<<endl;
	if(Mask_Mueller_in(ii,jj)==true){
	  normalized_fft (vec_plane_product[ii][jj], true);
	  vec_element_product[ii][jj]=zero_padding(vec_plane_product[ii][jj], nx);
	  normalized_fft (vec_element_product[ii][jj], false);
	  //store(vec_element_product[ii][jj],"vec_element_product"+String::toString(ii)+String::toString(jj)+".img");
	}

      }
    }

    //    assert(false);


    //#pragma omp parallel
    if(npol==4)
      {
	logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Multiply element and data in the image plane"<< LogIO::POST;//<<endl;
	int y=0;
	uInt ii=0;
	uInt jj=0;
#pragma omp parallel for private(y,ii,jj)
	for(int x=0 ; x<nx ; ++x){
	  //cout<<"x="<<x<<endl;
	  for(y=0 ; y<nx ; ++y){

	    for(ii=0;ii<4;++ii){
	      for(jj=0;jj<4;++jj){
		if(Mask_Mueller_in(ii,jj)==true){
		  grid_out(IPosition(4,x,y,jj,0)) += vec_element_product[ii][jj](x,y) * input_grid(IPosition(4,x,y,ii,0))/Spheroid_cut_im_element(x,y);

		}
	      }
	    }
	  }
	}
      }

    if(npol==1)
      {
	logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Multiply element and data in the image plane"<< LogIO::POST;//<<endl;
	int y=0;
	uInt ii=0;
	uInt jj=0;
#pragma omp parallel for private(y,ii,jj)
	for(int x=0 ; x<nx ; ++x){
	  //cout<<"x="<<x<<endl;
	  for(y=0 ; y<nx ; ++y){

	    for(ii=0;ii<4;++ii){
	      for(jj=0;jj<4;++jj){
		if(Mask_Mueller_in(ii,jj)==true){
		  grid_out(IPosition(4,x,y,0,0)) += vec_element_product[ii][jj](x,y) * input_grid(IPosition(4,x,y,0,0))/(2.*Spheroid_cut_im_element(x,y));
		}
	      }
	    }
	  }
	}

      }

    logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Shapes InputGrid:"<<input_grid.shape()<<", Shapes OutputGrid:"<< LogIO::POST;//<<grid_out.shape()<<endl;


    return grid_out;

  }

  //==================================================================
  //==================================================================
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
    Bool allElem = True;
    UNUSED(allElem);
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
    Int ny(input_grid.shape()[1]);
    UNUSED(ny);
    Int npol(input_grid.shape()[2]);

    vector< vector< Matrix<Complex> > > vec_plane_product;
    vec_plane_product.resize(4);


    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Calculate element beams"<< LogIO::POST;//<<endl;
    for(uInt ii=0;ii<4;++ii){
      vec_plane_product[ii].resize(4);
      for(uInt jj=0;jj<4;++jj){
	if(Mask_Mueller_in(ii,jj)==true){
	  vec_plane_product[ii][jj].resize(aTermA.xyPlane(0).shape());
	  vec_plane_product[ii][jj]=aTermA.xyPlane((Mueller_Coordinates[ii][jj])[0]) * conj(aTermA.xyPlane((Mueller_Coordinates[ii][jj])[1]));
	  taper(vec_plane_product[ii][jj]);
	  if(!degridding_step){vec_plane_product[ii][jj]=conj(vec_plane_product[ii][jj]);};
	  //store(vec_plane_product[ii][jj],"vec_plane_product."+String::toString(ii)+"."+String::toString(jj)+".img");
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

    //logIO()<<"LofarConvolutionFunction::ApplyElementBeam "<<"Convolve ... Done!"<< LogIO::POST;//<<endl;
    // Int ii;
    // Int jj;
    // for(uInt iii=0;iii<16;++iii){
    //   jj=floor(float(iii)/4.);
    //   ii=floor((float(iii)/4.-jj)*4.);
    //   //cout<<"iii"<<iii<<" "<<ii<<" "<<jj<<endl;
    //   if(Mask_Mueller_in(ii,jj)==true){
    // 	store(GridsMueller[ii][jj],"grid_out"+String::toString(ii)+String::toString(jj)+".img");
    //   }
    // }





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

  //==================================================================
  //==================================================================



  //================================================
  // Compute the convolution function for all channel, for the polarisations specified in the Mueller_mask matrix
  // Also specify weither to compute the Mueller matrix for the forward or the backward step. A dirty way to calculate
  // the average beam has been implemented, by specifying the beam correcping to the given baseline and timeslot.
  // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]

  LofarCFStore LofarConvolutionFunction::makeConvolutionFunction
  (uInt stationA, uInt stationB, Double time, Double w,
   const Matrix<bool>& /*Mask_Mueller_in*/, bool degridding_step,
   double sum_weight, uInt spw, Int /*TaylorTerm*/, double /*RefFreq*/)
  {
    // Initialize timers.
    PrecTimer timerFFT;
    PrecTimer timerPar;
    PrecTimer timerCyril;
    timerPar.start();

    Matrix<bool> Mask_Mueller(IPosition(2,4,4),false);
    Mask_Mueller(0,0)=true;

    // Stack_PB_CF should be called Sum_PB_CF (it is a sum, no stack).
    CountedPtr<CFTypeVec> res (new vector< vector< vector < Matrix<Complex> > > >());
    CFTypeVec& result = *res;
    vector< vector< vector < Matrix<Complex> > > > result_non_padded;


    // Stack the convolution function if averagepb.img don't exist
    //Matrix<Complex> Stack_PB_CF_fft(IPosition(2,m_shape(0),m_shape(0)),0.);
    Bool Stack = (sum_weight != 0.);

    // If the beam is not in memory, compute it

    // map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter =
    //   m_AtermStore.find(time);
    // AlwaysAssert (aiter!=m_AtermStore.end(), AipsError);
    // const vector< vector< Cube<Complex> > >& aterm = aiter->second;

    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter_station =
      m_AtermStore_station.find(time);
    AlwaysAssert (aiter_station!=m_AtermStore_station.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm_station = aiter_station->second;


    ///        if(m_AtermStore.find(time)==m_AtermStore.end()){computeAterm(time);}

    // Load the Wterm
    double ratio_freqs=list_freq[0]/list_freq[spw];
    uInt w_index = m_wScale.plane(w*ratio_freqs);
    Matrix<Complex> wTerm;
    wTerm = m_WplanesStore[w_index];
    Int Npix_out = 0;
    Int Npix_out2 = 0;

    // Matrix<Complex> Term_test(IPosition(2,101,101),1.);
    // normalized_fft(Term_test);
    // store (Term_test,"Term_test.img");
    // normalized_fft(Term_test,false);
    // store (Term_test,"Term_test_0.img");
    // normalized_fft(Term_test);
    // store (Term_test,"Term_test_1.img");
    // assert(false);

    if (w > 0.) {
      wTerm.reference (conj(wTerm));
    }

    uInt ch(spw);
    //for (uInt ch=0; ch<m_nChannel; ++ch) {
      // Load the Aterm
    //const Cube<Complex> aTermA(aterm_station[stationA][ch].copy());
    Cube<Complex> aTermA(aterm_station[stationA][ch].copy());


    //const Cube<Complex>& aTermB(aterm_station[stationB][ch]);
    Cube<Complex> aTermB(aterm_station[stationB][ch].copy());
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
    Npix_out = std::max(std::max(aTermA.shape()[0]-2*interpolation_kernel_width, aTermB.shape()[0]-2*interpolation_kernel_width),
                        std::max(wTerm.shape()[0], Spheroid_cut.shape()[0]));

    //cout << "CF Shapes, Wterm:" << wTerm.shape()[0] << ", Beam " << aTermA.shape()[0] << ", Spheroid: " << Spheroid_cut.shape()[0] << endl;

    // Zero pad to make the image planes of the A1, A2, and W term have the same resolution in the image plane
    Matrix<Complex> Spheroid_cut_paddedf(zero_padding(Spheroid_cut,Npix_out));
    Matrix<Complex> wTerm_paddedf(zero_padding(wTerm, Npix_out));
    
    Cube<Complex> aTermA_padded(IPosition(3, Npix_out, Npix_out, aTermA.shape()(2)), 0.0);
    Cube<Complex> aTermB_padded(IPosition(3, Npix_out, Npix_out, aTermB.shape()(2)), 0.0);

    // FFT (backward) the A and W terms
    normalized_fft (timerFFT, wTerm_paddedf, false);
    normalized_fft (timerFFT, Spheroid_cut_paddedf, false);
    if (itsVerbose > 0) {
      cout << "fft shapes " << wTerm_paddedf.shape() << ' ' << Spheroid_cut_paddedf.shape()
            << ' ' << aTermA_padded.shape() << ' ' << aTermB_padded.shape() << endl;
    }

//    The interpolation has been commented out, instead we interpolate after the multiplication of aTermA and aTermB
//       for (uInt i=0; i<4; ++i) {
//         // Make a matrix referencing the data in the cube's plane.
//         Matrix<Complex> planeA(aTermA_padded.xyPlane(i));
//         Matrix<Complex> planeB(aTermB_padded.xyPlane(i));
//         interpolate (aTermA.xyPlane(i), planeA);
//         interpolate (aTermB.xyPlane(i), planeB);
//       }

    // Create the vectors of Matrices giving the convolution functions
    // for each Mueller element.
    vector< vector < Matrix<Complex> > > Kron_Product;
    Kron_Product.reserve(4);

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

//                 Matrix<Complex> plane_product (conj(aTermB_padded.xyPlane(ind0)) *
//                                                aTermA_padded.xyPlane(ind1));

              Matrix<Complex> plane_product0 (conj(aTermB.xyPlane(ind0)) *
                                              aTermA.xyPlane(ind1));
              if (Stack) {
                #pragma omp critical (protect_itsSumPB)
                {
                  itsSumPB[ch][ii] = itsSumPB[ch][ii] + (real(plane_product0)*real(plane_product0) + imag(plane_product0)*imag(plane_product0))*sum_weight;
                  itsSumWeight += sum_weight;
                }
              }
              
              Matrix<Complex> plane_product(IPosition(2, Npix_out, Npix_out), 0.0);
              interpolate(plane_product0, plane_product);
              plane_product *= wTerm_paddedf;
              plane_product *= Spheroid_cut_paddedf;


              Matrix<Complex> plane_product_paddedf
                (zero_padding(plane_product,
                              plane_product.shape()[0] * m_oversampling));

              normalized_fft (timerFFT, plane_product_paddedf);
//               Matrix<Complex> plane_clipped = find_support(plane_product_paddedf);
//               plane_product_paddedf.reference( plane_clipped);

              // write debug info to logging table
              // ========================================
//               #pragma omp critical 
//               { 
// //                   if ((stationA == 0) and (stationB ==1)) {
//                   ScalarColumn<Int> antenna1(itsLoggingTable, "ANTENNA1");
//                   ScalarColumn<Int> antenna2(itsLoggingTable, "ANTENNA2");
//                   ArrayColumn<Complex> data(itsLoggingTable, "DATA");
//                   itsLoggingTable.addRow();
//                   antenna1.put(itsLoggingTable.nrow()-1, stationA);
//                   antenna2.put(itsLoggingTable.nrow()-1, stationB);
// //                     cout << plane_product.shape() << endl;
//                   data.put(itsLoggingTable.nrow()-1, plane_product_paddedf);
// //                   }
//               }
              // =======================================
              
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
            } else {
              allElem = False;
            }
            ++jj;
          }
        }
        ++ii;
        Kron_Product.push_back(Row);
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
    result.push_back(Kron_Product);


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
  Cube<Float> LofarConvolutionFunction::Give_avg_pb()
  {
//     // Only read if not available.
//     if (Im_Stack_PB_CF0.empty()) {
//       if (itsVerbose > 0) {
//         cout<<"==============Give_avg_pb()"<<endl;
//       }
//       String PBFile_name(itsImgName + ".avgpb");
//       File PBFile(PBFile_name);
//       if (! PBFile.exists()) {
//         throw SynthesisError (PBFile_name + " not found");
//       }
//       if (itsVerbose > 0) {
//         cout<<"..... loading Primary Beam image from disk ....."<<endl;
//       }
//       PagedImage<Float> tmp(PBFile_name);
//       IPosition shape(tmp.shape());
//       AlwaysAssert (shape[0]==m_shape[0] && shape[1]==m_shape[1], AipsError);
//       tmp.get (Im_Stack_PB_CF0, True);   // remove degenerate axes.
//     }
    return itsSumPB[0][0];
  }

  // Compute the average Primary Beam from the Stack of convolution functions
  Array<Float> LofarConvolutionFunction::Compute_avg_pb (const IPosition shape)
  {
//     // Only calculate if not done yet.
//     if (itsAvgPB.empty()) {
//       if (itsVerbose > 0) {
//         cout<<"..... Compute average PB"<<endl;
//       }
// //       Sum_Stack_PB_CF /= float(sum_weight);
// 
// //       normalized_fft(Sum_Stack_PB_CF, false);
//       Im_Stack_PB_CF0.resize (IPosition(2, m_shape[0], m_shape[1]));
// 
//       float maxPB(0.);
//       for(uInt i=0;i<m_shape[1];++i)
//       {
//          for(uInt j=0;j<m_shape[1];++j)
//          {
//            Complex pixel(Sum_Stack_PB_CF(i,j));
//            if(abs(pixel)>maxPB) maxPB = abs(pixel);
//          }
//       }
//       
// //       untaper(Sum_Stack_PB_CF);
//       
//       float threshold = 1.e-12*maxPB;
//       for (Int jj=0; jj<m_shape[1]; ++jj) {
//         for (Int ii=0; ii<m_shape[0]; ++ii) {
//           Im_Stack_PB_CF0(ii,jj) = abs(sqrt(Sum_Stack_PB_CF(ii,jj))) * (Sum_Stack_PB_CF(ii,jj) > (threshold*maxPB));
//         }
//       }
//       
//       // Make it persistent.
//       store(m_coordinates,Im_Stack_PB_CF0, itsImgName + ".avgpb");
//     }
    uInt N_pol = itsSumPB.shape()(2);
    uInt N_chan = itsSumPB.shape()(3);
    Array<Float> result( IPosition(4, shape(0), shape(1), N_pol, N_chan ), 0.0);
    for(uInt i = 0; i<N_pol; i++) 
    {
      for(uInt j = 0; j<N_chan; j++) 
      {
        Matrix<Float> a,b;
        a.reference(itsSumPB[j][i]);
        b.reference(result[j][i]);
        interpolate( a, b );
      }   
    }
    result = result/itsSumWeight;
    itsLoggingTable.addRow();
    ArrayColumn<Complex> data(itsLoggingTable, "DATA");
    Array<Complex> result0;
    result0.resize(result.shape());
    convertArray( result0, result );
    data.put(itsLoggingTable.nrow()-1, result0);
    
    return result;
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
    taper(spheroidal);
    if (itsVerbose > 0) {
      store(spheroidal, itsImgName + ".spheroidal");
    }
    normalized_fft(spheroidal);
    Double Support_Spheroidal = findSupport(spheroidal, 0.0001);
    if (itsVerbose > 0) {
      store(spheroidal, itsImgName + ".spheroidal_fft");
    }

    Double res_ini = abs(m_coordinates.increment()(0));
    Double diam_image = res_ini*m_shape[0];
    Double Pixel_Size_Spheroidal = diam_image/Support_Spheroidal;
    uInt Npix = floor(diam_image/Pixel_Size_Spheroidal);
    if (Npix%2 != 1) {
    // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
      ++Npix;
      Pixel_Size_Spheroidal = diam_image/Npix;
    }
    Matrix<Complex> Spheroid_cut0(IPosition(2,Npix,Npix),0.);
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
    Matrix<Complex> Spheroid_cut_paddedf=zero_padding(Spheroid_cut,m_shape[0]);
    normalized_fft(Spheroid_cut_paddedf, false);
    Spheroid_cut_im.reference (real(Spheroid_cut_paddedf));
    // Only this one is really needed.
    store(Spheroid_cut_im, itsImgName + ".spheroid_cut_im");
    if (itsVerbose > 0) {
      store(Spheroid_cut, itsImgName + ".spheroid_cut");
    }
    return Pixel_Size_Spheroidal;
  }

  const Matrix<Float>& LofarConvolutionFunction::getSpheroidCut()
  {
    if (Spheroid_cut_im.empty()) {
      makeSpheroidCut();
    }
    return Spheroid_cut_im;
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
    Double Res_w_image = 0.5/(sqrt(2.)*w*(shape[0]/2.)*pixelSize);
    Res_w_image *= 0.8; // SvdT: temporary hack to experiment with resolution
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
    //Double station_diam = 70.;                                           // station diameter in meters: To be adapted to the individual station size.
    Double Res_beam_image= 0.5*((C::c/m_refFrequency)/station_diam)/2.;      // pixel size in A-term image in radian
    Res_beam_image *= 0.07;   // SvdT: temporary hack to experiment with different A-term resolutions
    uInt Npix=floor(diam_image/Res_beam_image);                         // Number of pixel size in A-term image
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
  
  double sinc(double x)
  {
    if (x == 0)
      return 1.0;
    else
      return sin(x*C::pi)/(x*C::pi);
  }
  
//   template <typename T>
//   void interpolate( const Matrix<T> &m1, Matrix<T> &m2) {
  void interpolate( const Matrix<Float> &m1, Matrix<Float> &m2) {

    double factor = double((m1.shape()(0)-2*interpolation_kernel_width))/m2.shape()(0);

    for (int i = 0; i < m2.shape()(0); i++) {
      for (int j = 0; j < m2.shape()(1); j++) {
        double x = i*factor;
        double y = j*factor;
        for (int k = 0; k < 2*interpolation_kernel_width; k++) {
          double x1 = ceil(x)+k;
          for (int l = 0; l < 2*interpolation_kernel_width; l++) {
            double y1 = ceil(y) + l;
            double dx = x1-x-interpolation_kernel_width;
            double dy = y1-y-interpolation_kernel_width;
            m2(i,j) += sinc(dx)*sinc(dx/interpolation_kernel_width)*sinc(dy)*sinc(dy/interpolation_kernel_width)*m1(int(x1), int(y1));
          }
        }
      }
    }
  }
  
  void interpolate( const Matrix<Complex> &m1, Matrix<Complex> &m2) {

    double factor = double((m1.shape()(0)-2*interpolation_kernel_width))/m2.shape()(0);

    for (int i = 0; i < m2.shape()(0); i++) {
      for (int j = 0; j < m2.shape()(1); j++) {
        double x = i*factor;
        double y = j*factor;
        for (int k = 0; k < 2*interpolation_kernel_width; k++) {
          double x1 = ceil(x)+k;
          for (int l = 0; l < 2*interpolation_kernel_width; l++) {
            double y1 = ceil(y) + l;
            double dx = x1-x-interpolation_kernel_width;
            double dy = y1-y-interpolation_kernel_width;
            m2(i,j) += sinc(dx)*sinc(dx/interpolation_kernel_width)*sinc(dy)*sinc(dy/interpolation_kernel_width)*m1(int(x1), int(y1));
          }
        }
      }
    }
  }
 
  Matrix<Complex> find_support( const Matrix<Complex> m ) {
    Vector<Float> maxrow(m.shape()(0), 0.0);
    Vector<Float> maxcol(m.shape()(1), 0.0);
    Float maxval = 0.0;
    for (uInt i = 0; i<m.shape()(0); i++) {
      for(uInt j = 0; j<m.shape()(1); j++) {
        Float absval = abs(m(i,j));
        if (absval > maxrow(i)) maxrow(i) = absval;
        if (absval > maxcol(j)) maxcol(j) = absval;
      }
      if (maxrow(i) > maxval) maxval = maxrow(i);
    }
    Float threshold = maxval * 0.01;
    uInt startrow, endrow, startcol, endcol;
    for( startrow = 0; maxrow(startrow) < threshold; startrow++);
    for( endrow = m.shape()(0)-1; maxrow(endrow) < threshold; endrow--);
    for( startcol = 0; maxcol(startcol) < threshold; startcol++);
    for( endcol = m.shape()(1)-1; maxcol(endcol) < threshold; endcol--);
    if ((startrow == 0) || (endrow == (m.shape()(0)-1)) || (startcol == 0) || (endcol == (m.shape()(1)-1))) {
      cout << "undersampled!! " << threshold << " " << maxrow(0) << " " << maxrow(m.shape()(0)-1) << " " << maxcol(0) << " " << maxcol(m.shape()(1)-1) << endl;
      cout << m.shape() << " " << startrow << " " << endrow << " " << startcol << " " << endcol << endl;
    }
    return m(IPosition(2, startrow, startcol), IPosition(2, endrow, endcol));
  }

} //# end namespace casa
