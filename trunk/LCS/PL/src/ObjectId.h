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

//# Includes
#include <PL/Exception.h>

#include <string>

namespace LCS
{
  namespace PL
  {

    //
    // ObjectId is used to uniquely identify a persistent object.
    //
    // \todo Currently, random bytes are read from /dev/urandom. It
    // turned out that reading from this device is relatively slow.
    // Generating a random 8-byte number using get_random_bytes()
    // takes approx. 10 microseconds on lofar9 (which has a 1.6 GHz CPU).
    // If we use random() only we can reduce this time significantly,
    // to something like 0.7 microseconds or better. This will probably
    // be good enough for our purposes.
    //
    class ObjectId
    {
    public:
      // We will be using a 64-bit integer for our unique key
      typedef unsigned long long oid_t;

      // Default constructor. The object-id is lazily initialized,
      // i.e. it is initialized once get() or set() is being called.
      ObjectId();

      // Set the stored object-id equal to aOid.
      // \post itsIsInitialized is true.
      void set(const oid_t& aOid);

      // Return the stored object-id.
      // \note We cannot make this method const, because we use lazy
      // initialization for itsOid.
      // \post itsOid will have been initialized if it wasn't already.
      // \post itsIsInitialized is true.
      const oid_t& get();

    private:
      // Here we keep the unique object-id.
      oid_t itsOid;

      // Flag that indicates whether itsOid has been initialized.
      bool itsIsInitialized;

      // Initialize itsOid with a (hopefully) unique object-id.
      void init();

    };

  } // namespace PL

} // namespace LCS

#endif
