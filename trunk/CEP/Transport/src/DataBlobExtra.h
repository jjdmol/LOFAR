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
class BlobString;

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
  // Construct with the given name and version for the given DataHolder.
  DataBlobExtra (const string& name, int version, DataHolder*);

  ~DataBlobExtra();

  // Initialize the extra output blob holding arbitrary fields.
  // The return reference can be used to store the fields in.
  // It is meant for DataHolders writing data.
  BlobOStream& createBlock();

  // Clear the extra output blob.
  void clearBlock();

  // Get access to the extra input blob holding arbitrary fields.
  // It fills the version of the extra data blob.
  // found is set to false if there is no extra blob.
  BlobIStream& openBlock (bool& found, int& version, const BlobString& data);

  // Write the extra block into the main block.
  void write();

  // Get the name of the extra data blob.
  const std::string& getName() const;

  // Get the version of the extra data blob.
  int getVersion();

  // Clear the output blob.
  void clearOut()
    { itsCreateDone = false; }

  // Get read access to the blob used last (i.e. the blob opened or created).
  // The first version assures that there is a blob.
  // <group>
  BlobIStream& getBlock();
  BlobIStream& getBlock (bool& found, int& version);
  // <group>

private:
  // Forbid copy constructor.
  DataBlobExtra (const DataBlobExtra&);

  // Forbid assignment.
  DataBlobExtra& operator= (const DataBlobExtra&);


  BlobOStream* itsOut;
  BlobIStream* itsIn;
  BlobOBufChar itsBufOut;     //# write buffer for extra blob data
  BlobIBufChar itsBufIn;      //# read buffer for extra blob data
  std::string  itsName;       //# blob type name for extra data
  int          itsVersion;    //# blob version for extra data
  char*        itsDataPtr;    //# pointer to extra block in the main block
  bool         itsCreateDone; //# true = createBlock has been done
  int          itsLastDone;   //# 0=nothing, 1=create, 2=open done as last
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
