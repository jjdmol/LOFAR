//  test_input.cc: tests the MSVisInputAgent class
//
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

#include "../src/MSInputAgent.h"
#include "../src/MSOutputAgent.h"
#include <DMI/DataArray.h>

#include <AppAgent/AppControlAgent.h>

int main (int argc,const char *argv[])
{
  MeasurementSet ms1("test.ms",Table::Old);
  MeasurementSet ms2("test.ms",Table::Update);
    
  using namespace MSVisAgent;
  
  try 
  {
    Debug::setLevel("MSVisAgent",2);
    Debug::initLevels(argc,argv);

      // initialize parameter record
    DataRecord params;
    DataRecord &args = params[FMSInputParams] <<= new DataRecord;
    
      args[FMSName] = "test.ms";
      args[FDataColumnName] = "DATA";
      args[FTileSize] = 10;

      // setup selection
      DataRecord &select = args[FSelection] <<= new DataRecord;

        select[FDDID] = 0;
        select[FFieldIndex] = 1;
        select[FChannelStartIndex] = 10;
        select[FChannelEndIndex]   = 20;
        select[FSelectionString] = "ANTENNA1=1 && ANTENNA2=2";
        
    DataRecord &outargs = params[FMSOutputParams] <<= new DataRecord;
    
      outargs[FWriteFlags]  = True;
      outargs[FFlagMask]    = 0xFF;
      
      outargs[FDataColumn]      = "";
      outargs[FPredictColumn]   = "MODEL_DATA";
      outargs[FResidualsColumn] = "RESIDUAL_DATA";

    cout<<"=================== creating input agent ======================\n";
    // create agent
    MSInputAgent agent;
  
    cout<<"=================== creating output agent ======================\n";
    // create agent
    MSOutputAgent outagent;

    cout<<"=================== initializing input agent ==================\n";
    bool res = agent.init(params);
    cout<<"init(): "<<res<<endl;
    cout<<"hasHeader(): "<<agent.hasHeader()<<endl;
    cout<<"hasTile(): "<<agent.hasTile()<<endl;

    if( !res )
    {
      cout<<"init has failed, exiting...\n";
      return 1;
    }
    
//    MeasurementSet ms("test.ms",Table::Update);
    
    cout<<"=================== initializing output agent ================\n";
    // initialize parameter record
    res = outagent.init(params);
    cout<<"init(): "<<res<<endl;

    if( !res )
    {
      cout<<"init has failed, exiting...\n";
      return 1;
    }

    cout<<"=================== getting header ============================\n";
    DataRecord::Ref header;
    cout<<"getHeader(): "<<agent.getHeader(header)<<endl;
    cout<<header->sdebug(10)<<endl;

    cout<<"=================== putting header ============================\n";
    cout<<"putHeader(): "<<outagent.putHeader(header)<<endl;
       
    cout<<"=================== copying tiles =============================\n";
    int state = 1;
    while( state > 0 )
    {
      VisTile::Ref tile;
      cout<<"getNextTile(): "<<(state=agent.getNextTile(tile))<<endl;
      if( state > 0 )
      {
        cout<<tile->sdebug(10)<<endl;
        // add columns to tile
        LoShape shape = tile->data().shape();
        VisTile::Format::Ref newformat;
        newformat <<= new VisTile::Format(tile->format());
        newformat().add(VisTile::PREDICT,Tpfcomplex,tile->data().shape())
                   .add(VisTile::RESIDUALS,Tpfcomplex,tile->data().shape());
        // fill the columns
        tile().wpredict()   = -tile->data();
        tile().wresiduals() = conj(tile->data());

        cout<<"putNextTile(): "<<outagent.putNextTile(tile)<<endl;
      }
    }

    cout<<"=================== end of run ================================\n";
    cout<<"hasHeader(): "<<agent.hasHeader()<<endl;
    cout<<"hasTile(): "<<agent.hasTile()<<endl;

    agent.close();
    outagent.close();
  } 
  catch ( std::exception &exc ) 
  {
    cout<<"Exiting with exception: "<<exc.what()<<endl;  
    return 1;
  }
  
  return 0;  
}
