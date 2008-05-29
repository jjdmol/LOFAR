/// @file
/// @brief Class representing a global step in the MWSpec composite pattern
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCONTROL_MWGLOBALSPEC_H
#define LOFAR_MWCONTROL_MWGLOBALSPEC_H

//# Includes
#include <MWControl/MWSpec.h>
#include <MWCommon/MWGlobalStep.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcontrol
  /// @brief Class representing a global step in the MWSpec composite pattern.

  /// This is the class representing a global step in the MWSpec composite
  /// pattern (see Design Patterns, Gamma et al, 1995).
  /// It holds the information about the step in a ParameterSet object.

  class MWGlobalSpec : public MWGlobalStep
  {
  public:
    /// Default constructor.
    MWGlobalSpec();

    /// Construct from the given .parset file.
    /// Unspecified items are taken from the parent specification.
    MWGlobalSpec(const std::string& name,
		 const ParameterSet& parset,
		 const MWSpec* parent);

    virtual ~MWGlobalSpec();

    // Create as a new MWStep object.
    static MWStep::ShPtr create();

    /// Register the create function in the MWStepFactory.
    static void registerCreate();

    /// Clone the step object.
    virtual MWGlobalSpec* clone() const;

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
