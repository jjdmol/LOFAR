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
#ifndef RFI_FOURIERTRANSFORM_ACTION_H
#define RFI_FOURIERTRANSFORM_ACTION_H

#include <iostream>

#include <AOFlagger/util/ffttools.h>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/strategy/actions/action.h>

#include <AOFlagger/strategy/control/artifactset.h>
#include <AOFlagger/strategy/control/actionblock.h>

namespace rfiStrategy {

	class FourierTransformAction : public Action
	{
		public:
			FourierTransformAction() : Action(), _inverse(false), _dynamic(false), _sections(64)
			{
			}
			virtual std::string Description()
			{
				if(_inverse)
					return "Inv Fourier transform";
				else
					return "Fourier transform";
			}
			virtual ActionType Type() const { return FourierTransformActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &)
			{
				perform(artifacts.ContaminatedData(), artifacts.MetaData());
			}
			bool Inverse() const { return _inverse; }
			void SetInverse(bool inverse) { _inverse = inverse; }
		private:
			void perform(TimeFrequencyData &data, TimeFrequencyMetaDataCPtr /*metaData*/)
			{
				if(data.PhaseRepresentation() != TimeFrequencyData::ComplexRepresentation || data.ImageCount() != 2)
					throw std::runtime_error("Fourier transform action needs a single complex image as input");
				Image2DPtr
					real = Image2D::CreateCopy(data.GetImage(0)),
					imaginary = Image2D::CreateCopy(data.GetImage(1));
				if(_dynamic)
				{
					FFTTools::CreateDynamicHorizontalFFTImage(real, imaginary, _sections, _inverse);
				} else {
					FFTTools::CreateHorizontalFFTImage(*real, *imaginary, _inverse);
				}
				data.SetImage(0, real);
				data.SetImage(1, imaginary);
			}

			bool _inverse;
			bool _dynamic;
			unsigned _sections;
	};

} // namespace

#endif // RFI_FOURIERTRANSFORM_ACTION_H
