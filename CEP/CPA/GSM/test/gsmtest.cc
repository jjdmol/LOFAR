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
#include <string>

#include <GSM/PointSource.h>
#include <GSM/SkyModel.h>

#include <aips/aips.h>
#include <aips/Exceptions.h>
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
  
  //Table table(newTable);
  //table.addRow();
  //table.addRow();
  //table.addRow();
}



void addPointSource(unsigned int               number, 
                    const std::string&         name,
                    double                     ra,
                    double                     dec,
                    const std::vector<double>& flux,
                    Table&                     table)
{
  GSM::PointSource ps(ra, dec, number, name, flux);
  table.addRow();
  ps.store(table, table.nrow()-1);
}


int main(int argc, char* argv[])
try
{
  using std::cout;
  using std::endl;

  if(argc == 2) {
    //    createPointSourceTable(argv[1]);
    for(int i = 0; i < argc; i++) {
      cout << argv[i] << endl;
    }

    Table table(argv[1],Table::Update);
    std::vector<double> Flux(4);

    Flux[GSM::I] = 2.2;
    Flux[GSM::Q] = 0;
    Flux[GSM::U] = 0;
    Flux[GSM::V] = 0;
    
    addPointSource(1, "8C1435+635-a",
                   (14.0*15+35.0/60)*M_PI/180.0, 63.5*M_PI/180.0, Flux,
                   table);
    
    Flux[GSM::I] = 1.8;
    Flux[GSM::Q] = 0;
    Flux[GSM::U] = 0;
    Flux[GSM::V] = 0;
    
    addPointSource(1, "8C1435+635-b",
                   (14.0*15+35.0/60)*M_PI/180.0, 63.4*M_PI/180.0, Flux,
                   table);
    
    
    Flux[GSM::I] = 1;
    Flux[GSM::Q] = 0;
    Flux[GSM::U] = 0;
    Flux[GSM::V] = 0;
    
    addPointSource(1, "A",
                   (14.0*15+39.0/60)*M_PI/180.0, 63.8*M_PI/180.0, Flux,
                   table);
    
    
    Flux[GSM::I] = 0.5;
    Flux[GSM::Q] = 0;
    Flux[GSM::U] = 0;
    Flux[GSM::V] = 0;
    
    addPointSource(1, "B",
                   (14.0*15+50.0/60)*M_PI/180.0, 64.0*M_PI/180.0, Flux,
                   table);
  } else {
    cout << "gsmtest <tablename>" <<endl;
  }
  return 0;
}
catch(AipsError& error)
{
  cout << error.getMesg() << endl;
}
catch(...)
{
  cout << "Unhandled exception caught" << endl;
}
