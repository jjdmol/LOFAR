//  CycBufferManager.cc:
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
// CycBufferManager.cc: implementation of the CycBufferManager class.
//
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <CEPFrame/CycBufferManager.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CycBufferManager:: CycBufferManager(int bufferSize)
  : DHPoolManager(),
    size(bufferSize)
{
  ASSERTSTR(bufferSize > 0, "Cannot create a cylicbuffer smaller than 1.");
}

CycBufferManager:: ~CycBufferManager()
{
}

DataHolder* CycBufferManager::getWriteLockedDH(int* id)
{
  if (itsBuf.GetCount() == itsBuf.GetSize())    //check if buffer is full
  {
    LOG_WARN("Cyclic buffer is full");
    //increase buffer til max size? 
  }
  return itsBuf.GetWriteLockedDataItem(id);
}

DataHolder* CycBufferManager::getReadLockedDH(int* id)
{
  return itsBuf.GetReadDataItem(id);
}

DataHolder* CycBufferManager::getRWLockedDH(int* id)
{
  return itsBuf.GetRWLockedDataItem(id);
}
  
void CycBufferManager::writeUnlock(int id)
{
  itsBuf.WriteUnlockElement(id);
}

void CycBufferManager::readUnlock(int id)
{
  itsBuf.ReadUnlockElement(id);
}

void CycBufferManager::preprocess()
{
  ASSERTSTR(itsDataHolder!=0, "DataHolder has not been set");
  for (int i = 0; i < size; i++)   // Fill buffer
  {
    DataHolder* dhPtr = itsDataHolder->clone();
    dhPtr->init();
    itsBuf.AddBufferElement(dhPtr);
  }
}
  
int CycBufferManager::getSize()
{
  return itsBuf.GetSize();
}

int CycBufferManager::getAndResetMaxUsage()
{
  return itsBuf.GetAndResetMaxUsage();
}

}
