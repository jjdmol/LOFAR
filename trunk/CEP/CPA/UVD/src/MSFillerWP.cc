//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CEA390F0258.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CEA390F0258.cm

//## begin module%3CEA390F0258.cp preserve=no
//## end module%3CEA390F0258.cp

//## Module: MSFillerWP%3CEA390F0258; Package body
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSFillerWP.cc

//## begin module%3CEA390F0258.additionalIncludes preserve=no
//## end module%3CEA390F0258.additionalIncludes

//## begin module%3CEA390F0258.includes preserve=yes
//## end module%3CEA390F0258.includes

// MSFillerWP
#include "UVD/MSFillerWP.h"
//## begin module%3CEA390F0258.declarations preserve=no
//## end module%3CEA390F0258.declarations

//## begin module%3CEA390F0258.additionalDeclarations preserve=yes
#include "UVD/UVD.h"
//## end module%3CEA390F0258.additionalDeclarations


// Class MSFillerWP 

MSFillerWP::MSFillerWP()
  //## begin MSFillerWP::MSFillerWP%3CEA38B303B4_const.hasinit preserve=no
  //## end MSFillerWP::MSFillerWP%3CEA38B303B4_const.hasinit
  //## begin MSFillerWP::MSFillerWP%3CEA38B303B4_const.initialization preserve=yes
  : WorkProcess(AidMSFillerWP)
  //## end MSFillerWP::MSFillerWP%3CEA38B303B4_const.initialization
{
  //## begin MSFillerWP::MSFillerWP%3CEA38B303B4_const.body preserve=yes
  //## end MSFillerWP::MSFillerWP%3CEA38B303B4_const.body
}


MSFillerWP::~MSFillerWP()
{
  //## begin MSFillerWP::~MSFillerWP%3CEA38B303B4_dest.body preserve=yes
  //## end MSFillerWP::~MSFillerWP%3CEA38B303B4_dest.body
}



//## Other Operations (implementation)
void MSFillerWP::setHeader (const HIID &id)
{
  //## begin MSFillerWP::setHeader%3CEA38D802A9.body preserve=yes
  if( isRunning() )
    unsubscribe(hdr_id);
  hdr_id = id;
  if( isRunning() )
    subscribe(hdr_id);
  //## end MSFillerWP::setHeader%3CEA38D802A9.body
}

void MSFillerWP::setSegmentHeader (const HIID& id)
{
  //## begin MSFillerWP::setSegmentHeader%3CF60D490192.body preserve=yes
  if( isRunning() )
    unsubscribe(chunk_hdr_id);
  chunk_hdr_id = id;
  if( isRunning() )
    subscribe(chunk_hdr_id);
  //## end MSFillerWP::setSegmentHeader%3CF60D490192.body
}

void MSFillerWP::setChunk (const HIID& id)
{
  //## begin MSFillerWP::setChunk%3CF60D5602F0.body preserve=yes
  if( isRunning() )
    unsubscribe(chunk_id);
  chunk_id = id;
  if( isRunning() )
    subscribe(chunk_id);
  //## end MSFillerWP::setChunk%3CF60D5602F0.body
}

void MSFillerWP::setFooter (const HIID &id)
{
  //## begin MSFillerWP::setFooter%3CF6215402A9.body preserve=yes
  if( isRunning() )
    unsubscribe(footer_id);
  footer_id = id;
  if( isRunning() )
    subscribe(footer_id);
  //## end MSFillerWP::setFooter%3CF6215402A9.body
}

void MSFillerWP::setMSName (const string &ms)
{
  //## begin MSFillerWP::setMSName%3CF60D670394.body preserve=yes
  msname = ms;
  //## end MSFillerWP::setMSName%3CF60D670394.body
}

void MSFillerWP::init ()
{
  //## begin MSFillerWP::init%3CEA38C50374.body preserve=yes
  FailWhen( !hdr_id.size() || !chunk_hdr_id.size() || !chunk_id.size(),
      "message HIIDs not set up");
  subscribe(hdr_id);
  subscribe(chunk_hdr_id);
  subscribe(chunk_id);
  subscribe(footer_id);
  setState(IDLE);
  //## end MSFillerWP::init%3CEA38C50374.body
}

int MSFillerWP::receive (MessageRef &mref)
{
  //## begin MSFillerWP::receive%3CEA38CD00E0.body preserve=yes
  if( mref->id().matches(hdr_id) )
  {
    dprintf(1)("got MS header: %s\n",mref->sdebug(1).c_str());
    if( state() == DATA )
    {
      lprintf(1,LogWarning,"already filling ms %s, ignoring MS header %s",
          current_ms.c_str(),mref->id().toString().c_str());
      return Message::ACCEPT;
    }
    // figure out name for output MS
    string aipsname;
    const DataRecord &rec = mref->record();
    if( rec[UVD::FAipsMSName].exists() )
    {
      aipsname = rec["AIPSPP.MS.Name"].as_string();
    }
    if( !msname.length() )
    {
      if( !aipsname.length() )
      {
        lprintf(1,LogError,"no MS name specified and none found in header %s",current_ms.c_str());
        return Message::ACCEPT;
      }
      current_ms = aipsname;
    }
    else
    {
      current_ms = msname;
      uint pos = current_ms.find("%M");
      if( pos != string::npos )
      {
        // trim extension from original name and insert instead of "%M"
        uint pos1 = aipsname.rfind('.');
        if( pos1 != string::npos )
          aipsname.replace(pos1,string::npos,"");
        current_ms.replace(pos,2,aipsname);
      }
    }
    lprintf(1,"creating ms %s",current_ms.c_str());
    filler.create(current_ms,rec);
    setState(DATA);
  }
  else if( mref->id().matches(chunk_hdr_id) )
  {
    dprintf(2)("got segment header: %s\n",mref->sdebug(1).c_str());
    if( state() != DATA )
    {
      lprintf(1,LogWarning,"no MS header yet, ignoring segment header %s",mref->id().toString().c_str());
      return Message::ACCEPT;
    }
    filler.endSegment();
    filler.startSegment(mref->record());
  }
  else if( mref->id().matches(chunk_id) )
  {
    dprintf(2)("got data chunk: %s\n",mref->sdebug(1).c_str());
    if( state() != DATA )
    {
      lprintf(1,LogWarning,"no MS header yet, ignoring chunk %s",mref->sdebug(1).c_str());
      return Message::ACCEPT;
    }
    filler.addChunk(mref->record());
  }
  else if( mref->id().matches(footer_id) )
  {
    dprintf(1)("got MS footer: %s\n",mref->sdebug(1).c_str());
    if( state() != DATA )
    {
      lprintf(1,LogWarning,"no MS header yet, ignoring footer %s",mref->id().toString().c_str());
      return Message::ACCEPT;
    }
    lprintf(1,"closing ms %s",current_ms.c_str());
    filler.close();
    setState(IDLE);
  }
  
  return Message::ACCEPT;
  //## end MSFillerWP::receive%3CEA38CD00E0.body
}

// Additional Declarations
  //## begin MSFillerWP%3CEA38B303B4.declarations preserve=yes
  //## end MSFillerWP%3CEA38B303B4.declarations

//## begin module%3CEA390F0258.epilog preserve=yes
//## end module%3CEA390F0258.epilog
