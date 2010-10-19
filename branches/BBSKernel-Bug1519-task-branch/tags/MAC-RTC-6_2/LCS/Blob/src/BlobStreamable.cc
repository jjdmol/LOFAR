//#  BlobStreamable.cc: 
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

#include <lofar_config.h>

#include <Blob/BlobStreamable.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

  BlobStreamable* BlobStreamable::deserialize(BlobIStream& bis)
  {
    LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

    // Read the "start-of-object" marker.
    bis.getStart("BlobStreamable");

    // Read the class type of the BlobStreamable object to be created.
    string type;
    bis >> type;
    LOG_TRACE_VAR_STR("Class type to be created: " << type);

    // Create a new BlobStreamable object based on \a type.
    BlobStreamable* obj = BlobStreamableFactory::instance().create(type);
    ASSERTSTR(obj, "Failed to create object of type " << type);

    // Read the rest of the data into the new BlobStreamable object.
    obj->read(bis);

    // Read the "end-of-object" marker
    bis.getEnd();

    // Return the new BlobStreamable object.
    return obj;
  }


  void BlobStreamable::serialize(BlobOStream& bos) const
  {
    LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

    // Write the "start-of-object" marker.
    bos.putStart("BlobStreamable", 1);

    // Write the class type of \c *this.
    bos << classType();

    // Write the contents of \c *this into the output blob stream \a bos.
    write(bos);

    // Write the "end-of-object" marker.
    bos.putEnd();
  }

} // namespace LOFAR
