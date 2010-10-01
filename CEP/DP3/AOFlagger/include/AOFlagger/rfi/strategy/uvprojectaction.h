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
#ifndef RFI_UV_PROJECT_ACTION
#define RFI_UV_PROJECT_ACTION

#include "artifactset.h"
#include "actionblock.h"

#include <AOFlagger/rfi/strategy/action.h>

#include <AOFlagger/msio/timefrequencydata.h>

namespace rfiStrategy {

	class UVProjectAction : public Action
	{
		public:
			UVProjectAction() : Action(), _directionRad(1.0/4.0 * M_PIn)
			{
			}
			virtual std::string Description()
			{
				return "UV-project";
			}
			virtual ActionType Type() const { return UVProjectActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData &data = artifacts.ContaminatedData();

				for(size_t imageIndex = 0; imageIndex != data.ImageCount(); ++imageIndex)
				{
					Image2DCPtr image = data.GetImage(imageIndex);
					Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
	
					const num_t cosRotate = cosn(_directionRad);
					const num_t sinRotate = sinn(_directionRad);
					const size_t width = image->Width();
	
					num_t maxU = -1e20, minU = 1e20;
					for(size_t x=0;x<width;++x)
					{
						const UVW &uvw = artifacts.MetaData()->UVW()[x];
						const num_t uProject = uvw.u * cosRotate - uvw.v * sinRotate;
						if(uProject > maxU) maxU = uProject;
						if(uProject < minU) minU = uProject;
					}
	
					size_t nextX = 0;
					size_t firstX = 0;
					bool forwardDirection = true;
					for(size_t xI=0;xI<width;++xI)
					{
						size_t x;
						if(forwardDirection)
							x = xI;
						else
							x = width - 1 - xI;
						const UVW &uvw = artifacts.MetaData()->UVW()[x];
						const num_t vProject = uvw.u * sinRotate + uvw.v * cosRotate;
						num_t uProject;
						if(vProject >= 0.0)
							uProject = uvw.u * cosRotate - uvw.v * sinRotate;
						else
							uProject = -uvw.u * cosRotate + uvw.v * sinRotate;
						size_t xProject = (size_t) ((uProject-minU) / (maxU-minU) * (num_t) width) % width;
						if(xI != 0)
						{
							while(nextX != (xProject + 1)%width)
							{
								for(size_t y=0;y<image->Height();++y)
									newImage->SetValue(nextX, y, image->Value(x, y));
	
								nextX = (nextX + 1) % width;
							}
						} else {
							firstX = xProject;
							nextX = xProject;
						}
					}
					if(forwardDirection)
					{
						for(size_t x=nextX;x!=firstX;x=(x+1)%width)
						{
							for(size_t y=0;y<image->Height();++y)
								newImage->SetValue(x, y, image->Value(0, y));
						}
					}

					data.SetImage(imageIndex, newImage);
				}

				/*TimeFrequencyData *contaminatedData =
					TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), data);
				contaminatedData->SetMask(artifacts.ContaminatedData());
				artifacts.SetContaminatedData(*contaminatedData);
				delete contaminatedData;*/
			}
		private:
			num_t _directionRad;
	};

} // namespace

#endif // RFI_UV_PROJECT_ACTION
