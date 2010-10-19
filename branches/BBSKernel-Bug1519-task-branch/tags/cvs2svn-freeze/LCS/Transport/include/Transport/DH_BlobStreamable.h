//# DH_BlobStreamable.h: DataHolder for BlobStreamable objects.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_TRANSPORT_DH_BLOBSTREAMABLE_H
#define LOFAR_TRANSPORT_DH_BLOBSTREAMABLE_H

// \file
// DataHolder for  BlobStreamable objects.

//# Includes
#include <Transport/DataHolder.h>

namespace LOFAR
{
  //# Forward Declarations.
  class BlobStreamable;

  // \addtogroup Transport
  // @{

  // DataHolder for BlobStreamable objects. This DataHolder is a generic
  // container for classes that implement the BlobStreamble interface. The
  // data, in general variable length, will be stored in a so-called
  // ExtraBlob.
  // \note BlobStreamable objects must be default constructable in order to
  // be used with this DataHolder class.
  class DH_BlobStreamable : public DataHolder
  {
  public:
    // Construct a DataHolder for a BlobStreamable object. The argument \a
    // name can be used to uniquely identify the object. The optional
    // argument \a version can be used to indicate a specific version of the
    // BlobStreamable object.
    explicit DH_BlobStreamable(const string& name = "aDH_BlobStreamable", 
			       int version = 1);

    // Deserialize the object contained in this DataHolder. The object is
    // deserialized by first reading the class type. Next a new
    // BlobStreamable object of the right class type is instantiated and
    // initialized using the contents of the DataHolder's ExtraBlob.
    // \return A pointer to the new BlobStreamable object.
    BlobStreamable* deserialize();

    // Serialize the object and store it in this DataHolder. First the class
    // type of \a bs is stored in \c itsClassType. Next the data in \a bs
    // are stored in the DataHolder's ExtraBlob.
    void serialize(const BlobStreamable& bs);

    // Get the class typename. The class typename is used by a factory
    // method to construct a new instance of the specified class type.
    string classType() const { return string(itsClassType); }

  private:
    // Allocate buffers.
    // \note Moved it to the private section; it will be called (implicitly)
    // by the constructor, and it should not be called more than once.
    virtual void init();

    // Fill the pointers to the data in the blob. 
    // \note This method is called by DataHolder::createDataBlock().
    virtual void fillDataPointers();

    virtual DH_BlobStreamable* clone() const;

    // The class type of the object that is being sent/received using this
    // DataHolder. 
    char* itsClassType;
  };

} // namespace LOFAR

#endif
