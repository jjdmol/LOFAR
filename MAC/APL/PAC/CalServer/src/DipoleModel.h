//#  -*- mode: c++ -*-
//#  DipoleModel.h: definition of the Dipole Model (sensitivity of the Dipole)
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef DIPOLEMODEL_H_
#define DIPOLEMODEL_H_

#include <complex>
#include <blitz/array.h>

namespace CAL
{
  class DipoleModel
  {
  public:
    DipoleModel(std::string name);
    virtual ~DipoleModel();

    /**
     * Get the name of the dipole model.
     */
    std::string getName() const { return m_name; }

    /**
     * Get reference to the dipole model array.
     */
    blitz::Array<std::complex<double>, 3>& getModel() { return m_sens; }

  private:

    friend class DipoleModelLoader; // to access m_sens

    std::string                           m_name;
    blitz::Array<std::complex<double>, 3> m_sens;
  };

  class DipoleModelLoader
  {
  public:
    
    /**
     * Load dipole model from file.
     *
     * File format: Blitz++ array of appropriate dimensions.
     * 
     * @param filename Name of the input file.
     * @return The newly allocated dipole model instance.
     */
    static /*const*/ DipoleModel* loadFromBlitzString(std::string name, std::string array);
  };

};

#endif /* DIPOLEMODEL_H_ */

