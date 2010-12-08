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

#include <AOFlagger/rfi/strategy/action.h>
#include <AOFlagger/rfi/strategy/actionblock.h>
#include <AOFlagger/rfi/strategy/artifactset.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/rfi/sinusfitter.h>
#include <AOFlagger/rfi/thresholdtools.h>
#include <AOFlagger/rfi/uvprojection.h>

#include <AOFlagger/util/aologger.h>
#include <AOFlagger/util/ffttools.h>

namespace rfiStrategy {

	class TimeConvolutionAction : public Action
	{
		public:
			enum Operation { SingleSincOperation, SincOperation, ProjectedSincOperation, ProjectedFTOperation, ExtrapolatedSincOperation, IterativeExtrapolatedSincOperation };
			
			TimeConvolutionAction() : Action(), _operation(IterativeExtrapolatedSincOperation), _sincSize(32.0), _directionRad(M_PI*(-86.7/180.0)), _etaParameter(0.2), _autoAngle(true), _isSincScaleInSamples(false), _iterations(1)
			{
			}
			virtual std::string Description()
			{
				switch(_operation)
				{
					case SingleSincOperation:
						return "Time sinc convolution (once)";
						break;
					case SincOperation:
						return "Time sinc convolution (round)";
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
					case IterativeExtrapolatedSincOperation:
						return "Iterative projected extrapolated sinc";
						break;
					default:
						return "?";
						break;
				}
			}
			virtual ActionType Type() const { return TimeConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				Image2DCPtr newImage;
				TimeFrequencyData newRevisedData;
				switch(_operation)
				{
					case SingleSincOperation:
						newImage = PerformSingleSincOperation(artifacts);
						break;
					case SincOperation:
						newImage = PerformSincOperation(artifacts);
						break;
					case ProjectedSincOperation:
						newImage = PerformProjectedSincOperation(artifacts, listener);
						break;
					case ProjectedFTOperation:
					case ExtrapolatedSincOperation:
					case IterativeExtrapolatedSincOperation:
					{
						if(_autoAngle)
							_directionRad = FindStrongestSourceAngle(artifacts, artifacts.ContaminatedData());
						TimeFrequencyData data = artifacts.ContaminatedData();
						TimeFrequencyData *realData = data.CreateTFData(TimeFrequencyData::RealPart);
						TimeFrequencyData *imagData = data.CreateTFData(TimeFrequencyData::ImaginaryPart);
						Image2DPtr real = Image2D::CreateCopy(realData->GetSingleImage());
						Image2DPtr imaginary = Image2D::CreateCopy(imagData->GetSingleImage());
						delete realData;
						delete imagData;
						PerformExtrapolatedSincOperation(artifacts, real, imaginary, listener);
						newRevisedData = TimeFrequencyData(data.Polarisation(), real, imaginary);
					}
					break;
				}
				
				if(_operation == SingleSincOperation || _operation == SincOperation || _operation == ProjectedSincOperation)
				{
					newRevisedData = TimeFrequencyData(artifacts.ContaminatedData().PhaseRepresentation(), artifacts.ContaminatedData().Polarisation(), newImage);
				}

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

			num_t EtaParameter() const { return _etaParameter; }
			void SetEtaParameter(num_t etaParameter) { _etaParameter = etaParameter; }

			unsigned Iterations() const { return _iterations; }
			void SetIterations(unsigned iterations) { _iterations = iterations; }

			bool AutoAngle() const { return _autoAngle; }
			void SetAutoAngle(bool autoAngle) { _autoAngle = autoAngle; }

			bool IsSincScaleInSamples() const { return _isSincScaleInSamples; }
			void SetIsSincScaleInSamples(bool inSamples) { _isSincScaleInSamples = inSamples; }
private:
			struct IterationData
			{
				ArtifactSet
					*artifacts;
				size_t
					width,
					fourierWidth,
					rangeStart,
					rangeEnd,
					vZeroPos,
					startXf,
					endXf;
				numl_t
					maxDist;
				numl_t
					*rowRValues,
					*rowIValues,
					*rowUPositions,
					*rowVPositions,
					*fourierValuesReal,
					*fourierValuesImag;
				numl_t
					**fSinTable,
					**fCosTable;
				bool
					*rowSignsNegated;
			};

			Image2DPtr PerformSingleSincOperation(ArtifactSet &artifacts) const
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()];
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();

				const BandInfo band = artifacts.MetaData()->Band();
				for(unsigned y=0;y<image->Height();++y)
				{
					const num_t sincScale = ActualSincScaleInSamples(artifacts, band.channels[y].frequencyHz);
					for(unsigned x=0;x<width;++x)
						row[x] = image->Value(x, y);

					ThresholdTools::OneDimensionalSincConvolution(row, width, sincScale / (2.0*M_PInl));
					for(unsigned x=0;x<width;++x)
						newImage->SetValue(x, y, row[x]);
				}
				delete[] row;
				
				return newImage;
			}
			
			Image2DPtr PerformSincOperation(ArtifactSet &artifacts) const
			{
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
				const BandInfo band = artifacts.MetaData()->Band();
				for(unsigned y=0;y<image->Height();++y)
				{
					const num_t sincScale = ActualSincScaleInSamples(artifacts, band.channels[y].frequencyHz);

					for(unsigned x=0;x<width;++x) {
						row[x] = sign * image->Value(x, y);
						row[x+width] = image->Value(x, y);
						row[x+2*width] = sign * image->Value(x, y);
					}
					ThresholdTools::OneDimensionalSincConvolution(row, width*3, sincScale / (2.0*M_PInl));
					for(unsigned x=0;x<width;++x)
						newImage->SetValue(x, y, row[x+width]);
				}
				delete[] row;
				
				return newImage;
			}
			
			Image2DPtr PerformProjectedSincOperation(ArtifactSet &artifacts, class ProgressListener &listener) const
			{
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				bool isImaginary = IsImaginary(data);
				const size_t width = image->Width();
				const BandInfo band = artifacts.MetaData()->Band();

				for(size_t y=0;y<image->Height();++y)
				{
					const num_t sincScale = ActualSincScaleInLambda(artifacts, band.channels[y].frequencyHz);
					listener.OnProgress(*this, y, image->Height());
					
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
				listener.OnProgress(*this, image->Height(), image->Height());
				
				return newImage;
			}

			void Project(class IterationData &iterData, Image2DCPtr real, Image2DCPtr imaginary, size_t y) const
			{
				UVProjection::Project(real, iterData.artifacts->MetaData(), y, iterData.rowRValues, iterData.rowUPositions, iterData.rowVPositions, iterData.rowSignsNegated, _directionRad, false);
				UVProjection::Project(imaginary, iterData.artifacts->MetaData(), y, iterData.rowIValues, iterData.rowUPositions, iterData.rowVPositions, iterData.rowSignsNegated, _directionRad, true);

				// Find the point closest to v=0
				const size_t width = real->Width();
				numl_t vDist = fabsnl( iterData.rowVPositions[0]);
				size_t vZeroPos = 0;
				for(unsigned i=1;i<width;++i)
				{
					if(fabsnl( iterData.rowVPositions[i]) < vDist)
					{
						vDist = fabsnl( iterData.rowVPositions[i]);
						vZeroPos = i;
					}
				}
				iterData.vZeroPos = iterData.vZeroPos;
				iterData.maxDist = fabsnl( iterData.rowUPositions[vZeroPos]);

				AOLogger::Debug << "v is min at t=" << vZeroPos << " (v=+-" << vDist << ", maxDist=" << iterData.maxDist << ")\n";
			}

			void PrecalculateFTFactors(class IterationData &iterData) const
			{
				const size_t
					width = iterData.width,
					fourierWidth = iterData.fourierWidth,
					rangeStart = iterData.rangeStart,
					rangeEnd = iterData.rangeEnd;

				iterData.fSinTable = new numl_t*[fourierWidth],
				iterData.fCosTable = new numl_t*[fourierWidth];
				for(size_t xF=0;xF<fourierWidth;++xF)
				{
					iterData.fSinTable[xF] = new numl_t[rangeEnd - rangeStart];
					iterData.fCosTable[xF] = new numl_t[rangeEnd - rangeStart];
					// F(xF) = \int f(u) * e^(-i 2 \pi * u_n * xF_n)
					// xF \in [0 : fourierWidth] -> xF_n = 2 xF / fourierWidth - 1 \in [-1 : 1];
					// u \in [-maxDist : maxDist] -> u_n = u * width / maxDist \in [ -width : width ]
					// final frequenty domain covers [-maxDist : maxDist]
					const numl_t
						fourierPos = (numl_t) xF / fourierWidth - 0.5,
						fourierFactor = -fourierPos * 2.0 * M_PInl * width * 0.5 / iterData.maxDist;
					for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
					{
						size_t t = (tIndex + width - iterData.vZeroPos) % width;
						const numl_t posU = iterData.rowUPositions[t];
						iterData.fCosTable[xF][tIndex-rangeStart] = cosnl(fourierFactor * posU);
						iterData.fSinTable[xF][tIndex-rangeStart] = sinnl(fourierFactor * posU);
					}
				}
			}

			void PerformFourierTransform(class IterationData &iterData, Image2DPtr real, Image2DPtr imaginary, size_t y) const
			{
				const size_t
					rangeStart = iterData.rangeStart,
					rangeEnd = iterData.rangeEnd,
					width = iterData.width,
					vZeroPos = iterData.vZeroPos;

				// Perform a 1d Fourier transform, ignoring eta part of the data
				for(size_t xF=0;xF<iterData.fourierWidth;++xF)
				{
					numl_t
						realVal = 0.0,
						imagVal = 0.0,
						weightSum = 0.0;
					const numl_t
						*fCosTable = iterData.fCosTable[xF],
						*fSinTable = iterData.fSinTable[xF];
					// compute F(xF) = \int f(x) * exp( -2 * \pi * i * x * xF )
					for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
					{
						size_t t = (tIndex + width - vZeroPos) % width;
						if(iterData.rowUPositions[t] != 0.0)
						{
							const numl_t weight = 1.0;//fabsnl(rowVPositions[t]/posU);
							const numl_t weightSqrt = 1.0;//sqrtnl(weight);
							realVal += (iterData.rowRValues[t] * fCosTable[tIndex-rangeStart] -
													iterData.rowIValues[t] * fSinTable[tIndex-rangeStart]) * weightSqrt;
							imagVal += (iterData.rowIValues[t] * fCosTable[tIndex-rangeStart] +
													iterData.rowRValues[t] * fSinTable[tIndex-rangeStart]) * weightSqrt;
							weightSum += weight;
						}
					}
					iterData.fourierValuesReal[xF] = realVal / weightSum;
					iterData.fourierValuesImag[xF] = imagVal / weightSum;
					if(_operation == ProjectedFTOperation)
					{
						real->SetValue(xF/2, y, (num_t) realVal / weightSum);
						imaginary->SetValue(xF/2, y, (num_t) imagVal / weightSum);
					}
				}
			}

			void InvFourierTransform(class IterationData &iterData, Image2DPtr real, Image2DPtr imaginary, size_t y) const
			{
				const size_t
					startXf = iterData.startXf,
					endXf = iterData.endXf,
					width = iterData.width,
					fourierWidth = iterData.fourierWidth;

				AOLogger::Debug << "Inv FT, using 0-" << startXf << " and " << endXf << "-" << fourierWidth << '\n';
				
				for(size_t t=0;t<width;++t)
				{
					const numl_t
						posU = iterData.rowUPositions[t],
						posV = iterData.rowVPositions[t],
						posFactor = posU * 2.0 * M_PInl;
					numl_t
						realVal = 0.0,
						imagVal = 0.0;
					bool residual = false;

					if(posV != 0.0)
					{
						const numl_t weightSum = 1.0; //(endXf - startXf); // * fabsnl(posV / posU);
						// compute f(x) = \int F(xF) * exp( 2 * \pi * i * x * xF )
						size_t xF, loopEnd;
						if(residual)
						{
							loopEnd = startXf;
							xF = 0;
						}
						else
						{
							loopEnd = endXf;
							xF = startXf;
						}
						while(xF < loopEnd)
						{
							const numl_t
								fourierPosL = (numl_t) xF / fourierWidth - 0.5,
								fourierRealL = iterData.fourierValuesReal[xF],
								fourierImagL = iterData.fourierValuesImag[xF];

							const numl_t
								cosValL = cosnl(fourierPosL * posFactor),
								sinValL = sinnl(fourierPosL * posFactor);

							realVal += fourierRealL * cosValL - fourierImagL * sinValL;
							imagVal += fourierImagL * cosValL + fourierRealL * sinValL;

							if(residual)
							{
								const numl_t
									fourierPosR = (numl_t) (endXf + xF) / fourierWidth - 0.5,
									fourierRealR = iterData.fourierValuesReal[endXf + xF],
									fourierImagR = iterData.fourierValuesImag[endXf + xF];

								const numl_t
									cosValR = cosnl(fourierPosR * posFactor),
									sinValR = sinnl(fourierPosR * posFactor);

								realVal += fourierRealR * cosValR - fourierImagR * sinValR;
								imagVal += fourierImagR * cosValR + fourierRealR * sinValR;
							}
							++xF;
						}
						real->SetValue(t, y, -realVal/weightSum);
						if(iterData.rowSignsNegated[t])
							imaginary->SetValue(t, y, imagVal/weightSum);
						else
							imaginary->SetValue(t, y, -imagVal/weightSum);
					}
				}
			}

			void SubtractComponent(class IterationData &iterData, Image2DPtr real, Image2DPtr imaginary, size_t y) const
			{
				const size_t
					startXf = iterData.startXf,
					endXf = iterData.endXf,
					width = iterData.width,
					fourierWidth = iterData.fourierWidth;

				// Find strongest frequency
				AOLogger::Debug << "Limiting search to xF<" << startXf << " and xF>" << endXf << '\n'; 
				size_t xFRemoval = 0;
				double xFValue =
					iterData.fourierValuesReal[0]*iterData.fourierValuesReal[0] + iterData.fourierValuesImag[0]*iterData.fourierValuesImag[0];
				for(size_t xF=0;xF<startXf;++xF)
				{
					numl_t
						fReal = iterData.fourierValuesReal[xF],
						fImag = iterData.fourierValuesImag[xF],
						val = fReal*fReal + fImag*fImag;
					if(val > xFValue)
					{
						xFRemoval = xF;
						xFValue = val;
					}
					fReal = iterData.fourierValuesReal[xF+endXf];
					fImag = iterData.fourierValuesImag[xF+endXf];
					val = fReal*fReal + fImag*fImag;
					if(val > xFValue)
					{
						xFRemoval = xF+endXf;
						xFValue = val;
					}
				}
				const numl_t
					fourierPos = (numl_t) xFRemoval / fourierWidth - 0.5,
					fourierFactor = fourierPos * 2.0 * M_PInl * width * 0.5 / iterData.maxDist,
					fourierReal = iterData.fourierValuesReal[xFRemoval],
					fourierImag = iterData.fourierValuesImag[xFRemoval];

				AOLogger::Debug << "Strongest frequency at xF=" << xFRemoval << ", amp^2=" << xFValue << '\n';
				AOLogger::Debug << "Corresponding frequency: " << fourierPos << " x " << iterData.maxDist << " / "
												<< width << " = " << (fourierPos*iterData.maxDist/width) << " (pixels/fringe)\n";

				// Remove strongest frequency
				for(size_t t=0;t<width;++t)
				{
					const numl_t
						posU = iterData.rowUPositions[t],
						weightSum = 1.0;
						
					const numl_t
						cosValL = cosnl(fourierFactor * posU),
						sinValL = sinnl(fourierFactor * posU);

					numl_t realVal = (fourierReal * cosValL - fourierImag * sinValL) * 0.75 / weightSum;
					numl_t imagVal = (fourierImag * cosValL + fourierReal * sinValL) * 0.75 / weightSum;
					
					iterData.rowRValues[t] -= realVal;
					iterData.rowIValues[t] -= imagVal;
					real->SetValue(t, y, iterData.rowRValues[t]);
					if(iterData.rowSignsNegated[t])
						imaginary->SetValue(t, y, -iterData.rowIValues[t]);
					else
						imaginary->SetValue(t, y, iterData.rowIValues[t]);
				}
			}
			
			void PerformExtrapolatedSincOperation(ArtifactSet &artifacts, Image2DPtr real, Image2DPtr imaginary, class ProgressListener &listener) const
			{
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				const size_t width = real->Width();
				const BandInfo band = artifacts.MetaData()->Band();
					
				class IterationData iterData;

				iterData.artifacts = &artifacts;

				iterData.width = width;
				iterData.rowRValues = new numl_t[width];
				iterData.rowIValues = new numl_t[width];
				iterData.rowUPositions = new numl_t[width];
				iterData.rowVPositions = new numl_t[width];
				iterData.fourierWidth = width * 2;
				iterData.fourierValuesReal = new numl_t[iterData.fourierWidth];
				iterData.fourierValuesImag = new numl_t[iterData.fourierWidth];
				iterData.rowSignsNegated = new bool[width];

				iterData.rangeStart = (size_t) roundn(_etaParameter * (num_t) width / 2.0),
				iterData.rangeEnd = width - iterData.rangeStart;

				for(size_t y=0;y<real->Height();++y)
				{
					listener.OnProgress(*this, y, real->Height());

					Project(iterData, real, imaginary, y);
					
					PrecalculateFTFactors(iterData);

					for(unsigned iteration=0;iteration<_iterations;++iteration)
					{
						PerformFourierTransform(iterData, real, imaginary, y);
					
						numl_t sincScale = ActualSincScaleInLambda(artifacts, band.channels[y].frequencyHz);
						numl_t clippingFrequency = 1.0/(sincScale * width / iterData.maxDist);
						long fourierClippingIndex =
							(long) ceilnl((numl_t) iterData.fourierWidth * 0.5 * clippingFrequency);
						if(fourierClippingIndex*2 > (long) iterData.fourierWidth)
							fourierClippingIndex = iterData.fourierWidth/2;
						if(fourierClippingIndex < 0)
							fourierClippingIndex = 0;
						iterData.startXf = iterData.fourierWidth/2 - fourierClippingIndex,
						iterData.endXf = iterData.fourierWidth/2 + fourierClippingIndex;

						if(_operation == ExtrapolatedSincOperation)
						{
							InvFourierTransform(iterData, real, imaginary, y);
						}
						else if(_operation == IterativeExtrapolatedSincOperation)
						{
							SubtractComponent(iterData, real, imaginary, y);
						}
					}
					for(size_t xF=0;xF<iterData.fourierWidth;++xF)
					{
						delete[] iterData.fSinTable[xF];
						delete[] iterData.fCosTable[xF];
					}
					delete[] iterData.fSinTable;
					delete[] iterData.fCosTable;
				}
				listener.OnProgress(*this, real->Height(), real->Height());

				delete[] iterData.rowRValues;
				delete[] iterData.rowIValues;
				delete[] iterData.rowUPositions;
				delete[] iterData.rowVPositions;
				delete[] iterData.rowSignsNegated;
				delete[] iterData.fourierValuesReal;
				delete[] iterData.fourierValuesImag;
			}

			numl_t avgUVDistance(ArtifactSet &artifacts, const double frequencyHz) const
			{
				const std::vector<UVW> &uvw = artifacts.MetaData()->UVW();
				numl_t avgDist = 0.0;
				for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
				{
					numl_t dist = i->u*i->u + i->v*i->v;
					avgDist += sqrtnl(dist);
				}
				return avgDist * frequencyHz / (UVImager::SpeedOfLight() * (numl_t) uvw.size());
			}

			numl_t ActualSincScaleInSamples(ArtifactSet &artifacts, const double frequencyHz) const
			{
				if(_isSincScaleInSamples)
					return _sincSize;
				else
					return _sincSize / avgUVDistance(artifacts, frequencyHz) * (0.5 * (numl_t) artifacts.ContaminatedData().ImageWidth());
			}

			numl_t ActualSincScaleInLambda(ArtifactSet &artifacts, const double frequencyHz) const
			{
				if(_isSincScaleInSamples)
					return _sincSize / (0.5 * (numl_t) artifacts.ContaminatedData().ImageWidth()) * avgUVDistance(artifacts, frequencyHz);
				else
					return _sincSize;
			}
			
			numl_t ActualSincScaleAsRaDecDist(ArtifactSet &artifacts, const double frequencyHz) const
			{
				return 1.0/ActualSincScaleInLambda(artifacts, frequencyHz);
			}

			numl_t FindStrongestSourceAngle(ArtifactSet &artifacts, const TimeFrequencyData &data)
			{
				UVImager imager(1024*3, 1024*3);
				imager.Image(data, artifacts.MetaData());
				imager.PerformFFT();
				Image2DPtr image(FFTTools::CreateAbsoluteImage(imager.FTReal(), imager.FTImaginary()));
				const numl_t centralFreq = artifacts.MetaData()->Band().channels[data.ImageHeight()/2].frequencyHz;
				AOLogger::Debug << "Central frequency: " << centralFreq << "\n";
				AOLogger::Debug << "Baseline length: " << artifacts.MetaData()->Baseline().Distance() << '\n';
				AOLogger::Debug << "Sinc scale in lambda: " << ActualSincScaleInLambda(artifacts, centralFreq) << '\n';
				AOLogger::Debug << "Average distance: " << avgUVDistance(artifacts, centralFreq) << '\n';
				const numl_t sincDist = ActualSincScaleAsRaDecDist(artifacts, centralFreq);
				numl_t ignoreRadius = sincDist / imager.UVScaling();
				AOLogger::Debug << "Ignoring radius=" << ignoreRadius << "\n";

				long maxX = 0, maxY = 0;
				num_t maxValue = image->Value(maxX, maxY);
				for(unsigned y=0;y<image->Height();++y)
				{
					for(unsigned x=0;x<image->Width();++x)
					{
						if(image->Value(x, y) > maxValue)
						{
							int x_r = (x*2 - image->Width())/2;
							int y_r = (image->Height() - y*2)/2;
							numl_t distSqr = x_r*x_r + y_r*y_r;
							if(distSqr > ignoreRadius * ignoreRadius)
							{
								maxValue = image->Value(x, y);
								maxX = x;
								maxY = y;
							}
						}
					}
				}
				maxX = maxX*2-image->Width();
				maxY = image->Height() - maxY*2;
				numl_t angle = SinusFitter::Phase((numl_t) maxX, (numl_t) maxY);
 				AOLogger::Debug << "Angle: " << angle/M_PInl*180.0 << ",maxX=" << maxX << ",maxY=" << maxY << '\n';
				return angle;
				/*
				image = FFTTools::AngularTransform(image);
				unsigned pixelDist = (unsigned) (ActualSincScaleAsRaDecDist()*image->Height()/2.0);
				AOLogger::Debug << "Ignoring " << (image->Height()/2-pixelDist) << "-" << (image->Height()/2+pixelDist) << '\n';
				numl_t highestSum = -1e10;
				size_t highestIndex = 0;
				for(size_t x=0;x<image->Width();++x)
				{
					numl_t sum = 0.0;
					for(size_t y=0;y<pixelDist;++y)
					{
						sum += image->Value(x, y);
						sum += image->Value(x, image->Height() - y - 1);
					}
					if(sum > highestSum)
					{
						highestSum = sum;
						highestIndex = x;
					}
				}
				numl_t angle = (numl_t) highestIndex * M_PInl / image->Width();
				AOLogger::Debug << "Angle: " << angle/M_PInl*180.0 << '\n';
				return angle;*/
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
			bool _autoAngle, _isSincScaleInSamples;
			unsigned _iterations;
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
