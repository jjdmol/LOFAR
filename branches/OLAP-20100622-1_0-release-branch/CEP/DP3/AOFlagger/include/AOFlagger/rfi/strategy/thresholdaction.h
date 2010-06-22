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
#ifndef RFI_THRESHOLDACTION
#define RFI_THRESHOLDACTION

#include "../thresholdconfig.h"

#include "artifactset.h"
#include "actionblock.h"

namespace rfiStrategy {

	class ThresholdAction : public Action
	{
			public:
				ThresholdAction() : _baseSensitivity(1.0), _inTimeDirection(true), _inFrequencyDirection(true)
				{
					_thresholdConfig.InitializeLengthsDefault();
					_thresholdConfig.InitializeThresholdsFromFirstThreshold(6.0L, ThresholdConfig::Rayleigh);
				}
				virtual std::string Description()
				{
					return "SumThreshold";
				}
				virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
				{
					if(!_inTimeDirection)
						_thresholdConfig.RemoveHorizontalOperations();
					if(!_inFrequencyDirection)
						_thresholdConfig.RemoveVerticalOperations();
					
					TimeFrequencyData &contaminated = artifacts.ContaminatedData();
					Mask2DPtr mask = Mask2D::CreateCopy(contaminated.GetSingleMask());
					//_thresholdConfig.SetVerbose(true);
					_thresholdConfig.Execute(contaminated.GetSingleImage(), mask, false, artifacts.Sensitivity() * _baseSensitivity);
					contaminated.SetGlobalMask(mask);
				}
				num_t BaseSensitivity() const { return _baseSensitivity; }
				void SetBaseSensitivity(num_t baseSensitivity)
				{
					_baseSensitivity = baseSensitivity;
				}
				virtual ActionType Type() const { return ThresholdActionType; }
				
				bool TimeDirectionFlagging() const { return _inTimeDirection; }
				void SetTimeDirectionFlagging(bool timeDirection) { _inTimeDirection = timeDirection; }
				
				bool FrequencyDirectionFlagging() const { return _inFrequencyDirection; }
				void SetFrequencyDirectionFlagging(bool frequencyDirection) { _inFrequencyDirection = frequencyDirection; }
			private:
				ThresholdConfig _thresholdConfig;
				num_t _baseSensitivity;
				bool _inTimeDirection;
				bool _inFrequencyDirection;
	};

} // namespace

#endif // RFI_THRESHOLDACTION
