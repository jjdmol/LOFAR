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

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>

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
#include <casa/OS/Directory.h>
#include <casa/sstream.h>


namespace LOFAR
{
  LofarConvolutionFunction::LofarConvolutionFunction
  (const IPosition& shape,
   const DirectionCoordinate& coordinates,
   const MeasurementSet& ms,
   uInt nW, double Wmax,
   uInt oversample,
   const String& beamElementPath,
   const String& save_image_beam_directory,
   Int verbose,
   Int maxsupport)
    : m_shape(shape),
      m_coordinates(coordinates),
      m_aTerm(ms, beamElementPath),
      maxW(Wmax), //maximum W set by ft machine to flag the w>wmax
      nWPlanes(nW),
      OverSampling(oversample),
      save_image_Aterm_dir(save_image_beam_directory),
      itsVerbose (verbose),
      itsMaxSupport(maxsupport)
      //Not sure how useful that is
  {
    if (itsVerbose > 0) {
      cout<<"LofarConvolutionFunction:shape  "<<shape<<endl;
    }
    itsFFTMachines.resize (OpenMP::maxThreads());
    if(save_image_Aterm_dir!="")
      {
        String Dir_Aterm=save_image_Aterm_dir;
        Directory Dir_Aterm_Obj(Dir_Aterm);
        if (!Dir_Aterm_Obj.exists()) Dir_Aterm_Obj.create();
      }

    MaxCFSupport=0; //need this parameter to stack all the CF for average PB estimate

    m_wScale = WScale(maxW, nWPlanes);
    MEpoch start = observationStartTime(ms, 0);

    RefFrequency = observationReferenceFreq(ms, 0);

    list_freq = Vector<Double>(1);
    list_freq(0) = RefFrequency;
    //        list_freq.push_back(RefFrequency);
    Nchannel=list_freq.size();
    ROMSAntennaColumns antenna(ms.antenna());
    Nstations=antenna.nrow();

    Pixel_Size_Spheroidal=estimateSpheroidalResolution(m_shape, m_coordinates);
    //Double PixelSize=abs(m_coordinates.increment()(0));
    //Double ImageDiameter=PixelSize * m_shape(0);
    //Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, maxW));
    //MaxCFSupport= ImageDiameter / W_Pixel_Ang_Size;
    //Matrix<Complex> Stack_pb_cf0(IPosition(2,MaxCFSupport,MaxCFSupport),0.);
    Matrix<Complex> Stack_pb_cf0(IPosition(2,m_shape(0),m_shape(0)),0.);
    Matrix<float> Stack_pb_cf1(IPosition(2,m_shape(0),m_shape(0)),0.);

    //Stack_pb_cf0(256,300)=1.;
    //Matrix<Complex> Avg_PB_padded00(give_normalized_fft(Stack_pb_cf0,false));
    //store(Avg_PB_padded00,"Avg_PB_padded00.img");


    store_all_W_images(); // store the fft of Wterm into memory
  }

  //      ~LofarConvolutionFunction ()
  //      {
  //      }

  //Compute and store W-terms and A-terms in the fourier domain
  void LofarConvolutionFunction::store_all_W_images()
  {
    DirectionCoordinate coordinates_image_w=m_coordinates;
    Double PixelSize=abs(m_coordinates.increment()(0));
    Double ImageDiameter=PixelSize * m_shape(0);

    //Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, maxW));
    //uInt nPixels_Conv = ImageDiameter / W_Pixel_Ang_Size;
    /// #pragma omp parallel for	
    for(uInt i = 0; i < nWPlanes; ++i) {
      Double W=m_wScale.center(i);
      Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, W));
      Int nPixels_Conv = ImageDiameter / W_Pixel_Ang_Size;
      if (itsVerbose > 0) {
        cout<<"Number of pixel in the "<<i<<"-wplane: "<<nPixels_Conv<<"  (w="<<W<<")"<<endl;
      }
      if (nPixels_Conv > itsMaxSupport) {
        nPixels_Conv = itsMaxSupport;
      }
      // Make even for FFTCMatrix
      if (nPixels_Conv % 2 != 0) {
        nPixels_Conv++;
      }
      W_Pixel_Ang_Size = ImageDiameter / nPixels_Conv;
      IPosition shape_image_w(2, nPixels_Conv, nPixels_Conv);
      Vector<Double> increment(2,W_Pixel_Ang_Size); //Careful with the sign of increment!!!! To check!!!!!!!
      coordinates_image_w.setIncrement(increment);
      Vector<Double> Refpix(2,Double(nPixels_Conv-1)/2.);
      coordinates_image_w.setReferencePixel(Refpix);
      double wavelength(299792458./list_freq(0));
      Matrix<Complex> wTerm = m_wTerm.evaluate(shape_image_w, coordinates_image_w, W/wavelength);

      //Matrix<Complex> wTerm(IPosition(2,shape_image_w(0),shape_image_w(0)),1.);
      //store(wTerm,"Wplane"+String::toString(i)+".img"); //the spheroidal */

      normalized_fft(wTerm);
      /// #pragma critical-section
      Wplanes_store.push_back(wTerm);
    }

    /* uInt nPixels_Conv = ImageDiameter / Pixel_Size_Spheroidal; */
    /* Matrix<Complex> wTerm(IPosition(2,nPixels_Conv,nPixels_Conv),1.); */
    /* taper(wTerm); */
    /* Matrix<Complex> wTermfft(give_normalized_fft(wTerm.copy())); */
    /* Matrix<Complex> wTermfft_padded(zero_padding(wTermfft,m_shape(0))); */
    /* Matrix<Complex> sphe_cut(give_normalized_fft(wTermfft_padded.copy(),false)); */
    /* store(sphe_cut,"sphe_cut.img"); */
    /* Matrix<Complex> wTerm2(IPosition(2,m_shape(0),m_shape(0)),1.); */
    /* taper(wTerm2); */
    /* for(uInt iiii=0;iiii<m_shape(0);++iiii){ */
    /*   for(uInt iiiii=0;iiiii<m_shape(0);++iiiii){ */
    /*     wTerm2(iiii,iiiii)=sphe_cut(iiii,iiiii)/wTerm2(iiii,iiiii); */
    /*   }; */
    /* }; */
    /* store(wTerm2,"ratio_sphe_cut.img"); */
  }


  // Compute the fft of the beam at the minimal resolution for all antennas, and append it to a map object
  // with a (double time) key.
  void LofarConvolutionFunction::Append_Aterm(Double time)
  {
    Double PixelSize=abs(m_coordinates.increment()(0));
    ///Double Pixel_Size_aTerm = estimateAResolution(m_shape, m_coordinates);
    Double ImageDiameter=PixelSize * m_shape(0);
    vector< vector< Cube<Complex> > > list_beam;

    for(uInt i = 0; i < Nstations; ++i) {
      DirectionCoordinate coordinates_image_A(m_coordinates);
      Double A_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateAResolution(m_shape, m_coordinates));
      uInt nPixels_Conv = ImageDiameter / A_Pixel_Ang_Size;
      // Make even for FFTCMatrix
      if (nPixels_Conv % 2 != 0) {
        nPixels_Conv++;
      }
      A_Pixel_Ang_Size = ImageDiameter / nPixels_Conv;
      if (itsVerbose > 0) {
        cout.precision(20);
        cout<<"Number of pixel in the Aplane of "<<i<<": "<<nPixels_Conv<<", time="<<fixed<<time<<endl;
      }
      IPosition shape_image_A(2, nPixels_Conv, nPixels_Conv);
      Vector<Double> increment_old(coordinates_image_A.increment());
      Vector<Double> increment(2,A_Pixel_Ang_Size);
      increment[0]=A_Pixel_Ang_Size*sign(increment_old[0]);
      increment[1]=A_Pixel_Ang_Size*sign(increment_old[1]);
      coordinates_image_A.setIncrement(increment);
      Vector<Double> Refpix(2,Double(nPixels_Conv-1)/2.);
      coordinates_image_A.setReferencePixel(Refpix);


      MEpoch binEpoch;//(epoch);
      binEpoch.set(Quantity(time, "s"));
      //======================================
      // Disable the beam
      //======================================
      //Cube<Complex> aterm_cube(IPosition(3,nPixels_Conv,nPixels_Conv,4),1.);
      //for(uInt iiii=0;iiii<nPixels_Conv;++iiii){
      //  for(uInt iiiii=0;iiiii<nPixels_Conv;++iiiii){
      //    aterm_cube(iiii,iiiii,1)=0.;
      //    aterm_cube(iiii,iiiii,2)=0.;
      //  }
      //}
      //vector< Cube<Complex> > aTermA;
      //aTermA.push_back(aterm_cube);
      //======================================
      // Enable the beam
      //======================================
      vector< Cube<Complex> > aTermA= m_aTerm.evaluate(shape_image_A, coordinates_image_A, i, binEpoch, list_freq, true);
      //======================================

	  

      // Compute the fft on the beam
      for(uInt ch = 0; ch < Nchannel; ++ch) {
        //            cout<<"channel number: "<<ch<<endl;
        for(uInt pol= 0; pol < 4; ++pol) {
          Matrix<Complex> plane=aTermA[ch].xyPlane(pol).copy();
          normalized_fft(plane);
          aTermA[ch].xyPlane(pol) = plane;
        }
      }
      list_beam.push_back(aTermA);
    }
    Aterm_store[time]=list_beam;
  }

  //================================================
  // Compute the convolution function for all channel, for the polarisations specified in the Mueller_mask matrix
  // Also specify weither to compute the Mueller matrix for the forward or the backward step. A dirty way to calculate
  // the average beam has been implemented, by specifying the beam correcping to the given baseline and timeslot.
  // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]

  LofarCFStore LofarConvolutionFunction::makeConvolutionFunction
  (uInt stationA, uInt stationB, Double time, Double w,
   const Matrix<bool>& Mask_Mueller, bool degridding_step,
   double Append_average_PB_CF, Matrix<Complex>& Stack_PB_CF,
   double& sum_weight_square)
  {
    // Stack_PB_CF should be called Sum_PB_CF (it is a sum, no stack).
    CountedPtr<CFTypeVec> res (new vector< vector< vector < Matrix<Complex> > > >());
    CFTypeVec& result = *res;
    vector< vector< vector < Matrix<Complex> > > > result_non_padded;

    // Stack the convolution function if averagepb.img don't exist
    Matrix<Complex> Stack_PB_CF_fft(IPosition(2,m_shape(0),m_shape(0)),0.);
    Bool Stack(false);
    if(Append_average_PB_CF!=0.){Stack=true;}

    // If the beam is not in memory, compute it
        
    map<Double, vector< vector< Cube<Complex> > > >::const_iterator aiter =
      Aterm_store.find(time);
    AlwaysAssert (aiter!=Aterm_store.end(), AipsError);
    const vector< vector< Cube<Complex> > >& aterm = aiter->second;
    ///        if(Aterm_store.find(time)==Aterm_store.end()){Append_Aterm(time);};

    // Load the Wterm
    uInt w_index=m_wScale.plane(w);
    Matrix<Complex> wTerm;
    wTerm = Wplanes_store[w_index];
    Int Npix_out;
    Int Npix_out2;


    if(w>0.){wTerm=conj(wTerm);}
    //wTerm=Complex(0.,1.)*wTerm.copy();

    for(uInt ch=0;ch<Nchannel;++ch) {
      // Maybe putting ".copy()" everywhere is too conservative, but there is still a bug... So I wanted to be sure.

      // Load the Aterm
      const Cube<Complex>& aTermA(aterm[stationA][ch]);
      const Cube<Complex>& aTermB(aterm[stationB][ch]);
      // Determine maximum supprt of A, W, and Spheroidal function for zero padding
      Npix_out=std::max(aTermA.shape()(0),aTermB.shape()(0));
      Npix_out=std::max(static_cast<Int>(wTerm.shape()(0)),Npix_out);
      Npix_out=std::max(static_cast<Int>(Spheroid_cut.shape()(0)),Npix_out);
      if (itsVerbose > 0) {
        cout<<"Number of pixel in the final conv function for baseline ["<< stationA<<", "<<stationB<<"] = "<<Npix_out
            <<" "<<aTermA.shape()(0)<<" "<<aTermB.shape()(0)<<" "<<wTerm.shape()(0)<<endl;
      }

      // Zero pad to make the image planes of the A1, A2, and W term have the same resolution in the image plane
      Matrix<Complex> Spheroid_cut_paddedf(zero_padding(Spheroid_cut,Npix_out));
      Matrix<Complex> wTerm_paddedf(zero_padding(wTerm,Npix_out));
      Cube<Complex> aTermA_padded(zero_padding(aTermA,Npix_out));
      Cube<Complex> aTermB_padded(zero_padding(aTermB,Npix_out));

      // FFT the A and W terms
      normalized_fft(wTerm_paddedf, false);
      normalized_fft(Spheroid_cut_paddedf, false);
      if (itsVerbose > 0) {
        cout << "fft shapes " << wTerm_paddedf.shape() << ' ' << Spheroid_cut_paddedf.shape()
             << ' ' << aTermA_padded.shape() << ' ' << aTermB_padded.shape() << endl;
      }
      for(uInt i=0;i<4;++i) {
        Matrix<Complex> planeAf(aTermA_padded.xyPlane(i).copy());
        Matrix<Complex> planeBf(aTermB_padded.xyPlane(i).copy());
        normalized_fft(planeAf, false);
        normalized_fft(planeBf, false);
        aTermA_padded.xyPlane(i) = planeAf;
        aTermB_padded.xyPlane(i) = conj(planeBf);
      }

      vector< vector < Matrix<Complex> > > Kron_Product;

      // Something I still don't completely understand: for the average PB calculation.
      // The convolution functions padded with a higher value than the minimum one give a
      // better result in the end. If you try Npix_out2=Npix_out, then the average PB shows
      // structure like aliasing, producing high values in the devided disrty map... This
      // is likely to be due to the way fft works?...
      // FIX: I now do the average of the PB by stacking the CF, FFT the result and square 
      // it in the end. This is not the way to do in principle but the result is almost the 
      // same. It should pose no problem I think.
      Matrix<Complex> Spheroid_cut_padded2f;
      Matrix<Complex> wTerm_padded2f;
      Cube<Complex> aTermA_padded2;
      Cube<Complex> aTermB_padded2;
      vector< vector < Matrix<Complex> > > Kron_Product_non_padded; // for average PB claculation
	    
      if(Stack==true){
        Npix_out2=Npix_out;
        Spheroid_cut_padded2f = zero_padding(Spheroid_cut,Npix_out2);
        wTerm_padded2f = zero_padding(wTerm,Npix_out2);
        aTermA_padded2=zero_padding(aTermA,Npix_out2);
        aTermB_padded2=zero_padding(aTermB,Npix_out2);

        normalized_fft(wTerm_padded2f, false);
        normalized_fft(Spheroid_cut_padded2f, false);

        for(uInt i=0;i<4;++i) {
          Matrix<Complex> planeA2f(aTermA_padded2.xyPlane(i).copy());
          Matrix<Complex> planeB2f(aTermB_padded2.xyPlane(i).copy());
          normalized_fft(planeA2f, false);
          normalized_fft(planeB2f, false);
          aTermA_padded2.xyPlane(i) = planeA2f;
          aTermB_padded2.xyPlane(i) = conj(planeB2f);
        }
      }

      // Compute the Mueller matrix considering the Mueller Mask
      uInt ind0;
      uInt ind1;
      uInt ii(0);

      for(uInt row0=0;row0<=1;++row0){
        for(uInt col0=0;col0<=1;++col0){
          vector < Matrix<Complex> > Row;
          vector < Matrix<Complex> > Row_non_padded; // for average PB claculation
          uInt jj(0);
          for(uInt row1=0;row1<=1;++row1){
            for(uInt col1=0;col1<=1;++col1){
              // This Mueller ordering is if the polarision is given as XX, YX, XY, YY
              //ind0=2*row0+row1;
              //ind1=2*col0+col1;
              // This Mueller ordering is if the polarision is given as XX, XY, YX, YY
              ind0=row0+2*row1;
              ind1=col0+2*col1;
              // Compute the convolution function for the given Mueller element
              if(Mask_Mueller(ii,jj)==1){
                // Padded version for oversampling the convolution function
                Matrix<Complex> plane_product=aTermB_padded.xyPlane(ind0)*aTermA_padded.xyPlane(ind1)*wTerm_paddedf*Spheroid_cut_paddedf;
                Matrix<Complex> plane_product_paddedf(zero_padding(plane_product,plane_product.shape()(0)*OverSampling));
                //store(plane_product,"plane_products/plane_product."+String::toString(ii)+"."+String::toString(jj)+".img");
                normalized_fft(plane_product_paddedf);
                plane_product_paddedf *= static_cast<Float>(OverSampling*OverSampling);
                if (itsVerbose>3 && row0==0 && col0==0 && row1==0 && col1==0) {
                  store (plane_product_paddedf, "awfft"+String::toString(stationA)+'-'+String::toString(stationB));
                }
                // Find circle (from outside to inside) where until a value > peak*1e-3.
                // Cut out that box to use as the convolution function.
                // See nPBWProjectFT.cc (findSupport).
                ///for {
                /// }
                Row.push_back(plane_product_paddedf);
                // Non padded version for PB calculation (And no W-term)
                if(Stack==true){
                  Matrix<Complex> plane_productf(aTermB_padded2.xyPlane(ind0).copy()*aTermA_padded2.xyPlane(ind1).copy()*Spheroid_cut_padded2f.copy());
                  //plane_product2=plane_product2*conj(plane_product2.copy());
                  normalized_fft(plane_productf);
                  Row_non_padded.push_back(plane_productf);
                }
              }
              else{
                Matrix<Complex> plane_product;
                Row.push_back(plane_product);
                if(Stack==true){Row_non_padded.push_back(plane_product);}
              }
              jj+=1;
            }
          }
          ii+=1;
          Kron_Product.push_back(Row);
          if(Stack==true){Kron_Product_non_padded.push_back(Row_non_padded);}
        }
      }
      //assert(false);

      if(degridding_step) {
        for (uInt i=0;i<4;++i){
          for (uInt j=i;j<4;++j){
            if(Mask_Mueller(i,j)==true){
              if(i!=j){
                Matrix<Complex> plane_product(Kron_Product[i][j].copy());
                Kron_Product[i][j]=conj(Kron_Product[j][i].copy());
                Kron_Product[j][i]=conj(plane_product.copy());
              }
              else{
                Kron_Product[i][j]=conj(Kron_Product[i][j].copy());
              }
            }
          }
        }
      }
      result.push_back(Kron_Product);
      if(Stack==true){result_non_padded.push_back(Kron_Product_non_padded);}
    }

    // Stacks the weighted quadratic sum of the convolution function of average PB estimate (!!!!! done for channel 0 only!!!)
    if(Stack==true){
      //	  cout<<"...Stack CF for PB estimate"<<endl;
      double weight_square(4.*Append_average_PB_CF*Append_average_PB_CF);
      for (uInt i=0;i<4;++i){
        //if((i==2)||(i==1)) break;
        for (uInt j=0;j<4;++j){
          // Only use diagonal terms for average primary beam.
          if(i==j  &&  Mask_Mueller(i,j)){
            //Stack_PB_CF=0.;
            double istart(m_shape(0)/2.-Npix_out2/2.);
            if((istart-floor(istart))!=0.){istart+=0.5;} //If number of pixel odd then 0th order at the center, shifted by one otherwise
            for(Int ii=0;ii<Npix_out2;++ii){
              for(Int jj=0;jj<Npix_out2;++jj){
                Complex gain(result_non_padded[0][i][j](ii,jj));
                Stack_PB_CF(istart+ii,istart+jj)+=gain*weight_square*weight_square;
              }
            }
		
            sum_weight_square += weight_square*weight_square;

          }
        }
      }
    }
	
    // Put the resulting vec(vec(vec))) in a LofarCFStore object
    CoordinateSystem csys;
    Vector<Float> samp(2,OverSampling);
    Vector<Int> xsup(2,Npix_out);
    Vector<Int> ysup(2,Npix_out);
    Int maxXSup(Npix_out);
    Int maxYSup(Npix_out);
    Quantity PA(0., "deg");
    Int mosPointing(0);
    LofarCFStore CFS(res, csys, samp,  xsup, ysup, maxXSup, maxYSup, PA, mosPointing, Mask_Mueller);

    return CFS;
  }

  //================================================
      
  // Returns the average Primary Beam from the disk
  Matrix<float> LofarConvolutionFunction::Give_avg_pb()
  {
    cout<<"==============Give_avg_pb()"<<endl;
    String PBFile_name("averagepb.img");
    File PBFile(PBFile_name);
    Matrix<float>     Im_Stack_PB_CF;
    if(PBFile.exists()){
      cout<<"..... loading Primary Beam image from disk ....."<<endl;
      ostringstream name(PBFile_name);
      PagedImage<Float> tmp(name.str().c_str());
      Slicer slice(IPosition(4,0,0,0,0), tmp.shape(), IPosition(4,1,1,1,1));
      Array<Float> data;
      tmp.doGetSlice(data, slice);
      IPosition pos(4,m_shape(0),m_shape(0),1,1);
      pos[2]=0.;
      pos[3]=0.;
      for(Int i=0;i<m_shape(0);++i){
        for(Int j=0;j<m_shape(0);++j){
          pos[0]=i;
          pos[1]=j;
          Im_Stack_PB_CF(i,j)=data(pos);
        }
      }
    }
    return Im_Stack_PB_CF;
  };

  // Compute the average Primary Beam from the Stack of convolution functions
  Matrix<Float> LofarConvolutionFunction::Compute_avg_pb
  (Matrix<Complex> &Sum_Stack_PB_CF, double sum_weight_square)
  {

    cout<<"..... Compute average PB"<<endl;

    Sum_Stack_PB_CF /= float(sum_weight_square);
    //store(Stack_PB_CF,"Stack_PB_CF.img");

    normalized_fft(Sum_Stack_PB_CF, false);
    //store(Im_Stack_PB_CF00,"Im_Stack_PB_CF00.img");
    Matrix<Float> Im_Stack_PB_CF0(IPosition(2,m_shape(0),m_shape(0)),0.);
	
    double threshold=1.e-6;
    for(Int jj=0;jj<m_shape(1);++jj){
      for(Int ii=0;ii<m_shape(0);++ii){
        Im_Stack_PB_CF0(ii,jj)=abs(Sum_Stack_PB_CF(ii,jj))*abs(Sum_Stack_PB_CF(ii,jj));
        if(Im_Stack_PB_CF0(ii,jj)<threshold) {
          Im_Stack_PB_CF0(ii,jj)=threshold;
        }
      }
    }
	
    store(Im_Stack_PB_CF0,"averagepb.img");
    return Im_Stack_PB_CF0;
  }

  //================================================
  // Does Zeros padding of a Cube

  Cube<Complex> LofarConvolutionFunction::zero_padding
  (const Cube<Complex>& Image, int Npixel_Out)
  {
    Cube<Complex> Image_Enlarged(Npixel_Out,Npixel_Out,Image.shape()[2]);
    if(Image.shape()[0]==Npixel_Out){
      Image_Enlarged=Image;
      return Image_Enlarged;
    }
    uInt Dii=Image.shape()(0)/2;
    uInt Start_image_enlarged=Npixel_Out/2-Dii; //Is an even number, Assume square image
    if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){Start_image_enlarged+=0.5;} //If number of pixel odd then 0th order at the center, shifted by one otherwise
    //	if((Npix%2)!=1){Npix+=1;Res_w_image = diam_image/Npix;};  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;}; */

    //double ratio(double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0)));
    //if(!toFrequency){ratio=1./ratio;};
    double ratio=1.;

    for(Int pol=0; pol<Image.shape()[2]; ++pol){
      //cout<<"pol: "<<pol<<endl;
      for(Int jj=0;jj<Image.shape()[1];++jj){
        for(Int ii=0;ii<Image.shape()[0];++ii){
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
    IPosition shape_im_out(2, Npixel_Out, Npixel_Out);
    Matrix<Complex> Image_Enlarged(shape_im_out,0.);
    if(Image.shape()[0]==Npixel_Out){
      Image_Enlarged=Image;
      return Image_Enlarged;
    }

    double ratio=1.;

    //if(!toFrequency){ratio=double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0));};

    uInt Dii=Image.shape()(0)/2;
    uInt Start_image_enlarged=shape_im_out(0)/2-Dii; //Is an even number, Assume square image
    if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){Start_image_enlarged+=0.5;} //If number of pixel odd then 0th order at the center, shifted by one otherwise
    /* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
    /* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
    /*   cout<<"Not even!!!"<<endl; */
    /*   Start_image_enlarged+=0.5;}; */
    for(Int ii=0;ii<Image.shape()(0);++ii){
      for(Int jj=0;jj<Image.shape()(1);++jj){
        Image_Enlarged(Start_image_enlarged+ii,Start_image_enlarged+jj)=ratio*Image(ii,jj);
      }
    }
    return Image_Enlarged;
  }

  //================================================
  void LofarConvolutionFunction::normalized_fft
  (Matrix<Complex> &im, bool toFreq)
  {
    ASSERT (im.ncolumn() == im.nrow()  &&  im.size() > 0);
    int tnr = OpenMP::threadNum();
    if (toFreq) {
      itsFFTMachines[tnr].normalized_forward (im.nrow(), im.data());
    } else {
      itsFFTMachines[tnr].normalized_backward (im.nrow(), im.data());
    }
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
  Double LofarConvolutionFunction::observationReferenceFreq
  (const MeasurementSet &ms, uInt idDataDescription)
  {
    // Read polarization id and spectral window id.
    ROMSDataDescColumns desc(ms.dataDescription());
    AlwaysAssert(desc.nrow() > idDataDescription, SynthesisError);
    AlwaysAssert(!desc.flagRow()(idDataDescription), SynthesisError);

    const uInt idWindow = desc.spectralWindowId()(idDataDescription);

    /*        logIO() << LogOrigin("LofarATerm", "initReferenceFreq") << LogIO::NORMAL
              << "spectral window: " << desc.spectralWindowId()(idDataDescription) << LogIO::POST;*/
    //            << "spectral window: " << desc.spectralWindowId() << LogIO::POST;
    // Get spectral information.
    ROMSSpWindowColumns window(ms.spectralWindow());
    AlwaysAssert(window.nrow() > idWindow, SynthesisError);
    AlwaysAssert(!window.flagRow()(idWindow), SynthesisError);

    return window.refFrequency()(idWindow);
  }

  //=================================================
  // Estime spheroidal convolution function from the support of the fft of the spheroidal in the image plane

  Double LofarConvolutionFunction::estimateSpheroidalResolution
  (const IPosition &shape, const DirectionCoordinate &coordinates)
  {


    //cout<<"   99999999999999999999999999999999999999999999999"<<endl;
    Matrix<Complex> spheroidal(shape(0), shape(1));
    spheroidal=1.;
    taper(spheroidal);
    store(spheroidal,"spheroidal.img");
    //ArrayLattice<Complex> lattice0(spheroidal);
    //LatticeFFT::cfft2d(lattice0);
    normalized_fft(spheroidal);
    Double Support_Speroidal=findSupport(spheroidal,0.0001);
    store(spheroidal, "spheroidal.fft.img");
    //        cout<<"Support spheroidal" << Support_Speroidal <<endl;

    Double res_ini=abs(coordinates.increment()(0));
    Double diam_image=res_ini*shape(0);
    Double Pixel_Size_Spheroidal=diam_image/Support_Speroidal;
    uInt Npix=floor(diam_image/Pixel_Size_Spheroidal);
    if((Npix%2)!=1){Npix+=1;Pixel_Size_Spheroidal = diam_image/Npix;}  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
    Matrix<Complex> Spheroid_cut0(IPosition(2,Npix,Npix),0.);
    Spheroid_cut=Spheroid_cut0;
    double istart(shape(0)/2.-Npix/2.);
    if((istart-floor(istart))!=0.){istart+=0.5;} //If number of pixel odd then 0th order at the center, shifted by one otherwise
    for(uInt i=0;i<Npix;++i){
      for(uInt j=0;j<Npix;++j){
        Spheroid_cut(i,j)=spheroidal(istart+i,istart+j);
      }
    }
    Matrix<Complex> Spheroid_cut_paddedf=zero_padding(Spheroid_cut,shape(0));
    normalized_fft(Spheroid_cut_paddedf, false);
    Spheroid_cut_im.reference (real(Spheroid_cut_paddedf));
    store(Spheroid_cut_im,"Spheroid_cut_im.img");
    store(Spheroid_cut,"Spheroid_cut.img");
	
    return Pixel_Size_Spheroidal;
  }

  //=================================================
  // Return the angular resolution required for making the image of the angular size determined by
  // coordinates and shape. The resolution is assumed to be the same on both direction axes.
  Double LofarConvolutionFunction::estimateWResolution
  (const IPosition &shape, const DirectionCoordinate &coordinates,
   Double w) const
  {
    Double res_ini=abs(coordinates.increment()(0));           // pixel size in image in radian
    Double diam_image=res_ini*shape(0);                       // image diameter in radian
    if(w==0.){return diam_image;}
    Double Res_w_image=.5/(sqrt(2.)*w*(shape(0)/2.)*res_ini); // pixel size in W-term image in radian
    uInt Npix=floor(diam_image/Res_w_image);                  // Number of pixel size in W-term image
    Res_w_image = diam_image/Npix;
    if((Npix%2)!=1){Npix+=1;Res_w_image = diam_image/Npix;}  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
    return Res_w_image;
  }

  //=================================================
  // Return the angular resolution required for making the image of the angular size determined by
  // coordinates and shape. The resolution is assumed to be the same on both direction axes.
  Double LofarConvolutionFunction::estimateAResolution
  (const IPosition &shape, const DirectionCoordinate &coordinates) const
  {
    Double res_ini=abs(coordinates.increment()(0));                      // pixel size in image in radian
    Double diam_image=res_ini*shape(0);                                  // image diameter in radian
    Double station_diam = 70.;                                           // station diameter in meters: To be adapted to the individual station size.
    Double Res_beam_image= ((299792458./RefFrequency)/station_diam)/2.; // pixel size in A-term image in radian
    uInt Npix=floor(diam_image/Res_beam_image);                         // Number of pixel size in A-term image
    Res_beam_image=diam_image/Npix;
    if((Npix%2)!=1){Npix+=1;Res_beam_image = diam_image/Npix;}         // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
    return Res_beam_image;
  }

  //=================================================
  Double LofarConvolutionFunction::spheroidal(Double nu) const
  {
    static Double P[2][5] = {{8.203343e-2, -3.644705e-1, 6.278660e-1, -5.335581e-1, 2.312756e-1},
                             {4.028559e-3, -3.697768e-2, 1.021332e-1,-1.201436e-1, 6.412774e-2}};
    static Double Q[2][3] = {{1.0000000e0, 8.212018e-1, 2.078043e-1},
                             {1.0000000e0, 9.599102e-1, 2.918724e-1}};
    uInt part = 0;
    Double end = 0.0;
    if(nu >= 0.0 && nu < 0.75) {
      part = 0;
      end = 0.75;
    }
    else if(nu >= 0.75 && nu <= 1.00) {
      part = 1;
      end = 1.00;
    }
    else {
      return 0.0;
    }
    Double nusq = nu * nu;
    Double delnusq = nusq - end * end;
    Double delnusqPow = delnusq;
    Double top = P[part][0];
    for(uInt k = 1; k < 5; ++k) {
      top += P[part][k] * delnusqPow;
      delnusqPow *= delnusq;
    }

    Double bot = Q[part][0];
    delnusqPow = delnusq;
    for(uInt k = 1; k < 3; ++k) {
      bot += Q[part][k] * delnusqPow;
      delnusqPow *= delnusq;
    }
	
    double result((1.0 - nusq) * (top / bot));
    //if(result<1.e-3){result=1.e-3;};
    return bot == 0.0 ? 0.0 : result;
  }


} //# end namespace casa
