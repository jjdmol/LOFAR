//#  ObjectId.h: ObjectId is used to uniquely identify a persistent object.
//#
//#  Copyright (C) 2002
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

#ifndef LCS_PL_OBJECTID_H
#define LCS_PL_OBJECTID_H

namespace LCS
{
  namespace PL
  {

    //
    // ObjectId is used to uniquely identify a persistent object.
    //
    class ObjectId
    {
    public:
      // We will be using a 64-bit integer for our unique key
      typedef unsigned long long oid_t;

      // NullId represents a (default) null object-id value.
      static const oid_t NullId;

      // Default constructor.
      ObjectId() { generate(); }

      // Set the stored object-id equal to aOid.
      void set(const oid_t& aOid) { itsOid = aOid; }

      // Return the stored object-id.
      const oid_t& get() const { return itsOid; }

    private:
      // Flag indicating whether the random generator has been initialized.
      static bool theirInitFlag;

      // Here we keep the unique object-id.
      oid_t itsOid;

      // Generate a (hopefully) unique object-id.
      void generate();
    };


    inline bool operator==(const ObjectId& lhs, const ObjectId& rhs)
    {
      return lhs.get() == rhs.get();
    }


  } // namespace PL

} // namespace LCS

#endif
