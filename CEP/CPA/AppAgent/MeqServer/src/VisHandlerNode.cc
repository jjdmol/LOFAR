#include "VisHandlerNode.h"
#include "AID-MeqServer.h"
#include <MEQ/Cells.h>

namespace Meq {

InitDebugContext(VisHandlerNode,"MeqVisHandler");
  
//##ModelId=3F98DAE60319
VisHandlerNode::VisHandlerNode (int nchildren,const HIID *labels,int nmandatory)
 : Node(nchildren,labels,nmandatory),
   data_id(-1)
{}

//##ModelId=3F98DAE60336
void VisHandlerNode::setDataId (int id)    
{ 
  wstate()[FDataId] = data_id = id;
}

//##ModelId=400E5B6E00CD
void VisHandlerNode::checkInitState (DataRecord &rec)
{
  requiresInitField(rec,FStation1Index);
  requiresInitField(rec,FStation2Index);
  Node::checkInitState(rec);
}

//##ModelId=400E5B6E01FA
void VisHandlerNode::setStateImpl (DataRecord &rec,bool initializing)
{
  if( !initializing )
  {
    protectStateField(rec,FStation1Index);
    protectStateField(rec,FStation2Index);
  }
  Node::setStateImpl(rec,initializing);
}

//##ModelId=3F9FF6970269
void VisHandlerNode::fillCells (Cells &cells,const VisTile &tile,double fq1,double fq2)
{
  // form domain & cells based on stuff in the tile
  LoVec_bool valid( tile.rowflag() != int(VisTile::MissingData) );
  cdebug1(5)<<"valid rows: "<<valid<<endl;
  double maxtime = max(where(valid,tile.time(),0));
  LoVec_double 
    time    ( where(valid,tile.time(),maxtime) ),
    interval( where(valid,tile.interval(),0)   );
  cdebug1(5)<<"time:     "<<time<<endl;
  cdebug1(5)<<"interval: "<<interval<<endl;
  cells.setCells(FREQ,fq1,fq2,tile.nfreq());
  cells.setCells(TIME,time,interval);
  cells.recomputeDomain();
  cdebug1(5)<<"cells: "<<cells;
}


}
