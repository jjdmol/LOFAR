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
#ifndef RFI_TIME_CONVOLUTION_ACTION
#define RFI_TIME_CONVOLUTION_ACTION

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/actionblock.h>

#include <AOFlagger/rfi/strategy/action.h>

#include <AOFlagger/rfi/thresholdtools.h>

namespace rfiStrategy {

	class TimeConvolutionAction : public Action
	{
		public:
			TimeConvolutionAction() : Action(), _sincSize(10000.0)
			{
			}
			virtual std::string Description()
			{
				return "Time convolution";
			}
			virtual ActionType Type() const { return TimeConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				num_t sincScale = _sincSize / maxUVDistance(artifacts.MetaData()->UVW());
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()*3];
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();
				num_t sign;
				if(data.PhaseRepresentation() == TimeFrequencyData::RealPart)
					sign = 1.0;
				else if(data.PhaseRepresentation() == TimeFrequencyData::ImaginaryPart)
					sign = -1.0;
				else
					throw BadUsageException("Data is not real or imaginary");
				for(unsigned y=0;y<image->Height();++y)
				{
					for(unsigned x=0;x<width;++x) {
						row[x] = sign * image->Value(x, y);
						row[x+width] = image->Value(x, y);
						row[x+2*width] = sign * image->Value(x, y);
					}
					ThresholdTools::OneDimensionalSincConvolution(row, width*3, sincScale);
					for(unsigned x=0;x<width;++x)
						newImage->SetValue(x, y, row[x+width]);
				}
				delete[] row;
				
				TimeFrequencyData newRevisedData = TimeFrequencyData(data.PhaseRepresentation(), data.Polarisation(), newImage);
				newRevisedData.SetMask(artifacts.RevisedData());

				TimeFrequencyData *contaminatedData =
					TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
				contaminatedData->SetMask(artifacts.ContaminatedData());
				artifacts.SetRevisedData(newRevisedData);
				artifacts.SetContaminatedData(*contaminatedData);
				delete contaminatedData;
			}
		private:
			double _sincSize;
			
			num_t maxUVDistance(const std::vector<UVW> &uvw)
			{
				num_t maxDist = 0.0;
				for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
				{
					num_t dist = i->u * i->u + i->v * i->v;
					if(dist > maxDist) maxDist = dist;
				}
				return sqrtn(maxDist);
			}
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
