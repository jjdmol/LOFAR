//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C90BFDD0236.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C90BFDD0236.cm

//## begin module%3C90BFDD0236.cp preserve=no
//## end module%3C90BFDD0236.cp

//## Module: GatewayWP%3C90BFDD0236; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\GatewayWP.h

#ifndef GatewayWP_h
#define GatewayWP_h 1

//## begin module%3C90BFDD0236.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C90BFDD0236.additionalIncludes

//## begin module%3C90BFDD0236.includes preserve=yes
//## end module%3C90BFDD0236.includes

// Socket
#include "Socket.h"
// WorkProcess
#include "WorkProcess.h"
//## begin module%3C90BFDD0236.declarations preserve=no
//## end module%3C90BFDD0236.declarations

//## begin module%3C90BFDD0236.additionalDeclarations preserve=yes
//## end module%3C90BFDD0236.additionalDeclarations


//## begin GatewayWP%3C90BEF001E5.preface preserve=yes
//## end GatewayWP%3C90BEF001E5.preface

//## Class: GatewayWP%3C90BEF001E5
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class GatewayWP : public WorkProcess  //## Inherits: <unnamed>%3C90BF100390
{
  //## begin GatewayWP%3C90BEF001E5.initialDeclarations preserve=yes
  //## end GatewayWP%3C90BEF001E5.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: GatewayWP%3C95C53D00AE
      GatewayWP (Socket* sk);

    //## Destructor (generated)
      ~GatewayWP();


    //## Other Operations (specified)
      //## Operation: start%3C90BF460080
      virtual void start ();

      //## Operation: stop%3C90BF4A039D
      virtual void stop ();

      //## Operation: willForward%3C90BF5C001E
      //	Returns True if this WP will forward this non-local message.
      virtual bool willForward (const Message &msg) const;

      //## Operation: receive%3C90BF63005A
      virtual int receive (MessageRef& mref);

      //## Operation: timeout%3C90BF6702C3
      virtual int timeout (const HIID &id);

      //## Operation: input%3C90BF6F00ED
      virtual int input (int fd, int flags);

    // Additional Public Declarations
      //## begin GatewayWP%3C90BEF001E5.public preserve=yes
      //## end GatewayWP%3C90BEF001E5.public

  protected:
    // Additional Protected Declarations
      //## begin GatewayWP%3C90BEF001E5.protected preserve=yes
      // packet header structure
      typedef struct { char  signature[3];
                       uchar type;
                       long  content; 
                     } PacketHeader;
                       
      // data block trailer structure
      typedef struct { int  seq;
                       long checksum;
                       int  msgsize;
                     } DataTrailer;
                       
      
      typedef enum { MT_PING=0,MT_DATA=1,MT_ACK=2,MT_RETRY=3,
                     MT_ABORT=4,MT_MAXTYPE=4 } PacketTypes;
      
      typedef enum { IDLE,HEADER,BLOCK,TRAILER } DataState;
      

      
      // Helper functions for reading from socket
      int requestResync   ();   // ask remote to abort & resync
      int requestRetry    ();   // ask remote to resend last message
      int readyForHeader  ();   // start looking for header
      int readyForTrailer ();   // peraprte to receive trailer
      int readyForData    (const PacketHeader &hdr); // prepare to receive data block
      void processIncoming();   // unblocks and sends off incoming message
      
      // Helper functions for writing to socket
      void prepareMessage (MessageRef &mref);
      void prepareHeader  ();
      void prepareData    ();
      void prepareTrailer ();
      
      // incoming/outgoing packet header
      PacketHeader header,wr_header;
      // read buffer for data trailer
      DataTrailer  trailer,wr_trailer;
      // max size of xmitted block. Anything bigger than that will cause
      // an error (should be in shared memory!)
      static const int MaxBlockSize = 1024*1024;
      
      // reading state
      DataState readstate;
      char *read_buf;
      int   read_buf_size,
            nread,
            read_junk;
      long  read_checksum,
            incoming_checksum;
      SmartBlock *read_block;
      BlockSet read_bset;
      
      // writing state
      DataState writestate;
      const char *write_buf;
      int   write_buf_size,nwritten;
      long  write_checksum;
      BlockSet write_queue;
      int   write_seq;                 // sequence number
      int   write_msgsize;
      MessageRef pending_msg;          // one pending write-slot
      
      // timestamps for pings
      Timestamp last_read,last_write;      
      
      //## end GatewayWP%3C90BEF001E5.protected
  private:
    //## Constructors (generated)
      GatewayWP();

      GatewayWP(const GatewayWP &right);

    //## Assignment Operation (generated)
      GatewayWP & operator=(const GatewayWP &right);

    // Additional Private Declarations
      //## begin GatewayWP%3C90BEF001E5.private preserve=yes
      //## end GatewayWP%3C90BEF001E5.private

  private: //## implementation
    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C9225740182
      //## Role: GatewayWP::sock%3C9225740345
      //## begin GatewayWP::sock%3C9225740345.role preserve=no  private: Socket { -> 0..1RHgN}
      Socket *sock;
      //## end GatewayWP::sock%3C9225740345.role

    // Additional Implementation Declarations
      //## begin GatewayWP%3C90BEF001E5.implementation preserve=yes
      //## end GatewayWP%3C90BEF001E5.implementation

};

//## begin GatewayWP%3C90BEF001E5.postscript preserve=yes
//## end GatewayWP%3C90BEF001E5.postscript

// Class GatewayWP 

//## begin module%3C90BFDD0236.epilog preserve=yes
//## end module%3C90BFDD0236.epilog


#endif
