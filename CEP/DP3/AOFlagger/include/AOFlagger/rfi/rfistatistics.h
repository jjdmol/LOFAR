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
#ifndef RFISTATISTICS_H
#define RFISTATISTICS_H

#include <cstring>
#include <map>

#include <boost/thread/mutex.hpp>

#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/timefrequencymetadata.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/segmentedimage.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class RFIStatistics {
	public:
		RFIStatistics();
		~RFIStatistics();
		void Add(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData);

		static long double FitScore(const Image2D &image, const Image2D &fit, Mask2DCPtr mask);
		static long double FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask);

		static num_t DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX);

		static num_t FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel);

	private:
		struct ChannelInfo {
			ChannelInfo(double _frequencyHz) : frequencyHz(_frequencyHz), totalCount(0), rfiCount(0), rfiSummedAmplitude(0), broadbandRfiCount(0), lineRfiCount(0)
			{
			}
			double frequencyHz;
			long unsigned totalCount;
			long unsigned rfiCount;
			long double rfiSummedAmplitude;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};
		struct TimestepInfo {
			TimestepInfo(double _time) : time(_time), totalCount(0), rfiCount(0), rfiSummedAmplitude(0), broadbandRfiCount(0), lineRfiCount(0), broadbandRfiAmplitude(0), lineRfiAmplitude(0)
			{
			}
			double time;
			long unsigned totalCount;
			long unsigned rfiCount;
			long double rfiSummedAmplitude;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};
		struct AmplitudeBin {
			AmplitudeBin() : centralAmplitude(0.0), centralLogAmplitude(0.0), count(0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0)
			{
			}
			double centralAmplitude;
			double centralLogAmplitude;
			long unsigned count;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
		};
		std::map<double, class ChannelInfo> _autoChannels, _crossChannels;
		std::map<double, class TimestepInfo> _autoTimesteps, _crossTimesteps;
		std::map<double, class AmplitudeBin> _autoAmplitudes, _crossAmplitudes;
		
		void addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addTimesteps(std::map<double, class TimestepInfo> &timesteps, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);

		void saveChannels(std::map<double, class ChannelInfo> &channels, const char *filename);
		void saveTimesteps(std::map<double, class TimestepInfo> &timesteps, const char *filename);
		void saveAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, const char *filename);
		
		double getCentralAmplitude(double amplitude)
		{
			//double decimals = pow(10.0, floor(log10(amplitude)));
			//return round(100.0 * amplitude / decimals) / 100.0 * decimals;
			return pow(10.0, round(100.0*log10(amplitude))/100.0);
		}
		boost::mutex _mutex;
};

#endif
