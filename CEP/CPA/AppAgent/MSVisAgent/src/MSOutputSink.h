//  MSVisOutputAgent.h: agent for writing an AIPS++ MS
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

#ifndef MSVISAGENT_SRC_MSVISOUTPUTAGENT_H_HEADER_INCLUDED_F5146265
#define MSVISAGENT_SRC_MSVISOUTPUTAGENT_H_HEADER_INCLUDED_F5146265
    
#include <AppAgent/FileSink.h>
#include <MSVisAgent/MSVisAgentDebugContext.h>
#include <MSVisAgent/MSVisAgentVocabulary.h>
#include <VisCube/VisVocabulary.h>
#include <VisCube/VisTile.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Array.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

namespace MSVisAgent
{
  
using namespace VisVocabulary;

//##ModelId=3E02FF340070
class MSOutputSink : public FileSink
{
  public:
    //##ModelId=3E2831C7010D
    MSOutputSink ();

    //##ModelId=3E28315F0001
    //##Documentation
    //## Agent initialization method. Called by the application to initialize
    //## or reinitializean agent. Agent parameters are supplied via a
    //## DataRecord.
    virtual bool init(const DataRecord &data);

    //##ModelId=3E283161032B
    //##Documentation
    //## Applications call close() when they're done speaking to an agent.
    virtual void close();

    //##ModelId=3EC25BF002D4
    //##Documentation
    //## Posts an event on behalf of the application.
    //## If a non-empty destination is specified, the event is directed at
    //## a specific destination, if the event sink implementation supports
    //## this (e.g., if responding to a request event, destination could be
    //## equal to the original event source).
    virtual void postEvent(const HIID &id, const ObjRef::Xfer &data = ObjRef(), const HIID &destination = HIID());

    //##ModelId=3EC25BF002E4
    //##Documentation
    //## Checks whether a specific event is bound to any output. I.e., if the
    //## event would be simply discarded when posted, returns False; otherwise,
    //## returns True. Apps can check this before posting "expensive" events.
    //## Default implementation always returns False.
    virtual bool isEventBound(const HIID &id);
    
    //##ModelId=3E283172001B
    string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;
    
    //##ModelId=400E5B6C00EC
    ImportDebugContext(MSVisAgentDebugContext);
    
  protected:
    //##ModelId=3EC25F74033F
    //##Documentation
    //## called to put more objects on the stream. Returns SUCCESS if something
    //## was put on, or <0 code (ERROR/CLOSED/whatever)
    virtual int refillStream();

  private:
    //##ModelId=3E2ED3290292
    typedef struct 
    {
      string name;
      ArrayColumn<Complex> col;
      bool   valid;
    }
    Column;
    
    //##ModelId=3E2831C7011D
    MSOutputSink(const MSOutputSink& right);

    //##ModelId=3E2831C70155
    MSOutputSink& operator=(const MSOutputSink& right);
    
    //##ModelId=3E2D73EB00CE
    bool setupDataColumn (Column &col);

    //##ModelId=3E28316403E4
    //##Documentation
    //## Puts visibilities header onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end
    //##          CLOSED    stream closed
    virtual void doPutHeader(const DataRecord &header);

    //##ModelId=3E28316B012D
    //##Documentation
    //## Puts next tile onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end    
    //##          CLOSED    stream closed
    virtual void doPutTile (const VisTile &tile);

    //##ModelId=3F5F436303AB
    void putColumn (Column &col,int irow,const LoMat_fcomplex &data);
    
    //##ModelId=3E2D6130030C
    string msname_;
        
    //##ModelId=3E2BC34C0076
    MeasurementSet ms_;
    
    //##ModelId=3E2D5FF503A1
    DataRecord params_;
    
    //##ModelId=3E2D5FF60047
    bool write_flags_;
    //##ModelId=3E2D5FF600B0
    int flagmask_;
    
    //##ModelId=3E2D61D901E8
    int tilecount_;
    //##ModelId=3E2D61D9024A
    int rowcount_;
        
    //##ModelId=3E2D5FF60119
    Slicer column_slicer_;
    
    //##ModelId=3E42745300A8
    Column datacol_;
    //##ModelId=3E42745300C2
    Column predictcol_;
    //##ModelId=3E42745300DA
    Column rescol_;
    
    //##ModelId=3E2ED50E0113
    ScalarColumn<Bool> rowFlagCol_;
    //##ModelId=3E2ED50E0190
    ArrayColumn<Bool> flagCol_;
    
    //##ModelId=3F5F43630379
    Array<Complex> null_cell_;
};


} // namespace MSVisAgent

#endif /* MSVISAGENT_SRC_MSVISOUTPUTAGENT_H_HEADER_INCLUDED_F5146265 */
