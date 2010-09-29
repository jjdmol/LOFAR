//# LofarGridder.h: Gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen <diepen at astron dot nl>

#ifndef LOFAR_LOFARGRIDDER_LOFARGRIDDER_H
#define LOFAR_LOFARGRIDDER_LOFARGRIDDER_H

#include <gridding/TableVisGridder.h>
#include <Common/ParameterSet.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  // @brief Gridder for LOFAR data correcting for DD effects
  //
  // @ingroup testgridder

  class LofarGridder : public askap::synthesis::IVisGridder
  {
  public:

    // Construct from the given parset.
    explicit LofarGridder (const ParameterSet&);

    // Clone this Gridder
    virtual askap::synthesis::IVisGridder::ShPtr clone();

    virtual ~LofarGridder();

    // @brief Function to create the gridder from a parset.
    // This function will be registered in the gridder registry.
    static askap::synthesis::IVisGridder::ShPtr makeGridder
    (const ParameterSet&);
    // @brief Return the (unique) name of the gridder.
    static const std::string& gridderName();

    // @brief Register the gridder create function with its name.
    static void registerGridder();

    // @brief Initialise the gridding
    // @param axes axes specifications
    // @param shape Shape of output image: u,v,pol,chan
    // @param dopsf Make the psf?
    virtual void initialiseGrid (const askap::scimath::Axes& axes,
                                 const casa::IPosition& shape,
                                 const bool dopsf=true);

    // @brief Grid the visibility data.
    // @param acc const data accessor to work with
    // @note a non-const adapter is created behind the scene. If no on-the-fly
    // visibility correction is performed, this adapter is equivalent to the
    // original const data accessor.
    virtual void grid (askap::synthesis::IConstDataAccessor& acc);
      
    // Form the final output image
    // @param out Output double precision image or PSF
    virtual void finaliseGrid (casa::Array<double>& out);

    // @brief Calculate weights image
    // @details Form the sum of the convolution function squared, 
    // multiplied by the weights for each different convolution 
    // function. This is used in the evaluation of the position
    // dependent sensitivity
    // @param out Output double precision sum of weights images
    virtual void finaliseWeights (casa::Array<double>& out);

    // @brief Initialise the degridding
    // @param axes axes specifications
    // @param image Input image: cube: u,v,pol,chan
    virtual void initialiseDegrid (const askap::scimath::Axes& axes,
                                   const casa::Array<double>& image);

    // @brief Make context-dependant changes to the gridder behaviour
    // @param context context
    virtual void customiseForContext (casa::String context);
      
    // @brief assign weights
    // @param viswt shared pointer to visibility weights
    virtual void initVisWeights (askap::synthesis::IVisWeights::ShPtr viswt);
      
    // @brief Degrid the visibility data.
    // @param[in] acc non-const data accessor to work with  
    virtual void degrid (askap::synthesis::IDataAccessor& acc);

    // @brief Finalise degridding.
    virtual void finaliseDegrid();

    // @brief Initialise the indices
    // @param[in] acc const data accessor to work with
    ///    virtual void initIndices (const askap::synthesis::IConstDataAccessor& acc);

    // @brief Correct for gridding convolution function
    // @param image image to be corrected
    ///    virtual void correctConvolution(casa::Array<double>& image);
				
  protected:
    // Initialize convolution function
    // @param[in] acc const data accessor to work with
    ///    virtual void initConvolutionFunction
    ///    (const askap::synthesis::IConstDataAccessor& acc);

  private:
    // Initialize the corrections.
    void initCorrections (const askap::synthesis::IConstDataAccessor& acc);


    //# Data members.
    // Is the gridder correction initialized?
    bool itsInitialized;
    // Nr of frequency channels to average.
    uint itsFreqAvg;
    // Nr of time slots to average.
    uint itsTimeAvg;
    // The effects to correct for.
    vector<string> itsCorrect;
    // The ParmDB to use.
    string itsParmDBName;
    // The gridder to use after correcting and averaging.
    //# Unfortunatelty we need to use TableVisGridder because the
    //# function getImageCntre is available only there.
    askap::synthesis::IVisGridder::ShPtr itsIGridder;
    askap::synthesis::TableVisGridder* itsGridder;
  };

} //# end namespace

#endif
