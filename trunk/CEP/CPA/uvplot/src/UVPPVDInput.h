// Copyright notice should go here

#if !defined(UVPPVDINPUT_H)
#define UVPPVDINPUT_H

// $Id$

#include <string>

#include <DMI/BOIO.h>

#include <UVPDataAtom.h>
#include <UVPDataSet.h>

//! Interface to PVD dataset
/*! Uses BOIO to read the data from a file */

class UVPPVDInput
{
public:
  //! Constructor
  UVPPVDInput(const std::string &filename);
  
  unsigned int numberOfAntennae() const;

  //  unsigned int numberOfPolarizations() const;

  unsigned int numberOfChannels() const;

  bool getDataAtoms(UVPDataSet* dataset,
                    unsigned int ant1,
                    unsigned int ant2);
  
protected:
private:

  unsigned int itsNumberOfAntennae;
  unsigned int itsNumberOfBaselines;
  unsigned int itsNumberOfPolarizations;
  unsigned int itsNumberOfChannels;
  UVPDataAtom  itsDataAtom;
  BOIO         itsBOIO;

};

#endif // UVPPVDInput
