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
#ifndef TIMESELECTIONACTION_H
#define TIMESELECTIONACTION_H

#include "../../msio/types.h"

#include "action.h"

namespace rfiStrategy {
	
	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class TimeSelectionAction : public Action {
		public:
			TimeSelectionAction() : _partCount(25), _selectionCount(20), _manualSelection(false),  _threshold(3.5)
			{
			}
			~TimeSelectionAction()
			{
			}
			virtual std::string Description()
			{
				return "Time selection";
			}
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				if(_manualSelection)
					ManualSelection(artifacts, listener);
				else
					AutomaticSelection(artifacts, listener);
			}
			virtual ActionType Type() const { return TimeSelectionActionType; }

			size_t PartCount() const { return _partCount; }
			void SetPartCount(size_t partCount) { _partCount = partCount; }

			size_t SelectionCount() const { return _selectionCount; }
			void SetSelectionCount(size_t selectionCount) { _selectionCount = selectionCount; }

			num_t Threshold() const { return _threshold; }
			void SetThreshold(num_t threshold) { _threshold = threshold; }
		private:
			void ManualSelection(ArtifactSet &artifacts, class ProgressListener &listener);
			void AutomaticSelection(ArtifactSet &artifacts, class ProgressListener &listener);

			size_t _partCount;
			size_t _selectionCount;
			bool _manualSelection;
			num_t _threshold;
	};

}

#endif
