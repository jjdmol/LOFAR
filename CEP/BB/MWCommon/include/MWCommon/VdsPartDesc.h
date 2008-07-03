/// @file
/// @brief Description of a visibility data set or part thereof.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCOMMON_VDSPARTDESC_H
#define LOFAR_MWCOMMON_VDSPARTDESC_H

//# Includes
#include <APS/ParameterSet.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcommon
  /// @brief Description of a visibility data set or part thereof.

  /// This class holds the description of a visibility data set (VDS) part.
  /// It defines the name of the part and on which file system it is located.
  /// Using the ClusterDesc object it can be derived on which node this
  /// VDS part can be processed best. This is done by the WorkersDesc
  /// class.
  ///
  /// The description of the VDS also contains info about the time,
  /// frequency, and baseline domain of the visibility data.
  ///
  /// The information is made persistent in a LOFAR .parset file.

  class VdsPartDesc
  {
  public:
    /// Construct an empty object.
    VdsPartDesc()
      : itsStartTime(0), itsStepTime(1)
      {}

    /// Construct from the given parameterset.
    explicit VdsPartDesc (const ACC::APS::ParameterSet&);

    /// Set VDS name and file system.
    void setName (const std::string& name, const std::string& fileSys);

    /// Set the start and end time.
    void setTimes (double startTime, double endTime, double stepTime);

    /// Add a band.
    // <group>
    void addBand (int nchan, double startFreq, double endFreq);
    void addBand (int nchan, const vector<double>& startFreq,
		  const vector<double>& endFreq);
    // </group>

    // Add an extra parameter. It is added to the subset 'Extra.'.
    // If the paramter already exists, it is replaced.
    void addParm (const std::string& key, const std::string& value)
      { return itsParms.add (key, value); }

    // Get access to the extra parameters.
    const ACC::APS::ParameterSet& getParms() const
      { return itsParms; }

    // Clear the extra parameters.
    void clearParms()
      { itsParms.clear(); }

    /// Write the VdsPartDesc object in parset format.
    void write (std::ostream& os, const std::string& prefix) const;

    /// Get the values.
    /// @{
    const std::string& getName() const
      { return itsName; }
    const std::string& getFileSys() const
      { return itsFileSys; }
    double getStartTime() const
      { return itsStartTime; }
    double getEndTime() const
      { return itsEndTime; }
    double getStepTime() const
      { return itsStepTime; }
    int getNBand() const
      { return itsNChan.size(); }
    const std::vector<int>& getNChan() const
      { return itsNChan; }
    const std::vector<double>& getStartFreqs() const
      { return itsStartFreqs; }
    const std::vector<double>& getEndFreqs() const
      { return itsEndFreqs; }
    /// @}

  // Put/get the object to/from a blob.
  // <group>
  BlobOStream& toBlob (BlobOStream&) const;
  BlobIStream& fromBlob (BlobIStream&);
  // </group>

  private:
    std::string itsName;       //# full name of the VDS
    std::string itsFileSys;    //# name of file system the VDS resides on
    double      itsStartTime;
    double      itsEndTime;
    double      itsStepTime;
    std::vector<int32>  itsNChan;        //# nr of channels per band
    std::vector<double> itsStartFreqs;   //# start freq of each channel
    std::vector<double> itsEndFreqs;     //# end freq of each channel
    ACC::APS::ParameterSet itsParms;     //# extra parameters
  };
    
  // Put/get the object to/from a blob.
  // <group>
    inline BlobOStream& operator<< (BlobOStream& bs, const VdsPartDesc& vpd)
    { return vpd.toBlob (bs); }
    inline BlobIStream& operator>> (BlobIStream& bs, VdsPartDesc& vpd)
    { return vpd.fromBlob (bs); }
  // </group>

}} /// end namespaces

#endif
