//#  ObjectId.cc: Implementation of the Object-Id key generator
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

#include <PL/ObjectId.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

/* 
 * It would probably be better to add a test in configure, but most systems
 * support srandom() and random() nowadays. (GML: 18-09-2003)
 */
#define HAVE_SRANDOM 1

#ifdef HAVE_SRANDOM
#define srand(x) 	srandom(x)
#define rand() 		random()
#endif

namespace LOFAR
{
  namespace PL
  {

    ////////////////////////////////////////////////////////////////
    //                                                            //
    //                Definition of static members                //
    //                                                            //
    ////////////////////////////////////////////////////////////////

    const ObjectId::oid_t ObjectId::NullId = ObjectId::oid_t();
    bool ObjectId::theirRandomGeneratorIsInitialized = false;


    ///////////////////////////////////////////////////////////////////
    //                                                               //
    //                Implementation of class methods                //
    //                                                               //
    ///////////////////////////////////////////////////////////////////

    const ObjectId::oid_t& ObjectId::get() const
    {
      if (!itsOidIsSet) {
        generate();
        itsOidIsSet = true;
      }
      return itsOid;
    }
    
    void ObjectId::reset()
    {
      itsOid = NullId;
      itsOidIsSet = false;
    }

    void ObjectId::set(const ObjectId::oid_t& aOid) 
    {
      itsOid = aOid;
      itsOidIsSet = true;
    }

    void ObjectId::generate() const
    {
      if (!theirRandomGeneratorIsInitialized) {
        // Initialize the random generator using a random seed.
        timeval tv;
        gettimeofday(&tv, 0);
        srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
        theirRandomGeneratorIsInitialized = true;
      }

      // Generate our random number one byte at a time.
      // We have to do it this way, because on older rand() implementations
      // \li the returned number is in the range from 0 to (2^15)-1;
      // \li the lower-order bits are much less random than the higher-order
      //     bits, so we can only use the high-byte.
      unsigned char *cp = (unsigned char *)(&itsOid);
      for (unsigned int i = 0; i < sizeof(itsOid); i++) {
        *cp++ ^= (rand() >> 7) & 0xFF;
      }

    }

  } // namespace PL

} // namespace LOFAR
