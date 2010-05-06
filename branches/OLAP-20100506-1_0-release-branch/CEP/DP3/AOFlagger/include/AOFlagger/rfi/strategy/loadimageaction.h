/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef LOAD_IMAGE_ACTION_H
#define LOAD_IMAGE_ACTION_H

#include "action.h"

#include "../../msio/timefrequencyimager.h"

namespace rfiStrategy {
	
	class LoadImageAction : public Action
	{
		public:
			LoadImageAction() : _dataKind(ObservedData), _polarisations(ReadAllPol) 
			{
			}
			virtual std::string Description()
			{
				return "Load image";
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return LoadImageActionType; }

			void SetReadAllPolarisations() throw()
			{
				_polarisations = ReadAllPol;
			}
			void SetReadDipoleAutoPolarisations() throw()
			{
				_polarisations = ReadAutoPol;
			}
			void SetReadStokesI() throw()
			{
				_polarisations = ReadSI;
			}
			bool ReadAllPolarisations() const { return _polarisations == ReadAllPol; }
			bool ReadDipoleAutoPolarisations() const { return _polarisations == ReadAutoPol; }
			bool ReadStokesI() const { return _polarisations == ReadSI; }

			enum DataKind DataKind() const { return _dataKind; }
			void SetDataKind(enum DataKind dataKind) { _dataKind = dataKind; }
		private:
			enum DataKind _dataKind;
			enum Polarisations { ReadAllPol, ReadAutoPol, ReadSI }
				_polarisations;
	};

}

#endif
