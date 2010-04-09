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
#include <AOFlagger/rfi/antennaflagcountplot.h>
#include <AOFlagger/rfi/frequencyflagcountplot.h>
#include <AOFlagger/rfi/frequencypowerplot.h>
#include <AOFlagger/rfi/timeflagcountplot.h>

#include <AOFlagger/rfi/strategy/plotaction.h>
#include <AOFlagger/rfi/strategy/artifactset.h>

namespace rfiStrategy {
	
	void PlotAction::Perform(ArtifactSet &artifacts, ProgressListener &listener)
	{
		switch(_plotKind)
		{
			case AntennaFlagCountPlot:
				plotAntennaFlagCounts(artifacts);
				break;
			case FrequencyFlagCountPlot:
				plotFrequencyFlagCounts(artifacts);
				break;
			case FrequencyPowerPlot:
				plotFrequencyPower(artifacts);
				break;
			case TimeFlagCountPlot:
				plotTimeFlagCounts(artifacts);
				break;
			case BaselineSpectrumPlot:
				plotSpectrumPerBaseline(artifacts);
				break;
		}
	}

	void PlotAction::plotAntennaFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.AntennaFlagCountPlot() == 0)
			throw BadUsageException("No antenna flag count plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.AntennaFlagCountPlot()->Add(data, meta);
	}

	void PlotAction::plotFrequencyFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.FrequencyFlagCountPlot() == 0)
			throw BadUsageException("No frequency flag count plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.FrequencyFlagCountPlot()->Add(data, meta);
	}

	void PlotAction::plotFrequencyPower(ArtifactSet &artifacts)
	{
		if(artifacts.FrequencyPowerPlot() == 0)
			throw BadUsageException("No frequency power plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.FrequencyPowerPlot()->Add(data, meta);
	}

	void PlotAction::plotTimeFlagCounts(ArtifactSet &artifacts)
	{
		if(artifacts.TimeFlagCountPlot() == 0)
			throw BadUsageException("No time flag count plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.TimeFlagCountPlot()->Add(data, meta);
	}

	void PlotAction::plotSpectrumPerBaseline(ArtifactSet &artifacts)
	{
		if(artifacts.FrequencyPowerPlot() == 0)
			throw BadUsageException("No frequency power plot in the artifact set");

		TimeFrequencyData &data = artifacts.ContaminatedData();
		TimeFrequencyMetaDataCPtr meta = artifacts.MetaData();
		artifacts.FrequencyPowerPlot()->SetLogYAxis(_logYAxis);
		artifacts.FrequencyPowerPlot()->StartNewLine(meta->Antenna1().name + " x " + meta->Antenna2().name);
		artifacts.FrequencyPowerPlot()->Add(data, meta);
	}
}
