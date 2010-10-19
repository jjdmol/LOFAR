//# DH_ExampleExtra2.h: Example DataHolder storing individual fields
//#
//# Copyright (C) 2004
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

#ifndef TRANSPORTPL_TEST_DH_EXAMPLEEXTRA2_H
#define TRANSPORTPL_TEST_DH_EXAMPLEEXTRA2_H


#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <Common/lofar_complex.h>


namespace LOFAR
{

// This class is an example DataHolder. This DataHolder shows how to use the
// extra blob to store variable sized and different types of data in.
// It is the same as DH_ExampleExtra, but it stores the Counter as an extra
// column in the database table.
//
// All extra code needed to store the Counter column is marked with the
// //PL comment at the end of the line (also in the .cc file).

class DH_ExampleExtra2: public DH_PL
{
public:
  typedef PL::TPersistentObject<DH_ExampleExtra2> PO_DH_EX;          //PL
  typedef fcomplex BufferType;

  explicit DH_ExampleExtra2 (const string& name="dh_example2",
			unsigned int nbuffer = 10);

  DH_ExampleExtra2(const DH_ExampleExtra2&);

  virtual ~DH_ExampleExtra2();

  DataHolder* clone() const;

  // Get a reference to the PersistentObject.                   //PL
  virtual PL::PersistentObject& getPO() const;	                //PL

  // Create a TPO object and set the table name in it.          //PL
  virtual void initPO (const string& tableName);                //PL

  /// Set the Counter attribute.
  void setCounter (int counter);
  /// Get the Counter attribute.
  int getCounter() const;

  /// Allocate the buffers.
  /// Note: call this after this DataHolder has been connected to make 
  /// sure the buffers are allocated in the right place (in normal or 
  /// shared memory in case of connection with TH_ShMem)
  virtual void init();

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  /// Get a BlobOStream object to fill the variable buffer (extra blob).
  BlobOStream& fillVariableBuffer();

  /// Get a BlobIStream object to read from the variable buffer.
  BlobIStream& readVariableBuffer();
  BlobIStream& readVariableBuffer(bool& found, int& version);

  /// Clear the variable buffer.
  void clearVariableBuffer();

private:
  /// Forbid assignment.
  DH_ExampleExtra2& operator= (const DH_ExampleExtra2&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();


  int*         itsCounter;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
  PO_DH_EX*    itsPODHEX;                                       //PL
};
 
 
inline void DH_ExampleExtra2::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_ExampleExtra2::getCounter() const
  { return *itsCounter; }

inline DH_ExampleExtra2::BufferType* DH_ExampleExtra2::getBuffer()
  { return itsBuffer; }

inline const DH_ExampleExtra2::BufferType* DH_ExampleExtra2::getBuffer() const
  { return itsBuffer; }

inline BlobOStream& DH_ExampleExtra2::fillVariableBuffer()
  { return createExtraBlob(); }

inline BlobIStream& DH_ExampleExtra2::readVariableBuffer()
  { return getExtraBlob(); }

inline BlobIStream& DH_ExampleExtra2::readVariableBuffer(bool& found, int& version)
  { return getExtraBlob(found, version); }

inline void DH_ExampleExtra2::clearVariableBuffer()
  { clearExtraBlob(); }

  // Define the class needed to tell PL that the counter should be
  // stored as an extra field in the database table.
  namespace PL {                                                //PL
    template<>                                                  //PL
    class DBRep<DH_ExampleExtra2> : public DBRep<DH_PL>         //PL
    {                                                           //PL
    public:                                                     //PL
      void bindCols (dtl::BoundIOs& cols);                      //PL
      void toDBRep (const DH_ExampleExtra2&);                   //PL
    private:                                                    //PL
      int itsCounter;                                           //PL
    };                                                          //PL
  } // end namespace PL                                         //PL


} // end namespace LOFAR


#endif 
