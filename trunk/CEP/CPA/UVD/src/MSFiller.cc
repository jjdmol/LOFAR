//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CEA4ACC0191.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CEA4ACC0191.cm

//## begin module%3CEA4ACC0191.cp preserve=no
//## end module%3CEA4ACC0191.cp

//## Module: MSFiller%3CEA4ACC0191; Package body
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSFiller.cc

//## begin module%3CEA4ACC0191.additionalIncludes preserve=no
//## end module%3CEA4ACC0191.additionalIncludes

//## begin module%3CEA4ACC0191.includes preserve=yes
#include <aips/MeasurementSets.h>
#include <aips/Tables/IncrementalStMan.h>
#include <aips/Tables/StandardStMan.h>
#include <aips/Tables/TiledShapeStMan.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/TableDesc.h>
#include <aips/Tables/TableRecord.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/Cube.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/Containers/Block.h>
#include <aips/Measures/MPosition.h>
#include <aips/Measures/MBaseline.h>
#include <aips/Measures/Muvw.h>
#include <aips/Measures/MeasTable.h>
#include <aips/Measures/Stokes.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/Quanta/MVEpoch.h>
#include <aips/Quanta/MVDirection.h>
#include <aips/Quanta/MVPosition.h>
#include <aips/Quanta/MVBaseline.h>
#include <aips/OS/Time.h>
#include <aips/OS/Time.h>
#include <aips/Mathematics/Constants.h>
#include <aips/Utilities/Assert.h>
#include <aips/Exceptions/Error.h>
#include "DMI/AIPSPP-Hooks.h"
#include "UVD/UVD.h"
//## end module%3CEA4ACC0191.includes

// MSFiller
#include "UVD/MSFiller.h"
//## begin module%3CEA4ACC0191.declarations preserve=no
//## end module%3CEA4ACC0191.declarations

//## begin module%3CEA4ACC0191.additionalDeclarations preserve=yes
using namespace UVD;

// this helper class implements a virtual columns
// (in lieu of truly virtual array dimensions)
template<class T>
class Virtual
{
  protected:
      const T* data;
      bool is_vec;

  public:
      // initializes a virtual column
      void init (const NestableContainer &rec,const HIID &id,int nrow)
      {
        int sz;
        data = &rec[id].size(sz);
        is_vec = sz > 1;
        if( is_vec )
          FailWhen(sz != nrow,"mismatch in size of column "+id.toString());
      }
      
      Virtual () {};

      Virtual (const NestableContainer &rec,const HIID &id,int nrow)
      {
        init(rec,id,nrow);
      }

      T operator [] (int n) const 
      {
        return data[ is_vec ? n : 0 ];
      }
};

// this helper class implements an optional virtual column
// same as Virtual, except the column is allowed to not exist
template<class T>
class Optional : public Virtual<T>
{
  protected:
      T defval;
  
  public:
      // initializes an optional/virtual column
      Optional(const NestableContainer &rec,const HIID &id,int nrow,T Defval = 0 )
        : defval(Defval)
      {
        if( rec[id].exists() )
          init(rec,id,nrow);
        else
          data = 0;
      }

      bool exists () const
      { return data != 0; };

      T operator [] (int n) const 
      {
        if( !data )
          return defval;
        return Virtual<T>::operator [] (n);
      }
};

//## end module%3CEA4ACC0191.additionalDeclarations


// Class MSFiller 

MSFiller::MSFiller (const string &msname, const DataRecord &hdr)
  //## begin MSFiller::MSFiller%3CEB5F5A027D.hasinit preserve=no
  //## end MSFiller::MSFiller%3CEB5F5A027D.hasinit
  //## begin MSFiller::MSFiller%3CEB5F5A027D.initialization preserve=yes
  : mscol_(0),mspolCol_(0),msddCol_(0)
  //## end MSFiller::MSFiller%3CEB5F5A027D.initialization
{
  //## begin MSFiller::MSFiller%3CEB5F5A027D.body preserve=yes
  if( msname.length() )
    create(msname,hdr);
  //## end MSFiller::MSFiller%3CEB5F5A027D.body
}


MSFiller::~MSFiller()
{
  //## begin MSFiller::~MSFiller%3CEA4A9E0070_dest.body preserve=yes
  if( mscol_ )
    delete mscol_;
  if( mspolCol_ )
    delete mspolCol_;
  if( msddCol_ )
    delete msddCol_;
  //## end MSFiller::~MSFiller%3CEA4A9E0070_dest.body
}



//## Other Operations (implementation)
bool MSFiller::create (const string &msname, const DataRecord &hdr)
{
  //## begin MSFiller::create%3CEA4AFB02B9.body preserve=yes
  // for now: delete table first
  try 
  {
    MeasurementSet dum(msname,Table::Delete);
  } 
  catch( AipsError & )
  {
  }

  
  // grab & store header
  // Get the MS main default table description..
  TableDesc td = MS::requiredTableDesc();
  // Add the data column and its unit.
  MS::addColumnToDesc(td, MS::DATA, 2);
  td.rwColumnDesc(MS::columnName(MS::DATA)).rwKeywordSet().define("UNIT","Jy");
  // Store the data and flags using the TiledStMan.
  Vector<String> tsmNames(2);
  tsmNames(0) = MS::columnName(MS::DATA);
  tsmNames(1) = MS::columnName(MS::FLAG);
  td.defineHypercolumn("TiledData", 3, tsmNames);
  // Setup the new table.
  // Most columns use the IncrStMan; some use others.
  SetupNewTable newTab(msname, td, Table::New);
  IncrementalStMan incrStMan("ISMData");
  StandardStMan    stanStMan;
  TiledShapeStMan  tiledStMan("TiledData", IPosition(3,4,16,512));
  newTab.bindAll (incrStMan);
  newTab.bindColumn(MS::columnName(MS::ANTENNA1),stanStMan);
  newTab.bindColumn(MS::columnName(MS::ANTENNA2),stanStMan);
  newTab.bindColumn(MS::columnName(MS::DATA),tiledStMan);
  newTab.bindColumn(MS::columnName(MS::FLAG),tiledStMan);
  newTab.bindColumn(MS::columnName(MS::UVW),stanStMan);
  // Create the MS and its subtables.
  // Get access to its columns.
  ms_ = MeasurementSet(newTab);
  mscol_ = new MSMainColumns(ms_);
  // Create all subtables.
  // Do this after the creation of optional subtables,
  // so the MS will know about those optional sutables.
  ms_.createDefaultSubtables(Table::New);
  
  // fill in subtables based on header
  fillAntenna(hdr);
  fillFeed(hdr);
  fillSource(hdr);
  fillField(hdr);
  fillSpectralWindow(hdr);

  // these subtables are filled in on the fly
  mspol_ = ms_.polarization();
  mspolCol_ = new MSPolarizationColumns(mspol_);
  polmap.clear();
  
  msdd_ = ms_.dataDescription();
  msddCol_ = new MSDataDescColumns(msdd_);
  
  return True;
  //## end MSFiller::create%3CEA4AFB02B9.body
}

bool MSFiller::startSegment (const DataRecord &hdr)
{
  //## begin MSFiller::startSegment%3CEB5AA201D5.body preserve=yes
  // allocate new DDIs for the spw, field & correlation set
  allocateDDI(hdr[FSPWIndex].as_int(),hdr[FCorr].as_vector<int>());
  field_id = hdr[FFieldIndex];
  num_channels = hdr[FNumChannels];
  return True;
  //## end MSFiller::startSegment%3CEB5AA201D5.body
}

bool MSFiller::addChunk (const DataRecord &rec)
{
  //## begin MSFiller::addChunk%3CEB5E3E00C6.body preserve=yes
  // data chunk is indexed by either time or baseline
  // perhaps both, but then they must be congruent
  int num_ifrs = rec[FIFRIndex].size();
  int num_times = rec[FTime].size();
  FailWhen( num_ifrs>1 && num_times>1 && num_ifrs != num_times,
            "mismatch in number of times or baselines");
  int nrows = max(num_ifrs,num_times);

// this tells us howto index the antenna array (in lieu of virtual array dim's)  
  bool ant_vec = num_ifrs > 1;

  // get the correlation type, and determine the DDID
  int corrtype = rec[FCorr];
  FailWhen( ddimap.find(corrtype) == ddimap.end(),"unexpected correlation type" );
  int ddi = ddimap[corrtype];
  
  // add rows to MS
  int irow = ms_.nrow();
  ms_.addRow(nrows);
  
  // setup pointers to data and flags
  const dcomplex *pdata = &rec[FData];
  const int *pflag = rec[FDataFlag].exists() ? rec[FDataFlag].as_int_p() : 0;
  Vector<Complex> datavec(num_channels);
  Vector<Bool> flagvec(num_channels,False);
  
  const float *pweight = rec[FWeight].exists() ? rec[FWeight].as_float_p() : 0;
  const float *psigma  = rec[FSigma].exists() ? rec[FSigma].as_float_p() : 0;
  
  // setup optional columns
  Virtual<double> 
    time(rec,FTime,nrows),
    exposure(rec,FExposure,nrows);
  Optional<double> 
    centroid(rec,FTimeCentroid,nrows),
    interval(rec,FInterval,nrows);
  
  // get UVW matrix (3,nrows)
  Matrix<Double> uvw = rec[FUVW].as_Array_double();

  // loop over all rows in the chunk
  for( int i=0; i<nrows; i++,irow++ )
  {
    // build data and flag vectors
    if( pflag )
    {
      for( int j=0; j<num_channels; j++,pdata++ )
      {
        datavec(i) = Complex(pdata->real(),pdata->imag());
        flagvec(i) = *pflag++;
      }
    }
    else
      for( int j=0; j<num_channels; j++,pdata++ )
        datavec(i) = Complex(pdata->real(),pdata->imag());
    // write row to MS
    mscol_->data().put(irow,datavec);
    mscol_->flag().put(irow,flagvec);
	  mscol_->flagRow().put(irow,rec[FRowFlag][i].as_int(0));
	  mscol_->time().put(irow,time[i]);
	  mscol_->antenna1().put(irow,ant_vec ? rec[FAntennaIndex](0,i).as_int() : rec[FAntennaIndex][0].as_int());
	  mscol_->antenna2().put(irow,ant_vec ? rec[FAntennaIndex](1,i).as_int() : rec[FAntennaIndex][1].as_int());
	  mscol_->feed1().put(irow,0);
	  mscol_->feed2().put(irow,0);
	  mscol_->dataDescId().put(irow,ddi);
	  mscol_->processorId().put(irow,0);
	  mscol_->fieldId().put(irow,field_id);
	  mscol_->interval().put(irow,interval[i]);
	  mscol_->exposure().put(irow,exposure[i]);
    if( centroid.exists() )
	    mscol_->timeCentroid().put(irow,centroid[i]);
    else
	    mscol_->timeCentroid().put(irow,time[i]);
	  mscol_->scanNumber().put(irow,0);
	  mscol_->arrayId().put(irow,0);
	  mscol_->observationId().put(irow,0);
	  mscol_->stateId().put(irow,0);
	  mscol_->uvw().put(irow,uvw.column(i));
    // fill in weight and sigma, if given
    // cast away const since we know the vector is read-only
    if( pweight )
    {
	    mscol_->weight().put(irow,Vector<Float>(IPosition(1,num_channels),
                  const_cast<float*>(pweight),SHARE));
      pweight += num_channels;
    }
    if( psigma )
    {
	    mscol_->sigma().put(irow,Vector<Float>(IPosition(1,num_channels),
                  const_cast<float*>(psigma),SHARE));
      psigma += num_channels;
    }
  }
  return True;
  //## end MSFiller::addChunk%3CEB5E3E00C6.body
}

bool MSFiller::endSegment ()
{
  //## begin MSFiller::endSegment%3CEB5E4F01CF.body preserve=yes
  return True;
  //## end MSFiller::endSegment%3CEB5E4F01CF.body
}

void MSFiller::close ()
{
  //## begin MSFiller::close%3CECBBD102DA.body preserve=yes
  msdd_.flush();
  msdd_ = MSDataDescription();
  mspol_.flush();
  mspol_ = MSPolarization();
  ms_.flush();
  ms_ = MeasurementSet();
  delete mscol_;    mscol_ = 0;
  delete mspolCol_; mspolCol_ = 0;
  delete msddCol_;  msddCol_ = 0;
  //## end MSFiller::close%3CECBBD102DA.body
}

// Additional Declarations
  //## begin MSFiller%3CEA4A9E0070.declarations preserve=yes
  //## end MSFiller%3CEA4A9E0070.declarations

//## begin module%3CEA4ACC0191.epilog preserve=yes
// Allocates a DDI for this SPW and every correlation.
// Updates the DATA_DESCRIPTION and POLARIZATION tables
void MSFiller::allocateDDI (int spwid,const vector<int> &corrs)
{
  ddimap.clear();
  int nrow = msddCol_->nrow();
  int ncorr = corrs.size();
  msdd_.addRow(ncorr);
  for( int i=0; i<ncorr; i++,nrow++ )
  {
    int corrtype = corrs[i];
    int polzn_id;
    ddimap[corrtype] = nrow;
    // see if this corrtype is already in the POLARIZATION table
    CPMI iter = polmap.find(corrtype);
    if( iter == polmap.end() )
    {
      // no? then allocate an entry
      polmap[corrtype] = polzn_id = mspolCol_->nrow();
      mspol_.addRow(1);
      mspolCol_->numCorr().put(polzn_id,1);
      Vector<Int> corrtypes(1);
      corrtypes = corrtype;
      mspolCol_->corrType().put(polzn_id,corrtypes);
      Matrix<Int> corrproducts(2,1);
      corrproducts(0,0) = Stokes::receptor1(Stokes::type(corrtype));
      corrproducts(1,0) = Stokes::receptor2(Stokes::type(corrtype));
      mspolCol_->corrProduct().put(i,corrproducts);
      mspolCol_->flagRow().put(polzn_id,False);
    }
    else
      polzn_id = iter->second;
    // add entry to DD table
    msddCol_->spectralWindowId().put(nrow,spwid);
    msddCol_->polarizationId().put(nrow,polzn_id);
    msddCol_->flagRow().put(nrow,False);
  }
}

void MSFiller::fillAntenna (const DataRecord &rec)
{
  // use AIPS++ antenna subtable, if it exists
  if( rec[AidAIPSPP|FAntennaSubtable].exists() )
  {
    const DataTable &ant = rec[AidAIPSPP|FAntennaSubtable];
    // Fill the ANTENNA subtable.
    MSAntenna msant = ms_.antenna();
    MSAntennaColumns msantCol(msant);
    int nant = ant[FName].size();
    msant.addRow(nant);
    
    msantCol.name().putColumn(ant[FName].as_Vector<String>());
    msantCol.station().putColumn(ant[FStationName].as_Vector<String>());
    msantCol.type().putColumn(ant[FType].as_Vector<String>());
    msantCol.mount().putColumn(ant[FMount].as_Vector<String>());
    msantCol.position().putColumn(ant[FPosition].as_Array_double());
    msantCol.offset().putColumn(ant[FOffset].as_Array_double());
    msantCol.dishDiameter().putColumn(ant[FDishDiameter].as_Vector<Double>());
    
    for( int i=0; i<nant; i++) 
      msantCol.flagRow().put(i,ant[FRowFlag][i].as_int(0) != 0);
    
    msant.flush();
  }
}

void MSFiller::fillSource (const DataRecord &rec)
{
  if( rec[AidAIPSPP|FSourceSubtable].exists() )
  {
    const DataTable &src = rec[AidAIPSPP|FSourceSubtable];
    // Fill the ANTENNA subtable.
    MSSource mssrc = ms_.source();
    MSSourceColumns mssrcCol(mssrc);
    int nsrc = src[FSourceIndex].size();
    mssrc.addRow(nsrc);
    mssrcCol.sourceId().putColumn(src[FSourceIndex].as_Vector<Int>());
    mssrcCol.time().putColumn(src[FTime].as_Vector<Double>());
    mssrcCol.interval().putColumn(src[FTime].as_Vector<Double>());
    mssrcCol.spectralWindowId().putColumn(src[FSPWIndex].as_Vector<Int>());
    mssrcCol.numLines().putColumn(src[FNumLines].as_Vector<Int>());
    mssrcCol.name().putColumn(src[FName].as_Vector<String>());
    mssrcCol.calibrationGroup().putColumn(src[FCalibrationGroup].as_Vector<Int>());
    mssrcCol.code().putColumn(src[FCode].as_Vector<String>());
    mssrcCol.direction().putColumn(src[FDirection].as_Array_double());
    mssrcCol.position().putColumn(src[FPosition].as_Array_double());
    mssrcCol.properMotion().putColumn(src[FProperMotion].as_Array_double());
    mssrc.flush();
  }
}

void MSFiller::fillSpectralWindow (const DataRecord &rec)
{
  // use AIPS++ antenna subtable, if it exists
  if( rec[AidAIPSPP|FSPWSubtable].exists() )
  {
    const DataTable &spw = rec[AidAIPSPP|FSPWSubtable];
    MSSpectralWindow msspw = ms_.spectralWindow();
    MSSpWindowColumns msspwCol(msspw);
    int n = spw[FName].size();
    msspw.addRow(n);
    
    msspwCol.name().putColumn(spw[FName].as_Vector<String>());
    msspwCol.numChan().putColumn(spw[FNumChannels].as_Vector<Int>());
    msspwCol.refFrequency().putColumn(spw[FRefFreq].as_Vector<Double>());
    msspwCol.chanFreq().putColumn(spw[FChannelFreq].as_Array_double());
    msspwCol.chanWidth().putColumn(spw[FChannelWidth].as_Array_double());
    msspwCol.measFreqRef().putColumn(spw[FMeasFreqRef].as_Vector<Int>());
    msspwCol.effectiveBW().putColumn(spw[FEffectiveBW].as_Array_double());
    msspwCol.resolution().putColumn(spw[FResolution].as_Array_double());
    msspwCol.totalBandwidth().putColumn(spw[FTotalBW].as_Vector<Double>());
    msspwCol.netSideband().putColumn(spw[FNetSideband].as_Vector<Int>());
    msspwCol.ifConvChain().putColumn(spw[FIFConvChain].as_Vector<Int>());
    msspwCol.freqGroup().putColumn(spw[FFreqGroup].as_Vector<Int>());
    msspwCol.freqGroupName().putColumn(spw[FFreqGroupName].as_Vector<String>());
      
    for( int i=0; i<n; i++ )
      msspwCol.flagRow().put(i,spw[FRowFlag][i].as_bool(False));
    
    msspw.flush();
  }
}

void MSFiller::fillField (const DataRecord &rec)
{
  // use AIPS++ antenna subtable, if it exists
  if( rec[AidAIPSPP|FFieldSubtable].exists() )
  {
    const DataTable &fld = rec[AidAIPSPP|FFieldSubtable];
    MSField msfield = ms_.field();
    MSFieldColumns msfieldCol(msfield);
    int n = fld[FName].size();
    msfield.addRow(n);
    
    msfieldCol.name().putColumn(fld[FName].as_Vector<String>());
    msfieldCol.code().putColumn(fld[FCode].as_Vector<String>());
    msfieldCol.time().putColumn(fld[FTime].as_Vector<Double>());
    msfieldCol.numPoly().putColumn(fld[FNumPoly].as_Vector<Int>());
    msfieldCol.delayDir().putColumn(fld[FDelayDirMeas].as_Array_double());
    msfieldCol.phaseDir().putColumn(fld[FPhaseDirMeas].as_Array_double());
    msfieldCol.referenceDir().putColumn(fld[FRefDirMeas].as_Array_double());
    msfieldCol.sourceId().putColumn(fld[FSourceIndex].as_Vector<Int>());
      
    for( int i=0; i<n; i++ )
      msfieldCol.flagRow().put(i,fld[FRowFlag][i].as_int()!=0);
    msfield.flush();
  }
}

void MSFiller::fillFeed (const DataRecord &rec)
{
  if( rec[AidAIPSPP|FFeedSubtable].exists() )
  {
    const DataTable &feed = rec[AidAIPSPP|FFeedSubtable];
    // Fill the ANTENNA subtable.
    MSFeed msfeed = ms_.feed();
    MSFeedColumns msfeedCol(msfeed);
    int nfeed = feed[FAntennaIndex].size();
    msfeed.addRow(nfeed);
    
    msfeedCol.antennaId().putColumn(feed[FAntennaIndex].as_Vector<Int>());
    msfeedCol.feedId().putColumn(feed[FFeedIndex].as_Vector<Int>());
    msfeedCol.spectralWindowId().putColumn(feed[FSPWIndex].as_Vector<Int>());
    msfeedCol.time().putColumn(feed[FTime].as_Vector<Double>());
    msfeedCol.interval().putColumn(feed[FInterval].as_Vector<Double>());
    msfeedCol.numReceptors().putColumn(feed[FNumReceptors].as_Vector<Int>());
    msfeedCol.beamId().putColumn(feed[FBeamIndex].as_Vector<Int>());
    msfeedCol.beamOffset().putColumn(feed[FBeamOffset].as_Array_double());
    msfeedCol.polResponse().putColumn(feed[FPolznResponse].as_Array_fcomplex());
    msfeedCol.position().putColumn(feed[FPosition].as_Array_double());
    msfeedCol.receptorAngle().putColumn(feed[FReceptorAngle].as_Array_double());
      
    for( int i=0; i<nfeed; i++) 
    {
      int nfeed = feed[FNumReceptors][i].as_int();
      // split feed[FPolznType][i] at spaces, and fill in vector
      String poltype[nfeed];
      split(feed[FPolznType][i].as_string(),poltype,nfeed," ");
      msfeedCol.polarizationType().put(i,Vector<String>(IPosition(1,nfeed),poltype,SHARE));
    }
    msfeed.flush();
  }
}

//## end module%3CEA4ACC0191.epilog
