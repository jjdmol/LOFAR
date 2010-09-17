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
#ifndef CHANGERESOLUTIONACTION_H
#define CHANGERESOLUTIONACTION_H

#include "../../msio/timefrequencydata.h"

#include "actionblock.h"

namespace rfiStrategy {
	
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class CutAreaAction : public ActionBlock {
		public:
			ChangeResolutionAction() : _topChannels(1), _bottomChannel(0), _startTimeSteps(0), _endTimeSteps(0)
			{
			}
			~ChangeResolutionAction()
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

			void SetBottomChannels(int channels) { _bottomChannel = channels; }
			int BottomChannels() const { return _bottomChannel; }

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &progress)
			{
				TimeFrequencyData oldOriginal(artifacts.OriginalData());
				TimeFrequencyData oldRevised(artifacts.RevisedData());
				TimeFrequencyData oldContaminated(artifacts.ContaminatedData());

				Cut(artifacts.OriginalData());
				Cut(artifacts.RevisedData());
				Cut(artifacts.ContaminatedData());

				ActionBlock::Perform(artifactsm, progress);

				artifacts.SetOriginalData(oldOriginal);
				artifacts.SetRevisedData(oldRevised);
				artifacts.SetContaminatedData(oldContaminated);
			}

			virtual ActionType Type() const { return CutAreaActionType; }
		private:
			void Cut(class TimeFrequencyData &data)
			{
				size_t width = _endTimeSteps - _startTimeSteps;
			}

			int _startTimeSteps;
			int _endTimeSteps;
			int _topChannels;
			int _bottomChannel;
	};

}
	
#endif
