
// Copyright notice should go here

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
