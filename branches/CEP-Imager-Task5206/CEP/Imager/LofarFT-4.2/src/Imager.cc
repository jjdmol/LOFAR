//# LofarImager.h: Imager for LOFAR data correcting for DD effects
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
//#
//# @author Ger van Diepen <diepen at astron dot nl>

#include <lofar_config.h>
#include <LofarFT/Imager.h>
#include <LofarFT/VisResampler.h>
#include <casa/Utilities/CountedPtr.h>
#include <synthesis/MeasurementComponents/WBCleanImageSkyModel.h>
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/MSVis/VisSet.h>
#include <LofarFT/SkyEquation.h>

#include <tables/Tables/TableIter.h>
#include <assert.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
// @brief Imager for LOFAR data correcting for DD effects

Imager::Imager (MeasurementSet& ms, const Record& parameters, LOFAR::ParameterSet& parset)
  : casa::Imager(ms,false, true),
    itsParameters (parameters),
    itsParset (parset),
    itsFTMachine    (0)
{}

Imager::~Imager()
{}

Bool Imager::createFTMachine()
{
  CountedPtr<VisibilityResamplerBase> visResampler;
  Bool useDoublePrecGrid = False;
  Double RefFreq = 0.0;
  if (sm_p) RefFreq = Double((*sm_p).getReferenceFrequency());

  cout << "splitbeam: " << itsParset.getBool("splitbeam",true) << endl;
  
  if (itsParset.getBool("splitbeam",true)) {
//     cout << itsParameters<<endl;
//     itsMachine = new FTMachine(
//       *ms_p, 
//       wprojPlanes_p, 
//       mLocation_p,
//       padding_p, 
//       false, 
//       useDoublePrecGrid,
//       RefFreq,
//       itsParameters);//,
//                                     //itsParameters.asDouble("FillFactor"));
//   
//     itsMachine->initGridThreads(itsGridsParallel,itsGridsParallel2);
// 
// 
//     ft_p  = itsMachine;
  } 
  else 
  {
//     itsFTMachine = FTMachineFactory::instance().create("FTMachineSimpleWB", *ms_p, itsParameters);
    itsFTMachine = FTMachineFactory::instance().create("FTMachineSplitBeamWStackWB", *ms_p, itsParameters, itsParset);
//     itsFTMachine = new FTMachineSimple(
//       *ms_p, 
//       wprojPlanes_p, 
//       mLocation_p,
//       padding_p, 
//       useDoublePrecGrid,
//       itsParameters);
    ft_p = itsFTMachine;
  }

  cft_p = new SimpleComponentFTMachine();

  rvi_p->setRowBlocking (1000000);
  if (itsParset.getInt("RowBlock",0)>0){
    rvi_p->setRowBlocking (itsParset.getInt("RowBlock",0));
  };
  
  return True;
}

void Imager::setSkyEquation()
{
  se_p = new SkyEquation(*sm_p, *lofar_rvi_p, *itsFTMachine, *cft_p,
                                  !useModelCol_p);

  return;
}

void Imager::makeVisSet(
  MeasurementSet& ms, 
  Bool compress, 
  Bool mosaicOrder)
{

  if(rvi_p) 
  {
      delete rvi_p;
      rvi_p=0;
      wvi_p=0;
  }

  Block<Int> sort(0);
  if(mosaicOrder){
      sort.resize(4);
      sort[0] = MS::FIELD_ID;
      sort[1] = MS::ARRAY_ID;
      sort[2] = MS::DATA_DESC_ID;
      sort[3] = MS::TIME;

  }
  //else use default sort order
  else{
      sort.resize(4);
      sort[0] = MS::ARRAY_ID;
      sort[1] = MS::FIELD_ID;
      sort[2] = MS::DATA_DESC_ID;
      sort[3] = MS::TIME;
  }
  Matrix<Int> noselection;
  Double timeInterval = 0;
  if (itsParset.getInt("ApplyElement",0))
  {
      timeInterval = itsParset.getDouble("TWElement",20.);
  }

  //if you want to use scratch col...make sure they are there
  if(useModelCol_p)
      VisSet(ms,sort,noselection,useModelCol_p,timeInterval,compress);

  if(imwgt_p.getType()=="none"){
      lofar_imwgt_p = VisImagingWeight("natural");
      imwgt_p = lofar_imwgt_p;
  }

  lofar_rvi_p = new VisibilityIterator(ms, sort, timeInterval);
  rvi_p = lofar_rvi_p;
  if(useModelCol_p){
      wvi_p=lofar_rvi_p;    
  }
  lofar_rvi_p->useImagingWeight(lofar_imwgt_p);

}

// Weight the MeasurementSet
Bool Imager::weight(const String& type, const String& rmode,
                    const Quantity& noise, const Double robust,
                    const Quantity& fieldofview,
                    const Int npixels, const Bool multiField)
{
  if(!valid()) return False;
  logSink_p.clearLocally();
  LogIO os(LogOrigin("imager", "weight()"),logSink_p);

  this->lock();
  try {
      
    os << LogIO::NORMAL // Loglevel INFO
    << "Weighting MS: Imaging weights will be changed" << LogIO::POST;
      
    if (type=="natural")
    {
      os << LogIO::NORMAL // Loglevel INFO
          << "Natural weighting" << LogIO::POST;
      lofar_imwgt_p = VisImagingWeight("natural");
    }
    else if(type=="superuniform")
    {
      if(!assertDefinedImageParameters()) return False;
      Int actualNpix=npixels;
      if(actualNpix <=0)
        actualNpix=3;
      os << LogIO::NORMAL // Loglevel INFO
          << "SuperUniform weighting over a square cell spanning [" 
          << -actualNpix 
          << ", " << actualNpix << "] in the uv plane" << LogIO::POST;
      lofar_imwgt_p = VisImagingWeight(*rvi_p, rmode, noise, robust, nx_p, 
                            ny_p, mcellx_p, mcelly_p, actualNpix, 
                            actualNpix, multiField);
    }
    else if ((type=="robust")||(type=="uniform")||(type=="briggs")) 
    {
      if(!assertDefinedImageParameters()) return False;
      Quantity actualFieldOfView(fieldofview);
      Int actualNPixels(npixels);
      String wtype;
      if(type=="briggs") 
      {
        wtype = "Briggs";
      }
      else 
      {
        wtype = "Uniform";
      }
      if(actualFieldOfView.get().getValue()==0.0&&actualNPixels==0) 
      {
        actualNPixels=nx_p;
        actualFieldOfView=Quantity(actualNPixels*mcellx_p.get("rad").getValue(),
                                                                "rad");
        os << LogIO::NORMAL // Loglevel INFO
          << wtype
          << " weighting: sidelobes will be suppressed over full image"
          << LogIO::POST;
      }
      else if(actualFieldOfView.get().getValue()>0.0&&actualNPixels==0) 
      {
        actualNPixels=nx_p;
        os << LogIO::NORMAL // Loglevel INFO
          << wtype
          << " weighting: sidelobes will be suppressed over specified field of view: "
          << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
      }
      else if(actualFieldOfView.get().getValue()==0.0&&actualNPixels>0) 
      {
        actualFieldOfView=Quantity(actualNPixels*mcellx_p.get("rad").getValue(),
                                                                "rad");
        os << LogIO::NORMAL // Loglevel INFO
        << wtype
        << " weighting: sidelobes will be suppressed over full image field of view: "
        << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
      }
      else 
      {
        os << LogIO::NORMAL // Loglevel INFO
        << wtype
        << " weighting: sidelobes will be suppressed over specified field of view: "
        << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
      }
      os << LogIO::DEBUG1
        << "Weighting used " << actualNPixels << " uv pixels."
        << LogIO::POST;
      Quantity actualCellSize(actualFieldOfView.get("rad").getValue()/actualNPixels, "rad");

      lofar_imwgt_p = VisImagingWeight(*rvi_p, rmode, noise, robust, 
                            actualNPixels, actualNPixels, actualCellSize, 
                            actualCellSize, 0, 0, multiField);
    
    }
    else if (type=="radial") 
    {
      os << "Radial weighting" << LogIO::POST;
      imwgt_p = VisImagingWeight("radial");
    }
    else 
    {
      this->unlock();
      os << LogIO::SEVERE << "Unknown weighting " << type
        << LogIO::EXCEPTION;    
      return False;
    }
    
    lofar_rvi_p->useImagingWeight(lofar_imwgt_p);
    
    // Beam is no longer valid
    beamValid_p=False;
    destroySkyEquation();
    this->writeHistory(os);
    this->unlock();
    return True;
  } catch (AipsError x) {
      this->unlock();
      os << LogIO::SEVERE << "Caught exception: " << x.getMesg()
      << LogIO::EXCEPTION;
      return False;
  } 

  return True;
}

Bool Imager::restoreImages(const Vector<String>& restored, Bool modresiduals)
{
  return casa::Imager::restoreImages(restored, modresiduals);
}

// Show the relative timings of the various steps.
void Imager::showTimings (std::ostream&, double duration) const
{
  if (itsFTMachine) 
  {
    itsFTMachine->showTimings (cout, duration);
  }
}

// Clean algorithm
Record Imager::clean(const String& algorithm,
                     const Int niter,
                     const Float gain,
                     const Quantity& threshold,
                     const Vector<String>& model,
                     const Vector<Bool>& fixed,
                     const String& complist,
                     const Vector<String>& mask,
                     const Vector<String>& image,
                     const Vector<String>& residual,
                     const Vector<String>& psfnames,
                     const Bool firstrun)
{
  Record retval;
  Bool converged=True;
  retval.define("converged", False);
  retval.define("iterations", Int(0));
  retval.define("maxresidual", Float(0.0));
  
  if(!valid())
    {
      return retval;
    }
  logSink_p.clearLocally();
  LogIO os(LogOrigin("imager", "clean()"),logSink_p);
  
  this->lock();
  try {
    if(!assertDefinedImageParameters()) 
      {
        return retval;
      }
    
    Int nmodels=model.nelements();
    os << LogIO::DEBUG1
       << "Found " << nmodels << " specified model images" << LogIO::POST;
    
    if(model.nelements()>0) {
      for (uInt thismodel=0;thismodel<model.nelements(); ++thismodel) {
        if(model(thismodel)=="") {
          this->unlock();
          os << LogIO::SEVERE << "Need a name for model "
             << thismodel << LogIO::POST;
          return retval;
        }
      }
    }
    
    Vector<String> modelNames=model;
    // Make first image with the required shape and coordinates only if
    // it doesn't exist yet. Otherwise we'll throw an exception later
    if(modelNames(0)=="") modelNames(0)=imageName()+".clean";
    if(!Table::isWritable(modelNames(0))) {
      make(modelNames(0));
    }
    else{
      Bool coordMatch=False;
      CoordinateSystem coordsys;
      //imagecoordinates(coordsys, firstrun);
      imagecoordinates2(coordsys, firstrun);
      for (uInt modelNum=0; modelNum < modelNames.nelements(); ++modelNum){
        if(Table::isWritable(modelNames(modelNum))){
          coordMatch= coordMatch || 
            (this->checkCoord(coordsys, modelNames(modelNum)));
        }
      } 
      if(!coordMatch){
        os << LogIO::WARN << "The model(s) image exists on disk " 
           << LogIO::POST;
        os << LogIO::WARN 
           << "The coordinates or shape were found not to match the one "
           << "defined by setimage " 
           << LogIO::POST;

        os << LogIO::WARN 
           << "Cleaning process is going to ignore setimage parameters and "
           << "continue cleaning from from model on disk " 
           << LogIO::POST;
      }
    }
    Vector<String> maskNames(nmodels);
    if(Int(mask.nelements())==nmodels) {
      maskNames=mask;
    }
    else {
      /* For msmfs, the one input mask PER FIELD must be replicated for all 
         Taylor-planes PER FIELD */
      if(algorithm=="msmfs" && (Int(mask.nelements())>=(nmodels/ntaylor_p)) ){
       for(Int tay=0;tay<nmodels;tay++)
         {
           maskNames[tay] = mask[ tay%(nmodels/ntaylor_p)  ];
         }
      }
      else {
         /* No mask */
         maskNames="";
      }
    }

    if(sm_p){
      if( sm_p->getAlgorithm() != "clean") destroySkyEquation();
      if(images_p.nelements() != uInt(nmodels)){
        destroySkyEquation();
      }
      else{
        for (Int k=0; k < nmodels ; ++k){
          if(!(images_p[k]->name().contains(modelNames[k]))) destroySkyEquation();
        }
      }
    }

    // Always fill in the residual images
    Vector<String> residualNames(nmodels);
    if(Int(residual.nelements())==nmodels) 
    {
        residualNames=residual;
    }
    else 
    {
      residualNames=""; 
    }
    for (Int thismodel=0; thismodel < Int(model.nelements()); ++thismodel) 
    {
      if(residualNames[thismodel]=="")
        residualNames(thismodel) = modelNames(thismodel)+".residual";
    }
    
    redoSkyModel_p = False; // added by vdtol to disable redoSkyMode, whatever that means
    
    if(redoSkyModel_p)
    {
      for (Int thismodel=0;thismodel<Int(model.nelements());++thismodel) 
      {
        removeTable(residualNames(thismodel));
        if(!clone(model(thismodel), residualNames(thismodel)))
        {
          return retval;
        }
      }
    }
    
    // Make an ImageSkyModel with the specified polarization representation
    // (i.e. circular or linear)

    if( redoSkyModel_p || !sm_p)
    {
      if(sm_p) delete sm_p;
      if (algorithm=="msmfs") 
      {
        doMultiFields_p = False;
        doWideBand_p = True;
        useNewMTFT_p=False;

        if (!scaleInfoValid_p) {
          this->unlock();
          os << LogIO::WARN << "Scales not yet set, using power law" << LogIO::POST;
          sm_p = new WBCleanImageSkyModel(ntaylor_p, 1 ,reffreq_p);
        }
        if (scaleMethod_p=="uservector") {      
          sm_p = new WBCleanImageSkyModel(ntaylor_p,userScaleSizes_p,reffreq_p);
        } else {
          sm_p = new WBCleanImageSkyModel(ntaylor_p,nscales_p,reffreq_p);
        }
        os << LogIO::NORMAL // Loglevel INFO
           << "Using multi frequency synthesis algorithm" << LogIO::POST;
        ((WBCleanImageSkyModel*)sm_p)->imageNames = Vector<String>(image);
        /* Check masks. Should be only one per field. Duplicate the name ntaylor_p times 
           Note : To store taylor-coefficients, msmfs uses the same data structure as for
                  multi-field imaging. In the case of multifield and msmfs, the list of 
                  images is nested and follows a field-major ordering.
                  All taylor-coeffs for a single field should have the same mask (for now).
           For now, since only single-field is allowed for msmfs, we have the following.*/
      }
      else {
        this->unlock();
        os << LogIO::SEVERE << "Unknown algorithm: " << algorithm 
           << LogIO::POST;
        return retval;
      }
   
      AlwaysAssert(sm_p, AipsError);
      sm_p->setAlgorithm("clean");

      //    if (!se_p)
      if(!createSkyEquation(modelNames, fixed, maskNames, complist)) 
        {
          return retval;
        }
      os << LogIO::NORMAL3 << "Created Sky Equation" << LogIO::POST;
    }
    else
    {
      //adding or modifying mask associated with skyModel
      addMasksToSkyEquation(maskNames,fixed);
    }
    //No need to add residuals will let sm_p use tmpimage ones and we'll copy them in restore 
    if(!addResiduals(residualNames))
       throw(AipsError("Problem in attaching to residual images")); 

    sm_p->setGain(gain);
    sm_p->setNumberIterations(niter);
    sm_p->setThreshold(threshold.get("Jy").getValue());
    sm_p->setCycleFactor(cyclefactor_p);
    sm_p->setCycleSpeedup(cyclespeedup_p);
    sm_p->setCycleMaxPsfFraction(cyclemaxpsffraction_p);
    {
      ostringstream oos;
      oos << "Clean gain = " <<gain<<", Niter = "<<niter<<", Threshold = "
          << threshold;
      os << LogIO::NORMAL << String(oos) << LogIO::POST; // More for the
                                                         // logfile than the
                                                         // log window.
    }
    os << LogIO::NORMAL << (firstrun ? "Start" : "Continu")
       << "ing deconvolution" << LogIO::POST; // Loglevel PROGRESS
    if(se_p->solveSkyModel()) 
    {
      os << LogIO::NORMAL
         << (niter == 0 ? "Image OK" : "Successfully deconvolved image")
         << LogIO::POST; // Loglevel PROGRESS
    }
    else 
    {
      converged=False;
      os << LogIO::NORMAL << "Threshhold not reached yet." << LogIO::POST; // Loglevel PROGRESS
    }
    printbeam(sm_p, os, firstrun);
    
    for (uInt k=0 ; k < residuals_p.nelements(); ++k){
      (residuals_p[k])->copyData(sm_p->getResidual(k));
    }
   
    retval.define("maxresidual", (sm_p->threshold()));
    retval.define("iterations", (sm_p->numberIterations()));
    retval.define("converged", converged);
    savePSF(psfnames);
    redoSkyModel_p=False;
    writeFluxScales(fluxscale_p);
    restoreImages(image);
    
    
    this->unlock();
    return retval;
  }
  catch (PSFZero&  x)
  {
    //os << LogIO::WARN << x.what() << LogIO::POST;
    savePSF(psfnames);
    this->unlock();
    throw(AipsError(String("PSFZero  ")+ x.getMesg() + String(" : Please check that the required data exists and is not flagged.")));
    return retval;
  }  
  catch (exception &x) 
  { 
    this->unlock();
    destroySkyEquation();
    throw(AipsError(x.what()));
    return retval;
  } 

  catch(...){
    cout << "Unknown exception" << endl;
    this->unlock();
    destroySkyEquation();
    //Unknown exception...
    throw;
  }
  this->unlock();

  os << LogIO::NORMAL << "Exiting Imager::clean" << LogIO::POST; // Loglevel PROGRESS
  return retval;
}

void Imager::predict(const Vector<String>& modelNames)
{
  cout << "I am Imager::predict" << endl;
  createSkyEquation(modelNames);
  cout << "calling se_p->predict" << endl;
  se_p->predict();
}


} // end namespace LofarFT
} // end namespace LOFAR
