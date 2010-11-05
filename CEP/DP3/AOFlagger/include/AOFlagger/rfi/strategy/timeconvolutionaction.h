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
#include <AOFlagger/rfi/uvprojection.h>

namespace rfiStrategy {

	class TimeConvolutionAction : public Action
	{
		public:
			enum Operation { SincOperation, ProjectedSincOperation, ProjectedFTOperation, ExtrapolatedSincOperation };
			
			TimeConvolutionAction() : Action(), _operation(ExtrapolatedSincOperation), _sincSize(32.0), _directionRad(M_PI*(-92.0/180.0)), _etaParameter(0.1)
			{
			}
			virtual std::string Description()
			{
				switch(_operation)
				{
					case SincOperation:
						return "Time sinc convolution";
						break;
					case ProjectedSincOperation:
						return "Projected sinc convolution";
						break;
					case ProjectedFTOperation:
						return "Projected Fourier transform";
						break;
					case ExtrapolatedSincOperation:
						return "Projected extrapolated sinc";
						break;
				}
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
					case ProjectedFTOperation:
					case ExtrapolatedSincOperation:
						newImage = PerformExtrapolatedSincOperation(artifacts, listener);
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
				num_t sincScale = ActualSincScaleInSamples(artifacts);
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()*3];
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();
				num_t sign;
				if(IsImaginary(data))
					sign = -1.0;
				else
					sign = 1.0;
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
				num_t sincScale = ActualSincScaleInLambda(artifacts);
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				bool isImaginary = IsImaginary(data);
				const size_t width = image->Width();
					
				for(size_t y=0;y<image->Height();++y)
				{
					listener.OnProgress(y, image->Height());
					
					numl_t
						*rowValues = new numl_t[width],
						*rowUPositions = new numl_t[width],
						*rowVPositions = new numl_t[width];
					bool
						*rowSignsNegated = new bool[width];

					UVProjection::Project(image, metaData, y, rowValues, rowUPositions, rowVPositions, rowSignsNegated, _directionRad, isImaginary);

					// Perform the convolution
					for(size_t t=0;t<width;++t)
					{
						const numl_t pos = rowUPositions[t];

						numl_t valueSum = 0.0;
						numl_t weightSum = 0.0;
						
						for(size_t x=0;x<width;++x)
						{
							// if this is exactly a point on the u axis, this point is ignored
							// (it would have infinite weight)
							const UVW &uvw = metaData->UVW()[x];
							if(uvw.v != 0.0) 
							{
								//const numl_t weight = fabsnl(uvw.u / uvw.v);
								const numl_t dist = (rowUPositions[x] - pos) / sincScale;
								if(dist!=0.0)
								{
									const numl_t sincValue = sinnl(dist) / dist;
									valueSum += sincValue * rowValues[x];// * weight;
									weightSum += sincValue;
								}
								else
								{
									valueSum += rowValues[x];// * weight;
									weightSum += 1.0;
								}
							}
						}
						const numl_t currentSign = rowSignsNegated[t] ? (-1.0) : 1.0;
						newImage->SetValue(t, y, (num_t) (valueSum * currentSign / weightSum));
					}
					
					delete[] rowValues;
					delete[] rowUPositions;
					delete[] rowVPositions;
					delete[] rowSignsNegated;
				}
				listener.OnProgress(image->Height(), image->Height());
				
				return newImage;
			}
			
			Image2DPtr PerformExtrapolatedSincOperation(ArtifactSet &artifacts, class ProgressListener &listener) const
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				bool isImaginary = IsImaginary(data);
				const size_t width = image->Width();
				const size_t fourierWidth = width * 2;
					
				for(size_t y=0;y<image->Height();++y)
				{
					listener.OnProgress(y, image->Height());
					
					numl_t
						*rowValues = new numl_t[width],
						*rowUPositions = new numl_t[width],
						*rowVPositions = new numl_t[width],
						*fourierValuesReal = new numl_t[fourierWidth],
						*fourierValuesImag = new numl_t[fourierWidth];
					bool
						*rowSignsNegated = new bool[width];

					UVProjection::Project(image, metaData, y, rowValues, rowUPositions, rowVPositions, rowSignsNegated, _directionRad, isImaginary);

					numl_t maxDist = maxUVDistance(metaData->UVW());

					// Find the point closest to v=0
					numl_t vDist = fabsnl(rowVPositions[0]);
					size_t vZeroPos = 0;
					for(unsigned i=1;i<width;++i)
					{
						if(fabsnl(rowVPositions[i]) < vDist)
						{
							vDist = fabsnl(rowVPositions[i]);
							vZeroPos = i;
						}
					}
					std::cout << "U is max at t=" << vZeroPos << " (u=+-" << vDist << ")" << std::endl;

					// Perform a 1d Fourier transform, ignoring eta part of the data
					size_t
						rangeStart = (size_t) roundn(_etaParameter * (num_t) width / 2.0),
						rangeEnd = width - rangeStart;
					for(size_t xF=0;xF<fourierWidth;++xF)
					{
						const numl_t
							fourierPos = (2.0 * (numl_t) xF / fourierWidth - 1.0),
							fourierFactor = -fourierPos * 2.0 * M_PInl * maxDist / width;

						numl_t
							realVal = 0.0,
							imagVal = 0.0,
							weightSum = 0.0;

						// compute F(xF) = \int f(x) * exp( -2 * \pi * i * x * xF )
						for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
						{
							size_t t = (tIndex + width - vZeroPos) % width;
							const numl_t posU = rowUPositions[t];
							if(posU != 0.0)
							{
								const numl_t val = rowValues[t];
								const numl_t cosVal = cosnl(fourierFactor * posU);
								const numl_t sinVal = sinnl(fourierFactor * posU);
								const numl_t weight = fabsnl(rowVPositions[t]/posU);
								realVal += val * cosVal * weight;
								imagVal += val * sinVal * weight;
								weightSum += weight;
							}
						}
						fourierValuesReal[xF] = realVal / weightSum;
						fourierValuesImag[xF] = imagVal / weightSum;
						if(_operation == ProjectedFTOperation)
							newImage->SetValue(xF/2, y, (num_t) sqrtnl(realVal*realVal + imagVal*imagVal) / weightSum);
					}
					
					if(_operation == ExtrapolatedSincOperation)
					{
						numl_t sincScale = ActualSincScaleInLambda(artifacts);
						numl_t clippingFrequency = 2.0*M_PInl/sincScale;
						long fourierClippingIndex = (long) ((numl_t) fourierWidth * clippingFrequency * maxDist / (2.0 * (numl_t) width));
						if(fourierClippingIndex*2 > (long) width) fourierClippingIndex = width/2;
						if(fourierClippingIndex < 0) fourierClippingIndex = 0;
						size_t
							startXf = fourierWidth/2 - fourierClippingIndex,
							endXf = fourierWidth/2 + fourierClippingIndex;
						std::cout << "Inv FT, using 0-" << startXf << " and " << endXf << "-" << fourierWidth << std::endl;
						
						for(size_t t=0;t<width;++t)
						{
							const numl_t
								posU = rowUPositions[t],
								posV = rowVPositions[t],
								posFactor = posU * 2.0 * M_PInl * maxDist / width;
							numl_t
								realVal = 0.0;
	
							if(posV != 0.0)
							{
								const numl_t weight = 1.0;//posU / posV;
								// compute f(x) = \int F(xF) * exp( 2 * \pi * i * x * xF )
								for(size_t xF=0;xF<startXf;++xF)
								{
									const numl_t
										fourierPosL = (2.0 * (numl_t) xF / fourierWidth - 1.0),
										fourierRealL = fourierValuesReal[xF],
										fourierImagL = fourierValuesImag[xF];
		
									const numl_t
										cosValL = cosnl(fourierPosL * posFactor),
										sinValL = sinnl(fourierPosL * posFactor);
									realVal +=
										fourierRealL * cosValL +
										fourierImagL * sinValL;

									const numl_t
										fourierPosR = (2.0 * (numl_t) (endXf + xF) / fourierWidth - 1.0),
										fourierRealR = fourierValuesReal[endXf + xF],
										fourierImagR = fourierValuesImag[endXf + xF];

									const numl_t
										cosValR = cosnl(fourierPosR * posFactor),
										sinValR = sinnl(fourierPosR * posFactor);
									realVal +=
										fourierRealR * cosValR +
										fourierImagR * sinValR;
								}
								newImage->SetValue(t, y, image->Value(t, y) - realVal*weight);
							}
						}
					}
					
					delete[] rowValues;
					delete[] rowUPositions;
					delete[] rowVPositions;
					delete[] rowSignsNegated;
					delete[] fourierValuesReal;
					delete[] fourierValuesImag;
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
			
			numl_t ActualSincScaleInSamples(ArtifactSet &artifacts) const
			{
				return artifacts.ContaminatedData().ImageWidth() * _sincSize / maxUVDistance(artifacts.MetaData()->UVW());
			}

			numl_t ActualSincScaleInLambda(ArtifactSet &) const
			{
				return _sincSize;
			}

			bool IsImaginary(const TimeFrequencyData &data) const
			{
				if(data.PhaseRepresentation() == TimeFrequencyData::RealPart)
					return false;
				else if(data.PhaseRepresentation() == TimeFrequencyData::ImaginaryPart)
					return true;
				else
					throw BadUsageException("Data is not real or imaginary");
			}
			
			enum Operation _operation;
			num_t _sincSize, _directionRad, _etaParameter;
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
