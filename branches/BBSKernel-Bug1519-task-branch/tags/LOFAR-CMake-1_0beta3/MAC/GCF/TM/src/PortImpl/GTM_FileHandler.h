//#  GTM_FileHandler.h: handles all socket communication
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

#ifndef GTM_SOCKETHANDLER_H
#define GTM_SOCKETHANDLER_H

#include <GCF/TM/GCF_Handler.h>
#include <Common/lofar_map.h>
#include <sys/time.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {
// forward declaration
class GTMFile;

/**
 * This singleton class implements the main loop part of message exchange 
 * handling, which uses the "file" pattern. It calls one select for all file 
 * descriptors of the registered "files".
 */
class GTMFileHandler : public GCFHandler
{
public: 
	// constructors, destructors and default operators
    virtual ~GTMFileHandler () { _pInstance = 0;}

	// GTMFileHandler specific member methods
    /// singleton pattern methods
    static GTMFileHandler* instance ();
    static void release ();

    void workProc (); /// part of the mainloop
    void stop ();
    void registerFile (GTMFile& file);
    void deregisterFile (GTMFile& file); 

  
private:
    friend class GTMFile; // is not necessary but suppress a warning, which we can accept

    GTMFileHandler ();
    /// Don't allow copying of the GTMTimerHandler object.
    GTMFileHandler (const GTMFileHandler&);
    GTMFileHandler& operator= (const GTMFileHandler&);

	// ----- Data Members -----
    static GTMFileHandler* _pInstance; // singleton pointer

    /// all registered "files"
    typedef map<int, GTMFile*> TFiles;
    TFiles _files;
   
    /// needed for the "::select" method
    fd_set _readFDs;
    fd_set _writeFDs;
    fd_set _errorFDs;
    
    bool _running;    
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
