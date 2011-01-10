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
#ifndef RFI_DIRECTIONALCLEAN_ACTION_H
#define RFI_DIRECTIONALCLEAN_ACTION_H

#include <iostream>

#include <AOFlagger/util/ffttools.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/action.h>
#include <AOFlagger/rfi/strategy/actionblock.h>

#include <AOFlagger/rfi/uvprojection.h>

namespace rfiStrategy {

	class DirectionalCleanAction : public Action
	{
		public:
			DirectionalCleanAction() : Action(), _limitingDistance(1.0)
			{
			}
			virtual std::string Description()
			{
				return "Directional cleaning";
			}
			virtual ActionType Type() const { return DirectionalCleanActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				TimeFrequencyData &contaminated = artifacts.ContaminatedData();
				if(contaminated.ImageCount() != 2 || contaminated.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
					throw std::runtime_error("Directional clean action requires single complex image in contaminated data");

				TimeFrequencyData &revised = artifacts.RevisedData();
				if(revised.ImageCount() != 2 || revised.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation)
					throw std::runtime_error("Directional clean action requires single complex image in revised data");

				Image2DPtr
					realDest = Image2D::CreateCopy(revised.GetRealPart()),
					imagDest = Image2D::CreateCopy(revised.GetImaginaryPart());
				for(unsigned y=0;y<artifacts.ContaminatedData().ImageHeight();++y)
				{
					performFrequency(artifacts, realDest, imagDest, y);
				}
				revised.SetImage(0, realDest);
				revised.SetImage(1, imagDest);
			}
		private:
			double _limitingDistance;

			void performFrequency(ArtifactSet &artifacts, Image2DPtr realDest, Image2DPtr imagDest, unsigned y)
			{
				Image2DCPtr
					realInput = artifacts.ContaminatedData().GetRealPart(),
					imagInput = artifacts.ContaminatedData().GetImaginaryPart();
				
				SampleRowCPtr row = SampleRow::CreateAmplitudeFromRow(realInput, imagInput, y);
				unsigned fIndex = row->IndexOfMax();
				
				AOLogger::Debug << "Removing component index " << fIndex << '\n';
				
				const size_t
					inputWidth = realInput->Width(),
					destWidth = realDest->Width();
				numl_t
					*rowUPositions = new numl_t[inputWidth],
					*rowVPositions = new numl_t[inputWidth];
					
				UVProjection::ProjectPositions(artifacts.MetaData(), inputWidth, y, rowUPositions, rowVPositions, artifacts.ProjectedDirectionRad());
				
				numl_t minU, maxU;
				UVProjection::MaximalUPositions(inputWidth, rowUPositions, minU, maxU);
				
				unsigned lowestIndex, highestIndex;
				UVProjection::GetIndicesInProjectedImage(_limitingDistance, minU, maxU, inputWidth, destWidth, lowestIndex, highestIndex);
				
				numl_t mean = row->Mean(), sigma = row->StdDev(mean);
				numl_t
					diffRealValue = realInput->Value(fIndex, y) * 0.75,
					diffImagValue = imagInput->Value(fIndex, y) * 0.75;

				AOLogger::Debug << "Mean=" << mean << ", sigma=" << sigma << ", component = " << ((row->Value(fIndex)-mean)/sigma) << " x sigma\n";

				subtractComponent(artifacts, realDest, imagDest, inputWidth, rowUPositions, fIndex, diffRealValue, diffImagValue, y);
				
				if(fIndex >= lowestIndex && fIndex < highestIndex)
				{
					AOLogger::Debug << "Within limits " << lowestIndex << "-" << highestIndex << '\n';
				}
			}

			void subtractComponent(ArtifactSet &artifacts, Image2DPtr real, Image2DPtr imaginary, const size_t inputWidth, const numl_t *uPositions, unsigned fIndex, numl_t diffR, numl_t diffI, unsigned y)
			{
				numl_t minU, maxU;
				UVProjection::MaximalUPositions(inputWidth, uPositions, minU, maxU);

				numl_t w = (numl_t) fIndex / real->Width();
				if(w > real->Width()/2) w = real->Width() - w;

				numl_t amplitude = sqrtnl(diffR*diffR + diffI*diffI);
				numl_t phase = atan2nl(diffR, diffI);
				
				// The following component will be subtracted:
				// amplitude e ^ ( -i (2 pi w (u - minU) / (maxU - minU) + phase) )
				
				// prefactor w
				w = w / (maxU - minU);
				
				for(unsigned t=0;t<real->Width();++t)
				{
					numl_t u = uPositions[t];
					numl_t exponent = 2.0 * M_PInl * w * (u - minU);
					numl_t realValue = amplitude * cosnl(exponent + phase);
					numl_t imagValue = amplitude * sinnl(exponent + phase);
					real->SetValue(t, y, real->Value(t, y) - realValue);
					imaginary->SetValue(t, y, imaginary->Value(t, y) - imagValue);
				}
			}
	};

} // namespace

#endif // RFI_DIRECTIONALCLEAN_ACTION_H
