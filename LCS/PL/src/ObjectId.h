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
    class ObjectId
    {
    public:
      // We will be using a 64-bit integer for our unique key
      typedef unsigned long long ObjectIdType;

      // Create a new unique identifier.
      ObjectId() { 
	THROW(NotImplemented,"No Object ID generator available yet"); 
      }

      // Return the stored unique key.
      ObjectIdType& value() const;

      // Return the object-id as a string
      std::string asString() const;
  
    private:
      // Here we keep the unique object-id.
      ObjectIdType itsOid;

      // Flag that indicates whether the ObjectId key generator was
      // already initialized.
      bool isInitialized;

      // Initialize the ObjectId key generator
      void init();

    };



  } // namespace PL

} // namespace LCS

#endif
