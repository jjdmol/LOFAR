//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CD79DB900E7.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CD79DB900E7.cm

//## begin module%3CD79DB900E7.cp preserve=no
//## end module%3CD79DB900E7.cp

//## Module: UVSorterWP%3CD79DB900E7; Package body
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\UVSorterWP.cc

//## begin module%3CD79DB900E7.additionalIncludes preserve=no
//## end module%3CD79DB900E7.additionalIncludes

//## begin module%3CD79DB900E7.includes preserve=yes
#include <algorithm>
#include "UVD/UVD.h"
#include "DMI/DataRecord.h"
#include "DMI/DataArray.h"
#include "DMI/AIPSPP-Hooks.h"
//## end module%3CD79DB900E7.includes

// UVSorterWP
#include "UVD/UVSorterWP.h"
//## begin module%3CD79DB900E7.declarations preserve=no
//## end module%3CD79DB900E7.declarations

//## begin module%3CD79DB900E7.additionalDeclarations preserve=yes
using namespace UVD;
//## end module%3CD79DB900E7.additionalDeclarations


// Class UVSorterWP 

UVSorterWP::UVSorterWP (int ipatch, int icorr)
  //## begin UVSorterWP::UVSorterWP%3CD7CF6E020D.hasinit preserve=no
  //## end UVSorterWP::UVSorterWP%3CD7CF6E020D.hasinit
  //## begin UVSorterWP::UVSorterWP%3CD7CF6E020D.initialization preserve=yes
    : WorkProcess(AidUVSorterWP),
    mypatch(ipatch),mycorr(icorr)
  //## end UVSorterWP::UVSorterWP%3CD7CF6E020D.initialization
{
  //## begin UVSorterWP::UVSorterWP%3CD7CF6E020D.body preserve=yes
  //## end UVSorterWP::UVSorterWP%3CD7CF6E020D.body
}


UVSorterWP::~UVSorterWP()
{
  //## begin UVSorterWP::~UVSorterWP%3CD79D3D0227_dest.body preserve=yes
  //## end UVSorterWP::~UVSorterWP%3CD79D3D0227_dest.body
}



//## Other Operations (implementation)
void UVSorterWP::init ()
{
  //## begin UVSorterWP::init%3CD79D680391.body preserve=yes
  HIID id = HIID("UVData.?.?.Patch") | mypatch;
  
  header_hiid = id | AidHeader | AidCorr | AidTimeslot;
  chunk_hiid = id | AidData | AidCorr | AidTimeslot | mycorr | AidAny;
  footer_hiid = id | AidFooter | AidCorr | AidTimeslot;
  
  subscribe(header_hiid);
  subscribe(chunk_hiid);
  subscribe(footer_hiid);

  setState(IDLE);  
  uvset_id = segment_id = -1;
  //## end UVSorterWP::init%3CD79D680391.body
}

int UVSorterWP::receive (MessageRef &mref)
{
  //## begin UVSorterWP::receive%3CD79D7301B7.body preserve=yes
  const Message &msg = mref.deref();
  if( msg.id().matches(header_hiid) )
  {
    // check that header is intended for us
    int ncorr;
    const int *pcorr = &msg[FCorr].size(ncorr);
    if( !find(pcorr,pcorr+ncorr,mycorr) || msg[FPatchIndex].as_int() != mypatch )
    {
      lprintf(1,LogWarning,"ignoring %s: patch # or correlation type mismatch",msg.id().toString().c_str());
      return Message::ACCEPT;
    }
    // are we already receiving?
    if( uvset_id >= 0 )
      lprintf(1,LogWarning,"discarding unfinished segment %d:%d",uvset_id,segment_id);
    setState(SORTING);
    header_ref.copy(msg.payload()).privatize(DMI::WRITE);
    const DataRecord &hdr = 
        dynamic_cast<const DataRecord&>(header_ref.deref());
    uvset_id = hdr[FUVSetIndex];
    segment_id = hdr[FSegmentIndex];
    num_ifrs = hdr[FNumBaselines];
    num_times = hdr[FNumTimeslots];
    num_channels = hdr[FNumChannels];
    
    // init record for per-IFR accumulators
    DataRecord &rec = prec_template_ref <<= new DataRecord;
    rec[FTimeSlotIndex] <<= new DataField(Tpint,num_times);
    rec[FTime] <<= new DataField(Tpdouble,num_times);
    
    rec[FCorr] = mycorr;
    rec[FSPWIndex] = hdr[FSPWIndex].as_int();
    rec[FFieldIndex] = hdr[FFieldIndex].as_int();
    rec[FFieldName] = hdr[FFieldName].as_string();

    rec[FExposure] <<= new DataField(Tpdouble,num_times);
    rec[FNumIntTimes] <<= new DataField(Tpint,num_times);
    rec[FUVW] <<= new DataArray(Tpdouble,IPosition(2,3,num_times));
    rec[FData] <<= new DataArray(Tpfcomplex,IPosition(2,num_channels,num_times));
    rec[FNumIntPixels] <<= new DataArray(Tpint,IPosition(2,num_channels,num_times));
    
    dprintf(1)("got header for %d:%d (spw %d, field %s): %d IFRs, %d channels, %d times\n",
        uvset_id,segment_id,rec[FSPWIndex].as_int(),rec[FFieldName].as_string().c_str(),
        num_ifrs,num_channels,num_times);
    
    // init accumulators (empty & unattached, until data for
    // that IFR actually gets there)
    prec.resize(num_ifrs);
    prec_ref.resize(num_ifrs);
    pdata.resize(num_ifrs);
    pnumpoints.resize(num_ifrs);
    for( int i=0; i<num_ifrs; i++ )
      prec_ref[i].detach();
    
    ts_header = Timestamp::now();
    vis_count = 0;
  }
  else if( msg.id().matches(chunk_hiid) )
  {
    if( uvset_id<0 )
    {
      dprintf(1)("no header yet, ignoring data chunk\n");
      return Message::ACCEPT;
    }
    // check for matching data id
    if( msg[FUVSetIndex].as_int() != uvset_id || msg[FSegmentIndex].as_int() != segment_id )
    {
      dprintf(1)("mismatch in uvset or segment id, ignoring data chunk\n");
      return Message::ACCEPT;
    }
    // process a data chunk
    const DataRecord &rec = dynamic_cast<const DataRecord &>(msg.payload().deref());
    if( rec[FPatchIndex].as_int() != mypatch || rec[FCorr].as_int() != mycorr )
    {
      lprintf(1,LogWarning,"ignoring %s: patch # or correlation type mismatch",msg.id().toString().c_str());
      return Message::ACCEPT;
    }
    IPosition visshape = rec[FData].as_Array_fcomplex().shape();
    FailWhen(visshape(0) != num_channels,"mismatch in number of channels");
    FailWhen(visshape(1) != num_ifrs,"mismatch in number of baselines");

    // direct pointers into the visibility and numpoints planes of the
    // source data, iterated over by the loop
    const fcomplex *pvd0 = &rec[FData];     
    const int *pnp0 = &rec[FNumIntPixels];          
    int itime = rec[FTimeSlotIndex];
    double timeval = rec[FTime];
    dprintf(1)("got data chunk for time slot %d\n",itime);
    // loop over all baselines in this chunk
    for( int ifr = 0; ifr < num_ifrs; ifr++,pvd0+=num_channels,pnp0+=num_channels )
    {
      if( rec[FNumIntTimes][ifr].as_int() != 0 )
      {
        FailWhen(itime>=num_times,"unexpected time slot index");
        // create accumulator for this IFR, if not initialized yet
        if( !prec_ref[ifr].valid() )
        {
          prec_ref[ifr].copy(prec_template_ref).privatize(DMI::WRITE|DMI::DEEP);
          prec[ifr] = dynamic_cast<DataRecord*>(prec_ref[ifr].dewr_p());
          Assert(prec[ifr]); 
          // initialize scalar fields
          DataRecord & acc = *prec[ifr];
          acc[FIFRIndex] = ifr;
          acc[FAntennaIndex] = rec[FAntennaIndex](0,ifr).as_int();
          acc[FAntennaIndex][1] = rec[FAntennaIndex](1,ifr).as_int();
          acc[FReceptorIndex] = rec[FReceptorIndex](0,ifr).as_int();
          acc[FReceptorIndex][1] = rec[FReceptorIndex](1,ifr).as_int();
        }
        // use hooks to xfer most of the data
        DataRecord & acc = *prec[ifr];
        acc[FTimeSlotIndex][itime] = itime;
        acc[FTime][itime] = timeval;
        acc[FExposure][itime] = rec[FExposure][ifr].as_double();
        acc[FNumIntTimes][itime] = rec[FNumIntTimes][ifr].as_int();
        for( int j=0; j<3; j++ )
          acc[FUVW](j,itime) = rec[FUVW](j,ifr).as_double();
        // use memcpy to transfer Data and NumIntPixels vectors
        fcomplex *pvd = &acc[FData];
        int *pnp = &acc[FNumIntPixels];
        memcpy(pvd+itime*num_channels,pvd0,sizeof(*pvd)*num_channels);
        memcpy(pnp+itime*num_channels,pnp0,sizeof(*pnp)*num_channels);
        vis_count += num_channels;
      }
    } // end of loop over baselines in chunk
  } // end if received chunk
  else if( msg.id().matches(footer_hiid) )
  {
    if( uvset_id<0 )
    {
      dprintf(1)("no header yet, ignoring footer\n");
    }
    else if( msg[FUVSetIndex].as_int() != uvset_id || msg[FSegmentIndex].as_int() != segment_id )
    {
      dprintf(1)("mismatch in uvset or segment id, ignoring footer\n");
    }
    else
    {
      dprintf(1)("got footer for %d:%d\n",uvset_id,segment_id);
      double diff = Timestamp::now() - ts_header;
      lprintf(1,"%.2f seconds elapsed for patch %d:%d:%d\n",diff,uvset_id,segment_id,mypatch);
      lprintf(1,"%d visibilities (%g/s)\n",vis_count,vis_count/diff);
      // publish header
      MessageRef mref;
      mref <<= new Message(
        AidUVData|uvset_id|segment_id|AidPatch|mypatch|AidHeader|AidCorr|AidIFR,
        header_ref.copy());
      publish(mref);
      // publish accumulated chunks
      int num_pub=0;
      for( int ifr = 0; ifr < num_ifrs; ifr ++ )
      {
        if( prec_ref[ifr].valid() )
        {
          mref <<= new Message(
            AidUVData|uvset_id|segment_id|AidPatch|mypatch|AidData|AidCorr|AidIFR|mycorr|ifr,
            prec_ref[ifr]);
          publish(mref);
          num_pub++;
        }
      }
      lprintf(1,"publishing re-sorted data for %d of %d baselines",num_pub,num_ifrs);
      // publish footer
      mref <<= new Message(
        AidUVData|uvset_id|segment_id|AidPatch|mypatch|AidFooter|AidCorr|AidIFR,
        header_ref);
      publish(mref);
      // clear status
      uvset_id = segment_id = -1;
      setState(IDLE);
    }
  }
  else 
    dprintf(1)("ignoring unrecognized message: %s\n",msg.sdebug(1).c_str());
  return Message::ACCEPT;
  //## end UVSorterWP::receive%3CD79D7301B7.body
}

// Additional Declarations
  //## begin UVSorterWP%3CD79D3D0227.declarations preserve=yes
  //## end UVSorterWP%3CD79D3D0227.declarations

//## begin module%3CD79DB900E7.epilog preserve=yes
//## end module%3CD79DB900E7.epilog
