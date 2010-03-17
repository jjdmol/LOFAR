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
			ForEachPolarisationBlock()
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

				switch(artifacts.ContaminatedData().PolarisationType())
				{
					case TimeFrequencyData::SinglePolarisation:
					case TimeFrequencyData::XX:
					case TimeFrequencyData::XY:
					case TimeFrequencyData::YX:
					case TimeFrequencyData::YY:
					case TimeFrequencyData::StokesI:
					{
						// There is only one polarisation in the contaminated data; just run all childs
						progress.OnStartTask(0, 1, "For each polarisation (single polarisation)");
						ActionBlock::Perform(artifacts, progress);
						progress.OnEndTask();
					}
					break;
					case TimeFrequencyData::AutoDipolePolarisation:
					{
						bool changeBoth = (artifacts.RevisedData().PolarisationType() == artifacts.ContaminatedData().PolarisationType());

						progress.OnStartTask(0, 2, "XX polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::XX, changeBoth);
						TimeFrequencyData
							contaminatedXX = artifacts.ContaminatedData(),
							revisedXX = artifacts.RevisedData(),
							originalXX = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						progress.OnStartTask(1, 2, "YY polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::YY, changeBoth);
						TimeFrequencyData
							contaminatedYY = artifacts.ContaminatedData(),
							revisedYY = artifacts.RevisedData(),
							originalYY = artifacts.RevisedData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						TimeFrequencyData *newTF =
							TimeFrequencyData::CreateTFDataFromAutoDipoleCombination(
								contaminatedXX, contaminatedYY);
						artifacts.SetContaminatedData(*newTF);
						delete newTF;

						newTF = 
							TimeFrequencyData::CreateTFDataFromAutoDipoleCombination(
								revisedXX, revisedYY);
						artifacts.SetRevisedData(*newTF);
						delete newTF;

						newTF = 
							TimeFrequencyData::CreateTFDataFromAutoDipoleCombination(
								originalXX, originalYY);
						artifacts.SetOriginalData(*newTF);
						delete newTF;
					}
					case TimeFrequencyData::CrossDipolePolarisation:
					{
						bool changeBoth = (artifacts.RevisedData().PolarisationType() == artifacts.ContaminatedData().PolarisationType());

						progress.OnStartTask(0, 2, "XY polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::XY, changeBoth);
						TimeFrequencyData
							contaminatedXY = artifacts.ContaminatedData(),
							revisedXY = artifacts.RevisedData(),
							originalXY = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						progress.OnStartTask(1, 2, "YX polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::YX, changeBoth);
						TimeFrequencyData
							contaminatedYX = artifacts.ContaminatedData(),
							revisedYX = artifacts.RevisedData(),
							originalYX = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						TimeFrequencyData *newTF =
							TimeFrequencyData::CreateTFDataFromCrossDipoleCombination(
								contaminatedXY, contaminatedYX);
						artifacts.SetContaminatedData(*newTF);
						delete newTF;

						newTF = 
							TimeFrequencyData::CreateTFDataFromCrossDipoleCombination(
								revisedXY, revisedYX);
						artifacts.SetRevisedData(*newTF);
						delete newTF;

						newTF = 
							TimeFrequencyData::CreateTFDataFromCrossDipoleCombination(
								originalXY, originalYX);
						artifacts.SetOriginalData(*newTF);
						delete newTF;
					}
					break;
					case TimeFrequencyData::DipolePolarisation:
					{
						bool changeBoth = (artifacts.RevisedData().PolarisationType() == artifacts.ContaminatedData().PolarisationType());

						progress.OnStartTask(0, 4, "XX polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::XX, changeBoth);
						TimeFrequencyData
							contaminatedXX = artifacts.ContaminatedData(),
							revisedXX = artifacts.RevisedData(),
							originalXX = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						progress.OnStartTask(1, 4, "XY polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::XY, changeBoth);
						TimeFrequencyData
							contaminatedXY = artifacts.ContaminatedData(),
							revisedXY = artifacts.RevisedData(),
							originalXY = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						progress.OnStartTask(2, 4, "YX polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::YX, changeBoth);
						TimeFrequencyData
							contaminatedYX = artifacts.ContaminatedData(),
							revisedYX = artifacts.RevisedData(),
							originalYX = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						progress.OnStartTask(3, 4, "YY polarisation");
						performPolarisation(artifacts, progress, TimeFrequencyData::YY, changeBoth);
						TimeFrequencyData
							contaminatedYY = artifacts.ContaminatedData(),
							revisedYY = artifacts.RevisedData(),
							originalYY = artifacts.OriginalData();
						artifacts.SetContaminatedData(oldContaminatedData);
						artifacts.SetRevisedData(oldRevisedData);
						artifacts.SetOriginalData(oldOriginalData);
						progress.OnEndTask();

						TimeFrequencyData *newTF =
							TimeFrequencyData::CreateTFDataFromDipoleCombination(
								contaminatedXX, contaminatedXY, contaminatedYX, contaminatedYY);
						artifacts.SetContaminatedData(*newTF);
						delete newTF;

						newTF = 
							TimeFrequencyData::CreateTFDataFromDipoleCombination(
								revisedXX, revisedXY, revisedYX, revisedYY);
						artifacts.SetRevisedData(*newTF);
						delete newTF;

						newTF = 
							TimeFrequencyData::CreateTFDataFromDipoleCombination(
								originalXX, originalXY, originalYX, originalYY);
						artifacts.SetOriginalData(*newTF);
						delete newTF;
					}
					break;
				}
			}
		private:
			void performPolarisation(ArtifactSet &artifacts, ProgressListener &progress, enum TimeFrequencyData::PolarisationType polarisation, bool changeBoth)
			{
				TimeFrequencyData *newContaminatedData =
					artifacts.ContaminatedData().CreateTFData(polarisation);
				TimeFrequencyData *newOriginalData =
					artifacts.OriginalData().CreateTFData(polarisation);
				artifacts.SetContaminatedData(*newContaminatedData);
				artifacts.SetOriginalData(*newOriginalData);
				delete newContaminatedData;
				delete newOriginalData;

				TimeFrequencyData *newRevised;
				if(changeBoth)
				{
					newRevised = artifacts.RevisedData().CreateTFData(polarisation);
					artifacts.SetRevisedData(*newRevised);
					delete newRevised;
				}

				ActionBlock::Perform(artifacts, progress);
			}
	};

}

#endif // RFI_FOREACHPOLARISATION_H
