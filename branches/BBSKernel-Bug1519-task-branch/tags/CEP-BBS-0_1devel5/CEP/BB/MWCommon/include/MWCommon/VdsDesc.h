/// @file
/// @brief Describe an entire visibility data set
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCOMMON_VDSDESC_H
#define LOFAR_MWCOMMON_VDSDESC_H

//# Includes
#include <MWCommon/VdsPartDesc.h>
#include <MWCommon/ParameterHandler.h>
#include <casa/Utilities/Regex.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcommon
  /// @brief Describe an entire visibility data set

  /// This class holds the description of an entire visibility data set (VDS).
  /// In VdsPartDesc objects it describes the parts it consists of and
  /// on which file systems they are located.
  /// A VdsPartDesc object is also used to describe the entire VDS.
  /// Furthermore it contains the names of all antennae, which can be used
  /// to map the antenna name to the antenna number when a selection on
  /// antenna names is done.
  ///
  /// Currently the information is made persistent in a LOFAR .parset file.
  /// In the future it needs to use the Centrol Processor Resource Manager.

  class VdsDesc
  {
  public:
    /// Construct with a description of the entire visibility data set.
    explicit VdsDesc (const VdsPartDesc&);

    /// Construct from the given parameterset.
    /// @{
    explicit VdsDesc (const std::string& parsetName);
    explicit VdsDesc (const ParameterSet& parset)
      { init (parset); }
    /// @}

    /// Add a part.
    void addPart (const VdsPartDesc& part)
      { itsParts.push_back (part); }

    /// Get the description of the parts.
    const std::vector<VdsPartDesc>& getParts() const
      { return itsParts; }

    /// Get the description of the VDS.
    const VdsPartDesc& getDesc() const
      { return itsDesc; }

    /// Write it in parset format.
    void write (std::ostream& os) const;

  private:
    /// Fill the object from the given parset file.
    void init (const ParameterSet& parset);

    VdsPartDesc              itsDesc;
    std::vector<VdsPartDesc> itsParts;
  };

}} /// end namespaces

#endif
