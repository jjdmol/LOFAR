#include "VisHandlerNode.h"
#include "AID-MeqServer.h"
#include <MEQ/Cells.h>

namespace MEQ {

InitDebugContext(VisHandlerNode,"MeqVisHandler");
  
const HIID FDataId      = AidData|AidId;

//##ModelId=3F98DAE60319
VisHandlerNode::VisHandlerNode ()
    : data_id(-1)
{}

//##ModelId=3F98DAE60336
void VisHandlerNode::setDataId (int id)    
{ 
  wstate()[FDataId] = data_id = id;
}

//##ModelId=3F9FF6970269
Cells VisHandlerNode::makeCells (const VisTile &tile)
{
  // form domain & cells based on stuff in the tile
  LoVec_bool valid( tile.rowflag() != int(VisTile::MissingData) );
  cdebug1(5)<<"valid rows: "<<valid<<endl;
  double maxtime = max(where(valid,tile.time(),0));
  LoVec_double 
    int_2( where(valid,tile.interval()*0.5,1) ),
    start_time( where(valid,tile.time(),maxtime)-int_2 ),
      end_time( where(valid,tile.time(),maxtime)+int_2 );
  cdebug1(5)<<"interval/2: "<<int_2<<endl;
  cdebug1(5)<<"start_time: "<<start_time<<endl;
  cdebug1(5)<<"  end_time: "<<end_time<<endl;
  // note: frequency domain is 0:1 for now
  return Cells(Domain(0,1,min(start_time),max(end_time)),
               tile.nfreq(),start_time,end_time);
}


}
