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
    
#include <VisAgent/VisFileOutputAgent.h>
#include <VisCube/VisVocabulary.h>
#include <MSVisAgent/MSVisAgentDebugContext.h>
#include <MSVisAgent/MSVisAgentVocabulary.h>

#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/Arrays/Slicer.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/ScalarColumn.h>

//##ModelId=3E02FF340070
class MSVisOutputAgent : public VisFileOutputAgent, public MSVisAgentDebugContext
{
  public:
    //##ModelId=3E2831C7010D
    MSVisOutputAgent();

    //##ModelId=3E28315F0001
    //##Documentation
    //## Agent initialization method. Called by the application to initialize
    //## or reinitializean agent. Agent parameters are supplied via a
    //## DataRecord.
    virtual bool init(const DataRecord::Ref &data);

    //##ModelId=3E283161032B
    //##Documentation
    //## Applications call close() when they're done speaking to an agent.
    virtual void close();

    //##ModelId=3E28316403E4
    //##Documentation
    //## Puts visibilities header onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end
    //##          CLOSED    stream closed
    virtual int putHeader(DataRecord::Ref &hdr, bool wait = True);

    //##ModelId=3E28316B012D
    //##Documentation
    //## Puts next tile onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end    
    //##          CLOSED    stream closed
    virtual int putNextTile(VisTile::Ref &tile, bool wait = True);
    
    //##ModelId=3E2D73EB0084
    //##Documentation
    //## Field name for parameter record.
    static const HIID & FParams () { return MSVisAgentVocabulary::FMSVisOutputAgentParams; }

    //##ModelId=3E283172001B
    string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

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
    MSVisOutputAgent(const MSVisOutputAgent& right);

    //##ModelId=3E2831C70155
    MSVisOutputAgent& operator=(const MSVisOutputAgent& right);
    
    //##ModelId=3E2D73EB00CE
    bool setupDataColumn (Column &col);
    
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
    
    //##ModelId=3E2ED32A0050
    Column datacol_;
    //##ModelId=3E2ED32A00A1
    Column predictcol_;
    //##ModelId=3E2ED32A00C2
    Column rescol_;
    
    //##ModelId=3E2ED50E0113
    ScalarColumn<Bool> rowFlagCol_;
    //##ModelId=3E2ED50E0190
    ArrayColumn<Bool> flagCol_;
};



#endif /* MSVISAGENT_SRC_MSVISOUTPUTAGENT_H_HEADER_INCLUDED_F5146265 */
