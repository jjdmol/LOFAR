//  VisFileInputHeader.cc: common base class for VisInputAgents that read files
    
//  Copyright (C) 2002
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

#include "VisFileInputAgent.h"

static int dum = aidRegistry_VisAgent();

//##ModelId=3DF9FECE03AB
DataRecord & VisFileInputAgent::initHeader ()
{
  DataRecord *hdr;
  header_ <<= hdr = new DataRecord;
  return *hdr;
}

//##ModelId=3DF9FECF009D
int VisFileInputAgent::hasHeader () const
{
  if( filestate_ == HEADER )
    return SUCCESS;
  else if( filestate_ == DATA )
    return OUTOFSEQ;
  else 
    return CLOSED;
}

//##ModelId=3DF9FECE03D1
int VisFileInputAgent::getHeader (DataRecord::Ref &hdr,bool)
{
  // is the file state correct for a header?
  int res = hasHeader();
  if( res == SUCCESS )
  {
    hdr.copy(header_);
    setFileState(DATA);
    return SUCCESS;
  }
  else
    return res;
}


//##ModelId=3DF9FECF00BF
int VisFileInputAgent::hasTile   () const
{
  if( filestate_ == HEADER )
    return OUTOFSEQ;
  else if( filestate_ == DATA )
    return SUCCESS;
  else 
    return CLOSED;
}

//##ModelId=3DF9FECE024A
void VisFileInputAgent::generateEvent (const HIID &evname)
{
  DataRecord::Ref dum;
  generateEvent(evname,dum);
}

//##ModelId=3DF9FECE02BA
void VisFileInputAgent::generateEvent (const HIID &evname,DataRecord::Ref &data)
{
  event_ = evname;
  if( data.valid() )
    eventdata_ = data;
  else
    eventdata_.detach();
}


//##ModelId=3DFDFC07004C
string VisFileInputAgent::fileStateString () const
{
  switch( filestate_ )
  {
    case CLOSED:    return "CLOSED";
    case HEADER:    return "HEADER";
    case DATA:      return "DATA";
    case ENDFILE:   return "ENDFILE";
    case ERROR:     return "ERROR";
    default:        return "unknown";
  }
}


//##ModelId=3DF9FECE0154
void VisFileInputAgent::setFileState (FileState state)
{
  if( filestate_ == state )
    return;
  // generate events depending on new state
  switch( state )
  {
    case CLOSED:
        break;
    
    case HEADER:
        generateEvent(HeaderEvent());
        break;
        
    case DATA:
        generateEvent(DataEvent());
        break;
        
    case ENDFILE:
        generateEvent(EndDataEvent());
        break;
        
    case ERROR:
        generateEvent(InputErrorEvent());
        break;
        
    default:
        Throw(Debug::ssprintf("Unknown filestate: %d",state));
  }
  filestate_ = state;
}

//##ModelId=3DF9FECE01CA
void VisFileInputAgent::setErrorState (const string &msg)
{
  filestate_ = ERROR;
  DataRecord::Ref data(new DataRecord,DMI::ANONWR);
  data()[AidMessage] = msg;
  generateEvent(InputErrorEvent(),data);
}

//##ModelId=3DF9FECF00DB
bool VisFileInputAgent::getEvent (HIID &ev,DataRecord::Ref &evdata,bool wait)
{
  if( event_.empty() )
  {
    FailWhen( wait,"VisFileInputAgent can't wait for event: would wait forever");
    return False;
  }
  ev = event_;
  if( eventdata_.valid() )
    evdata = eventdata_;  // transfers event data
  else
    evdata.detach();
  // clears event
  event_.clear();
  return True;
}
      
//##ModelId=3DF9FECF01DA
bool VisFileInputAgent::hasEvent (const HIID &mask,bool)
{
  if( event_.empty() )
    return False;
  if( mask.empty() )
    return True;
  return event_.matches(mask);
}
