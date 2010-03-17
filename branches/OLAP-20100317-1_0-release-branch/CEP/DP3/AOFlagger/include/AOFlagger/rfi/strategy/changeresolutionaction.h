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
			ChangeResolutionAction() : _decreaseFactor(10)
			{
			}

			~ChangeResolutionAction()
			{
			}
			virtual std::string Description()
			{
				return "Change resolution";
			}
			void SetDecreaseFactor(int newFactor) { _decreaseFactor = newFactor; }
			int DecreaseFactor() const { return _decreaseFactor; }

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener);
			virtual ActionType Type() const { return ChangeResolutionActionType; }
		private:
			int _decreaseFactor;
			void DecreaseSize(TimeFrequencyData &data);
			void IncreaseSize(TimeFrequencyData &originalData, TimeFrequencyData &changedData);
	};

}
	
#endif
