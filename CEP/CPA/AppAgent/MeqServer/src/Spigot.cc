#include "Spigot.h"
#include "AID-MeqServer.h"
#include <VisCube/VisVocabulary.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>

namespace Meq {

InitDebugContext(Spigot,"MeqSpigot");
  
const HIID FInputColumn = AidInput|AidCol,
           FCorr        = AidCorr|AidIndex,
           FNext        = AidNext,
           FRequestId   = AidRequest|AidId,
           FResult      = AidResult;
    
//##ModelId=3F98DAE6022D
void Spigot::init (DataRecord::Ref::Xfer &initrec,Forest * frst)
{
  // let the base class initialize itself
  VisHandlerNode::init(initrec,frst);
  // default uses DATA column
  icolumn = VisTile::DATA;
  icorr = 0;
  // setup stuff from state record
  setStateImpl(state());
}

//##ModelId=3F9FF6AA03D2
void Spigot::setStateImpl (const DataRecord &rec)
{
  if( rec[FCorr].exists() )
  {
    wstate()[FCorr] = icorr = rec[FCorr].as<int>();
  }
  if( rec[FInputColumn].exists() )
  {
    string colname = struppercase( rec[FInputColumn].as<string>() );
    const VisTile::NameToIndexMap &colmap = VisTile::getNameToIndexMap();
    VisTile::NameToIndexMap::const_iterator iter = colmap.find(colname);
    FailWhen(iter==colmap.end(),"unknown input column "+colname);
    icolumn = iter->second;
    wstate()[FInputColumn] = colname;
  }
}

//##ModelId=3F98DAE60235
void Spigot::setState (const DataRecord &rec)
{
  VisHandlerNode::setState(rec);
  setStateImpl(rec);
}

//##ModelId=3F98DAE6023B
int Spigot::deliver (const Request &req,VisTile::Ref::Copy &tileref,
                     VisTile::Format::Ref &)
{
  const VisTile &tile = *tileref;
  const HIID &rqid = req.getId();
  cdebug(3)<<"deliver: tile "<<tile.tileId()<<", rqid "<<rqid<<endl;
  // already waiting for such a request? Do nothing for now
  if( currentRequestId() == rqid )
  {
    cdebug(2)<<"deliver: already at rqid but notify not implemented, doing nothing"<<endl;
    Throw("Spigot: parent notify mechanism not yet implemented");
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
    next_res <<= new Result;
    // get array 
    if( coltype == Tpdouble )
    {
      if( colshape.size() == 3 )
      {
        LoMat_double mat = 
            LoCube_double(static_cast<double*>(coldata),colshape,blitz::neverDeleteData)
            (icorr,LoRange::all(),LoRange::all());
        next_res().setReal(colshape[1],colshape[2]) = mat;
      }
      else if( colshape.size() == 2 )
      {
        LoMat_double mat(static_cast<double*>(coldata),colshape,blitz::neverDeleteData);
        next_res().setReal(colshape[0],colshape[1]) = mat;
      }
      else
        Throw("bad input column shape");
    }
    else if( coltype == Tpfcomplex )
    {
      if( colshape.size() == 3 )
      {
        LoMat_fcomplex mat = 
            LoCube_fcomplex(static_cast<fcomplex*>(coldata),colshape,blitz::neverDeleteData)
            (icorr,LoRange::all(),LoRange::all());
        next_res().setComplex(colshape[1],colshape[2]) = 
            blitz::cast<dcomplex>(mat);
      }
      else if( colshape.size() == 2 )
      {
        LoMat_fcomplex mat(static_cast<fcomplex*>(coldata),colshape,blitz::neverDeleteData);
        next_res().setComplex(colshape[0],colshape[1]) = 
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
int Spigot::getResultImpl (Result::Ref &resref,const Request &req,bool newreq)
{
  // have we got a cached result?
  if( next_res.valid() )
  {
    // return fail if unable to satisfy this request
    if( req.getId() != next_rqid )
      return RES_FAIL;
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
