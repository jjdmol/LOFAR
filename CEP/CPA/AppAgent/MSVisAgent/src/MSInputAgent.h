//  MSVisInputAgent.h: agent for reading an AIPS++ MS
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

#ifndef _MSVISAGENT_MSVISINPUTAGENT_H
#define _MSVISAGENT_MSVISINPUTAGENT_H 1
    
#include <VisAgent/FileInputAgent.h>
#include <MSVisAgent/MSVisAgentDebugContext.h>
#include <MSVisAgent/MSVisAgentVocabulary.h>
#include <VisCube/VisVocabulary.h>
#include <VisCube/VisTile.h>

#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/Tables/TableIter.h>

namespace MSVisAgent
{
  
using namespace VisVocabulary;

//##ModelId=3DF9FECD013C
//##Documentation
//## MSVisInputAgent is an input agent for reading data from an AIPS++
//## Measurement Set. It is initialized from a DataRecord laid out as follows:
//## 
//## rec[MSVisInputAgentParams]  (record)    contains all parameters below
//##    +--[FMSName]             (string)    MS filename (must be present)
//##    +--[FDataColumnName]     (string)    column to read visibilities from
//##                                         (default is "DATA", but can use, 
//##                                           e.g., "MODEL_DATA")
//##    +--[FTileSize]           (int)       tile size (default: 1 timeslot)
//##    +--[FSelection]          (record)    determines MS selection:
//##       +--[FDDID]              (int)     selects data description ID 
//##       +--[FFieldIndex]        (int)     selects field  
//##       +--[FChannelStartIndex] (int)     starting channel (default: 0)
//##       +--[FChannelEndIndex]   (int)     ending channel (default: last chan.)
//##       +--[FSelectionString] (string)    additional TaQL selection applied 
//##                                           to MS
class MSInputAgent : public VisAgent::FileInputAgent, public MSVisAgentDebugContext
{
  public:
    //##ModelId=3DF9FECD0219
      MSInputAgent (const HIID &initf = MSVisAgent::FMSInputParams);
      
    //##ModelId=3DF9FECD0235
      virtual bool init (const DataRecord &data);
      
    //##ModelId=3DF9FECD0244
      virtual void close ();
      
    //##ModelId=3DF9FECD021B
      virtual int getNextTile (VisTile::Ref &tile,int wait = AppEvent::WAIT);
      
    //##ModelId=3DFDFC060373
      string sdebug ( int detail = 1,const string &prefix = "",
                      const char *name = 0 ) const;
   
      
  private:
    //##ModelId=3DF9FECD0248
      MSInputAgent(const MSInputAgent &right);
    //##ModelId=3DF9FECD0253
      MSInputAgent& operator = (const MSInputAgent &right);
      
      // prepares MS for reading
    //##ModelId=3DF9FECD025E
      void openMS     (DataRecord &hdr,const DataRecord &selection);
      // fills headers from subtables
    //##ModelId=3DF9FECD0285
      void fillHeader (DataRecord &hdr,const DataRecord &selection);

  
    //##ModelId=3DFDFC06033A
      string msname_;  
    //##ModelId=3DF9FECD0199
      MeasurementSet ms_;
    //##ModelId=3DF9FECD019A
      MeasurementSet selms_;
      
      // VDS id
    //##ModelId=3E00AA5101A0
      VDSID vdsid_;
      // observation ID -- this is incremented by 1 for each MS
    //##ModelId=3DF9FECD01A9
      int obsid_; 
      
      // name of data column used
    //##ModelId=3DF9FECD01B0
      string dataColName_;
      
      // requested tile size
    //##ModelId=3DF9FECD01C0
      int tilesize_;
      
      // channel subset
    //##ModelId=3DF9FECD01C8
      int channels_[2];
      // various counts
    //##ModelId=3DF9FECD01D0
      int num_channels_;
    //##ModelId=3DF9FECD01D7
      int num_corrs_;
    //##ModelId=3DF9FECD01DF
      int num_antennas_;
    //##ModelId=3DF9FECD01E6
      int num_ifrs_;
      
      // count of timeslots already returned
    //##ModelId=3DFDFC060354
      int current_timeslot_;
      // iterator
    //##ModelId=3DF9FECD01EE
      TableIterator tableiter_;
      
      // tile format
    //##ModelId=3DF9FECD01F6
      VisTile::Format::Ref tileformat_;
      
      // cache of prepared tiles
    //##ModelId=3DF9FECD0142
      typedef vector<VisTile::Ref> TileCache;
    //##ModelId=3DF9FECD01FF
      TileCache tiles_;
    //##ModelId=3DF9FECD020D
      TileCache::iterator tileiter_;

      
};

} // namespace MSVisAgent
    
#endif
