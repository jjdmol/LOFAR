//#  ResourceAllocatorTest.cc: Main entry for the ResourceAllocator test
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <sys/time.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ResourceAllocator.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

int main(int /*argc*/, char** /*argv*/)
{
  int retval=0;
  
  INIT_LOGGER("./ResourceAllocatorTest.log_prop");   
  {
  
    ResourceAllocator::ResourceAllocatorPtr resourceAllocator(ResourceAllocator::instance());
    
    ResourceAllocator::LogicalDevicePtr ldIgnore;
    resourceAllocator->claimSO(ldIgnore,100,120.0);   // 1 allocated
    resourceAllocator->logSOallocation();
    resourceAllocator->releaseSO(ldIgnore);           // 0 allocated
  
    resourceAllocator->claimSO(ldIgnore,100,120.0);   // 1 allocated
    resourceAllocator->claimSO(ldIgnore,80,120.0);    // 2 allocated
    resourceAllocator->logSOallocation();
    resourceAllocator->claimSO(ldIgnore,100,100.0);   // 2 allocated
    resourceAllocator->logSOallocation();
    resourceAllocator->claimSO(ldIgnore,70,100.0);    // 3 allocated
    resourceAllocator->logSOallocation();
    
    resourceAllocator->releaseSO(ldIgnore);           // 2 allocated
    resourceAllocator->logSOallocation();
    resourceAllocator->releaseSO(ldIgnore);           // 1 allocated
    resourceAllocator->logSOallocation();
    resourceAllocator->releaseSO(ldIgnore);           // 0 allocated
    resourceAllocator->logSOallocation();
    
    ResourceAllocator::TRcuSubset subset03(0x03); // 0000 0011
    ResourceAllocator::TRcuSubset subsetF0(0xF0); // 1111 0000
    ResourceAllocator::TRcuSubset subset0C(0x0C); // 0000 1100
    ResourceAllocator::TRcuSubset subset1C(0x1C); // 0001 1100
    
    resourceAllocator->claimSRG(ldIgnore,100,subset03,1,1); // 1 allocated
    resourceAllocator->logSRGallocation();
    resourceAllocator->releaseSRG(ldIgnore);                // 0 allocated
    
    resourceAllocator->claimSRG(ldIgnore,100,subset03,1,1); // 1 allocated
    resourceAllocator->claimSRG(ldIgnore,100,subsetF0,2,2); // 2 allocated
    resourceAllocator->claimSRG(ldIgnore,100,subset0C,3,3); // 3 allocated
    resourceAllocator->logSRGallocation();

    resourceAllocator->claimSRG(ldIgnore,100,subset03,3,3); // 3 allocated
    resourceAllocator->claimSRG(ldIgnore,100,subset1C,4,4); // 3 allocated
    resourceAllocator->logSRGallocation();
    
    resourceAllocator->claimSRG(ldIgnore,120,subset03,3,3); // 3 allocated
    resourceAllocator->claimSRG(ldIgnore, 80,subset03,3,3); // 4 allocated
    resourceAllocator->logSRGallocation();
    resourceAllocator->releaseSRG(ldIgnore);                // 3 allocated
    resourceAllocator->logSRGallocation();
    
    resourceAllocator->releaseSRG(ldIgnore);                // 2 allocated
    resourceAllocator->logSRGallocation();
    resourceAllocator->releaseSRG(ldIgnore);                // 1 allocated
    resourceAllocator->logSRGallocation();
    resourceAllocator->releaseSRG(ldIgnore);                // 0 allocated
    resourceAllocator->logSRGallocation();
    
  }
  return retval;
}

