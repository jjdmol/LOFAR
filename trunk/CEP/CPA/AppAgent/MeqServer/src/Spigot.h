#ifndef MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78
    
#include <MeqServer/VisHandlerNode.h>
#include <MeqServer/TID-MeqServer.h>

#pragma aid Spigot 
#pragma aid Input Col Next Request Id
#pragma types #Meq::Spigot
    
// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqSpigot
//  A MeqSpigot is attached to a VisAgent data source, and represents
//  one interferometer. For every matching VisTile at the input of the 
//  source, it caches the visibility data. If a matching request is then
//  received, it returns that data as the result set (with one plane per
//  correlation.) A MeqSpigot usually works in concert with a MeqSink,
//  in that a sink is placed at the base of the tree, and generates 
//  results matching the input data. Note that for this to work properly,
//  a spigot must be created BEFORE its corresponding sink.
//  A MeqSpigot can have no children.
//field: station_1_index 0
//  Index (1-based) of first station comprising the interferometer
//field: station_2_index 0
//  Index (1-based) of second station comprising the interferometer
//field: input_col 'DATA'
//  tile column to get result from: DATA, PREDICT or RESIDUALS. 
//defrec end

namespace Meq {

//##ModelId=3F98DAE503C9
class Spigot : public VisHandlerNode
{
  public:
    //##ModelId=3F98DAE6022D
    virtual void init (DataRecord::Ref::Xfer &initrec,Forest * frst);

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
    virtual int getResult (Result::Ref &resref, 
                           const std::vector<Result::Ref> &childres,
                           const Request &req,bool newreq);
  
    //##ModelId=400E5B6D00BF
    virtual void checkInitState (DataRecord &rec);
    
    //##ModelId=3F9FF6AA03D2
    virtual void setStateImpl (DataRecord &rec,bool initializing);

  private:
    //##ModelId=3F9FF6AA01A3
    int icolumn;
//,icorr;
    
    //##ModelId=3F9FF6AA0221
    Result::Ref next_res;
    //##ModelId=3F9FF6AA0238
    HIID next_rqid;
};

}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
