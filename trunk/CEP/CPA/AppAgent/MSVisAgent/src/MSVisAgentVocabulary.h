//  MSVisAgentVocabulary.h: HIIDs for MSVisAgent terms
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

#ifndef _MSVISAGENT_MSVISAGENTVOCABULARY_H
#define _MSVISAGENT_MSVISAGENTVOCABULARY_H 1
    
#include <DMI/HIID.h>
#include <VisAgent/VisAgentVocabulary.h>
#include <VisCube/VisVocabulary.h>
#include <MSVisAgent/AID-MSVisAgent.h>
   
#pragma aidgroup MSVisAgent
#pragma aid MS 

#pragma aid DDID VDSID Selection Tile String Column Size Format 
#pragma aid Vis Input Output Params Start End Write Flags Flag Mask
#pragma aid Time Data Predict Residuals Column Name Message
#pragma aid Throw Error

// this defines constants for field names used in parameter records
namespace MSVisAgent
{
  using namespace VisAgent;
  using namespace VisVocabulary;
  
  const HIID 
      
       FDDID              = AidDDID|AidIndex,
       FSelection         = AidSelection,
       FPhaseRef          = AidPhase|AidRef,
       FAntennaPos        = AidAntenna|AidPos,
       FMSName            = AidMS|AidName,
       FVDSID             = AidVDSID,
       FChannelStartIndex = AidChannel|AidStart|AidIndex,
       FChannelEndIndex   = AidChannel|AidEnd|AidIndex,
       FSelectionString   = AidSelection|AidString,
       FDataColumnName    = AidData|AidColumn|AidName,
       FTileSize          = AidTile|AidSize,
       FTileFormat        = AidTile|AidFormat,

                          
       FWriteFlags        = AidWrite|AidFlags,
       FFlagMask          = AidFlag|AidMask,
       FDataColumn        = AidData|AidColumn,
       FPredictColumn     = AidPredict|AidColumn,
       FResidualsColumn   = AidResiduals|AidColumn,
                          
                          
       __last_declaration;
       
};
     
#endif
