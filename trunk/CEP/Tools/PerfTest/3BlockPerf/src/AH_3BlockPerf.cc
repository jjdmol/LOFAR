//  AH_3BlockPerf.cc: Concrete Simulator class for performance measurements on a
//             simple source-heat-dest line.
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include <3BlockPerf/AH_3BlockPerf.h>
#include <3BlockPerf/WH_Src.h>
#include <3BlockPerf/WH_Heat.h>
#include <3BlockPerf/WH_Dest.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/ApplicationHolder.h>
#include <CEPFrame/WH_Empty.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Mem.h>
#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

namespace LOFAR {

AH_3BlockPerf::AH_3BlockPerf()
{
  itsSrcStep = 0;
  itsHeatStep = 0;
  itsDstStep = 0;
}

AH_3BlockPerf::~AH_3BlockPerf()
{
  undefine();
}

/**
   define function for the AH_3BlockPerf simulation. It defines a list
   of steps that each process a part of the data.
*/
void AH_3BlockPerf::define(const KeyValueMap& params)
{
  // free any memory previously allocated
  undefine();

  WH_Empty empty;

  Composite comp(empty, // workholder
		 "AH_3BlockPerf", // name
		 true,  // add name suffix
		 true,  // controllable	      
		 false); // monitor
  setComposite(comp);
  comp.runOnNode(0);
  comp.setCurAppl(0);

  // parameters to read:
  // - dataholder size
  // - amount of heat per byte
  unsigned int size = params.getInt("dataSize", 1024);
  unsigned int flopsPerByte = params.getInt("flopsPerByte", 1);
  unsigned int packetsPerMeasurement = params.getInt("packetsPerMeasurement", 10);
  // Create the Source Steps
  // Create the Source Step
  WorkHolder* srcWH = new WH_Src("SourceWH", size, packetsPerMeasurement, flopsPerByte);
  WorkHolder* heatWH = new WH_Heat("HeatWH", size, flopsPerByte);
  WorkHolder* dstWH = new WH_Dest("DestinationWH", size);
  itsSrcStep = new Step(srcWH, "srcStep", 1, 0);
  itsHeatStep = new Step(heatWH, "srcStep", 1, 0);
  itsDstStep = new Step(dstWH, "srcStep", 1, 0);

  delete srcWH;
  delete heatWH;
  delete dstWH;

  //  srcWH->runOnNode(0,0);
  itsSrcStep->runOnNode(0, 0);
  //  heatWH->runOnNode(1,0);
  itsHeatStep->runOnNode(1, 0);
  //  dstWH->runOnNode(2,0);
  itsDstStep->runOnNode(2, 0);

  comp.addStep(itsSrcStep);
  comp.addStep(itsHeatStep);
  comp.addStep(itsDstStep);

  // set synchronisity of steps
#if 0
  itsSrcStep->setOutBufferingProperties(0, false, false);
  itsHeatStep->setOutBufferingProperties(0, false, false);
  itsHeatStep->setInBufferingProperties(0, false, false);
  itsDstStep->setInBufferingProperties(0, false, false);
#endif

#ifdef HAVE_MPI
  itsHeatStep->connect(itsSrcStep,
		       0, // thisDHIndex
		       0, // thatDHIndex
		       1, // no of DH's
		       TH_MPI(itsSrcStep->getNode(), itsHeatStep->getNode()),
		       true);
  itsDstStep->connect(itsHeatStep,
		      0, // thisDHIndex
		      0, // thatDHIndex
		      1, // no of DH's
		      TH_MPI(itsHeatStep->getNode(), itsDstStep->getNode()),
		      true);
#else
  itsHeatStep->connect(itsSrcStep,
		       0, // thisDHIndex
		       0, // thatDHIndex
		       1, // no of DH's
		       TH_Mem(),
		       false);
  itsDstStep->connect(itsHeatStep,
		      0, // thisDHIndex
		      0, // thatDHIndex
		      1, // no of DH's
		      TH_Mem(),
		      false);
#endif
}

void AH_3BlockPerf::run(int nSteps) {
  Step::clearEventCount();

  for (int i=0; i<nSteps; i++) {
    getComposite().process();
  }
}

void AH_3BlockPerf::dump() const {
  getComposite().dump();
}

void AH_3BlockPerf::quit() {  
}

void AH_3BlockPerf::undefine() {
  delete itsSrcStep;
  delete itsHeatStep;
  delete itsDstStep;
}

}
