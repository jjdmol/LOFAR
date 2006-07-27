//#  WH_Control.cc:
//#
//#  Copyright (C) 2000, 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <BBSControl/WH_Control.h>
#include <CEPFrame/Step.h>
#include <BBSControl/DH_Solution.h>
#include <BBSControl/DH_WOPrediff.h>
#include <BBSControl/DH_WOSolve.h>
#include <BBSControl/SC_Simple.h>
#include <BBSControl/SC_WritePredData.h>
#include <BBSControl/SC_CompoundIter.h>
#include <BBSControl/StrategyController.h>
#include <BBS/BBSTestLogger.h>
#include <Common/lofar_sstream.h>

namespace LOFAR
{
  namespace BBS
  {

    WH_Control::WH_Control (const string& name, int nrPrediffers,
			    const ParameterSet& args)
      : WorkHolder     (3, 3, name,"WH_Control"),
	itsNrPrediffers(nrPrediffers),
	itsArgs        (args)
    {
      getDataManager().addInDataHolder(0, new DH_Solution("in_0")); 
      getDataManager().addInDataHolder(1, new DH_WOPrediff("in_1"));  // dummy
      getDataManager().addInDataHolder(2, new DH_WOSolve("in_2"));    // dummy
      getDataManager().addOutDataHolder(0, new DH_Solution("out_0")); // dummy
      getDataManager().addOutDataHolder(1, new DH_WOPrediff("out_1"));
      getDataManager().addOutDataHolder(2, new DH_WOSolve("out_2")); 
      // switch output trigger of
      getDataManager().setAutoTriggerIn(0, false);
      getDataManager().setAutoTriggerOut(0, false);
      getDataManager().setAutoTriggerIn(1, false);
      getDataManager().setAutoTriggerOut(1, false);
      getDataManager().setAutoTriggerIn(2, false);
      getDataManager().setAutoTriggerOut(2, false);
    }

    WH_Control::~WH_Control()
    {
      list<StrategyController*>::iterator iter;
      for (iter=itsControllers.begin(); iter!=itsControllers.end(); iter++)
      {
	delete (*iter);
      }
      itsControllers.clear();
    }

    WorkHolder* WH_Control::construct (const string& name, int nrPrediffers,
				       const ParameterSet& args)
    {
      return new WH_Control (name, nrPrediffers,args);
    }

    void WH_Control::preprocess()
    {
      BBSTest::ScopedTimer st("C:total_WH_Control::process");
      createStrategyControllers();
      itsCtrlIter = itsControllers.begin();
    }

    WH_Control* WH_Control::make (const string& name)
    {
      return new WH_Control(name, itsNrPrediffers, itsArgs);
    }

    void WH_Control::process()
    {
      if ((*itsCtrlIter)->execute() == false)  // Remove from list
      {
	delete (*itsCtrlIter);
	// Return an iterator that designates the first element remaining
	// beyond any elements removed, or end()
	itsCtrlIter = itsControllers.erase(itsCtrlIter);
	itsCtrlIter--;
      }

      // Set the iterator to the next Controller. Restart at the beginning
      // when at the end.
      if (++itsCtrlIter == itsControllers.end())
      {
	itsCtrlIter = itsControllers.begin();
      }

    }

    void WH_Control::dump() const
    {
    }

    void WH_Control::postprocess()
    {
      ControllerList::iterator iter;
      for (iter=itsControllers.begin(); iter!=itsControllers.end(); iter++)
      {
	(*iter)->postprocess();
      }
    }

    void WH_Control::createStrategyControllers()
    {
      Connection* inSolConn = getDataManager().getInConnection(0);
      Connection* outPDConn = getDataManager().getOutConnection(1);
      Connection* outSVConn = getDataManager().getOutConnection(2);
      int nrStrat = itsArgs.getInt32("nrStrategies");

      for (int i=1; i<=nrStrat; i++)
      {
	// Get strategy type
	ostringstream oss;
	oss << "SC" << i << "params.";
	ParameterSet params = itsArgs.makeSubset(oss.str());
	string stratType = params.getString("strategy");
	// Create StrategyController and add to list
	// Each StrategyController must get an unique ID
	if (stratType == "Simple")
	{
	  SC_Simple* sc = 
	    new SC_Simple(inSolConn, outPDConn, outSVConn, 
			  itsNrPrediffers, params);
	  itsControllers.push_back(sc);
	}
	else if (stratType == "WritePredData")
	{
	  SC_WritePredData* sc = 
	    new SC_WritePredData(inSolConn, outPDConn, outSVConn, 
				 itsNrPrediffers, params);
	  itsControllers.push_back(sc);
	}
	else if (stratType == "CompoundIter")
	{
	  SC_CompoundIter* sc = 
	    new SC_CompoundIter(inSolConn, outPDConn, outSVConn, 
				itsNrPrediffers, params);
	  itsControllers.push_back(sc);
	}
	else
	{
	  THROW(LOFAR::Exception, "Strategy type " << stratType 
		<< " unknown or not defined");
	}
      }
    }

  } // namespace BBS

} // namespace LOFAR
