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

#include "../src/MSVisInputAgent.h"
#include <DMI/DataArray.h>

#include <AppAgent/AppControlAgent.h>

// this function checks the agent for any events and prints them out
void checkEvents (AppAgent &agent)
{
  if( !agent.hasEvent() )
  {
    cout<<"No events pending.\n";
    return;
  }
  int count = 0;
  while( agent.hasEvent() )
  {
    FailWhen1( count++ > 100, "Too many events. I bet the agent is not flushing them." );
    HIID name;
    DataRecord::Ref data;
    bool res = agent.getEvent(name,data);
    FailWhen1( !res,"Oops, hasEvents() is True but getEvent() has failed. Debug your agent." );
    cout<<"getEvent: "<<name.toString()<<" data: "<<data.sdebug(10)<<endl;
    if( data.valid() )
    {
      // if event has data, check for a message field and print that
      string msg = (*data)[AidMessage].as_string("");
      if( msg != "" )
        cout<<"Event message: "<<msg<<endl;
    }
  }
}
    
int main (int argc,const char *argv[])
{
  using namespace MSVisAgentVocabulary;
  
  try 
  {
    Debug::initLevels(argc,argv);
    Debug::setLevel("MSVisAgent",2);

    // initialize parameter record
    DataRecord::Ref dataref;
    dataref <<= new DataRecord;

    DataRecord &args = *new DataRecord;
    dataref.dewr()[MSVisInputAgent::FParams()] <<= args;

    args[FMSName] = "test.ms";
    args[FDataColumnName] = "DATA";
    args[FTileSize] = 10;

    // setup selection
    DataRecord &select = *new DataRecord;
    args[FSelection] <<= select;

    select[FDDID] = 0;
    select[FFieldIndex] = 1;
    select[FChannelStartIndex] = 10;
    select[FChannelEndIndex]   = 20;
    select[FSelectionString] = "ANTENNA1=1 && ANTENNA2=2";

    cout<<"=================== creating agent ============================\n";
    // create agent
    MSVisInputAgent agent;
    checkEvents(agent);

    cout<<"=================== initializing agent ========================\n";
    bool res = agent.init(dataref);
    cout<<"init(): "<<res<<endl;
    checkEvents(agent);
    cout<<"hasHeader(): "<<agent.hasHeader()<<endl;
    cout<<"hasTile(): "<<agent.hasTile()<<endl;

    if( !res )
    {
      cout<<"init has failed, exiting...\n";
      return 1;
    }

    cout<<"=================== getting header ============================\n";
    DataRecord::Ref header;
    cout<<"getHeader(): "<<agent.getHeader(header)<<endl;
    cout<<header->sdebug(10)<<endl;
    checkEvents(agent);

    cout<<"=================== getting tiles =============================\n";
    int state = 1;
    while( state > 0 )
    {
      VisTile::Ref tile;
      cout<<"getNextTile(): "<<(state=agent.getNextTile(tile))<<endl;
      if( state > 0 )
        cout<<tile->sdebug(10)<<endl;
      checkEvents(agent);
    }

    cout<<"=================== end of run ================================\n";
    cout<<"hasHeader(): "<<agent.hasHeader()<<endl;
    cout<<"hasTile(): "<<agent.hasTile()<<endl;
    checkEvents(agent);

    agent.close();
  } 
  catch ( std::exception &exc ) 
  {
    cout<<"Exiting with exception: "<<exc.what()<<endl;  
    return 1;
  }
  
  return 0;  
}
