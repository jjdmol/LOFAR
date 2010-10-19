//#  Format.h: defines the format of the RAW datafile
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id: Storage_main.cc 12953 2009-03-26 17:10:42Z nieuwpoort $

#ifndef LOFAR_STORAGEFORMAT_H
#define LOFAR_STORAGEFORMAT_H

#define ENDIANNESS      0      // data is bigendian

#include <Interface/Mutex.h>
#include <Interface/Parset.h>
#include <casa/aips.h>
#include <tables/Tables/Table.h>

#include <Storage/Format.h>

//# Forward Declarations
namespace casa
{
  class MPosition;
  class MeasurementSet;
  template<class T> class Block;
}


namespace LOFAR {
namespace RTCP {

class MeasurementSetFormat : public Format
{
 public:
  enum Endianness {
    BigEndian = 0,
    LittleEndian
  };

  MeasurementSetFormat(const Parset *, const uint32 alignment = 1);
  ~MeasurementSetFormat();

  void addSubband(unsigned subband);

 private:
  const Parset *itsPS;

  unsigned itsNrAnt;
  uint32   itsNrTimes;

  double itsStartTime;
  double itsTimeStep;
  
  vector<string> stationNames;
  vector<double> antPos;

  casa::MeasurementSet* itsMS;
/*   casa::Table* itsMS; */
  uint32 itsAlignment;

  static Mutex sharedMutex;

  void createMSTables(unsigned subband);
  void createMSMetaFile(unsigned subband);

  void fillFeed();
  void fillAntenna(const casa::Block<casa::MPosition>& antMPos);
  void fillField(unsigned subband);
  void fillPola();
  void fillDataDesc();
  void fillSpecWindow(unsigned subband);
  void fillObs();
};
  
} //RTCP
} //LOFAR
#endif // LOFAR_STORAGEFORMAT_H
