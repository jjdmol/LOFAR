//# LofarConvolutionFunction.h: Compute LOFAR convolution functions on demand.
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

#ifndef LOFARFT_LOFARCONVOLUTIONFUNCTION_H
#define LOFARFT_LOFARCONVOLUTIONFUNCTION_H

#include <assert.h>
#include <LofarFT/LofarATerm.h>
#include <LofarFT/LofarWTerm.h>
#include <LofarFT/LofarCFStore.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/PagedImage.h>
#include <casa/Utilities/Assert.h>

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

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <stdio.h>
#include <stdlib.h>
#include <casa/vector.h>
#include <casa/OS/Directory.h>

#include <casa/sstream.h>
#include <casa/BasicSL/String.h>

using namespace casa;
namespace LOFAR
{

  template <class T>
  void store(const DirectionCoordinate &dir, const Matrix<T> &data, const string &name);

  template <class T>
  void store(const DirectionCoordinate &dir, const Cube<T> &data, const string &name);

  template <class T>
  void store(const Matrix<T> &data, const string &name);

  template <class T>
  void store(const Cube<T> &data, const string &name);


  class LofarConvolutionFunction {
    public:
      LofarConvolutionFunction(const IPosition &shape, const DirectionCoordinate &coordinates,
                               const MeasurementSet &ms, uInt nW, double Wmax, uInt oversample, String save_image_beam_directory="")
                               //, vector< Double > Freqs)
        :   m_shape(shape),
        m_coordinates(coordinates),
        m_aTerm(ms),
        OverSampling(oversample),
        maxW(Wmax), //maximum W set by ft machine to flag the w>wmax
        nWPlanes(nW),
        save_image_Aterm_dir(save_image_beam_directory)
        //Not sure how useful that is
      {
	//cout<<"LofarConvolutionFunction:shape  "<<shape<<endl;
        ind_time_check=0;
        sum_weight_square=0;
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
        Stack_PB_CF=Stack_pb_cf0;
        Matrix<float> Stack_pb_cf1(IPosition(2,m_shape(0),m_shape(0)),0.);
        Im_Stack_PB_CF=Stack_pb_cf1;

	//Stack_pb_cf0(256,300)=1.;
        //Matrix<Complex> Avg_PB_padded00(give_normalized_fft(Stack_pb_cf0,false));
	//store(Avg_PB_padded00,"Avg_PB_padded00.img");


        store_all_W_images(); // store the fft of Wterm into memory
      }

//      ~LofarConvolutionFunction ()
//      {
//      }

      //Compute and store W-terms and A-terms in the fourier domain
      void store_all_W_images()
      {
        DirectionCoordinate coordinates_image_w=m_coordinates;
        Double PixelSize=abs(m_coordinates.increment()(0));
        Double ImageDiameter=PixelSize * m_shape(0);

        //Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, maxW));
        //uInt nPixels_Conv = ImageDiameter / W_Pixel_Ang_Size;
	
        for(uInt i = 0; i < nWPlanes; ++i) {
          Double W=m_wScale.center(i);
          Double W_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateWResolution(m_shape, m_coordinates, W));
          uInt nPixels_Conv = ImageDiameter / W_Pixel_Ang_Size;
          //cout<<"Number of pixel in the "<<i<<"-wplane: "<<nPixels_Conv<<"  (w="<<W<<")"<<endl;
          IPosition shape_image_w(2, nPixels_Conv, nPixels_Conv);
          Vector<Double> increment(2,W_Pixel_Ang_Size); //Careful with the sign of increment!!!! To check!!!!!!!
          coordinates_image_w.setIncrement(increment);
          Vector<Double> Refpix(2,Double(nPixels_Conv-1)/2.);
          coordinates_image_w.setReferencePixel(Refpix);
	  double wavelength(299792458./136523437.5);//list_freq(0));
          Matrix<Complex> wTerm = m_wTerm.evaluate(shape_image_w, coordinates_image_w, W/wavelength);

	  //Matrix<Complex> wTerm(IPosition(2,shape_image_w(0),shape_image_w(0)),1.);
          //store(wTerm,"Wplane"+String::toString(i)+".img"); //the spheroidal */

          Matrix<Complex> wTermfft(give_normalized_fft(wTerm));

          Wplanes_store.push_back(wTermfft);
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
      void Append_Aterm(Double time) {
        Double PixelSize=abs(m_coordinates.increment()(0));
        Double Pixel_Size_aTerm = estimateAResolution(m_shape, m_coordinates);
        Double ImageDiameter=PixelSize * m_shape(0);
        vector< vector< Cube<Complex> > > list_beam;

        for(uInt i = 0; i < Nstations; ++i) {
	  DirectionCoordinate coordinates_image_A(m_coordinates);
          Double A_Pixel_Ang_Size=min(Pixel_Size_Spheroidal,estimateAResolution(m_shape, m_coordinates));
          uInt nPixels_Conv = ImageDiameter / A_Pixel_Ang_Size;
	  //cout.precision(20);
          //cout<<"Number of pixel in the Aplane of "<<i<<": "<<nPixels_Conv<<", time="<<fixed<<time<<endl;
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
          /* Cube<Complex> aterm_cube(IPosition(3,nPixels_Conv,nPixels_Conv,4),1.); */
	  /* for(uInt iiii=0;iiii<nPixels_Conv;++iiii){ */
	  /*   for(uInt iiiii=0;iiiii<nPixels_Conv;++iiiii){ */
	  /*     aterm_cube(iiii,iiiii,1)=0.; */
	  /*     aterm_cube(iiii,iiiii,2)=0.; */
	  /*   }; */
	  /* }; */
	  /* vector< Cube<Complex> > aTermA; */
          /* aTermA.push_back(aterm_cube); */
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
              Matrix<Complex> planefft=give_normalized_fft(plane);
              aTermA[ch].xyPlane(pol)=planefft;
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

      LofarCFStore makeConvolutionFunction(uInt stationA, uInt stationB, Double time, Double w, Matrix<bool> Mask_Mueller, bool degridding_step, double Append_average_PB_CF=0.)
      {
        vector< vector< vector < Matrix<Complex> > > > result;
        vector< vector< vector < Matrix<Complex> > > > result_non_padded;

        // If the beam is not in memory, compute it
        if(Aterm_store.find(time)==Aterm_store.end()){Append_Aterm(time);};//else{cout<<"time="<<time<<" already exists"<<endl;};

        // Load the Wterm
        uInt w_index=m_wScale.plane(w);
        Matrix<Complex> wTerm=Wplanes_store[w_index].copy();
        Int Npix_out;

	if(w>0.){wTerm=conj(wTerm.copy());};
	//wTerm=Complex(0.,1.)*wTerm.copy();

        for(uInt ch=0;ch<Nchannel;++ch) {
          // Maybe putting ".copy()" everywhere is too conservative, but there is still a bug... So I wanted to be sure.

          // Load the Aterm
          Cube<Complex> aTermA(Aterm_store[time][stationA][ch].copy());
          Cube<Complex> aTermB(Aterm_store[time][stationB][ch].copy());
	  /* //=========================================== */
	  /* uInt nPixels_Conv(aTermA.shape()(0)); */
	  /* Cube<Complex> planestore_paddedA(zero_padding(aTermA,m_shape(0))); */
	  /* Cube<Complex> planestore_paddedB(zero_padding(aTermB,m_shape(0))); */
	  /* Matrix<Complex> planestore_paddedW(zero_padding(wTerm,m_shape(0))); */
	  /* Cube<Complex> planestore_paddedA_fft(IPosition(3,m_shape(0),m_shape(0),4),0.); */
	  /* Cube<Complex> planestore_paddedB_fft(IPosition(3,m_shape(0),m_shape(0),4),0.); */
	  /* Matrix<Complex> planestore_paddedW_fft(IPosition(2,m_shape(0),m_shape(0)),0.); */
	  /* for(uInt pol= 0; pol < 4; ++pol) { */
	  /*   planestore_paddedA_fft.xyPlane(pol)=give_normalized_fft(planestore_paddedA.xyPlane(pol),false); */
	  /*   planestore_paddedB_fft.xyPlane(pol)=conj(give_normalized_fft(planestore_paddedB.xyPlane(pol),false)); */
	  /* }; */
	  /* planestore_paddedW_fft=give_normalized_fft(planestore_paddedW,false); */
	  /* store(m_coordinates,planestore_paddedA_fft,"beamA.fft.T"+String::toString(ind_time_check)+".img"); */
	  /* store(m_coordinates,planestore_paddedB_fft,"beamB.fft.T"+String::toString(ind_time_check)+".img"); */
	  /* ind_time_check+=1; */
	  /* cout<<"============"<<endl; */
	  /* cout<<"beam_meas 0 "<<ind_time_check<<" "<<100.*planestore_paddedA_fft(351,319,0)*planestore_paddedB_fft(351,319,0)*planestore_paddedW_fft(351,319)+100.*planestore_paddedA_fft(351,319,1)*planestore_paddedB_fft(351,319,1)*planestore_paddedW_fft(351,319)<<endl; */
	  /* cout<<"beam_meas 1 "<<ind_time_check<<" "<<100.*planestore_paddedA_fft(351,319,0)*planestore_paddedB_fft(351,319,2)*planestore_paddedW_fft(351,319)+100.*planestore_paddedA_fft(351,319,1)*planestore_paddedB_fft(351,319,3)*planestore_paddedW_fft(351,319)<<endl; */
	  /* cout<<"beam_meas 2 "<<ind_time_check<<" "<<100.*planestore_paddedA_fft(351,319,2)*planestore_paddedB_fft(351,319,0)*planestore_paddedW_fft(351,319)+100.*planestore_paddedA_fft(351,319,3)*planestore_paddedB_fft(351,319,1)*planestore_paddedW_fft(351,319)<<endl; */
	  /* cout<<"beam_meas 3 "<<ind_time_check<<" "<<100.*planestore_paddedA_fft(351,319,2)*planestore_paddedB_fft(351,319,2)*planestore_paddedW_fft(351,319)+100.*planestore_paddedA_fft(351,319,3)*planestore_paddedB_fft(351,319,3)*planestore_paddedW_fft(351,319)<<endl; */
	  /* //=========================================== */
          Npix_out=std::max(aTermA.shape()(0),aTermB.shape()(0));
          Npix_out=std::max(static_cast<Int>(wTerm.shape()(0)),Npix_out);
          Npix_out=std::max(static_cast<Int>(Spheroid_cut.shape()(0)),Npix_out);

          cout<<"Number of pixel in the final conv function for baseline ["<< stationA<<", "<<stationB<<"] = "<<Npix_out
	      <<" "<<aTermA.shape()(0)<<" "<<aTermB.shape()(0)<<" "<<wTerm.shape()(0)<<endl;
	  //	  store(aTermA,"aTermA.img");
	  //store(aTermB,"aTermB.img");
	  //store(wTerm,"wTerm.fft.img");

          // To make the image planes of the A1, A2, and W term have the same resolution in the image plane
	  Matrix<Complex> Spheroid_cut_padded(zero_padding(Spheroid_cut,Npix_out));
          Matrix<Complex> wTerm_padded(zero_padding(wTerm,Npix_out));
          Cube<Complex> aTermA_padded(zero_padding(aTermA,Npix_out));
          Cube<Complex> aTermB_padded(zero_padding(aTermB,Npix_out));


          // FFT the A and W terms
          Matrix<Complex> wTerm_padded_fft(give_normalized_fft(wTerm_padded,false));
          Matrix<Complex> Spheroid_cut_padded_fft(give_normalized_fft(Spheroid_cut_padded,false));


          for(uInt i=0;i<4;++i) {
            Matrix<Complex> planeA(aTermA_padded.xyPlane(i).copy());
            Matrix<Complex> planeB(aTermB_padded.xyPlane(i).copy());
            Matrix<Complex> planeA_fft(give_normalized_fft(planeA,false));
            Matrix<Complex> planeB_fft(give_normalized_fft(planeB,false));
            aTermA_padded.xyPlane(i)=planeA_fft.copy();//*wTerm_padded_fft.copy();
            aTermB_padded.xyPlane(i)=conj(planeB_fft.copy());
          }

	  //store(aTermA_padded,"aTermA_padded.img");
	  //store(aTermB_padded,"aTermB_padded.img");
	  //store(wTerm_padded_fft,"wTerm.img");

	  /* Matrix<Complex> dummy(aTermB_padded.xyPlane(1).copy()); */
	  /* aTermB_padded.xyPlane(1)=aTermB_padded.xyPlane(2).copy(); */
	  /* aTermB_padded.xyPlane(2)=dummy.copy(); */
	  /* dummy=aTermA_padded.xyPlane(1); */
	  /* aTermA_padded.xyPlane(1)=aTermA_padded.xyPlane(2).copy(); */
	  /* aTermA_padded.xyPlane(2)=dummy.copy(); */
            
          vector< vector < Matrix<Complex> > > Kron_Product;
          vector< vector < Matrix<Complex> > > Kron_Product_non_padded; // for average PB claculation

          // Compute the Mueller matrix considering the Muleller Mask
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
		  // This Mueller ordering is if the oplarision is given as XX, YX, XY, YY
                  //ind0=2*row0+row1;
                  //ind1=2*col0+col1;
                  ind0=row0+2*row1;
                  ind1=col0+2*col1;
                  if(Mask_Mueller(ii,jj)==1){
		    //cout<<"Mueller term: "<<ii<<jj<<", Index BA: "<<ind0<<ind1<<endl;
                    Matrix<Complex> plane_product(aTermB_padded.xyPlane(ind0).copy()*aTermA_padded.xyPlane(ind1).copy()*Spheroid_cut_padded_fft.copy());
		    //taper(plane_product);
		    
                    Matrix<Complex> plane_product_fft(give_normalized_fft(plane_product));
                    Row_non_padded.push_back(plane_product_fft.copy());

                    plane_product=aTermB_padded.xyPlane(ind0).copy()*aTermA_padded.xyPlane(ind1).copy()*wTerm_padded_fft.copy()*Spheroid_cut_padded_fft.copy();
                    //plane_product_fft*=Npix_out*Npix_out;

                    //store(plane_product,"beam.T"+String::toString(ind_time_check)+".A"+String::toString(stationA)+".A"+String::toString(stationB)+".M"+String::toString(ii)+".M"+String::toString(jj)+".img");
                    //store(plane_product_fft,"beamfft.T"+String::toString(ind_time_check)+".A"+String::toString(stationA)+".A"+String::toString(stationB)+".M"+String::toString(ii)+".M"+String::toString(jj)+".img");

                    Matrix<Complex> plane_product_padded(zero_padding(plane_product,plane_product.shape()(0)*OverSampling));
                    Matrix<Complex> plane_product_padded_fft(give_normalized_fft(plane_product_padded));
                    plane_product_padded_fft *= static_cast<Float>(OverSampling*OverSampling);
                    Row.push_back(plane_product_padded_fft);
                  }
                  else{
                    Matrix<Complex> plane_product;
                    Row.push_back(plane_product);
                    Row_non_padded.push_back(plane_product);
                  }
                  jj+=1;
                }
              }
              ii+=1;
              Kron_Product.push_back(Row);
              Kron_Product_non_padded.push_back(Row_non_padded);
            }
          }

          // If imaging step, then we have to take the hermitian conjugate
          // !!! More general case of any Mask_Mueller case should be implemented
          /* if(!degridding_step) { */
          /*   for (uInt i=0;i<4;++i){ */
          /*     for (uInt j=i;j<4;++j){ */
          /*       if(Mask_Mueller(i,j)==true){ */
          /*         /\* if(i!=j){ *\/ */
          /*         /\*   Matrix<Complex> plane_product(Kron_Product[i][j].copy()); *\/ */
          /*         /\*   Kron_Product[i][j]=Kron_Product[j][i].copy(); *\/ */
          /*         /\*   Kron_Product[j][i]=plane_product.copy(); *\/ */
          /*         /\* } *\/ */
          /*         Kron_Product[j][i]=conj(Kron_Product[j][i]); */
          /*       } */
          /*     }; */
          /*   } */
          /* }; */

          /* if(degridding_step) { */
          /*   for (uInt i=0;i<4;++i){ */
          /*     for (uInt j=i;j<4;++j){ */
          /*       if(Mask_Mueller(i,j)==true){ */
          /*         if(i!=j){ */
          /*           Matrix<Complex> plane_product(Kron_Product[i][j].copy()); */
          /*           Kron_Product[i][j]=conj(Kron_Product[j][i].copy()); */
          /*           Kron_Product[j][i]=conj(plane_product.copy()); */
          /*         } */
          /*         else{ */
	  /* 	    Kron_Product[i][j]=conj(Kron_Product[i][j].copy()); */
          /*         }; */
          /*       } */
          /*     }; */
          /*   } */
          /* }; */
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
                  };
                }
              };
            }
          };
          result.push_back(Kron_Product);
          result_non_padded.push_back(Kron_Product_non_padded);
        }

        // Stacks the weighted quadratic sum of the convolution function of average PB estimate (!!!!! done for channel 0 only!!!)
	
	Matrix<Complex> Stack_PB_CF_fft(IPosition(2,m_shape(0),m_shape(0)),0.);
	File PBFile("averagepb.img");
        Matrix<float> ones_taper(IPosition(2,m_shape(0),m_shape(0)),1.);
        taper(ones_taper);

	if((Append_average_PB_CF!=0.)&&((!PBFile.exists()))){
	  cout<<"...Stack CF for PB estimate"<<endl;
          double weight_square(Append_average_PB_CF*Append_average_PB_CF);
	  //weight_square=1.;

	  
	  for (uInt i=0;i<4;++i){
            //if((i==2)||(i==1)) break;
            for (uInt j=0;j<4;++j){
	      if(!(i==j)) break;
	      //if(!(i==0)) break;
              if(Mask_Mueller(i,j)==true){
		Stack_PB_CF=0.;
                double istart(m_shape(0)/2.-Npix_out/2.);
		if((istart-floor(istart))!=0.){istart+=0.5;}; //If number of pixel odd then 0th order at the center, shifted by one otherwise
                for(uInt ii=0;ii<Npix_out;++ii){
                  for(uInt jj=0;jj<Npix_out;++jj){
                    Complex gain(result_non_padded[0][i][j](ii,jj));
                    Stack_PB_CF(istart+ii,istart+jj)=gain;//*conj(gain));
		    //if(((istart+ii)==(istart+jj))&&((istart+jj)==456-100.)){Stack_PB_CF(istart+ii,istart+jj)=1.;};
                  }
                }
		Stack_PB_CF_fft=give_normalized_fft(Stack_PB_CF,false);
		//Im_Stack_PB_CF=Im_Stack_PB_CF+weight_square*weight_square*real(Stack_PB_CF_fft)*real(Stack_PB_CF_fft);
                for(uInt ii=0;ii<m_shape(0);++ii){
                  for(uInt jj=0;jj<m_shape(0);++jj){
		    double pixel(abs(Stack_PB_CF_fft(ii,jj)));
		    //pixel*=weight_square/ones_taper(ii,jj);
		    //pixel*=weight_square/ones_taper(ii,jj);
                    Im_Stack_PB_CF(ii,jj)=Im_Stack_PB_CF(ii,jj)+weight_square*weight_square*abs(pixel)*abs(pixel);
                  }
                }

		//store(Stack_PB_CF_fft,"beam_square"+String::toString(i)+String::toString(j)+".img");
		//store(Im_Stack_PB_CF,"beam_square"+String::toString(i)+String::toString(j)+".img");
		//store(Stack_PB_CF,"uvbeam_square"+String::toString(i)+String::toString(j)+".img");
		
                sum_weight_square+=weight_square*weight_square;
              };
            }
          }
	  //sum_weight_square+=weight_square*weight_square;
        };
	
        ind_time_check+=1;
        // Put the resulting vec(vec(vec))) in a LofarCFStore object
        CFTypeVec* res(&result);
        CoordinateSystem csys;
        Vector<Float> samp(2,OverSampling);
        Vector<Int> xsup(2,Npix_out);//change, TRY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        Vector<Int> ysup(2,Npix_out);
        Int maxXSup(Npix_out);
        Int maxYSup(Npix_out);
        Quantity PA(0., "deg");
        Int mosPointing(0);
        LofarCFStore CFS(res, csys, samp,  xsup, ysup, maxXSup, maxYSup, PA, mosPointing, Mask_Mueller);
        return CFS;
      }

      //================================================
      // Compute the average Primary Beam from the Stack of convolution functions

      Matrix<float> Compute_avg_pb()
      {
	String PBFile_name("averagepb.img");
	File PBFile(PBFile_name);
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
	  for(uInt i=0;i<m_shape(0);++i){
	    for(uInt j=0;j<m_shape(0);++j){
	      pos[0]=i;
	      pos[1]=j;
	      Im_Stack_PB_CF(i,j)=data(pos);
	    };
	  };
	  return Im_Stack_PB_CF;
	};

	cout<<"..... Compute average PB"<<endl;
	
	


        //Matrix<Complex> Stack_PB_CF_norm(Stack_PB_CF);
        //store(Stack_PB_CF,"Stack_PB_CF.img");

	//        Matrix<Complex> Avg_PB_padded(give_normalized_fft(Stack_PB_CF_norm,false));
	//store(Avg_PB_padded,"Avg_PB_padded.img");

        Matrix<float> Avg_PB_padded_out(IPosition(2,m_shape(0),m_shape(0)),0.);
        Matrix<float> Avg_PB_padded_out_taper(IPosition(2,m_shape(0),m_shape(0)),1.);
        taper(Avg_PB_padded_out_taper);
	store(Im_Stack_PB_CF,"SumPB.img");

	
        for(uInt ii=0;ii<m_shape(0);++ii){
          for(uInt jj=0;jj<m_shape(0);++jj){
	    double pixel(Im_Stack_PB_CF(ii,jj));
	    double fact(sum_weight_square);//*Avg_PB_padded_out_taper(ii,jj)*Avg_PB_padded_out_taper(ii,jj));
            Avg_PB_padded_out(ii,jj)=pixel/fact;
	    if(Avg_PB_padded_out(ii,jj)<0.00001){Avg_PB_padded_out(ii,jj)=0.00001;};
	    //Im_Stack_PB_CF(ii,jj)/=sum_weight_square*pixel;
	    //pixel=Im_Stack_PB_CF(ii,jj);
	    //if(pixel<0.00001){pixel=0.00001;};
	    //Im_Stack_PB_CF(ii,jj)=pixel;
          }
        }
	store(Avg_PB_padded_out_taper,"sphe.img");
        //store(Avg_PB_padded_out,"averagepb.img");
	
        store(Avg_PB_padded_out,"averagepb.img");
        return Avg_PB_padded_out;//Avg_PB_padded_out;
      }

      //================================================
      // Does Zeros padding of a Cube

      Cube<Complex> zero_padding(const Cube<Complex> Image, int Npixel_Out)//, bool toFrequency=true)
      {
        Cube<Complex> Image_Enlarged(Npixel_Out,Npixel_Out,Image.shape()(2));
        if(Image.shape()(0)==Npixel_Out){
          Image_Enlarged=Image;
          return Image_Enlarged;
        };
        uInt Dii=Image.shape()(0)/2;
        uInt Start_image_enlarged=Npixel_Out/2-Dii; //Is an even number, Assume square image
	if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){Start_image_enlarged+=0.5;}; //If number of pixel odd then 0th order at the center, shifted by one otherwise
	//	if((Npix%2)!=1){Npix+=1;Res_w_image = diam_image/Npix;};  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
	/* cout<<Start_image_enlarged<<"  "<<floor(Start_image_enlarged)<<endl; */
	/* if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){ */
	/*   cout<<"Not even!!!"<<endl; */
	/*   Start_image_enlarged+=0.5;}; */

        //double ratio(double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0)));
        //if(!toFrequency){ratio=1./ratio;};
        double ratio=1.;


        /* ArrayIterator<Complex> iter(mapout,2); */
        /* iter.array()=planeA*planeB; */
        /* iter.next(); */

        for(Int pol=0; pol<Image.shape()[2]; ++pol){
          //cout<<"pol: "<<pol<<endl;
          for(Int ii=0;ii<Image.shape()[0];++ii){
            for(Int jj=0;jj<Image.shape()[1];++jj){
              Image_Enlarged(Start_image_enlarged+ii,
                             Start_image_enlarged+jj,pol) = ratio*Image(ii,jj,pol);
            }
          }
        }
        return Image_Enlarged;
      }

      //================================================
      // Zeros padding of a Matrix

      Matrix<Complex> zero_padding(const Matrix<Complex> Image, int Npixel_Out)//, bool toFrequency=true)
      {
        IPosition shape_im_out(2, Npixel_Out, Npixel_Out);
        Matrix<Complex> Image_Enlarged(shape_im_out,0.);
        if(Image.shape()(0)==Npixel_Out){
          Image_Enlarged=Image;
          return Image_Enlarged;
        };

        double ratio=1.;

        //if(!toFrequency){ratio=double(Npixel_Out)*double(Npixel_Out)/(Image.shape()(0)*Image.shape()(0));};

        uInt Dii=Image.shape()(0)/2;
        uInt Start_image_enlarged=shape_im_out(0)/2-Dii; //Is an even number, Assume square image
	if((Start_image_enlarged-floor(Start_image_enlarged))!=0.){Start_image_enlarged+=0.5;}; //If number of pixel odd then 0th order at the center, shifted by one otherwise
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

      const WScale &wScale() const
      {
        return m_wScale;
      }

    private:

      Matrix<Complex> give_normalized_fft(const Matrix<Complex> &im, bool toFreq=true)
      {
        Matrix<Complex> result(im.copy());
        ArrayLattice<Complex> lattice(result);
        LatticeFFT::cfft2d(lattice, toFreq);
        if(toFreq){
          result/=static_cast<Float>(result.shape()(0)*result.shape()(1));
        }
        else{
          result*=static_cast<Float>(result.shape()(0)*result.shape()(1));
        };
        return result;
      }


/*      LogIO &logIO() const
      {
        return m_logIO;
      }*/

      //=================================================
      MEpoch observationStartTime(const MeasurementSet &ms, uInt idObservation) const
      {
        // Get phase center as RA and DEC (J2000).
        ROMSObservationColumns observation(ms.observation());
        AlwaysAssert(observation.nrow() > idObservation, SynthesisError);
        AlwaysAssert(!observation.flagRow()(idObservation), SynthesisError);

        return observation.timeRangeMeas()(0)(IPosition(1, 0));
      }

      //=================================================
      Double observationReferenceFreq(const MeasurementSet &ms, uInt idDataDescription)
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
      // Not used anymore

      Double estimateConvResolution(const IPosition &shape, const DirectionCoordinate &coordinates, Double w) const
      {
        uInt wPlane = m_wScale.plane(w);
        Double wmax_center = m_wScale.center(wPlane);
        // 1. Request required angular scales for A-term, W-term.
        Double wTermResolution = estimateWResolution(m_shape, m_coordinates, wmax_center);
        Double aTermResolution = estimateAResolution(m_shape, m_coordinates);
        // 2. Compute minimum angular scale.
        Double minResolution = min(wTermResolution, aTermResolution);
        return minResolution;
      }

      //=================================================
      // Estime spheroidal convolution function from the support of the fft of the spheroidal in the image plane

      Double estimateSpheroidalResolution(const IPosition &shape, const DirectionCoordinate &coordinates)
      {


	//cout<<"   99999999999999999999999999999999999999999999999"<<endl;
        Matrix<Complex> spheroidal(shape(0), shape(1));
        spheroidal=1.;
        taper(spheroidal);
        store(spheroidal,"spheroidal.img");
        //ArrayLattice<Complex> lattice0(spheroidal);
        //LatticeFFT::cfft2d(lattice0);
	spheroidal=give_normalized_fft(spheroidal);
        Double Support_Speroidal=findSupport(spheroidal,0.0001);
        store(spheroidal, "spheroidal.fft.img");
//        cout<<"Support spheroidal" << Support_Speroidal <<endl;




        Double res_ini=abs(coordinates.increment()(0));
        Double diam_image=res_ini*shape(0);
        Double Pixel_Size_Spheroidal=diam_image/Support_Speroidal;
        uInt Npix=floor(diam_image/Pixel_Size_Spheroidal);
        if((Npix%2)!=1){Npix+=1;Pixel_Size_Spheroidal = diam_image/Npix;};  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
	Matrix<Complex> Spheroid_cut0(IPosition(2,Npix,Npix),0.);
	Spheroid_cut=Spheroid_cut0;
	double istart(shape(0)/2.-Npix/2.);
	if((istart-floor(istart))!=0.){istart+=0.5;}; //If number of pixel odd then 0th order at the center, shifted by one otherwise
	for(uInt i=0;i<Npix;++i){
	  for(uInt j=0;j<Npix;++j){
	    Spheroid_cut(i,j)=spheroidal(istart+i,istart+j);
	  };
	};
	Matrix<Complex> Spheroid_cut_padded=zero_padding(Spheroid_cut,shape(0));
	Matrix<Complex> Spheroid_cut_padded0=give_normalized_fft(Spheroid_cut_padded,false);
	Matrix<float> Spheroid_cut_padded00(IPosition(2,shape(0),shape(0)),0.);
	Spheroid_cut_padded00=real(Spheroid_cut_padded0.copy());
	Spheroid_cut_im=Spheroid_cut_padded00.copy();
	store(Spheroid_cut_im,"Spheroid_cut_im.img");
	store(Spheroid_cut,"Spheroid_cut.img");
	
        return Pixel_Size_Spheroidal;
      }

      //=================================================
      // Return the angular resolution required for making the image of the angular size determined by
      // coordinates and shape. The resolution is assumed to be the same on both direction axes.
      Double estimateWResolution(const IPosition &shape, const DirectionCoordinate &coordinates, Double w) const
      {
        Double res_ini=abs(coordinates.increment()(0));           // pixel size in image in radian
        Double diam_image=res_ini*shape(0);                       // image diameter in radian
        if(w==0.){return diam_image;};
        Double Res_w_image=.5/(sqrt(2.)*w*(shape(0)/2.)*res_ini); // pixel size in W-term image in radian
        uInt Npix=floor(diam_image/Res_w_image);                  // Number of pixel size in W-term image
        Res_w_image = diam_image/Npix;
        if((Npix%2)!=1){Npix+=1;Res_w_image = diam_image/Npix;};  // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
        return Res_w_image;
      }

      //=================================================
      // Return the angular resolution required for making the image of the angular size determined by
      // coordinates and shape. The resolution is assumed to be the same on both direction axes.
      Double estimateAResolution(const IPosition &shape, const DirectionCoordinate &coordinates) const
      {
        Double res_ini=abs(coordinates.increment()(0));                      // pixel size in image in radian
        Double diam_image=res_ini*shape(0);                                  // image diameter in radian
        Double station_diam = 70.;                                           // station diameter in meters: To be adapted to the individual station size.
        Double Res_beam_image= ((299792458./RefFrequency)/station_diam)/2.; // pixel size in A-term image in radian
        uInt Npix=floor(diam_image/Res_beam_image);                         // Number of pixel size in A-term image
        Res_beam_image=diam_image/Npix;
        if((Npix%2)!=1){Npix+=1;Res_beam_image = diam_image/Npix;};         // Make the resulting image have an even number of pixel (to make the zeros padding step easier)
        return Res_beam_image;
      }

      //=================================================
      // Apply a spheroidal taper to the input function.
      template <typename T>
        void taper(Matrix<T> &function) const
        {
          AlwaysAssert(function.shape()(0) == function.shape()(1), SynthesisError);
	  //cout<<"function.shape()(0) "<<function.shape()(0)<<endl;
          uInt size = function.shape()(0);
          Double halfSize = (size-1) / 2.0;
          Vector<Double> x(size);
          for(uInt i = 0; i < size; ++i)
          {
            x(i) = spheroidal(abs(Double(i) - halfSize) / halfSize);
          }
          for(uInt i = 0; i < size; ++i) {
            for(uInt j = 0; j < size; ++j) {
              function(j, i) *= x(i) * x(j);
            }
          }
        }

      //=================================================
      Double spheroidal(Double nu) const
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

      //=================================================
      template <typename T>
        uInt findSupport(Matrix<T> &function, Double threshold) const
        {
          Double peak = abs(max(abs(function)));
          threshold *= peak;
          uInt halfSize = function.shape()(0) / 2;
          uInt x = 0;
          while(x < halfSize && abs(function(x, halfSize)) < threshold) {
            ++x;
          }
          return 2 * (halfSize - x);
        }

      IPosition           m_shape;
      DirectionCoordinate m_coordinates;
      WScale              m_wScale;
      LofarWTerm          m_wTerm;
      LofarATerm          m_aTerm;
      Double              maxW;
      Double              Pixel_Size_Spheroidal;
      uInt                nWPlanes;
      uInt                Nstations;
      uInt                OverSampling;
      uInt                Nchannel;
      Double              RefFrequency;
      Double              sum_weight_square;
      uInt                MaxCFSupport;
      Matrix<Complex>     Stack_PB_CF; // Stack of the convolution functions for the average PB calculation
      Matrix<Complex>     Spheroid_cut; // Stack of the convolution functions for the average PB calculation
      Matrix<float>     Spheroid_cut_im; // Stack of the convolution functions for the average PB calculation
      Matrix<float>     Im_Stack_PB_CF; // Stack of the convolution functions for the average PB calculation
      DirectionCoordinate coordinates_Conv_Func_image;
      string                save_image_Aterm_dir;
      uInt ind_time_check;
      Vector< Double >   list_freq;   // List of the ferquencies the CF have to be caluclated for
      vector< Matrix<Complex> >            Wplanes_store;
      map<Double, vector< vector< Cube<Complex> > > > Aterm_store;//Aterm_store[double time][antenna][channel]=Cube[Npix,Npix,4]
//       mutable LogIO       m_logIO;
      };

      //=================================================
// Utility function to store a Matrix as an image for debugging. It uses arbitrary values for the
    // direction, Stokes and frequency axes.
    template <class T>
    void store(const Matrix<T> &data, const string &name)
    {
      Matrix<Double> xform(2, 2);
      xform = 0.0;
      xform.diagonal() = 1.0;
      Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
      Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
      Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
      DirectionCoordinate dir(MDirection::J2000, Projection(Projection::SIN),
                         refLatLon, refLatLon, incLon, incLat,
                         xform, data.shape()(0) / 2, data.shape()(1) / 2);

      store(dir, data, name);
    }

    template <class T>
    void store(const DirectionCoordinate &dir, const Matrix<T> &data, const string &name)
    {
      cout<<"Saving... "<<name<<endl;

      Vector<Int> stokes(1);
      stokes(0) = Stokes::I;

      CoordinateSystem csys;
      csys.addCoordinate(dir);
      csys.addCoordinate(StokesCoordinate(stokes));
      csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));

      PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
      im.putSlice(data, IPosition(4, 0, 0, 0, 0));
    }


    // Utility function to store a Cube as an image for debugging. It uses arbitrary values for the
    // direction, Stokes and frequency axes. The size of the third axis is assumed to be 4.
    template <class T>
    void store(const Cube<T> &data, const string &name)
    {
        Matrix<Double> xform(2, 2);
        xform = 0.0;
        xform.diagonal() = 1.0;
        Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
        Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
        Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
        DirectionCoordinate dir(MDirection::J2000, Projection(Projection::SIN),
                         refLatLon, refLatLon, incLon, incLat,
                         xform, data.shape()(0) / 2, data.shape()(1) / 2);

        store(dir, data, name);
    }

    template <class T>
    void store(const DirectionCoordinate &dir, const Cube<T> &data, const string &name)
    {
        AlwaysAssert(data.shape()(2) == 4, SynthesisError);

        cout<<"Saving... "<<name<<endl;

        Vector<Int> stokes(4);
        stokes(0) = Stokes::XX;
        stokes(1) = Stokes::XY;
        stokes(2) = Stokes::YX;
        stokes(3) = Stokes::YY;

        CoordinateSystem csys;
        csys.addCoordinate(dir);
        csys.addCoordinate(StokesCoordinate(stokes));
        csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));

        PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)), csys, name);
        im.putSlice(data, IPosition(4, 0, 0, 0, 0));
    }

} // namespace casa

#endif
