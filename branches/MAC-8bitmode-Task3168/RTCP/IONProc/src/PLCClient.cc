//#  BeamletBufferToComputeNode.cc: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: BeamletBufferToComputeNode.cc 17549 2011-03-11 10:34:48Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PLCClient.h>
#include <Scheduling.h>
#include <Common/DataConvert.h>
#include <Common/DataFormat.h>
#include <Interface/Exceptions.h>
#include <Common/Thread/Cancellation.h>

#include <iostream>
#include <boost/format.hpp>
#include <cstdio>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::RTCP;
using boost::format;

// Read/write a Blob at the binary level through an LCS/Stream
class FakeBlob {
public:
  FakeBlob( const char *name )
  :
    nesting_level(0)
  {
    // check for compiler tricks which pad the header or footer structs
    ASSERT( sizeof header == 16 );
    ASSERT( sizeof footer == 4 );

    if (strlen(name) > maxNameLength )
      THROW(IONProcException, "Blob name exceeds " << maxNameLength << " characters.");

    strncpy( this->name, name, sizeof this->name );
    this->name[sizeof this->name - 1] = 0;
  }

  // ----- Reading -----

  void readHeader( Stream &s ) {
    s.read( &header, sizeof header );

    if (swapEndian())
      dataConvert( LittleEndian, header.length );

    if (header.begin_marker != bobMagicValue)
      THROW(IONProcException, "Blob header begin marker not found.");

    if (header.nesting_level != nesting_level)
      THROW(IONProcException, "Nesting level does not match. Got " << header.nesting_level << " expected " << nesting_level);

    // TODO: check whether name matches expected one
    char readname[maxNameLength+1];
    s.read( &readname, header.name_length );
    readname[header.name_length] = 0;

    if (strcmp(name, readname))
      THROW(IONProcException, "Blob type name mismatch. Got " << readname << " expected " << name);

    ASSERT( sizeof name == sizeof readname );
    strncpy(name, readname, sizeof name);  // still use strncpy, the assertions and conditions do not guarantee correct operation 100%
                                           // for example, assertions could be turned off
  }

  void readFooter( Stream &s ) {
    s.read( &footer, sizeof footer );

    if (footer.end_marker != eobMagicValue) {
      THROW(IONProcException, "Blob footer end marker not found.");
    }
  }

  bool swapEndian() const {
    return header.endianness != dataFormat();
  }

  template<typename T> void read( Stream &s, T& x ) const {
    s.read( &x, sizeof x );

    if (swapEndian())
      dataConvert( dataFormat(), x );
  }

  // ----- Size prediction -----

  template<typename T> size_t size( const T& x ) const {
    return sizeof x;
  }

  // ----- Writing -----

  size_t overheadSize() const {
    return sizeof header + strlen( name ) + sizeof footer;
  }

  void writeHeader( Stream &s, int datalen, int8 version = 0 ) {
    header.begin_marker = bobMagicValue;
    header.length = datalen + overheadSize();
    header.version = version;
    header.endianness = dataFormat();
    header.nesting_level = nesting_level;
    header.name_length = strlen( name );

    s.write( &header, sizeof header );
    s.write( this->name, header.name_length );
  }

  void writeFooter( Stream &s ) {
    footer.end_marker = eobMagicValue;

    s.write( &footer, sizeof footer );
  }

  template<typename T> void write( Stream &s, const T& x ) const {
    s.write( &x, sizeof x );
  }


private:  
  // LCS/Blob/include/Blob/BlobHeader.h
  struct blob_header {
    uint64 length;        // or 0 if unknown
    uint32 begin_marker;  // BOB_MAGICVALUE
    int8   version;
    int8   endianness;    // 0 = little endian, 1 = big endian, see LCS/Common/include/Common/DataFormat.h
    uint8  nesting_level; // starts with 0
    uint8  name_length;
  } __attribute__((packed)) header;

  #define MAX_UINT8 ((1 << 8) - 1)
  static const size_t maxNameLength = MAX_UINT8;

  char name[maxNameLength + 1]; // we zero-terminate, but Blob doesn't need it as the length is communicated in the header

  struct blob_footer {
    uint32 end_marker;
  } __attribute__((packed)) footer;

  const uint8 nesting_level; // current nesting level when reading or writing Blobs

  // LCS/Blob/include/Blob/BlobHeader.h
  static const uint32 bobMagicValue = 0xbebebebe;
  static const uint32 eobMagicValue = 0xbfbfbfbf;
};

// strings require special routines to read/write

template<> size_t FakeBlob::size<string>( const string& x ) const {
  return sizeof (int64) + x.size();
}

template<> void FakeBlob::read<string>( Stream &s, string &str ) const {
  int64 length;
  char *cstr = 0;

  read( s, length );

  cstr = new char[length+1];

  // make sure cstr won't leak if s.read throws
  struct D {
    ~D() { delete[] cstr; }
    char *cstr;
  } onDestruct = { cstr };
  (void)onDestruct;

  s.read( cstr, length );
  cstr[length] = 0;

  str = cstr;
}

template<> void FakeBlob::write<string>( Stream &s, const string &str ) const {
  int64 size = str.size();

  write( s, size );
  s.write( str.c_str(), str.size() );
}

// Emulate a ProcControl DataHolder as defined in LCS/ACC/PLC/src/DH_ProcControl.cc
struct DH_ProcControl {
  uint16 version;
  int16  command; // PCCmd
  uint16 result; // bitfield, see LCS/ACC/ALC/include/ALC/ApplControlComm.h:
                 // 0x0001: OK (completed successfully)
                 // 0x0002: Scheduled
                 // 0x0004: Overruled, but scheduled
                 // 0x8000: AC could not be reached

  // extraBlob fields
  string options;

  bool isOK() const {
    return result & PCCmdMaskOk;
  }

  void read( Stream &s ) {
    FakeBlob b( "DH_ProcControl" );
    FakeBlob e( "Extra" );

    b.readHeader( s );

    // read regular data fields
    b.read( s, version );
    b.read( s, command );
    b.read( s, result );

    // read 'extraBlob'
    e.readHeader( s );
    e.read( s, options );
    e.readFooter( s );

    b.readFooter( s );
  }

  void write( Stream &s ) const {
    FakeBlob b( "DH_ProcControl" );
    FakeBlob e( "Extra" );

    // we need to predict the size of the payload, recursively
    unsigned eSize = e.size(options);
    unsigned bSize = b.size(version) + b.size(command) + b.size(result) + e.overheadSize() + eSize;

    b.writeHeader( s, bSize );

    // write regular data fields
    b.write( s, version );
    b.write( s, command );
    b.write( s, result );

    // write 'extraBlob'
    e.writeHeader( s, eSize, 1 );
    e.write( s, options );
    e.writeFooter( s );

    b.writeFooter( s );
  }
};

namespace LOFAR {
namespace RTCP {

PLCClient::PLCClient( Stream &s, PLCRunnable &job, const std::string &procID, unsigned observationID )
:
  itsStream( s ),
  itsJob( job ),
  itsProcID( procID ),
  itsStartTime( time(0L) ),
  itsDefineCalled( false ),
  itsDone( false ),
  itsLogPrefix( str(format("[obs %u] [PLC] ") % observationID) )
{
}

void PLCClient::start()
{
  itsThread = new Thread(this, &PLCClient::mainLoop, "[PLC] ", 65535);
}

PLCClient::~PLCClient()
{
  ASSERT(itsThread);

  // wait until ApplController called define(), so that invalid parsets are reported
  // as such before the connection is terminated
  struct timespec disconnectAt = { itsStartTime + defineWaitTimeout, 0 };

  {
    ScopedLock lock(itsMutex);
    ScopedDelayCancellation dc; // TODO: make this cancellable, by moving the loop below outside the destructor

    while (!itsDefineCalled && !itsDone) {
      LOG_DEBUG_STR( itsLogPrefix << "Waiting for ApplController to call define()" );

      if (!itsCondition.wait(itsMutex, disconnectAt)) {
        // timeout
        LOG_WARN_STR( itsLogPrefix << "ApplController did not ask whether parset was ok (define())" );
        break;
      }
    }

    itsDone = true;
  }

  // Release the thread first! Use waitForDone() as the
  // thread might have been deleted in an earlier waitForDone()
  waitForDone();
}

void PLCClient::waitForDone()
{
  if (itsThread != 0)
    delete itsThread.release();
}

void PLCClient::sendCmd( PCCmd cmd, const string &options )
{
  struct DH_ProcControl pc;

  pc.version = 0x100;
  pc.command = cmd;
  pc.result = 0;

  pc.options = options;

  pc.write( itsStream );
}

void PLCClient::sendResult( PCCmd cmd, const string &options, uint16 result )
{
  struct DH_ProcControl pc;

  pc.version = 0x100;
  pc.command = cmd | PCCmdResult;
  pc.result = result;

  pc.options = options;

  pc.write( itsStream );
}

void PLCClient::recvCmd( PCCmd &cmd, string &options ) const
{
  struct DH_ProcControl pc;

  pc.read( itsStream );

  cmd = static_cast<PCCmd>(pc.command);
  options = pc.options;
}

// outer control structure (PLC protocol):
//   LCS/ACC/PLC/src/ProcCtrlRemote.cc
// inner control structure (PLC message handling):
//   LCS/ACC/PLC/src/ProcControlServer.cc

void PLCClient::mainLoop()
{
#if defined HAVE_BGP_ION
  //doNotRunOnCore0();
  runOnCore0();
#endif

  bool running = false;
  bool pausing = false;

  // make sure we set itsDone in case of exceptions
  struct D {
    ~D() {
      ScopedLock lock(mutex);

      done = true;

      // signal our destructor that we're done
      condition.broadcast();
    }

    bool &done;
    Mutex &mutex;
    Condition &condition;
  } onDestruct = { itsDone, itsMutex, itsCondition };
  (void)onDestruct;

  // register: send BOOT command
  LOG_DEBUG_STR( itsLogPrefix << "Register" );
  sendCmd( PCCmdBoot, itsProcID );

  while (!itsDone) {
    PCCmd cmd;
    string options;
    bool result = false;
    bool resultExpected = true;
    bool supported = true;

    try {
      recvCmd( cmd, options );
    } catch (SystemCallException &ex) {
      if (itsDone)
        LOG_INFO_STR( itsLogPrefix << "Lost connection to ApplController, but am quitting.");
      else
        throw;
    }

    switch (cmd) {
      case PCCmdInfo:
        LOG_DEBUG_STR( itsLogPrefix << "info()" );

        sendResult( cmd, "PCCmdInfo is not supported", PCCmdMaskOk );
        resultExpected = false;
        break;

      case PCCmdAnswer:
        LOG_DEBUG_STR( itsLogPrefix << "answer()" );

        resultExpected = false;
        break;

      case PCCmdDefine:
        LOG_DEBUG_STR( itsLogPrefix << "define()" );

        result = itsJob.define();

        {
          ScopedLock lock(itsMutex);

          itsDefineCalled = true;

          // signal our destructor that define() was called
          itsCondition.broadcast();
        }  
        break;

      case PCCmdInit:
        LOG_DEBUG_STR( itsLogPrefix << "init()" );

        result = itsJob.init();
        break;

      case PCCmdRun:
        LOG_DEBUG_STR( itsLogPrefix << "run()" );

        result = itsJob.run();
        running = true;
        break;

      case PCCmdPause:
        LOG_DEBUG_STR( itsLogPrefix << "pause( " << options << " )" );

        {
          double when;

          // condition can be:
          //  "now"  -> immediately, non-graceful
          //  "asap" -> immediately, graceful
          //  "timestamp=xxx" -> at unix time xxx (GMT)

          if (options == "now" || options == "asap") {
            when = 0;
          } else {
             // condition == "timestamp=xxx"
             if (sscanf( options.c_str(), "timestamp=%lf", &when ) != 1) {
               LOG_ERROR_STR( "PLC: Could not parse timestamp in " << options );
               when = 0;
             }
          }

          result = itsJob.pause(when);
        }

        pausing = true;

        if (running && result) {
          // delay sending the result until we actually pause
          resultExpected = false;
        }
        break;

      case PCCmdRelease:
        LOG_DEBUG_STR( itsLogPrefix << "release() -- silent ignore" );
        break;

      case PCCmdQuit:
        LOG_DEBUG_STR( itsLogPrefix << "quit()" );

        itsJob.quit();

        itsDone = true;
        resultExpected = false;
        break;

      case PCCmdSnapshot:
        LOG_ERROR_STR( itsLogPrefix << "snapshot( " << options << " ) -- not supported" );

        supported = false;
        break;

      case PCCmdRecover:
        LOG_ERROR_STR( itsLogPrefix << "recover( " << options << " ) -- not supported" );

        supported = false;
        break;

      case PCCmdReinit:
        LOG_ERROR_STR( itsLogPrefix << "reinit( " << options << " ) -- not supported" );

        supported = false;
        break;

      default:
        break;
    }

    /* TODO: do this properly. We are expected to acknowledge the pause command once we enter
             pause mode, but this code is only triggered when commands are processed */
    if (running && pausing && !itsJob.observationRunning()) {
      // we paused -- ack the pause command that triggered it
      LOG_DEBUG_STR( itsLogPrefix << "Sending ack for pause()" );
      sendResult( PCCmdPause, "", PCCmdMaskOk ); 
      running = false;
      pausing = false;
    }

    if (resultExpected) {
      // report back if we support the command and if so, if it succeeded
      sendResult( cmd, "", 
        supported ? (result ? PCCmdMaskOk : 0)
                  : PCCmdMaskNotSupported );
    }
  }

  // unregister: send QUIT command
  LOG_DEBUG_STR( itsLogPrefix << "Unregister" );
  sendCmd( PCCmdQuit, "" );
}

} // namespace RTCP
} // namespace LOFAR


