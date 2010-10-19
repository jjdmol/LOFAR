//#  StationStartDaemonMain.cc: Main entry for the Station start daemon
//#
//#  Copyright (C) 2002-2005
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>
#include <GCF/ParameterSet.h>
#include <APLCommon/StartDaemon.h>
#include <VirtualTelescope/VirtualTelescope.h>
#include <StationReceptorGroup/StationReceptorGroup.h>
#include <StationOperations/StationOperations.h>
#include <APLCommon/LogicalDeviceFactory.h>
#include <APLCommon/SharedLogicalDeviceFactory.h>
#include <APLCommon/SingleInstanceLogicalDeviceFactory.h>

using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::APLCommon;
using namespace LOFAR::AVT;
using namespace LOFAR::ASR;
using namespace LOFAR::ASO; // :-)

const string SD_CONFIG_PREFIX            = string("mac.apl.sd.");
const string SD_CONFIG_SHARELOCATION     = string("shareLocation");
const string SD_CONFIG_TASKNAME          = string("taskName");
const string SD_CONFIG_PARAMETERFILE     = string("parameterFile");

string _getStationOperationsParameterFile()
{
  string parameterFile("");
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    parameterFile = pParamSet->getString(SD_CONFIG_PREFIX + string("SO.") + SD_CONFIG_PARAMETERFILE);
  } 
  catch(Exception& e)
  {
    LOG_WARN(formatString("(%s) SO paramFile parameter not found.",e.message().c_str()));
  }
  return parameterFile;
}

string _getStationOperationsTaskName()
{
  string taskName("");
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    taskName = pParamSet->getString(SD_CONFIG_PREFIX + string("SO.") + SD_CONFIG_TASKNAME);
  } 
  catch(Exception& e)
  {
    LOG_WARN(formatString("(%s) SO taskName parameter not found.",e.message().c_str()));
  }
  return taskName;
}

int main(int argc, char* argv[])
{
  int retval=0;
  try
  {
    GCFTask::init(argc, argv);
  
    boost::shared_ptr<LogicalDeviceFactory<VirtualTelescope> > vtFactory(
      new LogicalDeviceFactory<VirtualTelescope>);
    boost::shared_ptr<SharedLogicalDeviceFactory<StationReceptorGroup> > srgFactory(
      new SharedLogicalDeviceFactory<StationReceptorGroup>);
    //    boost::shared_ptr<SingleInstanceLogicalDeviceFactory<StationOperations> > soFactory(
    //      new SingleInstanceLogicalDeviceFactory<StationOperations>);
    boost::shared_ptr<SharedLogicalDeviceFactory<StationOperations> > soFactory(
      new SharedLogicalDeviceFactory<StationOperations>);
    
    StartDaemon sd(string("PAC_StartDaemon"));
    
    sd.registerFactory(LDTYPE_VIRTUALTELESCOPE,vtFactory);
    sd.registerFactory(LDTYPE_STATIONRECEPTORGROUP,srgFactory);
    sd.registerFactory(LDTYPE_STATIONOPERATIONS,soFactory);
    
    sd.start(); // make initial transition
    
    string taskName       = _getStationOperationsTaskName();
    string parameterFile  = _getStationOperationsParameterFile();
  
    TSDResult result = sd.createLogicalDevice(LDTYPE_STATIONOPERATIONS,taskName,parameterFile);
    if(result == SD_RESULT_NO_ERROR)
    {
      GCFTask::run();
    }
    else
    {
      retval=-1;
    }
  } 
  catch(APLCommon::WrongVersionException& e)
  {
    LOG_FATAL(formatString("(%s) Unable to start StationOperations Logical Device due to version conflict.",e.message().c_str()));
    retval = -1;
  }
  catch(APLCommon::ParameterNotFoundException& e)
  {
    LOG_FATAL(formatString("(%s) Unable to start StationOperations Logical Device.",e.message().c_str()));
    retval = -1;
  }
  catch(Exception& e)
  {
    LOG_FATAL(formatString("%s",e.message().c_str()));
    retval = -1;
  }
  return retval;
}

