#ifndef MEQSERVER_SRC_VISHANDLERNODE_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_VISHANDLERNODE_H_HEADER_INCLUDED_9B1ECA78
    
#include <MEQ/Node.h>
#include <VisCube/VisTile.h>

#pragma aid VisHandlerNode 
#pragma aid Data Id
    
namespace MEQ {
  
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
    
    // returns MEQ::Cells object corresponding to this VisTile
    //##ModelId=3F9FF6970269
    static Cells makeCells (const VisTile &tile);
    
    //##ModelId=3F98DAE602DA
    LocalDebugContext;
    
  private:
    //##ModelId=3F98DAE602F6
    int data_id;
};

//##ModelId=3F98DAE60327
inline int VisHandlerNode::dataId () const   
{ return data_id; }


}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
