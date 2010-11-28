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

#ifndef RFISETIMAGEACTION_H
#define RFISETIMAGEACTION_H

#include <AOFlagger/util/progresslistener.h>

#include <AOFlagger/rfi/strategy/actioncontainer.h>
#include <AOFlagger/rfi/strategy/artifactset.h>

namespace rfiStrategy {

	class SetImageAction : public Action
	{
		public:
			enum NewImage { Zero, FromOriginal, SwapRevisedAndContaminated, ReplaceFlaggedValues, SetFlaggedValuesToZero };

			SetImageAction() : _newImage(FromOriginal), _add(false) { }

			virtual std::string Description()
			{
				if(_add)
				{
					switch(_newImage)
					{
						default:
						case Zero:
						case SwapRevisedAndContaminated:
							return "Do nothing";
						case FromOriginal:
							return "Add original image";
					}
				} else {
					switch(_newImage)
					{
						default:
						case Zero:
							return "Set image to zero";
						case FromOriginal:
							return "Set original image";
						case SwapRevisedAndContaminated:
							return "Swap revised and contaminated";
						case ReplaceFlaggedValues:
							return "Revise flagged values";
						case SetFlaggedValuesToZero:
							return "Set flagged values to zero";
					}
				}
			}
			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
			{
				if(_add)
					PerformAdd(artifacts, listener);
				else
					PerformSet(artifacts, listener);
			}
			enum NewImage NewImage() const throw() { return _newImage; }
			void SetNewImage(enum NewImage newImage) throw()
			{
				_newImage = newImage;
			}
			bool Add() const throw() { return _add; }
			void SetAdd(bool add) throw()
			{
				_add = add;
			}
			virtual ActionType Type() const { return SetImageActionType; }
		private:
			void PerformSet(class ArtifactSet &artifacts, class ProgressListener &)
			{
				switch(_newImage)
				{
					default:
					case FromOriginal:
					{
						TimeFrequencyData *phaseData =
							artifacts.OriginalData().CreateTFData(artifacts.ContaminatedData().PhaseRepresentation());
						TimeFrequencyData *phaseAndPolData =
							phaseData->CreateTFData(artifacts.ContaminatedData().Polarisation());
						delete phaseData;
						phaseAndPolData->SetMask(artifacts.ContaminatedData());
						artifacts.SetContaminatedData(*phaseAndPolData);
						delete phaseAndPolData;
					}
					break;
					case Zero:
					{
						Image2DPtr zero =
							Image2D::CreateEmptyImagePtr(artifacts.ContaminatedData().ImageWidth(), artifacts.ContaminatedData().ImageHeight());
						TimeFrequencyData data(artifacts.ContaminatedData().PhaseRepresentation(), artifacts.ContaminatedData().Polarisation(), zero);
						data.SetMask(artifacts.ContaminatedData());
						artifacts.SetContaminatedData(data);
						break;
					}
					case SwapRevisedAndContaminated:
					{
						TimeFrequencyData data = artifacts.ContaminatedData();
						artifacts.SetContaminatedData(artifacts.RevisedData());
						artifacts.SetRevisedData(data);
						break;
					}
					case ReplaceFlaggedValues:
					{
						TimeFrequencyData contaminatedData = artifacts.ContaminatedData();
						const TimeFrequencyData
							revisedData = artifacts.RevisedData(),
							originalData = artifacts.OriginalData();
						if(contaminatedData.PolarisationCount() != 1)
							throw BadUsageException("Can not replace flagged values for multiple polarizations: use a For Each Polarisation action");
						if(revisedData.PolarisationCount() != 1 || originalData.PolarisationCount() != 1)
							throw BadUsageException("Revised or original data has multiple polarisations");
						if(contaminatedData.PhaseRepresentation() != revisedData.PhaseRepresentation() || contaminatedData.PhaseRepresentation() != originalData.PhaseRepresentation())
							throw BadUsageException("Contaminated and Revised data do not have equal phase representations");
						Mask2DCPtr mask = contaminatedData.GetSingleMask();
						unsigned imageCount = contaminatedData.ImageCount();
						for(unsigned i=0;i<imageCount;++i)
						{
							Image2DCPtr
								revisedImage = revisedData.GetImage(i),
								originalImage = originalData.GetImage(i);
							Image2DPtr image = Image2D::CreateCopy(contaminatedData.GetImage(i));
							for(size_t y=0;y<image->Height();++y)
							{
								for(size_t x=0;x<image->Width();++x)
								{
									if(mask->Value(x, y))
										image->SetValue(x, y, revisedImage->Value(x, y));
									else
										image->SetValue(x, y, originalImage->Value(x, y));
								}
							}
							artifacts.ContaminatedData().SetImage(i, image);
						}
						break;
					}
					case SetFlaggedValuesToZero:
					{
						TimeFrequencyData contaminatedData = artifacts.ContaminatedData();
						Mask2DCPtr mask = contaminatedData.GetSingleMask();
						unsigned imageCount = contaminatedData.ImageCount();
						for(unsigned i=0;i<imageCount;++i)
						{
							Image2DPtr image = Image2D::CreateCopy(contaminatedData.GetImage(i));
							for(size_t y=0;y<image->Height();++y)
							{
								for(size_t x=0;x<image->Width();++x)
								{
									if(mask->Value(x, y))
										image->SetValue(x, y, 0.0);
								}
							}
							artifacts.ContaminatedData().SetImage(i, image);
						}
						break;
					}
				}
			}
			void PerformAdd(class ArtifactSet &artifacts, class ProgressListener &)
			{
				switch(_newImage)
				{
					default:
					case FromOriginal:
					{
						TimeFrequencyData *phaseData =
							artifacts.OriginalData().CreateTFData(artifacts.RevisedData().PhaseRepresentation());
						TimeFrequencyData *phaseAndPolData =
							phaseData->CreateTFData(artifacts.RevisedData().Polarisation());
						delete phaseData;
						TimeFrequencyData *summedData =
							TimeFrequencyData::CreateTFDataFromSum(*phaseAndPolData, artifacts.RevisedData());
						delete phaseAndPolData;
						summedData->SetMask(artifacts.RevisedData());
						artifacts.SetRevisedData(*summedData);
						delete summedData;
					}
					break;
					case Zero:
					case SwapRevisedAndContaminated:
					case ReplaceFlaggedValues:
					case SetFlaggedValuesToZero:
					break;
				}
			}
			enum NewImage _newImage;
			bool _add;
	};
}

#endif // RFISETIMAGEACTION_H
