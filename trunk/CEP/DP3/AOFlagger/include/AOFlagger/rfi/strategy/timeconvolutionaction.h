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
#ifndef RFI_TIME_CONVOLUTION_ACTION
#define RFI_TIME_CONVOLUTION_ACTION

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/actionblock.h>

#include <AOFlagger/rfi/strategy/action.h>

#include <AOFlagger/rfi/thresholdtools.h>

namespace rfiStrategy {

	class TimeConvolutionAction : public Action
	{
		public:
			enum Operation { SincOperation, ProjectedSincOperation };
			
			TimeConvolutionAction() : Action(), _operation(ProjectedSincOperation), _sincSize(10000.0), _directionRad(M_PI*(-92.0/180.0))
			{
			}
			virtual std::string Description()
			{
				return "Time convolution";
			}
			virtual ActionType Type() const { return TimeConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				Image2DCPtr newImage;
				switch(_operation)
				{
					case SincOperation:
						newImage = PerformSincOperation(artifacts);
						break;
					case ProjectedSincOperation:
						newImage = PerformProjectedSincOperation(artifacts, listener);
						break;
				}
				
				TimeFrequencyData newRevisedData = TimeFrequencyData(artifacts.ContaminatedData().PhaseRepresentation(), artifacts.ContaminatedData().Polarisation(), newImage);
				newRevisedData.SetMask(artifacts.RevisedData());

				TimeFrequencyData *contaminatedData =
					TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
				contaminatedData->SetMask(artifacts.ContaminatedData());
				artifacts.SetRevisedData(newRevisedData);
				artifacts.SetContaminatedData(*contaminatedData);
				delete contaminatedData;
			}
			
			enum Operation Operation() const { return _operation; }
			void SetOperation(enum Operation operation) { _operation = operation; }

			num_t DirectionRad() const { return _directionRad; }
			void SetDirectionRad(num_t directionRad) { _directionRad = directionRad; }
			
			num_t SincScale() const { return _sincSize; }
			void SetSincScale(num_t sincSize) { _sincSize = sincSize; }
private:
			Image2DPtr PerformSincOperation(ArtifactSet &artifacts) const
			{
				num_t sincScale = ActualSincScale(artifacts);
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()*3];
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();
				num_t sign;
				if(data.PhaseRepresentation() == TimeFrequencyData::RealPart)
					sign = 1.0;
				else if(data.PhaseRepresentation() == TimeFrequencyData::ImaginaryPart)
					sign = -1.0;
				else
					throw BadUsageException("Data is not real or imaginary");
				for(unsigned y=0;y<image->Height();++y)
				{
					for(unsigned x=0;x<width;++x) {
						row[x] = sign * image->Value(x, y);
						row[x+width] = image->Value(x, y);
						row[x+2*width] = sign * image->Value(x, y);
					}
					ThresholdTools::OneDimensionalSincConvolution(row, width*3, sincScale);
					for(unsigned x=0;x<width;++x)
						newImage->SetValue(x, y, row[x+width]);
				}
				delete[] row;
				
				return newImage;
			}
			
			Image2DPtr PerformProjectedSincOperation(ArtifactSet &artifacts, class ProgressListener &listener) const
			{
				num_t sincScale = ActualSincScale(artifacts);
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				numl_t bottomSign;
				if(data.PhaseRepresentation() == TimeFrequencyData::RealPart)
					bottomSign = 1.0;
				else if(data.PhaseRepresentation() == TimeFrequencyData::ImaginaryPart)
					bottomSign = -1.0;
				else
					throw BadUsageException("Data is not real or imaginary");
				const numl_t cosRotate = cosnl(_directionRad);
				const numl_t sinRotate = sinnl(_directionRad);
				const size_t width = image->Width();
					
				for(size_t y=0;y<image->Height();++y)
				{
					listener.OnProgress(y, image->Height());
					
					// Find length of the major axis of the ellipse
					numl_t maxU = -1e20, minU = 1e20;
					for(size_t x=0;x<width;++x)
					{
						const UVW &uvw = metaData->UVW()[x];
						const numl_t uProjectUpper = uvw.u * cosRotate - uvw.v * sinRotate;
						if(uProjectUpper > maxU) maxU = uProjectUpper;
						if(uProjectUpper < minU) minU = uProjectUpper;
						const numl_t uProjectBottom = -uvw.u * cosRotate + uvw.v * sinRotate;
						if(uProjectBottom > maxU) maxU = uProjectBottom;
						if(uProjectBottom < minU) minU = uProjectBottom;
					}

					// Calculate the projected positions and change sign if necessary
					numl_t
						*rowValues = new numl_t[width*3],
						*rowPositions = new numl_t[width*3];
					for(size_t t=0;t<width;++t)
					{
						const UVW &uvw = metaData->UVW()[t];
						const numl_t vProject = uvw.u * sinRotate + uvw.v * cosRotate;
						numl_t uProject, currentSign;
						if(vProject >= 0.0) {
							uProject = uvw.u * cosRotate - uvw.v * sinRotate;
							currentSign = 1.0;
						} else {
							uProject = -uvw.u * cosRotate + uvw.v * sinRotate;
							currentSign = bottomSign;
						}
						numl_t centerValue = currentSign * image->Value(t, y);
						rowValues[t] = bottomSign * centerValue;
						rowValues[t + width] = centerValue;
						rowValues[t + width*2] = bottomSign * centerValue;
						rowPositions[t] = uProject - (maxU - minU);
						rowPositions[t + width] = uProject;
						rowPositions[t + width*2] = uProject + (maxU - minU);
					}
					
					// Perform the convolution
					for(size_t t=width;t<2*width;++t)
					{
						const UVW &uvw = metaData->UVW()[t-width];
						numl_t currentSign;
						const numl_t vProject = uvw.u * sinRotate + uvw.v * cosRotate;
						if(vProject >= 0.0)
							currentSign = 1.0;
						else
							currentSign = bottomSign;
						
						numl_t pos = rowPositions[t];
						numl_t valueSum = 0.0;
						numl_t weightSum = 0.0;
						
						for(size_t x=0;x<3*width;++x)
						{
							// if this is exactly a point on the u axis, this point is ignored
							// (it would have infinite weight)
							const UVW &uvw = metaData->UVW()[t%width];
							if(uvw.v != 0.0) 
							{
								const numl_t weight = uvw.u / uvw.v;
								const numl_t dist = (rowPositions[x] - pos) / sincScale;
								const numl_t sincValue = sinnl(dist) / dist;
								if(dist!=0.0)
								{
									valueSum += sincValue * rowValues[x] * weight;
									weightSum += sincValue * weight;
								}
								else
								{
									valueSum += rowValues[x] * weight;
									weightSum += weight;
								}
							}
						}
						//std::cout << sincSum << ' ';
						newImage->SetValue(t-width, y, (num_t) valueSum * currentSign / weightSum);
					}
					//std::cout << std::endl;
					
					delete[] rowValues;
					delete[] rowPositions;
				}
				listener.OnProgress(image->Height(), image->Height());
				
				return newImage;
			}
			
			numl_t maxUVDistance(const std::vector<UVW> &uvw) const
			{
				numl_t maxDist = 0.0;
				for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
				{
					numl_t dist = i->u * i->u + i->v * i->v;
					if(dist > maxDist) maxDist = dist;
				}
				return sqrtnl(maxDist);
			}
			
			num_t ActualSincScale(ArtifactSet &artifacts) const
			{
				return _sincSize / maxUVDistance(artifacts.MetaData()->UVW());
			}
			
			enum Operation _operation;
			num_t _sincSize, _directionRad;
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
