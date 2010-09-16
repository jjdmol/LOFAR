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
	class ChangeResolutionAction : public ActionBlock {
		public:
			ChangeResolutionAction() : _timeDecreaseFactor(10), _frequencyDecreaseFactor(1), _restoreRevised(true), _restoreMasks(false)
			{
			}
			~ChangeResolutionAction()
			{
			}
			virtual std::string Description()
			{
				return "Change resolution";
			}

			void SetTimeDecreaseFactor(int newFactor) { _timeDecreaseFactor = newFactor; }
			int TimeDecreaseFactor() const { return _timeDecreaseFactor; }

			void SetFrequencyDecreaseFactor(int newFactor) { _frequencyDecreaseFactor = newFactor; }
			int FrequencyDecreaseFactor() const { return _frequencyDecreaseFactor; }

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return ChangeResolutionActionType; }
		private:
			void PerformFrequencyChange(class ArtifactSet &artifacts, class ProgressListener &listener);

			int _timeDecreaseFactor;
			int _frequencyDecreaseFactor;

			void DecreaseTime(TimeFrequencyData &data);
			void IncreaseTime(TimeFrequencyData &originalData, TimeFrequencyData &changedData, bool restoreImage);

			void DecreaseFrequency(TimeFrequencyData &data);
			void IncreaseFrequency(TimeFrequencyData &originalData, TimeFrequencyData &changedData, bool restoreImage);

			bool _restoreRevised;
			bool _restoreMasks;
	};

}
	
#endif
