#ifndef MEQSERVER_SRC_SINK_H_HEADER_INCLUDED_9B1ECA78
#define MEQSERVER_SRC_SINK_H_HEADER_INCLUDED_9B1ECA78
    
#include <MeqServer/VisHandlerNode.h>
#include <MeqServer/TID-MeqServer.h>

#pragma aid Sink 
#pragma aid Output Col Corr Index
#pragma types #Meq::Sink

// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqSink
//  A MeqSink is attached to a VisAgent data source. A MeqSink represents
//  one interferometer. For every matching VisTile at the input of the 
//  source, it generates a MeqRequest corresponding to the domain/cells 
//  of the tile, and optionally stores the result back in the tile.
//  A MeqSink must have exactly one child. The child may return a 
//  multi-plane result.
//field: station_1_index 0
//  Index (1-based) of first station composing the interferometer
//field: station_2_index 0
//  Index (1-based) of second station composing the interferometer
//field: output_col ''
//  tile column to write results to: DATA, PREDICT or RESIDUALS.
//  If empty, then no output is generated.
//field: corr_index []
//  Defines mappings from result planes to correlations. If empty, then
//  a default one-to-one mapping is used. Otherwise, should contain one
//  correlation index (1-based) per each result plane.
//defrec end
    
namespace Meq {

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
    { return TpMeqSink; }
    
  protected:
    //##ModelId=3F98DAE60217
    virtual int getResultImpl (ResultSet::Ref &resref, const Request &req,bool newreq);

  private:
//    //##ModelId=3F98DD7400A9
//    void assignOutputColumn (int ichild,string colname);
  
    //##ModelId=3F9918390169
    void setStateImpl (const DataRecord &rec);
    
    // maps plane to output correlation
    int mapOutputCorr (int iplane);
    
    template<class T,class U>
    void fillTileColumn (T *coldata,const LoShape &colshape,
                         const blitz::Array<U,2> &arr,int icorr);
      
    //##ModelId=3F98DAE60211
    int output_col;
    //##ModelId=3F9918390123
    vector<int> output_icorrs;
    
};

}

#endif /* MEQSERVER_SRC_SPIGOT_H_HEADER_INCLUDED_9B1ECA78 */
