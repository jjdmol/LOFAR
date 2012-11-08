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

#include <LofarFT/LofarATerm.h>
#include <LofarFT/LofarWTerm.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/FFTCMatrix.h>
#include <Common/Timer.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/PagedImage.h>
#include <casa/Utilities/Assert.h>
#include <casa/sstream.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/OS/PrecTimer.h>

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>






using namespace casa;

namespace LOFAR
{

  // Functions to store a 2D or 3D array in an PagedImage file.
  template <class T>
  void store(const DirectionCoordinate &dir, const Matrix<T> &data, const string &name);

  template <class T>
  void store(const DirectionCoordinate &dir, const Cube<T> &data, const string &name);

  template <class T>
  void store(const Matrix<T> &data, const string &name);

  template <class T>
  void store(const Cube<T> &data, const string &name);


  class LofarConvolutionFunction
  {

  public:
    LofarConvolutionFunction(const IPosition& shape,
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
                            );
    //,
    //			     Int TaylorTerm,
    //			     Double RefFreq);

//      ~LofarConvolutionFunction ()
//      {
//      }


    // Show the relative timings of the various steps.
    void showTimings (std::ostream&, double duration, double timeCF) const;

    // Show percentage of value in total with 1 decimal.
    static void showPerc1 (std::ostream& os, double value, double total);

    // Compute and store W-terms and A-terms in the fourier domain
    void store_all_W_images();

    // Get the spheroidal cut.
    const Matrix<Float>& getSpheroidCut();
    Matrix<Float> getSpheroid(uInt npix);

    // Get the spheroidal cut from the file.
    static Matrix<Float> getSpheroidCut (const String& imgName);

    // Get the average PB from the file.
    static Matrix<Float> getAveragePB (const String& imgName);


    // Compute the fft of the beam at the minimal resolution for all antennas,
    // and append it to a map object with a (double time) key.
    void computeAterm(Double time);
    vector<Double> VecTimesAterm;
    void computeVecAterm(Double t0, Double t1, Double dt)
    {

      Double tmax(0.);
      for(uInt i=0; i<VecTimesAterm.size(); ++i){
	if(VecTimesAterm[i]>tmax){
	  tmax=VecTimesAterm[i];
	}
      }

      if(t0>tmax){
	for(Double tat=t0+dt/2.;tat<t1; tat+=dt)
	  {
	    computeAterm(tat);
	    VecTimesAterm.push_back(tat);
	  }
      }

    }

    Double GiveClosestTimeAterm(Double tat)
    {
      Double dtmin(1e30);
      Double tmin(0.);
      Double dt;
      for(uInt ind=0; ind <VecTimesAterm.size(); ++ind)
	{
	  dt=abs(tat-VecTimesAterm[ind]);
	  if(dt<dtmin){
	    tmin=VecTimesAterm[ind];
	    dtmin=dt;
	  }
	}
      return tmin;
    }

    // Compute the convolution function for all channel, for the polarisations
    // specified in the Mueller_mask matrix
    // Also specify weither to compute the Mueller matrix for the forward or
    // the backward step. A dirty way to calculate the average beam has been
    // implemented, by specifying the beam correcting to the given baseline
    // and timeslot.
    // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]
    LofarCFStore makeConvolutionFunction(uInt stationA, uInt stationB,
                                         Double time, Double w,
                                         const Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         Matrix<Complex>& Stack_PB_CF,
                                         double& sum_weight_square,
					 Vector<uInt> ChanBlock, Int TaylorTerm, double RefFreq,
					 vector< vector < Matrix<Complex> > > & StackMuellerNew,
					 Int ImposeSupport, Bool UseWTerm);

    LofarCFStore makeConvolutionFunctionAterm(uInt stationA, uInt stationB,
                                         Double time, Double w,
                                         const Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         Matrix<Complex>& Stack_PB_CF,
                                         double& sum_weight_square,
					 uInt spw, Int TaylorTerm, double RefFreq,
					 vector< vector < Matrix<Complex> > > & StackMuellerNew,
					 Int ImposeSupport);

    Int GiveWSupport(Double w,uInt spw);
    uInt GiveWindex(Double w,uInt spw);
    Int GiveWindexIncludeNegative(Double w,uInt spw);
    void initMeanWStepsGridder();
    Int FindNWplanes();

    Array<Complex>  ApplyElementBeam(Array<Complex> input_grid, Double time, uInt spw, const Matrix<bool>& Mask_Mueller_in, bool degridding_step);
    Array<Complex> ApplyElementBeam_Image(Array<Complex>& input_grid, Double timeIn, uInt spw, const Matrix<bool>& Mask_Mueller_in2, bool degridding_step);
    Array<Complex>  ApplyElementBeam2(Array<Complex>& input_grid, Double time, uInt spw, const Matrix<bool>& Mask_Mueller_in, bool degridding_step, Int UsedMask=-1);
    Array<Complex>  ApplyElementBeam3(Array<Complex>& input_grid, Double time, uInt spw, const Matrix<bool>& Mask_Mueller_in, bool degridding_step, vector< Array<Complex> >& gridsparalel, Int UsedMask);
    Array<Complex>  ApplyWterm(Array<Complex>& input_grid, uInt spw, bool degridding_step, Int w_index, vector< Array<Complex> >& gridsparalel, Int TnumMask, Int WnumMask);
    void ApplyWterm_Image(Array<Complex>& input_grid, Array<Complex>& output_grid, uInt spw, bool degridding_step, Int w_index);
    void ConvolveArrayArrayParallel( const Array<Complex>& gridin, Array<Complex>& gridout,
				     const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel);
    void ConvolveArrayArrayParallel2( const Array<Complex>& gridin, Array<Complex>& gridout,
				      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel);
    void ConvolveArrayArrayParallel2( const Array<Complex>& gridin, Array<Complex>& gridout,
				      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<Bool> MaskIn);
    void ConvolveArrayArrayParallel3( const Array<Complex>& gridin, Array<Complex>& gridout,
				      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel);
    void ConvolveArrayArrayParallel3( const Array<Complex>& gridin, Array<Complex>& gridout,
				      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<Bool> MaskIn);
    void ConvolveArrayArrayParallel4( const Array<Complex>& gridin, Array<Complex>& gridout,
				      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<uShort> MaskIn);
    void ConvolveArrayArrayParallel4( const Array<Complex>& gridin, Array<Complex>& gridout, uInt polNum,
				      const Matrix<Complex>& ConvFunc, vector< Array<Complex> >&  GridsParallel, Matrix<uShort> MaskIn);
    void SumGridsOMP(Array<Complex>& grid, const vector< Array<Complex> >& GridToAdd0 );
    void SumGridsOMP(Array<Complex>& grid, const vector< Array<Complex> >& GridToAdd0 , uInt PolNumIn, uInt PolNumOut);
    void SumGridsOMP(Array<Complex>& grid, const Array<Complex> & GridToAdd0 , uInt PolNumIn, uInt PolNumOut);

    
    // Returns the average Primary Beam from the disk
    Matrix<float> Give_avg_pb();

    // Compute the average Primary Beam from the Stack of convolution functions
    Matrix<Float> Compute_avg_pb(Matrix<Complex> &Sum_Stack_PB_CF,
                                 double sum_weight_square);

    // Zero padding of a Cube
    Cube<Complex> zero_padding(const Cube<Complex>& Image, int Npixel_Out);

    // Zero padding of a Matrix
    Matrix<Complex> zero_padding(const Matrix<Complex>& Image, int Npixel_Out);
    Matrix<Complex> zero_padding(const Matrix<Complex>& Image, Matrix<Complex>& Image_Enlarged, bool tozero);

    

    // Get the W scale.
    const WScale& wScale() const
      { return m_wScale; }

    Float wStep()
      { return its_wStep; }
    vector<Complex> wCorrGridder()
      { return its_wCorrGridder; }

    vector<Complex> its_wCorrGridder;
    vector<Matrix< Complex > > its_wCorrGridderMatrix;
    Float its_wStep;
    Bool its_UseWSplit;

    vector< Matrix< Bool > > itsVectorMasksDegridElement;
    vector< vector< Matrix< Bool > > > itsVecMasks;
    vector< vector< Matrix< uShort > > > itsVecMasksNew;
    vector< vector< Matrix< uShort > > > itsVecMasksNewW;
    vector< Matrix< uShort > > itsVecMasksNewElement;
    uInt NBigChunks;
    void initStoreMasks()
    {
      NBigChunks=20;
      Int sizeVec(2*m_nWPlanes);
      itsVecMasks.resize(NBigChunks);
      for(uInt i=0; i<NBigChunks; ++i)
	{
	  itsVecMasks[i].resize(sizeVec);
	}
    }
    void initStoreMasksNew()
    {
      NBigChunks=20;
      Int sizeVec(2*m_nWPlanes);
      itsVecMasksNew.resize(NBigChunks);
      itsVecMasksNewW.resize(NBigChunks);
      itsVecMasksNewElement.resize(0);
      for(uInt i=0; i<NBigChunks; ++i)
	{
	  itsVecMasksNew[i].resize(sizeVec);
	  itsVecMasksNewW[i].resize(sizeVec);
	}
    }

    void MakeMaskDegrid( const Array<Complex>& gridin, Int NumMask)
    {

      String MaskName("JAWS_products/Mask"+String::toString(NumMask)+".boolim");
      File MaskFile(MaskName);
      if(!MaskFile.exists()){
	//cout<<"... Making Masks ..."<<endl;
	Matrix<Bool> Mask(IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	Matrix<Int> IntMask(IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	int GridSize(gridin.shape()[0]);
	const Complex* inPtr = gridin.data();
	Bool* outPtr = Mask.data();
	for (uInt i=0; i<GridSize; ++i) {
	  for (uInt j=0; j<GridSize; ++j) {
	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	      (*(outPtr)) = true;
	    }
	    inPtr++;
	    outPtr++;
	  }
	}
	//itsVectorMasksDegridElement.push_back(Mask);
	
	store(Mask,MaskName);
	//cout<<"... Done Making Masks ..."<<endl;
      }
    }

    
    void MakeVectorMaskWplanes( const Array<Complex>& gridin, Int NumTime, Int NumWplane)
    {
      String MaskName("JAWS_products/Mask.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim");
      File MaskFile(MaskName);


      if(!MaskFile.exists()){
	//cout<<"... Making Masks ..."<<endl;
	Matrix<Bool> Mask(IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	Matrix<Int> IntMask(IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	int GridSize(gridin.shape()[0]);
	const Complex* inPtr = gridin.data();
	Bool* outPtr = Mask.data();
	for (uInt i=0; i<GridSize; ++i) {
	  for (uInt j=0; j<GridSize; ++j) {
	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	      (*(outPtr)) = true;
	    }
	    inPtr++;
	    outPtr++;
	  }
	}
	//itsVectorMasksDegridElement.push_back(Mask);
	store(Mask,MaskName);
	//cout<<"... Done Making Masks ..."<<endl;
      }
    }

    void MakeVectorMaskWplanesNew( const Array<Complex>& gridin, Int NumTime, Int NumWplane, Bool grid, Bool Element, Int MaskType)
    {
      //cout<<"make mask "<<grid<<endl;
      String MaskName;
      File MaskFile;
      if(MaskType==0){
	MaskName="JAWS_products/MaskGrid.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim";
	File MaskFilein(MaskName);
	MaskFile=MaskFilein;
	} 
      if(MaskType==1){
	MaskName="JAWS_products/MaskDeGrid.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim";
	File MaskFilein(MaskName);
	MaskFile=MaskFilein;
      }
      if(MaskType==2){
	MaskName="JAWS_products/MaskGrid.Element.T"+String::toString(NumTime)+".boolim";
	File MaskFilein(MaskName);
	MaskFile=MaskFilein;
      }

      if(!MaskFile.exists()){
    	Matrix<Int> IntMask(IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
    	int GridSize(gridin.shape()[0]);
    	const Complex* inPtr = gridin.data();
    	uInt Nnonzero(0);
    	for (uInt i=0; i<GridSize; ++i) {
    	  for (uInt j=0; j<GridSize; ++j) {
    	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
    	      Nnonzero+=1;
	    }
    	    inPtr++;
    	  }
	}
    	inPtr = gridin.data();
	Nnonzero=std::max(1,int(Nnonzero));
    	Matrix<uShort> Mask(IPosition(2,Nnonzero,2));
	Mask=0;
	uInt indec(0);
	uShort* outPtr = Mask.data();
    	for (uInt i=0; i<GridSize; ++i) {
    	  for (uInt j=0; j<GridSize; ++j) {
    	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	      Mask(indec,0)=i;
	      Mask(indec,1)=j;
	      indec+=1;
	    }
    	    inPtr++;
    	  }
    	}
    	//itsVectorMasksDegridElement.push_back(Mask);
    	store(Mask,MaskName);
	if(MaskType==0){
	  itsVecMasksNew[NumTime][NumWplane+m_nWPlanes].reference (Mask);
	}
	if(MaskType==1){
	  itsVecMasksNewW[NumTime][NumWplane+m_nWPlanes].reference (Mask);
	}
	if(MaskType==2){
	  itsVecMasksNewElement.push_back(Mask);
	}
	
    	//cout<<"... Done Making Masks ... t="<<NumTime<<" w="<<NumWplane+m_nWPlanes<<" npix="<<Nnonzero<<endl;
      }
    }


    void Make_MuellerAvgPB(vector< vector< vector < Matrix<Complex> > > > & StackMueller, double sum_weight_square);

    Array< Complex > Correct_CC(Array< Complex > & ModelImage);


    Bool itsFilledVectorMasks;
      //vector< Matrix< Bool > > itsVectorMasksDegridElement;
    void ReadMaskDegrid()
    {
      Int NumMask(0);
      while(true){
	String MaskName("JAWS_products/Mask"+String::toString(NumMask)+".boolim");
	File MaskFile(MaskName);
	if(MaskFile.exists())
	  {
	    //cout<<"Reading:"<<MaskName<<endl;
	    PagedImage<Bool> pim(MaskName);
	    Array<Bool> arr = pim.get();
	    Matrix<Bool> Mask;
	    Mask.reference (arr.nonDegenerate(2));
	    itsVectorMasksDegridElement.push_back(Mask);
	    NumMask+=1;
	  }
	else
	  {
	    break;
	  }
      }
      itsFilledVectorMasks=true;
      
    }
      
    void ReadMaskDegridW()
    {
      initStoreMasks();
      Int NumMask(0);
      Int Wc(0);
      for(Int Tnum=0;Tnum<NBigChunks;++Tnum){
	for(Int Wnum=0;Wnum<2*m_nWPlanes;++Wnum){
	
	  Int Wsearch(Wnum-m_nWPlanes);
	  String MaskName("JAWS_products/Mask.T"+String::toString(Tnum)+".W"+String::toString(Wsearch)+".boolim");
	  File MaskFile(MaskName);
	  if(MaskFile.exists())
	    {
	      PagedImage<Bool> pim(MaskName);
	      Array<Bool> arr = pim.get();
	      itsVecMasks[Tnum][Wnum].reference (arr.nonDegenerate(2));
	      //cout<<"  ... read t="<<Tnum<<" w="<<Wsearch<<" put at:"<<Wnum<<endl;
	      Wc+=1;
	    }
	}
      }
      itsFilledVectorMasks=true;
      
    }

    void ReadMaskDegridWNew()
    {
      //cout<<"...reading masks degrid"<<endl;
      Int NumMask(0);
      Int Wc(0);
      initStoreMasksNew();

      /* MaskName="JAWS_products/MaskGrid.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim"; */
      /* MaskName="JAWS_products/MaskDeGrid.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim"; */
      /* MaskName="JAWS_products/MaskGrid.Element.T"+String::toString(NumTime)+".boolim"; */
      /* vector< vector< Matrix< uInt > > > itsVecMasksNew; */
      /* vector< vector< Matrix< uInt > > > itsVecMasksNewW; */
      /* vector< Matrix< uInt > > itsVecMasksNewElement; */

      for(Int Tnum=0;Tnum<NBigChunks;++Tnum){
	for(Int Wnum=0;Wnum<2*m_nWPlanes;++Wnum){
	
	  Int Wsearch(Wnum-m_nWPlanes);
	  String MaskName("JAWS_products/MaskGrid.T"+String::toString(Tnum)+".W"+String::toString(Wsearch)+".boolim");
	  File MaskFile(MaskName);
	  if(MaskFile.exists())
	    {
	      //cout<<".. reading "<<MaskName<<endl;
	      PagedImage<uShort> pim(MaskName);
	      Array<uShort> arr = pim.get();
	      itsVecMasksNew[Tnum][Wnum].reference (arr.nonDegenerate(2));
	      Wc+=1;
	    }
	  String MaskName2("JAWS_products/MaskDeGrid.T"+String::toString(Tnum)+".W"+String::toString(Wsearch)+".boolim");
	  File MaskFile2(MaskName);
	  if(MaskFile2.exists())
	    {
	      //cout<<".. reading "<<MaskName2<<endl;
	      PagedImage<uShort> pim(MaskName2);
	      Array<uShort> arr = pim.get();
	      itsVecMasksNewW[Tnum][Wnum].reference (arr.nonDegenerate(2));
	      Wc+=1;
	    }
	}
	String MaskName("JAWS_products/MaskGrid.Element.T"+String::toString(Tnum)+".boolim");
	File MaskFile(MaskName);
	if(MaskFile.exists())
	  {
	    //cout<<".. reading "<<MaskName<<endl;
	    PagedImage<uShort> pim(MaskName);
	    Array<uShort> arr = pim.get();
	    Matrix<uShort> Mask;
	    Mask.reference(arr.nonDegenerate(2));
	    itsVecMasksNewElement.push_back(Mask);
	  }

      }
      //cout<<"... deon reading masks degrid"<<endl;
      itsFilledVectorMasks=true;
      
    }
      
      Bool VectorMaskIsFilled(){return itsFilledVectorMasks;}
    void normalized_fft (Matrix<Complex>&, bool toFreq=true);
    void normalized_fft_parallel(Matrix<Complex> &im, bool toFreq=true);
    void normalized_fft (PrecTimer& timer, Matrix<Complex>&, bool toFreq=true);

    Vector< Double >    list_freq_spw;
    Vector< Double >    list_freq_chanBlock;
    Vector< uInt >      map_chan_chanBlock;
    Vector< uInt >      map_chanBlock_spw;
    vector<Vector< uInt > >     map_spw_chanBlock;
    Vector< uInt > map_chan_Block_buffer;




    uInt                m_nWPlanes;
  private:

    Matrix<Complex> give_normalized_fft_lapack(const Matrix<Complex> &im, bool toFreq=true)
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

    MEpoch observationStartTime (const MeasurementSet &ms,
                                 uInt idObservation) const;

    // Estime spheroidal convolution function from the support of the fft
    // of the spheroidal in the image plane
    Double makeSpheroidCut();

    // Return the angular resolution required for making the image of the
    // angular size determined by coordinates and shape.
    // The resolution is assumed to be the same on both direction axes.
    Double estimateWResolution(const IPosition &shape,
                               Double pixelSize,
                               Double w) const;


    // Return the angular resolution required for making the image of the
    // angular size determined by coordinates and shape.
    // The resolution is assumed to be the same on both direction axes.
    Double estimateAResolution(const IPosition &shape,
                               const DirectionCoordinate &coordinates, double station_diam = 70.) const;

    // Apply a spheroidal taper to the input function.
    template <typename T>
    void taper (Matrix<T> &function) const
    {
      AlwaysAssert(function.shape()[0] == function.shape()[1], SynthesisError);
      uInt size = function.shape()[0];
      Double halfSize = (size-1) / 2.0;
      Vector<Double> x(size);
      for (uInt i=0; i<size; ++i) {
        x[i] = spheroidal(abs(i - halfSize) / halfSize);
      }
      for (uInt i=0; i<size; ++i) {
        for (uInt j=0; j<size; ++j) {
          function(j, i) *= x[i] * x[j];
        }
      }
    }

    template <typename T>
    void taper_parallel (Matrix<T> &function) const
    {
      AlwaysAssert(function.shape()[0] == function.shape()[1], SynthesisError);
      uInt size = function.shape()[0];
      Double halfSize = (size-1) / 2.0;
      Vector<Double> x(size);
      for (uInt i=0; i<size; ++i) {
        x[i] = spheroidal(abs(i - halfSize) / halfSize);
      }
      uInt j;
#pragma omp parallel
    {
#pragma omp for private(j) schedule(dynamic)
      for (uInt i=0; i<size; ++i) {
        for (j=0; j<size; ++j) {
          function(j, i) *= x[i] * x[j];
        }
      }
    }
    }


    // Linear interpolation
    template <typename T>
    Matrix< T > LinearInterpol(Matrix<T> ImageIn, Int  NpixOut)
      {
	Matrix<T> ImageOut(IPosition(2,NpixOut,NpixOut),0.);
	float d0(1./(NpixOut-1.));
	float d1(1./(ImageIn.shape()[0]-1.));
	float dd(d0/d1);
	float dx,dy,dxd,dyd,xin,yin;
	float onef(1.);
	uInt NpixOutm(NpixOut-1);
	for(uInt i=0;i<(NpixOut);++i){
	  dxd=i*dd;
	  xin=floor(dxd);
	  dx=dxd-xin;
	  for(uInt j=0;j<(NpixOut);++j){
	    dyd=j*dd;
	    yin=floor(dyd);
	    dy=dyd-yin;
	    ImageOut(i,j)=(onef-dx)*(onef-dy)*ImageIn(xin,yin) + (onef-dx)*(dy)*ImageIn(xin,yin+1) + (dx)*(onef-dy)*ImageIn(xin+1,yin) + (dx)*(dy)*ImageIn(xin+1,yin+1);
	  }
	}
	return ImageOut;
      }

    void Convolve(Matrix<Complex> gridin, Matrix<Complex> gridout, Matrix<Complex> ConvFunc){
      Int Support(ConvFunc.shape()[0]);
      Int GridSize(gridin.shape()[0]);
      Int off(Support/2);
      for(uInt i=Support/2;i<GridSize-Support/2;++i){
	for(uInt j=Support/2;j<GridSize-Support/2;++j){
	  if((gridin(i,j))!=Complex(0.,0.)){
	    Complex val(gridin(i,j));
	    for(uInt ii=0;ii<Support;++ii){
	      for(uInt jj=0;jj<Support;++jj){
		gridout(i-off+ii,j-off+jj)+=ConvFunc(ii,jj)*val;
	      }
	    }
	  }
	}
      }
    }

    void ConvolveOpt(Matrix<Complex> gridin, Matrix<Complex> gridout, Matrix<Complex> ConvFunc){
      Int Support(ConvFunc.shape()[0]);
      Int GridSize(gridin.shape()[0]);
      Int off(Support/2);

      Complex* __restrict__ gridInPtr = gridin.data();
      Complex* __restrict__ gridOutPtr = gridout.data();
      Complex* __restrict__ ConvFuncPtr = ConvFunc.data();

      for(uInt i=Support/2;i<GridSize-Support/2;++i){
	for(uInt j=Support/2;j<GridSize-Support/2;++j){
	  gridInPtr=gridin.data()+GridSize*i+j;
	  if (gridInPtr->real() != 0  ||  gridInPtr->imag() != 0) {//if((*gridInPtr)!=Complex(0.,0.)){
	    ConvFuncPtr = ConvFunc.data();
	    for(uInt jj=0;jj<Support;++jj){
	      for(uInt ii=0;ii<Support;++ii){
		gridOutPtr = gridout.data()+(j-off+jj)*GridSize+i-off+ii;
		(*gridOutPtr) += (*ConvFuncPtr)*(*gridInPtr);
		ConvFuncPtr++;//=ConvFunc.data()+Support*ii+jj;
	      }
	    }
	  }
	  //gridInPtr++;
	}
      }
      
    }

    void ConvolveGer( const Matrix<Complex>& gridin, Matrix<Complex>& gridout,
		      const Matrix<Complex>& ConvFunc)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);
      const Complex* inPtr = gridin.data() + off*GridSize + off;
      for (uInt i=0; i<GridSize-Support; ++i) {
	for (uInt j=0; j<GridSize-Support; ++j) {
	  if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	    const Complex* cfPtr = ConvFunc.data();
	    for (uInt ii=0; ii<Support; ++ii) {
	      Complex* outPtr = gridout.data() + (i+ii)*GridSize + j;
	      for (uInt jj=0; jj<Support; ++jj) {
		outPtr[jj] += *cfPtr++ * *inPtr;
	      }
	    }
	  }
	  inPtr++;
	}
	inPtr += Support;
      }
    }

    void ConvolveGerArray( const Array<Complex>& gridin, Int ConvPol, Matrix<Complex>& gridout,
			   const Matrix<Complex>& ConvFunc)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
      for (uInt i=0; i<GridSize-Support; ++i) {
	for (uInt j=0; j<GridSize-Support; ++j) {
	  if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	    const Complex* cfPtr = ConvFunc.data();
	    for (uInt ii=0; ii<Support; ++ii) {
	      Complex* outPtr = gridout.data() + (i+ii)*GridSize + j;
	      for (uInt jj=0; jj<Support; ++jj) {
		outPtr[jj] += *cfPtr++ * *inPtr;
	      }
	    }
	  }
	  inPtr++;
	  }
	inPtr += Support;
      }
    }
    
    void ConvolveArrayArray( const Array<Complex>& gridin, Array<Complex>& gridout,
			   const Matrix<Complex>& ConvFunc)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);



      for(uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	Int offPol(ConvPol*GridSize*GridSize);
	const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	for (uInt i=0; i<GridSize-Support; ++i) {
	  for (uInt j=0; j<GridSize-Support; ++j) {
	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	      const Complex* cfPtr = ConvFunc.data();
	      for (uInt ii=0; ii<Support; ++ii) {
		Complex* outPtr = gridout.data() + (i+ii)*GridSize + j +offPol;
		for (uInt jj=0; jj<Support; ++jj) {
		  outPtr[jj] += *cfPtr++ * *inPtr;
		}
	      }
	    }
	    inPtr++;
	  }
	  inPtr += Support;
	}

      }
    }
    


    void ConvolveGerArrayMask( const Array<Complex>& gridin, Int ConvPol, Matrix<Complex>& gridout,
			       const Matrix<Complex>& ConvFunc, Int UsedMask)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);

      const Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
      const Bool* MaskPtr = itsVectorMasksDegridElement[UsedMask].data() + off*GridSize + off;
      for (uInt i=0; i<GridSize-Support; ++i) {
	for (uInt j=0; j<GridSize-Support; ++j) {
	  if ((*MaskPtr)==true) {
	    const Complex* cfPtr = ConvFunc.data();
	    for (uInt ii=0; ii<Support; ++ii) {
	      Complex* outPtr = gridout.data() + (i+ii)*GridSize + j;
	      for (uInt jj=0; jj<Support; ++jj) {
		outPtr[jj] += *cfPtr++ * *inPtr;
	      }
	    }
	  }
	  MaskPtr++;
	  inPtr++;
	}
	inPtr += Support;
	MaskPtr += Support;
      }
    }
    
    
    
    // Linear interpolation
    template <typename T>
    Matrix< T > LinearInterpol2(Matrix<T> ImageIn, Int  NpixOut)
      {
	Matrix<T> ImageOut(IPosition(2,NpixOut,NpixOut),1e-7);
	int nd(ImageIn.shape()[0]);
	int ni(NpixOut);
	float off(-.5);//-(((1.+1./(nd-1.))-1.)/2.)*(nd-1));
	float a(nd/(ni-1.));//((1.+1./(nd-1.))/(ni-1.))*(nd-1));
	float dx,dy,dxd,dyd,xin,yin;
	float onef(1.);
	uInt NpixOutm(NpixOut-1);
	for(uInt i=0;i<(NpixOut);++i){
	  dxd=i*a+off;
	  xin=floor(dxd);
	  dx=dxd-xin;
	  for(uInt j=0;j<(NpixOut);++j){
	    dyd=j*a+off;
	    yin=floor(dyd);
	    dy=dyd-yin;
	    if((dxd<0)||((xin+1)>ImageIn.shape()[0]-1.)){continue;}
	    if((dyd<0)||((yin+1)>ImageIn.shape()[0]-1.)){continue;}
	    ImageOut(i,j)=(onef-dx)*(onef-dy)*ImageIn(xin,yin) + (onef-dx)*(dy)*ImageIn(xin,yin+1) + (dx)*(onef-dy)*ImageIn(xin+1,yin) + (dx)*(dy)*ImageIn(xin+1,yin+1);
	  }
	}
	/* store(ImageIn,"ImageIn.img"); */
	/* store(ImageOut,"ImageOut.img"); */
	/* assert(false); */
	return ImageOut;
      }

    void EstimateCoordShape(IPosition shape, DirectionCoordinate coordinate, double station_diameter=70.){
      coordinate = m_coordinates;
      Double aPixelAngSize = min(m_pixelSizeSpheroidal,
				 estimateAResolution(m_shape, m_coordinates, station_diameter));
      
      Double pixelSize = abs(m_coordinates.increment()[0]);
      Double imageDiameter = pixelSize * m_shape(0);
      Int nPixelsConv = imageDiameter / aPixelAngSize;
      if (nPixelsConv > itsMaxSupport) {
          nPixelsConv = itsMaxSupport;
      }
      // Make odd and optimal.
      nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
      aPixelAngSize = imageDiameter / nPixelsConv;

      shape=IPosition(2, nPixelsConv, nPixelsConv);
      Vector<Double> increment_old(coordinate.increment());
      Vector<Double> increment(2);
      increment[0] = aPixelAngSize*sign(increment_old[0]);
      increment[1] = aPixelAngSize*sign(increment_old[1]);
      coordinate.setIncrement(increment);
      Vector<Double> refpix(2, 0.5*(nPixelsConv-1));
      coordinate.setReferencePixel(refpix);
    }




    Double spheroidal(Double nu) const;






    template <typename T>
    uInt findSupport(Matrix<T> &function, Double threshold) const
    {
      ///      Double peak = abs(max(abs(function)));
      Double peak = max(amplitude(function));
      threshold *= peak;
      uInt halfSize = function.shape()[0] / 2;
      uInt x = 0;
      while (x < halfSize && abs(function(x, halfSize)) < threshold) {
        ++x;
      }
      return 2 * (halfSize - x);
    }


    //# Data members.
    casa::Record       itsParameters;
    Array<Complex> its_output_grid_element;
    Matrix<Complex> its_ArrMatrix_out_element;
    IPosition           m_shape;
    DirectionCoordinate m_coordinates;
    WScale              m_wScale;
    LofarWTerm          m_wTerm;
    LofarATerm          m_aTerm;
    Double              m_maxW;
    Double              m_pixelSizeSpheroidal;
    uInt                m_nStations;
    uInt                m_oversampling;
    uInt                m_nChannelBlocks;
    uInt                m_NPixATerm;
    Double              m_refFrequency;
    uInt                m_maxCFSupport;
    vector<Double>      its_VectorThreadsSumWeights;

    //# Stack of the convolution functions for the average PB calculation
    Matrix<Complex>     Spheroid_cut;
    //# Stack of the convolution functions for the average PB calculation
    Matrix<Float>       Spheroid_cut_im;
    Matrix<Float>       Spheroid_cut_im_element;
    //# List of the ferquencies the CF have to be caluclated for
    vector< Matrix<Complex> > m_WplanesStore;
    //# Aterm_store[double time][antenna][channel]=Cube[Npix,Npix,4]
    map<Double, vector< vector< Cube<Complex> > > > m_AtermStore;
    map<Double, vector< vector< Cube<Complex> > > > m_AtermStore_element;
    map<Double, vector< vector< Cube<Complex> > > > m_AtermStore_station;
    //# Average primary beam
    Matrix<Float>       Im_Stack_PB_CF0;
    Int                 itsVerbose;
    Int                 itsMaxSupport;
    //    Int                 itsTaylorTerm;
    //Double              itsRefFreq;
    String              itsImgName;
    vector<FFTCMatrix>  itsFFTMachines;
    Double              itsTimeW;
    Double              itsTimeWpar;
    Double              itsTimeWfft;
    unsigned long long  itsTimeWcnt;
    Double              itsTimeA;
    Double              itsTimeApar;
    Double              itsTimeAfft;
    unsigned long long  itsTimeAcnt;
    Double              itsTimeCF;
    Double              itsTimeCFpar;
    Double              itsTimeCFfft;
    unsigned long long  itsTimeCFcnt;
    Bool                its_Use_EJones;
    Bool                its_Apply_Element;
    Bool                its_NotApplyElement;
    Bool                its_NotApplyArray;
    uInt                its_MaxWSupport;
    uInt                its_count_time;
    mutable LogIO       m_logIO;
    Int                 its_ChanBlockSize;
    Matrix<Complex>     spheroid_cut_element_fft;
    vector< vector< Matrix< Complex > > > GridsMueller;
    LogIO &logIO() const
      {
        return m_logIO;
      }
  };



  //# =================================================
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
  void store (const DirectionCoordinate &dir, const Matrix<T> &data,
              const string &name)
  {
    //cout<<"Saving... "<<name<<endl;
    Vector<Int> stokes(1);
    stokes(0) = Stokes::I;
    CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
  }

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
  void store(const DirectionCoordinate &dir, const Cube<T> &data,
             const string &name)
  {
    AlwaysAssert(data.shape()(2) == 4, SynthesisError);
    //cout<<"Saving... "<<name<<endl;
    Vector<Int> stokes(4);
    stokes(0) = Stokes::XX;
    stokes(1) = Stokes::XY;
    stokes(2) = Stokes::YX;
    stokes(3) = Stokes::YY;
    CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    PagedImage<T>
      im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)),
         csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
  }

} //# end namespace casa

#endif
