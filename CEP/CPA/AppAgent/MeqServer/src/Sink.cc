#include "Sink.h"
#include "AID-MeqServer.h"
#include <DMI/DataField.h>
#include <VisCube/VisVocabulary.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>

namespace MEQ {

const HIID FOutputColumn = AidOutput|AidCol,
           FCorr         = AidCorr|AidIndex;

//##ModelId=3F98DAE60213
void Sink::init (DataRecord::Ref::Xfer &initrec,Forest* frst)
{
  // let the base class initialize itself
  VisHandlerNode::init(initrec,frst);
  // init child correlation indices
  // default is 0 to child 0, 1 to child 1, etc.
  FailWhen(!numChildren(),"sink must have child nodes");
  child_icorrs.resize(numChildren());
  for( int i=0; i<numChildren(); i++ )
    child_icorrs[i] = i;
  // assign output columns -- default is -1
  output_cols.resize(numChildren());
  output_cols.assign(numChildren(),-1); // default disables output of VisTiles
  // set stuff from record
  setStateImpl(state());
  // setup default if not already specified
  if( !wstate()[FCorr].exists() )
    wstate()[FCorr] = child_icorrs;
}

//##ModelId=3F9918390169
void Sink::setStateImpl (const DataRecord &rec)
{
  // check if output columns are specified
  if( rec[FOutputColumn].exists() )
  {
    vector<string> colnames = rec[FOutputColumn];
    // must specify a single name for all, or exactly one name per child
    FailWhen( colnames.size() != 1 && int(colnames.size()) != numChildren(),
        "length of "+FOutputColumn.toString()+"field must match # of children");
    for( uint i=0; i<colnames.size(); i++ )
    {
      if( colnames[i].length() )
      {
        const VisTile::NameToIndexMap &colmap = VisTile::getNameToIndexMap();
        VisTile::NameToIndexMap::const_iterator iter = 
            colmap.find(colnames[i] = struppercase(colnames[i]));
        FailWhen(iter==colmap.end(),"unknown output column "+colnames[i]);
        output_cols[i] = iter->second;
      }
      else
        output_cols[i] = -1;
    }
    wstate()[FOutputColumn].replace() = colnames;
  }
  // check if child correlations are (re)specified
  if( rec[FCorr].exists() )
  {
    vector<int> icorr = rec[FCorr];
    FailWhen( int(icorr.size())!=numChildren(),"length of "+FCorr.toString()+
              "field must match # of children");
    child_icorrs = icorr;
    wstate()[FCorr].replace() = icorr;
  }
}

//##ModelId=3F98DAE6021B
void Sink::setState (const DataRecord &rec)
{
  VisHandlerNode::setState(rec);
  setStateImpl(rec);
}

//##ModelId=3F9509770277
int Sink::getResultImpl (Result::Ref &resref,const Request &req,bool newreq)
{
  int flag;
  // get results for all children, dump updated results to output
  for( int i=0; i<numChildren(); i++ )
  {
    flag = getChild(i).getResult(resref,req);
//** commented out for now, sinks produce no result
//    if( !(flag&RES_WAIT) && (newreq || flag&RES_UPDATED) )
//      outputResult(i,resref,flag);
  }
  // ends up returning result of last child
  return flag;
}

template<class T,class U>
void Sink::fillTileColumn (T *coldata,const LoShape &colshape,
                           const blitz::Array<U,2> &arr,int icorr)
{
  const LoShape arrshape = arr.shape();
  FailWhen(arrshape[1]!=colshape.back(),"shape of child result does not match output column");
  // option 1 is writing to a 2D column with same shape
  if( colshape.size() == 2 && colshape[0] == arrshape[0] )
  {
    blitz::Array<T,2> colarr(coldata,colshape,blitz::neverDeleteData);
    colarr = blitz::cast<T>(arr);
  }
  // option 2 is writing to a cube column using the current correlation
  else if( colshape.size() == 3 && colshape[1] == arrshape[0] )
  {
    blitz::Array<T,3> colarr(coldata,colshape,blitz::neverDeleteData);
    colarr(icorr,LoRange::all(),LoRange::all()) = blitz::cast<T>(arr);
  }
  else
  {
    Throw("shape of child result does not match output column")
  }
}
  
//##ModelId=3F98DAE6021E
int Sink::deliver (const Request &req,VisTile::Ref::Copy &tileref,
                   VisTile::Format::Ref &outformat)
{
  const VisTile &tile = *tileref;
  const VisTile::Format * pformat = &(tile.format());
  
  cdebug(3)<<"deliver: processing tile "<<tile.tileId()<<" of "<<tile.ntime()<<" timeslots"<<endl;
  
  // get results from all child nodes 
  vector<Result::Ref> resref(numChildren());
  vector<int> flags(numChildren());
  for( int ichild=0; ichild<numChildren(); ichild++ )
  {
    cdebug(5)<<"calling getResult() on child "<<ichild<<endl;
    flags[ichild] = getChild(ichild).getResult(resref[ichild],req);
  }
  int result_flag = 0;
  // store resulting Vells into the tile
  // loop over children and get a tf-plane from each
  VisTile *ptile = 0;  // we will privatize the tile for writing as needed
  for( int ichild=0; ichild<numChildren(); ichild++ )
  {
    FailWhen(flags[ichild]&RES_WAIT,"MEQ::Sink can't cope with a WAIT result code yet");
    int icol = output_cols[ichild], icorr = child_icorrs[ichild];
    if( icol >= 0 && icorr >= 0 )
    {
      // make tile writable if so required
      if( !ptile )
      {
        if( !tileref.isWritable() )
          tileref.privatize(DMI::WRITE|DMI::DEEP);
        ptile = tileref.dewr_p();
      }
      // add output column to tile as needed
      if( !pformat->defined(icol) )
      {
        // if column is not present in default output format, add it
        if( !outformat.valid() )
          outformat.copy(tile.formatRef(),DMI::PRESERVE_RW);
        if( !outformat->defined(icol) )
        {
          if( icol == VisTile::PREDICT || icol == VisTile::RESIDUALS )
          {
            outformat.privatize(DMI::WRITE|DMI::DEEP);
            outformat().add(icol,outformat->type(VisTile::DATA),
                                 outformat->shape(VisTile::DATA));
          }
          else
          {
            Throw("output column format is not known");
          }
        }
        cdebug(3)<<"adding output column to tile"<<endl;
        ptile->changeFormat(outformat);
        pformat = outformat.deref_p();
      }
      // fill column
      void *coldata = ptile->wcolumn(icol);
      TypeId coltype = pformat->type(icol);
      LoShape colshape = pformat->shape(icol);
      colshape.push_back(tile.nrow()); // add third dimension to column shape
      if( flags[ichild] != RES_FAIL )
      {
        cdebug(3)<<"child "<<ichild<<" result is "<<flags[ichild]<<endl;
        // get the values out, and copy them to tile column
        const Vells &vells = resref[ichild]->getValue();
        if( vells.isReal() ) // real values
        {
          FailWhen(coltype!=Tpdouble,"type mismatch: double Vells, "+coltype.toString()+" column");
          fillTileColumn(static_cast<double*>(coldata),colshape,vells.getRealArray(),icorr);
        }
        else  // complex values
        {
          FailWhen(coltype!=Tpfcomplex,"type mismatch: complex Vells, "+coltype.toString()+" column");
          fillTileColumn(static_cast<fcomplex*>(coldata),colshape,vells.getComplexArray(),icorr);
        }
        result_flag |= RES_UPDATED;
      }
      else
      {
        cdebug(3)<<"child "<<ichild<<" result is FAIL, skipping"<<endl;
      }
    }
    else
    {
      cdebug(3)<<"child "<<ichild<<" output disabled, skipping"<<endl;
    }
    resref[ichild].detach();
  }
  return result_flag;
}




}
