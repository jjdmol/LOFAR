//  DH_Database.cc: Implementation Database DataHolder
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

#include <DH_Database.h>
#include <PO_DH_Database.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{

DH_Database::DH_Database (const string& name, const string& type)
  : DataHolder (name, type) {}
  
bool DH_Database::StoreInDatabase (int, int, char *, int) {
  cout << "DH_Database::StoreInDatabase () has been called." << endl;
  cout << "  This probably means that you have forgotten to" << endl;
  cout << "  derive one or more of your DataHolder sub-classes" << endl;
  cout << "  from DH_Database." << endl;

  return true; 
}


bool DH_Database::RetrieveFromDatabase (int, int, char *, int) { 
  cout << "DH_Database::RetrieveFromDatabase () called." << endl;
  cout << "  This probably means that you have forgotten to" << endl;
  cout << "  derive one or more of your DataHolder sub-classes" << endl;
  cout << "  from DH_Database." << endl;


  return true;
}

}



