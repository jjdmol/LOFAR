//  DHPoolManager.cc:
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
// DHPoolManager.cc: implementation of the DHPoolManager class.
//
//////////////////////////////////////////////////////////////////////

#include "CEPFrame/DHPoolManager.h"
#include "Common/Debug.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DHPoolManager::DHPoolManager ()
  : itsDataHolder(0),
    itsShared(false)
{
  TRACER2("DHPoolManager constructor");
}

DHPoolManager::~DHPoolManager()
{
  delete itsDataHolder;
}

void DHPoolManager::preprocess()
{
  AssertStr(itsDataHolder!=0, "DataHolder has not been set");
  itsDataHolder->basePreprocess();
}

int DHPoolManager::getSize()
{
  return 1;
}

DataHolder* DHPoolManager::getWriteLockedDH(int* id)
{
  *id = 0;
  return itsDataHolder;
}

DataHolder* DHPoolManager::getReadLockedDH(int* id)
{
  *id = 0;
  return itsDataHolder;
} 

DataHolder* DHPoolManager::getRWLockedDH(int* id)
{
  *id = 0;
  return itsDataHolder;
}

