//#  VisDataMux.cc
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

#include "VisDataMux.h"
#include "AID-MeqServer.h"
#include <VisCube/VisVocabulary.h>
#include <MEQ/Forest.h>
#include <MEQ/Cells.h>
#include <MEQ/Request.h>
    
//##ModelId=3F9FF71B006A
Meq::VisDataMux::VisDataMux (Meq::Forest &frst)
    : forest_(frst)
{
  // use reasonable default
  handlers_.resize(VisVocabulary::ifrNumber(30,30)+1);
}

//##ModelId=3FA1016000B0
void Meq::VisDataMux::init (const DataRecord &rec)
{
  out_columns_.clear();
  out_colnames_.clear();
  // setup output column indices
  if( rec[FOutputColumn].exists() )
  {
    out_colnames_ = rec[FOutputColumn];
    out_columns_.resize(out_colnames_.size());
    const VisTile::NameToIndexMap &colmap = VisTile::getNameToIndexMap();
    for( uint i=0; i<out_colnames_.size(); i++ )
    {
      VisTile::NameToIndexMap::const_iterator iter = 
          colmap.find(out_colnames_[i] = struppercase(out_colnames_[i]));
      FailWhen(iter==colmap.end(),"unknown output column "+out_colnames_[i]);
      out_columns_[i] = iter->second;
      cdebug(2)<<"indicated output column: "<<out_colnames_[i]<<endl;
    }
  }
}

//##ModelId=3F716E98002E
void Meq::VisDataMux::addNode (Node &check_node)
{
  // return if the node is not a VisHandlerNode
  VisHandlerNode *node = dynamic_cast<VisHandlerNode*>(&check_node);
  if( !node )
    return;
  cdebug(2)<<"node is a visdata handler, adding to data mux\n";
  // form data ID from state record
  const DataRecord &state = node->state();
  int did;
  try
  {
    did = formDataId(state[FStation1Index].as<int>(),
                     state[FStation2Index].as<int>());
  }
  catch(...)
  {
    Throw(node->objectType().toString()+
          " state record is missing station and/or correlation identifiers");
  }
  // let the node know about its data id
  node->setDataId(did);
  // add list of handlers for this data id (if necessary)
  if( did >= int(handlers_.size()) )
    handlers_.resize(did+100);
  // add node to list of handlers
  VisHandlerList &hlist = handlers_[did];
  VisHandlerList::const_iterator iter = hlist.begin();
  // ... though not if it's already there on the list
  for( ; iter != hlist.end(); iter++ )
    if( *iter == node )
      return;
  hlist.push_back(node); 
}

//##ModelId=3F716EAA0106
void Meq::VisDataMux::removeNode (Node &check_node)
{
  // return if the node is not a VisHandlerNode
  VisHandlerNode *node = dynamic_cast<VisHandlerNode*>(&check_node);
  if( !node )
    return;
  cdebug(2)<<"node is a visdata handler, removing from spigot mux\n";
  int did = node->dataId();
  if( did < 0 )
  {
    cdebug(2)<<"no data ID in node: not attached to this spigot mux?\n";
    return;
  }
  // erase from handler list
  VisHandlerList &hlist = handlers_[did];
  VisHandlerList::iterator iter = hlist.begin();
  for( ; iter != hlist.end(); iter++ )
    if( *iter == node )
    {
      hlist.erase(iter);
      break;
    }
}

//##ModelId=3F992F280174
int Meq::VisDataMux::formDataId (int sta1,int sta2)
{
  return VisVocabulary::ifrNumber(sta1,sta2);
}

//##ModelId=3F98DAE6024A
void Meq::VisDataMux::deliverHeader (const DataRecord &header)
{
  // check header for number of stations, use a reasonable default
  cdebug(3)<<"got header: "<<header.sdebug(DebugLevel)<<endl;
  int nstations = header[FNumStations].as<int>(-1);
  if( nstations>0 )
  {
    cdebug(2)<<"header indicates "<<nstations<<" stations\n";
  }
  else
  {
    nstations = 30;
    cdebug(2)<<"no NumStations parameter in header, assuming 30\n";
  }
  handlers_.resize(VisVocabulary::ifrNumber(nstations,nstations)+1);
  // get frequencies 
  LoVec_double freq = header[VisVocabulary::FChannelFreq];
  minfreq = min(freq);
  maxfreq = max(freq);
  // init output tile format
  out_format_.attach(header[FTileFormat].as_p<VisTile::Format>(),DMI::READONLY);
  for( uint i=0; i<out_columns_.size(); i++ )
  {
    if( out_format_->defined(out_columns_[i]) )
    {
      cdebug(3)<<"output column "<<out_colnames_[i]<<" already present in tile format"<<endl;
    }
    else
    {
      cdebug(2)<<"adding output column "<<out_colnames_[i]<<" to tile format\n";
      out_format_.privatize(DMI::WRITE);
      out_format_().add(out_columns_[i],out_format_->type(VisTile::DATA),
                        out_format_->shape(VisTile::DATA));
    }
  }
}

//##ModelId=3F950ACA0160
int Meq::VisDataMux::deliverTile (VisTile::Ref::Copy &tileref)
{
  int result_flag = 0;
  int did = formDataId(tileref->antenna1(),tileref->antenna2());
  if( did > int(handlers_.size()) )
  {
    cdebug(4)<<"no handlers for did "<<did<<", skipping tile "<<tileref->sdebug(DebugLevel-2)<<endl;
    return 0;
  }
  VisHandlerList &hlist = handlers_[did];
  if( hlist.empty() )
  {
    cdebug(4)<<"no handlers for did "<<did<<", skipping tile "<<tileref->sdebug(DebugLevel-2)<<endl;
    return 0;
  }
  else
  {
    cdebug(3)<<"have handlers for did "<<did<<", got tile "<<tileref->sdebug(DebugLevel-1)<<endl;
    // For now, generate the request right here.
    Cells::Ref cellref(DMI::ANONWR);
    VisHandlerNode::fillCells(cellref(),*tileref,minfreq,maxfreq);
    Request::Ref reqref;
    Request &req = reqref <<= new Request(cellref.deref_p(),0);
    forest_.assignRequestId(req);
    cdebug(3)<<"have handler, generated request id="<<req.id()<<endl;
    // deliver to all known handlers
    VisHandlerList::iterator iter = hlist.begin();
    for( ; iter != hlist.end(); iter++ )
      result_flag |= (*iter)->deliver(req,tileref,out_format_);
  }
  return result_flag;
}



