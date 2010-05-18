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

#ifndef RFI_FOREACHPOLARISATION_H
#define RFI_FOREACHPOLARISATION_H 

#include "actionblock.h"
#include "artifactset.h"

#include "../../msio/timefrequencydata.h"

namespace rfiStrategy {

	class ForEachPolarisationBlock : public ActionBlock
	{
		public:
			ForEachPolarisationBlock() : _iterateStokesValues(false)
			{
			}
			virtual ~ForEachPolarisationBlock()
			{
			}
			virtual std::string Description()
			{
				return "For each polarisation";
			}
			virtual void Initialize()
			{
			}
			virtual ActionType Type() const { return ForEachPolarisationBlockType; }
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress)
			{
				TimeFrequencyData
					oldContaminatedData = artifacts.ContaminatedData(),
					oldRevisedData = artifacts.RevisedData(),
					oldOriginalData = artifacts.OriginalData();

				if(oldContaminatedData.Polarisation() != oldOriginalData.Polarisation())
					throw BadUsageException("Contaminated and original do not have equal polarisation, in for each polarisation block");

				if(oldContaminatedData.Polarisation() == DipolePolarisation && _iterateStokesValues)
					performStokesIteration(artifacts, progress);
				else if(oldContaminatedData.PolarisationCount() == 1) {
					// There is only one polarisation in the contaminated data; just run all childs
					ActionBlock::Perform(artifacts, progress);
				} else {
					bool changeRevised = (oldRevisedData.Polarisation() == oldContaminatedData.Polarisation());
					unsigned count = oldContaminatedData.PolarisationCount();

					for(unsigned polarizationIndex = 0; polarizationIndex < count; ++polarizationIndex)
					{
						TimeFrequencyData *newContaminatedData =
							oldContaminatedData.CreateTFDataFromPolarisationIndex(polarizationIndex);
						TimeFrequencyData *newOriginalData =
							oldOriginalData.CreateTFDataFromPolarisationIndex(polarizationIndex);

						artifacts.SetContaminatedData(*newContaminatedData);
						artifacts.SetOriginalData(*newOriginalData);
		
						progress.OnStartTask(polarizationIndex, count, newContaminatedData->Description());
		
						delete newContaminatedData;
						delete newOriginalData;
						
						if(changeRevised)
						{
							TimeFrequencyData *newRevised = oldRevisedData.CreateTFDataFromPolarisationIndex(polarizationIndex);
							artifacts.SetRevisedData(*newRevised);
							delete newRevised;
						}
		
						ActionBlock::Perform(artifacts, progress);

						oldContaminatedData.SetPolarizationData(polarizationIndex, artifacts.ContaminatedData());
						oldOriginalData.SetPolarizationData(polarizationIndex, artifacts.OriginalData());
						if(changeRevised)
							oldRevisedData.SetPolarizationData(polarizationIndex, artifacts.RevisedData());

						progress.OnEndTask();
					}

					artifacts.SetContaminatedData(oldContaminatedData);
					artifacts.SetRevisedData(oldRevisedData);
					artifacts.SetOriginalData(oldOriginalData);
				}
			}

			void SetIterateStokesValues(bool iterateStokesValues)
			{
				_iterateStokesValues = iterateStokesValues;
			}
			bool IterateStokesValues() const
			{
				return _iterateStokesValues;
			}
		private:
			bool _iterateStokesValues;

			void performStokesIteration(ArtifactSet &artifacts, ProgressListener &progress)
			{
				TimeFrequencyData
					oldContaminatedData = artifacts.ContaminatedData(),
					oldRevisedData = artifacts.RevisedData(),
					oldOriginalData = artifacts.OriginalData();

				bool changeRevised = (oldRevisedData.Polarisation() == oldContaminatedData.Polarisation());

				performPolarisation(artifacts, progress, StokesIPolarisation, oldContaminatedData, oldOriginalData, oldRevisedData, changeRevised, 0, 4);
				performPolarisation(artifacts, progress, StokesQPolarisation, oldContaminatedData, oldOriginalData, oldRevisedData, changeRevised, 1, 4);
				performPolarisation(artifacts, progress, StokesUPolarisation, oldContaminatedData, oldOriginalData, oldRevisedData, changeRevised, 2, 4);
				performPolarisation(artifacts, progress, StokesVPolarisation, oldContaminatedData, oldOriginalData, oldRevisedData, changeRevised, 3, 4);
				
				artifacts.SetContaminatedData(oldContaminatedData);
				artifacts.SetRevisedData(oldRevisedData);
				artifacts.SetOriginalData(oldOriginalData);
			}

			void performPolarisation(ArtifactSet &artifacts, ProgressListener &progress, enum PolarisationType polarisation, const TimeFrequencyData &oldContaminatedData, const TimeFrequencyData &oldOriginalData, const TimeFrequencyData &oldRevisedData, bool changeRevised, size_t taskNr, size_t taskCount)
			{
				TimeFrequencyData *newContaminatedData =
					oldContaminatedData.CreateTFData(polarisation);
				artifacts.SetContaminatedData(*newContaminatedData);
				progress.OnStartTask(taskNr, taskCount, newContaminatedData->Description());
				delete newContaminatedData;

				TimeFrequencyData *newOriginalData =
					oldOriginalData.CreateTFData(polarisation);
				artifacts.SetOriginalData(*newOriginalData);
				delete newOriginalData;


				if(changeRevised)
				{
					TimeFrequencyData *newRevised = oldRevisedData.CreateTFData(polarisation);
					artifacts.SetRevisedData(*newRevised);
					delete newRevised;
				}

				ActionBlock::Perform(artifacts, progress);

				progress.OnEndTask();
			}
	};

}

#endif // RFI_FOREACHPOLARISATION_H
