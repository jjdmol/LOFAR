//#  filename.h: one line description
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef PACKAGE_FILENAME_H
#define PACKAGE_FILENAME_H

#include <PL/PersistentObject.h>
#include <PL/TPersistentObject.h>
//#include <PO_DH_PL.tcc>

class DH_PL;

namespace LOFAR
{

  // Description of class.
  class DH_PL_PO  
    {
      
    public:
      PL::TPersistentObject< DH_PL >& getTPO();
    private:

      // Create the PersistentObject class 'around' DH_PL_MessageRecord. The
      // PersistentObject is a wrapper which knows how to store a
      // DH_PL_MessageRecord on a (PL supported) database and make it
      // persistent. 
      PL::TPersistentObject< DH_PL > itsTPO;
    };
  
  inline PL::TPersistentObject< DH_PL >& DH_PL_PO::getTPO() {
    return &itsTPO;
  }
  
} // namespace LOFAR

#endif
