//#  DBRepMeta.cc: implementation of the DBRepMeta class.
//#
//#  Copyright (C) 2002-2004
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

#include <PL/DBRepMeta.h>

namespace LOFAR
{
  namespace PL
  {

    void DBRepMeta::bindColsMeta(dtl::BoundIOs& cols)
    {
      cols["ObjId"]     == itsOid;
      cols["Owner"]     == itsOwnerOid;
      cols["VersionNr"] == itsVersionNr;
    }

    void DBRepMeta::toDBRepMeta(const PersistentObject& po)
    {
      itsOid       = po.metaData().oid()->get();
      itsOwnerOid  = po.metaData().ownerOid()->get();
      itsVersionNr = po.metaData().versionNr();
    }

    void DBRepMeta::fromDBRepMeta(PersistentObject& po) const
    {
      po.metaData().oid()->set      (itsOid);
      po.metaData().ownerOid()->set (itsOwnerOid);
      po.metaData().versionNr()   =  itsVersionNr;
    }

  } // namespace PL

} // namespace LOFAR
