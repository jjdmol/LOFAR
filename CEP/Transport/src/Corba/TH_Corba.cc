//  TH_Corba.cc: Corba transport mechanism
//
//  Copyright (C) 2000-2002
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
//
//////////////////////////////////////////////////////////////////////

#include <Corba/TH_Corba.h>
#include <Corba/BS_Corba.h>
#include <Common/Debug.h>

/// declare tranportholder corba prototype
TH_Corba TH_Corba::proto;

TH_Corba::TH_Corba()
: itsCorbaOut(0),
  itsCorbaIn (0)
{
}

TH_Corba::~TH_Corba()
{
}

TH_Corba* TH_Corba::make() const
{
  return new TH_Corba();
}

string TH_Corba::getType() const
{
  return "TH_Corba";
}

void TH_Corba::init(int argc, char *argv[])
{
}

void TH_Corba::finalize()
{
}

bool TH_Corba::recvBlocking(void* buf, int nbytes, int tag)
{ 
  if (itsCorbaIn == 0)
  {
      itsCorbaIn = new CorbaTransportIn(tag);
  }
  
  try
  {
      itsCorbaIn->getBuffer (buf, nbytes);
  }
  catch(const CORBA::Exception& e)
  {
      cerr << e << endl;
  }

  return true;
}

bool TH_Corba::recvNonBlocking(void* buf, int nbytes, int tag)
{
  cerr << "**Warning** TH_Corba::recvNonBlocking() is not implemented. " 
       << "recvBlocking() is used instead." << endl;    
  return recvBlocking(buf, nbytes, source, tag);
}

bool TH_Corba::waitForReceived(void*, int, int)
{
  return true;
}

bool TH_Corba::sendBlocking(void* buf, int nbytes, int tag)
{
    if (itsCorbaOut == 0) {
	itsCorbaOut = new CorbaTransportOut(BS_Corba::getPOA(),
					    BS_Corba::getPOAManager(),
					    tag);
    }

    try
    {
	itsCorbaOut->putBuffer (buf, nbytes);
    }
    catch(const CORBA::Exception& e)
    {
	cerr << e << endl;
    }

    return true;
}

bool TH_Corba::sendNonBlocking(void* buf, int nbytes, int tag)
{
  cerr << "**Warning** TH_Corba::sendNonBlocking() is not implemented. " 
       << "The sendBlocking() method is used instead." << endl;    
  return sendBlocking(buf, nbytes, destination, tag);

}

bool TH_Corba::waitForSent(void*, int, int)
{
  return true;
}
