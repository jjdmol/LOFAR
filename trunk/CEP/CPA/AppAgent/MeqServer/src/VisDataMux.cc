#include "VisDataMux.h"
#include <VisCube/VisVocabulary.h>
#include <MEQ/Forest.h>
#include <MEQ/Cells.h>
#include <MEQ/Request.h>
    
namespace MEQ    
{
  const HIID FStation1      = AidStation|1|AidIndex,
             FStation2      = AidStation|2|AidIndex;
}

MEQ::VisDataMux::VisDataMux (MEQ::Forest &frst)
    : forest_(frst)
{
}

//##ModelId=3F98DAE6024A
void MEQ::VisDataMux::init (const DataRecord &header)
{
  // check header for number of stations, use a reasonable default
  int nstations = header[AidNum|AidAntenna].as<int>(30);
  handlers_.resize(VisVocabulary::ifrNumber(nstations,nstations)+1);
}

//##ModelId=3F992F280174
int MEQ::VisDataMux::formDataId (int sta1,int sta2)
{
  return VisVocabulary::ifrNumber(sta1,sta2);
}
    
//##ModelId=3F716E98002E
void MEQ::VisDataMux::addNode (Node &check_node)
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
    did = formDataId(state[FStation1].as<int>(),
                     state[FStation2].as<int>());
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
    handlers_.resize(did+1);
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
void MEQ::VisDataMux::removeNode (Node &check_node)
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

//##ModelId=3F950ACA0160
int MEQ::VisDataMux::deliver (VisTile::Ref::Copy &tileref)
{
  int result_flag = 0;
  int did = formDataId(tileref->antenna1(),tileref->antenna2());
  VisHandlerList &hlist = handlers_[did];
  if( hlist.empty() )
  {
    cdebug(4)<<"no handlers for did "<<did<<", skipping tile "<<tileref->sdebug(DebugLevel-2)<<endl;
  }
  else
  {
    cdebug(3)<<"have handlers for did "<<did<<", got tile "<<tileref->sdebug(DebugLevel-1)<<endl;
    // For now, generate the request right here.
    Request req(VisHandlerNode::makeCells(*tileref),False);
    forest_.assignRequestId(req);
    cdebug(3)<<"have handler, generated request id="<<req.getId()<<endl;
    // deliver to all known handlers
    VisHandlerList::iterator iter = hlist.begin();
    for( ; iter != hlist.end(); iter++ )
      result_flag |= (*iter)->deliver(req,tileref);
  }
  return result_flag;
}

