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

#include <libTransport/DataHolder.h>		// for super-class definition
#include <Common/LofarTypes.h>			// for ulong
#include <PL/PLfwd.h>


namespace LOFAR {

/// DH_PL is a DataHolder implementation based on (LOFAR/Common) PL.  Note
/// that the (current) implementation of PL uses DTL above ODBC above
/// Postgresql.

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
  PL::PersistentObject& preparePO (int id, int tag, int seqnr);		       
  
  // Create a TPO object and set the table name in it.
  // It should be overidden in a derived class having its own TPO object.
  virtual void initPO (const string& tableName);

  // Get the variable values.
  // <group>
  int getId() const
    { return itsAppId; }
  int getTag() const
    { return itsTag; }
  int getSeqNr() const
    { return itsSeqNr; }
  // </group>

protected:
  // Copy constructor.
  DH_PL (const DH_PL&);

private:
  // Forbid assignment.
  DH_PL& operator= (const DH_PL&);
  
  int itsAppId;
  int itsTag;
  int itsSeqNr;
  PO_DH_PL* itsPODHPL;
};
 
 
inline PL::PersistentObject& DH_PL::preparePO (int id, int tag, int seqnr)
{
  itsAppId = id;
  itsTag   = tag;
  itsSeqNr = seqnr;
  return getPO();
} 


}// namespace LOFAR

#endif
