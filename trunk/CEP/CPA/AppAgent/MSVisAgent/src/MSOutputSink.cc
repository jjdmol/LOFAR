//  MSVisOutputAgent.h: agent for writing an AIPS++ MS
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include "MSOutputSink.h"

#include <Common/BlitzToAips.h>

#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/ScalarColumn.h>

namespace MSVisAgent
{
using namespace VisAgent;
using namespace AppEvent;
using namespace AppState;
  
static int dum = aidRegistry_MSVisAgent();

//##ModelId=3E2831C7010D
MSOutputSink::MSOutputSink ()
    : FileSink(),msname_("(none)")
{
  setState(CLOSED);
}


//##ModelId=3E28315F0001
bool MSOutputSink::init (const DataRecord &params)
{
  FailWhen( !FileSink::init(params),"FileSink init failed" );

  params_ = params;
  ms_ = MeasurementSet();
  msname_ = "(none)";
  setState(CLOSED);
  cdebug(1)<<"initialized with "<<params_.sdebug(3)<<endl;
  
  return True;
}

void MSOutputSink::close ()
{
  FileSink::close();
  ms_ = MeasurementSet();
  msname_ = "(none)";
  setState(CLOSED);
  cdebug(1)<<"closed\n";
}

//##ModelId=3EC25BF002D4
void MSOutputSink::postEvent (const HIID &id, const ObjRef::Xfer &data, const HIID &)
{
  try
  {
    int code = VisEventType(id);
    if( code == HEADER )
    {
      DataRecord::Ref ref = data;
      doPutHeader(*ref);
      setState(DATA);
    }
    else if( code == DATA )
    {
      if( state() != DATA )
      {
        cdebug(3)<<"got tile but state is not DATA, ignoring\n";
      }
      else
      {
        VisTile::Ref ref = data;
        // get read-only snapshot
        ref.privatize(DMI::READONLY|DMI::DEEP);
        doPutTile(*ref);
      }
    }
    else if( code == FOOTER )
    {
      if( state() == DATA )
      {
        cdebug(2)<<"got footer event, flushing & closing ms\n";
        setState(CLOSED);
        ms_.unlock();
        ms_.flush();
        ms_ = MeasurementSet();
      }
      else
      {
        cdebug(2)<<"got footer but state is not DATA, ignoring\n";
      }
    }
  }
  catch( std::exception &exc )
  {
    cdebug(1)<<"error handling event ["<<id<<"]: "<<exc.what()<<endl;
  }
  catch( ... )
  {
    cdebug(1)<<"unknown exception handling event ["<<id<<"]\n";
  }
}

//##ModelId=3EC25BF002E4
bool MSOutputSink::isEventBound (const HIID &id)
{
  return id.matches(VisEventMask());
}

bool MSOutputSink::setupDataColumn (Column &col)
{
  // if name is not set, then column is ignored
  if( !col.name.length() )
    return col.valid = False;
  // add column to MS, if it doesn't exist
  if( !ms_.tableDesc().isColumn(col.name) ) 
  {
    cdebug(2)<<"creating new column "<<col.name<<", shape "<<null_cell_.shape()<<endl;
    ArrayColumnDesc<Complex> coldesc(col.name,"added by MSOutputAgent",2);
    ms_.addColumn(coldesc);
  }
  // init the column
  cdebug(2)<<"attaching to column "<<col.name<<endl;
  col.col.attach(ms_,col.name);
  return col.valid = True;
}

//##ModelId=3EC25F74033F
int MSVisAgent::MSOutputSink::refillStream() 
{
  return AppEvent::CLOSED; // no input events
}

//##ModelId=3E28316403E4
void MSOutputSink::doPutHeader (const DataRecord &header)
{
  // open the MS named in the header (incidentally, this will also
  // flush & close any previously named MS)
  msname_ = header[FMSName].as<string>();
  ms_ = MeasurementSet(msname_,TableLock(TableLock::AutoNoReadLocking),Table::Update);
  // get range of channels from header and setup slicer
  int chan0 = header[FChannelStartIndex].as<int>(),
      chan1 = header[FChannelEndIndex].as<int>();
  int ncorr = header[FCorr].size(Tpstring);
  column_slicer_ = Slicer(IPosition(2,0,chan0),IPosition(2,ncorr-1,chan1),
                   Slicer::endIsLast);
  IPosition origshape = LoShape(header[FOriginalDataShape].as_vector<int>());
  null_cell_.resize(origshape);
  null_cell_.set(0);
  // setup parameters from default record
  write_flags_       = params_[FWriteFlags].as<bool>(False);
  flagmask_          = params_[FFlagMask].as<int>(0xFFFFFFFF);
  datacol_.name      = params_[FDataColumn].as<string>("");
  predictcol_.name   = params_[FPredictColumn].as<string>("");
  rescol_.name       = params_[FResidualsColumn].as<string>("");
  // and override them from the header
  if( header[FOutputParams].exists() )
  {
    const DataRecord &hparm = header[FOutputParams].as<DataRecord>();
    write_flags_       = hparm[FWriteFlags].as<bool>(write_flags_);
    flagmask_          = hparm[FFlagMask].as<int>(flagmask_);
    datacol_.name      = hparm[FDataColumn].as<string>(datacol_.name);
    predictcol_.name   = hparm[FPredictColumn].as<string>(predictcol_.name);
    rescol_.name       = hparm[FResidualsColumn].as<string>(rescol_.name);
  }
  // setup columns
  setupDataColumn(datacol_);
  setupDataColumn(predictcol_);
  setupDataColumn(rescol_);
  if( write_flags_ )
  {
    rowFlagCol_.attach(ms_,"FLAG_ROW");
    flagCol_.attach(ms_,"FLAG");
  }

  cdebug(2)<<"got header for MS "<<msname_<<endl;
  cdebug(2)<<"  orig shape: "<<origshape<<endl;
  cdebug(2)<<"  channels: "<<chan0<<"-"<<chan1<<endl;
  cdebug(2)<<"  correlations: "<<ncorr<<endl;
  cdebug(2)<<"  write_flags: "<<write_flags_<<endl;
  cdebug(2)<<"  flagmask: "<<flagmask_<<endl;
  cdebug(2)<<"  colname_data: "<<datacol_.name<<endl;
  cdebug(2)<<"  colname_predict: "<<predictcol_.name<<endl;
  cdebug(2)<<"  colname_residuals: "<<rescol_.name<<endl;
  // set state to indicate success
  tilecount_ = rowcount_ = 0;
}

//##ModelId=3F5F436303AB
void MSOutputSink::putColumn (Column &col,int irow,const LoMat_fcomplex &data)
{
  Matrix<Complex> aips_data = copyBlitzToAips(data);
  cdebug(6)<<"writing "<<col.name<<": "<<aips_data<<endl;
  if( !col.col.isDefined(irow) )
    col.col.put(irow,null_cell_);
  col.col.putSlice(irow,column_slicer_,aips_data);
}    

//##ModelId=3E28316B012D
void MSOutputSink::doPutTile (const VisTile &tile)
{
  tilecount_++;
  cdebug(3)<<"putting tile "<<tile.tileId()<<", "<<tile.nrow()<<" rows"<<endl;
  cdebug(4)<<"  into table rows: "<<tile.seqnr()<<endl;
  cdebug(4)<<"  rowflags are: "<<tile.rowflag()<<endl;
  // iterate over rows of the tile
  int count=0;
  for( VisTile::const_iterator iter = tile.begin(); iter != tile.end(); iter++ )
  {
    int rowflag = iter.rowflag();
    if( rowflag == int(VisTile::MissingData) )
    {
      cdebug(5)<<"  tile row flagged as missing, skipping\n";
      continue;
    }
    count++;
    rowcount_++;
    int irow = iter.seqnr();
    cdebug(5)<<"  writing to table row "<<irow<<endl;
    // write flags if required
    if( write_flags_ )
    {
      rowFlagCol_.put(irow,rowflag&flagmask_ != 0);
      LoMat_bool flags( iter.flags().shape() );
      flags = blitz::cast<bool>(iter.flags() & flagmask_ );
      Matrix<Bool> aflags = refBlitzToAips(flags);
      cdebug(6)<<"writing to FLAG column: "<<aflags<<endl;
      flagCol_.putSlice(irow,column_slicer_,aflags);
    }
    // write data columns
    if( tile.defined(VisTile::DATA) && datacol_.valid )
      putColumn(datacol_,irow,iter.data());
    if( tile.defined(VisTile::PREDICT) && predictcol_.valid )
      putColumn(predictcol_,irow,iter.predict());
    if( tile.defined(VisTile::RESIDUALS) && rescol_.valid )
      putColumn(rescol_,irow,iter.residuals());
  }
  cdebug(4)<<"  wrote "<<count<<"/"<<tile.nrow()<<" rows\n";
}

string MSOutputSink::sdebug (int detail,const string &prefix,const char *name) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"MSOutputSink",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    appendf(out,"MS %s, state %s",msname_.c_str(),stateString().c_str());
  }
  if( detail >= 2 || detail <= -2 )
  {
    appendf(out,"wrote %d tiles/%d rows",tilecount_,rowcount_);
  }
  return out;
}

} // namespace MSVisAgent
