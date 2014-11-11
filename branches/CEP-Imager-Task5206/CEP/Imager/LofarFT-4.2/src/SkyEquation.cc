//# CubeSkyEquation.cc: Implementation of Cube Optimized Sky Equation classes
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <lofar_config.h>

#include <casa/iostream.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/OS/HostInfo.h>
#include <casa/System/ProgressMeter.h>
#include <casa/Utilities/CountedPtr.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <lattices/Lattices/LatticeExpr.h>

#include <LofarFT/VisBuffer.h>
#include <LofarFT/VisibilityIterator.h>
#include <LofarFT/SkyEquation.h>
#include <LofarFT/FTMachine.h>

#include <synthesis/MeasurementComponents/SkyModel.h>
#include <synthesis/TransformMachines/ComponentFTMachine.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/Utilities/SigHandler.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>

#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/VisBufferUtil.h>
#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisibilityIterator.h>

using namespace casa;

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT {
  
SkyEquation::SkyEquation(
  SkyModel& sm, 
  VisibilityIterator& vi, 
  FTMachine& ft,
  ComponentFTMachine& cft, 
  Bool noModelCol)
: casa::SkyEquation(sm, vi, ft, cft, noModelCol),
  itsFTMachine(&ft),
  rvi_p(&vi),
  wvi_p(&vi)
//   itsFTMachine(ft)
{}

SkyEquation::~SkyEquation()
{
}

void  SkyEquation::predict(Bool incremental, MS::PredefinedColumns col) 
{
  
  cout << "I am SkyEquation->predict" << endl;

  VisibilityIterator::DataColumn visCol = VisibilityIterator::Model;
  
  if(col==MS::DATA)
  {
    visCol=VisibilityIterator::Observed;
  } 
  
  if(col==MS::CORRECTED_DATA)
  {
    visCol=VisibilityIterator::Corrected;
  }

  AlwaysAssert(sm_, AipsError);
  
  if(sm_->numberOfModels()!= 0)  AlwaysAssert(ok(),AipsError);
  
  if(wvi_p==NULL)
    throw(AipsError("Cannot save model in non-writable ms"));
  
  VisibilityIterator& vi=*wvi_p;
  //Lets get the channel selection for later use
  CountedPtr<VisBuffer> vb (new VisBuffer(vi)); // uses write VI so no ROVIA conversion
  // Reset the visibilities only if this is not an incremental
  // change to the model
  Bool initialized=False;
  
  Bool isEmpty=True;
  for (Int model=0; model < (sm_->numberOfModels());++model){
    isEmpty = isEmpty &&  (sm_->isEmpty(model));                
  }

  //TODO: check for the existence of column visCol instead of MODEL_DATA
  //TODO: optionally create column if it does not exists
  noModelCol_p = rvi_p->msColumns().modelData().isNull();
  
  if (isEmpty) cout << "Sorry, model is empty" << endl;
  if (noModelCol_p) cout << "Sorry, no model column" << endl;
  
  
  Int nmodels = sm_->numberOfModels();
  
  PtrBlock<ImageInterface<Float> * > model_images(nmodels);
  for (Int i=0; i<nmodels; i++)
  {
    model_images[i] = &sm_->image(i);
  }
  
  itsFTMachine->initializeToVis(model_images, False);

  if( !isEmpty  && !noModelCol_p)
  { 
    // We are at the begining with an empty model as starting point
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) 
    {
      for (vi.origin(); vi.more(); vi++) 
      {
        vb->setModelVisCube(Complex(0.0,0.0));

        // Get the model visibilities -> degridding
        // Model visibilities are put in the modelVisCube of vb
        itsFTMachine->get(* vb, -1);
//         cout << vb->modelVisCube() << endl;
        cout << "visCol: " << visCol << " " << VisibilityIterator::Model << endl;
        vi.setVis(vb->modelVisCube(),visCol);
      }
    }
  }
}

// WBCleanImageSkyModel uses makeApproxPSF in a strange way
// It ignores the results passed in psfs
// and assumes that we put the result directly in gS of the SkyModel sm_
void SkyEquation::makeApproxPSF(PtrBlock<ImageInterface<Float> * >& psfs) 
{
    LogIO os(LogOrigin("LOFAR::LofarFT::SkyEquation", "makeApproxPSF"));

    sm_->setImageNormalization(True);
    
    Int nmodels = psfs.nelements();
    
    cout << "Number of PSF: " << nmodels << endl;
    cout << "Number of taylor terms: " << sm_->numberOfTaylorTerms() << endl;
    cout << "Number of models: " << sm_->numberOfModels() << endl;
    cout << "psf shape: " << psfs[0]->shape() << endl;
    cout << sm_->cImage(0).shape() << endl;
    
    PtrBlock<ImageInterface<Float> * > images;
    images.resize(nmodels);
    for (Int i=0; i<nmodels; i++)
    {
      images[i] = &sm_->gS(i);
    }
    
    Bool doPSF = True;
    sm_->initializeGradients();
    VisibilityIterator& vi(*rvi_p);
    CountedPtr<VisBuffer> vb (new VisBuffer(vi));
    
    cout << "======================" << endl;
    itsFTMachine->initializeToSky(images, doPSF);
    cout << "======================" << endl;

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) 
      {
        itsFTMachine->put(* vb, -1, doPSF, FTMachine::MODEL);
      }
    }
    itsFTMachine->finalizeToSky();
    
    Matrix<Float> weights;
    itsFTMachine->getImages(weights, True);
    
    //lets return original selection back to iterator

    sm_->finalizeGradients();

    for(Int model=0; model < nmodels; ++model)
    {
      LatticeExprNode maxPSF=max(sm_->gS(model));
      Float maxpsf=maxPSF.getFloat();
      if(abs(maxpsf-1.0) > 1e-3) {
        os << "Maximum of approximate PSF for field " << model << " = "
            << maxpsf << " : renormalizing to unity" <<  LogIO::POST;
      }
      if(maxpsf > 0.0 )
      {
//         LatticeExpr<Float> len((*psfs[model])/maxpsf);
//         psfs[model]->copyData(len);
      }
      else
      {
        if(sm_->numberOfTaylorTerms()>1) 
        { /* MFS */
          os << "PSF calculation resulted in a PSF with its peak being 0 or less. This is ok for MS-MFS." << LogIO::POST;
        }
        else
        {
          throw(PSFZero("SkyEquation:: PSF calculation resulted in a PSF with its peak being 0 or less!"));
        }
      }
    }
}

void SkyEquation::gradientsChiSquared(Bool /*incr*/, Bool commitModel){

  AlwaysAssert(sm_, AipsError);

  VisibilityIterator& vi(*rvi_p);
  CountedPtr<VisBuffer> vb (new VisBuffer(vi));
  vi.originChunks();
  vi.origin();

  Bool isEmpty=True;
  for (Int model=0; model < (sm_->numberOfModels());++model)
  {
    isEmpty = isEmpty &&  sm_->isEmpty(model);
  }
    
  sm_->setImageNormalization(True);
  
  Int nmodels = sm_->numberOfModels();
  
  PtrBlock<ImageInterface<Float> * > images;
  images.resize(nmodels);
  for (Int i=0; i<nmodels; i++)
  {
    images[i] = &sm_->gS(i);
  }
  
  Bool doPSF = False;
  sm_->initializeGradients();
  

  if( isEmpty )
  {
    // model image is empty, residual image is dirty image
    cout << "Model images are empty" << endl;
    
    itsFTMachine->initializeToSky(images, doPSF);
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) 
      {
        itsFTMachine->put(* vb, -1, doPSF, FTMachine::CORRECTED);
      }
    }
    itsFTMachine->finalizeToSky();
  }
  else
  {
    cout << "Model images are not empty" << endl;
    
    // Fill a PtrBlock with pointers the the model images in the SkyModel sm_
    PtrBlock<ImageInterface<Float> * > model_images(nmodels);
    for (Int i=0; i<nmodels; i++)
    {
      model_images[i] = &sm_->image(i);
    }
    itsFTMachine->initializeResidual(model_images, images, True);
    
    for (rvi_p->originChunks();rvi_p->moreChunks();rvi_p->nextChunk()) 
    {
      for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++) 
      {
        //This here forces the modelVisCube shape and prevents reading model column
        vb->setModelVisCube(Complex(0.0,0.0));
        
        //TODO: call the residual method of FTMachine instead of get and put
   
        // Get the model visibilities -> degridding
        // Model visibilities are put in the modelVisCube of vb
        itsFTMachine->get(* vb, -1);
        
        //TODO: optionally write model visibilities to MS
        
        // Compute residual 
        vb->modelVisCube() = vb->correctedVisCube() - vb->modelVisCube();
        
        // Put residual on grid -> gridding
        itsFTMachine->put(* vb, -1, doPSF, FTMachine::MODEL);
      }
    }
    itsFTMachine->finalizeResidual();
  }
  
  
  Matrix<Float> weights;
  itsFTMachine->getImages(weights, True);
  
  sm_->finalizeGradients();
}

} // end namespace LofarFT
} // end namespace LOFAR
