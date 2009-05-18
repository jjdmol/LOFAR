//#  Format.h: defines the format of the RAW datafile
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id: Storage_main.cc 12953 2009-03-26 17:10:42Z nieuwpoort $

#ifndef LOFAR_STORAGEFORMAT_H
#define LOFAR_STORAGEFORMAT_H


#define FORMATVERSION   1              
#define ENDIANNESS      0      // data is bigendian
#define WEIGHTSIZE      2      // weights are 2 byted short per 4 pols
			       // (#samples correlated)

#include <Interface/Parset.h>
#include <Interface/PipelineOutput.h>
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

  MeasurementSetFormat(const Parset *);
  ~MeasurementSetFormat();

  void addSubband(unsigned subband);

 private:
  const Parset *itsPS;
  const PipelineOutputSet itsPipelineOutputSet;

  unsigned itsNrOutputs;
  unsigned itsNrAnt;
  uint32   itsNrTimes;

  double itsStartTime;
  double itsTimeStep;
  
  vector<string> stationNames;
  vector<double> antPos;

  casa::MeasurementSet* itsMS;
/*   casa::Table* itsMS; */

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
