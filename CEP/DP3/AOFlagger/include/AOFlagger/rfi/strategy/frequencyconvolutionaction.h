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
#ifndef RFI_FREQUENCY_CONVOLUTION_ACTION
#define RFI_FREQUENCY_CONVOLUTION_ACTION

#include <stdexcept>

#include <AOFlagger/rfi/strategy/action.h>
#include <AOFlagger/rfi/strategy/actionblock.h>
#include <AOFlagger/rfi/strategy/artifactset.h>

#include <AOFlagger/rfi/thresholdtools.h>

namespace rfiStrategy {

	class FrequencyConvolutionAction : public Action
	{
		public:
			FrequencyConvolutionAction() : Action(), _convolutionSize(16)
			{
			}
			virtual std::string Description()
			{
				return "Frequency convolution";
			}
			virtual ActionType Type() const { return FrequencyConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData &data = artifacts.ContaminatedData();
				if(data.ImageCount() != 1)
					throw std::runtime_error("A frequency convolution can only be applied on single component data");

				Image2DPtr image = ThresholdTools::FrequencyRectangularConvolution(data.GetImage(0), _convolutionSize);
				data.SetImage(0, image);
			}
			
			unsigned ConvolutionSize() const { return _convolutionSize; }
			void SetConvolutionSize(unsigned size) { _convolutionSize = size; }
		private:
			unsigned _convolutionSize;
	};

} // namespace

#endif // RFI_FREQUENCY_CONVOLUTION_ACTION
