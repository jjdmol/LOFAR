//#  ARAPhysicalModel.cc: Implementation of the ARAPhysicalModel object
//#
//#  Copyright (C) 2002-2004
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

#include "ARAPhysicalModel.h"
#include "ARAPropertyDefines.h"
#include "ARAConstants.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/ParameterSet.h>

using namespace std;
using namespace boost;

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{

namespace ARA
{

ARAPhysicalModel::ARAPhysicalModel() : 
  m_maintenanceModel(),
  m_maintenanceFlags()
{
  int rack,subrack,board,ap,rcu;

  ParameterSet::instance()->adoptFile("RegisterAccess.conf");

  int n_racks               = ParameterSet::instance()->getInt(PARAM_N_RACKS);
  int n_subracks_per_rack   = ParameterSet::instance()->getInt(PARAM_N_SUBRACKS_PER_RACK);
  int n_boards_per_subrack  = ParameterSet::instance()->getInt(PARAM_N_BOARDS_PER_SUBRACK);
  int n_aps_per_board       = ParameterSet::instance()->getInt(PARAM_N_APS_PER_BOARD);
  int n_rcus_per_ap         = ParameterSet::instance()->getInt(PARAM_N_RCUS_PER_AP);

  char tempString[200];
  vector<string> childrenPIC,childrenRack,childrenSubRack,childrenBoard;
  for(rack=1;rack<=n_racks;rack++)
  {
    for(subrack=1;subrack<=n_subracks_per_rack;subrack++)
    {
      for(board=1;board<=n_boards_per_subrack;board++)
      {
        for(ap=1;ap<=n_aps_per_board;ap++)
        {
          for(rcu=1;rcu<=n_rcus_per_ap;rcu++)
          {
            sprintf(tempString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance,rack,subrack,board,ap,rcu);
            childrenBoard.push_back(string(tempString));
          }
        }
        sprintf(tempString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
        m_maintenanceModel[tempString] = childrenBoard;
        childrenBoard.clear();

        childrenSubRack.push_back(string(tempString));
      }
      sprintf(tempString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
      m_maintenanceModel[tempString] = childrenSubRack;
      childrenSubRack.clear();

      childrenRack.push_back(string(tempString));
    }
    sprintf(tempString,SCOPE_PIC_RackN_Maintenance,rack);
    m_maintenanceModel[tempString] = childrenRack;
    childrenRack.clear();

    childrenPIC.push_back(string(tempString));
  }
  m_maintenanceModel[tempString] = childrenPIC;
  childrenPIC.clear();
}

ARAPhysicalModel::~ARAPhysicalModel()
{
}

void ARAPhysicalModel::inMaintenance(bool maintenance, string& resource)
{
  if(maintenance)
  {
    m_maintenanceFlags.insert(resource);
    // set maintenance status in database of every child resource
    
    PhysicalModelIterT pIt = m_maintenanceModel.find(resource);
    if(pIt != m_maintenanceModel.end())
    {
      vector<string>::iterator mIt;
      for(mIt=pIt->second.begin();mIt!=pIt->second.end();++mIt)
      {
        string propName(*mIt + string(".") + string(PROPNAME_STATUS));
        TPropertyInfo propInfo(propName.c_str(),LPT_UNSIGNED);
        GCFExtProperty extPropMaintenance(propInfo);
        GCFPVUnsigned inMaintenance(1);
        extPropMaintenance.setValue(inMaintenance);
      }
    }
  }
  else
  {
    m_maintenanceFlags.erase(resource);
    // reset maintenance status in database of every child resource, 
    // except the ones that exist in the maintenance flags list.

    PhysicalModelIterT pIt = m_maintenanceModel.find(resource);
    if(pIt != m_maintenanceModel.end())
    {
      vector<string>::iterator mIt;
      for(mIt=pIt->second.begin();mIt!=pIt->second.end();++mIt)
      {
        MaintenanceFlagsIterT fIt = m_maintenanceFlags.find(*mIt);
        if(fIt == m_maintenanceFlags.end())
        {
          // maintenance of resource is not individually set
          string propName(*mIt + string(".") + string(PROPNAME_STATUS));
          TPropertyInfo propInfo(propName.c_str(),LPT_UNSIGNED);
          GCFExtProperty extPropMaintenance(propInfo);
          GCFPVUnsigned inMaintenance(0);
          extPropMaintenance.setValue(inMaintenance);
        }
      }
    }
  }
}

} // namespace ARA


} // namespace LOFAR

