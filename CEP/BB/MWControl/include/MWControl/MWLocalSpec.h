/// @file
/// @brief Class representing a local step in the MWSpec composite pattern
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCONTROL_MWLOCALSPEC_H
#define LOFAR_MWCONTROL_MWLOCALSPEC_H

//# Includes
#include <MWControl/MWSpec.h>
#include <MWCommon/MWLocalStep.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcontrol
  /// @brief Class representing a local step in the MWSpec composite pattern.

  /// This is the class representing a local step in the MWSpec composite
  /// pattern (see Design Patterns, Gamma et al, 1995).
  /// It holds the information about the step in a ParameterSet object.

  class MWLocalSpec : public MWLocalStep
  {
  public:
    /// Default constructor.
    MWLocalSpec();

    /// Construct from the given .parset file.
    /// Unspecified items are taken from the parent specification.
    MWLocalSpec(const std::string& name,
		const ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWLocalSpec();

    // Create as a new MWStep object.
    static MWStep::ShPtr create();

    /// Register the create function in the MWStepFactory.
    static void registerCreate();

    /// Clone the step object.
    virtual MWLocalSpec* clone() const;

    /// Give the (unique) class name of this MWStep.
    virtual std::string className() const;

    /// Get the parameter set.
    virtual ParameterSet getParms() const;

    /// Convert to/from blob.
    /// @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    /// @}

    /// Print the contents and type. Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWStepVisitor&) const;

  private:
    MWSpec itsSpec;
  };

}} /// end namespaces

#endif
