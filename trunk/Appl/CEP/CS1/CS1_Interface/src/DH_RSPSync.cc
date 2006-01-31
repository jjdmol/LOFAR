//  DH_RSPSync.cc: DataHolder used to synchronize incoming RSP data
//
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


#include <DH_RSPSync.h>
#include <Blob/KeyValueMap.h>

namespace LOFAR
{

DH_RSPSync::DH_RSPSync (const string& name)
  : DataHolder    (name, "DH_RSPSync")
{
}

DH_RSPSync::DH_RSPSync(const DH_RSPSync& that)
  : DataHolder(that)
{
}

DH_RSPSync::~DH_RSPSync()
{
}

DataHolder* DH_RSPSync::clone() const
{
  return new DH_RSPSync(*this);
}

void DH_RSPSync::init()
{
  // this could be done nicer, but it works for now because SyncStamp doesn't contain
  // any pointers
  addField("RSPsyncStamp", BlobField<char>(1, sizeof(timestamp_t)));
  createDataBlock();
  fillDataPointers();
  itsSyncStamp->setStamp(0, 0);
}

void DH_RSPSync::fillDataPointers() {
  itsSyncStamp = (timestamp_t*)getData<char> ("RSPsyncStamp");
}

}
