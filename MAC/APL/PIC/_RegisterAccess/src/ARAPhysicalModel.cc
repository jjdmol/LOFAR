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

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_Property.h>
#include <GCF/GCF_PVUnsigned.h>

using namespace LOFAR;
using namespace ARA;
using namespace std;
using namespace boost;

ARAPhysicalModel::ARAPhysicalModel() : 
  m_maintenanceModel(),
  m_maintenanceFlags()
{
  int rack,subrack,board,ap,rcu;
  char tempString[200];
  vector<string> childrenPIC,childrenRack,childrenSubRack,childrenBoard;
  for(rack=1;rack<=N_RACKS;rack++)
  {
    for(subrack=1;subrack<=N_SUBRACKS_PER_RACK;subrack++)
    {
      for(board=1;board<=N_BOARDS_PER_SUBRACK;board++)
      {
        for(ap=1;ap<=N_APS_PER_BOARD;ap++)
        {
          for(rcu=1;rcu<=N_APS_PER_BOARD;rcu++)
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
        GCFProperty maintenanceProp(*mIt);
        GCFPVUnsigned inMaintenance(1);
        maintenanceProp.setValue(inMaintenance);
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
          GCFProperty maintenanceProp(*mIt);
          GCFPVUnsigned inMaintenance(0);
          maintenanceProp.setValue(inMaintenance);
        }
      }
    }
  }
}
