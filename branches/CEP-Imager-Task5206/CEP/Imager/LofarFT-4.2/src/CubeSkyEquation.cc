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

#include <LofarFT/CubeSkyEquation.h>
#include <LofarFT/VisBuffer.h>
#include <LofarFT/VisibilityIterator.h>

#include <synthesis/MeasurementComponents/SkyModel.h>
#include <synthesis/TransformMachines/SkyJones.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/TransformMachines/SetJyGridFT.h>
#include <synthesis/TransformMachines/GridFT.h>
#include <synthesis/TransformMachines/MosaicFT.h>
#include <synthesis/TransformMachines/MultiTermFT.h>
#include <synthesis/TransformMachines/NewMultiTermFT.h>
#include <synthesis/MeasurementComponents/GridBoth.h>
#include <synthesis/TransformMachines/WProjectFT.h>
#include <synthesis/MeasurementComponents/nPBWProjectFT.h>
#include <synthesis/TransformMachines/AWProjectFT.h>
#include <synthesis/TransformMachines/AWProjectWBFT.h>
#include <synthesis/MeasurementComponents/PBMosaicFT.h>
#include <synthesis/TransformMachines/WPConvFunc.h>
#include <synthesis/TransformMachines/SimplePBConvFunc.h>
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

#ifdef HAS_OMP
#include <omp.h>
#endif

using namespace casa;

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT {
  
CubeSkyEquation::CubeSkyEquation(
  SkyModel& sm, 
  VisSet& vs, 
  casa::FTMachine& ft,
  ComponentFTMachine& cft, 
  Bool noModelCol)
: casa::SkyEquation(sm, vs, ft, cft, noModelCol),
  destroyVisibilityIterator_p (False),
  internalChangesPut_p(False),
  internalChangesGet_p(False),
  firstOneChangesPut_p(False),
  firstOneChangesGet_p(False)
{
    init(ft);
}

CubeSkyEquation::CubeSkyEquation(
  SkyModel& sm, 
  VisibilityIterator& vi, 
  casa::FTMachine& ft,
  ComponentFTMachine& cft, 
  Bool noModelCol)
: SkyEquation(sm, vi, ft, cft, noModelCol),
  rvi_p(&vi),
  wvi_p(&vi),
  destroyVisibilityIterator_p (False),
  internalChangesPut_p(False),
  internalChangesGet_p(False),
  firstOneChangesPut_p(False),
  firstOneChangesGet_p(False)
{
    init(ft);
}

void CubeSkyEquation::init(casa::FTMachine& ft){
  
  Int nmod = sm_->numberOfModels()/sm_->numberOfTaylorTerms();
  nmod = 1;

  doflat_p=False;
  
  //case of component ft only
  if(nmod==0)
    nmod=1;
  
  ftm_p.resize(nmod, True);
  iftm_p.resize(nmod, True);
  
  //make a distinct ift_ as gridding and degridding can occur simultaneously
  for (Int k=0; k < (nmod); ++k){ 
    ftm_p[k] = ft.cloneFTM();
    iftm_p[k] = ft.cloneFTM();
  }
  ft_ = &*ftm_p[0];
  ift_ = &*iftm_p[0];

  imGetSlice_p.resize(nmod, True, False);
  imPutSlice_p.resize(nmod, True, False);
  weightSlice_p.resize(nmod, True, False);

}

CubeSkyEquation::~CubeSkyEquation(){
  //As we  make an explicit ift_ in the constructor we need 
  //to take care of it here...
  //if(ift_ && (ift_ != ft_))
  //  delete ift_;

    if (destroyVisibilityIterator_p){
        delete rvi_p;
        rvi_p = NULL;
        delete (vb_p.release ()); // free up the associated VisBuffer
    }
    SigHandler::resetSignalHandlers();
}

void  CubeSkyEquation::predict(Bool incremental, MS::PredefinedColumns col) {

  VisibilityIterator::DataColumn visCol=VisibilityIterator::Model;
  if(col==MS::DATA){
    visCol=VisibilityIterator::Observed;
  } 
  if(col==MS::CORRECTED_DATA){
    visCol=VisibilityIterator::Corrected;
  }
  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  //AlwaysAssert(vs_, AipsError);
  if(sm_->numberOfModels()!= 0)  AlwaysAssert(ok(),AipsError);
  if(wvi_p==NULL)
    throw(AipsError("Cannot save model in non-writable ms"));
  VisibilityIterator& vi=*wvi_p;
  //Lets get the channel selection for later use
  vi.getChannelSelection(blockNumChanGroup_p, blockChanStart_p,
                          blockChanWidth_p, blockChanInc_p, blockSpw_p);
  checkVisIterNumRows(vi);
  CountedPtr<VisBuffer> vb (new VisBuffer(vi)); // uses write VI so no ROVIA conversion
  Bool changedVI=False;
  // Reset the visibilities only if this is not an incremental
  // change to the model
  Bool initialized=False;
  predictComponents(incremental, initialized);
  //set to zero then loop over model...check for size...subimage then loop over  subimages
  
  
  Bool isEmpty=True;
  for (Int model=0; model < (sm_->numberOfModels());++model){
    isEmpty=isEmpty &&  (sm_->isEmpty(model));                
    
  }
  ////if people want to use model but it isn't there..we'll ignore you
  if(!noModelCol_p)
    noModelCol_p=rvi_p->msColumns().modelData().isNull();
  
  
  if( (sm_->numberOfModels() >0) && isEmpty  && !initialized && !incremental){ 
    // We are at the begining with an empty model as starting point
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
	if(!noModelCol_p){
	  vb->setModelVisCube(Complex(0.0,0.0));
	  vi.setVis(vb->modelVisCube(),visCol);
	}
      }
    }
  }
  
    //If all model is zero...no need to continue
  if(isEmpty) 
    return;
  //TODO if nomodel set flat to 0.0
  
  
  // Now do the images
  for (Int model=0; model < (sm_->numberOfModels());++model){ 
    // Change the model polarization frame
    if(vb->polFrame()==MSIter::Linear) {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					StokesImageUtil::LINEAR);
    }
    else {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					StokesImageUtil::CIRCULAR);
    }
    scaleImage(model, incremental);
  }
  ft_=&(*ftm_p[0]);
  // Reset the various SkyJones
  resetSkyJones();
  Int nCubeSlice=1;
  isLargeCube(sm_->cImage(0), nCubeSlice);
  for (Int cubeSlice=0; cubeSlice< nCubeSlice; ++cubeSlice){
    changedVI = getFreqRange(vi, sm_->cImage(0).coordinates(),
			    cubeSlice, nCubeSlice) || changedVI;
    vi.originChunks();
    vi.origin();
    initializeGetSlice(* vb, 0,incremental, cubeSlice, nCubeSlice);
    ///vb->newMS does not seem to be set to true with originchunks
    //so have to monitor msid
    Int oldmsid=-1;
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
    {
      for (vi.origin(); vi.more(); vi++) 
      {
        if(noModelCol_p)
        {
          if(vb->msId() != oldmsid)
          {
            oldmsid=vb->msId();
            for (Int model=0; model < (sm_->numberOfModels());++model)
            {
              Record ftrec;
              String error;
              //cerr << "in ftrec saving" << endl;
              if(!(ftm_p[model]->toRecord(error, ftrec, True)))
                throw(AipsError("Error in record saving:  "+error));
              vi.putModel(ftrec, False, ((model>0) || incremental || (cubeSlice > 0)));
            }
          }
        }
        else
        {
          if(!incremental && !initialized) {
            vb->setModelVisCube(Complex(0.0,0.0));
          }
          // get the model visibility and write it to the model MS
          getSlice(*vb, incremental, cubeSlice, nCubeSlice);
          vi.setVis(vb->modelVisCube(),visCol);
        }
      }
    }
    finalizeGetSlice();
    if(!incremental&&!initialized) initialized=True;
  }
  
  for(Int model=0; model < sm_->numberOfModels(); ++model){
      //For now unscale test on name of ft_
    ft_=&(*ftm_p[model]);
    unScaleImage(model, incremental);
  }
  ft_=&(*ftm_p[0]);
  
  //lets return original selection back to iterator
  if(changedVI)
    vi.selectChannel(blockNumChanGroup_p, blockChanStart_p, 
		     blockChanWidth_p, blockChanInc_p, blockSpw_p); 
  
}

void CubeSkyEquation::makeApproxPSF(PtrBlock<ImageInterface<Float> * >& psfs) 
{

  if(iftm_p[0]->name()=="MosaicFT")
    makeMosaicPSF(psfs);
  else
    makeSimplePSF(psfs);

}
void CubeSkyEquation::makeMosaicPSF(PtrBlock<ImageInterface<Float> * >& psfs){
  //lets try to make the psf directly
  LogIO os(LogOrigin("SkyEquation", "makeMosaicPSF"));
  makeSimplePSF(psfs);
  Int xpos;
  Int ypos;
  Matrix<Float> psfplane;
  Float peak;
  StokesImageUtil::locatePeakPSF(*(psfs[0]), xpos, ypos, peak, psfplane);
  Int nx=psfplane.shape()(0);
  Int ny=psfplane.shape()(1);
  Bool centered=True;
  // lets ignore  misers who made 10x10 pixel images
  centered=(abs(xpos-nx/2) <=5) && (abs(ypos-ny/2) <=5);

  ///////////////////////////////
  if(centered){
    //for cubes some of the planes may not have a central peak
    Int nchana= (psfs[0])->shape()(3);
    if(nchana > 1){
      IPosition blc(4,nx, ny, 0, nchana);
      IPosition trc(4, nx, ny, 0, nchana);
      blc(0)=0; blc(1)=0; trc(0)=nx-1; trc(1)=ny-1;
      Array<Float> goodplane(psfplane.reform(IPosition(4, nx,ny,1,1)));
      for (Int k=0; k < nchana ; ++k)
      {
        blc(3)=k; trc(3)=k;
        Slicer sl(blc, trc, Slicer::endIsLast);
        SubImage<Float> psfSub(*(psfs[0]), sl, True);
        Float planeMax;
        LatticeExprNode LEN = max( psfSub );
        planeMax =  LEN.getFloat();
        if( (planeMax >0.0) && (planeMax < 0.8 *peak))
        {
          psfSub.put(goodplane);
        }
      }
    }
    return;
  }
  //lets back up the ftmachines
  MosaicFT *ft_back= new MosaicFT(static_cast<MosaicFT &>(*ftm_p[0]));
  MosaicFT *ift_back = new MosaicFT(static_cast<MosaicFT &>(*iftm_p[0]));
  os << LogIO::WARN << "Mosaic psf is off. \nCould be no pointing in center of image \n"
     << "Will retry to make an approximate one without primary beam "
     << LogIO::POST;
  MPosition loc=iftm_p[0]->getLocation();
  ftm_p[0]=new GridFT(1000000, 16, "SF", loc, 1.0, False);
  iftm_p[0]=new GridFT(1000000, 16, "SF", loc, 1.0, False);
  ft_=&(*ftm_p[0]);
  ift_=&(*iftm_p[0]);
  // try again with simple ftmachines
  makeSimplePSF(psfs);
  //that 's the best psf you'd get
  //restore back MosaicFT machinas
  ftm_p[0]=ft_back;
  ft_=ft_back;
  iftm_p[0]=ift_back;
  ift_=ift_back;
}

void CubeSkyEquation::makeSimplePSF(PtrBlock<ImageInterface<Float> * >& psfs) {

    Int nmodels=psfs.nelements();
    LogIO os(LogOrigin("LOFAR::LofarFT::CubeSkyEquation", "makeSimplePSF"));
    SigHandler myStopSig;
    ft_->setNoPadding(noModelCol_p);
    isPSFWork_p= True; // avoid PB correction etc for PSF estimation
    Bool doPSF=True;
    Bool changedVI=False;
    // Initialize the gradients
    sm_->initializeGradients();
    VisibilityIterator& vi(*rvi_p);
    //Lets get the channel selection for later use
    vi.getChannelSelection(blockNumChanGroup_p, blockChanStart_p,
                           blockChanWidth_p, blockChanInc_p, blockSpw_p);
    // Reset the various SkyJones
    resetSkyJones();
    checkVisIterNumRows(vi);
    // Loop over all visibilities and pixels
    CountedPtr<VisBuffer> vb (new VisBuffer(vi));
    vi.originChunks();
    vi.origin();
    // Change the model polarization frame
    for (Int model=0; model < nmodels; ++model){
        if(vb->polFrame()==MSIter::Linear) {
            StokesImageUtil::changeCStokesRep(sm_->cImage(model),
                                              StokesImageUtil::LINEAR);
        }
        else {
            StokesImageUtil::changeCStokesRep(sm_->cImage(model),
                                              StokesImageUtil::CIRCULAR);
        }
    }


    Int nCubeSlice=1;
    isLargeCube(sm_->cImage(0), nCubeSlice);
    for (Int cubeSlice=0; cubeSlice< nCubeSlice; ++cubeSlice){
        changedVI= getFreqRange(vi, sm_->cImage(0).coordinates(),
                                cubeSlice, nCubeSlice) || changedVI;
        vi.originChunks();
        vi.origin();
        vb->invalidate();
        Int cohDone=0;
        ProgressMeter pm(1.0, Double(vb->numberCoh()),
                         "Gridding weights for PSF",
                         "", "", "", True);

        initializePutSlice(* vb, doPSF, cubeSlice, nCubeSlice);

        for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
            for (vi.origin(); vi.more(); vi++) {
	      if(myStopSig.gotStopSignal())
		throw(AipsError("Terminating...."));
                if(noModelCol_p) {
                    //This here forces the modelVisCube shape and prevents reading model column
                    vb->setModelVisCube(Complex(0.0,0.0));
                }
                putSlice(* vb, doPSF, FTMachine::MODEL, cubeSlice, nCubeSlice);
                cohDone += vb->nRow();
                pm.update(Double(cohDone));

            }
        }
        finalizePutSlice(* vb, doPSF, cubeSlice, nCubeSlice);
    }

    //Don't need these for now
    for(Int model=0; model < nmodels; ++model)
    {
      sm_->work(model).clearCache();
      sm_->cImage(model).clearCache();
    }

    //lets return original selection back to iterator

    if(changedVI)
        vi.selectChannel(blockNumChanGroup_p, blockChanStart_p,
                         blockChanWidth_p, blockChanInc_p, blockSpw_p);
    sm_->finalizeGradients();
    fixImageScale();
    for(Int model=0; model < nmodels; ++model){
      {
	//Normalize the gS image
	Int nXX=sm_->ggS(model).shape()(0);
	Int nYY=sm_->ggS(model).shape()(1);
	Int npola= sm_->ggS(model).shape()(2);
	Int nchana= sm_->ggS(model).shape()(3);
	IPosition blc(4,nXX, nYY, npola, nchana);
	IPosition trc(4, nXX, nYY, npola, nchana);
	blc(0)=0; blc(1)=0; trc(0)=nXX-1; trc(1)=nYY-1;
	//max weights per plane
	for (Int j=0; j < npola; ++j){
	  for (Int k=0; k < nchana ; ++k){
	    
	    blc(2)=j; trc(2)=j;
	    blc(3)=k; trc(3)=k;
	    Slicer sl(blc, trc, Slicer::endIsLast);
	    SubImage<Float> gSSub(sm_->gS(model), sl, False);
	    SubImage<Float> ggSSub(sm_->ggS(model), sl, False);
	    SubImage<Float> psfSub(*(psfs[model]), sl, True);
	    Float planeMax;
	    LatticeExprNode LEN = max( ggSSub );
	    planeMax =  LEN.getFloat();
	    if(planeMax !=0){
	      psfSub.copyData( (LatticeExpr<Float>)
			       (iif(ggSSub > (0.0),
				    (gSSub/planeMax),0.0)));
	    }
	    else{
	      psfSub.set(0.0);
	    }
	  }
	}
	//
      }

      LatticeExprNode maxPSF=max(*psfs[model]);
      Float maxpsf=maxPSF.getFloat();
        if(abs(maxpsf-1.0) > 1e-3) {
	  os << "Maximum of approximate PSF for field " << model << " = "
	     << maxpsf << " : renormalizing to unity" <<  LogIO::POST;
        }
        if(maxpsf > 0.0 ){
	  LatticeExpr<Float> len((*psfs[model])/maxpsf);
	  psfs[model]->copyData(len);
        }
        else{
	  if(sm_->numberOfTaylorTerms()>1) { /* MFS */
	    os << "PSF calculation resulted in a PSF with its peak being 0 or less. This is ok for MS-MFS." << LogIO::POST;
	  }
	  else{
	    throw(PSFZero("SkyEquation:: PSF calculation resulted in a PSF with its peak being 0 or less!"));
	  }
        }
	
	sm_->PSF(model).clearCache();
	sm_->gS(model).clearCache();
	sm_->ggS(model).clearCache();
    }

    isPSFWork_p=False; // resetting this flag so that subsequent calculation uses
    // the right SkyJones correction;
}

void CubeSkyEquation::gradientsChiSquared(Bool /*incr*/, Bool commitModel){
    AlwaysAssert(cft_, AipsError);
    AlwaysAssert(sm_, AipsError);
    //AlwaysAssert(vs_, AipsError);
    Bool initialized=False;
    Bool changedVI=False;

    Bool incremental = False;
    predictComponents(incremental, initialized);
    Bool predictedComp=initialized;

    SigHandler myStopSig;
    ////if people want to use model but it isn't there

    if(!noModelCol_p)
      noModelCol_p=rvi_p->msColumns().modelData().isNull();

    ROVisibilityIterator * oldRvi = NULL;
    VisibilityIterator * oldWvi = NULL;

    //    Timers tInitGrad=Timers::getTime();
    sm_->initializeGradients();
    //Lets get the channel selection for later use
    //    Timers tGetChanSel=Timers::getTime();
    rvi_p->getChannelSelection(blockNumChanGroup_p, blockChanStart_p,
                               blockChanWidth_p, blockChanInc_p, blockSpw_p);
    //    Timers tCheckVisRows=Timers::getTime();
    checkVisIterNumRows(*rvi_p);
    CountedPtr<VisBuffer> vb (new VisBuffer(*rvi_p));

    Bool isEmpty=True;
    for (Int model=0; model < (sm_->numberOfModels());++model){
        isEmpty=isEmpty &&  sm_->isEmpty(model);
    }
    // Now do the images

    for (Int model=0;model< (sm_->numberOfModels()); ++model) 
    {
        // Don't bother with empty images
        // Change the model polarization frame
        if(vb->polFrame()==MSIter::Linear) 
        {
            StokesImageUtil::changeCStokesRep(sm_->cImage(model),
                                              StokesImageUtil::LINEAR);
        }
        else 
        {
            StokesImageUtil::changeCStokesRep(sm_->cImage(model),
                                              StokesImageUtil::CIRCULAR);
        }
        scaleImage(model);
        // Reset the various SkyJones
    }
    //    Timers tChangeStokes=Timers::getTime();

    ft_=&(*ftm_p[0]);
    resetSkyJones();
    firstOneChangesPut_p=False;
    firstOneChangesGet_p=False;

    Int nCubeSlice=1;

    isLargeCube(sm_->cImage(0), nCubeSlice);

    for (Int cubeSlice=0; cubeSlice< nCubeSlice; ++cubeSlice)
    {
        changedVI= getFreqRange(*rvi_p, sm_->cImage(0).coordinates(),
                                cubeSlice, nCubeSlice) || changedVI;

        rvi_p->originChunks();
        rvi_p->origin();
        Bool useCorrected= !(vb->msColumns().correctedData().isNull());

        if( ! isEmpty )
        {
            initializeGetSlice(* vb, 0, False, cubeSlice, nCubeSlice);
        }
        //	Timers tInitPutSlice=Timers::getTime();
        initializePutSlice(* vb, False, cubeSlice, nCubeSlice);
        //	Timers tDonePutSlice=Timers::getTime();
        Int cohDone=0;
        ProgressMeter pm(1.0, Double(vb->numberCoh()),
                         "Gridding residual",
                         "", "", "", True);
        Int oldmsid=-1;
        for (rvi_p->originChunks();rvi_p->moreChunks();rvi_p->nextChunk()) {
            for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++) 
            {
                if(myStopSig.gotStopSignal())
                throw(AipsError("Terminating..."));
                //	      Timers tInitModel=Timers::getTime();
                if(!incremental && !predictedComp) {
                    //This here forces the modelVisCube shape and prevents reading model column
                    vb->setModelVisCube(Complex(0.0,0.0));
                }
                // get the model visibility and write it to the model MS
                //	Timers tGetSlice=Timers::getTime();

                //		Timers tgetSlice=Timers::getTime();
		if( vb->newMS() )
		  {
		    useCorrected = !(vb->msColumns().correctedData().isNull());
		  }

                if(!isEmpty)
                    getSlice(* vb, (predictedComp || incremental), cubeSlice, nCubeSlice);
		if(!useCorrected)
		  vb->visCube();
		else
		  vb->correctedVisCube();

                // Now lets grid the -ve of residual
                // use visCube if there is no correctedData
                //		Timers tGetRes=Timers::getTime();
                if (!iftm_p[0]->canComputeResiduals())
                    if(!useCorrected) vb->modelVisCube()-=vb->visCube();
                    else              vb->modelVisCube()-=vb->correctedVisCube();
                else
                    iftm_p[0]->ComputeResiduals(*vb,useCorrected);

                putSlice(* vb, False, FTMachine::MODEL, cubeSlice, nCubeSlice);
                cohDone+=vb->nRow();
                pm.update(Double(cohDone));
            }
        }

        finalizeGetSlice();
        if(!incremental&&!initialized) initialized=True;
        finalizePutSlice(* vb, False, cubeSlice, nCubeSlice);
    }

    for (Int model=0;model<sm_->numberOfModels();model++) 
    {
      sm_->cImage(model).clearCache();
      sm_->gS(model).clearCache();
      sm_->ggS(model).clearCache();
      sm_->work(model).clearCache();
      
      unScaleImage(model);

    }
    ft_=&(*ftm_p[0]);

    this->fixImageScale();
    //lets return original selection back to iterator
    if(changedVI)
        rvi_p->selectChannel(blockNumChanGroup_p, blockChanStart_p,
                             blockChanWidth_p, blockChanInc_p, blockSpw_p);
}


void CubeSkyEquation::isLargeCube(
  ImageInterface<Complex>& theIm, 
  Int& nslice) 
{
  //non-cube
  if(theIm.shape()[3]==1){
    nslice=1;
  }
  else{
    Long npix=theIm.shape().product();
    // use memory size denfined in aisprc if exists
    Long memtot=HostInfo::memoryTotal(true); // Use aipsrc/casarc
    //check for 32 bit OS and limit it to 2Gbyte
    if( sizeof(void*) == 4){
      if(memtot > 2000000)
	memtot=2000000;
    }
    if(memtot < 512000){
      ostringstream oss;
      oss << "The amount of memory reported " << memtot << " kB is too small to work with" << endl;
      throw(AipsError(String(oss))); 

    }
    Long pixInMem=(memtot/8)*1024;
    // cerr << "CSE: " << memtot << " " << pixInMem << endl;
    // cerr << npix << " " << pixInMem/8 << endl;
    nslice=1;
    //There are roughly 13 float images worth held in memory
    //plus some extra
    if(npix > (pixInMem/25)){
      //Lets slice it so grid is at most 1/25th of memory
      pixInMem=pixInMem/25;
      //One plane is
      npix=theIm.shape()(0)*theIm.shape()(1)*theIm.shape()(2);
      nchanPerSlice_p=Int(floor(pixInMem/npix));
      // cerr << "Nchan " << nchanPerSlice_p << " " << pixInMem << " " << npix << " " << pixInMem/npix << endl;
      if (nchanPerSlice_p==0){
	nchanPerSlice_p=1;
      }
      nslice=theIm.shape()(3)/nchanPerSlice_p;
      if( (theIm.shape()(3) % nchanPerSlice_p) > 0)
	++nslice;
    }
  }
}

void CubeSkyEquation::initializePutSlice(
  const VisBuffer& vb, 
  Bool dopsf, 
  Int cubeSlice, 
  Int nCubeSlice) 
{
  AlwaysAssert(ok(),AipsError);
  Bool dirDep= (ej_ != NULL);

  Int ntaylors=sm_->numberOfTaylorTerms(),
    nfields = sm_->numberOfModels()/ntaylors;

  for(Int field=0; field < nfields ; ++field){

    Int ntaylors = sm_->numberOfTaylorTerms();
    if(dopsf) ntaylors = 2 * sm_->numberOfTaylorTerms() - 1;

    Block<CountedPtr<ImageInterface<Complex> > > imPutSliceVec(ntaylors);
    Block<Matrix<Float> > weightSliceVec(ntaylors);
    for(Int taylor=0; taylor < ntaylors ; ++taylor) 
      {
	Int model = sm_->getModelIndex(field,taylor);
	sliceCube(imPutSlice_p[model], model, cubeSlice, nCubeSlice, 0);
	weightSlice_p[model].resize();
	imPutSliceVec[taylor] = imPutSlice_p[model];
	weightSliceVec[taylor] = weightSlice_p[model];
      }

    if(nCubeSlice>1){
      iftm_p[field]->reset();
    }
    //U// cout << "CubeSkyEqn :: Calling new initializeToSky with dopsf " << dopsf << endl;
    iftm_p[field]->initializeToSky(imPutSliceVec, weightSliceVec,vb,dopsf);
    dirDep= dirDep || (ftm_p[field]->name() == "MosaicFT");
  }// end of field
  assertSkyJones(vb, -1);
  //vb_p is used to finalize things if vb has changed propoerties
  vb_p->assign(vb, False);
  vb_p->updateCoordInfo(& vb, dirDep);
}

void CubeSkyEquation::getCoverageImage(Int model, ImageInterface<Float>& im){
  if ((sm_->doFluxScale(model)) && (ftm_p.nelements() > uInt(model))){
    ftm_p[model]->getFluxImage(im);
  }

}

void CubeSkyEquation::getWeightImage(
  Int model, 
  ImageInterface<Float>& im)
{
  if (iftm_p.nelements() > uInt(model)){
    Matrix<Float> weights;
    iftm_p[model]->getWeightImage(im, weights);
  }
}

void CubeSkyEquation::putSlice(
  VisBuffer & vb, 
  Bool dopsf, 
  FTMachine::Type col, 
  Int cubeSlice, 
  Int nCubeSlice) 
{
  AlwaysAssert(ok(),AipsError);
  Int nRow=vb.nRow();
  for (Int model=0; model<sm_->numberOfModels()/( sm_->numberOfTaylorTerms()); ++model)
  {
    iftm_p[model]->put(vb, -1, dopsf, col);
  }
}

void CubeSkyEquation::finalizePutSlice(
  const VisBuffer& vb,  
  Bool dopsf,
  Int cubeSlice, 
  Int nCubeSlice) 
{
  //============================================================================
  // NEW CODE BEGINS
  // Iterate across fields
  LogIO os(LogOrigin("CubeSkyEquation", "FinalizePutSlice"));
    {
      
      sm_->setImageNormalization(True);
      for (Int field=0; field < sm_->numberOfModels()/sm_->numberOfTaylorTerms(); ++field)
      {
        ft_=&(*ftm_p[field]);
        ift_=&(*iftm_p[field]);

        // Number of Taylor terms per field
        Int ntaylors = sm_->numberOfTaylorTerms();
        if(dopsf) ntaylors = 2 * sm_->numberOfTaylorTerms() - 1;

        // Build a list of reference images to send into FTMachine
        PtrBlock<SubImage<Float> *> gSSliceVec(ntaylors);
        PtrBlock<SubImage<Float> *> ggSSliceVec(ntaylors);
        PtrBlock<SubImage<Float> *> fluxScaleVec(ntaylors);
        Block<CountedPtr<ImageInterface<Complex> > > imPutSliceVec(ntaylors);
        Block<Matrix<Float> > weightSliceVec(ntaylors); // this is by value
        for (Int taylor=0; taylor < ntaylors; ++taylor)
        {
          Int model = sm_->getModelIndex(field,taylor);
          sliceCube(gSSliceVec[taylor], sm_->gS(model), cubeSlice, nCubeSlice);
          sliceCube(ggSSliceVec[taylor], sm_->ggS(model), cubeSlice, nCubeSlice);
          sliceCube(fluxScaleVec[taylor], sm_->fluxScale(model), cubeSlice, nCubeSlice);
          imPutSliceVec[taylor] = imPutSlice_p[model];
          weightSliceVec[taylor] = weightSlice_p[model];
        }// end of taylor

        // Call finalizeToSky for this field.
        // -- calls getImage, getWeightImage, does Stokes conversion, and gS/ggS normalization
        //U// cout << "CubeSkyEqn :: calling new finalizeToSky with dopsf " << dopsf << endl;
        iftm_p[field]->finalizeToSky( imPutSliceVec , gSSliceVec , ggSSliceVec , fluxScaleVec, dopsf , weightSliceVec, vb );
        
        // Clean up temporary reference images      
        for (Int taylor=0; taylor < ntaylors; ++taylor)
        {
          Int model = sm_->getModelIndex(field,taylor);
          weightSlice_p[model] = weightSliceVec[taylor]; // because this is by value...
          delete gSSliceVec[taylor]; 
          delete ggSSliceVec[taylor];
          delete fluxScaleVec[taylor];
        }
      }// end of field

//       tmpWBNormalizeImage(dopsf,ft_->getPBLimit());
      
      ft_=&(*ftm_p[0]);
      ift_=&(*iftm_p[0]);
      // 4. Finally, we add the statistics
      sm_->addStatistics(sumwt, chisq);
    }
}


void CubeSkyEquation::initializeGetSlice(
  const VisBuffer& vb, 
  Int /*row*/, 
  Bool incremental, 
  Int cubeSlice, 
  Int nCubeSlice)
{
  //  imGetSlice_p.resize(sm_->numberOfModels(), True, False);
  //  for(Int field=0; field < sm_->numberOfFields(); ++field){
  sm_->setImageNormalization(True);
  imGetSlice_p.resize(sm_->numberOfModels(), True, False);
  for(Int model=0; model < sm_->numberOfModels()/sm_->numberOfTaylorTerms(); ++model)
    {
      if(nCubeSlice>1)
      ftm_p[model]->reset();
    }

  Int ntaylors=sm_->numberOfTaylorTerms(),
    nfields = sm_->numberOfModels()/ntaylors;
  
  for(Int field=0; field < nfields; ++field)
    {
      //the different apply...jones user ft_ and ift_
      ft_=&(*ftm_p[field]);
      ift_=&(*iftm_p[field]);
      
      Block<CountedPtr<ImageInterface<Complex> > > imGetSliceVec(sm_->numberOfTaylorTerms());
      PtrBlock<SubImage<Float> *> modelSliceVec(sm_->numberOfTaylorTerms());
      PtrBlock<SubImage<Float> *> weightSliceVec(sm_->numberOfTaylorTerms());
      PtrBlock<SubImage<Float> *> fluxScaleVec(sm_->numberOfTaylorTerms());
      Block<Matrix<Float> > weightVec(sm_->numberOfTaylorTerms()); // this is by value
      
      for(Int taylor=0; taylor < sm_->numberOfTaylorTerms(); ++taylor)
	{
	  Int model = sm_->getModelIndex(field,taylor);
	  // NEW : Do the applySkyJones slice-by-slice -- to make it go into initializeToVis :(
	  ///cerr << "Taylor, Model, Field: " << taylor << " " << model << " " << field << endl;
	  if(incremental)
	    sliceCube(modelSliceVec[taylor], sm_->deltaImage(model), cubeSlice, nCubeSlice);
	  else
	    sliceCube(modelSliceVec[taylor], sm_->image(model), cubeSlice, nCubeSlice);

	  sliceCube(fluxScaleVec[taylor], sm_->fluxScale(model), cubeSlice, nCubeSlice);
	  sliceCube(weightSliceVec[taylor], sm_->ggS(model), cubeSlice, nCubeSlice);
	  sliceCube(imGetSlice_p[model], model, cubeSlice, nCubeSlice, 1);
	  imGetSliceVec[taylor] = imGetSlice_p[model];
	  weightVec[taylor] = weightSlice_p[model];
	}// end of taylor
      
      //U// cout << "CubeSkyEquation :: Calling new initializeToVis with " << modelSliceVec.nelements() << " models and " << imGetSliceVec.nelements() << " complex grids " << endl;
      //U// LatticeExprNode LEN = max( *(modelSliceVec[0] ) );
      //U// cout << "CubeSkyEq  : Peak in image to be predicted : " << LEN.getFloat() << endl;
      
      ftm_p[field]->initializeToVis(imGetSliceVec, modelSliceVec, weightSliceVec, 
				    fluxScaleVec, weightVec,vb);
      
      for (Int taylor=0; taylor < sm_->numberOfTaylorTerms(); ++taylor)
	{
	  //     Int model = sm_->getModelIndex(field,taylor);
	  // weightSlice_p[model] = weightSliceVec[taylor]; // because this is by value...
	  delete modelSliceVec[taylor];
	  delete weightSliceVec[taylor];
	  delete fluxScaleVec[taylor];
	}
    }//end of field
  ft_=&(*ftm_p[0]);
  ift_=&(*iftm_p[0]);
}

void CubeSkyEquation::sliceCube(
  CountedPtr<ImageInterface<Complex> >& slice,
  Int model, 
  Int cubeSlice, 
  Int nCubeSlice, 
  Int typeOfSlice)
{
  IPosition blc(4,0,0,0,0);
  IPosition trc(4,sm_->cImage(model).shape()(0)-1,
		sm_->cImage(model).shape()(1)-1,sm_->cImage(model).shape()(2)-1,
		0);
  Int beginChannel=cubeSlice*nchanPerSlice_p;
  Int endChannel=beginChannel+nchanPerSlice_p-1;
  if(cubeSlice==(nCubeSlice-1))
    endChannel=sm_->image(model).shape()(3)-1;
  blc(3)=beginChannel;
  trc(3)=endChannel;
  sl_p=Slicer (blc, trc, Slicer::endIsLast);
  SubImage<Complex>* sliceIm= new SubImage<Complex>(sm_->cImage(model), sl_p, True); /// UUU changes to True
  //  cerr << "SliceCube: " << beginChannel << " " << endChannel << endl;
  if(typeOfSlice==0){    
    
    Double memoryMB=HostInfo::memoryFree()/1024.0/(5.0*(sm_->numberOfModels()));
    slice=new TempImage<Complex> (TiledShape(sliceIm->shape(), 
					     IPosition(4, casa::min(sliceIm->shape()(0)/4, 1000), casa::min(sliceIm->shape()(1)/4, 1000),sliceIm->shape()(2) , 1)), sliceIm->coordinates(), sm_->getMemoryUse() ? memoryMB: 0);
    
 
    //slice->setMaximumCacheSize((sliceIm->shape()[0])*(sliceIm->shape()[1])/4);
    slice->setMaximumCacheSize(sliceIm->shape().product());
    //slice.copyData(sliceIm);
    slice->set(Complex(0.0, 0.0));
    //slice->setCoordinateInfo(sm_->image(model).coordinates());
    delete sliceIm;
  }
  else{
    slice=sliceIm;
  }
  
}

void CubeSkyEquation::sliceCube(
  SubImage<Float>*& slice,
  ImageInterface<Float>& image, 
  Int cubeSlice, 
  Int nCubeSlice)
{
  IPosition blc(4,0,0,0,0);
  IPosition trc(4,image.shape()(0)-1,
		image.shape()(1)-1,image.shape()(2)-1,
		0);
  Int beginChannel=cubeSlice*nchanPerSlice_p;
  Int endChannel=beginChannel+nchanPerSlice_p-1;
  if(cubeSlice==(nCubeSlice-1))
    endChannel=image.shape()(3)-1;
  blc(3)=beginChannel;
  trc(3)=endChannel;
  sl_p=Slicer(blc, trc, Slicer::endIsLast);
  //writeable if possible
  slice=  new SubImage<Float> (image, sl_p, True);
}

VisBuffer& CubeSkyEquation::getSlice(
  VisBuffer& result,  
  Bool incremental,
  Int cubeSlice, 
  Int nCubeSlice) 
{
  Int nRow=result.nRow();
  
  result.modelVisCube(); // get the visibility so vb will have it
  VisBuffer vb(result); // method only called using writable VI so no ROVIA
  
  Int nmodels=sm_->numberOfModels()/( sm_->numberOfTaylorTerms());
  Bool FTChanged=ftm_p[0]->changed(vb);
  
  // we might need to recompute the "sky" for every single row, but we
  // avoid this if possible.
  internalChangesGet_p=False;  // Does this VB change inside itself?
  firstOneChangesGet_p=False;  // Has this VB changed from the previous one?
  if((ftm_p[0]->name() != "MosaicFT")    && (ftm_p[0]->name() != "PBWProjectFT") &&
     (ftm_p[0]->name() != "AWProjectFT") && (ftm_p[0]->name() != "AWProjectWBFT")) {
    changedSkyJonesLogic(result, firstOneChangesGet_p, internalChangesGet_p);
  }
  
  if(internalChangesGet_p || internalChangesPut_p) {
    if(internalChangesPut_p)
      internalChangesPut_p=False;
    // Yes there are changes within this buffer: go row by row.
    // This will automatically catch a change in the FTMachine so
    // we don't have to check for that.
    
    Matrix<Complex> refres;
    Matrix<Complex> refvb;
    for (Int row=0; row<nRow; row++) {
      finalizeGetSlice();
      initializeGetSlice(result, row, False, cubeSlice, 
			 nCubeSlice);
      if(incremental || (nmodels > 1)){
	for (Int model=0; model < nmodels; ++model){
	  ftm_p[model]->get(vb,row);
	  refvb.reference(vb.modelVisCube().xyPlane(row));
	  refres.reference(result.modelVisCube().xyPlane(row));
	  refres += refvb;
	}
      }
      else
	ftm_p[0]->get(result, row);
    }
  }
  else if (FTChanged || firstOneChangesGet_p || firstOneChangesPut_p) {
    if(firstOneChangesPut_p)
      firstOneChangesPut_p=False;
    // This buffer has changed wrt the previous buffer, but
    // this buffer has no changes within it. Again we don't need to
    // check for the FTMachine changing.
    
    finalizeGetSlice();
    initializeGetSlice(result, 0, False, cubeSlice, nCubeSlice);
    if(incremental || (nmodels > 1)){
      for (Int model=0; model < nmodels; ++model){
	ftm_p[model]->get(vb);
	result.modelVisCube()+=vb.modelVisCube();
      }
    }
    else
      ftm_p[0]->get(result);
  }
  else {
    if(incremental || (nmodels >1)){
      for (Int model=0; model < nmodels; ++model){
	ftm_p[model]->get(vb);
	result.modelVisCube()+=vb.modelVisCube();
      }
    }
    else
      ftm_p[0]->get(result);
  }
  return result;
  
}

void CubeSkyEquation::finalizeGetSlice()
{
  //// place-holders.... there is nothing to do after degridding
  //for (Int model=0; model < sm_->numberOfModels(); ++model)
  //        ftm_p[model]->finalizeToVis();
}

Bool CubeSkyEquation::getFreqRange(
  VisibilityIterator& vi,
  const CoordinateSystem& coords,
  Int slice, 
  Int nslice)
{
  //bypass this for now
  return False;
    // Enforce that all SPWs are in the same frequency frame.
    //
    // If all the SPWs in the MS are in LSRK frame, we can do data
    // selection (since image is always in LSRK).
    //
    // If not all SPWs in the MS are in the same frequency frame and
    // in LSRK frame, for now, disable data selection since the
    // mapping between image (in LSRK) and MS channels will be time
    // variable.
    CountedPtr<VisBuffer> vb (new VisBuffer(vi));
    ROScalarMeasColumn<MFrequency> freqFrame=vb->msColumns().spectralWindow().refFrequencyMeas();
    uInt nrows=vb->msColumns().spectralWindow().nrow();
    String firstString = freqFrame(0).getRefString();
    Bool allFramesSame=True;
    for (uInt i=0;i<nrows;i++)
        if (freqFrame(i).getRefString() != firstString)
        {allFramesSame = False;break;}

    if (!allFramesSame || (firstString!="LSRK"))
        return False;

    // Only one slice lets keep what the user selected
    if(nslice==1)
        return False;

    Double start=0.0; 
    Double end=0.0;
    Double chanwidth=1.0;
    Int specIndex=coords.findCoordinate(Coordinate::SPECTRAL);
    SpectralCoordinate specCoord=coords.spectralCoordinate(specIndex);
    Vector<Int>spectralPixelAxis=coords.pixelAxes(specIndex);
    if(nchanPerSlice_p>0){
        specCoord.toWorld(start,Double(slice*nchanPerSlice_p)-0.5);
        specCoord.toWorld(end, Double(nchanPerSlice_p*(slice+1))+0.5);
        chanwidth=fabs(end-start)/Double(nchanPerSlice_p);
    }
    if(end < start){
        Double tempoo=start;
        start=end;
        end=tempoo;
    }

    Block<Vector<Int> > spwb;
    Block<Vector<Int> > startb;
    Block<Vector<Int> > nchanb;
    Block<Vector<Int> > incrb=blockChanInc_p;
    vi.getSpwInFreqRange(spwb, startb, nchanb, start, end, chanwidth);
    cerr << "CSE: " << start << " " << end << " " << chanwidth << endl
     	 << "     " << spwb[0] << " " << startb[0] << " " << nchanb[0] << " " << incrb[0] << endl;
    if(spwb.nelements()==0)
        return False;

    //cerr << "Original is " << blockChanStart_p[0] <<  "   " << blockChanWidth_p[0] << "  " <<  blockChanInc_p[0] << "   " 
    //	 <<  blockSpw_p[0] << endl;
    //vi.selectChannel(1, startb[0][0], nchanb[0][0], 1, spwb[0][0]); 
    vi.selectChannel(blockNumChanGroup_p, startb, nchanb, incrb, spwb); 

    return True;

}

void CubeSkyEquation::fixImageScale()
{
  LogIO os(LogOrigin("CubeSkyEquation", "fixImageScale"));
  
  // make a minimum value to ggS
  // This has the same effect as Sault Weighting, but 
  // is implemented somewhat differently.
  // We also keep the fluxScale(mod) images around to
  // undo the weighting.
  Float ggSMax=0.0;
  for (Int model=0;model<sm_->numberOfModels();model++) {
    
    LatticeExprNode LEN = max( sm_->ggS(model) );
    ggSMax =  max(ggSMax,LEN.getFloat());
  }
  ggSMax_p=ggSMax;
  Float ggSMin1;
  Float ggSMin2;
  
  ggSMin1 = ggSMax * constPB_p * constPB_p;
  ggSMin2 = ggSMax * minPB_p * minPB_p;
  
  for (Int model=0;model<sm_->numberOfModels()/( sm_->numberOfTaylorTerms() );model++) {
    if(ej_ || (ftm_p[model]->name() == "MosaicFT") ) {
      sm_->fluxScale(model).removeRegion ("mask0", RegionHandler::Any, False);
      if ((ftm_p[model]->name()!="MosaicFT")) {
	if(scaleType_p=="SAULT"){
	  
	  // Adjust flux scale to account for ggS being truncated at ggSMin1
	  // Below ggSMin2, set flux scale to 0.0
	  // FluxScale * image => true brightness distribution, but
	  // noise increases at edge.
	  // if ggS < ggSMin2, set to Zero;
	  // if ggS > ggSMin2 && < ggSMin1, set to ggSMin1/ggS
	  // if ggS > ggSMin1, set to 1.0
	  
	  sm_->fluxScale(model).copyData( (LatticeExpr<Float>) 
					  (iif(sm_->ggS(model) < (ggSMin2), 0.0,
					       sqrt((sm_->ggS(model))/ggSMin1) )) );
	  sm_->fluxScale(model).copyData( (LatticeExpr<Float>) 
					  (iif(sm_->ggS(model) > (ggSMin1), 1.0,
					       (sm_->fluxScale(model)) )) );
	  // truncate ggS at ggSMin1
	  sm_->ggS(model).copyData( (LatticeExpr<Float>) 
				    (iif(sm_->ggS(model) < (ggSMin1), ggSMin1*(sm_->fluxScale(model)), 
					 sm_->ggS(model)) )
				    );
	  
	}
	
	else
        {
	  
	  sm_->fluxScale(model).copyData( (LatticeExpr<Float>) 
					  (iif(sm_->ggS(model) < (ggSMin2), 0.0,
					       sqrt((sm_->ggS(model))/ggSMax) )) );
	  sm_->ggS(model).copyData( (LatticeExpr<Float>) 
				    (iif(sm_->ggS(model) < (ggSMin2), 0.0,
					 sqrt((sm_->ggS(model))*ggSMax) )) );
	  
	}
	
      }
      else 
      {
	
	Int nXX=sm_->ggS(model).shape()(0);
	Int nYY=sm_->ggS(model).shape()(1);
	Int npola= sm_->ggS(model).shape()(2);
	Int nchana= sm_->ggS(model).shape()(3);
	IPosition blc(4,nXX, nYY, npola, nchana);
	IPosition trc(4, nXX, nYY, npola, nchana);
	blc(0)=0; blc(1)=0; trc(0)=nXX-1; trc(1)=nYY-1; 
	
	//Those damn weights per plane can be wildly different so 
	//deal with it properly here
	for (Int j=0; j < npola; ++j)
        {
	  for (Int k=0; k < nchana ; ++k)
          {
	    
	    blc(2)=j; trc(2)=j;
	    blc(3)=k; trc(3)=k;
	    Slicer sl(blc, trc, Slicer::endIsLast);
	    SubImage<Float> fscalesub(sm_->fluxScale(model), sl, True);
	    SubImage<Float> ggSSub(sm_->ggS(model), sl, True);
	    Float planeMax;
	    LatticeExprNode LEN = max( ggSSub );
	    planeMax =  LEN.getFloat();
	    
	    ///////////
	    LatticeExprNode LEN1 = min( ggSSub );
	    os << LogIO::DEBUG1
	       << "Max " << planeMax << " min " << LEN1.getFloat() << LogIO::POST;
	    
	    //////////
	    ///As we chop the image later...the weight can vary per channel
	    ///lets be conservative and go to 1% of ggsMin2
	    if(planeMax !=0)
            {
	      if(doflat_p){
		fscalesub.copyData( (LatticeExpr<Float>) 
				    (iif(ggSSub < (ggSMin2/100.0), 
					 0.0, sqrt(ggSSub/planeMax))));
		ggSSub.copyData( (LatticeExpr<Float>) 
				 (iif(ggSSub < (ggSMin2/100.0), 0.0, 
				      sqrt(planeMax*ggSSub))));
	      }
	      else{
		fscalesub.copyData( (LatticeExpr<Float>) 
				    (iif(ggSSub < (ggSMin2/100.0), 
					 0.0, (ggSSub/planeMax))));
		ggSSub.copyData( (LatticeExpr<Float>) 
				 (iif(ggSSub < (ggSMin2/100.0), 0.0, 
				      (planeMax))));
	      }
	    }
	  }
	}
      }
      sm_->fluxScale(model).clearCache();
      sm_->ggS(model).clearCache();
    }
  }
}

void CubeSkyEquation::tmpWBNormalizeImage(Bool& dopsf, const Float& pbLimit)
{
  LogIO os(LogOrigin("CubeSkyEquation", "tmpNormalizeImage"));

  if (dopsf) return;

  Int nCubeSlice;
  // Number of Taylor terms per field
  Int ntaylors = sm_->numberOfTaylorTerms();
  isLargeCube(sm_->cImage(0), nCubeSlice);
  
  // PSFs are normalized in makeApproxPSF()
  if(dopsf) ntaylors = 2 * sm_->numberOfTaylorTerms() - 1;
  
  Int nfields = sm_->numberOfModels()/ntaylors;
  
  for (Int cubeSlice=0; cubeSlice<nCubeSlice;cubeSlice++)
    {
    for (Int field=0; field<nfields; field++)
      {
	Int baseindex = sm_->getModelIndex(field,0); // field,taylorterm

	SubImage<Float> *ggSSliceVec;
	sliceCube(ggSSliceVec, sm_->ggS(baseindex), cubeSlice, nCubeSlice);
	
	for (Int taylor=0; taylor < ntaylors; ++taylor)
	  {
	    Int index = sm_->getModelIndex(field, taylor);
	    
	    SubImage<Float> *gSSliceVec;
	    sliceCube(gSSliceVec, sm_->gS(index), cubeSlice, nCubeSlice);
	    
	    //
	    // If the FTM is NewMultiTermFT and is configure to not
	    // apply PB corrections, don't apply the PB correction
	    // here either.
	    //
	    LatticeExpr<Float> le;
	    if ((ft_->name()=="NewMultiTermFT"))
	      {
		if (((NewMultiTermFT *)ft_)->getDOPBCorrection())
		  {
		    //le=LatticeExpr<Float>(iif((*ggSSliceVec)>(pbLimit), (*gSSliceVec)/(sqrt(*ggSSliceVec)), 0.0)); // The negative sign is in FTM::normalizeImage()
		     le=LatticeExpr<Float>(iif((*ggSSliceVec)>(pbLimit), (*gSSliceVec)/((*ggSSliceVec)), 0.0)); // The negative sign is in FTM::normalizeImage()
		    gSSliceVec->copyData(le);
		  }
	      }
	    else
	      {
		//le=LatticeExpr<Float>(iif((*ggSSliceVec)>(pbLimit), (*gSSliceVec)/(sqrt(*ggSSliceVec)), 0.0)); // The negative sign is in FTM::normalizeImage()
		le=LatticeExpr<Float>(iif((*ggSSliceVec)>(pbLimit), (*gSSliceVec)/((*ggSSliceVec)), 0.0)); // The negative sign is in FTM::normalizeImage()
		gSSliceVec->copyData(le);
	      }
	   delete gSSliceVec;
	  }
	  delete ggSSliceVec;

      }
    }
}

} // end namespace LofarFT
} // end namespace LOFAR
