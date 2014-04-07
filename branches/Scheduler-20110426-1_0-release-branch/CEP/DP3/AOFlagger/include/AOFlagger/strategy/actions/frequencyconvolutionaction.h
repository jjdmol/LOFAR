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

#include <AOFlagger/msio/samplerow.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/util/aologger.h>

#include <AOFlagger/strategy/actions/action.h>

#include <AOFlagger/strategy/control/actionblock.h>
#include <AOFlagger/strategy/control/artifactset.h>

#include <AOFlagger/strategy/algorithms/thresholdtools.h>

namespace rfiStrategy {

	class FrequencyConvolutionAction : public Action
	{
		public:
			enum KernelKind { RectangleKernel, SincKernel };
			
			FrequencyConvolutionAction() : Action(), _kernelKind(RectangleKernel), _convolutionSize(2.0), _inSamples(false)
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

				Image2DPtr newImage;
				switch(_kernelKind)
				{
					default:
					case RectangleKernel:
					newImage = ThresholdTools::FrequencyRectangularConvolution(data.GetImage(0), (unsigned) roundn(_convolutionSize));
					break;
					case SincKernel:
					newImage = sincConvolution(artifacts.MetaData(), data.GetImage(0));
					break;
				}
				
				data.SetImage(0, newImage);
			}
			
			numl_t ConvolutionSize() const { return _convolutionSize; }
			void SetConvolutionSize(numl_t size) { _convolutionSize = size; }
			
			enum KernelKind KernelKind() const { return _kernelKind; }
			void SetKernelKind(enum KernelKind kind) { _kernelKind = kind; }
			
			bool InSamples() const { return _inSamples; }
			void SetInSamples(bool inSamples) { _inSamples = inSamples; }
		private:
			Image2DPtr sincConvolution(TimeFrequencyMetaDataCPtr metaData, Image2DCPtr source)
			{
				numl_t uvDist = averageUVDist(metaData);
				AOLogger::Debug << "Avg uv dist: " << uvDist << '\n';
				numl_t convolutionSize = convolutionSizeInSamples(uvDist, source->Height());
				AOLogger::Debug << "Convolution size: " << convolutionSize << '\n';
				Image2DPtr destination = Image2D::CreateEmptyImagePtr(source->Width(), source->Height());
				for(unsigned x=0;x<source->Width();++x)
				{
					SampleRowPtr row = SampleRow::CreateFromColumn(source, x);
					row->ConvolveWithSinc(1.0 / convolutionSize);
					row->SetVerticalImageValues(destination, x);
				}
				return destination;
			}
			
			numl_t averageUVDist(TimeFrequencyMetaDataCPtr metaData)
			{
				numl_t sum = 0.0;
				const numl_t
					lowFreq = metaData->Band().channels.begin()->frequencyHz,
					highFreq = metaData->Band().channels.rbegin()->frequencyHz;
				const std::vector<UVW> &uvw = metaData->UVW();
				for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
				{
					const numl_t
						lowU = i->u * lowFreq,
						lowV = i->v * lowFreq,
						highU = i->u * highFreq,
						highV = i->v * highFreq,
						ud = lowU - highU,
						vd = lowV - highV;
					sum += sqrtnl(ud * ud + vd * vd);
				}
				return sum / ((numl_t) uvw.size() * UVImager::SpeedOfLight());
			}
			
			numl_t convolutionSizeInSamples(numl_t uvDist, unsigned height)
			{
				if(_inSamples)
					return _convolutionSize;
				else
					return _convolutionSize * height / uvDist;
			}
			
			enum KernelKind _kernelKind;
			numl_t _convolutionSize;
			bool _inSamples;
	};

} // namespace

#endif // RFI_FREQUENCY_CONVOLUTION_ACTION
