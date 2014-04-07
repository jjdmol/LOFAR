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

namespace LOFAR
{

  // Functions to store a 2D or 3D array in an PagedImage file.
  template <class T>
  void store(const casa::DirectionCoordinate &dir, const casa::Matrix<T> &data, const string &name);

  template <class T>
  void store(const casa::DirectionCoordinate &dir, const casa::Cube<T> &data, const string &name);

  template <class T>
  void store(const casa::Matrix<T> &data, const string &name);

  template <class T>
  void store(const casa::Cube<T> &data, const string &name);


  class LofarConvolutionFunction
  {

  public:
    LofarConvolutionFunction(const casa::IPosition& shape,    //# padded shape
                             const casa::IPosition& imageShape,
                             const casa::DirectionCoordinate& coordinates,
                             const casa::MeasurementSet& ms,
                             casa::uInt nW, 
                             double Wmax,
                             casa::uInt oversample,
                             casa::Int verbose,
                             casa::Int maxsupport,
                             const casa::String& imgName,
			     casa::Bool Use_EJones,
			     casa::Bool Apply_Element,
			     int ApplyBeamCode,
                             const casa::Record& parameters,
			     vector< vector< vector < casa::Matrix<casa::Complex> > > > & StackMuellerNew
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
    const casa::Matrix<casa::Float>& getSpheroidCut();
    casa::Matrix<casa::Float> getSpheroid(casa::uInt npix);

    // Get the spheroidal cut from the file.
    static casa::Matrix<casa::Float> getSpheroidCut (const casa::String& imgName);

    // Get the average PB from the file.
    static casa::Matrix<casa::Float> getAveragePB (const casa::String& imgName);


    // Compute the fft of the beam at the minimal resolution for all antennas,
    // and append it to a map object with a (double time) key.
    void computeAterm(casa::Double time);
    
    vector<casa::Double> VecTimesAterm;
    
    void computeVecAterm(casa::Double t0, casa::Double t1, casa::Double dt)
    {

      casa::Double tmax(0.);
      for(casa::uInt i=0; i<VecTimesAterm.size(); ++i){
	if(VecTimesAterm[i]>tmax){
	  tmax=VecTimesAterm[i];
	}
      }

      if(t0>tmax){
	double d = std::min(dt, t1-t0);
	for(casa::Double tat=t0+d/2.;tat<t1; tat+=d)
	  {
	    computeAterm(tat);
	    VecTimesAterm.push_back(tat);
	  }
      }

    }

    casa::Double GiveClosestTimeAterm(casa::Double tat)
    {
      casa::Double dtmin(1e30);
      casa::Double tmin(0.);
      casa::Double dt;
      for(casa::uInt ind=0; ind <VecTimesAterm.size(); ++ind)
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
    LofarCFStore makeConvolutionFunction(casa::uInt stationA, casa::uInt stationB,
                                         casa::Double time, casa::Double w,
                                         const casa::Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         casa::Matrix<casa::Complex>& Stack_PB_CF,
                                         double& sum_weight_square,
					 casa::Vector<casa::uInt> ChanBlock, casa::Int TaylorTerm, double RefFreq,
					 vector< vector < casa::Matrix<casa::Complex> > > & StackMuellerNew,
					 casa::Int ImposeSupport, casa::Bool UseWTerm);

    LofarCFStore makeConvolutionFunctionAterm(casa::uInt stationA, casa::uInt stationB,
                                         casa::Double time, casa::Double w,
                                         const casa::Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         casa::Matrix<casa::Complex>& Stack_PB_CF,
                                         double& sum_weight_square,
					 casa::uInt spw, casa::Int TaylorTerm, double RefFreq,
					 vector< vector < casa::Matrix<casa::Complex> > > & StackMuellerNew,
					 casa::Int ImposeSupport);

    casa::Int GiveWSupport(casa::Double w, casa::uInt spw);
    casa::uInt GiveWindex(casa::Double w, casa::uInt spw);
    casa::Int GiveWindexIncludeNegative(casa::Double w, casa::uInt spw);
    void initMeanWStepsGridder();
    casa::Int FindNWplanes();

    casa::Array<casa::Complex>  ApplyElementBeam(casa::Array<casa::Complex> input_grid, casa::Double time, casa::uInt spw, const casa::Matrix<bool>& Mask_Mueller_in, bool degridding_step);
    casa::Array<casa::Complex> ApplyElementBeam_Image(casa::Array<casa::Complex>& input_grid, casa::Double timeIn, casa::uInt spw, const casa::Matrix<bool>& Mask_Mueller_in2, bool degridding_step);
    casa::Array<casa::Complex>  ApplyElementBeam2(casa::Array<casa::Complex>& input_grid, casa::Double time, casa::uInt spw, const casa::Matrix<bool>& Mask_Mueller_in, bool degridding_step, casa::Int UsedMask=-1);
    casa::Array<casa::Complex>  ApplyElementBeam3(casa::Array<casa::Complex>& input_grid, casa::Double time, casa::uInt spw, const casa::Matrix<bool>& Mask_Mueller_in, bool degridding_step, vector< casa::Array<casa::Complex> >& gridsparalel, casa::Int UsedMask);
    casa::Array<casa::Complex>  ApplyWterm(casa::Array<casa::Complex>& input_grid, casa::uInt spw, bool degridding_step, casa::Int w_index, vector< casa::Array<casa::Complex> >& gridsparalel, casa::Int TnumMask, casa::Int WnumMask);
    void ApplyWterm_Image(casa::Array<casa::Complex>& input_grid, casa::Array<casa::Complex>& output_grid, casa::uInt spw, bool degridding_step, casa::Int w_index);
    void ConvolveArrayArrayParallel( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
				     const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel);
    void ConvolveArrayArrayParallel2( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
				      const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel);
    void ConvolveArrayArrayParallel2( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
				      const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel, casa::Matrix<casa::Bool> MaskIn);
    void ConvolveArrayArrayParallel3( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
				      const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel);
    void ConvolveArrayArrayParallel3( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
				      const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel, casa::Matrix<casa::Bool> MaskIn);
    void ConvolveArrayArrayParallel4( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
				      const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel, casa::Matrix<casa::uShort> MaskIn);
    void ConvolveArrayArrayParallel4( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout, casa::uInt polNum,
				      const casa::Matrix<casa::Complex>& ConvFunc, vector< casa::Array<casa::Complex> >&  GridsParallel, casa::Matrix<casa::uShort> MaskIn);
    void SumGridsOMP(casa::Array<casa::Complex>& grid, const vector< casa::Array<casa::Complex> >& GridToAdd0 );
    void SumGridsOMP(casa::Array<casa::Complex>& grid, const vector< casa::Array<casa::Complex> >& GridToAdd0 , casa::uInt PolNumIn, casa::uInt PolNumOut);
    void SumGridsOMP(casa::Array<casa::Complex>& grid, const casa::Array<casa::Complex> & GridToAdd0 , casa::uInt PolNumIn, casa::uInt PolNumOut);

    
    // Returns the average Primary Beam from the disk
    casa::Matrix<float> Give_avg_pb();

    // Compute the average Primary Beam from the Stack of convolution functions
    casa::Matrix<casa::Float> Compute_avg_pb(casa::Matrix<casa::Complex> &Sum_Stack_PB_CF,
                                 double sum_weight_square);

    // Zero padding of a Cube
    casa::Cube<casa::Complex> zero_padding(const casa::Cube<casa::Complex>& Image, int Npixel_Out);

    // Zero padding of a Matrix
    casa::Matrix<casa::Complex> zero_padding(const casa::Matrix<casa::Complex>& Image, int Npixel_Out);
    casa::Matrix<casa::Complex> zero_padding(const casa::Matrix<casa::Complex>& Image, casa::Matrix<casa::Complex>& Image_Enlarged, bool tozero);

    

    // Get the W scale.
    const WScale& wScale() const
      { return m_wScale; }

    casa::Float wStep()
      { return its_wStep; }
    vector<casa::Complex> wCorrGridder()
      { return its_wCorrGridder; }

    vector<casa::Complex> its_wCorrGridder;
    vector<casa::Matrix< casa::Complex > > its_wCorrGridderMatrix;
    casa::Float its_wStep;
    casa::Bool its_UseWSplit;

    vector< casa::Matrix< casa::Bool > > itsVectorMasksDegridElement;
    vector< vector< casa::Matrix< casa::Bool > > > itsVecMasks;
    vector< vector< casa::Matrix< casa::uShort > > > itsVecMasksNew;
    vector< vector< casa::Matrix< casa::uShort > > > itsVecMasksNewW;
    vector< casa::Matrix< casa::uShort > > itsVecMasksNewElement;
    casa::uInt NBigChunks;
    
    void initStoreMasks()
    {
      NBigChunks=20;
      casa::Int sizeVec(2*m_nWPlanes);
      itsVecMasks.resize(NBigChunks);
      for(casa::uInt i=0; i<NBigChunks; ++i)
	{
	  itsVecMasks[i].resize(sizeVec);
	}
    }
    
    void initStoreMasksNew()
    {
      NBigChunks=20;
      casa::Int sizeVec(2*m_nWPlanes);
      itsVecMasksNew.resize(NBigChunks);
      itsVecMasksNewW.resize(NBigChunks);
      itsVecMasksNewElement.resize(0);
      for(casa::uInt i=0; i<NBigChunks; ++i)
	{
	  itsVecMasksNew[i].resize(sizeVec);
	  itsVecMasksNewW[i].resize(sizeVec);
	}
    }

    void MakeMaskDegrid( const casa::Array<casa::Complex>& gridin, casa::Int NumMask)
    {

      casa::String MaskName("JAWS_products/Mask" + casa::String::toString(NumMask) + ".boolim");
      casa::File MaskFile(MaskName);
      if(!MaskFile.exists()){
	//cout<<"... Making Masks ..."<<endl;
	casa::Matrix<casa::Bool> Mask(casa::IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	casa::Matrix<casa::Int> IntMask(casa::IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	int GridSize(gridin.shape()[0]);
	const casa::Complex* inPtr = gridin.data();
	casa::Bool* outPtr = Mask.data();
	for (int i=0; i<GridSize; ++i) {
	  for (int j=0; j<GridSize; ++j) {
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

    
    void MakeVectorMaskWplanes( const casa::Array<casa::Complex>& gridin, casa::Int NumTime, casa::Int NumWplane)
    {
      casa::String MaskName("JAWS_products/Mask.T" + casa::String::toString(NumTime) + ".W" + casa::String::toString(NumWplane) + ".boolim");
      casa::File MaskFile(MaskName);


      if(!MaskFile.exists()){
	//cout<<"... Making Masks ..."<<endl;
	casa::Matrix<casa::Bool> Mask(casa::IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	casa::Matrix<casa::Int> IntMask(casa::IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
	int GridSize(gridin.shape()[0]);
	const casa::Complex* inPtr = gridin.data();
	casa::Bool* outPtr = Mask.data();
	for (int i=0; i<GridSize; ++i) {
	  for (int j=0; j<GridSize; ++j) {
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

    void MakeVectorMaskWplanesNew( const casa::Array<casa::Complex>& gridin, casa::Int NumTime, casa::Int NumWplane, casa::Bool /*grid*/, casa::Bool /*Element*/, casa::Int MaskType)
    {
      //cout<<"make mask "<<grid<<endl;
      casa::String MaskName;
      casa::File MaskFile;
      if(MaskType==0){
	MaskName="JAWS_products/MaskGrid.T"+casa::String::toString(NumTime)+".W"+casa::String::toString(NumWplane)+".boolim";
	casa::File MaskFilein(MaskName);
	MaskFile=MaskFilein;
	} 
      if(MaskType==1){
	MaskName="JAWS_products/MaskDeGrid.T"+casa::String::toString(NumTime)+".W"+casa::String::toString(NumWplane)+".boolim";
	casa::File MaskFilein(MaskName);
	MaskFile=MaskFilein;
      }
      if(MaskType==2){
	MaskName="JAWS_products/MaskGrid.Element.T"+casa::String::toString(NumTime)+".boolim";
	casa::File MaskFilein(MaskName);
	MaskFile=MaskFilein;
      }

      if(!MaskFile.exists()){
    	casa::Matrix<casa::Int> IntMask(casa::IPosition(2,gridin.shape()[0],gridin.shape()[0]),false);
    	int GridSize(gridin.shape()[0]);
    	const casa::Complex* inPtr = gridin.data();
    	casa::uInt Nnonzero(0);
    	for (int i=0; i<GridSize; ++i) {
    	  for (int j=0; j<GridSize; ++j) {
    	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
    	      Nnonzero+=1;
	    }
    	    inPtr++;
    	  }
	}
    	inPtr = gridin.data();
	Nnonzero=std::max(1,int(Nnonzero));
    	casa::Matrix<casa::uShort> Mask(casa::IPosition(2,Nnonzero,2));
	Mask=0;
	casa::uInt indec(0);
    	for (int i=0; i<GridSize; ++i) {
    	  for (int j=0; j<GridSize; ++j) {
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


    void Make_MuellerAvgPB(vector< vector< vector < casa::Matrix<casa::Complex> > > > & StackMueller, double sum_weight_square);

    casa::Array< casa::Complex > Correct_CC(casa::Array< casa::Complex > & ModelImage);


    casa::Bool itsFilledVectorMasks;
      //vector< Matrix< Bool > > itsVectorMasksDegridElement;
    void ReadMaskDegrid()
    {
      casa::Int NumMask(0);
      while(true){
	casa::String MaskName("JAWS_products/Mask"+casa::String::toString(NumMask)+".boolim");
	casa::File MaskFile(MaskName);
	if(MaskFile.exists())
	  {
	    //cout<<"Reading:"<<MaskName<<endl;
	    casa::PagedImage<casa::Bool> pim(MaskName);
	    casa::Array<casa::Bool> arr = pim.get();
	    casa::Matrix<casa::Bool> Mask;
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
      casa::Int Wc(0);
      for(casa::uInt Tnum=0;Tnum<NBigChunks;++Tnum){
	for(casa::uInt Wnum=0;Wnum<2*m_nWPlanes;++Wnum){
	
	  casa::Int Wsearch(Wnum-m_nWPlanes);
	  casa::String MaskName("JAWS_products/Mask.T"+casa::String::toString(Tnum)+".W"+casa::String::toString(Wsearch)+".boolim");
	  casa::File MaskFile(MaskName);
	  if(MaskFile.exists())
	    {
	      casa::PagedImage<casa::Bool> pim(MaskName);
	      casa::Array<casa::Bool> arr = pim.get();
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
      casa::Int Wc(0);
      initStoreMasksNew();

      /* MaskName="JAWS_products/MaskGrid.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim"; */
      /* MaskName="JAWS_products/MaskDeGrid.T"+String::toString(NumTime)+".W"+String::toString(NumWplane)+".boolim"; */
      /* MaskName="JAWS_products/MaskGrid.Element.T"+String::toString(NumTime)+".boolim"; */
      /* vector< vector< Matrix< uInt > > > itsVecMasksNew; */
      /* vector< vector< Matrix< uInt > > > itsVecMasksNewW; */
      /* vector< Matrix< uInt > > itsVecMasksNewElement; */

      for(casa::uInt Tnum=0;Tnum<NBigChunks;++Tnum){
	for(casa::uInt Wnum=0;Wnum<2*m_nWPlanes;++Wnum){
	
	  casa::Int Wsearch(Wnum-m_nWPlanes);
	  casa::String MaskName("JAWS_products/MaskGrid.T"+casa::String::toString(Tnum)+".W"+casa::String::toString(Wsearch)+".boolim");
	  casa::File MaskFile(MaskName);
	  if(MaskFile.exists())
	    {
	      //cout<<".. reading "<<MaskName<<endl;
	      casa::PagedImage<casa::uShort> pim(MaskName);
	      casa::Array<casa::uShort> arr = pim.get();
	      itsVecMasksNew[Tnum][Wnum].reference (arr.nonDegenerate(2));
	      Wc+=1;
	    }
	  casa::String MaskName2("JAWS_products/MaskDeGrid.T"+casa::String::toString(Tnum)+".W"+casa::String::toString(Wsearch)+".boolim");
	  casa::File MaskFile2(MaskName);
	  if(MaskFile2.exists())
	    {
	      //cout<<".. reading "<<MaskName2<<endl;
	      casa::PagedImage<casa::uShort> pim(MaskName2);
	      casa::Array<casa::uShort> arr = pim.get();
	      itsVecMasksNewW[Tnum][Wnum].reference (arr.nonDegenerate(2));
	      Wc+=1;
	    }
	}
	casa::String MaskName("JAWS_products/MaskGrid.Element.T"+casa::String::toString(Tnum)+".boolim");
	casa::File MaskFile(MaskName);
	if(MaskFile.exists())
	  {
	    //cout<<".. reading "<<MaskName<<endl;
	    casa::PagedImage<casa::uShort> pim(MaskName);
	    casa::Array<casa::uShort> arr = pim.get();
	    casa::Matrix<casa::uShort> Mask;
	    Mask.reference(arr.nonDegenerate(2));
	    itsVecMasksNewElement.push_back(Mask);
	  }

      }
      //cout<<"... deon reading masks degrid"<<endl;
      itsFilledVectorMasks=true;
      
    }
      
    casa::Bool VectorMaskIsFilled(){return itsFilledVectorMasks;}
    
    void normalized_fft (casa::Matrix<casa::Complex>&, bool toFreq=true);
    void normalized_fft_parallel(casa::Matrix<casa::Complex> &im, bool toFreq=true);
    void normalized_fft (casa::PrecTimer& timer, casa::Matrix<casa::Complex>&, bool toFreq=true);

    casa::Vector< casa::Double >    list_freq_spw;
    casa::Vector< casa::Double >    list_freq_chanBlock;
    casa::Vector< casa::uInt >      map_chan_chanBlock;
    casa::Vector< casa::uInt >      map_chanBlock_spw;
    vector<casa::Vector< casa::uInt > >     map_spw_chanBlock;
    casa::Vector< casa::uInt > map_chan_Block_buffer;
    casa::uInt                m_nWPlanes;


  private:

    casa::Matrix<casa::Complex> give_normalized_fft_lapack(const casa::Matrix<casa::Complex> &im, bool toFreq=true)
      {
        casa::Matrix<casa::Complex> result(im.copy());
        casa::ArrayLattice<casa::Complex> lattice(result);
        casa::LatticeFFT::cfft2d(lattice, toFreq);
        if(toFreq){
          result/=static_cast<casa::Float>(result.shape()(0)*result.shape()(1));
        }
        else{
          result*=static_cast<casa::Float>(result.shape()(0)*result.shape()(1));
        };
        return result;
      }

    casa::MEpoch observationStartTime (const casa::MeasurementSet &ms,
                                 casa::uInt idObservation) const;

    // Estime spheroidal convolution function from the support of the fft
    // of the spheroidal in the image plane
    casa::Double makeSpheroidCut();

    // Return the angular resolution required for making the image of the
    // angular size determined by coordinates and shape.
    // The resolution is assumed to be the same on both direction axes.
    casa::Double estimateWResolution(const casa::IPosition &shape,
                               casa::Double pixelSize,
                               casa::Double w) const;


    // Return the angular resolution required for making the image of the
    // angular size determined by coordinates and shape.
    // The resolution is assumed to be the same on both direction axes.
    casa::Double estimateAResolution(const casa::IPosition &shape,
                               const casa::DirectionCoordinate &coordinates, double station_diam = 70.) const;

    // Apply a spheroidal taper to the input function.
    template <typename T>
    void taper (casa::Matrix<T> &function) const
    {
//       AlwaysAssert(function.shape()[0] == function.shape()[1], casa::SynthesisError);
      casa::uInt size = function.shape()[0];
      casa::Double halfSize = (size-1) / 2.0;
      casa::Vector<casa::Double> x(size);
      for (casa::uInt i=0; i<size; ++i) {
        x[i] = spheroidal(abs(i - halfSize) / halfSize);
      }
      for (casa::uInt i=0; i<size; ++i) {
        for (casa::uInt j=0; j<size; ++j) {
          function(j, i) *= x[i] * x[j];
        }
      }
    }

    template <typename T>
    void taper_parallel (casa::Matrix<T> &function) const
    {
//       AlwaysAssert(function.shape()[0] == function.shape()[1], SynthesisError);
      casa::uInt size = function.shape()[0];
      casa::Double halfSize = (size-1) / 2.0;
      casa::Vector<casa::Double> x(size);
      for (casa::uInt i=0; i<size; ++i) {
        x[i] = spheroidal(abs(i - halfSize) / halfSize);
      }
      casa::uInt j;
#pragma omp parallel
      {
#pragma omp for private(j) schedule(dynamic)
        for (casa::uInt i=0; i<size; ++i) 
        {
          for (j=0; j<size; ++j) 
          {
            function(j, i) *= x[i] * x[j];
          }
        }
      }
    }


    // Linear interpolation
    template <typename T>
    casa::Matrix< T > LinearInterpol(casa::Matrix<T> ImageIn, casa::Int  NpixOut)
      {
	casa::Matrix<T> ImageOut(casa::IPosition(2,NpixOut,NpixOut),0.);
	float d0(1./(NpixOut-1.));
	float d1(1./(ImageIn.shape()[0]-1.));
	float dd(d0/d1);
	float dx,dy,dxd,dyd,xin,yin;
	float onef(1.);
	for(casa::Int i=0;i<(NpixOut);++i){
	  dxd=i*dd;
	  xin=floor(dxd);
	  dx=dxd-xin;
	  for(casa::Int j=0;j<(NpixOut);++j){
	    dyd=j*dd;
	    yin=floor(dyd);
	    dy=dyd-yin;
	    ImageOut(i,j)=(onef-dx)*(onef-dy)*ImageIn(xin,yin) + (onef-dx)*(dy)*ImageIn(xin,yin+1) + (dx)*(onef-dy)*ImageIn(xin+1,yin) + (dx)*(dy)*ImageIn(xin+1,yin+1);
	  }
	}
	return ImageOut;
      }

    void Convolve(casa::Matrix<casa::Complex> gridin, casa::Matrix<casa::Complex> gridout, casa::Matrix<casa::Complex> ConvFunc){
      casa::uInt Support(ConvFunc.shape()[0]);
      casa::uInt GridSize(gridin.shape()[0]);
      casa::uInt off(Support/2);
      for(casa::uInt i=Support/2;i<GridSize-Support/2;++i){
	for(casa::uInt j=Support/2;j<GridSize-Support/2;++j){
	  if((gridin(i,j))!=casa::Complex(0.,0.)){
	    casa::Complex val(gridin(i,j));
	    for(casa::uInt ii=0;ii<Support;++ii){
	      for(casa::uInt jj=0;jj<Support;++jj){
		gridout(i-off+ii,j-off+jj)+=ConvFunc(ii,jj)*val;
	      }
	    }
	  }
	}
      }
    }

    void ConvolveOpt(casa::Matrix<casa::Complex> gridin, casa::Matrix<casa::Complex> gridout, casa::Matrix<casa::Complex> ConvFunc){
      casa::uInt Support(ConvFunc.shape()[0]);
      casa::uInt GridSize(gridin.shape()[0]);
      casa::uInt off(Support/2);

      casa::Complex* __restrict__ gridInPtr = gridin.data();
      casa::Complex* __restrict__ gridOutPtr = gridout.data();
      casa::Complex* __restrict__ ConvFuncPtr = ConvFunc.data();

      for(casa::uInt i=Support/2;i<GridSize-Support/2;++i){
	for(casa::uInt j=Support/2;j<GridSize-Support/2;++j){
	  gridInPtr=gridin.data()+GridSize*i+j;
	  if (gridInPtr->real() != 0  ||  gridInPtr->imag() != 0) {//if((*gridInPtr)!=Complex(0.,0.)){
	    ConvFuncPtr = ConvFunc.data();
	    for(casa::uInt jj=0;jj<Support;++jj){
	      for(casa::uInt ii=0;ii<Support;++ii){
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

    void ConvolveGer( const casa::Matrix<casa::Complex>& gridin, casa::Matrix<casa::Complex>& gridout,
		      const casa::Matrix<casa::Complex>& ConvFunc)
    {
      casa::uInt Support(ConvFunc.shape()[0]);
      casa::uInt GridSize(gridin.shape()[0]);
      casa::uInt off(Support/2);
      const casa::Complex* inPtr = gridin.data() + off*GridSize + off;
      for (casa::uInt i=0; i<GridSize-Support; ++i) {
	for (casa::uInt j=0; j<GridSize-Support; ++j) {
	  if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	    const casa::Complex* cfPtr = ConvFunc.data();
	    for (casa::uInt ii=0; ii<Support; ++ii) {
	      casa::Complex* outPtr = gridout.data() + (i+ii)*GridSize + j;
	      for (casa::uInt jj=0; jj<Support; ++jj) {
		outPtr[jj] += *cfPtr++ * *inPtr;
	      }
	    }
	  }
	  inPtr++;
	}
	inPtr += Support;
      }
    }

    void ConvolveGerArray( const casa::Array<casa::Complex>& gridin, casa::Int ConvPol, casa::Matrix<casa::Complex>& gridout,
			   const casa::Matrix<casa::Complex>& ConvFunc)
    {
      casa::uInt Support(ConvFunc.shape()[0]);
      casa::uInt GridSize(gridin.shape()[0]);
      casa::uInt off(Support/2);

      const casa::Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
      for (casa::uInt i=0; i<GridSize-Support; ++i) {
	for (casa::uInt j=0; j<GridSize-Support; ++j) {
	  if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	    const casa::Complex* cfPtr = ConvFunc.data();
	    for (casa::uInt ii=0; ii<Support; ++ii) {
	      casa::Complex* outPtr = gridout.data() + (i+ii)*GridSize + j;
	      for (casa::uInt jj=0; jj<Support; ++jj) {
		outPtr[jj] += *cfPtr++ * *inPtr;
	      }
	    }
	  }
	  inPtr++;
	  }
	inPtr += Support;
      }
    }
    
    void ConvolveArrayArray( const casa::Array<casa::Complex>& gridin, casa::Array<casa::Complex>& gridout,
			   const casa::Matrix<casa::Complex>& ConvFunc)
    {
      int Support(ConvFunc.shape()[0]);
      int GridSize(gridin.shape()[0]);
      int off(Support/2);



      for(casa::uInt ConvPol=0; ConvPol<gridin.shape()[2];++ConvPol){

	casa::Int offPol(ConvPol*GridSize*GridSize);
	const casa::Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
	for (casa::Int i=0; i<GridSize-Support; ++i) {
	  for (casa::Int j=0; j<GridSize-Support; ++j) {
	    if (inPtr->real() != 0  ||  inPtr->imag() != 0) {
	      const casa::Complex* cfPtr = ConvFunc.data();
	      for (casa::Int ii=0; ii<Support; ++ii) {
		casa::Complex* outPtr = gridout.data() + (i+ii)*GridSize + j +offPol;
		for (casa::Int jj=0; jj<Support; ++jj) {
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
    


    void ConvolveGerArrayMask( const casa::Array<casa::Complex>& gridin, casa::Int ConvPol, casa::Matrix<casa::Complex>& gridout,
			       const casa::Matrix<casa::Complex>& ConvFunc, casa::Int UsedMask)
    {
      casa::uInt Support(ConvFunc.shape()[0]);
      casa::uInt GridSize(gridin.shape()[0]);
      casa::uInt off(Support/2);

      const casa::Complex* inPtr = gridin.data() + ConvPol*GridSize*GridSize + off*GridSize + off;
      const casa::Bool* MaskPtr = itsVectorMasksDegridElement[UsedMask].data() + off*GridSize + off;
      for (casa::uInt i=0; i<GridSize-Support; ++i) {
	for (casa::uInt j=0; j<GridSize-Support; ++j) {
	  if ((*MaskPtr)==true) {
	    const casa::Complex* cfPtr = ConvFunc.data();
	    for (casa::uInt ii=0; ii<Support; ++ii) {
	      casa::Complex* outPtr = gridout.data() + (i+ii)*GridSize + j;
	      for (casa::uInt jj=0; jj<Support; ++jj) {
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
    casa::Matrix< T > LinearInterpol2(casa::Matrix<T> ImageIn, casa::Int  NpixOut)
      {
	casa::Matrix<T> ImageOut(casa::IPosition(2,NpixOut,NpixOut),1e-7);
	int nd(ImageIn.shape()[0]);
	int ni(NpixOut);
	float off(-.5);//-(((1.+1./(nd-1.))-1.)/2.)*(nd-1));
	float a(nd/(ni-1.));//((1.+1./(nd-1.))/(ni-1.))*(nd-1));
	float dx,dy,dxd,dyd,xin,yin;
	float onef(1.);
	for(casa::Int i=0;i<(NpixOut);++i){
	  dxd=i*a+off;
	  xin=floor(dxd);
	  dx=dxd-xin;
	  for(casa::Int j=0;j<(NpixOut);++j){
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

    void EstimateCoordShape(casa::IPosition shape, casa::DirectionCoordinate coordinate, double station_diameter=70.){
      coordinate = m_coordinates;
      casa::Double aPixelAngSize = min(m_pixelSizeSpheroidal,
				 estimateAResolution(m_shape, m_coordinates, station_diameter));
      
      casa::Double pixelSize = abs(m_coordinates.increment()[0]);
      casa::Double imageDiameter = pixelSize * m_shape(0);
      casa::Int nPixelsConv = imageDiameter / aPixelAngSize;
      if (nPixelsConv > itsMaxSupport) {
          nPixelsConv = itsMaxSupport;
      }
      // Make odd and optimal.
      nPixelsConv = FFTCMatrix::optimalOddFFTSize (nPixelsConv);
      aPixelAngSize = imageDiameter / nPixelsConv;

      shape=casa::IPosition(2, nPixelsConv, nPixelsConv);
      casa::Vector<casa::Double> increment_old(coordinate.increment());
      casa::Vector<casa::Double> increment(2);
      increment[0] = aPixelAngSize*casa::sign(increment_old[0]);
      increment[1] = aPixelAngSize*casa::sign(increment_old[1]);
      coordinate.setIncrement(increment);
      casa::Vector<casa::Double> refpix(2, 0.5*(nPixelsConv-1));
      coordinate.setReferencePixel(refpix);
    }




    casa::Double spheroidal(casa::Double nu) const;






    template <typename T>
    casa::uInt findSupport(casa::Matrix<T> &function, casa::Double threshold) const
    {
      ///      Double peak = abs(max(abs(function)));
      casa::Double peak = max(amplitude(function));
      threshold *= peak;
      casa::uInt halfSize = function.shape()[0] / 2;
      casa::uInt x = 0;
      while (x < halfSize && abs(function(x, halfSize)) < threshold) {
        ++x;
      }
      return 2 * (halfSize - x);
    }


    //# Data members.
    casa::Record       itsParameters;
    casa::Array<casa::Complex> its_output_grid_element;
    casa::Matrix<casa::Complex> its_ArrMatrix_out_element;
    casa::IPosition           m_shape;
    casa::DirectionCoordinate m_coordinates;
    WScale              m_wScale;
    LofarWTerm          m_wTerm;
    LofarATerm          m_aTerm;
    casa::Double              m_maxW;
    casa::Double              m_pixelSizeSpheroidal;
    casa::uInt                m_nStations;
    casa::uInt                m_oversampling;
    casa::uInt                m_nChannelBlocks;
    casa::uInt                m_NPixATerm;
    casa::Double              m_refFrequency;
    casa::uInt                m_maxCFSupport;
    vector<casa::Double>      its_VectorThreadsSumWeights;

    //# Stack of the convolution functions for the average PB calculation
    casa::Matrix<casa::Complex>     Spheroid_cut;
    //# Stack of the convolution functions for the average PB calculation
    casa::Matrix<casa::Float>       Spheroid_cut_im;
    casa::Matrix<casa::Float>       Spheroid_cut_im_element;
    //# List of the ferquencies the CF have to be caluclated for
    vector< casa::Matrix<casa::Complex> > m_WplanesStore;
    //# Aterm_store[double time][antenna][channel]=Cube[Npix,Npix,4]
    map<casa::Double, vector< vector< casa::Cube<casa::Complex> > > > m_AtermStore;
    map<casa::Double, vector< vector< casa::Cube<casa::Complex> > > > m_AtermStore_element;
    map<casa::Double, vector< vector< casa::Cube<casa::Complex> > > > m_AtermStore_station;
    //# Average primary beam
    casa::Matrix<casa::Float>       Im_Stack_PB_CF0;
    casa::Int                 itsVerbose;
    casa::Int                 itsMaxSupport;
    //    Int                 itsTaylorTerm;
    //Double              itsRefFreq;
    casa::String              itsImgName;
    vector<FFTCMatrix>  itsFFTMachines;
    casa::Double              itsTimeW;
    casa::Double              itsTimeWpar;
    casa::Double              itsTimeWfft;
    unsigned long long  itsTimeWcnt;
    casa::Double              itsTimeA;
    casa::Double              itsTimeApar;
    casa::Double              itsTimeAfft;
    unsigned long long  itsTimeAcnt;
    casa::Double              itsTimeCF;
    casa::Double              itsTimeCFpar;
    casa::Double              itsTimeCFfft;
    unsigned long long  itsTimeCFcnt;
    casa::Bool                its_Use_EJones;
    casa::Bool                its_Apply_Element;
    casa::Bool                its_NotApplyElement;
    casa::Bool                its_NotApplyArray;
    casa::uInt                its_MaxWSupport;
    casa::uInt                its_count_time;
    mutable casa::LogIO       m_logIO;
    casa::Int                 its_ChanBlockSize;
    casa::Matrix<casa::Complex>     spheroid_cut_element_fft;
    vector< vector< casa::Matrix< casa::Complex > > > GridsMueller;
    casa::LogIO &logIO() const
      {
        return m_logIO;
      }
  };



  //# =================================================
  template <class T>
  void store(const casa::Matrix<T> &data, const string &name)
  {
    casa::Matrix<casa::Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    casa::Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> refLatLon(45.0 * casa::C::pi / 180.0, "rad");
    casa::DirectionCoordinate dir(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store (const casa::DirectionCoordinate &dir, const casa::Matrix<T> &data,
              const string &name)
  {
    //cout<<"Saving... "<<name<<endl;
    casa::Vector<casa::Int> stokes(1);
    stokes(0) = casa::Stokes::I;
    casa::CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(casa::StokesCoordinate(stokes));
    csys.addCoordinate(casa::SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    casa::PagedImage<T> im(casa::TiledShape(casa::IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, casa::IPosition(4, 0, 0, 0, 0));
  }

  template <class T>
  void store(const casa::Cube<T> &data, const string &name)
  {
    casa::Matrix<casa::Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    casa::Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> refLatLon(45.0 * casa::C::pi / 180.0, "rad");
    casa::DirectionCoordinate dir(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const casa::DirectionCoordinate &dir, const casa::Cube<T> &data,
             const string &name)
  {
//     AlwaysAssert(data.shape()(2) == 4, SynthesisError);
    //cout<<"Saving... "<<name<<endl;
    casa::Vector<casa::Int> stokes(4);
    stokes(0) = casa::Stokes::XX;
    stokes(1) = casa::Stokes::XY;
    stokes(2) = casa::Stokes::YX;
    stokes(3) = casa::Stokes::YY;
    casa::CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(casa::StokesCoordinate(stokes));
    csys.addCoordinate(casa::SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    casa::PagedImage<T>
      im(casa::TiledShape(casa::IPosition(4, data.shape()(0), data.shape()(1), 4, 1)),
         csys, name);
    im.putSlice(data, casa::IPosition(4, 0, 0, 0, 0));
  }

} //# end namespace LOFAR

#endif
