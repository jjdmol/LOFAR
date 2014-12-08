//#  KVLUtils.cc: 
//#
//#  Copyright (C) 2002-2003
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

#include "KVLUtils.h"
#include <Common/ParameterSet.h>
#include <KVLDefines.h>
#include <sys/time.h>
#include <time.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
  namespace LogSys 
  {

Value::~Value() 
{
  if (_unpacked) delete _pValue;
}

unsigned int Value::pack(char* buffer)
{
  ASSERT(_pValue);
  return _pValue->pack(buffer);
}

unsigned int Value::unpack(char* buffer)
{
  ASSERT(!_pValue);
  _unpacked = true;
  _pValue = GCFPValue::unpackValue(buffer);
  return _pValue->getSize();
}

unsigned int Value::getSize()
{
  ASSERT(_pValue);
  return _pValue->getSize();
}

unsigned int EventCollection::unpack(char* buffer)
{
  GCFPValue* pValue(GCFPValue::unpackValue(buffer));
  ASSERT(pValue);
  buf.copy(*pValue);
  delete pValue;
  return buf.getSize();
}
  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
