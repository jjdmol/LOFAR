#ifndef MEQSERVER_SRC_SINK_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_SINK_H_HEADER_INCLUDED_9B1ECA78
    
#include <MeqServer/VisHandlerNode.h>
#include <MeqServer/TID-MeqServer.h>

#pragma aid Sink 
#pragma aid Output Col Corr Index
#pragma types #MEQ::Sink
    
namespace MEQ {

//##ModelId=3F98DAE503B5
class Sink : public VisHandlerNode
{
  public:
    //##ModelId=3F98DAE60213
    virtual void init (DataRecord::Ref::Xfer &initrec, Forest* frst);

    //##ModelId=3F98DAE6021B
    virtual void setState (const DataRecord &rec);
    
    //##ModelId=3F98DAE6021E
    virtual int deliver (const Request &req,VisTile::Ref::Copy &tileref,
                         VisTile::Format::Ref &outformat);
    
    //##ModelId=3F98DAE60222
    virtual TypeId objectType() const
    { return TpMEQSink; }
    
  protected:
    //##ModelId=3F98DAE60217
    virtual int getResultImpl (Result::Ref &resref, const Request &req,bool newreq);

  private:
//    //##ModelId=3F98DD7400A9
//    void assignOutputColumn (int ichild,string colname);
  
    //##ModelId=3F9918390169
    void setStateImpl (const DataRecord &rec);
    
    
    template<class T,class U>
    void fillTileColumn (T *coldata,const LoShape &colshape,
                         const blitz::Array<U,2> &arr,int icorr);
      
    //##ModelId=3F98DAE60211
    vector<int> output_cols;
    //##ModelId=3F9918390123
    vector<int> child_icorrs;
};

}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
