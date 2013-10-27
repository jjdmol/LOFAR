//# PortBroker.h: 
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: PortBroker.h 20465 2012-03-16 15:53:48Z mol $

#ifndef LOFAR_LCS_STREAM_PORTBROKER_H
#define LOFAR_LCS_STREAM_PORTBROKER_H

#ifdef USE_THREADS

#include <lofar_config.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>
#include <Common/Singleton.h>
#include <Stream/FileDescriptorBasedStream.h>
#include <Stream/SocketStream.h>

#include <time.h>
#include <string>
#include <memory>
#include <map>

namespace LOFAR {

class PortBroker: protected SocketStream {
  public:
    static const unsigned DEFAULT_PORT = 5000;

    virtual ~PortBroker();

    static void createInstance( uint16 port );
    static void destroyInstance();

    static PortBroker &instance();
    
    void start();

    size_t nrOutstandingRequests() const;

    /*
     * ServerStream waits for a client to connect asking for a specific
     * resource (prefix=false), or a class of resources (prefix=true).
     */
    class ServerStream: public FileDescriptorBasedStream {
      public:
        // Listen for a client offering resource `resource'. If prefix = true,
        // the resource only has to start with the given string.
        ServerStream( const std::string &resource, bool prefix = false, time_t deadline = 0 );

        // The resource that was requested (useful if prefix == true)
        std::string getResource() const;
      private:
        std::string resource;
    };

    /*
     * ClientStream connects to a ServerStream to obtain a specific resource.
     */
    class ClientStream: public SocketStream {
      public:
        ClientStream( const std::string &hostname, uint16 port, const std::string &resource, time_t deadline = 0 );
    };

  protected:
    static void requestResource( Stream &stream, const std::string &resource );

    // Information about a connected client
    struct ConnectedClient {
      // The data stream
      FileDescriptorBasedStream *stream;

      // The name of the requested resource
      string resource;
    };

    ConnectedClient waitForClient( const std::string &resource, bool prefix, time_t deadline );

  private:
    PortBroker( uint16 port );

    struct PortRequest {
      char resource[128];

      void set( const std::string &_resource );
      std::string key() const;

      void read(Stream&);
      void write(Stream&);
    };

    std::auto_ptr<Thread> itsThread;
    bool itsDone;

    mutable Mutex itsMutex;
    Condition itsCondition;

    typedef std::map<string, FileDescriptorBasedStream*> requestMapType;
    requestMapType itsRequestMap;

    static bool serverStarted();
    void serverLoop();
};


} // namespace LOFAR

#endif

#endif

