// Copyright Notice

#if !defined(UVPDATATRANSFERWP_H)
#define UVPDATATRANSFERWP_H

// $Id$



#include <OCTOPUSSY/WorkProcess.h>

#include <vector>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

#include <UVPDataHeader.h>
#include <UVPDataAtom.h>

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
  UVPDataTransferWP(int correlation,
                    int baseline,
                    int patchID);

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

  //! Returns the number of rows received up to now.
  unsigned int size() const;

  //! Returns a const pointer to a row.
  /*! \param rowIndex is the index of the row that you are interested
    in. One should make sure yourself that rowIndex < size(). rowIndex
    is zero based.
  */
  const UVPDataAtom *getRow(unsigned int rowIndex) const;


 protected:
 private:

  std::vector<UVPDataAtom> itsCachedData;

  int                     itsCorrelation;
  int                     itsBaseline;
  int                     itsPatchID; /* Zero based */
  
  bool                     itsHeaderIsReceived;
  UVPDataHeader            itsHeader;
  

  // Octopussy stuff
  HIID                     itsHeaderHIID;
  HIID                     itsDataHIID;
  HIID                     itsFooterHIID;
};


#endif // UVPDATATRANSFERWP_H
