//  VisFileInputAgent.h: common base class for VisInputAgents that read files
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef _VISAGENT_VISFILEINPUTAGENT_H
#define _VISAGENT_VISFILEINPUTAGENT_H 1

#include <VisAgent/VisInputAgent.h>

//##ModelId=3DF9FECD0159
//##Documentation
//## VisFileInputAgent implements some common features & services for agents
//## that get their input from a static data file (e.g., AIPS++ MSs, BOIO
//## files).
class VisFileInputAgent : public VisInputAgent
{
  protected:
    //##ModelId=3DF9FECD015F
      typedef enum {
        CLOSED  = 0,
        HEADER  = 1,
        DATA    = 2,
        ENDFILE = 3,
        ERROR   = 4
      } FileState;
      
  private:
    //##ModelId=3DF9FECE0047
      FileState         filestate_;
    //##ModelId=3DF9FECE0055
      DataRecord::Ref   header_,eventdata_;
      
      // will generate at most one event at a time
    //##ModelId=3DF9FECE00EB
      HIID              event_;
      
  protected:
      
    //##ModelId=3DF9FECE00FC
      VisFileInputAgent ()
          : filestate_(CLOSED)
          {}
      
      // reports the file state
    //##ModelId=3DF9FECE012A
      FileState fileState    () const
          { return filestate_; }
      
    //##ModelId=3DFDFC07004C
      string fileStateString () const;
      
      // called form subclass to set the file state.
    //##ModelId=3DF9FECE0154
      void setFileState (FileState state);
      
      // sets the ERROR state, and caches an error message
      // This will also generate an error event
    //##ModelId=3DF9FECE01CA
      void setErrorState (const string &msg);
      
      // generates an event
    //##ModelId=3DF9FECE024A
      void generateEvent (const HIID &evname);
    //##ModelId=3DF9FECE02BA
      void generateEvent (const HIID &evname,DataRecord::Ref &data);
      
      // returns header by reference
    //##ModelId=3DF9FECE0384
      DataRecord &  header     ()
          { return header_.dewr(); }

      // initializes an empty header
    //##ModelId=3DF9FECE03AB
      DataRecord &  initHeader ();
          
  public:
      
    //##ModelId=3DF9FECE03D1
      virtual int getHeader (DataRecord::Ref &hdr,bool wait = True);
  
    //##ModelId=3DF9FECF009D
      virtual int hasHeader () const;
      
    //##ModelId=3DF9FECF00BF
      virtual int hasTile   () const;
      
    //##ModelId=3DF9FECF00DB
      virtual bool getEvent (HIID &,DataRecord::Ref &data,bool wait = False);
      
    //##ModelId=3DF9FECF01DA
      virtual bool hasEvent (const HIID &mask = HIID(),bool outOfSeq = False);
};


#endif
    
