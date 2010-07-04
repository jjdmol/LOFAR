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
		struct ChannelInfo {
			ChannelInfo() : frequencyHz(0), totalCount(0), rfiCount(0), rfiSummedAmplitude(0), broadbandRfiCount(0), lineRfiCount(0)
			{
			}
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
			TimestepInfo() : time(0), totalCount(0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiSummedAmplitude(0), broadbandRfiAmplitude(0), lineRfiAmplitude(0)
			{
			}
			TimestepInfo(double _time) : time(_time), totalCount(0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiSummedAmplitude(0), broadbandRfiAmplitude(0), lineRfiAmplitude(0)
			{
			}
			double time;
			long unsigned totalCount;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double rfiSummedAmplitude;
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
		struct BaselineInfo {
			BaselineInfo() : antenna1(0), antenna2(0), antenna1Name(), antenna2Name(), count(0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiSummedAmplitude(0), broadbandRfiAmplitude(0), lineRfiAmplitude(0)
			{
			}
			BaselineInfo(const BaselineInfo &source) : antenna1(source.antenna1), antenna2(source.antenna2), antenna1Name(source.antenna1Name), antenna2Name(source.antenna2Name), count(source.count), rfiCount(source.rfiCount), broadbandRfiCount(source.broadbandRfiCount), lineRfiCount(source.lineRfiCount), rfiSummedAmplitude(source.rfiSummedAmplitude), broadbandRfiAmplitude(source.broadbandRfiAmplitude), lineRfiAmplitude(source.lineRfiAmplitude)
			{
			}
			void operator=(const BaselineInfo &rhs)
			{
				antenna1 = rhs.antenna1;
				antenna2 = rhs.antenna2;
				antenna1Name = rhs.antenna1Name;
				antenna2Name = rhs.antenna2Name;
				count = rhs.count;
				rfiCount = rhs.rfiCount;
				broadbandRfiCount = rhs.broadbandRfiCount;
				lineRfiCount = rhs.lineRfiCount;
				rfiSummedAmplitude = rhs.rfiSummedAmplitude;
				broadbandRfiAmplitude = rhs.broadbandRfiAmplitude;
				lineRfiAmplitude = rhs.lineRfiAmplitude;
			}

			int antenna1, antenna2;
			std::string antenna1Name, antenna2Name;
			long unsigned count;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double rfiSummedAmplitude;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};

		RFIStatistics();
		~RFIStatistics();
		
		void Add(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData);
		void Add(const ChannelInfo &channel, bool autocorrelation);
		void Add(const TimestepInfo &timestep, bool autocorrelation);
		void Add(const AmplitudeBin &amplitudeBin, bool autocorrelation);
		void Add(const BaselineInfo &baseline);

		static long double FitScore(const Image2D &image, const Image2D &fit, Mask2DCPtr mask);
		static long double FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask);

		static num_t DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX);

		static num_t FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel);
		
		void Save()
		{
			saveChannels(_autoChannels, "counts-channels-auto.txt");
			saveTimesteps(_autoTimesteps, "counts-timesteps-auto.txt");
			saveAmplitudes(_autoAmplitudes, "counts-amplitudes-auto.txt");
			saveChannels(_crossChannels, "counts-channels-cross.txt");
			saveTimesteps(_crossTimesteps, "counts-timesteps-cross.txt");
			saveAmplitudes(_crossAmplitudes, "counts-amplitudes-cross.txt");
			saveBaselines("counts-baselines.txt");
		}

	private:
		typedef std::map<int, std::map<int, BaselineInfo> > BaselineMatrix;
		
		std::map<double, class ChannelInfo> _autoChannels, _crossChannels;
		std::map<double, class TimestepInfo> _autoTimesteps, _crossTimesteps;
		std::map<double, class AmplitudeBin> _autoAmplitudes, _crossAmplitudes;
		BaselineMatrix _baselines;
		
		void addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addTimesteps(std::map<double, class TimestepInfo> &timesteps, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addBaselines(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);

		void saveChannels(std::map<double, class ChannelInfo> &channels, const char *filename);
		void saveTimesteps(std::map<double, class TimestepInfo> &timesteps, const char *filename);
		void saveAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, const char *filename);
		void saveBaselines(const char *filename);
		
		double getCentralAmplitude(double amplitude)
		{
			//double decimals = pow(10.0, floor(log10(amplitude)));
			//return round(100.0 * amplitude / decimals) / 100.0 * decimals;
			return pow(10.0, round(100.0*log10(amplitude))/100.0);
		}
		boost::mutex _mutex;
};

#endif
