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
#ifndef RFI_ADAPTER
#define RFI_ADAPTER

#include "artifactset.h"
#include "actionblock.h"

namespace rfiStrategy {

	class Adapter : public ActionBlock
	{
			public:
				virtual std::string Description()
				{
					return "On amplitude";
				}
				virtual ActionType Type() const { return AdapterType; }
				virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
				{
					enum TimeFrequencyData::PhaseRepresentation contaminatedPhase = 
						artifacts.ContaminatedData().PhaseRepresentation();
					enum TimeFrequencyData::PhaseRepresentation revisedPhase = 
						artifacts.RevisedData().PhaseRepresentation();

					if(contaminatedPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newContaminatedData =
							artifacts.ContaminatedData().CreateTFData(TimeFrequencyData::AmplitudePart);
						artifacts.SetContaminatedData(*newContaminatedData);
						delete newContaminatedData;
					}
					if(revisedPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newRevisedData =
							artifacts.RevisedData().CreateTFData(TimeFrequencyData::AmplitudePart);
						artifacts.SetRevisedData(*newRevisedData);
						delete newRevisedData;
					}

					ActionBlock::Perform(artifacts, listener);

					if(contaminatedPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newContaminatedData =
							TimeFrequencyData::CreateTFDataFromComplexCombination(artifacts.ContaminatedData(), artifacts.ContaminatedData());
						newContaminatedData->MultiplyImages(1.0L/M_SQRT2);
						newContaminatedData->SetMaskFrom(artifacts.ContaminatedData());
						artifacts.SetContaminatedData(*newContaminatedData);
						delete newContaminatedData;
					}
					if(revisedPhase == TimeFrequencyData::ComplexRepresentation)
					{
						TimeFrequencyData *newRevisedData =
							TimeFrequencyData::CreateTFDataFromComplexCombination(artifacts.RevisedData(), artifacts.RevisedData());
						newRevisedData->MultiplyImages(1.0L/M_SQRT2);
						newRevisedData->SetMaskFrom(artifacts.RevisedData());
						artifacts.SetRevisedData(*newRevisedData);
						delete newRevisedData;
					}
				}
	};

} // namespace

#endif
