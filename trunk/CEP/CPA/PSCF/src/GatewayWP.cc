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

const Timeval to_init(5.0),
              to_write(5.0),
              to_heartbeat(2.0);
    
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
  setState(0);
  setPeerState(INITIALIZING);
  //## end GatewayWP::GatewayWP%3C95C53D00AE.body
}


GatewayWP::~GatewayWP()
{
  //## begin GatewayWP::~GatewayWP%3C90BEF001E5_dest.body preserve=yes
  if( sock )
    delete sock;
  //## end GatewayWP::~GatewayWP%3C90BEF001E5_dest.body
}



//## Other Operations (implementation)
void GatewayWP::start ()
{
  //## begin GatewayWP::start%3C90BF460080.body preserve=yes
  WorkProcess::start();
  // handle & ignore SIGURG -- out-of-band data on socket. 
  // addInput() will catch an exception on the fd anyway
  addSignal(SIGURG,EV_IGNORE);
  // ignore SIGPIPE, but maintain Socket's sigpipe counter
  addSignal(SIGPIPE,EV_IGNORE,&sock->sigpipe_counter);
  
  // collect local subscriptions data and send it to peer
  // iterate over all WPs to find total size of all subscriptions
  size_t nwp = 0, datasize = 0;
  Dispatcher::WPIter iter = dsp()->initWPIter();
  WPID id;
  const WPInterface *pwp;
  
  while( dsp()->getWPIter(iter,id,pwp) )
    if( id.wpclass() != AidGatewayWP ) // ignore gateway WPs
    {
      nwp++;
      datasize += pwp->getSubscriptions().packSize() + pwp->address().packSize();
    }
  size_t hdrsize = (1+2*nwp)*sizeof(size_t);
  // form block containing addresses and subscriptions
  SmartBlock *block = new SmartBlock(hdrsize+datasize);
  BlockRef blockref(block,DMI::ANON);
  size_t *hdr = static_cast<size_t*>(block->data());
  char *data  = static_cast<char*>(block->data()) + hdrsize,
       *enddata = data + datasize;
  *(hdr++) = nwp;
  iter = dsp()->initWPIter();
  while( dsp()->getWPIter(iter,id,pwp) )
    if( id.wpclass() != AidGatewayWP ) // ignore gateway WPs
    {
      Assert( data <= enddata );
      data += *(hdr++) = pwp->address().pack(data);
      data += *(hdr++) = pwp->getSubscriptions().pack(data);
    }
  Assert( data == enddata );
  dprintf(1)("generating init message for %d subscriptions, block size %d\n",
      nwp,hdrsize+datasize);
  // put this block into a message and send it to peer
  MessageRef msg(new Message(AidSubscriptions,blockref),DMI::ANON|DMI::WRITE);
  msg().setFrom(address());
  prepareMessage(msg);
  
  // subscribe to local subscribe notifications and Bye messages
  // (they will be forwarded to peer as-is)
  subscribe(AidSubscribe,Message::LOCAL);
  subscribe(AidBye,Message::LOCAL);
  
  // init timeouts
  addTimeout(to_init,AidInit,EV_ONESHOT);
  addTimeout(to_heartbeat,AidHeartbeat,EV_CONT);
  
  // start watching the socket fd
  addInput(sock->getSid(),EV_FDREAD|EV_FDWRITE|EV_FDEXCEPTION);
  write_seq = 0;
  read_junk = 0;
  readyForHeader();
  
  // init the status counters
  statmon.counter = 0;
  statmon.read = statmon.written = 0;
  statmon.ts = Timestamp::now();
  
  //## end GatewayWP::start%3C90BF460080.body
}

void GatewayWP::stop ()
{
  //## begin GatewayWP::stop%3C90BF4A039D.body preserve=yes
  if( sock )
    delete sock;
  sock = 0;
  read_bset.clear();
  write_queue.clear();
  //## end GatewayWP::stop%3C90BF4A039D.body
}

bool GatewayWP::willForward (const Message &msg) const
{
  //## begin GatewayWP::willForward%3C90BF5C001E.body preserve=yes
  if( peerState() != CONNECTED )
    return False;
  // We're doing a simple everybody-connects-to-everybody topology.
  // This determines the logic below:
  dprintf(3)("willForward(%s)",msg.sdebug(1).c_str());
  // Only forward messages of local origin (this avoids loops)
  if( msg.from().process() != address().process() ||
      msg.from().host() != address().host() )
  {
    dprintf(3)("no, non-local origin\n");
    return False;
  }
  // Check that to-scope of message matches remote 
  if( !rprocess.matches( msg.to().process() ) ||
      !rhost.matches( msg.to().host() ) )
  {
    dprintf(3)("no, `to' does not match remote %s.%s\n",
               rprocess.toString().c_str(),rhost.toString().c_str());
    return False;
  }
  // if message is published, search thru remote subscriptions
  if( msg.to().wpclass() == AidPublish )
  {
    for( CRSI iter = remote_subs.begin(); iter != remote_subs.end(); iter++ )
      if( iter->second.matches(msg) )
      {
        dprintf(3)("yes, subscribed to by remote %s\n",iter->first.toString().c_str());
        return True;
      }
    dprintf(3)("no, no remote subscribers\n");
  }
  else // else check for match with a remote address
  {
    for( CRSI iter = remote_subs.begin(); iter != remote_subs.end(); iter++ )
      if( iter->first.matches(msg.to()) )
      {
        dprintf(3)("yes, `to' address matches remote %s\n",
            iter->first.toString().c_str());
        return True;
      }
    dprintf(3)("no, no matching remote WPs\n");
  }
  return False;
  //## end GatewayWP::willForward%3C90BF5C001E.body
}

int GatewayWP::receive (MessageRef& mref)
{
  //## begin GatewayWP::receive%3C90BF63005A.body preserve=yes
  // ignore all messages from other gateways,
  // and any messages out of the remote's scope
  if( mref->from().wpclass() == AidGatewayWP ||
      !rprocess.matches(mref->to().process()) ||
      !rhost.matches(mref->to().host()) )
    return Message::ACCEPT;
  // hold off while still initializing the connection
  if( peerState() == INITIALIZING )
    return Message::HOLD;
  // else ignore if not connected
  else if( peerState() != CONNECTED )
    return Message::ACCEPT;
  
  if( writeState() != IDLE )   // writing something? 
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
    Timestamp::now(&last_write_to);
  }
  return Message::ACCEPT;
  //## end GatewayWP::receive%3C90BF63005A.body
}

int GatewayWP::timeout (const HIID &id)
{
  //## begin GatewayWP::timeout%3C90BF6702C3.body preserve=yes
  if( id == AidInit )  // connection timeout
  { 
    if( peerState() == INITIALIZING )
    {
      lprintf(1,"error: timed out waiting for init message from peer\n");
      shutdown();
    }
    return Message::CANCEL;
  }
  else if( id == AidHeartbeat )  // heartbeat 
  {
    // check that write is not blocked
    if( writeState() != IDLE && Timestamp::now() - last_write_to >= to_write )
    {
      lprintf(1,"error: timed out waiting for write()\n");
      shutdown();
    }
    // report on write queue status
    if( (statmon.counter++)%4 == 0 )
    {
      double now = Timestamp::now(), d = now - statmon.ts;
      lprintf(1,"%.2f seconds elapsed since last stats report\n"
                 "read %llu bytes (%.3f MB/s)\n"
                 "wrote %llu bytes (%.3f MB/s)\n",
                 d,statmon.read,statmon.read/(1024*1024*d),
                 statmon.written,statmon.written/(1024*1024*d));
      statmon.ts = now;
      statmon.read = statmon.written = 0;
    }
  }
  return Message::ACCEPT;
  //## end GatewayWP::timeout%3C90BF6702C3.body
}

int GatewayWP::input (int fd, int flags)
{
  //## begin GatewayWP::input%3C90BF6F00ED.body preserve=yes
  // in case we're shutting down, ignore the whole shebang
  if( !sock )
    return Message::CANCEL;
  // first handle out-of-band messages
  if( flags&EV_FDEXCEPTION ) 
  {
    flags &= ~EV_FDEXCEPTION;
    // to be implemented
    
  }
  // then handle writing
  if( flags&EV_FDWRITE )
  {
    // write is idle? disable the write input
    if( writeState() == IDLE )
    {
      removeInput(fd,EV_FDWRITE);
      return input(fd,flags&~EV_FDWRITE);    // call us again
    }
    // write data from current block
    int n = sock->write(write_buf + nwritten,write_buf_size - nwritten);
    if( n < 0 )
    {
      // on write error, just commit harakiri. GWClient/ServerWP will
      // take care of reopening a connection, eventually
      lprintf(1,"error: socket write(): %s. Aborting.\n",sock->errstr().c_str());
      shutdown();
      return Message::CANCEL; 
    }
    else if( n > 0 )
    {
      statmon.written += n;
      last_write_to = Timestamp::now(&last_write);
    }
    else // nothing written at all, so clear the write bit
      flags &= ~EV_FDWRITE;  
    // update checksum and advance the nread pointer
    for( ; n>0; n--,nwritten++ )
      write_checksum += write_buf[nwritten];
    // if buffer incomplete, then try it again
    if( nwritten < write_buf_size )
      return input(fd,flags);    
    nwritten = 0;
    // chunk written, advance to next one?
    if( writeState() == HEADER ) 
    {
      if( !write_queue.size() ) // were sending header but queue is empty
      {
        // make sure we haven't been sending a data header
        FailWhen(wr_header.type == MT_DATA,"write queue empty after data header");
        setWriteState( IDLE );
      }
      else // were sending header and queue is not empty, must be data header then
      {
        FailWhen(wr_header.type != MT_DATA,"write queue unexpectedly not empty");
        prepareData(); 
      }
    }
    else if( writeState() == BLOCK ) 
    {
      prepareTrailer();        // after block, send trailer
    }
    else if( writeState() == TRAILER ) 
    {
      if( write_queue.size() ) // something else in queue? send next header
        prepareHeader();
      else
        setWriteState( IDLE );
    }
    // have we changed state to IDLE? 
    if( writeState() == IDLE )
    {
      if( pending_msg.valid() )        // send pending message, if any
        prepareMessage(pending_msg);
      else                    
      {
        // else disable the write input
        removeInput(fd,EV_FDWRITE);
        flags &= ~EV_FDWRITE;
      }
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
      lprintf(1,"error: socket read(): %s. Aborting.\n",sock->errstr().c_str());
      shutdown();
      return Message::CANCEL; 
    }
    else if( n > 0 )
    {
      statmon.read += n;
      Timestamp::now(&last_read);
    }
    else // nothing read at all, so clear the read bit
      flags &= ~EV_FDREAD;
    // update checksum and advance the nread pointer
    for( ; n>0; n--,nread++ )
      read_checksum += read_buf[nread];
    // if buffer incomplete, then try again
    if( nread < read_buf_size )
      return input(fd,flags);     
    nread = 0;
    // else we have a complete buffer, so dispose of it according to mode
    if( readState() == HEADER ) // got a packet header?
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
        lprintf(2,"warning: %d junk bytes before header were discarded\n",read_junk);
        read_junk = 0;
      }
      Timestamp::now(&last_read);
      switch( header.type )
      {
        case MT_PING: 
            break; // ignore ping message
        
        case MT_DATA:       // data block coming up
            readyForData(header); // sets buffers and read states accordingly
            break;
            
        case MT_ACK:         // acknowledgement of data message
            // to do later
        case MT_RETRY:       // retry data message
            // to do later
            break;
        default:
          lprintf(2,"warning: unknown packet type %d, ignoring\n",header.type);
      }
    } // end if( readState() == HEADER )
    else if( readState() == BLOCK )
    {
      incoming_checksum = read_checksum;
      readyForTrailer(); // sets buffers and read states accordingly
    }
    else if( readState() == TRAILER )
    {
      // to do: check sequence number
      // verify checksum
      if( incoming_checksum == trailer.checksum )
      {
        dprintf(4)("received block #%d of size %d, checksum OK\n",
            trailer.seq,read_bset.back()->size());
//        acknowledge(True);    // reply with acknowledgment
        if( trailer.msgsize ) // accumulated a complete message?
        {
          if( read_bset.size() != trailer.msgsize )
          { // major oops
            lprintf(1,"error: block count mismatch, expected %d got %d\n",
                trailer.msgsize,read_bset.size());
            // NB: DO SOMETHING VIOLENT HERE!!
            read_bset.clear();
          }
          else
            processIncoming();     // process the incoming message
        }
        // expect header next
        readyForHeader(); // sets buffers and read states accordingly
      }
      else
      {
        dprintf(2)("block #%d: bad checksum\n",trailer.seq);
        requestRetry();  // ask for retry, clear buffer
      }
    }
    else
      Throw("unexpected read state");
  }
  // go at it again if something remains in flags (read and write will clear
  // their bits once a read/write truly fails (i.e., returns 0))
  if( flags )
    return input(fd,flags); 
  else
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
  setReadState( HEADER );
  return 0;
}

int GatewayWP::readyForTrailer ()
{
  read_buf_size = sizeof(trailer);
  read_buf = reinterpret_cast<char*>(&trailer);
  setReadState( TRAILER );
  return 0;
}

int GatewayWP::readyForData ( const PacketHeader &hdr )
{
  read_buf_size = hdr.content;
  if( read_buf_size > MaxBlockSize )
  {
    lprintf(1,"error: block size too big (%d), aborting\n",read_buf_size);
    return requestResync();
  }
  SmartBlock * bl = new SmartBlock(read_buf_size);
  read_bset.pushNew().attach(bl,DMI::ANON|DMI::WRITE);
  read_buf = static_cast<char*>(bl->data());
  setReadState( BLOCK );
  read_checksum = 0;
  return 0;
}

void GatewayWP::prepareMessage (MessageRef &mref)
{
  FailWhen(write_queue.size(),"write queue is not empty??");
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
  setWriteState( HEADER );
}

void GatewayWP::prepareData ()
{
  const BlockRef & ref = write_queue.front();
  write_buf = static_cast<const char *>(ref->data());
  write_buf_size = ref->size();
  FailWhen(write_buf_size>MaxBlockSize,"block size too large");
  write_checksum = 0;
  nwritten = 0;
  setWriteState( BLOCK );
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
  setWriteState( TRAILER );
}

void GatewayWP::processIncoming()
{
  MessageRef ref = MessageRef(new Message,DMI::ANON|DMI::WRITE);
  ref().fromBlock(read_bset);
  if( read_bset.size() )
  {
    lprintf(2,"warning: %d unclaimed incoming blocks were discarded\n",read_bset.size());
    read_bset.clear();
  }
// if connected, it is a true remote message, so send it off
  if( peerState() == CONNECTED )
  {
    // Bye message from remote: drop WP from routing table
    if( ref->id()[0] == AidBye )
    {
      RSI iter = remote_subs.find(ref->from());
      if( iter == remote_subs.end() )
        lprintf(1,"warning: got Bye [%s] from unknown remote WP\n",ref->sdebug(1).c_str());
      else
      {
        dprintf(2)("got Bye [%s], deleting routing entry\n",ref->sdebug(1).c_str());
        remote_subs.erase(iter);
      }
    } 
    // Subscribe message from remote: update table
    else if( ref->id()[0] == AidSubscribe )
    {
      // unpack subscriptions block, catching any exceptions
      Subscriptions subs;
      bool success = False;
      if( ref->data() )
      {
        try {
          subs.unpack(ref->data(),ref->datasize());
          success = True;
        } catch( std::exception &exc ) {
          lprintf(2,"warning: failed to unpack Subscribe message: %s\n",exc.what());
        }
      }
      if( success )
      {
        dprintf(2)("got Subscriptions [%s]: %d subscriptions\n",
                    ref->sdebug(1).c_str(),subs.size());
        RSI iter = remote_subs.find(ref->from());
        if( iter == remote_subs.end() )
        {
          dprintf(2)("inserting new entry into routing table\n");
          remote_subs[ref->from()] = subs;
        }
        else
        {
          dprintf(2)("updating entry in routing table\n");
          iter->second = subs;
        }
      }
      else
      {
        lprintf(2,"warning: ignoring bad Subscriptions message [%s]\n",ref->sdebug(1).c_str());
      }
    }
    // send the message on, regardless of the above
    dsp()->send(ref,ref->to());
  }
// if initializing, then it must be a Subscriptions message from peer
// (see start(), above)
  else if( peerState() == INITIALIZING ) 
  {
    dprintf(1)("received init message from peer: %s\n",ref->sdebug(1).c_str());
    if( ref->id() != HIID(AidSubscriptions) )
    {
      lprintf(1,"error: unexpected init message\n");
      shutdown();
      return;
    }
    // catch all excpetions during processing of init message
    try {
      processInitMessage(ref->data(),ref->datasize());
    } catch( std::exception &exc ) {
      lprintf(1,"error: processing init message: %s\n",exc.what());
      shutdown();
      return;
    }
    rprocess = ref->from().process();
    rhost    = ref->from().host();
    setPeerState(CONNECTED);
    lprintf(2,("connected to remote peer " + ref->from().toString() +
              "\ninitialized routing for %d remote WPs\n").c_str(),
               remote_subs.size());
    // tell dispatcher that we can forward messages now
    dsp()->declareForwarder(this);
    // publish a Remote.Up message
    MessageRef mref(new Message(AidRemote|AidUp|rprocess|rhost),DMI::ANON);
    publish(mref,Message::LOCAL);
    // publish (locally only) fake Hello messages on behalf of remote WPs
    for( CRSI iter = remote_subs.begin(); iter != remote_subs.end(); iter++ )
    {
      mref.attach(new Message(AidHello|iter->first),DMI::ANON|DMI::WRITE );
      mref().setFrom(iter->first);
      dsp()->send(mref,MsgAddress(AidPublish,AidPublish,
                                address().process(),address().host()));
    }
  }
  else
  {
    lprintf(1,"error: received remote message while in unexpected peer-state\n");
    shutdown();
  }
}

// processes subscriptions contained in peer's init-message
// (the message block is formed in start(), above)
void GatewayWP::processInitMessage( const void *block,size_t blocksize )
{
  FailWhen( !block,"no block" );
  size_t hdrsize;
  const size_t *hdr = static_cast<const size_t *>(block);
  int nwp;
  // big enough for header? 
  FailWhen( blocksize < sizeof(size_t) || 
            blocksize < ( hdrsize = (1 + 2*( nwp = *(hdr++) ))*sizeof(size_t) ),
            "corrupt block");
  // scan addresses and subscription blocks
  const char *data = static_cast<const char *>(block) + hdrsize,
             *enddata = static_cast<const char *>(block) + blocksize;
  for( int i=0; i<nwp; i++ )
  {
    size_t asz = *(hdr++), ssz = *(hdr++);
    // check block size again
    FailWhen( data+asz+ssz > enddata,"corrupt block" ); 
    // unpack address 
    MsgAddress addr; 
    addr.unpack(data,asz); data += asz;
    // unpack subscriptions
    Subscriptions &subs(remote_subs[addr]);
    subs.unpack(data,ssz);
    data += ssz;
  }
  FailWhen(data != enddata,"corrupt block");
}

void GatewayWP::shutdown () 
{
  if( peerState() == CONNECTED )     // publish a Remote.Down message
  {
    MessageRef mref(new Message(AidRemote|AidDown|rprocess|rhost),DMI::ANON);
    publish(mref,Message::LOCAL);
  }
  lprintf(1,"shutting down\n");
  setPeerState(CLOSING); 
  detachMyself();
}

  //## end GatewayWP%3C90BEF001E5.declarations
//## begin module%3C90BFDD0240.epilog preserve=yes
//## end module%3C90BFDD0240.epilog
