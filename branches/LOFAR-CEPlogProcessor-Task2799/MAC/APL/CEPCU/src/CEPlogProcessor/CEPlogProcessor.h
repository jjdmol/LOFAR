//#  CEPlogProcessor.cc: Moves the operator info from the logfiles to PVSS
//#
//#  Copyright (C) 2009
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
//#  $Id$
#ifndef LOFAR_APL_CEPLOGPROCESSOR_H
#define LOFAR_APL_CEPLOGPROCESSOR_H

// \file
// Daemon for launching Application Controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

#include "CircularBuffer.h"

#include <time.h>
#include <vector>
#include <string>
#include <map>

namespace LOFAR {
    using MACIO::GCFEvent;
    using GCF::TM::GCFTask;
    using GCF::TM::GCFTCPPort;
    using GCF::TM::GCFTimerPort;
    using GCF::TM::GCFPortInterface;
    using GCF::RTDB::RTDBPropertySet;
    namespace APL {

// \addtogroup CEPCU
// @{


// The CEPlogProcessor class implements a small daemon that ...
class CEPlogProcessor : public GCFTask
{
public:
    explicit CEPlogProcessor(const string&  cntlrName);
    ~CEPlogProcessor();

    // its processing states
    GCFEvent::TResult initial_state     (GCFEvent& event, GCFPortInterface& port);
    GCFEvent::TResult createPropertySets(GCFEvent& event, GCFPortInterface& port);
    GCFEvent::TResult startListener     (GCFEvent& event, GCFPortInterface& port);
    GCFEvent::TResult operational       (GCFEvent& event, GCFPortInterface& port);
    GCFEvent::TResult finish_state      (GCFEvent& event, GCFPortInterface& port);

    // Interrupthandler for switching to the finish state when exiting the program
    static void signalHandler (int signum);
    void        finish();
    
private:
    // Copying is not allowed
    CEPlogProcessor();
    CEPlogProcessor(const CEPlogProcessor&  that);
    CEPlogProcessor& operator=(const CEPlogProcessor& that);

    // Admin functions
    void     _deleteStream    (GCFPortInterface&    port);
    void     _handleConnectionRequest();

    // Routines for processing the loglines.
    void     _handleDataStream  (GCFPortInterface*  port);
    time_t   _parseDateTime     (const char *datestr, const char *timestr) const;
    void     _processLogLine    (const char *cString);

    struct logline {
      // info straight from splitting log line
      const char *process;
      const char *host;
      const char *date;
      const char *time;
      const char *loglevel;
      const char *target;
      const char *msg;

      // info parsed straight from log line
      time_t timestamp;
      int obsID; // or -1 if unknown

      // info calculated from log line
      const char *tempobsname;
    };
      
    void collectGarbage();

    // Return the observation ID, or -1 if none can be found
    int _getParam(const char *msg,const char *param) const;

    // Return the temporary obs name to use in PVSS. Also registers the temporary obs name
    // if the provided log line announces it.
    string getTempObsName(int obsID, const char *msg);

    void _processIONProcLine(const struct logline &);
    void _processCNProcLine(const struct logline &);
    void _processStorageLine(const struct logline &);

    //# --- Datamembers --- 
    // The listener socket to receive the requests on.
    GCFTCPPort*     itsListener;

    RTDBPropertySet*    itsOwnPropertySet;
    GCFTimerPort*       itsTimerPort;

    // internal structure for admin for 1 stream
    typedef struct {
        GCFTCPPort* socket;
        CircularBuffer* buffer;
    } streamBuffer_t;

    // Map containing all the streambuffers.
    map<GCFPortInterface*, streamBuffer_t>  itsLogStreams;
    vector<GCFPortInterface*>               itsLogStreamsGarbage;

    vector<RTDBPropertySet*>    itsInputBuffers;
    vector<RTDBPropertySet*>    itsAdders;
    vector<RTDBPropertySet*>    itsWriters;

    // values read from the conf file.
    unsigned        itsNrInputBuffers;
    unsigned        itsNrIONodes;
    unsigned        itsNrAdders;
    unsigned        itsNrStorage;
    unsigned        itsNrWriters;
    unsigned        itsBufferSize;

    template<typename T, typename U> class BiMap {
    public:
      void set( const T &t, const U &u ) {
        // erase old entries across both maps
        if (exists(t))
          backward.erase(forward[t]);
        if (exists(u))
          forward.erase(backward[u]);

        forward[t] = u;
        backward[u] = t;
      }

      void erase( const T &t ) {
        backward.erase( forward[t] );
        forward.erase( t );
      }

      bool exists( const T &t ) const {
        return forward.find(t) != forward.end();
      }

      bool exists( const U &u ) const {
        return backward.find(u) != backward.end();
      }

      T &lookup( const U &u ) {
        return backward[u];
      }

      U &lookup( const T &t ) {
        return forward[t];
      }

    private:
      map<T,U> forward;
      map<U,T> backward;
    };

    // a BiMap is needed to automatically remove obsIDs that point to
    // reused tempObsNames.
    BiMap<int,string> itsTempObsMapping;
};

// @} addgroup
  } // namespace APL
} // namespace LOFAR

#endif
