//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CD133DD02E0.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CD133DD02E0.cm

//## begin module%3CD133DD02E0.cp preserve=no
//## end module%3CD133DD02E0.cp

//## Module: MSIntegratorWP%3CD133DD02E0; Package body
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSIntegratorWP.cc

//## begin module%3CD133DD02E0.additionalIncludes preserve=no
//## end module%3CD133DD02E0.additionalIncludes

//## begin module%3CD133DD02E0.includes preserve=yes
#include <aips/Arrays/Vector.h>
#include <aips/MeasurementSets/MSColumns.h>
#include <aips/MeasurementSets/MSSpWindowColumns.h>
#include <aips/Mathematics/Complex.h>
#include <aips/Measures/Stokes.h>
#include <aips/Exceptions/Error.h>
#include <trial/MeasurementEquations/VisibilityIterator.h>
#include <trial/MeasurementEquations/VisBuffer.h>

#include "DMI/DataRecord.h"
#include "DMI/DataArray.h"
#include "DMI/AIPSPP-Hooks.h"

#include "UVD/MSReader.h"
//## end module%3CD133DD02E0.includes

// MSIntegratorWP
#include "UVD/MSIntegratorWP.h"
//## begin module%3CD133DD02E0.declarations preserve=no
//## end module%3CD133DD02E0.declarations

//## begin module%3CD133DD02E0.additionalDeclarations preserve=yes
using namespace UVD;

//## end module%3CD133DD02E0.additionalDeclarations


// Class MSIntegratorWP 

MSIntegratorWP::MSIntegratorWP (string msname, const HIID &act_msg, int nchan, int ntime, int npatch)
  //## begin MSIntegratorWP::MSIntegratorWP%3CD781CA01B8.hasinit preserve=no
  //## end MSIntegratorWP::MSIntegratorWP%3CD781CA01B8.hasinit
  //## begin MSIntegratorWP::MSIntegratorWP%3CD781CA01B8.initialization preserve=yes
    : WorkProcess(AidMSIntegratorWP),
      auto_ms(msname),auto_nchan(nchan),auto_ntime(ntime),auto_npatch(npatch),
      auto_activate(act_msg)
  //## end MSIntegratorWP::MSIntegratorWP%3CD781CA01B8.initialization
{
  //## begin MSIntegratorWP::MSIntegratorWP%3CD781CA01B8.body preserve=yes
  // if an MS was specified in the command line, publish it to
  // ourselves
  if( config.getOption("ms",auto_ms) || auto_ms.length() )
  {
    MessageRef mref(new Message(MSIntegrate),DMI::ANONWR);
    config.get("msichan",auto_nchan);
    config.get("msitime",auto_ntime);
    config.get("msipatches",auto_npatch);
  }
    // send to self
  //## end MSIntegratorWP::MSIntegratorWP%3CD781CA01B8.body
}


MSIntegratorWP::~MSIntegratorWP()
{
  //## begin MSIntegratorWP::~MSIntegratorWP%3CD133700076_dest.body preserve=yes
  //## end MSIntegratorWP::~MSIntegratorWP%3CD133700076_dest.body
}



//## Other Operations (implementation)
void MSIntegratorWP::init ()
{
  //## begin MSIntegratorWP::init%3CD133A303B9.body preserve=yes
  subscribe(MSIntegrate);
  // if running in demo mode (i.e. fixed ms), watch for a UVSorter to appear
  if( auto_ms.length() )
  {
    FailWhen( !auto_activate.size(),"auto-activation message must be specified" );
    dprintf(1)("predefined MS: %s, waiting for a %s\n",auto_ms.c_str(),
        auto_activate.toString().c_str());
    subscribe(auto_activate);
//    subscribe(MsgHello|AidUVSorterWP|AidWildcard);
  }
  //## end MSIntegratorWP::init%3CD133A303B9.body
}

bool MSIntegratorWP::start ()
{
  //## begin MSIntegratorWP::start%3CD1339B0349.body preserve=yes
  return False;
  //## end MSIntegratorWP::start%3CD1339B0349.body
}

int MSIntegratorWP::receive (MessageRef &mref)
{
  //## begin MSIntegratorWP::receive%3CD133AB011C.body preserve=yes
  if( auto_ms.length() && mref->id().matches(auto_activate) )
  {
    MessageRef mref( new Message(MSIntegrate),DMI::ANONWR );
    mref()["MS"] = auto_ms;
    mref()["Num.Channel"] = auto_nchan;
    mref()["Num.Time"] = auto_ntime;
    mref()["Num.Patch"] = auto_npatch;
    mref()["Flag.Ignore"] = False;
    send(mref,address());
    auto_ms = "";
  }
  else if( mref->id() == MSIntegrate )
  {
    MeasurementSet ms;
    try 
    {
      if( !initMS(mref.deref(),ms) )
        return Message::ACCEPT;

      // create iterator, visbuffer & chunk manager
      Block<Int> sortCol(1);
      sortCol[0] = MeasurementSet::TIME;
      ROVisibilityIterator vi(ms,sortCol,1000000000);
      VisBuffer vb(vi);

      // begin iterating over chunks -- each chunk has its own data description
      // ID, so we treat it as, essentially, a separate MS
      nchunk=0;
      for( vi.originChunks(); vi.moreChunks(); vi.nextChunk(),nchunk++ ) 
      {
        initSegment(vi);
        integrate(vi,vb);
      } // end of loop over chunks
      // send footer
      Message::Ref mref; 
      mref <<= new Message(AidUVData|msid|AidFooter|AidCorr|AidTimeslot); 
      publish(mref);
    }
    catch( const AipsError &err )
    {
      lprintf(0,LogError,"AIPS++ error: %s",err.getMesg().c_str());
    }
  } // end of MS processing
  return Message::ACCEPT;
  //## end MSIntegratorWP::receive%3CD133AB011C.body
}

// Additional Declarations
  //## begin MSIntegratorWP%3CD133700076.declarations preserve=yes
// opens and initializes ms based on init message    
bool MSIntegratorWP::initMS (const Message &msg,MeasurementSet &ms)
{
  msname = msg["MS"].as_string();
  window_chan = msg["Num.Channel"];
  window_time = msg["Num.Time"];
  num_patches = msg["Num.Patch"];
  ignore_flags = msg["Flag.Ignore"].as_bool(False);
  // open the measurement set
  ms = MeasurementSet(msname.c_str());
  
  // form a header with the required subtables
  MSReader reader(ms);
  DataRecord::Ref msheader(new DataRecord,DMI::ANONWR);
  reader.makeHeader(msheader.dewr());
  Message *hdrmsg = new Message(AidUVData|msid|AidHeader,msheader);
  Message::Ref mref; mref <<= hdrmsg;
  publish(mref);
  
  // obtain number of antennas and interferometers
  const MSAntenna msant( ms.antenna() );
  num_antennas = msant.nrow();
  num_ifrs = num_antennas*(num_antennas+1)/2; // cheap & dirty
//  ROScalarColumn<String> names(msant,"NAME");
//  antnames = names.getColumn();
//  antnames.apply(stringUpper);
  dprintf(1)("opened MS: %s, %d antennas\n",msname.c_str(),num_antennas);
  msid++;
// Create record of shared and/or constant datafields.
// These fields are shared between all patches and correlations.

  shrec_ref <<= shrec = new DataRecord;
  (*shrec)[FRowIndexing] = HIID("IFR"); // rows indexed by IFR
  (*shrec)[FTimeSlotIndex] = 0;
  (*shrec)[FTime] = 0.0;
  (*shrec)[FExposure] <<= new DataField(Tpdouble,num_ifrs);
  (*shrec)[FAntennaIndex] <<= new DataArray(TpArray_int,IPosition(2,2,num_ifrs));
  (*shrec)[FIFRIndex] <<= new DataField(Tpint,num_ifrs);
  (*shrec)[FSPWIndex] = 0;
  (*shrec)[FFieldIndex] = 0;
  (*shrec)[FFieldName] = string();
  (*shrec)[FNumIntTimes] <<= new DataField(Tpint,num_ifrs);
  (*shrec)[FUVW] <<= new DataArray(TpArray_double,IPosition(2,3,num_ifrs));
  (*shrec)[FRowFlag] <<= new DataField(Tpint,num_ifrs);

  pexp = &(*shrec)[FExposure];
  pnumtimes = &(*shrec)[FNumIntTimes];  
  puvw = &(*shrec)[FUVW];

  // fill in IFR/Antenna indexes since they are constant
  int *pifr = &(*shrec)[FIFRIndex],*pant = &(*shrec)[FAntennaIndex];
  for( int a1=0; a1<num_antennas; a1++ )
    for( int a2=0; a2<a1; a2++ )
    {
      int ifr = UVD::ifrNumber(a1,a2);
      pifr[ifr] = ifr;
      pant[ifr*2] = a2;
      pant[ifr*2+1] = a1;
    }

  return True;
}

// inits segment (a segment is defined by its unique DDI)
void MSIntegratorWP::initSegment (ROVisibilityIterator &vi)
{
  chunk_hiid = AidUVData|msid|nchunk;
  
  DataRecord *hrec;  
  href <<= (hrec = new DataRecord);
  
  (*hrec)[FUVSetIndex] = msid;
  (*hrec)[FSegmentIndex] = nchunk;
  (*hrec)[FFieldIndex] = vi.fieldId();
  (*hrec)[FFieldName] = vi.fieldName();
  (*hrec)[FSourceName] = vi.sourceName();
  (*hrec)[FSPWIndex] = vi.spectralWindow();
  // this tells the order in which chunks go out
  (*hrec)[FChunkIndexing] = AidCorr|AidTimeslot;

  num_times = vi.nSubInterval();
  num_integrated_times = num_times/window_time + 
      ( num_times%window_time != 0 );

  IPosition visshape = vi.visibilityShape();
  num_channels = visshape(1);
  num_integrated_channels = num_channels/window_chan + 
      ( num_channels%window_chan != 0 );

  (*hrec)[FNumTimeslots] = num_integrated_times;
  (*hrec)[FNumChannels] = num_integrated_channels;
  (*hrec)[FNumBaselines] = num_ifrs;
    
  // load correlation names
  Vector<Int> mscorrtypes;
  vi.corrType(mscorrtypes);
  
  num_corrs = mscorrtypes.nelements();
  corrtype.resize(num_corrs);
  recpt1.resize(num_corrs);
  recpt2.resize(num_corrs);
  
  DataField *corrs = new DataField(Tpint,num_corrs);
  (*hrec)[FCorr] <<= corrs;
  DataField *corrnames = new DataField(Tpstring,num_corrs);
  (*hrec)[FCorrName] <<= corrnames;
  
  string corrstr;
  for( int i=0; i<num_corrs; i++ )
  {
    Stokes::StokesTypes sttype = Stokes::type(mscorrtypes(i));
    (*corrs)[i] = corrtype[i] = sttype;
    corrstr += ( (*corrnames)[i] = Stokes::name(sttype) ) + ' ';
    recpt1[i] = Stokes::receptor1(sttype);
    recpt2[i] = Stokes::receptor2(sttype);
  }

  dprintf(1)("segment %d, spw %d, %d channels, %d times, correlations %s\n",
      nchunk,vi.spectralWindow(),num_channels,num_times,corrstr.c_str()); 

  // send out patch header message for every patch
  patch_header.resize(num_patches);
  for( int i=0; i<num_patches; i++ )
  {
    Message *msg = new Message(chunk_hiid|AidPatch|i|AidHeader|AidCorr|AidTimeslot);
    patch_header[i] <<= msg;
    // insert header record into message, and privatize it, non deeply
    ((*msg) <<= hrec).privatize(DMI::WRITE);
    (*msg)[FPatchIndex] = i;
    publish(patch_header[i].copy());
  }
  
  // setup shared fields
  (*shrec)[FUVSetIndex] = msid;
  (*shrec)[FSegmentIndex] = nchunk;
  (*shrec)[FSPWIndex] = vi.spectralWindow();
  (*shrec)[FFieldIndex] = vi.fieldId();
  (*shrec)[FFieldName] = vi.fieldName();
  
  // initialize separate accumulator record for each patch and correlation combo
  int nacc = num_patches*num_corrs;
  accrec0.resize(nacc);
  pdata.resize(nacc);
  pnumpixels.resize(nacc);
  int iacc = 0;
  for( int icorr = 0; icorr < num_corrs; icorr++ )
  {
    // create receptor index array based on antenna index array
    // (receptor index is antenna*2+receptor)
    DataArray *dreceptor = dynamic_cast<DataArray*>(
        (*shrec)[FAntennaIndex].as_DataArray_p()->clone(DMI::WRITE|DMI::DEEP));
    Assert(dreceptor);
    ObjRef receptor_ref;
    receptor_ref <<= dreceptor;
    int *precept = &(*dreceptor)[HIID()];
    for( int i=0; i<num_ifrs; i++ )
    {
      ( precept[2*i] *= 2 ) += recpt1[icorr];
      ( precept[2*i+1] *= 2 ) += recpt2[icorr];
    }
    // Create inital accumulator for every patch (based on the
    // shared record) and attach it to accrec0[iacc]
    for( int ipatch = 0; ipatch < num_patches; ipatch++,iacc++ )
    {
      // attach copy of the shared record and privatize (non-deeply)
      DataRecord &rec = accrec0[iacc].copy(shrec_ref).privatize(DMI::WRITE);
      // add fields specific to the patch/correlation
      rec[FCorr] = corrtype[icorr];
      rec[FReceptorIndex] = receptor_ref;
      rec[FData] <<= new DataArray(TpArray_fcomplex,IPosition(2,num_integrated_channels,num_ifrs),DMI::ZERO);
      rec[FNumIntPixels] <<= new DataArray(TpArray_int,IPosition(2,num_integrated_channels,num_ifrs),DMI::ZERO);
      pdata[iacc] = &rec[FData];
      if( Debug(2) && !iacc )
        cdebug(2)<<"init vis: "<<pdata[iacc][0]<<" "<<pdata[iacc][1]<<" "<<pdata[iacc][2]<<endl;
      pnumpixels[iacc] = &rec[FNumIntPixels];
    }
  }
}

// Send off chunk of integrated data (one integrated timeslot)
// and reset all accumulators
void MSIntegratorWP::finishIntegration (bool reset)
{
  // ignore if nothing has been accumulated
  if( !count_time_integrations )
    return;
  // store the average time
  (*shrec)[FTime] = integrated_time / count_time_integrations; 
  (*shrec)[FTimeSlotIndex] = integrated_timeslot;
  // average IFR-based data (exposures and UVW coordinates)
  for( int ifr=0; ifr < num_ifrs; ifr++ )
  {
    if( pnumtimes[ifr] )
    {
      pexp[ifr] /= pnumtimes[ifr];
      for( int j=0; j<3; j++ )
        puvw[ifr*3+j] /= pnumtimes[ifr];
      (*shrec)[FRowFlag][ifr] = 0;
    }
    else
      (*shrec)[FRowFlag][ifr] = FlagMissing;
  }
  //  send off chunks for each patch and correlation
  int num_points = num_ifrs*num_integrated_channels;
  int iacc=0;
  for( int icorr = 0; icorr < num_corrs; icorr++ )
  {
    for( int ipatch = 0; ipatch < num_patches; ipatch++,iacc++ )
    {
      DataRecord &rec = accrec0[iacc];
      rec[FPatchIndex] = ipatch;
      rec[FCorr] = corrtype[icorr];
      // average the visibilities
      ComplexType *pd = pdata[iacc];
      int *pn = pnumpixels[iacc];
      for( int j=0; j < num_points; j++ )
        if( pn[j] )
          pd[j] /= pn[j];
      if( Debug(2) && !iacc )
        cdebug(2)<<"avg. visibilities: "<<pd[0]<<" "<<pd[1]<<" "<<pd[2]<<endl;
      // NB: think about a privatize-and-reset implementation here
      // publish message
      MessageRef mref;
      Message &msg = mref <<= new Message(
          chunk_hiid|AidPatch|ipatch|AidData|
          AidCorr|AidTimeslot|corrtype[icorr]|integrated_timeslot);
      msg <<= &rec;
      publish(mref);
      // reset the data
      if( reset )
      {
        memset(pd,0,sizeof(*pd)*num_points);
        memset(pn,0,sizeof(*pn)*num_points);
      }
    }
  }
  // reset IFR-based data (exposures and UVW)
  if( reset )
  {
    memset(puvw,0,sizeof(*puvw)*num_ifrs*3);
    memset(pexp,0,sizeof(*pexp)*num_ifrs);
    memset(pnumtimes,0,sizeof(*pnumtimes)*num_ifrs);
  }
  // bump various counters 
  integrated_timeslot++;
  integrated_time = 0;
  count_time_integrations = 0;
}

// Main integration loop for one segment
void MSIntegratorWP::integrate (ROVisibilityIterator &vi,VisBuffer &vb)
{
  integrated_timeslot = 0;
  integrated_time = 0;
  count_time_integrations = 0;
  
  int itime=0,irow=0;
  for( vi.origin(); vi.more(); vi++,itime++ )
  {
    dprintf(1)("time slot %d, %d rows\n",itime,vb.nRow()); 
    // if we got to next window, send off chunk
    if( itime && !(itime%window_time) )
      finishIntegration();
    // process all data in this visbuffer (this time slot)
    integrated_time += vb.time()(0);
    count_time_integrations++;
    const Vector<Int> &ant1 = vb.antenna1(),&ant2 = vb.antenna2();
    const Vector<Bool> &flagrow = vb.flagRow();
    const Cube<Bool> &flagcube = vb.flagCube();
    const Cube<Complex> &viscube = vb.visCube();
    const Vector<RigidVector<Double,3> > &uvw = vb.uvw();
    int nrows = vb.nRow();
    for( int i=0; i<nrows; i++,irow++ )
    {
      // keep the dsp running
      if( !dsp()->yield() )
        return;
      // honor row flags by ignoring whole row
      if( flagrow(i) )
        continue;
      int ifr = ifrNumber(ant1(i),ant2(i));
      pnumtimes[ifr]++;
      // acumulate exposure
      //      (VisIter won't give us exposure time -- so assume 30)
      pexp[ifr] += 30; 
      // accumulate UVW
      for( int j=0; j<3; j++ )
        puvw[ifr*3+j] += uvw(i)(j);
      // integrate visibilities over channels, honoring flags
      bool del_vis,del_flag;
      Matrix<Complex> visplane = viscube.xyPlane(i);
      const Complex *pvisplane = visplane.getStorage(del_vis);
      Matrix<Bool>    flagplane = flagcube.xyPlane(i);
      const Bool *pflagplane = flagplane.getStorage(del_flag);
      
      int iacc=0;
      for( int icorr=0; icorr < num_corrs; icorr++ )
      {
        for( int ipatch = 0; ipatch < num_patches; ipatch++,iacc++ )
        {
          const Complex *pvp0 = pvisplane + icorr;
          const Bool *pfp0 = pflagplane + icorr;
          ComplexType *pdata0 = pdata[iacc];
          int *pnp0 = pnumpixels[iacc];
          for( int ichan = 0; ichan < num_integrated_channels; ichan++ )
          {
            int window = window_chan;
            // last channel window is smaller
            if( ichan == num_integrated_channels-1 && num_channels%window_chan )
              window = num_channels%window_chan;
            int j0 = ifr*num_integrated_channels+ichan;
            ComplexType &pd = pdata0[j0];
            int &pn = pnp0[j0];
            for( int j=0; j<window; j++,pvp0+=num_corrs,pfp0+=num_corrs )
              if( ignore_flags || !*pfp0 )
              {
                pd += *pvp0;
                pn++;
              }
          }
        } // end of loop over patches
      } // end of loop over correlations
      visplane.freeStorage(pvisplane,del_vis);
      flagplane.freeStorage(pflagplane,del_flag);
    } // end of loop over rows in this time slot
    
    // do a yield call to keep the system running
    // yield();
  } // end of loop over time slots
  // send off final chunk, if any
  finishIntegration(False);
  // send off footers
  patch_footer.resize(num_patches);
  for( int i=0; i<num_patches; i++ )
  {
    Message *msg = new Message(chunk_hiid|AidPatch|i|AidFooter|AidCorr|AidTimeslot);
    patch_footer[i] <<= msg;
    (*msg) <<= patch_header[i]().payload().copy().privatize(DMI::WRITE);
    publish(patch_footer[i].copy());
  }
}


  //## end MSIntegratorWP%3CD133700076.declarations
//## begin module%3CD133DD02E0.epilog preserve=yes
//## end module%3CD133DD02E0.epilog
