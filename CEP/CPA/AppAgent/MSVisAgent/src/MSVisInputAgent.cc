//  MSVisInputAgent.cc: VisInputAgent for reading an AIPS++ MS
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

#define AIPSPP_HOOKS
#include "MSVisInputAgent.h"
#include <Common/BlitzToAips.h>
#include <DMI/AIPSPP-Hooks.h>

#include <aips/MeasurementSets/MSAntenna.h>
#include <aips/MeasurementSets/MSAntennaColumns.h>
#include <aips/MeasurementSets/MSDataDescription.h>
#include <aips/MeasurementSets/MSDataDescColumns.h>
#include <aips/MeasurementSets/MSField.h>
#include <aips/MeasurementSets/MSFieldColumns.h>
#include <aips/MeasurementSets/MSPolarization.h>
#include <aips/MeasurementSets/MSPolColumns.h>
#include <aips/MeasurementSets/MSSpectralWindow.h>
#include <aips/MeasurementSets/MSSpWindowColumns.h>
#include <aips/Measures/MDirection.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/Quanta/MVPosition.h>
#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/ColumnDesc.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/ExprNodeSet.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/TableParse.h>

using namespace MSVisAgentVocabulary;

static int dum = aidRegistry_MSVisAgent();

//##ModelId=3DF9FECD0219
//##ModelId=3DF9FECD0248
MSVisInputAgent::MSVisInputAgent ()
    : obsid_(0)
{
}
    
//##ModelId=3DF9FECD0285
void MSVisInputAgent::fillHeader (DataRecord &hdr,const DataRecord &select)
{
  // get relevant selection parameters
  int ddid = select[FDDID].as_int(0);        
  int fieldid = select[FFieldIndex].as_int(0);        
  
  // place current selection into header
  // clone a readonly snapshot, since selection will not change
  hdr[FSelection] <<= new DataRecord(select,DMI::DEEP|DMI::READONLY);
  
  // get phase reference from FIELD subtable
  {
    MSField mssub(ms_.field());
    ROMSFieldColumns mssubc(mssub);
      // Get the phase reference of the first field.
    MDirection dir = mssubc.phaseDirMeasCol()(fieldid)(IPosition(1,0));
    Vector<Double> dirvec = MDirection::Convert(dir, MDirection::J2000)().getValue().get();
    // assign to header
    hdr[FPhaseRef] = dirvec;
  }
  // get frequencies from DATA_DESC and SPW subtables
  // ddid determines which segment to read
  {
    // Get the frequency domain of the given data descriptor id
    // which gives as the spwid.
    MSDataDescription mssub1(ms_.dataDescription());
    ROMSDataDescColumns mssub1c(mssub1);
    int spw = mssub1c.spectralWindowId()(ddid);
    MSSpectralWindow mssub(ms_.spectralWindow());
    ROMSSpWindowColumns mssubc(mssub);
    hdr[FChannelFreq]  = mssubc.chanFreq()(spw);
    hdr[FChannelWidth] = mssubc.chanWidth()(spw);
    num_channels_ = hdr[FChannelFreq].size();
    // now get the correlations & their types
    int polzn = mssub1c.polarizationId()(ddid);
    MSPolarization mssub2(ms_.polarization());
    ROMSPolarizationColumns mssub2c(mssub2);
    num_corrs_ = mssub2c.numCorr()(polzn);
    hdr[FCorr] = mssub2c.corrType()(spw);
  }
  // get antenna positions from ANTENNA subtable
  {
    MSAntenna          mssub(ms_.antenna());
    ROMSAntennaColumns mssubc(mssub);
    num_antennas_ = mssub.nrow();
    // Just get all positions as a single array
    hdr[FAntennaPos] = mssubc.position().getColumn();
  }
}
 
//##ModelId=3DF9FECD025E
void MSVisInputAgent::openMS (DataRecord &header,const DataRecord &select)
{
  // open MS
  ms_ = MeasurementSet(msname_,Table::Old);
  dprintf(1)("opened MS %s, %d rows\n",msname_.c_str(),ms_.nrow());
  
  // get DDID and Field ID (default is 0)
  int ddid = select[FDDID].as_int(0);        
  int fieldid = select[FFieldIndex].as_int(0);        
  
  // fill header from MS
  fillHeader(header,select);
  // put MS name into header
  header[FMSName] = msname_;
  
  // figure out max ifr index
  num_ifrs_ = ifrNumber(num_antennas_-1,num_antennas_-1) + 1;
  
  // We only handle the given field & data desc id
  TableExprNode expr = ( ms_.col("FIELD_ID")==fieldid && ms_.col("DATA_DESC_ID") == ddid );
  selms_ = ms_(expr);
  
  vdsid_ = VDSID(ddid,fieldid,obsid_++);
  header[FVDSID] = static_cast<HIID&>(vdsid_);
  
  // Get range of channels (default values: all channles)
  channels_[0] = header[FChannelStartIndex] = select[FChannelStartIndex].as_int(0);
  channels_[1] = header[FChannelEndIndex]   = select[FChannelEndIndex].as_int(header[FChannelFreq].size()-1);
  
  // get and apply selection string
  String where = select[FSelectionString].as_string("");
  dprintf(1)("select ddid=%d, field=%d, where=\"%s\", channels=[%d:%d]\n",
      ddid,fieldid,where.c_str(),channels_[0],channels_[1]);
  if( where.empty() ) 
  {
    tableiter_  = TableIterator(selms_, "TIME");
  }
  else
  {
    selms_ = tableCommand ("select from $1 where " + where, selms_);
    FailWhen( !selms_.nrow(),"selection yields empty table" );
    tableiter_  = TableIterator(selms_, "TIME");
  } 
  dprintf(1)("MS selection yields %d rows\n",selms_.nrow());
  tableiter_.reset();
  current_timeslot_ = 0;
}

//##ModelId=3DF9FECD0235
bool MSVisInputAgent::init (const DataRecord::Ref &data)
{
  try
  {
    // make sure resume flag is cleared
    resume();
    
    const DataRecord &params = (*data)[FParams()];

    DataRecord &header = initHeader();
    
    msname_ = params[FMSName].as_string();
    const DataRecord &selection = params[FSelection];
    
    openMS(header,selection);  

    // get name of data column (default is DATA)
    dataColName_ = params[FDataColumnName].as_string("DATA");
    // get # of timeslots per tile (default is 1)
    tilesize_ = params[FTileSize].as_int(1);

    // init common tile format and place it into header
    tileformat_ <<= new VisTile::Format;
    VisTile::makeDefaultFormat(tileformat_,num_corrs_,channels_[1]-channels_[0]+1);
    header[FTileFormat] <<= tileformat_.copy(); 

    // resize cache of per-IFR tiles
    tiles_.clear();
    tiles_.resize(num_ifrs_);
    tileiter_ = tiles_.end();

    setFileState(HEADER);
  }
//  catch( std::exception &exc )
//  {
//    setErrorState(exc.what());
//    return False;
//  }
  catch( AipsError &err )
  {
    Throw("AIPS++ error: "+string(err.getMesg()));
//    setErrorState("AIPS++ error: "+err.getMesg());
    return False;
  }
  return True;
} 

//##ModelId=3DF9FECD0244
void MSVisInputAgent::close ()
{
  // close & detach from everything
  initHeader();  // empties the header
  selms_ = MeasurementSet();
  ms_ = MeasurementSet();
  tiles_.clear();
  tileformat_.detach();
  setFileState(FILECLOSED);
}

//##ModelId=3DF9FECD021B
int MSVisInputAgent::getNextTile (VisTile::Ref &tileref,int wait)
{
  try
  {
    int res = hasTile();
    if( res != SUCCESS )
    {
      FailWhen( res == WAIT && wait != AppAgent::NOWAIT,
          "can't wait here: would block indefinitely" );
      return res;
    }

  // any more tiles in cache? Return one
    if( tileiter_ != tiles_.end() )
    {
      // return next valid tile from cache
      while( !tileiter_->valid() && tileiter_ != tiles_.end() )
        tileiter_++;
      if( tileiter_ != tiles_.end() )
      {
        tileref = *tileiter_;
        if( Debug(5) )
        {
          const VisTile &tile = *tileref;
          cdebug(5) << "Tile ID: " << tile.tileId().toString() << endl;
          cdebug(5) << "TIME:" << tile.time() << endl;
          cdebug(5) << "INTERVAL: " << tile.interval() << endl;
          cdebug(5) << "ROW_FLAG:" << tile.rowflag() << endl;
          cdebug(5) << "UVW:" << tile.uvw() << endl;
          cdebug(5) << "DATA:" << tile.data() << endl;
          cdebug(5) << "FLAGS:" << tile.flags() << endl;
          cdebug(5) << "SEQNR:" << tile.seqnr() << endl;
        }
        return True;
      }
    }
  // no more tiles in cache -- have to refill. Check table iterator first.
    if( tableiter_.pastEnd() )
    {
      setFileState(ENDFILE);
      return CLOSED;
    }
    const LoRange ALL = LoRange::all();
    const LoRange CHANS = LoRange(channels_[0],channels_[1]);
  // fill cache with next time interval
  // loop until we've got the requisite number of timeslots
    for( int ntimes = 0; ntimes < tilesize_ && !tableiter_.pastEnd(); ntimes++,tableiter_++ )
    {
      const Table &table = tableiter_.table();
      int nrows = table.nrow();
      FailWhen( !nrows,"unexpected empty table iteration");
      dprintf(3)("Table iterator yields %d rows\n",table.nrow());
      // get relevant table columns
      ROScalarColumn<Double> timeCol(table,"TIME");
      ROScalarColumn<Double> intCol(table,"INTERVAL");
      ROScalarColumn<Int> ant1col(table,"ANTENNA1");
      ROScalarColumn<Int> ant2col(table,"ANTENNA2");
      ROScalarColumn<Bool> rowflagCol(table,"FLAG_ROW");
      // get array columns as Lorrays
      Matrix<Double> uvwmat1 = ROArrayColumn<Double>(table, "UVW").getColumn();
//      cdebug(5)"UVWMAT1: "<<uvwmat1<<endl;
      LoMat_double uvwmat = refAipsToBlitz<double,2>(uvwmat1);
//      cdebug(5)"UVWMAT: "<<uvwmat<<endl;
      Cube<Complex> datacube1 = ROArrayColumn<Complex>(table, dataColName_).getColumn();
      LoCube_fcomplex datacube = refAipsToBlitz<fcomplex,3>(datacube1);
      Cube<Bool> flagcube1 = ROArrayColumn<Bool>(table,"FLAG").getColumn();
      LoCube_bool flagcube = refAipsToBlitz<bool,3>(flagcube1);
//      cdebug(5)"DATACUBE: "<<datacube<<endl;
      // get vector of row numbers 
      Vector<uInt> rownums = table.rowNumbers(ms_);
  // now process rows one by one
      for( int i=0; i<nrows; i++ )
      {
        int ant1 = ant1col(i), ant2 = ant2col(i);
        int ifr = ifrNumber(ant1,ant2);
  // init tile if one is not ready 
        VisTile *ptile;
        if( tiles_[ifr].valid() )
          ptile = tiles_[ifr].dewr_p();
        else
        {
          tiles_[ifr] <<= ptile = new VisTile(tileformat_,tilesize_);
          // set tile ID
          ptile->setTileId(ant1col(i),ant2col(i),vdsid_);
          // init all row flags to missing
          ptile->wrowflag() = FlagMissing;
        }
        ptile->wtime()(ntimes)     = timeCol(i);
        ptile->winterval()(ntimes) = intCol(i);
        ptile->wrowflag()(ntimes)  = rowflagCol(i) ? 1 : 0;
        ptile->wuvw()(ALL,ntimes)  = uvwmat(ALL,i);
        ptile->wdata()(ALL,ALL,ntimes) = datacube(ALL,CHANS,i);
        ptile->wflags()(ALL,ALL,ntimes) = where(flagcube(ALL,CHANS,i),1,0);
        ptile->wseqnr()(ntimes) = rownums(i);
      }
      current_timeslot_++;
    }
    // reset tile iterator to beginning of cache and call ourselves again
    // to return it
    tileiter_ = tiles_.begin();
    return getNextTile(tileref,wait);
  }
  // catch AIPS++ errors, but not our own exceptions -- these can only be
  // caused by real bugs
  catch( AipsError &err )
  {
    setErrorState("AIPS++ error: "+err.getMesg());
    return CLOSED;
  }
}


//##ModelId=3DFDFC060373
string MSVisInputAgent::sdebug ( int detail,const string &prefix,const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"MSVisInputAgent",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    appendf(out,"MS %s (%d rows) %s",msname_.c_str(),selms_.nrow(),
        stateString().c_str());
  }
  if( detail >= 2 || detail <= -2 )
  {
    appendf(out,"timeslot %d, state %s",current_timeslot_,fileStateString().c_str());
  }
  return out;
}
