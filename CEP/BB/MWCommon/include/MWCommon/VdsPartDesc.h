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
#include <MWCommon/ParameterHandler.h>
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
    explicit VdsPartDesc (const ParameterSet&);

    /// Set VDS name and file system.
    void setName (const std::string& name, const std::string& fileSys);

    /// Set the start and end time.
    void setTimes (double startTime, double endTime, double stepTime);

    /// Add a band.
    void addBand (int nchan, double startFreq, double endFreq);

    /// Write it in parset format.
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

  private:
    std::string itsName;       //# full name of the VDS
    std::string itsFileSys;    //# name of file system the VDS resides on
    double      itsStartTime;
    double      itsEndTime;
    double      itsStepTime;
    std::vector<int>    itsNChan;        //# nr of channels per band
    std::vector<double> itsStartFreqs;   //# start freq of each channel
    std::vector<double> itsEndFreqs;     //# end freq of each channel
  };
    
}} /// end namespaces

#endif
