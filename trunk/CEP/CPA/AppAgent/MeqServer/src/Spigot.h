#ifndef MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78
    
#include <MeqServer/VisHandlerNode.h>
#include <MeqServer/TID-MeqServer.h>

#pragma aid Spigot 
#pragma aid Input Col Next Request Id
#pragma types #MEQ::Spigot
    
namespace MEQ {

//##ModelId=3F98DAE503C9
class Spigot : public VisHandlerNode
{
  public:
    virtual void init (DataRecord::Ref::Xfer &initrec,Forest * frst);

    virtual void setState (const DataRecord &rec);
    
    virtual int deliver (const Request &req,VisTile::Ref::Copy &tileref);
    
    virtual TypeId objectType() const
    { return TpMEQSpigot; }
    
    LocalDebugContext;

  protected:
    virtual int getResultImpl (Result::Ref &resref, const Request &req,bool newreq);
  
  private:
    void setStateImpl (const DataRecord &rec);
  
    int icolumn,icorr;
    
    Result::Ref next_res;
    HIID next_rqid;
};

}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
