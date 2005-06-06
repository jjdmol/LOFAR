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

#include <lofar_config.h>

#include <CEPFrame/StepRep.h>
#include <CEPFrame/CompositeRep.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/DataManager.h>
#include <Transport/BaseSim.h>
#include TRANSPORTERINCLUDE
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>    // for min,max

#include <sstream>

using namespace std;

namespace LOFAR
{

StepRep::StepRep (WorkHolder& worker, 
		  const string& name,
		  bool addNameSuffix)
  : BlockRep (name, addNameSuffix),
    itsWorker(0)
{
  itsWorker = worker.baseMake();

  // Replace the tinyDataManager with a DataManager
  itsDataManager = new DataManager(itsWorker->getDataManager());
  itsWorker->setDataManager(itsDataManager);

}

StepRep::~StepRep() 
{
  delete itsWorker; // WorkHolder will delete DataManager.
}


bool StepRep::isComposite() const
{
  return false;
}

void StepRep::preprocess()
{
  itsWorker->basePreprocess();
}

void StepRep::process()
{
  // Call the baseProcess() method in the WorkHolder

  itsWorker->baseProcess();
}

void StepRep::postprocess()
{
  itsWorker->basePostprocess();
}
     
// void StepRep::replaceConnectionsWith(const TransportHolder& newTH,
// 				     bool blockingComm)
// {
//   LOG_TRACE_RTTI_STR("replaceConnectionsWith  " << newTH.getType());
//   for (int ch=0; ch<itsDataManager->getInputs(); ch++)
//   {
//     DataHolder* dh = itsDataManager->getGeneralInHolder(ch);
//     Transporter& transp = dh->getTransporter();
//     DataHolder* thatDH = transp.getSourceDataHolder();
//     if (thatDH)
//     {
//       LOG_TRACE_RTTI_STR("replace " << transp.getTransportHolder()->getType()
// 		<< " with " << newTH.getType());
//       dh->connectTo(*thatDH, newTH, blockingComm);
//     }
//   }
// }

void StepRep::dump() const
{
  // cout << "StepRep::dump " << itsName << endl;
  if (itsWorker->shouldProcess() ){
    itsWorker->dump();
  }
}

void StepRep::setProcessRate (int rate)
{
  getWorker()->getDataManager().setProcessRate(rate);
}

void StepRep::setInRate(int rate, int dhIndex)
{
  getWorker()->getDataManager().setInputRate(rate, dhIndex);
}

void StepRep::setOutRate (int rate, int dhIndex)
{
  getWorker()->getDataManager().setOutputRate(rate, dhIndex);
}

}
