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

#ifndef CEPFRAME_DH_PL_H
#define CEPFRAME_DH_PL_H

#include <CEPFrame/DH_Database.h>		// for super-class definition
#include <Common/LofarTypes.h>			// for ulong
#include <PL/PersistenceObject.h>

using namespace std;
using namespace LOFAR::PL;

namespace LOFAR {

/// DH_PL is a DataHolder implementation based on (LOFAR/Common) PL.  Note
/// that the (current) implementation of PL uses DTL above ODBC above
/// Postgresql.

class DH_PL : public DataHolder {

public:

  explicit DH_PL (const string& name, const string& type);
  virtual ~DH_PL ();

  // get a reference to the PersistentObject.
  PL::PersistentObject& getPO(); const
			       
			       
protected:
  class DataPacket:
    public DH_Database::DataPacket
    {
      
    public:
      DataPacket () {}
      
      int AppId;
      int Tag;
      int SeqNo;
      string Status;
      int Size;
      string TimeStamp;
      string Type;
      string Name; 
      string Blob;
    };
  
private:
   
  // Internal counters to synchronize the reads and writes.
  ulong itsReadSeqNo;
  ulong itsWriteSeqNo;
  
  /// Counter for the number of DH_PL instances. Used to coordinate
    /// the connection to the database.
  static ulong theirInstanceCount;
  
  
  // Now create a DH_PL_MessageRecord object. This object is a simple
  // structure with members for storing all of the information needed for
  // a message.
  DH_PL_MessageRecord itsMr;
  
  // Create the PersistentObject class 'around' DH_PL_MessageRecord. The
  // PersistentObject is a wrapper which knows how to store a
  // DH_PL_MessageRecord on a (PL supported) database and make it persistent. 
  TPersistentObject <DH_PL::DataPacket> *itsPO;
  
};
 
inline  PL::PersistentObject& DH_PL::getPO() const {
  return itsPO;
}
 
 

}// namespace LOFAR

#endif



