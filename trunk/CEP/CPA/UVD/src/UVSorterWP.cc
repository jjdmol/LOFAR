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
      lprintf(1,"ignoring %s: patch # or correlation type mismatch",msg.id().toString().c_str());
      return Message::ACCEPT;
    }
    uvset_id = msg[FUVSetIndex];
    segment_id = msg[FSegmentIndex];
    num_ifrs = msg[FNumBaselines];
    num_times = msg[FNumTimeslots];
    num_channels = msg[FNumChannels];
    dprintf(1)("got header for %d:%d, %d baselines, %d channels, %d times\n",
        uvset_id,segment_id,num_ifrs,num_channels,num_times);
    // init record for per-IFR accumulators
    DataRecord &rec = prec_template_ref <<= new DataRecord;
    rec[FTimeSlotIndex] <<= new DataField(Tpint,num_times);
    rec[FTime] <<= new DataField(Tpdouble,num_times);
    
    rec[FCorr] = mycorr;
    rec[FSPWIndex] = msg[FSPWIndex].as_int();
    rec[FFieldIndex] = msg[FFieldIndex].as_int();
    rec[FFieldName] = msg[FFieldName].as_string();
    
    rec[FExposure] <<= new DataField(Tpdouble,num_times);
    rec[FNumIntTimes] <<= new DataField(Tpint,num_times);
    rec[FUVW] <<= new DataArray(Tpdouble,IPosition(2,3,num_times));
    rec[FData] <<= new DataArray(Tpdcomplex,IPosition(2,num_channels,num_times));
    rec[FNumIntPixels] <<= new DataArray(Tpint,IPosition(2,num_channels,num_times));
    // init accumulators (initially empty, until data for
    // that IFR actually gets there)
    prec.resize(num_ifrs);
    prec_ref.resize(num_ifrs);
    pdata.resize(num_ifrs);
    pnumpoints.resize(num_ifrs);
    for( int i=0; i<num_ifrs; i++ )
      prec_ref[i].detach();
  }
  else if( msg.id().matches(chunk_hiid) )
  {
    // process a data chunk
    const DataRecord &rec = dynamic_cast<const DataRecord &>(msg.payload().deref());
    if( rec[FPatchIndex].as_int() != mypatch || rec[FCorr].as_int() != mycorr )
    {
      lprintf(1,"ignoring %s: patch # or correlation type mismatch",msg.id().toString().c_str());
      return Message::ACCEPT;
    }
    IPosition visshape = rec[FData].as_Array_dcomplex().shape();
    FailWhen(visshape(0) != num_channels,"mismatch in number of channels");
    FailWhen(visshape(1) != num_ifrs,"mismatch in number of baselines");

    // direct pointers into the visibility and numpoints planes of the
    // source data, iterated over by the loop
    const dcomplex *pvd0 = &rec[FData];     
    const int *pnp0 = &rec[FNumIntPixels];          
    int itime = rec[FTimeSlotIndex];
    double timeval = rec[FTime];
    dprintf(1)("got chunk for time slot %d\n",itime);
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
        {
          double x = rec[FUVW](j,ifr);
          acc[FUVW](j,itime) = x;
        }
        // use memcpy to transfer Data and NumIntPixels vectors
        dcomplex *pvd = &acc[FData];
        int *pnp = &acc[FNumIntPixels];
        memcpy(pvd+itime*num_channels,pvd0,sizeof(*pvd)*num_channels);
        memcpy(pnp+itime*num_channels,pnp0,sizeof(*pnp)*num_channels);
      }
      // have we now completely filled in this baseline?
      if( itime == num_times-1 )
      {
        if( prec_ref[ifr].valid() )
        {
          lprintf(1,"publishing data for patch %d, corr %d, baseline %d",
              mypatch,mycorr,ifr);
          MessageRef mref;
          Message &msg = mref <<= new Message(
            AidUVData|uvset_id|segment_id|AidPatch|mypatch|AidCorr|AidIFR|mycorr|ifr);
          msg <<= prec_ref[ifr];
          publish(mref);
        }
        else
        {
          lprintf(1,"no data accumulated for patch %d, corr %d, baseline %d",
              mypatch,mycorr,ifr);
        }
      }
    } // end of loop over baselines in chunk
  } // end if received chunk
  return Message::ACCEPT;
  //## end UVSorterWP::receive%3CD79D7301B7.body
}

// Additional Declarations
  //## begin UVSorterWP%3CD79D3D0227.declarations preserve=yes
  //## end UVSorterWP%3CD79D3D0227.declarations

//## begin module%3CD79DB900E7.epilog preserve=yes
//## end module%3CD79DB900E7.epilog
