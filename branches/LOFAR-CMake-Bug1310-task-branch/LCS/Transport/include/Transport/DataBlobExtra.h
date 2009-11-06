//# DataBlobExtra.h: Extra fields for the blob in the DataHolder
//#
//# Copyright (C) 2000, 2001
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

#ifndef TRANSPORT_DATABLOBEXTRA_H
#define TRANSPORT_DATABLOBEXTRA_H

// \file
// Extra fields for the blob in the DataHolder

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobOBufChar.h>
#include <string>

namespace LOFAR
{

// \addtogroup Transport
// @{

//# Forward Declarations
class DataHolder;
class BlobOStream;
class BlobIStream;
class BlobString;

/**
  Class DataBlobExtra holds extra (variable length) fields for the blob in 
  the DataHolder.
*/

class DataBlobExtra
{
public:
  // Construct with the given name and version for the given DataHolder.
  DataBlobExtra (const string& name, int version, DataHolder*);

  ~DataBlobExtra();

  // Initialize the extra output blob holding arbitrary fields.
  // The return reference can be used to store the fields in.
  // It is meant for DataHolders writing data.
  BlobOStream& createBlock();

  // Clear the extra output blob.
  void clearBlock();

  // Open the extra input blob holding arbitrary fields.
  // It can be accessed using getExtraBlob.
  void openBlock (const BlobString& data);

  // Write the extra block into the main block.
  void pack();

  // Get the name of the extra data blob.
  const std::string& getName() const;

  // Get the version of the extra data blob.
  int getVersion();

  // Clear the output blob.
  void clearOut()
    { itsCreateDone = false; }

  // \name Get read access to the blob used last
  // The blob used last is the blob opened or created.
  // <group>
  // Assures that there is a blob.
  BlobIStream& getBlock();
  // Sets \a found=true when a blob is found.
  BlobIStream& getBlock (bool& found, int& version);
  // <group>

private:
  // Forbid copy constructor.
  DataBlobExtra (const DataBlobExtra&);

  // Forbid assignment.
  DataBlobExtra& operator= (const DataBlobExtra&);


  BlobOStream* itsOut;
  BlobIStream* itsIn;
  BlobOBufChar itsBufOut;     ///< write buffer for extra blob data
  BlobIBufChar itsBufIn;      ///< read buffer for extra blob data
  std::string  itsName;       ///< blob type name for extra data
  int          itsVersion;    ///< blob version for extra data
  char*        itsDataPtr;    ///< pointer to extra block in the main block
  bool         itsCreateDone; ///< true = createBlock has been done
  int          itsLastDone;   ///< 0=nothing, 1=create, 2=open done as last
  DataHolder*  itsDH;
};


inline const std::string& DataBlobExtra::getName() const
{
  return itsName;
}

inline int DataBlobExtra::getVersion()
{
  return itsVersion; 
}

// @} // Doxygen endgroup Transport

} // end namespace

#endif
