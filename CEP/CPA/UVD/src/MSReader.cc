//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CF2376F01E4.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CF2376F01E4.cm

//## begin module%3CF2376F01E4.cp preserve=no
//## end module%3CF2376F01E4.cp

//## Module: MSReader%3CF2376F01E4; Package body
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSReader.cc

//## begin module%3CF2376F01E4.additionalIncludes preserve=no
//## end module%3CF2376F01E4.additionalIncludes

//## begin module%3CF2376F01E4.includes preserve=yes
#include "DMI/AIPSPP-Hooks.h"
#include "UVD/UVD.h"
#include <aips/Exceptions/Error.h>
//## end module%3CF2376F01E4.includes

// MSReader
#include "UVD/MSReader.h"
//## begin module%3CF2376F01E4.declarations preserve=no
//## end module%3CF2376F01E4.declarations

//## begin module%3CF2376F01E4.additionalDeclarations preserve=yes
using namespace UVD;
//##ModelId=3CF237080217
//## end module%3CF2376F01E4.additionalDeclarations


// Class MSReader 

MSReader::MSReader()
  //## begin MSReader::MSReader%3CF237080203_const.hasinit preserve=no
  //## end MSReader::MSReader%3CF237080203_const.hasinit
  //## begin MSReader::MSReader%3CF237080203_const.initialization preserve=yes
  //## end MSReader::MSReader%3CF237080203_const.initialization
{
  //## begin MSReader::MSReader%3CF237080203_const.body preserve=yes
  //## end MSReader::MSReader%3CF237080203_const.body
}

//##ModelId=3DB9388F01AB
MSReader::MSReader (const MeasurementSet &ms)
  //## begin MSReader::MSReader%3CF237080217.hasinit preserve=no
  //## end MSReader::MSReader%3CF237080217.hasinit
  //## begin MSReader::MSReader%3CF237080217.initialization preserve=yes
  //## end MSReader::MSReader%3CF237080217.initialization
{
  //## begin MSReader::MSReader%3CF237080217.body preserve=yes
  attach(ms);
  //## end MSReader::MSReader%3CF237080217.body
}


//##ModelId=3DB9388F01DD
MSReader::~MSReader()
{
  //## begin MSReader::~MSReader%3CF237080203_dest.body preserve=yes
  //## end MSReader::~MSReader%3CF237080203_dest.body
}



//##ModelId=3CF4A1F100DC
//## Other Operations (implementation)
void MSReader::attach (const MeasurementSet &ms)
{
  //## begin MSReader::attach%3CF4A1F100DC.body preserve=yes
  ms_ = ms;
  //## end MSReader::attach%3CF4A1F100DC.body
}

//##ModelId=3CF4AC140048
void MSReader::makeHeader (DataRecord &hdr)
{
  //## begin MSReader::makeHeader%3CF4AC140048.body preserve=yes
  hdr[FAipsMSName] = ms_.tableName();
  readAntenna(hdr);
  readSource(hdr);
  readSpectralWindow(hdr);
  readField(hdr);
  readFeed(hdr);
  //## end MSReader::makeHeader%3CF4AC140048.body
}

// Additional Declarations
  //## begin MSReader%3CF237080203.declarations preserve=yes
static DataRecord & addSubtable( DataRecord &rec,const HIID &id)
{
  DataRecord *subtable = new DataRecord;
  rec[AidAIPSPP|id] <<= subtable;
  return *subtable;
}

static void removeSubtable( DataRecord &rec,const HIID &id)
{
  if( rec[AidAIPSPP|id].exists() )
    rec[AidAIPSPP|id].remove();
}

static void addRowFlag( DataRecord &subtable,const ROScalarColumn<Bool> &col )
{
  int n = col.nrow();
  DataField *df = new DataField(Tpint,n);
  subtable[FRowFlag] <<= df;
  for( int i=0; i<n; i++ )
    (*df)[i] = col(i) ? 1 : 0;
}


//##ModelId=3DB9388F02A5
void MSReader::readAntenna (DataRecord &rec)
{
  try 
  {
    MSAntenna msant = ms_.antenna();
    ROMSAntennaColumns msantCol(msant);

    DataRecord &subtable = addSubtable(rec,FAntennaSubtable);

    subtable[FName] = msantCol.name().getColumn();
    subtable[FStationName] = msantCol.station().getColumn();
    subtable[FType] = msantCol.type().getColumn();
    subtable[FMount] = msantCol.mount().getColumn();
    subtable[FPosition] = msantCol.position().getColumn();
    subtable[FOffset] = msantCol.offset().getColumn();
    subtable[FDishDiameter] = msantCol.dishDiameter().getColumn();

    addRowFlag(subtable,msantCol.flagRow());
  }
  catch( AipsError &err )
  {
    dprintf(0)("error reading ANTENNA subtable: %s\n",err.getMesg().c_str());
    removeSubtable(rec,FAntennaSubtable);
  }
}

//##ModelId=3DB9388F031D
void MSReader::readSource (DataRecord &rec)
{
  try 
  {
    MSSource mssrc = ms_.source();
    ROMSSourceColumns mssrcCol(mssrc);

    DataRecord &subtable = addSubtable(rec,FSourceSubtable);

    subtable[FSourceIndex] = mssrcCol.sourceId().getColumn();
    subtable[FTime] = mssrcCol.time().getColumn();
    subtable[FInterval] = mssrcCol.interval().getColumn();
    subtable[FSPWIndex] = mssrcCol.spectralWindowId().getColumn();
    subtable[FNumLines] = mssrcCol.numLines().getColumn();
    subtable[FName] = mssrcCol.name().getColumn();
    subtable[FCalibrationGroup] = mssrcCol.calibrationGroup().getColumn();
    subtable[FCode] = mssrcCol.code().getColumn();
    subtable[FDirection] = mssrcCol.direction().getColumn();
    subtable[FPosition] = mssrcCol.position().getColumn();
    subtable[FProperMotion] = mssrcCol.properMotion().getColumn();
  }
  catch( AipsError &err )
  {
    dprintf(0)("error reading SOURCE subtable: %s\n",err.getMesg().c_str());
    removeSubtable(rec,FSourceSubtable);
  }
}

// void MSReader::readPolarization (DataRecord &rec)
// {
//   MSPolarization mssrc = ms_.polarization();
//   ROMSPolarizationColumns mspolCol(mspol);
//   int nrow = mspolCol.nrow();
//   
//   DataTable &src = *new DataField(TpRecord,nrow);
//   rec[AidAIPSPP|FPolarizationSubtable] <<= &src;
//   
//   for( int i=0; i<nrow; i++) 
//   {
//     DataTableRow &row = src[i];
//     int n = mspolCol.numCorr()(i);
//     row[FNumCorr] = n;
//     row[FCorrType] = mspolCol.corrType()(i);
//     row[FCorrProduct] = mspolCol.corrProduct()(i);
//     row[FRowFlag] = (mspolCol.flagRow()(i) ? 1 : 0);
//   }
// }
// 

//##ModelId=3DB9388F0396
void MSReader::readSpectralWindow (DataRecord &rec)
{
  try
  {
    MSSpectralWindow msspw = ms_.spectralWindow();
    ROMSSpWindowColumns msspwCol(msspw);

    DataRecord &subtable = addSubtable(rec,FSPWSubtable);

    subtable[FName] = msspwCol.name().getColumn();
    subtable[FRefFreq] = msspwCol.refFrequency().getColumn();
    subtable[FNumChannels] = msspwCol.numChan().getColumn();
    subtable[FChannelFreq] = msspwCol.chanFreq().getColumn();
    subtable[FChannelWidth] = msspwCol.chanWidth().getColumn();
    subtable[FMeasFreqRef] = msspwCol.measFreqRef().getColumn();
    subtable[FEffectiveBW] = msspwCol.effectiveBW().getColumn();
    subtable[FResolution] = msspwCol.resolution().getColumn();
    subtable[FTotalBW] = msspwCol.totalBandwidth().getColumn();
    subtable[FNetSideband] = msspwCol.netSideband().getColumn();
    subtable[FIFConvChain] = msspwCol.ifConvChain().getColumn();
    subtable[FFreqGroup] = msspwCol.freqGroup().getColumn();
    subtable[FFreqGroupName] = msspwCol.freqGroupName().getColumn();

    addRowFlag(subtable,msspwCol.flagRow());
  }
  catch( AipsError &err )
  {
    dprintf(0)("error reading SPECTRAL_WINDOW subtable: %s\n",err.getMesg().c_str());
    removeSubtable(rec,FSPWSubtable);
  }
}

// void MSReader::readDataDescription (DataRecord &rec)
// {
//   MSDataDescription msdd = ms_.dataDescription();
//   ROMSDataDescColumns msddCol(msdd);
//   int nrow = msddCol.nrow();
//   
//   DataTable &dd = *new DataField(TpRecord,nrow);
//   rec[AidAIPSPP|FDataDescriptionSubtable] <<= &spw;
//   
//   for( int i=0; i<nrow; i++ )
//   {
//     const DataTableRow &row = dd[i];
//     row[FSPWIndex] = msddCol.spectralWindowId()(i);
//     row[FPolarizationIndex] = msddCol.polarizationId()(i);
//     row[FRowFlag] = msddCol.flagRow()(i) ? 1 : 0;
//   }
// }
// 
//##ModelId=3DB93890003A
void MSReader::readField (DataRecord &rec)
{
  try
  {
    MSField msfield = ms_.field();
    ROMSFieldColumns msfieldCol(msfield);

    DataRecord &subtable = addSubtable(rec,FFieldSubtable);

    subtable[FName] = msfieldCol.name().getColumn();
    subtable[FCode] = msfieldCol.code().getColumn();
    subtable[FTime] = msfieldCol.time().getColumn();
    subtable[FNumPoly] = msfieldCol.numPoly().getColumn();
    subtable[FDelayDirMeas] = msfieldCol.delayDir().getColumn();
    subtable[FPhaseDirMeas] = msfieldCol.phaseDir().getColumn();
    subtable[FRefDirMeas] = msfieldCol.referenceDir().getColumn();
    subtable[FSourceIndex] = msfieldCol.sourceId().getColumn();

    addRowFlag(subtable,msfieldCol.flagRow());
  }
  catch( AipsError &err )
  {
    dprintf(0)("error reading FIELD subtable: %s\n",err.getMesg().c_str());
    removeSubtable(rec,FFieldSubtable);
  }
}

//##ModelId=3DB9389000E4
void MSReader::readFeed (DataRecord &rec)
{
  try
  {
    MSFeed msfeed = ms_.feed();
    ROMSFeedColumns msfeedCol(msfeed);

    DataRecord &subtable = addSubtable(rec,FFeedSubtable);

    subtable[FAntennaIndex] = msfeedCol.antennaId().getColumn();
    subtable[FFeedIndex] = msfeedCol.feedId().getColumn();
    subtable[FSPWIndex] = msfeedCol.spectralWindowId().getColumn();
    subtable[FTime] = msfeedCol.time().getColumn();
    subtable[FInterval] = msfeedCol.interval().getColumn();

    subtable[FNumReceptors] = msfeedCol.numReceptors().getColumn();
    subtable[FBeamIndex] = msfeedCol.beamId().getColumn();
    subtable[FBeamOffset] = msfeedCol.beamOffset().getColumn();

    int nrow = msfeedCol.nrow();
    DataField *df = new DataField(Tpstring,nrow);
    subtable[FPolznType] <<= df;

    for( int i=0; i<nrow; i++ )
    {
      Vector<String> poltype;
      msfeedCol.polarizationType().get(i,poltype);
      string pol;
      for( uint j=0; j<poltype.nelements(); j++ )
        Debug::append(pol,poltype(j)); 
      (*df)[i] = pol;
    }    

    subtable[FPolznResponse] = msfeedCol.polResponse().getColumn();
    subtable[FPosition] = msfeedCol.position().getColumn();
    subtable[FReceptorAngle] = msfeedCol.receptorAngle().getColumn();
  }
  catch( AipsError &err )
  {
    dprintf(0)("error reading FEED subtable: %s\n",err.getMesg().c_str());
    removeSubtable(rec,FFeedSubtable);
  }
}

  //## end MSReader%3CF237080203.declarations
//## begin module%3CF2376F01E4.epilog preserve=yes
//## end module%3CF2376F01E4.epilog
