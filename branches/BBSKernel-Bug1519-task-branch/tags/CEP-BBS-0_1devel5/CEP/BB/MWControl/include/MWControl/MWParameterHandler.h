/// @file
/// @brief Handle the master-worker part of a .parset file
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCONTROL_MWPARAMETERHANDLER_H
#define LOFAR_MWCONTROL_MWPARAMETERHANDLER_H

#include <MWCommon/ParameterHandler.h>

namespace LOFAR { namespace CEP {

  /// Forward Declarations
  class MWMultiSpec;
  class MWStrategySpec;

  /// @ingroup mwcontrol
  /// @brief Handle the master-worker part of a .parset file

  /// This class handles the processing of a .parset file
  /// It has functions to retrieve the specific master-worker info
  /// from the .parset file.
  /// These can be data set info, strategy specifications (MWStrategySpec),
  /// and step specifications (MWSpec).

  class MWParameterHandler: public ParameterHandler
  {
  public:
    explicit MWParameterHandler (const ParameterSet&);

    /// Get the name of the data set.
    std::string getDataSetName() const;

    /// Get the number of data parts.
    int getNParts() const;

    /// Get all strategies specifications from the parameters.
    std::vector<MWStrategySpec> getStrategies() const;

    /// Get all step specifications of a strategy from the parameters.
    MWMultiSpec getSteps (const std::string& name) const;
  };

}} /// end namespaces

#endif
