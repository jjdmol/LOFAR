//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <iostream>

#include <GSM/PointSource.h>
#include <GSM/SkyModel.h>

#include <aips/aips.h>
#include <aips/Tables/Table.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/TableDesc.h>
#include <aips/Tables/ScaColDesc.h>
#include <aips/Tables/ArrColDesc.h>


//! create an initially empty table.
void createPointSourceTable(const std::string &tableName)
{
  TableDesc description("PointSource", "1.0", TableDesc::New);
  description.addColumn(ScalarColumnDesc<int>("NUMBER", "Catalog Number."));
  description.addColumn(ScalarColumnDesc<String>("NAME", "Name of Source, e.g. M31, Virgo A, etc..."));
  description.addColumn(ScalarColumnDesc<int>("TYPE", "Type of source."));
  description.addColumn(ArrayColumnDesc<double>("RAPARMS", "J2000 RA in radians"));
  description.addColumn(ArrayColumnDesc<double>("DECPARMS", "J2000 Dec in radians"));
  description.addColumn(ArrayColumnDesc<double>("IPARMS", "I in Jansky"));
  description.addColumn(ArrayColumnDesc<double>("QPARMS", "Q in Jansky"));
  description.addColumn(ArrayColumnDesc<double>("UPARMS", "U in Jansky"));
  description.addColumn(ArrayColumnDesc<double>("VPARMS", "V in Jansky"));

  SetupNewTable newTable(tableName, description, Table::New);
  
  Table table(newTable);
  table.addRow();
  table.addRow();
  table.addRow();
}




int main(int argc, char* argv[])
{
  using std::cout;
  using std::endl;

  if(argc == 2) {
    createPointSourceTable(argv[1]);
  } else {
    cout << "gsmtest <tablename>" <<endl;
  }
  return 0;
}
