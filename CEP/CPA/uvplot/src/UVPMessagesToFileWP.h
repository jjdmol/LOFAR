
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

  HIID        itsSorterHeaderHIID;
  HIID        itsSorterMessageHIID;

  HIID        itsIntegraterHeaderHIID;
  HIID        itsIntegraterMessageHIID;

  bool        itsUseSorter;
};



#endif // UVPMESSAGESTOFILEWP_H
