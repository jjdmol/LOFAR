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

#include <iostream>

namespace rfiStrategy {

	class UVProjectAction : public Action
	{
		public:
			UVProjectAction() : Action(), _directionRad(-2.0/180.0*M_PI/*atann(600.0/(8.0*320.0))*/), _reverse(false), _onRevised(false), _onContaminated(true)
			{
			}
			virtual std::string Description()
			{
				if(_reverse)
					return "Reverse UV-project";
				else
					return "UV-project";
			}
			virtual ActionType Type() const { return UVProjectActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				if(_onContaminated)
					perform(artifacts.ContaminatedData(), artifacts.MetaData());
				if(_onRevised)
					perform(artifacts.RevisedData(), artifacts.MetaData());
			}
			num_t DirectionRad() const { return _directionRad; }
			void SetDirectionRad(num_t directionRad) { _directionRad = directionRad; }
			
			bool Reverse() const { return _reverse; }
			void SetReverse(bool reverse) { _reverse = reverse; }
			
			bool OnRevised() const { return _onRevised; }
			void SetOnRevised(bool onRevised) { _onRevised = onRevised; }
			
			bool OnContaminated() const { return _onContaminated; }
			void SetOnContaminated(bool onContaminated) { _onContaminated = onContaminated; }
		private:
			void perform(TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
			{
				for(size_t imageIndex = 0; imageIndex != data.ImageCount(); ++imageIndex)
				{
					Image2DCPtr image = data.GetImage(imageIndex);
					Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
	
					const long double cosRotate = cosl(_directionRad);
					const long double sinRotate = sinl(_directionRad);
					const size_t width = image->Width();
					
					long double conjugateSign;
					if((data.PhaseRepresentation() == TimeFrequencyData::ComplexRepresentation && (imageIndex%2)==1) || data.PhaseRepresentation() == TimeFrequencyData::ImaginaryPart)
						conjugateSign = -1.0;
					else
						conjugateSign = 1.0;

					// Find length of the major axis of the ellipse
					long double maxU = -1e20, minU = 1e20;
					for(size_t x=0;x<width;++x)
					{
						const UVW &uvw = metaData->UVW()[x];
						const long double uProjectUpper = uvw.u * cosRotate - uvw.v * sinRotate;
						if(uProjectUpper > maxU) maxU = uProjectUpper;
						if(uProjectUpper < minU) minU = uProjectUpper;
						const long double uProjectBottom = -uvw.u * cosRotate + uvw.v * sinRotate;
						if(uProjectBottom > maxU) maxU = uProjectBottom;
						if(uProjectBottom < minU) minU = uProjectBottom;
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
						const UVW &uvw = metaData->UVW()[x];
						const long double vProject = uvw.u * sinRotate + uvw.v * cosRotate;
						long double uProject, currentSign;
						if(vProject >= 0.0) {
							uProject = uvw.u * cosRotate - uvw.v * sinRotate;
							currentSign = 1.0;
						} else {
							uProject = -uvw.u * cosRotate + uvw.v * sinRotate;
							currentSign = conjugateSign;
						}
						size_t xProject = (size_t) ((uProject-minU) / (maxU-minU) * (long double) width) % width;
						if(xI != 0)
						{
							Set(newImage, nextX, image, x);
							// Solve rounding errors that might cause wrapping to occur at one point and
							// not at a later point
							if((int) xProject - (int) nextX > (int) width/2) xProject = (nextX+width-1)%width;
							while(nextX != (xProject + 1)%width)
							{
								Set(newImage, nextX, image, x);
								nextX = (nextX + 1) % width;
							}
						} else {
							firstX = xProject;
							nextX = xProject;
						}
					}
					if((firstX+width-nextX)%width < width/2)
					{
						for(size_t x=nextX;x!=firstX;x=(x+1)%width)
						{
							Set(newImage, x, image, 0);
						}
					}

					data.SetImage(imageIndex, newImage);
				}
			}
			
			void Set(Image2DPtr newImage, size_t xTo, Image2DCPtr image, size_t xFrom)
			{
				if(_reverse)
				{
					for(size_t y=0;y<image->Height();++y)
						newImage->SetValue(xFrom, y, image->Value(xTo, y));
				} else {
					for(size_t y=0;y<image->Height();++y)
						newImage->SetValue(xTo, y, image->Value(xFrom, y));
				}
			}
			
			num_t _directionRad;
			bool _reverse;
			bool _onRevised, _onContaminated;
	};

} // namespace

#endif // RFI_UV_PROJECT_ACTION
