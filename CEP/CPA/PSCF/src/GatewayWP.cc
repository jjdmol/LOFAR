//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C90BFDD0240.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C90BFDD0240.cm

//## begin module%3C90BFDD0240.cp preserve=no
//## end module%3C90BFDD0240.cp

//## Module: GatewayWP%3C90BFDD0240; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\GatewayWP.cc

//## begin module%3C90BFDD0240.additionalIncludes preserve=no
//## end module%3C90BFDD0240.additionalIncludes

//## begin module%3C90BFDD0240.includes preserve=yes
//## end module%3C90BFDD0240.includes

// GatewayWP
#include "GatewayWP.h"
//## begin module%3C90BFDD0240.declarations preserve=no
//## end module%3C90BFDD0240.declarations

//## begin module%3C90BFDD0240.additionalDeclarations preserve=yes
// all packet headers must start with this signature
static const char PacketSignature[] = "oMs";
//## end module%3C90BFDD0240.additionalDeclarations


// Class GatewayWP 

GatewayWP::GatewayWP (Socket* sk)
  //## begin GatewayWP::GatewayWP%3C95C53D00AE.hasinit preserve=no
  //## end GatewayWP::GatewayWP%3C95C53D00AE.hasinit
  //## begin GatewayWP::GatewayWP%3C95C53D00AE.initialization preserve=yes
  : WorkProcess(AidGatewayWP),sock(sk)
  //## end GatewayWP::GatewayWP%3C95C53D00AE.initialization
{
  //## begin GatewayWP::GatewayWP%3C95C53D00AE.body preserve=yes
  memcpy(wr_header.signature,PacketSignature,sizeof(wr_header.signature));
  //## end GatewayWP::GatewayWP%3C95C53D00AE.body
}


GatewayWP::~GatewayWP()
{
  //## begin GatewayWP::~GatewayWP%3C90BEF001E5_dest.body preserve=yes
  if( sock )
    delete sock;
  if( read_block )
    delete read_block;
  //## end GatewayWP::~GatewayWP%3C90BEF001E5_dest.body
}



//## Other Operations (implementation)
void GatewayWP::start ()
{
  //## begin GatewayWP::start%3C90BF460080.body preserve=yes
  WorkProcess::start();
  // tell dispatcher that we can forward messages
  dsp()->declareForwarder(this);
  // handle & ignore SIGURG -- out-of-band data on socket. 
  // addInput() will catch an exception on the fd anyway
  addSignal(SIGURG,EV_IGNORE);
  // ignore SIGPIPE, but maintain Socket's sigpipe counter
  addSignal(SIGPIPE,EV_IGNORE,&sock->sigpipe_counter);
  // start watching the socket for incoming data
  addInput(sock->getSid(),EV_FDREAD|EV_FDEXCEPTION);
  write_seq = 0;
  read_junk = 0;
  writestate = IDLE;
  readyForHeader();
  //## end GatewayWP::start%3C90BF460080.body
}

void GatewayWP::stop ()
{
  //## begin GatewayWP::stop%3C90BF4A039D.body preserve=yes
  if( sock )
    delete sock;
  sock = 0;
  // got any loose blocks?
  if( read_block )
    delete read_block;
  read_block = 0;
  read_bset.clear();
  write_queue.clear();
  //## end GatewayWP::stop%3C90BF4A039D.body
}

bool GatewayWP::willForward (const Message &msg) const
{
  //## begin GatewayWP::willForward%3C90BF5C001E.body preserve=yes
  // For now, forward all messages of local origin, since
  // we're doing a simple everybody-connects-to-everybody topology.
  // This avoid loops, etc.
  return msg.from().host() == address().host() &&
         msg.from().process() == address().process();
  //## end GatewayWP::willForward%3C90BF5C001E.body
}

int GatewayWP::receive (MessageRef& mref)
{
  //## begin GatewayWP::receive%3C90BF63005A.body preserve=yes
  // ignore any stray messages to myself
  if( mref->to() == address() )
    return Message::ACCEPT;
  
  if( writestate != IDLE )   // writing something? 
  {
    // hold off we already have a pending message
    if( pending_msg.valid() )  
      return Message::HOLD; 
    pending_msg.xfer(mref);
  }
  else // no, write state is idle, so start sending
  {
    prepareMessage(mref);
    addInput(sock->getSid(),EV_FDWRITE);  // enable write input
  }
  return Message::ACCEPT;
  //## end GatewayWP::receive%3C90BF63005A.body
}

int GatewayWP::timeout (const HIID &id)
{
  //## begin GatewayWP::timeout%3C90BF6702C3.body preserve=yes
  // NB: handle pings and such
  return Message::ACCEPT;
  //## end GatewayWP::timeout%3C90BF6702C3.body
}

int GatewayWP::input (int fd, int flags)
{
  //## begin GatewayWP::input%3C90BF6F00ED.body preserve=yes
  // first handle out-of-band messages
  if( flags&EV_FDEXCEPTION ) 
  {
    // to be implemented
  }
  // then handle writing
  if( flags&EV_FDWRITE )
  {
    // write is idle? disable the write input
    if( writestate == IDLE )
    {
      removeInput(sock->getSid(),EV_FDWRITE);
      return Message::ACCEPT;
    }
    // write data from current block
    int n = sock->write(write_buf + nwritten,write_buf_size - nwritten);
    if( n < 0 )
    {
      // on write error, just commit harakiri. GWClient/ServerWP will
      // take care of reopening a connection, eventually
      dprintf(1)("socket read error: %s. Aborting.",sock->errstr().c_str());
      detachMyself();
      return Message::CANCEL; 
    }
    else if( n > 0 )
      Timestamp::now(&last_write);
    // update checksum and advance the nread pointer
    for( ; n>0; n--,nwritten++ )
      write_checksum += write_buf[nwritten];
    // if buffer incomplete, then wait for next event
    if( nwritten < write_buf_size )
      return Message::ACCEPT;        
    nwritten = 0;
    // chunk written, advance to next one?
    if( writestate == HEADER ) 
    {
      if( !write_queue.size() ) // were sending header but queue is empty
      {
        // make sure we haven't been sending a data header
        FailWhen(wr_header.type == MT_DATA,"write queue empty after data header");
        writestate = IDLE;
      }
      else // were sending header and queue is not empty, must be data header then
      {
        FailWhen(wr_header.type != MT_DATA,"write queue unexpectedly not empty");
        prepareData(); 
      }
    }
    else if( writestate == BLOCK ) 
    {
      prepareTrailer();        // after block, send trailer
    }
    else if( writestate == TRAILER ) 
    {
      if( write_queue.size() ) // something else in queue? send next header
        prepareHeader();
      else
        writestate = IDLE;
    }
    // have we cvhanged state to IDLE? 
    if( writestate == IDLE )
    {
      if( pending_msg.valid() )        // send pending message, if any
        prepareMessage(pending_msg);
      else                             // else disable the write input
        removeInput(sock->getSid(),EV_FDWRITE);
    }
  }
  // now handle reading
  if( flags&EV_FDREAD )
  {
    // read up to full buffer
    int n = sock->read(read_buf + nread,read_buf_size - nread);
    if( n < 0 )
    {
      // on read error, just commit harakiri. GWClient/ServerWP will
      // take care of reopening a connection, eventually
      dprintf(1)("socket read error: %s. Aborting.",sock->errstr().c_str());
      detachMyself();
      return Message::CANCEL; 
    }
    else if( n > 0 )
      Timestamp::now(&last_read);
    // update checksum and advance the nread pointer
    for( ; n>0; n--,nread++ )
      read_checksum += read_buf[nread];
    // if buffer incomplete, then wait for next event
    if( nread < read_buf_size )
      return Message::ACCEPT;        
    nread = 0;
    // else we have a complete buffer, so dispose of it according to mode
    if( readstate == HEADER ) // got a packet header?
    {
      if( memcmp(header.signature,PacketSignature,sizeof(header.signature)) ||
          header.type > MT_MAXTYPE )
      {
        // invalid header -- flush it
        if( !read_junk )
        {
          dprintf(2)("error: junk data before header\n");
          requestResync();  // request resync if this is first instance
        }
        // look for first byte of signature in this header
        void *pos = memchr(&header,PacketSignature[0],sizeof(header));
        if( !pos ) // not found? flush everything
        {
          nread = 0;
          read_junk += sizeof(header);
        }
        else
        { // else flush everything up to matching byte
          int njunk = static_cast<char*>(pos) - reinterpret_cast<char*>(&header);
          nread = sizeof(header) - njunk;
          read_junk += njunk;
          memmove(&header,pos,nread);
        }
        // retry
        return input(fd,flags);
      }
      // else header is valid
      if( read_junk )
      { // got any junk before it? report it
        dprintf(2)("warning: %d junk bytes before header were discarded\n",read_junk);
        read_junk = 0;
      }
      Timestamp::now(&last_read);
      switch( header.type )
      {
        case MT_PING: 
            return Message::ACCEPT;  // ignore ping message
        
        case MT_DATA:       // data block coming up
            return readyForData(header); // sets buffers and read states accordingly
            
        case MT_ACK:         // acknowledgement of data message
            // to do later
        case MT_RETRY:       // retry data message
            // to do later
            break;
        default:
          dprintf(2)("warning: unknown packet type %d, ignoring\n",header.type);
      }
    } // end if( readstate == HEADER )
    else if( readstate == BLOCK )
    {
      incoming_checksum = read_checksum;
      return readyForTrailer(); // sets buffers and read states accordingly
    }
    else if( readstate == TRAILER )
    {
      // to do: check sequence number
      // verify checksum
      if( incoming_checksum == trailer.checksum )
      {
        dprintf(4)("received block #%d of size %d, checksum OK\n",
            trailer.seq,read_block->size());
//        acknowledge(True);    // reply with acknowledgment
        read_bset.pushNew().attach(read_block,DMI::ANON|DMI::WRITE);
        read_block = 0;
        if( trailer.msgsize ) // accumulated a complete message?
        {
          if( read_bset.size() != trailer.msgsize )
          { // major oops
            dprintf(1)("block count mismatch, expected %d got %d\n",
                trailer.msgsize,read_bset.size());
            // NB: DO SOMETHING VIOLENT HERE!!
            return readyForHeader();
          }
          else
            processIncoming();     // process the incoming message
        }
        // expect header next
        return readyForHeader(); // sets buffers and read states accordingly
      }
      else
      {
        dprintf(2)("block #%d: bad checksum\n",trailer.seq);
        return requestRetry();  // ask for retry, clear buffer
      }
    }
    else
      Throw("unexpected readstate");
  }
  return Message::ACCEPT;
  //## end GatewayWP::input%3C90BF6F00ED.body
}

// Additional Declarations
  //## begin GatewayWP%3C90BEF001E5.declarations preserve=yes
int GatewayWP::requestResync ()
{
  // Should eventually send an OOB message for a resync.
  // For now, just start looking for a header
  return readyForHeader();
}

int GatewayWP::requestRetry ()
{
  // Should eventually send an OOB message for a retransmit.
  // For now, just fllush incoming blocks and start looking for a header.
  read_bset.clear();
  return readyForHeader();
}

int GatewayWP::readyForHeader ()
{
  read_buf_size = sizeof(header);
  read_buf = reinterpret_cast<char*>(&header);
  readstate = HEADER;
  // call input procedure again to start reading
  return input(sock->getSid(),EV_FDREAD);
}

int GatewayWP::readyForTrailer ()
{
  read_buf_size = sizeof(trailer);
  read_buf = reinterpret_cast<char*>(&trailer);
  readstate = TRAILER;
  // call input procedure again to start reading
  return input(sock->getSid(),EV_FDREAD);
}

int GatewayWP::readyForData ( const PacketHeader &hdr )
{
  read_buf_size = hdr.content;
  if( read_buf_size > MaxBlockSize )
  {
    dprintf(2)("error: block size too big (%d), aborting\n",read_buf_size);
    return requestResync();
  }
  read_block = new SmartBlock(read_buf_size);
  read_buf = read_block->data();
  readstate = BLOCK;
  read_checksum = 0;
  // call input procedure again to start reading
  return input(sock->getSid(),EV_FDREAD);
}

void GatewayWP::prepareMessage (MessageRef &mref)
{
  FailWhen(write_queue.size(),"write state is not empty");
  // convert the message to blocks, placing them into the write queue
  write_msgsize = mref->toBlock(write_queue);
  // release ref, so as to minimize the blocks' ref counts
  mref.detach();
  // privatize the queue
  write_queue.privatizeAll(DMI::READONLY);
  // start sending
  prepareHeader();
}


void GatewayWP::prepareHeader ()
{
  if( !write_queue.size() )
  {
    wr_header.type = MT_PING;
    wr_header.content = 0;
  }
  else
  {
    wr_header.type = MT_DATA;
    wr_header.content = write_queue.front()->size();
  }
  write_buf = reinterpret_cast<char*>(&wr_header);
  write_buf_size = sizeof(wr_header);
  write_checksum = 0;
  nwritten = 0;
  writestate = HEADER;
}

void GatewayWP::prepareData ()
{
  const BlockRef & ref = write_queue.front();
  write_buf = ref->data();
  write_buf_size = ref->size();
  FailWhen(write_buf_size>MaxBlockSize,"block size too large");
  write_checksum = 0;
  nwritten = 0;
  writestate = BLOCK;
}

void GatewayWP::prepareTrailer ()
{
  write_queue.pop();
  wr_trailer.seq = write_seq++;
  wr_trailer.checksum = write_checksum;
  if( write_queue.size() )  // something left in queue?
    wr_trailer.msgsize = 0;
  else
    wr_trailer.msgsize = write_msgsize;
  write_buf = reinterpret_cast<char*>(&wr_trailer);
  write_buf_size = sizeof(wr_trailer);
  write_checksum = 0;
  nwritten = 0;
  writestate = TRAILER;
}

void GatewayWP::processIncoming()
{
  MessageRef ref = MessageRef(new Message,DMI::ANON|DMI::WRITE);
  ref().fromBlock(read_bset);
  if( read_bset.size() )
  {
    dprintf(1)("error: %d unclaimed incoming blocks were discarded\n",read_bset.size());
    read_bset.clear();
  }
  dsp()->send(ref,ref->to());
}

  //## end GatewayWP%3C90BEF001E5.declarations
//## begin module%3C90BFDD0240.epilog preserve=yes
//## end module%3C90BFDD0240.epilog
