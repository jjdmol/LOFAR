//#  PersistenceBroker.cc: handle save/retrieve of persistent objects.
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

#include <PL/PersistenceBroker.h>

namespace LCS
{
  namespace PL
  {

    template<typename T>
    void PersistenceBroker::save(TPersistentObject<T>& tpo,
				 enum SaveMode sm) const
    {
      switch(sm) {
      case AUTOMATIC:
	tpo.save(this);
	break;
      case INSERT:
	tpo.insert(this);
	break;
      case UPDATE:
	tpo.update(this);
	break;
      default:
	THROW(BrokerException,"Invalid SaveMode");
      }
    }

    template<typename T>
    Collection<TPersistentObject<T> >
    PersistenceBroker::retrieveCollection(const Query& q) const
    {
//       TPersistentObject<T>::retrieve(
      return Collection<TPersistentObject<T> >();
    }

  } // namespace PL

} // namespace LCS
