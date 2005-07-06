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

#include <boost/shared_ptr.hpp>

#include <GCF/ParameterSet.h>
#include <APLCommon/StartDaemon.h>
#include <VirtualTelescope/VirtualTelescopeFactory.h>
#include <StationReceptorGroup/StationReceptorGroupFactory.h>
#include <StationOperations/StationOperationsFactory.h>

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

string _getShareLocation()
{
  string shareLocation("/opt/lofar/MAC/parametersets/");
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    string tempShareLocation = pParamSet->getString(SD_CONFIG_PREFIX + SD_CONFIG_SHARELOCATION);
    if(tempShareLocation.length()>0)
    {
      if(tempShareLocation[tempShareLocation.length()-1] != '/')
      {
        tempShareLocation+=string("/");
      }
      shareLocation=tempShareLocation;
    }
  } 
  catch(Exception& e)
  {
    LOG_WARN(formatString("(%s) Sharelocation parameter not found. Using %s",e.message().c_str(),shareLocation.c_str()));
  }
  return shareLocation;
}

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
  GCFTask::init(argc, argv);
  
  boost::shared_ptr<VirtualTelescopeFactory>      vtFactory(new VirtualTelescopeFactory);
  boost::shared_ptr<StationReceptorGroupFactory>  srgFactory(new StationReceptorGroupFactory);
  boost::shared_ptr<StationOperationsFactory>     soFactory(new StationOperationsFactory);
  
  StartDaemon sd(string("PAC_StartDaemon"));
  
  sd.registerFactory(LDTYPE_VIRTUALTELESCOPE,vtFactory);
  sd.registerFactory(LDTYPE_STATIONRECEPTORGROUP,srgFactory);
  sd.registerFactory(LDTYPE_STATIONOPERATIONS,soFactory);
  
  sd.start(); // make initial transition
  
  string shareLocation  = _getShareLocation();
  string taskName       = _getStationOperationsTaskName();
  string parameterFile  = _getStationOperationsParameterFile();
  
  sd.createLogicalDevice(LDTYPE_STATIONOPERATIONS,taskName,shareLocation+parameterFile);
  
  GCFTask::run();
    
  return 0;
}

