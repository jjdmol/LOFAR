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
//#  $Id: DipoleModel.h 6818 2005-10-20 09:31:47Z cvs $

#ifndef DIPOLEMODEL_H_
#define DIPOLEMODEL_H_

#include <complex>
#include <blitz/array.h>
#include <map>

namespace LOFAR {
namespace CAL {

  class DipoleModel
  {
  public:
    DipoleModel(std::string name, const blitz::Array<std::complex<double>, 4>& sens);
    virtual ~DipoleModel();

    /**
     * Get the name of the dipole model.
     */
    std::string getName() const { return m_name; }

    /**
     * Get reference to the dipole model array.
     * @return The dipole model with dimensions:
     * 2 (0=phi, 1=theta) x 50 (l) x 50 (m) x 24 (frequency planes)
     */
    blitz::Array<std::complex<double>, 4> getModel() const { return m_sens; }

  private:

    friend class DipoleModelLoader; // to access m_sens

    std::string                           m_name;
    blitz::Array<std::complex<double>, 4> m_sens;
  };

  class DipoleModels
  {
  public:
    DipoleModels();
    virtual ~DipoleModels();

    /**
     * Get all models from file or database.
     * @param url the resource location (e.g. filename or database table),
     * currently only filename is supported.
     */
    void getAll(std::string url);

    /**
     * Get model by name.
     */
    const DipoleModel* getByName(std::string name) { return m_models[name]; }
  private:
    std::map<std::string, const DipoleModel*> m_models;
  };

}; // namespace CAL
}; // namespace LOFAR

#endif /* DIPOLEMODEL_H_ */

