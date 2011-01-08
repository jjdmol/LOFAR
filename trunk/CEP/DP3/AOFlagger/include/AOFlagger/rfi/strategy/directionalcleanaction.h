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
#ifndef RFI_DIRECTIONALCLEAN_ACTION_H
#define RFI_DIRECTIONALCLEAN_ACTION_H

#include <iostream>

#include <AOFlagger/util/ffttools.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/action.h>
#include <AOFlagger/rfi/strategy/actionblock.h>

namespace rfiStrategy {

	class DirectionalCleanAction : public Action
	{
		public:
			DirectionalCleanAction() : Action()
			{
			}
			virtual std::string Description()
			{
				return "Directional cleaning";
			}
			virtual ActionType Type() const { return DirectionalCleanActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				for(unsigned y=0;y<artifacts.ContaminatedData().ImageHeight();++y)
				{
					performFrequency(artifacts, y);
				}
			}
		private:
			void performFrequency(ArtifactSet &artifacts, unsigned y)
			{
			}

			unsigned findStrongestComponent(Image2DCPtr real, Image2DCPtr imaginary, unsigned y)
			{
				const unsigned width = real->Width();
				numl_t maxSq = real->Value(0, y) * real->Value(0, y) +
					imaginary->Value(0, y) * imaginary->Value(0, y);
				unsigned index = 0;
				for(unsigned x=1;x<width;++x)
				{
					numl_t val = real->Value(x, y) * real->Value(x, y) +
					imaginary->Value(x, y) * imaginary->Value(x, y);
					if(val > maxSq) {
						val = maxSq;
						index = 0;
					}
				}
				return index;
			}
	};

} // namespace

#endif // RFI_DIRECTIONALCLEAN_ACTION_H
