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

#include "FileInputAgent.h"
#include "AID-VisAgent.h"

namespace VisAgent
{
  
static int dum = aidRegistry_VisAgent();


//##ModelId=3DF9FECE03AB
DataRecord & FileInputAgent::initHeader ()
{
  DataRecord *hdr;
  header_ <<= hdr = new DataRecord;
  return *hdr;
}

//##ModelId=3DF9FECF009D
int FileInputAgent::hasHeader ()
{
  if( suspended() ) 
    return WAIT;
  if( fileState() == HEADER )
    return SUCCESS;
  else if( fileState() == DATA )
    return OUTOFSEQ;
  else 
    return CLOSED;
}

//##ModelId=3DF9FECE03D1
int FileInputAgent::getHeader (DataRecord::Ref &hdr,int wait)
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
  {
    FailWhen( res == WAIT && wait != NOWAIT,
        "can't wait here: would block indefinitely" );
    return res;
  }
}

//##ModelId=3DF9FECF00BF
int FileInputAgent::hasTile   ()
{
  if( suspended() )
    return WAIT;
  if( fileState() == HEADER )
    return OUTOFSEQ;
  else if( fileState() == DATA )
    return SUCCESS;
  else 
    return CLOSED;
}

//##ModelId=3E2C299201D6
int FileInputAgent::state() const
{
  return fileState() != FILEERROR 
          ? SUCCESS 
          : ERROR;
}

//##ModelId=3E2C2999029A
string FileInputAgent::stateString() const
{
  return fileState() == FILEERROR 
         ? "ERROR " + errorString()
         :  "OK (" + fileStateString() + ")";
}



}

