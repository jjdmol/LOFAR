#include "Spigot.h"
#include "AID-MeqServer.h"
#include <VisCube/VisVocabulary.h>
#include <MEQ/Request.h>
#include <MEQ/VellSet.h>
#include <MEQ/MeqVocabulary.h>

namespace Meq {

InitDebugContext(Spigot,"MeqSpigot");
  
//##ModelId=3F98DAE6022D
void Spigot::init (DataRecord::Ref::Xfer &initrec,Forest * frst)
{
  // default uses DATA column
  icolumn = VisTile::DATA;
  VisHandlerNode::init(initrec,frst);
}

void Spigot::checkInitState (DataRecord &rec)
{
  VisHandlerNode::checkInitState(rec);
  defaultInitField(rec,FInputColumn,"DATA");
}

void Spigot::setStateImpl (DataRecord &rec,bool initializing)
{
  VisHandlerNode::setStateImpl(rec,initializing);
  if( rec[FInputColumn].exists() )
  {
    string colname = struppercase( rec[FInputColumn].as<string>() );
    const VisTile::NameToIndexMap &colmap = VisTile::getNameToIndexMap();
    VisTile::NameToIndexMap::const_iterator iter = colmap.find(colname);
    if( iter == colmap.end() ) {
      NodeThrow(FailWithoutCleanup,"unknown input column "+colname);
    }
    icolumn = iter->second;
    wstate()[FInputColumn] = colname;
  }
}

//##ModelId=3F98DAE6023B
int Spigot::deliver (const Request &req,VisTile::Ref::Copy &tileref,
                     VisTile::Format::Ref &)
{
  const VisTile &tile = *tileref;
  const HIID &rqid = req.id();
  cdebug(3)<<"deliver: tile "<<tile.tileId()<<", rqid "<<rqid<<endl;
  // already waiting for such a request? Do nothing for now
  if( currentRequestId() == rqid )
  {
    cdebug(2)<<"deliver: already at rqid but notify not implemented, doing nothing"<<endl;
    Throw("Spigot: deliver() called after getResult() for the same request ID. "
          "This is not something we can handle w/o a parent notify mechanism, "
          "which is not yet implemented. Either something is wrong with your tree, "
          "or you're not genrating unique request IDs.");
  }
  else
  {
    const VisTile::Format &tileformat = tile.format();
    TypeId coltype = tileformat.type(icolumn);
    LoShape colshape = tileformat.shape(icolumn);
    colshape.push_back(tile.nrow());
    // casting away const because blitz constructors below only take non-const
    // pointers
    void *coldata = const_cast<void*>(tile.column(icolumn));
    int nplanes = colshape.size() == 3 ? colshape[0] : 1;
    next_res <<= new Result(nplanes);
    // get array 
    if( coltype == Tpdouble )
    {
      if( colshape.size() == 3 )
      {
        LoCube_double cube(static_cast<double*>(coldata),colshape,blitz::neverDeleteData);
        for( int i=0; i<nplanes; i++ )
          next_res().setNewVellSet(i).setReal(colshape[1],colshape[2]) = 
            cube(i,LoRange::all(),LoRange::all());
      }
      else if( colshape.size() == 2 )
      {
        LoMat_double mat(static_cast<double*>(coldata),colshape,blitz::neverDeleteData);
        next_res().setNewVellSet(0).setReal(colshape[0],colshape[1]) = mat;
      }
      else
        Throw("bad input column shape");
    }
    else if( coltype == Tpfcomplex )
    {
      if( colshape.size() == 3 )
      {
        LoCube_fcomplex cube(static_cast<fcomplex*>(coldata),colshape,blitz::neverDeleteData);
        for( int i=0; i<nplanes; i++ )
          next_res().setNewVellSet(i).setComplex(colshape[1],colshape[2]) = 
              blitz::cast<dcomplex>(cube(i,LoRange::all(),LoRange::all()));
      }
      else if( colshape.size() == 2 )
      {
        LoMat_fcomplex mat(static_cast<fcomplex*>(coldata),colshape,blitz::neverDeleteData);
        next_res().setNewVellSet(0).setComplex(colshape[0],colshape[1]) = 
            blitz::cast<dcomplex>(mat);
      }
      else
        Throw("bad input column shape");
    }
    else
    {
      Throw("invalid column type: "+coltype.toString());
    }
    next_res().setCells(req.cells());
    // store the next req/result in the state record
    DataRecord &rec = wstate()[FNext].replace() <<= new DataRecord;
    rec[FRequestId] = next_rqid = rqid;
    rec[FResult] <<= static_cast<const DataRecord*>(next_res.deref_p());
  }
  return 0;
}

//##ModelId=3F9FF6AA0300
int Spigot::getResult (Result::Ref &resref,const Request &req,bool newreq)
{
  // have we got a cached result?
  if( next_res.valid() )
  {
    // return fail if unable to satisfy this request
    if( req.id() != next_rqid )
    {
      resref <<= new Result(1,req);
      VellSet &vs = resref().setNewVellSet(0);
      MakeFailVellSet(vs,"spigot: got request id "+
                        req.id().toString()+", expecting "+next_rqid.toString());
      return RES_FAIL;
    }
    // return result and clear cache
    resref = next_res;
    next_rqid.clear();
    wstate()[FNext].remove();
    return RES_UPDATED;
  }
  else // no result at all, return WAIT
  {
    return RES_WAIT;
  }
}

}
