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
#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/timefrequencymetadata.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/segmentedimage.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class RFIStatistics {
	public:
		struct ChannelInfo {
			ChannelInfo() : frequencyHz(0), totalCount(0), totalAmplitude(0.0), rfiCount(0), rfiAmplitude(0.0), broadbandRfiCount(0), lineRfiCount(0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			ChannelInfo(double _frequencyHz) : frequencyHz(_frequencyHz), totalCount(0), totalAmplitude(0.0), rfiCount(0), rfiAmplitude(0.0), broadbandRfiCount(0), lineRfiCount(0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			double frequencyHz;
			long unsigned totalCount;
			long double totalAmplitude;
			long unsigned rfiCount;
			long double rfiAmplitude;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};
		struct TimestepInfo {
			TimestepInfo() : time(0), totalCount(0), totalAmplitude(0.0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiAmplitude(0.0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			TimestepInfo(double _time) : time(_time), totalCount(0), totalAmplitude(0.0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiAmplitude(0.0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			double time;
			long unsigned totalCount;
			long double totalAmplitude;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double rfiAmplitude;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};
		struct AmplitudeBin {
			AmplitudeBin() : centralAmplitude(0.0), count(0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), featureAvgCount(0), featureMaxCount(0), featureIntCount(0), xxCount(0), xyCount(0), yxCount(0), yyCount(0), xxRfiCount(0), xyRfiCount(0), yxRfiCount(0), yyRfiCount(0)
			{
			}
			double centralAmplitude;
			long unsigned count;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long unsigned featureAvgCount;
			long unsigned featureMaxCount;
			long unsigned featureIntCount;
			long unsigned xxCount;
			long unsigned xyCount;
			long unsigned yxCount;
			long unsigned yyCount;
			long unsigned xxRfiCount;
			long unsigned xyRfiCount;
			long unsigned yxRfiCount;
			long unsigned yyRfiCount;
		};
		struct BaselineInfo {
			BaselineInfo() : antenna1(0), antenna2(0), antenna1Name(), antenna2Name(), baselineLength(0.0), baselineAngle(0.0), count(0), totalAmplitude(0.0), rfiCount(0), broadbandRfiCount(0), lineRfiCount(0), rfiAmplitude(0.0), broadbandRfiAmplitude(0.0), lineRfiAmplitude(0.0)
			{
			}
			BaselineInfo(const BaselineInfo &source) : antenna1(source.antenna1), antenna2(source.antenna2), antenna1Name(source.antenna1Name), antenna2Name(source.antenna2Name), baselineLength(source.baselineLength), baselineAngle(source.baselineAngle), count(source.count), totalAmplitude(source.totalAmplitude), rfiCount(source.rfiCount), broadbandRfiCount(source.broadbandRfiCount), lineRfiCount(source.lineRfiCount), rfiAmplitude(source.rfiAmplitude), broadbandRfiAmplitude(source.broadbandRfiAmplitude), lineRfiAmplitude(source.lineRfiAmplitude)
			{
			}
			void operator=(const BaselineInfo &rhs)
			{
				antenna1 = rhs.antenna1;
				antenna2 = rhs.antenna2;
				antenna1Name = rhs.antenna1Name;
				antenna2Name = rhs.antenna2Name;
				baselineLength = rhs.baselineLength;
				baselineAngle = rhs.baselineAngle;
				count = rhs.count;
				totalAmplitude = rhs.totalAmplitude;
				rfiCount = rhs.rfiCount;
				broadbandRfiCount = rhs.broadbandRfiCount;
				lineRfiCount = rhs.lineRfiCount;
				rfiAmplitude = rhs.rfiAmplitude;
				broadbandRfiAmplitude = rhs.broadbandRfiAmplitude;
				lineRfiAmplitude = rhs.lineRfiAmplitude;
			}

			int antenna1, antenna2;
			std::string antenna1Name, antenna2Name;
			double baselineLength;
			double baselineAngle;
			long unsigned count;
			double totalAmplitude;
			long unsigned rfiCount;
			long unsigned broadbandRfiCount;
			long unsigned lineRfiCount;
			long double rfiAmplitude;
			long double broadbandRfiAmplitude;
			long double lineRfiAmplitude;
		};

		RFIStatistics();
		~RFIStatistics();
		
		void Add(const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData);
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
			saveSubbands(_autoChannels, "counts-subbands-auto.txt");
			saveSubbands(_crossChannels, "counts-subbands-cross.txt");
		}
		const std::map<double, class AmplitudeBin> &GetAutoAmplitudes() const
		{
			return _autoAmplitudes;
		}
		const std::map<double, class AmplitudeBin> &GetCrossAmplitudes() const
		{
			return _crossAmplitudes;
		}
		double RFIFractionInCrossChannels() const
		{
			return rfiFraction(_crossChannels);
		}
		double RFIFractionInAutoChannels() const
		{
			return rfiFraction(_autoChannels);
		}
		double RFIFractionInCrossTimeSteps() const
		{
			return rfiFraction(_crossTimesteps);
		}
		double RFIFractionInAutoTimeSteps() const
		{
			return rfiFraction(_autoTimesteps);
		}
		double AmplitudeCrossSlope(double startAmplitude, double endAmplitude) const
		{
			return amplitudeSlope(_crossAmplitudes, startAmplitude, endAmplitude);
		}
		long unsigned AmplitudeCrossCount(double startAmplitude, double endAmplitude)
		{
			return count(_crossAmplitudes, startAmplitude, endAmplitude);
		}
	private:
		struct FeatureInfo {
			long double amplitudeSum;
			num_t amplitudeMax;
			size_t sampleCount;
		};
		typedef std::map<size_t, struct FeatureInfo> FeatureMap;
		typedef std::map<int, std::map<int, BaselineInfo> > BaselineMatrix;
		
		std::map<double, class ChannelInfo> _autoChannels, _crossChannels;
		std::map<double, class TimestepInfo> _autoTimesteps, _crossTimesteps;
		std::map<double, class AmplitudeBin> _autoAmplitudes, _crossAmplitudes;
		BaselineMatrix _baselines;
		
		void addChannels(std::map<double, class ChannelInfo> &channels, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addTimesteps(std::map<double, class TimestepInfo> &timesteps, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addBaselines(Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addFeatures(std::map<double, class AmplitudeBin> &amplitudes, Image2DCPtr image, Mask2DCPtr mask, TimeFrequencyMetaDataCPtr metaData, SegmentedImageCPtr segmentedImage);
		void addPolarisations(std::map<double, class AmplitudeBin> &amplitudes, const TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData);

		void saveChannels(std::map<double, class ChannelInfo> &channels, const char *filename);
		void saveTimesteps(std::map<double, class TimestepInfo> &timesteps, const char *filename);
		void saveAmplitudes(std::map<double, class AmplitudeBin> &amplitudes, const char *filename);
		void saveBaselines(const char *filename);
		void saveSubbands(std::map<double, class ChannelInfo> &channels, const char *filename);
		
		double getCentralAmplitude(double amplitude)
		{
			//double decimals = pow(10.0, floor(log10(amplitude)));
			//return round(100.0 * amplitude / decimals) / 100.0 * decimals;
			return pow(10.0, round(100.0*log10(amplitude))/100.0);
		}
		boost::mutex _mutex;
		
		double rfiFraction(const std::map<double, class ChannelInfo> &channels) const
		{
			unsigned long totalRFI = 0, total = 0;
			for(std::map<double, class ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
			{
				totalRFI += i->second.rfiCount;
				total += i->second.totalCount;
			}
			return (long double) totalRFI / (long double) total;
		}
		double rfiFraction(const std::map<double, class TimestepInfo> &timesteps) const
		{
			unsigned long totalRFI = 0, total = 0;
			for(std::map<double, class TimestepInfo>::const_iterator i=timesteps.begin();i!=timesteps.end();++i)
			{
				totalRFI += i->second.rfiCount;
				total += i->second.totalCount;
			}
			return (long double) totalRFI / (long double) total;
		}
		double amplitudeSlope(const std::map<double, class AmplitudeBin> &amplitudes, double startAmplitude, double endAmplitude) const
		{
			unsigned long n = 0;
			long double sumX = 0.0, sumXY = 0.0, sumY = 0.0, sumXSquare = 0.0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
			{
				if(i->first > startAmplitude && i->first<endAmplitude)
				{
					double x = log10(i->first);
					double y = log10(i->second.count/i->first);
					++n;
					sumX += x;
					sumXSquare += x * x;
					sumY += y;
					sumXY += x * y;
				}
			}
			return (sumXY - sumX*sumY/n)/(sumXSquare - (sumX*sumX/n));
		}
		long unsigned count(const std::map<double, class AmplitudeBin> &amplitudes, double startAmplitude, double endAmplitude) const
		{
			unsigned long n = 0;
			for(std::map<double, class AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
			{
				if(i->first > startAmplitude && i->first<endAmplitude)
					n+=i->second.count;
			}
			return n;
		}
};

#endif
