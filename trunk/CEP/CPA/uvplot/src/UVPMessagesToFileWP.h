
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

#if !defined(UVPMESSAGESTOFILEWP_H)
#define UVPMESSAGESTOFILEWP_H

// $Id$


#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

#include <DMI/BOIO.h>
#include <OCTOPUSSY/WorkProcess.h>

//*********** AID mechanism declarations
#include <UVD/UVD.h>
#include <uvplot/AID-uvplot.h>
#pragma aid UVPMessagesToFileWP
//*********** End of AID mechanism declarations


namespace UVP
{
  const HIID  SorterHeaderHIID  = HIID(AidUVData|
                                       AidAny|  // UV-Set ID
                                       AidAny|  // Segment ID
                                       AidPatch|
                                       AidAny|  // Patch ID
                                       AidHeader|
                                       AidCorr|
                                       AidIFR);;

  const HIID  SorterMessageHIID = HIID(AidUVData|
                                       AidAny|  // UV-Set ID
                                       AidAny|  // Segment ID
                                       AidPatch|
                                       AidAny|  // Patch ID
                                       AidData|
                                       AidCorr|
                                       AidIFR|
                                       AidAny|  // Correlation ID
                                       AidAny); // IFR;

  const HIID  SorterFooterHIID  = HIID(AidUVData|
                                       AidAny|
                                       AidAny|
                                       AidPatch|
                                       AidAny|
                                       AidFooter|
                                       AidCorr|
                                       AidIFR);

  const HIID  IntegraterHeaderHIID  = HIID(AidUVData|
                                           AidAny|
                                           AidAny|
                                           AidPatch|
                                           AidAny|
                                           AidHeader|
                                           AidCorr|
                                           AidTimeslot);

  const HIID  IntegraterMessageHIID = HIID(AidUVData|
                                           AidAny|
                                           AidAny|
                                           AidPatch|
                                           AidAny|
                                           AidData|
                                           AidCorr|
                                           AidTimeslot|
                                           AidAny|
                                           AidAny);

  const HIID  IntegraterFooterHIID  = HIID(AidUVData|
                                           AidAny|  // UV-Set ID
                                           AidAny|  // Segment ID
                                           AidPatch|
                                           AidAny|  // Patch ID
                                           AidFooter|
                                           AidCorr|
                                           AidTimeslot);
};


//! Receives (time-frequency) sorted UV data and writes them to a file
/*!
  
 */
class UVPMessagesToFileWP: public WorkProcess
{
public:

  UVPMessagesToFileWP(const std::string& inputFilename,
                      const std::string& outputFilename,
                      bool               useSorter = true);

  virtual void init();
  virtual bool start();
  virtual int  receive(MessageRef &messageRef);
  
protected:
private:
  
  std::string itsInputFilename;
  BOIO        itsBOIO;          // BlockableObjectIO

  bool        itsIntegratorIsPresent;
  bool        itsSorterIsPresent;
  bool        itsIntegratorIsStarted;

  bool        itsUseSorter;
};



#endif // UVPMESSAGESTOFILEWP_H
