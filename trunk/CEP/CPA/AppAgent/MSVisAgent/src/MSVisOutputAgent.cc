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

#include "MSVisOutputAgent.h"

#include <Common/BlitzToAips.h>

#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/ScalarColumn.h>

using namespace MSVisAgentVocabulary;

static int dum = aidRegistry_MSVisAgent();

//##ModelId=3E2831C7010D
MSVisOutputAgent::MSVisOutputAgent ()
{
  setFileState(FILECLOSED);
  msname_ = "(none)";
}

//##ModelId=3E28315F0001
bool MSVisOutputAgent::init (const DataRecord::Ref &data)
{
  ms_ = MeasurementSet();
  msname_ = "(none)";
  setFileState(FILECLOSED);
  // get default parameters from init record, if any
  if( (*data)[FMSVisOutputAgentParams].exists() )
    params_ = (*data)[FMSVisOutputAgentParams].as_DataRecord();
  else
    params_ = DataRecord();
  cdebug(1)<<"initialized with "<<params_.sdebug(3)<<endl;
  return True;
}

void MSVisOutputAgent::close ()
{
  ms_ = MeasurementSet();
  msname_ = "(none)";
  setFileState(FILECLOSED);
  cdebug(1)<<"closed\n";
}

bool MSVisOutputAgent::setupDataColumn (Column &col)
{
  // if name is not set, then column is ignored
  if( !col.name.length() )
    return col.valid = False;
  // add column to MS, if it doesn't exist
  if( !ms_.tableDesc().isColumn(col.name) ) 
  {
    ArrayColumnDesc<Complex> coldesc(col.name);
    ms_.addColumn(coldesc);
  }
  // init the column
  col.col.attach(ms_,col.name);
  return col.valid = True;
}


//##ModelId=3E28316403E4
int MSVisOutputAgent::putHeader (DataRecord::Ref &hdr, bool)
{
  try
  {
    const DataRecord &header = *hdr;
    // open the MS named in the header (incidentally, this will also
    // flush & close any previously named MS)
    msname_ = header[FMSName].as_string();
    ms_ = MeasurementSet(msname_,Table::Update);
    // get range of channels from header and setup slicer
    int chan0 = header[FChannelStartIndex].as_int(),
        chan1 = header[FChannelEndIndex].as_int();
    int ncorr = header[FCorr].size(Tpint);
    column_slicer_ = Slicer(IPosition(2,0,chan0),IPosition(2,ncorr-1,chan1),
                     Slicer::endIsLast);
    // setup parameters from default record
    write_flags_       = params_[FWriteFlags].as_bool(False);
    flagmask_          = params_[FFlagMask].as_int(0xFFFFFFFF);
    datacol_.name      = params_[FDataColumn].as_string("");
    predictcol_.name   = params_[FPredictColumn].as_string("");
    rescol_.name       = params_[FResidualsColumn].as_string("");
    // and override them from the header
    if( header[FMSVisOutputAgentParams].exists() )
    {
      const DataRecord &hparm = header[FMSVisOutputAgentParams].as_DataRecord();
      write_flags_       = hparm[FWriteFlags].as_bool(write_flags_);
      flagmask_          = hparm[FFlagMask].as_int(flagmask_);
      datacol_.name      = hparm[FDataColumn].as_string(datacol_.name);
      predictcol_.name   = hparm[FPredictColumn].as_string(predictcol_.name);
      rescol_.name       = hparm[FResidualsColumn].as_string(rescol_.name);
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
    cdebug(4)<<"  channels: "<<chan0<<"-"<<chan1<<endl;
    cdebug(4)<<"  correlations: "<<ncorr<<endl;
    cdebug(4)<<"  write_flags: "<<write_flags_<<endl;
    cdebug(4)<<"  flagmask: "<<flagmask_<<endl;
    cdebug(4)<<"  colname_data: "<<datacol_.name<<endl;
    cdebug(4)<<"  colname_predict: "<<predictcol_.name<<endl;
    cdebug(4)<<"  colname_residuals: "<<rescol_.name<<endl;
    // set state to indicate success
    tilecount_ = rowcount_ = 0;
    setFileState(HEADER);
    return SUCCESS;
  }
  catch( std::exception &exc )
  {
    setErrorState(exc.what());
  }
  catch( AipsError &err )
  {
    setErrorState(err.getMesg());
  }
  return ERROR;
}

//##ModelId=3E28316B012D
int MSVisOutputAgent::putNextTile (VisTile::Ref &tileref, bool)
{
  if( fileState() != DATA && fileState() != HEADER  )
    return AppAgent::ERROR;
  
  try
  {
    const VisTile &tile = *tileref;
    tilecount_++;
    cdebug(3)<<"putting tile "<<tile.vdsId()<<", "<<tile.nrow()<<" rows"<<endl;
    cdebug(4)<<"  into table rows: "<<tile.seqnr()<<endl;
    cdebug(4)<<"  rowflags are: "<<tile.rowflag()<<endl;
    // iterate over rows of the tile
    int count=0;
    for( VisTile::const_iterator iter = tile.begin(); iter != tile.end(); iter++ )
    {
      int rowflag = iter.rowflag();
      if( rowflag == VisTile::MissingData )
      {
        cdebug(5)<<"  tile row flagged as missing, skipping\n";
        continue;
      }
      count++;
      rowcount_++;
      int irow = iter.seqnr();
      cdebug(5)<<"  writing to table row "<<irow<<endl;
      // write flags?
      if( write_flags_ )
      {
        rowFlagCol_.put(irow,rowflag&flagmask_ != 0);
        
        LoMat_bool flags( iter.flags().shape() );
        flags = blitz::cast<bool>(iter.flags() & flagmask_ );
        flagCol_.putSlice(irow,column_slicer_,refBlitzToAips(flags));
      }
      // write data columns
      if( tile.defined(VisTile::DATA) && datacol_.valid )
        datacol_.col.putSlice(irow,column_slicer_,copyBlitzToAips(iter.data()));
      if( tile.defined(VisTile::PREDICT) && predictcol_.valid )
        predictcol_.col.putSlice(irow,column_slicer_,copyBlitzToAips(iter.predict()));
      if( tile.defined(VisTile::RESIDUALS) && rescol_.valid )
        rescol_.col.putSlice(irow,column_slicer_,copyBlitzToAips(iter.residuals()));
    }
    cdebug(4)<<"  wrote "<<count<<"/"<<tile.nrow()<<" rows\n";
    setFileState(DATA);
  }
  catch( std::exception &exc )
  {
    cdebug(1)<<"exception writing tile: "<<exc.what()<<endl;
  }
  catch( AipsError &err )
  {
    cdebug(1)<<"exception writing tile: "<<err.getMesg()<<endl;
  }
  
  return SUCCESS;
}

string MSVisOutputAgent::sdebug (int detail,const string &prefix,const char *name) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"MSVisOutputAgent",(int)this);
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
