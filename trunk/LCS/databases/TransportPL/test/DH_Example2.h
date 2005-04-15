//# DH_Example2.h: Example DataHolder storing individual fields
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

#ifndef LIBTRANSPORT_TEST_DH_EXAMPLE2_H
#define LIBTRANSPORT_TEST_DH_EXAMPLE2_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <TransportPL/DH_PL.h>
#include <TransportPL/PO_DH_PL.h>
#include <Common/lofar_complex.h>


namespace LOFAR
{

// This class is an example DataHolder.
// It is the same as DH_Example, but it stores the Counter as an extra
// column in the database table.
//
// All extra code needed to store the Counter column is marked with the
// //PL comment at the end of the line (also in the .cc file).

class DH_Example2: public DH_PL
{
public:
  typedef PL::TPersistentObject<DH_Example2> PO_DH_EX;          //PL
  typedef fcomplex BufferType;

  explicit DH_Example2 (const string& name="dh_example2",
			unsigned int nbuffer = 10,
			bool useExtra = false);

  DH_Example2(const DH_Example2&);

  virtual ~DH_Example2();

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
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

private:
  /// Forbid assignment.
  DH_Example2& operator= (const DH_Example2&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();


  int*         itsCounter;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
  PO_DH_EX*    itsPODHEX;                                       //PL
};
 
 
inline void DH_Example2::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_Example2::getCounter() const
  { return *itsCounter; }

inline DH_Example2::BufferType* DH_Example2::getBuffer()
  { return itsBuffer; }

inline const DH_Example2::BufferType* DH_Example2::getBuffer() const
  { return itsBuffer; }


  // Define the class needed to tell PL that the counter should be
  // stored as an extra field in the database table.
  namespace PL {                                                //PL
    template<>                                                  //PL
    class DBRep<DH_Example2> : public DBRep<DH_PL>              //PL
    {                                                           //PL
    public:                                                     //PL
      void bindCols (dtl::BoundIOs& cols);                      //PL
      void toDBRep (const DH_Example2&);                        //PL
    private:                                                    //PL
      int itsCounter;                                           //PL
    };                                                          //PL
  } // end namespace PL                                         //PL


} // end namespace LOFAR


#endif 
