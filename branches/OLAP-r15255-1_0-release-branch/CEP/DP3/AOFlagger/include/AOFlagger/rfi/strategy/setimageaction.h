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

#include "actioncontainer.h"

#include "../../util/progresslistener.h"

namespace rfiStrategy {

	class SetImageAction : public Action
	{
		public:
			enum NewImage { Zero, FromOriginal };

			SetImageAction() : _newImage(FromOriginal), _add(false) { }

			virtual std::string Description()
			{
				if(_add)
				{
					switch(_newImage)
					{
						default:
						case Zero:
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
							phaseData->CreateTFData(artifacts.ContaminatedData().PolarisationType());
						delete phaseData;
						phaseAndPolData->SetMaskFrom(artifacts.ContaminatedData());
						artifacts.SetContaminatedData(*phaseAndPolData);
						delete phaseAndPolData;
					}
					break;
					case Zero:
					{
						Image2DPtr zero =
							Image2D::CreateEmptyImagePtr(artifacts.ContaminatedData().ImageWidth(), artifacts.ContaminatedData().ImageHeight());
						TimeFrequencyData data(artifacts.ContaminatedData().PhaseRepresentation(), artifacts.ContaminatedData().PolarisationType(), zero);
						data.SetMaskFrom(artifacts.ContaminatedData());
						artifacts.SetContaminatedData(data);
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
							phaseData->CreateTFData(artifacts.RevisedData().PolarisationType());
						delete phaseData;
						TimeFrequencyData *summedData =
							TimeFrequencyData::CreateTFDataFromSum(*phaseAndPolData, artifacts.RevisedData());
						delete phaseAndPolData;
						summedData->SetMaskFrom(artifacts.RevisedData());
						artifacts.SetRevisedData(*summedData);
						delete summedData;
					}
					break;
					case Zero:
					break;
				}
			}
			enum NewImage _newImage;
			bool _add;
	};
}

#endif // RFISETIMAGEACTION_H
