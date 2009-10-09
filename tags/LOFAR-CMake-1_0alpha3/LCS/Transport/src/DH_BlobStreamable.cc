//# DH_BlobStreamable.cc: DataHolder for BlobStreamable objects.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <Transport/DH_BlobStreamable.h>
#include <Blob/BlobStreamable.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  // Maximum length of the class type string. 
  const uint maxClassTypeLength = 64;


  DH_BlobStreamable::DH_BlobStreamable(const string& name, int version) :
    DataHolder(name, "DH_BlobStreamable", version),
    itsClassType(0)
  {
    LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_FLOW, name << "(" << version << ")");
    setExtraBlob("BlobStreamable", 1);
    init();   //<--- Added this call; we don't want to bother the end-user.
  }


  void DH_BlobStreamable::init()
  {
    LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    addField("ClassType", BlobField<char>(1, maxClassTypeLength));
    createDataBlock();
  }


  BlobStreamable* DH_BlobStreamable::deserialize()
  {
    LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    BlobStreamable* bs = 
      BlobStreamableFactory::instance().create(itsClassType);
    ASSERTSTR(bs, "Failed to create object of type " << itsClassType);
    bs->read(getExtraBlob());
    return bs;
  }


  void DH_BlobStreamable::serialize(const BlobStreamable& bs)
  {
    LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    string type(bs.classType());
    ASSERTSTR(type.size() < maxClassTypeLength, "Class typename too long");
    strcpy(itsClassType, type.c_str());
    bs.write(createExtraBlob());
  }


  void DH_BlobStreamable::fillDataPointers()
  {
    LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    itsClassType = getData<char>("ClassType");
  }


  DH_BlobStreamable* DH_BlobStreamable::clone() const
  {
    THROW(Exception, "We should NEVER call this method!");
  }


} // namespace LOFAR
