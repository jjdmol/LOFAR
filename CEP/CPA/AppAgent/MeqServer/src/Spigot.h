#ifndef MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78
    
#include <MeqServer/VisHandlerNode.h>
#include <MeqServer/TID-MeqServer.h>

#pragma aid Spigot 
#pragma aid Input Col Next Request Id
#pragma types #Meq::Spigot
    
namespace Meq {

//##ModelId=3F98DAE503C9
class Spigot : public VisHandlerNode
{
  public:
    //##ModelId=3F98DAE6022D
    virtual void init (DataRecord::Ref::Xfer &initrec,Forest * frst);

    //##ModelId=3F98DAE60235
    virtual void setState (const DataRecord &rec);
    
    //##ModelId=3F98DAE6023B
    virtual int deliver (const Request &req,VisTile::Ref::Copy &tileref,
                         VisTile::Format::Ref &outformat);
    
    //##ModelId=3F98DAE6023E
    virtual TypeId objectType() const
    { return TpMeqSpigot; }
    
    //##ModelId=3F9FF6AA016C
    LocalDebugContext;

  protected:
    //##ModelId=3F9FF6AA0300
    virtual int getResultImpl (ResultSet::Ref &resref, const Request &req,bool newreq);
  
  private:
    //##ModelId=3F9FF6AA03D2
    void setStateImpl (const DataRecord &rec);
  
    //##ModelId=3F9FF6AA01A3
    int icolumn;
//,icorr;
    
    //##ModelId=3F9FF6AA0221
    ResultSet::Ref next_res;
    //##ModelId=3F9FF6AA0238
    HIID next_rqid;
};

}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
