//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(UVPDATATRANSFERWP_H)
#define UVPDATATRANSFERWP_H

// $Id$



#include <OCTOPUSSY/WorkProcess.h>

#include <vector>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

#include <uvplot/UVPDataHeader.h>
#include <uvplot/UVPDataAtom.h>
#include <uvplot/UVPDataSet.h>



//*********** AID mechanism declarations
#include <UVD/UVD.h>
#include <AID-uvplot.h>
#pragma aidgroup uvplot
#pragma aid UVPDataTransferWP
static int AID_DUMMY_INITIALISATION = aidRegistry_uvplot();
//*********** End of AID mechanism declarations


//! The WorkProcess that receives visibility data and stores it locally
class UVPDataTransferWP: public WorkProcess
{
#if(DEBUG_MODE)
  LocalDebugContext;
#endif
 public:

  //! Constructor.
  /*!
    \param correlation The correlationtype that must be stored
    \param baseline    The IFR number that must be stored. IFR =
    ant1*(ant1+1)/2 + ant2.
    \param patchID     Number of the patch.
   */
  UVPDataTransferWP(int         patchID,
                    UVPDataSet *dataSet);

  //!Initializes communication. Sets up the subscriptions. 
  /*!Overridden from WPInterface, the parent class of WorkProcess. These
     functions are called by the base class.
     In init, one has to subscribe to the messages one wants to receive
  */
  virtual void init();
  
  //! Calls WorkProcess::start()
  /*! I have no idea what would be a could implementation of start. For
      the time being it just calls its baseclass::start() function.
  */
  virtual bool start();
  

  //! Actually receives the messages.
  /*! It translates them to UVPDataAtom objects and stores them in
      itsCachedData.
  */
  virtual int receive(MessageRef &messageRef);

 protected:
 private:

  UVPDataSet*             itsCachedData;

  int                     itsPatchID; /* Zero based */
  
  bool                     itsHeaderIsReceived;
  UVPDataHeader            itsHeader;
  

  // Octopussy stuff
  HIID                     itsHeaderHIID;
  HIID                     itsDataHIID;
  HIID                     itsFooterHIID;
};


#endif // UVPDATATRANSFERWP_H
