//  StepRep.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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
//////////////////////////////////////////////////////////////////////

#include "CEPFrame/StepRep.h"
#include "CEPFrame/CompositeRep.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/DataManager.h"
#include TRANSPORTERINCLUDE
#include "Transport/TH_Mem.h"
#include "CEPFrame/Profiler.h"
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>    // for min,max
#ifdef HAVE_CORBA
#include "Transport/Corba/BS_Corba.h"
#include "Transport/Corba/CorbaMonitor.h"
#endif 

#include <sstream>
using namespace std;

namespace LOFAR
{

// this will give all instances of Step the same event in the Profiling output
int          StepRep::theirProcessProfilerState=0;
unsigned int StepRep::theirNextID=0;
unsigned int StepRep::theirEventCnt=0;


StepRep::StepRep (WorkHolder& worker, 
		  const string& name,
		  bool addNameSuffix,
		  bool monitor)
: itsRefCount   (1),
  itsWorker     (0),
  itsDataManager(0),
  itsParent     (0),
  itsID         (-1),
  itsAddSuffix  (addNameSuffix),
  itsSeqNr      (-1),
  itsName       (name),
  itsMonitor    (0)
{
  itsWorker = worker.baseMake();

  // Replace the tinyDataManager with a DataManager
  itsDataManager = new DataManager(itsWorker->getDataManager());
  itsWorker->setDataManager(itsDataManager);

  if (monitor) {
    TRACER2("Create controllable Composite " << name);
#ifdef HAVE_CORBA
    // Create a CorbaMonitor object    
    itsMonitor = new CorbaMonitor(BS_Corba::getPOA(),
				  BS_Corba::getPOAManager(),
				  name,
				  getWorker());
#else
    TRACER1("CORBA is not configured, so CorbaMonitor cannot be used in Composite ");
#endif 
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH_Process","yellow");
  }
  TRACER2("Create Step " << name << " ID = " << getID());

}

StepRep::~StepRep() 
{
  delete itsWorker; // WorkHolder will delete DataManager.
#ifdef HAVE_CORBA
  delete itsMonitor;
#endif
}


bool StepRep::isComposite() const
{
  return false;
}

void StepRep::setSeqNr (int seqNr)
{
  itsSeqNr = seqNr;
  if (itsAddSuffix) {
    ostringstream os;
    os << '_' << itsSeqNr;
    itsName += os.str();
  }
}


void StepRep::preprocess()
{
  itsWorker->basePreprocess();
}

void StepRep::process()
{
  // Call the baseProcess() method in the WorkHolder
  Profiler::enterState (theirProcessProfilerState);
  itsWorker->baseProcess();
  Profiler::leaveState (theirProcessProfilerState);
}

void StepRep::postprocess()
{
  itsWorker->basePostprocess();
}
	       
bool StepRep::connectData (const TransportHolder& prototype,
			   DataHolder& sourceData, 
			   DataHolder& targetData,
			   bool blockingComm)
{
  return sourceData.connectTo(targetData, prototype, blockingComm);
}


bool StepRep::connectRep (StepRep* aStep,
			  int   thisDHIndex,
			  int   thatDHIndex,
			  int   nrDH,
			  const TransportHolder& prototype,
			  bool blockingComm)
{
  // determine how much DataHolders to loop
  if (nrDH < 0) {
    nrDH = min(aStep->getWorker()->getDataManager().getOutputs(),
	       this->getWorker()->getDataManager().getInputs());
  }
  bool result=true;
  for (int i=0; i<nrDH; i++) {
    int thisInx = i + thisDHIndex;  // DataHolder nr in this Step
    int thatInx = i + thatDHIndex;  // DataHolder nr in aStep

    result &= connectData (prototype,
			   aStep->getOutData(thatInx),
			   *itsWorker->getDataManager().getGeneralInHolder(thisInx), 
			   blockingComm);
 
    TRACER2( "StepRep::connect " << getName().c_str() << " (ID = "
	   << getID() << ") DataHolder " << thisInx << " to "
	   << aStep->getName().c_str() 
	   << " (ID = " << aStep->getID() << ") DataHolder " << thatInx);
  }
  return result;
}

bool StepRep::connectInput (Step* aStep,
			    const TransportHolder& prototype,
			    bool blockingComm)
{
  return connectRep (aStep->getRep(), 0, 0, -1, prototype, blockingComm);
}

bool StepRep::connectInputArray (Step* aStep[],
				 int   nrSteps,
				 const TransportHolder& prototype,
				 bool blockingComm)
{
  if (aStep==NULL) return false;
  if (nrSteps < 0) {  // set nrSteps automatically
    nrSteps = getWorker()->getDataManager().getInputs();
  }
  int dhIndex=0;
  for (int item=0; item<nrSteps; item++) {
    AssertStr (getWorker()->getDataManager().getInputs() >= 
	       dhIndex+aStep[item]->getWorker()->getDataManager().getOutputs(),
	       "connect " << getName() << " - " << aStep[item]->getName() <<
	       "; not enough inputs");
    connectRep (aStep[item]->getRep(), dhIndex, 0, -1, prototype,
		blockingComm);
    dhIndex += aStep[item]->getWorker()->getDataManager().getOutputs();
  }
  if (dhIndex != getWorker()->getDataManager().getInputs()) {
    cerr << "StepRep::connectInputArray() - Warning:" << endl
	 << "  " << getName() << " - " << aStep[0]->getName() 
	 << ", unequal number of inputs and outputs" << endl;
  }
  return true;
}


bool StepRep::connectOutputArray (Step* aStep[],
				  int   nrSteps,
				  const TransportHolder& prototype,
				  bool blockingComm)
{
  if (aStep == NULL) {
    return false;
  }
  if (nrSteps < 0) {  // set nrSteps automatically
    nrSteps = getWorker()->getDataManager().getOutputs();
  }
  int dhIndex=0;
  for (int item=0; item<nrSteps; item++) {

    AssertStr (getWorker()->getDataManager().getOutputs() >= 
	       dhIndex+aStep[item]->getWorker()->getDataManager().getInputs(),
	       "connect " << getName() << " - " << aStep[item]->getName() <<
	       "; not enough inputs");
    aStep[item]->getRep()->connectRep (this, 0, dhIndex, -1, prototype,
				       blockingComm);
    dhIndex += aStep[item]->getWorker()->getDataManager().getInputs();

  }
  if (dhIndex != getWorker()->getDataManager().getOutputs()) {
    cerr << "StepRep::connectOutputArray() - Warning:" << endl
	 << "  " << getName() << " - " << aStep[0]->getName()
	 << ", unequal number of inputs and outputs" << endl;
  }
  return true;
}

void StepRep::replaceConnectionsWith(const TransportHolder& newTH,
				     bool blockingComm)
{
  cdebug(3) << "replaceConnectionsWith  " << newTH.getType() << endl;
  for (int ch=0; ch<itsDataManager->getInputs(); ch++)
  {
    DataHolder* dh = itsDataManager->getGeneralInHolder(ch);
    Transporter& transp = dh->getTransporter();
    DataHolder* thatDH = transp.getSourceDataHolder();
    if (thatDH)
    {
      cdebug(3) << "replace " << transp.getTransportHolder()->getType()
		<< " with " << newTH.getType() << endl;
      dh->connectTo(*thatDH, newTH, blockingComm);
    }
  }
}

void StepRep::dump() const
{
  // cout << "StepRep::dump " << itsName << endl;
  if (itsWorker->shouldProcess() ){
    itsWorker->dump();
  }
}

bool StepRep::setProcessRate (int rate)
{
  bool result = true;
  getWorker()->getDataManager().setProcessRate(rate);
  return result;
}

bool StepRep::setInRate(int rate, int dhIndex)
{
  getWorker()->getDataManager().setInputRate(rate, dhIndex);
  return true;
}

bool StepRep::setOutRate (int rate, int dhIndex)
{
  getWorker()->getDataManager().setOutputRate(rate, dhIndex);
  return true;
}

}
