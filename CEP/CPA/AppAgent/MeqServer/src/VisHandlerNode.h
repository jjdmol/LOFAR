#ifndef MEQSERVER_SRC_VISHANDLERNODE_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_VISHANDLERNODE_H_HEADER_INCLUDED_9B1ECA78
    
#include <MEQ/Node.h>
#include <VisCube/VisTile.h>
#include <MeqServer/AID-MeqServer.h>

#pragma aid VisHandlerNode 
#pragma aid Station Index Data Id Num Antenna Tile Format Input 
#pragma aid Output Col Corr Next
    
namespace Meq {
  
const HIID  FStation1      = AidStation|1|AidIndex,
            FStation2      = AidStation|2|AidIndex,
            FNumStations   = AidNum|AidAntenna,
            FTileFormat    = AidTile|AidFormat,
            FDataId        = AidData|AidId,

            FOutputColumn = AidOutput|AidCol,
            FInputColumn  = AidInput|AidCol,
            FCorr         = AidCorr|AidIndex,
            FNext         = AidNext;
    

class Cells;

//##ModelId=3F98DAE60009
class VisHandlerNode : public Node
{
  public:
    //##ModelId=3F98DAE60319
    VisHandlerNode ();
      
    //##ModelId=3F98DAE60327
    int dataId () const;
    
    //##ModelId=3F98DAE60336
    void setDataId (int id);
    
    //##ModelId=3F98DAE60344
    //##Documentation
    //## Delivers a VisTile to the node.
    //## req is the request generated from this VisTile
    //## tileref is a ref to the tile
    //## Returns result state (see Node::RES_xxx constants), which can be
    //## Node::RES_FAIL for failure, or a combination of the following
    //## bit flags:
    //##    RES_WAIT    result not yet available, must wait
    //##    RES_UPDATED result available and tile was updated
    virtual int deliver (const Request &req,VisTile::Ref::Copy &tileref,
                         VisTile::Format::Ref &outformat) =0;
    
    // returns Meq::Cells object corresponding to this VisTile
    //##ModelId=3F9FF6970269
    static Cells * makeCells (const VisTile &tile,double minfreq,double maxfreq);
    
    //##ModelId=3F98DAE602DA
    LocalDebugContext;
    
  protected:
    virtual void checkInitState (DataRecord &rec);
    
    virtual void setStateImpl (DataRecord &rec,bool initializing);
    
  private:
    //##ModelId=3F98DAE602F6
    int data_id;
};

//##ModelId=3F98DAE60327
inline int VisHandlerNode::dataId () const   
{ return data_id; }


}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
