///# DataBlobExtra.h: Extra fields for the blob in the DataHolder
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

#include <lofar_config.h>

//# Includes
#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufChar.h>
#include <string>

namespace LOFAR
{

//# Forward Declarations
class DataHolder;
class BlobOStream;
class BlobIStream;

/**
  Class DataHolder is the abstract base class for all data holders
  in the CEPFrame environment. Its main purpose is to offer a common interface
  to a class like WorkHolder. Apart from that it also offers some common
  functionality to the classes derived from it.

  DataHolder has an internal class called DataPacket. This class holds
  the data of a DataHolder class. A Class derived from DataHolder
  should also have an internal class to hold its data. That class should
  be derived from DataHolder::DataPacket.
  The basic DataPacket class offers some functions to set, get, and
  compare the timestamp of a data packet.

  The constructors of a class derived from DataHolder should always
  call the function setDataPacket in order to make their DataPacket object
  known to this base class.

  Prove \code list<table> \endcode or \<table\>.
  \code
    main()
    {
       list<table> l;
    }
  \endcode
*/

class DataBlobExtra
{
public:
  // Construct a DataHolder with a default name.
  // The useExtraFields argument must be set to true if the DataHolder
  // wants to add arbitrary fields to the blob using .
  DataBlobExtra (const string& name, int version, DataHolder*);

  ~DataBlobExtra();

  // Initialize the extra output blob holding arbitrary fields.
  // The return reference can be used to store the fields in.
  // It is meant for DataHolders writing data.
  BlobOStream& createBlock();

  // Get access to the extra input blob holding arbitrary fields.
  // It fills the version of the extra data blob.
  BlobIStream& openBlock (int& version);

  // Write the extra block into the main block.
  void write();

  // Get the name of the extra data blob.
  const std::string& getName() const;

  // Get the version of the extra data blob.
  int getVersion();

private:
  // Forbid copy constructor.
  DataBlobExtra (const DataBlobExtra&);

  // Forbid assignment.
  DataBlobExtra& operator= (const DataBlobExtra&);


  BlobOStream* itsOut;
  BlobIStream* itsIn;
  BlobOBufChar itsBufOut;   //# output buffer for extra blob data
  BlobIBufChar itsBufIn;    //# output buffer for extra blob data
  std::string  itsName;     //# blob type name for extra data
  int          itsVersion;  //# blob version for extra data
  char*        itsDataPtr;  //# Pointer to extra block in the main block
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


} // end namespace

#endif
