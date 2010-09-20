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
#ifndef CUTAREAACTION_H
#define CUTAREAACTION_H

#include <AOFlagger/msio/timefrequencydata.h>

#include "actionblock.h"
#include "artifactset.h"

namespace rfiStrategy {
	
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class CutAreaAction : public ActionBlock {
		public:
			CutAreaAction() : _startTimeSteps(0), _endTimeSteps(0), _topChannels(1), _bottomChannels(0)
			{
			}
			~CutAreaAction()
			{
			}
			virtual std::string Description()
			{
				return "Cut area";
			}

			void SetStartTimeSteps(int channels) { _startTimeSteps = channels; }
			int StartTimeSteps() const { return _startTimeSteps; }

			void SetEndTimeSteps(int channels) { _endTimeSteps = channels; }
			int EndTimeSteps() const { return _endTimeSteps; }

			void SetTopChannels(int channels) { _topChannels = channels; }
			int TopChannels() const { return _topChannels; }

			void SetBottomChannels(int channels) { _bottomChannels = channels; }
			int BottomChannels() const { return _bottomChannels; }

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &progress)
			{
				TimeFrequencyData oldOriginal(artifacts.OriginalData());
				TimeFrequencyData oldRevised(artifacts.RevisedData());
				TimeFrequencyData oldContaminated(artifacts.ContaminatedData());

				Cut(artifacts.OriginalData());
				Cut(artifacts.RevisedData());
				Cut(artifacts.ContaminatedData());

				ActionBlock::Perform(artifacts, progress);
				
				PlaceBack(artifacts.OriginalData(), oldOriginal);
				PlaceBack(artifacts.RevisedData(), oldRevised);
				PlaceBack(artifacts.ContaminatedData(), oldContaminated);

				artifacts.SetOriginalData(oldOriginal);
				artifacts.SetRevisedData(oldRevised);
				artifacts.SetContaminatedData(oldContaminated);
			}

			virtual ActionType Type() const { return CutAreaActionType; }
		private:
			void Cut(class TimeFrequencyData &data)
			{
				size_t endTime = data.ImageWidth() - _endTimeSteps;
				size_t endChannel = data.ImageHeight() - _bottomChannels;
				data.Trim(_startTimeSteps, _topChannels, endTime, endChannel);
			}
			void PlaceBack(class TimeFrequencyData &cuttedData, class TimeFrequencyData &oldData)
			{
				oldData.CopyFrom(cuttedData, _startTimeSteps, _topChannels);
			}

			int _startTimeSteps;
			int _endTimeSteps;
			int _topChannels;
			int _bottomChannels;
	};

}
	
#endif // CUTAREAACTION_H
