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



#ifndef GSM_SKYMODEL_H
#define GSM_SKYMODEL_H

#include <vector>

#include <MNS/MeqPointSource.h>

#include <GSM/AbstractSource.h>
#include <GSM/PointSource.h>


class Table;


namespace GSM {

//! Manages pointers to all sources in the sky/observation
class SkyModel
{
public:
  
  //! Constructor
   SkyModel();
   SkyModel(const Table& table);
  ~SkyModel();

  //! Adds sources to the model from an Aips++ table.
  void load(const Table& table);

  //! Stores the entire model in an Aips++ table.
  /*! At the moment, only point sources are stored. there is no file
    format yet for extended sources. I currently like to let each type
    of source have its own (sub?)table.
  */
  void store(Table& table)const;
  
  //! Returns pointers to all point sources in the model.
  /*! \param result Source pointers are appended to "result".
    \returns the number of sources that were added to "result".
   */
  unsigned int getPointSources(std::vector<GSM::PointSource*> &result);

  //! Returns array of MeqPointSources.
  /*!
      MeqPointSources contain pointers to the IQUV, Ra and Dec
      expressions of the PointSources.
   */
  unsigned int getPointSources(std::vector<MeqPointSource> &result);

  //! Returns pointers to all non-point sources in the model.
  /*! \param result should be an empty vector.
  unsigned int getExtendedSources(std::vector<GSM::AbstractSource*> &result);
  */

  //! Adds a source to the sky model.
  /*!  \param source is a pointer to the instance that is to be added
    to the model. The sky model assumes full responsibility for
    destructing that object. DO NOT DELETE IT YOURSELF!
   */
  void         add(AbstractSource* source);

protected:
private:

  std::vector<AbstractSource*> itsSources;
};

} // End of namespace GSM


#endif // GSM_SKYMODEL_H
