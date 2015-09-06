//# CubeSkyEquation.h: CubeSkyEquation definition
//# Copyright (C) 2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#ifndef LOFAR_LOFARFT_CUBESKYEQUATION_H
#define LOFAR_LOFARFT_CUBESKYEQUATION_H

#include <synthesis/MeasurementEquations/SkyEquation.h>
//#include <synthesis/Utilities/ThreadTimers.h>


//Forward

namespace casa {    
  class ROVisibilityIterator;
  template <class T> class ImageInterface;
  template <class T> class TempImage;
  template <class T> class SubImage;
  template <class T> class Block;
}

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT { //# NAMESPACE LOFAR - BEGIN
  
class VisibilityIterator;
class VisBuffer;

class CubeSkyEquation : public casa::SkyEquation {

  public:
  CubeSkyEquation(
    casa::SkyModel& sm, 
    casa::VisSet& vs, 
    casa::FTMachine& ft, 
    casa::ComponentFTMachine& cft, 
    casa::Bool noModelCol = casa::False);

  //Read only iterator...hence no scratch col
  CubeSkyEquation(
    casa::SkyModel& sm, 
    VisibilityIterator& vi, 
    casa::FTMachine& ft, 
    casa::ComponentFTMachine& cft, 
    casa::Bool noModelCol = casa::False);

  virtual ~CubeSkyEquation();
  
  virtual void predict(
    casa::Bool incremental = casa::False, 
    casa::MS::PredefinedColumns Type = casa::MS::MODEL_DATA);
  
  virtual void gradientsChiSquared(casa::Bool incremental, casa::Bool commitModel=casa::False);

  virtual void initializePutSlice(
    const VisBuffer& vb, 
    casa::Bool dopsf, 
    casa::Int cubeSlice=0, 
    casa::Int nCubeSlice=1);
  
  virtual void putSlice(VisBuffer& vb, casa::Bool dopsf, 
                        casa::FTMachine::Type col, casa::Int cubeSlice=0, 
                        casa::Int nCubeSlice=1);
  virtual void finalizePutSlice(const VisBuffer& vb,  casa::Bool dopsf,
                                casa::Int cubeSlice=0, casa::Int nCubeSlice=1);
  void initializeGetSlice(const VisBuffer& vb, casa::Int row,
                          casa::Bool incremental, casa::Int cubeSlice=0, 
                          casa::Int nCubeSlice=1);   
  virtual VisBuffer& getSlice(VisBuffer& vb, 
                              casa::Bool incremental, casa::Int cubeSlice=0,
                              casa::Int nCubeSlice=1); 
  void finalizeGetSlice();
  void isLargeCube(casa::ImageInterface<casa::Complex>& theIm, casa::Int& nCubeSlice);
  //void makeApproxPSF(Int model, ImageInterface<Float>& psf);
  //virtual void makeApproxPSF(Int model, ImageInterface<Float>& psf); 
  void makeApproxPSF(casa::PtrBlock<casa::ImageInterface<casa::Float> * >& psfs);

  //Get the flux scale that the ftmachines have if they have
  virtual void getCoverageImage(casa::Int model, casa::ImageInterface<casa::Float>& im);

  //get the weight image from the ftmachines
  virtual void getWeightImage(const casa::Int model, casa::ImageInterface<casa::Float>& weightim);
  void tmpWBNormalizeImage(casa::Bool& dopsf, const casa::Float& pbLimit);

  protected:

  //Different versions of psf making
  void makeSimplePSF(casa::PtrBlock<casa::ImageInterface<casa::Float> * >& psfs);
  void makeMosaicPSF(casa::PtrBlock<casa::ImageInterface<casa::Float> * >& psfs);
  virtual void fixImageScale();
  casa::Block<casa::CountedPtr<casa::ImageInterface<casa::Complex> > >imGetSlice_p;
  casa::Block<casa::CountedPtr<casa::ImageInterface<casa::Complex> > >imPutSlice_p;
  casa::Block<casa::Matrix<casa::Float> >weightSlice_p;
  casa::Slicer sl_p;
  casa::Int nchanPerSlice_p;
  // Type of copy 
  // 0 => a independent image just with coordinates gotten from cImage
  // 1 => a subImage referencing cImage ...no image copy
  void sliceCube(casa::CountedPtr<casa::ImageInterface<casa::Complex> >& slice, casa::Int model, casa::Int cubeSlice, casa::Int nCubeSlice, casa::Int typeOfCopy=0); 
  void sliceCube(casa::SubImage<casa::Float>*& slice, casa::ImageInterface<casa::Float>& image, casa::Int cubeSlice, casa::Int nCubeSlice);
  //frequency range from image
  casa::Bool getFreqRange(VisibilityIterator& vi, const casa::CoordinateSystem& coords,
                  casa::Int slice, casa::Int nslice);

  casa::Bool isNewFTM(casa::FTMachine *);
  private:
  // if skyjones changed in get or put we need to tell put or get respectively
  // about it
  void init(casa::FTMachine& ft);

  casa::Bool destroyVisibilityIterator_p;

  casa::Bool internalChangesPut_p;
  casa::Bool internalChangesGet_p;
  casa::Bool firstOneChangesPut_p;
  casa::Bool firstOneChangesGet_p;

  casa::Block< casa::Vector<casa::Int> >blockNumChanGroup_p, blockChanStart_p;
  casa::Block< casa::Vector<casa::Int> > blockChanWidth_p, blockChanInc_p;
  casa::Block<casa::Vector<casa::Int> > blockSpw_p;
  casa::Block<casa::CountedPtr<casa::FTMachine> > ftm_p;
  casa::Block<casa::CountedPtr<casa::FTMachine> > iftm_p;

  VisibilityIterator * rvi_p;
  VisibilityIterator * wvi_p;

  // DT aInitGrad, aGetChanSel, aCheckVisRows, aGetFreq, aOrigChunks, aVBInValid, aInitGetSlice, aInitPutSlice, aPutSlice, aFinalizeGetSlice, aFinalizePutSlice, aChangeStokes, aInitModel, aGetSlice, aSetModel, aGetRes, aExtra;
};

} //# NAMESPACE LOFARFT
} //# NAMESPACE LOFAR 

#endif
