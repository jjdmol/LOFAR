/// @file
/// @brief Class representing a multi step in the MWSpec composite pattern
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCONTROL_MWMULTISPEC_H
#define LOFAR_MWCONTROL_MWMULTISPEC_H

//# Includes
#include <MWControl/MWSpec.h>
#include <MWCommon/MWMultiStep.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcontrol
  /// @brief Class representing a multi step in the MWSpec composite pattern.

  /// This is the class representing a multi step in the MWSpec composite
  /// pattern (see Design Patterns, Gamma et al, 1995).
  /// It holds the information about the step in a ParameterSet object.

  class MWMultiSpec : public MWMultiStep
  {
  public:
    /// Default constructor.
    MWMultiSpec();

    /// Construct from the given .parset file.
    /// Unspecified items are taken from the parent specification.
    MWMultiSpec(const std::string& name,
		const ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWMultiSpec();

    // Create as a new MWStep object.
    static MWStep::ShPtr create();

    /// Clone the step object.
    virtual MWMultiSpec* clone() const;

    /// Print the contents and type. Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

  private:
    MWSpec itsSpec;
  };

}} /// end namespaces

#endif
