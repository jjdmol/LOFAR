//#  GTM_File.h: base class for all sockets
//#
//#  Copyright (C) 2002-2003
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

#ifndef GTM_FILE_H
#define GTM_FILE_H

#include <unistd.h>
#include <MACIO/GCF_Event.h>

namespace LOFAR {
	using namespace MACIO;
	namespace GCF {
		namespace TM {

// forward declaration
class GTMFileHandler;
class GCFRawPort;

/**
 * This class consists of the basic implementation of a "file/device". 
 */

class GTMFile
{
  public: // constructors, destructors and default operators
    virtual ~GTMFile ();
  
  protected:
    GTMFile (GCFRawPort& port);

  private:
    GTMFile ();
    /// Don't allow copying of the GTMFile object.
    GTMFile (const GTMFile&);
    GTMFile& operator= (const GTMFile&);

  public: // GTMFile specific methods
    virtual bool close ();
  
    /**
     * send/recv methods
     */
    virtual ssize_t send (void* buf, size_t count) = 0;
    virtual ssize_t recv (void* buf, size_t count) = 0;

    int getFD () const {return _fd;}
    virtual int setFD (int fd);
    virtual void workProc ();
    
  protected: // helper methods
    GCFEvent::TResult   dispatch (GCFEvent& event);
        
  protected: // data members
    /// filedescriptor
    int               _fd;
    /// selects on all registered filedescriptors
    GTMFileHandler*   _pHandler; 
    /// related port
    GCFRawPort&       _port;    
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#include <GCF/TM/GCF_RawPort.h>

using namespace LOFAR::GCF::TM;

inline LOFAR::MACIO::GCFEvent::TResult GTMFile::dispatch (LOFAR::MACIO::GCFEvent& event) 
{
  return _port.dispatch(event);
}
#endif
