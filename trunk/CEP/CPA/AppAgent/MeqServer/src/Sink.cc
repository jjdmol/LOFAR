#include "Sink.h"
#include <DMI/DataField.h>
#include <VisCube/VisVocabulary.h>
#include <MEQ/Request.h>
#include <MEQ/VellSet.h>
#include <MEQ/MeqVocabulary.h>

namespace Meq {

//##ModelId=3F98DAE60213
void Sink::init (DataRecord::Ref::Xfer &initrec,Forest* frst)
{
  flag_mask = row_flag_mask = -1;
  // assign output column -- default is -1
  output_col = -1;
  // output correlations -- if clear, then plane i = corr i
  output_icorrs.clear();
  // let the base class initialize itself
  VisHandlerNode::init(initrec,frst);
  FailWhen(numChildren()!=1,"sink must have exactly one child node");
}

//##ModelId=400E5B6D0048
int Sink::mapOutputCorr (int iplane)
{
  if( output_icorrs.empty() )
    return iplane;
  if( iplane<0 || iplane >= int(output_icorrs.size()) )
    return -1;
  return output_icorrs[iplane]; 
}

//##ModelId=400E5B6C03D8
void Sink::checkInitState (DataRecord &rec)
{
  VisHandlerNode::checkInitState(rec);
  defaultInitField(rec,FOutputColumn,"");
  // FCorr is left empty if missing
}

//##ModelId=3F9918390169
void Sink::setStateImpl (DataRecord &rec,bool initializing)
{
  VisHandlerNode::setStateImpl(rec,initializing);
  // check if output column is specified
  if( rec[FOutputColumn].exists() )
  {
    string colname = struppercase(rec[FOutputColumn].as<string>());
    if( colname.length() )
    {
      const VisTile::NameToIndexMap &colmap = VisTile::getNameToIndexMap();
      VisTile::NameToIndexMap::const_iterator iter = colmap.find(colname);
      if( iter == colmap.end() ) {
        NodeThrow(FailWithoutCleanup,"unknown output column "+colname);
      }
      output_col = iter->second;
    }
    else
      output_col = -1;
  }
  // check if output correlation map is specified
  if( rec[FCorr].exists() )
    output_icorrs = rec[FCorr].as_vector<int>();
  // get flag masks
  getStateField(flag_mask,rec,FFlagMask);
  getStateField(row_flag_mask,rec,FRowFlagMask);
}

//##ModelId=3F9509770277
int Sink::getResult (Result::Ref &ref, 
                     const std::vector<Result::Ref> &childres,
                     const Request &,bool)
{
  ref.copy(childres[0],DMI::PRESERVE_RW);
  return 0;
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
  cdebug(3)<<"deliver: processing tile "<<tileref->tileId()<<" of "
            <<tileref->ntime()<<" timeslots"<<endl;
  // get results from all child nodes 
  Result::Ref resref;
  cdebug(5)<<"calling execute() on child "<<endl;
  int resflag = getChild(0).execute(resref,req);
  FailWhen(resflag&RES_WAIT,"Meq::Sink can't cope with a WAIT result code yet");
  if( resflag == RES_FAIL )
  {
    cdebug(3)<<"child result is FAIL, ignoring"<<endl;
    return RES_FAIL;
  }
  int nvs = resref->numVellSets();
  cdebug(3)<<"child returns "<<nvs<<" vellsets, resflag "<<resflag<<endl;
  if( output_col<0 )
  {
    cdebug(3)<<"output disabled, skipping"<<endl;
    return resflag;
  }
  // store resulting Vells into the tile
  // loop over vellsets and get a tf-plane from each
  VisTile *ptile = 0;  // we will privatize the tile for writing as needed
  const VisTile::Format *pformat = 0;
  void *coldata; 
  TypeId coltype;
  LoShape colshape; 
  int ncorr = tileref->ncorr();
  for( int ivs = 0; ivs < nvs; ivs++ )
  {
    int icorr = mapOutputCorr(ivs);
    if( icorr<0 )
    {
      cdebug(3)<<"vellset "<<ivs<<" output disabled, skipping"<<endl;
    }
    else if( icorr >= ncorr )
    {
      cdebug(3)<<"child "<<ivs<<" correlation not available, skipping"<<endl;
    }
    else // OK, write it
    {
      // make tile writable if so required
      // this is done the first time we actually try to write to it
      if( !ptile )
      {
        if( !tileref.isWritable() )
          tileref.privatize(DMI::WRITE|DMI::DEEP);
        ptile = tileref.dewr_p();
        pformat = &(ptile->format());
        // add output column to tile as needed
        if( !pformat->defined(output_col) )
        {
          // if column is not present in default output format, add it
          if( !outformat.valid() )
            outformat.copy(tileref->formatRef(),DMI::PRESERVE_RW);
          if( !outformat->defined(output_col) )
          {
            if( output_col == VisTile::PREDICT || output_col == VisTile::RESIDUALS )
            {
              outformat.privatize(DMI::WRITE|DMI::DEEP);
              outformat().add(output_col,
                  outformat->type(VisTile::DATA),outformat->shape(VisTile::DATA));
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
        coldata  = ptile->wcolumn(output_col);
        coltype  = pformat->type(output_col);
        colshape = pformat->shape(output_col);
        colshape.push_back(ptile->nrow()); // add third dimension to column shape
      }
      // get the values out, and copy them to tile column
      const Vells &vells = resref->vellSet(ivs).getValue();
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
      resflag |= RES_UPDATED;
    }
  }
  return resflag;
}




}
