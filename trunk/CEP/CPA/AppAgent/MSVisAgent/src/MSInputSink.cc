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
#include "MSInputSink.h"
#include <Common/BlitzToAips.h>
#include <DMI/AIPSPP-Hooks.h>

#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/MVPosition.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableParse.h>

using namespace LOFAR;

namespace MSVisAgent
{
using namespace VisAgent;
using namespace AppEvent;
using namespace AppState;

static int dum = aidRegistry_MSVisAgent();

//##ModelId=3DF9FECD0219
MSInputSink::MSInputSink ()
    : FileSink(),obsid_(0)
{
}

    
//##ModelId=3DF9FECD0285
void MSInputSink::fillHeader (DataRecord &hdr,const DataRecord &select)
{
  // get relevant selection parameters
  int ddid = select[FDDID].as<int>(0);        
  int fieldid = select[FFieldIndex].as<int>(0);        
  
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
    // get freqs
    Array<Double> ch_freq  = mssubc.chanFreq()(spw);
    Array<Double> ch_width = mssubc.chanWidth()(spw);
    num_channels_ = ch_freq.nelements();
    if( channels_[0]<0 )
      channels_[0] = 0; 
    if( channels_[1]<0 )
      channels_[1] = num_channels_; 
    FailWhen(channels_[1] < channels_[0] || channels_[1]>=num_channels_,
          "illegal channel selection");
    hdr[FChannelFreq]  = ch_freq(IPosition(1,channels_[0]),IPosition(1,channels_[1]));
    hdr[FChannelWidth] = ch_width(IPosition(1,channels_[0]),IPosition(1,channels_[1]));
        
    // now get the correlations & their types
    int polzn = mssub1c.polarizationId()(ddid);
    MSPolarization mssub2(ms_.polarization());
    ROMSPolarizationColumns mssub2c(mssub2);
    num_corrs_ = mssub2c.numCorr()(polzn);
    Vector<int> corrtypes = mssub2c.corrType()(polzn);
    vector<string> corrnames(corrtypes.nelements());
    for( uint i=0; i<corrtypes.nelements(); i++ )
      corrnames[i] = Stokes::name(Stokes::type(corrtypes(i)));
    hdr[FCorr] = corrnames;
  }
  // get antenna positions from ANTENNA subtable
  {
    MSAntenna          mssub(ms_.antenna());
    ROMSAntennaColumns mssubc(mssub);
    hdr[FNumAntenna] = num_antennas_ = mssub.nrow();
    // Just get all positions as a single array
    hdr[FAntennaPos] = mssubc.position().getColumn();
  }
}
 
//##ModelId=3DF9FECD025E
void MSInputSink::openMS (DataRecord &header,const DataRecord &select)
{
  // open MS
  ms_ = MeasurementSet(msname_,TableLock(TableLock::AutoNoReadLocking),Table::Old);
  dprintf(1)("opened MS %s, %d rows\n",msname_.c_str(),ms_.nrow());
  
  dprintf(3)("selection record is %s\n",select.sdebug(4).c_str());
  
  // get DDID and Field ID (default is 0)
  int ddid = select[FDDID].as<int>(0);        
  int fieldid = select[FFieldIndex].as<int>(0);        
  // Get range of channels (default values: all channles)
  channels_[0] = select[FChannelStartIndex].as<int>(0);
  channels_[1] = select[FChannelStartIndex].as<int>(-1);
  
  // fill header from MS
  fillHeader(header,select);
  // put MS name into header
  header[FMSName] = msname_;
  
  // figure out max ifr index
  num_ifrs_ = ifrNumber(num_antennas_-1,num_antennas_-1) + 1;
  
  // We only handle the given field & data desc id
  TableExprNode expr = ( ms_.col("FIELD_ID") == fieldid && ms_.col("DATA_DESC_ID") == ddid );
  selms_ = ms_(expr);
  
  vdsid_ = VDSID(obsid_++,0,0);
  header[FVDSID] = static_cast<HIID&>(vdsid_);
  header[FDDID] = ddid;
  header[FFieldIndex] = fieldid;
  header[FDataType] = HIID("MS.Non.Calibrated");
  header[FDataColumnName] = dataColName_;
  header[FDomainIndex] = -1; // negative domain index indicates full data
  
  header[FChannelStartIndex] = channels_[0];
  header[FChannelEndIndex]   = channels_[1];
  
  // get and apply selection string
  String where = select[FSelectionString].as<string>("");
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
  
  // get the first timeslot 
  header[FTime] = ROScalarColumn<double>(selms_, "TIME")(0);
  // get the original shape of the data array
  LoShape datashape = ROArrayColumn<Complex>(selms_,dataColName_).shape(0);
  header[FOriginalDataShape] = datashape;
  
  dprintf(1)("MS selection yields %d rows\n",selms_.nrow());
  tableiter_.reset();
  current_timeslot_ = 0;
}

//##ModelId=3DF9FECD0235
bool MSInputSink::init (const DataRecord &params)
{
  FailWhen( !FileSink::init(params),"FileSink init failed" );

  DataRecord &header = *new DataRecord;
  ObjRef href(header,DMI::ANONWR); 

  // get MS and selection
  msname_ = params[FMSName].as<string>();
  const DataRecord &selection = params[FSelection];
  // get name of data column (default is DATA)
  dataColName_ = params[FDataColumnName].as<string>("DATA");
  // get # of timeslots per tile (default is 1)
  tilesize_ = params[FTileSize].as<int>(1);

  openMS(header,selection);  

  // init common tile format and place it into header
  tileformat_ <<= new VisTile::Format;
  VisTile::makeDefaultFormat(tileformat_,num_corrs_,channels_[1]-channels_[0]+1);
  header[FTileFormat] <<= tileformat_.copy(); 

  tiles_.clear();
  
  setState(HEADER);

  // put header on output stream
  putOnStream(VisEventHIID(HEADER,vdsid_),href);

  return True;
}

//##ModelId=3DF9FECD0244
void MSInputSink::close ()
{
  FileSink::close();
  // close & detach from everything
  selms_ = MeasurementSet();
  ms_ = MeasurementSet();
  tileformat_.detach();
  setState(CLOSED);
}

//##ModelId=3DF9FECD021B
int MSInputSink::refillStream ()
{
  try
  {
    if( state() == HEADER )
      setState(DATA);
    else if( state() != DATA ) // return CLOSED when no more data
    {
      if( state() == FOOTER )
        setState(CLOSED);
      return AppEvent::CLOSED;
    }
  // loop until some tiles are generated
    int nout = 0;
    while( !nout )
    {
    // End of table iterator? Generate footer
      if( tableiter_.pastEnd() )
      {
        setState(FOOTER);
        DataRecord::Ref footer(DMI::ANONWR);
        footer()[FVDSID] = vdsid_; 
        putOnStream(VisEventHIID(FOOTER,vdsid_),footer);
        return AppEvent::SUCCESS;
      }
      const LoRange ALL = LoRange::all();
      const LoRange CHANS = LoRange(channels_[0],channels_[1]);
    // fill cache with next time interval
      if( tiles_.empty() )
        tiles_.resize(num_ifrs_);
    // loop until we've got the requisite number of timeslots
      for( int ntimes = 0; ntimes < tilesize_ && !tableiter_.pastEnd(); ntimes++,tableiter_++ )
      {
        const Table &table = tableiter_.table();
        int nrows = table.nrow();
        FailWhen( !nrows,"unexpected empty table iteration");
        dprintf(4)("Table iterator yields %d rows\n",table.nrow());
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
      // output all valid collected tiles onto stream
      for( uint i=0; i<tiles_.size(); i++ )
      {
        if( tiles_[i].valid() )
        {
          HIID id = VisEventHIID(DATA,tiles_[i]->tileId());
          putOnStream(id,tiles_[i]);
          tiles_[i].detach();
          nout++;  // increment pointer so that we break out of loop
        }
      }
      dprintf(2)("chunk yielded %d tiles\n",nout);
    }
    return AppEvent::SUCCESS;
  }
  // catch AIPS++ errors, but not our own exceptions -- these can only be
  // caused by real bugs
  catch( AipsError &err )
  {
    tiles_.clear();
    setErrorState("AIPS++ error: "+err.getMesg());
    return AppEvent::CLOSED;
  }
  return AppEvent::CLOSED;
}

//##ModelId=3DFDFC060373
string MSInputSink::sdebug ( int detail,const string &prefix,const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"MSInputSink",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    appendf(out,"MS %s (%d rows) %s",msname_.c_str(),selms_.nrow(),
        stateString().c_str());
  }
  if( detail >= 2 || detail <= -2 )
  {
    appendf(out,"timeslot %d, state %s",current_timeslot_,stateString().c_str());
  }
  return out;
}


} // namespace MSVisAgent
