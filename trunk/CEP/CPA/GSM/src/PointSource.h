//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef GSM_POINTSOURCE_H
#define GSM_POINTSOURCE_H

// $Id$


#include <GSM/AbstractSource.h>

namespace GSM
{

//! Implementation of a simple point source
class PointSource:public AbstractSource
{
public:
  
   PointSource(double             ra        = 0,
               double             dec       = 0,
               unsigned int       catNumber = 0,
               const std::string& name      = "",
               const std::vector<double>& flux = std::vector<double>(4));

  ~PointSource();

  virtual void load(Table&       table,
                    unsigned int row);

  virtual void store(Table&       table,
                     unsigned int row);

  virtual unsigned int getParameters(std::vector<MesParm*> &parameters);

  //! Makes deep copy of parameters;
  /*! 
    Order: RA   (1)
           DEC  (1)
           Flux (NUMBER_OF_POLARIZATIONS times)

  */
  virtual unsigned int setParameters(const std::vector<MesParm*> &parameters);

  virtual unsigned int getNumberOfParameters() const;

  virtual void         getFluxExpressions(std::vector<MesExpr*> &flux);

protected:
private:

  MesParm*  itsFlux[NUMBER_OF_POLARIZATIONS];
};



}// namespace GSM

#endif // GSM_POINTSOURCE_H
