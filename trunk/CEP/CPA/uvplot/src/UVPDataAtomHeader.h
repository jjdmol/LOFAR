// Copyright notice

#if !defined(UVPDATAATOMHEADER_H)
#define UVPDATAATOMHEADER_H

// $Id$

//! Simple class with public(!) members for managing data regarding the
//! location in the measurement set of UVPDataAtom objects.
/*!  Its primary purpose is to make looking up and storing UVPDataAtom
     objects faster by minimizing the number of parameters in function
     calls.  Pass objects of this class as a reference.  The
     datamembers are all zero based row indices in the corresponding
     tables.
 */

class UVPDataAtomHeader
{
 public:
  
  unsigned int itsFieldID;
  unsigned int itsPatchID;
  unsigned int itsIFR;
  unsigned int itsCorrelationIndex;
  unsigned int itsTimeslot;

  //! Constructor
  /*!
   */
  UVPDataAtomHeader(unsigned int timeslot         = 0,
                    unsigned int correlationIndex = 0,
                    unsigned int ifr              = 0,
                    unsigned int patchID          = 0,
                    unsigned int fieldID          = 0);
};


#endif // UVPDATAATOMHEADER_H
