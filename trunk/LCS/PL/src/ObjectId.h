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

      // Default constructor. If \c doGenerate is true, \c itsOid will be
      // lazily initialized when get() is called, else \c itsOid will be 
      // set equal to \c NullId and marked as set.
      ObjectId(bool doGenerate = true);

      // Set the stored object-id equal to \c aOid.
      // \post \c itsOidIsSet is true.
      void set(const oid_t& aOid);

      // Return the stored object-id.
      // \post \c itsOid will have been set if it wasn't already.
      // \post \c itsOidIsSet is true.
      const oid_t& get() const;

    private:
      // Flag indicating whether the random generator has been initialized.
      static bool theirRandomGeneratorIsInitialized;

      // Here we keep the unique object-id.
      oid_t itsOid;

      // Flag that indicates whether itsOid has been set.
      // \note itsOidIsSet must be mutable because we use lazy 
      // initialization in the get() method.
      mutable bool itsOidIsSet;

      // Generate a (hopefully) unique object-id.
      // \note This method must be \c const, because it is called by get().
      // However, it \e does change \c itsOid.
      void generate() const;
    };

    // Compare two ObjectIds.
    inline bool operator==(const ObjectId& lhs, const ObjectId& rhs)
    {
      return lhs.get() == rhs.get();
    }


  } // namespace PL

} // namespace LCS

#endif
