//  PO_DH_Database.cc: Database persistent DH_Database
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

#include <PO_DH_Database.h>
#include <Common/lofar_iostream.h>

#include <sstream>
using namespace std;

// ==========================================
// [>] Database implementation

#define TH_ARRAY_SIZE       (512)

typedef enum { msgPending, msgDiscarded } msgStatus;

int       NrOfMessages;

msgStatus     MessageStatus     [TH_ARRAY_SIZE];
int           MessageTag        [TH_ARRAY_SIZE];
int           ReservedData1     [TH_ARRAY_SIZE];
int	      ReservedData2     [TH_ARRAY_SIZE];
int	      ReservedData3     [TH_ARRAY_SIZE];
int	      ReservedData4     [TH_ARRAY_SIZE];
int           MessageSize       [TH_ARRAY_SIZE];
//void *        MessageContent    [TH_ARRAY_SIZE];
unsigned long TimeStamp         [TH_ARRAY_SIZE];
string        MessageType       [TH_ARRAY_SIZE];
string        MessageName       [TH_ARRAY_SIZE];
string        HumanReadableForm [TH_ARRAY_SIZE];
string        PseudoBlob        [TH_ARRAY_SIZE];

// ==========================================
// [>] DatabaseRecord

void DatabaseRecord::CopyFromByteString (char * dest, int size) {
  memcpy (dest, itsByteString, size);
}


void DatabaseRecord::CopyToByteString (char * src, int size) {
  memcpy (itsByteString, src, size);
}


// ==========================================
// [>] PO_DH_Database

bool PO_DH_Database::Store () {
  if (NrOfMessages == TH_ARRAY_SIZE - 1) {
    cout << "Fatal error in TH_Database::send (). Maximum number of message transfers reached." << endl;
    return false;
  }

  MessageStatus  [NrOfMessages] = msgPending;
  MessageTag     [NrOfMessages] = getMessageTag ();
  ReservedData1  [NrOfMessages] = getReservedData1 ();
  ReservedData2  [NrOfMessages] = getReservedData2 ();
  ReservedData3  [NrOfMessages] = getReservedData3 ();
  ReservedData4  [NrOfMessages] = getReservedData4 ();
  MessageSize    [NrOfMessages] = getByteStringLength ();

  //  MessageContent [NrOfMessages] = malloc (getByteStringLength ());

  // if (MessageContent [NrOfMessages] == 0) {
  //   cout << "Fatal error in TH_Database::send (). Could not allocate " 
  //   << getByteStringLength () << " memory." << endl;
  //   return false;
  // }

  TimeStamp   [NrOfMessages] = getTimeStamp ();
  MessageType [NrOfMessages] = getType ();
  MessageName [NrOfMessages] = getName ();

  int i;
  ostringstream ostr;

  for (i = 0; i < getByteStringLength (); i ++) {  
    ostr << (unsigned int) (getByteString ()) [i] << ' ';
  }
  ostr << "-1";

  PseudoBlob [NrOfMessages] = ostr.str ();

  NrOfMessages ++;

  return true;
}


bool PO_DH_Database::Retrieve () {
  int i;

  i = 0;

  // Search for the record with the correct tag.
  while (! (MessageTag [i] == getMessageTag () 
    && MessageStatus [i] == msgPending)) {
    i ++;
    if (i == NrOfMessages) {
      cout << "Fatal error in TH_Database::recv (). "
	   << "No message has been sent which can be received." 
	   << endl;
      return false;
    }
  }

  setMessageTag       (MessageTag     [i]);
  setReservedData1    (ReservedData1  [i]);
  setReservedData2    (ReservedData2  [i]);
  setReservedData3    (ReservedData3  [i]);
  setReservedData4    (ReservedData4  [i]);
  setByteStringLength (MessageSize    [i]);

  setTimeStamp (TimeStamp   [i]);
  setType      (MessageType [i]);
  setName      (MessageName [i]);

  MessageStatus [i] = msgDiscarded;

  cout << "PseudoBlob(Receive): " << PseudoBlob [i] << endl;

  int j, idx=0;
  istringstream istr (PseudoBlob[i]);

  istr >> j;
  while (j != -1) {
    * (getByteString () + idx) = (char) j;
    idx ++;
    istr >> j;
  }

  NrOfMessages ++;

  return true;
}



