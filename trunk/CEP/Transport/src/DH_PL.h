//  DH_PL.h: Standard database persistent DH_Database
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////////

#ifndef LIBTRANSPORT_DH_PL_H
#define LIBTRANSPORT_DH_PL_H

#include <Transport/DataHolder.h>		// for super-class definition
#include <Common/LofarTypes.h>			// for int64
#include <PL/PLfwd.h>


namespace LOFAR {

/// DH_PL is a DataHolder implementation based on (LOFAR/Common) PL.
// Note that the (current) implementation of PL uses DTL which uses ODBC.
// Usually a PostgreSQL database is used, but every ODBC-compliant database
// system can be used.
//
// If a programmer wants it to make possible that a DataHolder can be stored
// in a database, the particular DH class should be derived from DH_PL.
// The data are stored in a blob in the database, and no fields are stored
// individually, thus are not easily visible when accessing the database
// directly. In that case the derived DH class does not need extra code.
// An example for this case can be found in test/DH_Example.
// <br>If one or more fields should be stored individually, the user needs
// to add some extra code to tell PL which fields should be stored where.
// An example can be found in test/DH_Example2. Please note that such
// fields are also part of the blob, thus are stored twice. The only
// advantage of storing them individually is that they are easier visible
// in the database.

class DH_PL : public DataHolder {

public:
  typedef PL::TPersistentObject<DH_PL> PO_DH_PL;

  explicit DH_PL (const string& name="DH_PL", const string& type="DH_PL");

  virtual ~DH_PL ();

  virtual DataHolder* clone() const;

  // Get a reference to the PersistentObject.
  // It should be overidden in a derived class defining its own TPO object.
  virtual PL::PersistentObject& getPO() const;		       

  // Pass the seqnr and get a reference to the PersistentObject.
  PL::PersistentObject& preparePO (int id, int tag, int64 seqnr);
  
  // Create a TPO object and set the table name in it.
  // It should be overridden in a derived class having its own TPO object.
  virtual void initPO (const string& tableName);

  // Get the variable values.
  // <group>
  int getId() const
    { return itsAppId; }
  int getTag() const
    { return itsTag; }
  int64 getSeqNr() const
    { return itsSeqNr; }
  // </group>

protected:
  // Copy constructor.
  DH_PL (const DH_PL&);

private:
  // Forbid assignment.
  DH_PL& operator= (const DH_PL&);
  
  int   itsAppId;
  int   itsTag;
  int64 itsSeqNr;
  PO_DH_PL* itsPODHPL;
};
 
 
inline PL::PersistentObject& DH_PL::preparePO (int id, int tag, int64 seqnr)
{
  itsAppId = id;
  itsTag   = tag;
  itsSeqNr = seqnr;
  return getPO();
} 


}// namespace LOFAR

#endif
