//  PO_DH_Database.h: Standard database persistent DH_Database
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


#ifndef CEPFRAME_PO_DH_DATABASE_H
#define CEPFRAME_PO_DH_DATABASE_H

#include <Common/lofar_string.h>

namespace LOFAR
{

bool ConnectDatabase (char * hostname);
bool DisconnectDatabase (void);

// ==========================================
// [>] DatabaseRecord

#define DATABASE_RECORD_MAX_SIZE (1024)

class DatabaseRecord {
public:

  // Accessors

  int getMessageTag    () const { return itsMessageTag; }
  int getReservedData1 () const { return itsReservedData1; }
  int getReservedData2 () const { return itsReservedData2; }
  int getReservedData3 () const { return itsReservedData3; }
  int getReservedData4 () const { return itsReservedData4; }

  int    getByteStringLength () const { return itsByteStringLength; }
  char * getByteString       () { return itsByteString;      }

  unsigned long getTimeStamp () const { return itsTimeStamp; }

  string getType () const { return itsType; }
  string getName () const { return itsName; }

  string getHumanReadableForm () const { return itsHumanReadableForm; }

  // Mutators

  void setMessageTag    (const int mt)  { itsMessageTag    = mt; }
  void setReservedData1 (const int rd1) { itsReservedData1 = rd1; }
  void setReservedData2 (const int rd2) { itsReservedData2 = rd2; }
  void setReservedData3 (const int rd3) { itsReservedData2 = rd3; }
  void setReservedData4 (const int rd4) { itsReservedData2 = rd4; }

  void setByteStringLength (const int bsl)   { itsByteStringLength = bsl; }
  void CopyFromByteString (char * dest, int size);
  void CopyToByteString (char * src, int size);

  void setTimeStamp (const unsigned long ts) { itsTimeStamp = ts; }

  void setType (const string& t) { itsType = t; }
  void setName (const string& n) { itsName = n; }

  void setHumanReadableForm (string hrf) 
    { itsHumanReadableForm = hrf; }

private:

  int    itsMessageTag;
  int    itsReservedData1;			// Reserved for future use
  int    itsReservedData2;			// Reserved for future use
  int    itsReservedData3;			// Reserved for future use
  int    itsReservedData4;			// Reserved for future use

  int    itsByteStringLength;
  char   itsByteString [DATABASE_RECORD_MAX_SIZE];

  unsigned long itsTimeStamp;

  string itsType;
  string itsName;

  // maybe store the sending and receiving identifiers too?

  string itsHumanReadableForm;			// Reserved for future use

};


#define DB_HOSTNAME "dop49"


class PO_DH_Database : public DatabaseRecord {

  bool isConnected;

public:

  bool Store (unsigned long wrseqno);
  bool Retrieve (unsigned long rdweqno);

};

}

#endif



