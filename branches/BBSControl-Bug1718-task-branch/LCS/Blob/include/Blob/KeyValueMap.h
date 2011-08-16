//# KeyValueMap.h: Class to hold a collection of key name/value pairs.
//#
//# Copyright (C) 2001
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

#ifndef LOFAR_BLOB_KEYVALUEMAP_H
#define LOFAR_BLOB_KEYVALUEMAP_H

// \file
// Class to hold a collection of key name/value pairs.

#include <Blob/KeyValue.h>
#include <Common/lofar_map.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR {

  // \ingroup Blob
  // \addtogroup KeyValue
  // <group>

  // <summary> Class to hold a collection of key name/value pairs </summary>
  
  // The KeyValueMap class holds a collection of key name/value pairs.
  // It uses the STL map class to hold the pairs.

  class KeyValueMap: public map<string, KeyValue>
    {
    public:
      typedef map<string,KeyValue>::const_iterator const_iterator;
      typedef map<string,KeyValue>::iterator iterator;

      KeyValueMap();
      
      // Copy constructor (copy semantics)
      KeyValueMap (const KeyValueMap& that);
      
      ~KeyValueMap();
      
      // Assignment (copy semantics)
      KeyValueMap& operator= (const KeyValueMap& that);
      
      // Is a key defined?
      bool isDefined (const string& name) const
	{ return find(name) != end(); }
      
  // \name Get the value of a key
  // Use the default if not existing.
      // <group>
      bool getBool (const string& name, bool defVal) const;
      int getInt (const string& name, int defVal) const;
      float getFloat (const string& name, float defVal) const;
      double getDouble (const string& name, double defVal) const;
      fcomplex getFComplex (const string& name, const fcomplex& defVal) const;
      dcomplex getDComplex (const string& name, const dcomplex& defVal) const;
      const string& getString (const string& name, const string& defVal) const;
      // </group>
      
  // \name Show the contents of the object
      // <group>
      void show (ostream&) const;
      friend ostream& operator<< (ostream&, const KeyValueMap&);
      // </group>
      
      // Turn the object into a blob.
      friend BlobOStream& operator<< (BlobOStream&, const KeyValueMap&);
      // Fill the object from a blob. It destroys the current contents.
      friend BlobIStream& operator>> (BlobIStream&, KeyValueMap&);
    };

  // </group>

} //end namespace

#endif 
