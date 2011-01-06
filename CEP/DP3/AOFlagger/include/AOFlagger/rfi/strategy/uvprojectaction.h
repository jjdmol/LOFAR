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
#ifndef RFI_UV_PROJECT_ACTION_H
#define RFI_UV_PROJECT_ACTION_H

#include "artifactset.h"
#include "actionblock.h"

#include <AOFlagger/rfi/strategy/action.h>

#include <AOFlagger/rfi/uvprojection.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <iostream>

namespace rfiStrategy {

	class UVProjectAction : public Action
	{
		public:
			UVProjectAction() : Action(), _directionRad(68.0/180.0*M_PI/*atann(600.0/(8.0*320.0))*/), _etaParameter(0.2), _reverse(false), _onRevised(false), _onContaminated(true)
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
				if(data.ImageCount()!=1)
					throw std::runtime_error("UV Projection can be applied on single images only");
				Image2DCPtr image = data.GetImage(0);

				const size_t
					width = image->Width(),
					destWidth = width;
				Image2DPtr
					destination = Image2D::CreateEmptyImagePtr(destWidth, image->Height()),
					weights = Image2D::CreateEmptyImagePtr(destWidth, image->Height());
				numl_t
					*rowValues = new numl_t[width],
					*rowUPositions = new numl_t[width],
					*rowVPositions = new numl_t[width];
				bool
					*rowSignsNegated = new bool[width];

				for(unsigned y=0;y<data.ImageHeight();++y)
				{
					UVProjection::Project(image, metaData, y, rowValues, rowUPositions, rowVPositions, rowSignsNegated, _directionRad, data.IsImaginary());

					// Find the point closest to v=0
					numl_t
						vDist = fabsnl(rowVPositions[0]),
						leftDist = 0.0,
						rightDist = 0.0;
					size_t vZeroPos = 0;
					for(unsigned i=1;i<width;++i)
					{
						if(fabsnl(rowVPositions[i]) < vDist)
						{
							vDist = fabsnl(rowVPositions[i]);
							vZeroPos = i;
						}
						if(rowUPositions[i] < leftDist)
							leftDist = rowUPositions[i];
						if(rowUPositions[i] > rightDist)
							rightDist = rowUPositions[i];
					}
					numl_t maxDist = rightDist > -leftDist ? rightDist : -leftDist;
					std::cout << "MaxDist=" << maxDist << std::endl;

					for(unsigned t=0;t<width-1;++t)
					{
						unsigned
							t1 = t,
							t2 = t+1;
						double
							u1 = rowUPositions[t],
							u2 = rowUPositions[t+1];
						if(u1 > u2)
						{
							u1 = rowUPositions[t+1];
							u2 = rowUPositions[t];
							t1 = t+1;
							t2 = t;
						}
						if(u2 - u1 >= maxDist)
						{
							numl_t midValue = (rowValues[t1]+rowValues[t2])/2.0;
							Interpolate(destination, weights, leftDist, rightDist, leftDist, u1, midValue, rowValues[t2], y);
							Interpolate(destination, weights, leftDist, rightDist, u2, rightDist, rowValues[t1], midValue, y);
						} else {
							Interpolate(destination, weights, leftDist, rightDist, u1, u2, rowValues[t1], rowValues[t2], y);
						}
					}
					unsigned
						rangeStart = (unsigned) roundn(_etaParameter * (num_t) width / 2.0),
						rangeEnd = width - rangeStart;
					for(unsigned x=0;x<width;++x)
					{
						if(x > rangeStart && x < rangeEnd)
						{
							if(weights->Value(x, y) != 0.0)
								destination->SetValue(x, y, destination->Value(x, y) / weights->Value(x, y));
							else
								AOLogger::Warn << "UV projection did not fill entire range\n";
						} else {
							destination->SetValue(x, y, 0.0);
						}
					}
				}

				delete[] rowVPositions;
				delete[] rowUPositions;
				delete[] rowValues;
				delete[] rowSignsNegated;

				data.SetImage(0, destination);
			}

			void Interpolate(Image2DPtr destination, Image2DPtr weights, numl_t leftDist, numl_t rightDist, numl_t u1, numl_t u2, numl_t v1, numl_t v2, unsigned y) const
			{
				std::cout << "Inp: " << u1 << "-" << u2;
				int
					width = destination->Width(),
					left = (int) ((u1 - leftDist) * (numl_t) width / (rightDist-leftDist)),
					right = (int) ((u2 - leftDist) * (numl_t) width / (rightDist-leftDist));
				if(left < 0) left = 0;
				if(right >= width) right = width;
				if(right - left < 1 && left+1 < width) right = left+1;
				int count = right-left;
				std::cout << " pix " << left << "-" << right << std::endl;
				for(int x=left;x<right;++x)
				{
					numl_t value = v1 + ((v2-v1)*(numl_t) (x-left)/(numl_t) count);
					destination->SetValue(x, y, destination->Value(x, y) + value);
					weights->SetValue(x, y, weights->Value(x, y) + 1.0);
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
			
			num_t _directionRad, _etaParameter;
			bool _reverse;
			bool _onRevised, _onContaminated;
	};

} // namespace

#endif // RFI_UV_PROJECT_ACTION_H
