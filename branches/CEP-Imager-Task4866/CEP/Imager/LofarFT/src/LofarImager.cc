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
#include <LofarFT/LofarImager.h>
#include <LofarFT/LofarVisResampler.h>
#include <casa/Utilities/CountedPtr.h>
#include <synthesis/MeasurementComponents/SimpleComponentFTMachine.h>
#include <msvis/MSVis/VisSet.h>
#include <LofarFT/LofarCubeSkyEquation.h>

#include <tables/Tables/TableIter.h>
#include <assert.h>

using namespace casa;

namespace LOFAR
{
  // @brief Imager for LOFAR data correcting for DD effects

  LofarImager::LofarImager (MeasurementSet& ms, const Record& parameters)
    : Imager(ms,false, true),
      itsParameters (parameters),
      itsMachine    (0),
      itsMachineOld (0)
  {
  }

  LofarImager::~LofarImager()
  {}

  Bool LofarImager::createFTMachine()
  {
    CountedPtr<VisibilityResamplerBase> visResampler;
    Bool useDoublePrecGrid = False;
    Double RefFreq = 0.0;
    if (sm_p) RefFreq = Double((*sm_p).getReferenceFrequency());


    if (itsParameters.asBool("splitbeam")) {
      cout << itsParameters<<endl;
      itsMachine = new LofarFTMachine(cache_p/2, tile_p,
                                      visResampler, gridfunction_p,
                                      *ms_p, wprojPlanes_p, mLocation_p,
                                      padding_p, false, useDoublePrecGrid,
                                      itsParameters.asDouble("wmax"),
                                      itsParameters.asInt("verbose"),
                                      itsParameters.asInt("maxsupport"),
                                      itsParameters.asInt("oversample"),
                                      itsParameters.asString("imagename"),
                                      itsParameters.asArrayBool("mueller.grid"),
                                      itsParameters.asArrayBool("mueller.degrid"),
                                      RefFreq,
                                      itsParameters.asBool("UseLIG"),
                                      itsParameters.asBool("UseEJones"),
                                      itsParameters.asInt("StepApplyElement"),
                                      itsParameters.asInt("ApplyBeamCode"),
                                      itsParameters.asDouble("PBCut"),
                                      itsParameters.asBool("PredictFT"),
                                      itsParameters.asString("PsfImage"),
                                      itsParameters.asBool("UseMasksDegrid"),
                                      itsParameters.asBool("doPSF"),
				      itsParameters.asDouble("UVmin"),
				      itsParameters.asDouble("UVmax"),
                                      itsParameters.asBool("MakeDirtyCorr"),
                                      itsParameters);//,
                                      //itsParameters.asDouble("FillFactor"));
    
      itsMachine->initGridThreads(itsGridsParallel,itsGridsParallel2);


      ft_p  = itsMachine;
    } else {
    itsMachineOld = new LofarFTMachineOld(cache_p/2, tile_p,
                                    visResampler, gridfunction_p,
                                    *ms_p, wprojPlanes_p, mLocation_p,
                                    padding_p, false, useDoublePrecGrid,
                                    itsParameters.asDouble("wmax"),
                                    itsParameters.asString("beam.element.path"),
                                    itsParameters.asInt("verbose"),
                                    itsParameters.asInt("maxsupport"),
                                    itsParameters.asInt("oversample"),
                                    itsParameters.asString("imagename"),
                                    itsParameters.asArrayBool("mueller.grid"),
                                    itsParameters.asArrayBool("mueller.degrid"));
      ft_p  = itsMachineOld;
    }

    cft_p = new SimpleComponentFTMachine();

    rvi_p->setRowBlocking (1000000);
    if(itsParameters.asInt("RowBlock")>0){
      rvi_p->setRowBlocking (itsParameters.asInt("RowBlock"));
    };
    
    return True;
  }

  void LofarImager::setSkyEquation()
  {
    se_p = new LofarCubeSkyEquation(*sm_p, *rvi_p, *ft_p, *cft_p,
                                    !useModelCol_p);

    return;
  }
  
void LofarImager::makeVisSet(MeasurementSet& ms, 
                        Bool compress, Bool mosaicOrder){

  if(rvi_p) {
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
  Double timeInterval = itsParameters.asDouble("TWElement");

  //if you want to use scratch col...make sure they are there
  if(useModelCol_p)
    VisSet(ms,sort,noselection,useModelCol_p,timeInterval,compress);
  
  if(imwgt_p.getType()=="none"){
      imwgt_p=VisImagingWeight("natural");
  }

  if(!useModelCol_p){
    rvi_p=new ROVisibilityIterator(ms, sort, timeInterval);
  }
  else{
    wvi_p=new VisibilityIterator(ms, sort, timeInterval);
    rvi_p=wvi_p;    
  }
  rvi_p->useImagingWeight(imwgt_p);

}
  

  // Show the relative timings of the various steps.
  void LofarImager::showTimings (std::ostream&, double duration) const
  {
    if (itsMachine) {
      itsMachine->showTimings (cout, duration);
    } else if (itsMachineOld) {
      itsMachineOld->showTimings (cout, duration);
    }
  }


} //# end namespace
