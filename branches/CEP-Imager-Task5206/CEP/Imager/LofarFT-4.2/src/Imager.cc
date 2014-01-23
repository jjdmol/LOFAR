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
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/MSVis/VisSet.h>
#include <LofarFT/CubeSkyEquation.h>

#include <tables/Tables/TableIter.h>
#include <assert.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
// @brief Imager for LOFAR data correcting for DD effects

Imager::Imager (MeasurementSet& ms, const Record& parameters)
  : casa::Imager(ms,false, true),
    itsParameters (parameters),
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


  if (itsParameters.asBool("splitbeam")) {
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
    itsFTMachine = FTMachineFactory::instance().create("FTMachineSimple", *ms_p, itsParameters);
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
  if(itsParameters.asInt("RowBlock")>0){
    rvi_p->setRowBlocking (itsParameters.asInt("RowBlock"));
  };
  
  return True;
}

void Imager::setSkyEquation()
{
  se_p = new CubeSkyEquation(*sm_p, *lofar_rvi_p, *ft_p, *cft_p,
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
if (itsParameters.asInt("StepApplyElement"))
{
    timeInterval = itsParameters.asDouble("TWElement");
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

// Show the relative timings of the various steps.
void Imager::showTimings (std::ostream&, double duration) const
{
  if (itsFTMachine) 
  {
    itsFTMachine->showTimings (cout, duration);
  }
}


} // end namespace LofarFT
} // end namespace LOFAR
