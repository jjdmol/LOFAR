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
#include <AOFlagger/rfi/rfistatistics.h>

#include <deque>
#include <iostream>
#include <cmath>

#include <AOFlagger/msio/timefrequencydata.h>

#include <AOFlagger/util/plot.h>

RFIStatistics::RFIStatistics() : _totalNonRFIFlux(0.0), _totalAddedImages(0), _totalAddedSamples(0),  _totalAddedNonRFISamples(0), _totalInvalidValues(0), _eightConnected(true), _binCount(100)
{
}

RFIStatistics::~RFIStatistics()
{
}

void RFIStatistics::Add(Image2DCPtr image, Mask2DCPtr mask)
{
	calculateLengths(image, mask);

	size_t rfiCount = 0;
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);

	
	for(unsigned y=0;y<maskCopy->Height();++y)
	{
		for(unsigned x=0;x<maskCopy->Width();++x)
		{
			if(maskCopy->Value(x, y))
			{
				std::vector<SamplePosition> positions;
				FindConnectedSamples(maskCopy, positions, SamplePosition(x,y));
				SetSamples(maskCopy, positions, false);
				RFISampleProperties properties;
				GetSampleProperties(properties, image, positions);
				_samplesConnectedAlgorithm.push_back(properties);
				rfiCount += positions.size();
			}
		}
	}
	maskCopy.reset();

	size_t samples = mask->Width() * mask->Height();
	_totalAddedImages++;
	_totalAddedSamples += samples;
	_totalAddedNonRFISamples += samples - rfiCount;
	_totalNonRFIFlux += AverageMasked(image, mask, true);
	_totalInvalidValues += InvalidValues(image, mask, false);
}

void RFIStatistics::GetSampleProperties(RFISampleProperties &properties, Image2DCPtr image, const std::vector<SamplePosition> &positions)
{
	if(positions.size() == 0)
		throw BadUsageException("RFIStatistics::GetSampleProperties was called with positions.size()==0");
	unsigned int minX=positions.front().x;
	unsigned int maxX=positions.front().x;
	unsigned int minY=positions.front().y;
	unsigned int maxY=positions.front().y;
	properties.size = positions.size();
	long double flux = 0;
	for(std::vector<SamplePosition>::const_iterator i=positions.begin();i!=positions.end();++i)
	{
		if(i->x < minX) minX = i->x;
		if(i->x > maxX) maxX = i->x;
		if(i->y < minY) minY = i->y;
		if(i->y > maxY) maxY = i->y;
		flux += image->Value(i->x, i->y);
	}
	properties.duration = maxX-minX + 1;
	properties.frequencyCoverage = maxY-minY + 1;
	properties.flux = flux / (long double) positions.size();
}

void RFIStatistics::calculateLengths(Image2DCPtr image, Mask2DCPtr mask)
{
	// The strategy:
	// - For each channel and each time stamp, calculate the longest connected (in time / frequency
	//   direction) RFI sequence, and store this.
	// - Pick the longest top or left most sequence, and unset all samples in the sequence that
	//   have not only RFI containing neighbours, and mark these samples in a separate mask as
	//   "processed". If not all of the pixels have been processed previously, count this as
	//   a new sequence.
	// - Update channel / time sequence counts, and go back to step 2 until all counts are 0.
	
	Mask2DPtr maskCopy = Mask2D::CreateCopy(mask);
	Mask2DPtr processed = Mask2D::CreateSetMaskPtr<false>(mask->Width(), mask->Height());
	
	size_t
		*frequencyMax = new size_t[mask->Width()],
		*timeMax = new size_t[mask->Height()],
		pos;

	for(size_t y=0;y<mask->Height();++y)
		timeMax[y] = GetTimeMax(maskCopy, y, pos);
	for(size_t x=0;x<mask->Width();++x)
		frequencyMax[x] = GetFrequencyMax(maskCopy, x, pos);

	size_t *maxTimeMax = std::max_element(timeMax, timeMax + mask->Height());
	size_t *maxFreqMax = std::max_element(frequencyMax, frequencyMax + mask->Width());

	while(*maxTimeMax != 0)
	{
		if(*maxTimeMax > *maxFreqMax)
		{
			size_t xPos;
			size_t y = maxTimeMax - timeMax;
			GetTimeMax(maskCopy, y, xPos);
			bool isProcessed = true;
			for(size_t x=xPos;x<xPos + *maxTimeMax;++x)
			{
				if(!isSurrounded(maskCopy, x, y))
					maskCopy->SetValue(x, y, false);
				if(!processed->Value(x, y))
					isProcessed = false;
				processed->SetValue(x, y, true);
				frequencyMax[x] = GetFrequencyMax(maskCopy, x, pos);
			}
			if(!isProcessed) {
				countTimeLine(image, xPos, *maxTimeMax, y);
			}
			*maxTimeMax = GetTimeMax(maskCopy, y, pos);
		} else {
			size_t yPos;
			size_t x = maxFreqMax - frequencyMax;
			GetFrequencyMax(maskCopy, x, yPos);
			bool isProcessed = true;
			for(size_t y=yPos;y<yPos + *maxFreqMax;++y)
			{
				if(!isSurrounded(maskCopy, x, y))
					maskCopy->SetValue(x, y, false);
				if(!processed->Value(x, y))
					isProcessed = false;
				processed->SetValue(x, y, true);
				timeMax[y] = GetTimeMax(maskCopy, y, pos);
			}
			if(!isProcessed) {
				countFrequencyLine(image, yPos, *maxFreqMax, x);
			}
			*maxFreqMax = GetFrequencyMax(maskCopy, x, pos);
		}

		maxTimeMax = std::max_element(timeMax, timeMax + mask->Height());
		maxFreqMax = std::max_element(frequencyMax, frequencyMax + mask->Width());
	}
}

size_t RFIStatistics::GetTimeMax(Mask2DCPtr mask, size_t channel, size_t &pos)
{
	size_t maxLength = 0, start = 0;
	bool hasStarted = false;
	for(size_t time=0;time<mask->Width();++time)
	{
		if(mask->Value(time, channel))
		{
			if(hasStarted)
			{
				size_t length = time - start + 1;
				if(maxLength < length) {
					maxLength = length;
					pos = start;
				}
			} else {
				hasStarted = true;
				start = time;
				if(maxLength < 1) {
					maxLength = 1;
					pos = start;
				}
			}
		} else {
			hasStarted = false;
		}
	}
	return maxLength;
}

size_t RFIStatistics::GetFrequencyMax(Mask2DCPtr mask, size_t time, size_t &pos)
{
	size_t maxLength = 0, start = 0;
	bool hasStarted = false;
	for(size_t channel=0;channel<mask->Height();++channel)
	{
		if(mask->Value(time, channel))
		{
			if(hasStarted)
			{
				size_t length = channel - start + 1;
				if(maxLength < length) {
					maxLength = length;
					pos = start;
				}
			} else {
				hasStarted = true;
				start = channel;
				if(maxLength < 1) {
					maxLength = 1;
					pos = start;
				}
			}
		} else {
			hasStarted = false;
		}
	}
	return maxLength;
}

void RFIStatistics::FindConnectedSamples(Mask2DPtr mask, std::vector<SamplePosition> &positions, SamplePosition start)
{
	std::deque<SamplePosition> tosearch;
	tosearch.push_back(SamplePosition(start.x, start.y));

	do
	{
		SamplePosition c = tosearch.front();
		tosearch.pop_front();
		if(mask->Value(c.x, c.y))
		{
			mask->SetValue(c.x, c.y, false);
			positions.push_back(SamplePosition(c.x, c.y));
			if(c.x>0)
				tosearch.push_back(SamplePosition(c.x-1, c.y));
			if(c.x<mask->Width()-1)
				tosearch.push_back(SamplePosition(c.x+1, c.y));
			if(c.y>0)
				tosearch.push_back(SamplePosition(c.x, c.y-1));
			if(c.y<mask->Height()-1)
				tosearch.push_back(SamplePosition(c.x, c.y+1));
			if(_eightConnected) {
				if(c.x>0 && c.y>0)
					tosearch.push_back(SamplePosition(c.x-1, c.y-1));
				if(c.x<mask->Width()-1 && c.y<mask->Height()-1)
					tosearch.push_back(SamplePosition(c.x+1, c.y+1));
				if(c.x<mask->Width()-1 && c.y>0)
					tosearch.push_back(SamplePosition(c.x+1, c.y-1));
				if(c.x>0 && c.y<mask->Height()-1)
					tosearch.push_back(SamplePosition(c.x-1, c.y+1));
			}
		}
	}
	while(tosearch.size() != 0);

	SetSamples(mask, positions, 1.0);
}

void RFIStatistics::SetSamples(Mask2DPtr mask, const std::vector<SamplePosition> &positions, bool val)
{
	for(std::vector<SamplePosition>::const_iterator i = positions.begin();
			 i != positions.end(); ++i)
		mask->SetValue(i->x, i->y, val);
}

long double RFIStatistics::getAverageRFISize(std::vector<RFISampleProperties> &sampleProperties)
{
	long double avgSize = 0.0L;
	if(sampleProperties.size() > 0)
	{
		for(std::vector<RFISampleProperties>::const_iterator i = sampleProperties.begin();
				i != sampleProperties.end(); ++i)
			avgSize += i->size;
		avgSize /= (long double) sampleProperties.size();
	}
	return avgSize;
}

long double RFIStatistics::getAverageRFIFlux(std::vector<RFISampleProperties> &sampleProperties)
{
	long double avgFlux = 0.0L;
	if(sampleProperties.size() > 0)
	{
		for(std::vector<RFISampleProperties>::const_iterator i = sampleProperties.begin();
				i != sampleProperties.end(); ++i)
			avgFlux += i->flux;
		avgFlux /= (long double) sampleProperties.size();
	}
	return avgFlux;
}

long double RFIStatistics::getAverageRFIDuration(std::vector<RFISampleProperties> &sampleProperties)
{
	long double avgDuration = 0.0L;
	if(sampleProperties.size() > 0)
	{
		for(std::vector<RFISampleProperties>::const_iterator i = sampleProperties.begin();
				i != sampleProperties.end(); ++i)
			avgDuration += i->duration;
		avgDuration /= (long double) sampleProperties.size();
	}
	return avgDuration;
}

long double RFIStatistics::getAverageRFIFrequencyCoverage(std::vector<RFISampleProperties> &sampleProperties)
{
	long double avgFC = 0.0L;
	if(sampleProperties.size() > 0)
	{
		for(std::vector<RFISampleProperties>::const_iterator i = sampleProperties.begin();
				i != sampleProperties.end(); ++i)
			avgFC += i->frequencyCoverage;
		avgFC /= (long double) sampleProperties.size();
	}
	return avgFC;
}

long double RFIStatistics::AverageMasked(Image2DCPtr image, Mask2DCPtr mask, bool negate)
{
	size_t count = 0;
	long double sum = 0.0;
	for(size_t y=0;y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(mask->Value(x,y) != negate) {
				sum += image->Value(x,y);
				++count;
			}
		}
	}
	return sum / (long double) count;
}

long double RFIStatistics::GetAverageNonRFIFlux() const
{
	return _totalNonRFIFlux / _totalAddedImages;
}

size_t RFIStatistics::InvalidValues(Image2DCPtr image, Mask2DCPtr mask, bool negate) const
{
	size_t count = 0;
	for(size_t y=0;y<image->Height();++y) {
		for(size_t x=0;x<image->Width();++x) {
			if(mask->Value(x,y) != negate) {
				if(!std::isfinite(image->Value(x, y)))
				++count;
			}
		}
	}
	return count;
}

long double RFIStatistics::FitScore(Image2DCPtr image, Image2DCPtr fit, Mask2DCPtr mask)
{
	long double summedError = 0.0L;
	unsigned count = 0;

	for(unsigned y=0;y<image->Height();++y) {
		for(unsigned x=0;x<image->Width();++x) {
			if(!mask->Value(x, y) && std::isfinite(image->Value(x, y)))
			{
				long double error = image->Value(x, y) - fit->Value(x, y);
				summedError += error * error;
				++count;
			} else {
			}
		}
	}
	long double procentData = (long double) count / (image->Width() * image->Height());
	long double averageError = summedError / (image->Width() * image->Height());
	//long double avgError = summedError / (image->Width()*image->Height());
	//return 1.0L/(summedError + avgError * 2.0L * (long double) count);
	return procentData/averageError;
}

num_t RFIStatistics::DataQuality(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned startX, unsigned endX)
{
	unsigned count = 0;
	double sum = 0;
	for(unsigned y=0;y<image->Height();++y)
	{
		for(unsigned x=startX;x<endX;++x)
		{
			if(!mask->Value(x, y) && std::isfinite(image->Value(x, y)) && std::isfinite(model->Value(x,y)))
			{
				num_t noise = fabsl(image->Value(x, y) - model->Value(x, y));
				num_t signal = fabsl(model->Value(x, y));
				if(signal != 0.0)
				{
					if(noise <= 1e-50) noise = 1e-50;
					num_t snr = logl(signal / noise);
					sum += snr;

					++count;
				}
			}
		}
	}
	if(count == 0)
		return 0;
	else
		return sum / (sqrtl(count) * sqrtl((endX-startX) * image->Height()));
}

void RFIStatistics::countTimeLine(Image2DCPtr image, size_t xStart, size_t xLength, size_t y)
{
	num_t flux = 0.0;
	for(size_t x=xStart;x<xStart+xLength;++x)
	{
		flux += image->Value(x, y);
	}
	
	RFISampleProperties properties;
	properties.flux = flux / (double) xLength;
	properties.duration = xLength;
	properties.frequencyCoverage = 1;
	properties.size = xLength;
	_samplesPeeledAlgorithm.push_back(properties);
}

void RFIStatistics::countFrequencyLine(Image2DCPtr image, size_t yStart, size_t yLength, size_t x)
{
	num_t flux = 0.0;
	for(size_t y=yStart;y<yStart+yLength;++y)
	{
		flux += image->Value(x, y);
	}
	
	RFISampleProperties properties;
	properties.flux = flux / (double) yLength;
	properties.duration = 1;
	properties.frequencyCoverage = yLength;
	properties.size = yLength;
	_samplesPeeledAlgorithm.push_back(properties);
}

void RFIStatistics::plot(const std::vector<RFISampleProperties> &samples) const
{
	int histX[100], histY[100], histXY[100][100];
	for(size_t i=0;i<100;++i)
	{
		histX[i] = 0;
		histY[i] = 0;
		for(size_t j=0;j<100;++j)
			histXY[i][j] = 0;
	}
	for(std::vector<RFISampleProperties>::const_iterator i = samples.begin();
			i != samples.end(); ++i)
	{
		RFISampleProperties p = *i;
		if(p.duration < 100)
		{
			++histX[p.duration];
			if(p.frequencyCoverage < 100)
				++histXY[p.frequencyCoverage][p.duration];
		}
		if(p.frequencyCoverage < 100)
			++histY[p.duration];
	}
	Plot
		plotX("histx.pdf"),
		plotY("histy.pdf"),
		plotXY("histxy.pdf");
	plotX.SetXAxisText("Time");
	plotY.SetXAxisText("Channel");
	plotXY.SetXAxisText("Time");
	plotXY.SetYAxisText("Channel");
	plotXY.SetZAxisText("Count");
	plotXY.SetLogScale(false, false, true);
	plotX.StartLine();
	plotY.StartLine();
	plotXY.StartGrid();
	for(size_t i=1;i<100;++i)
	{
		plotX.PushDataPoint(i, histX[i]);
		plotY.PushDataPoint(i, histY[i]);
		for(size_t j=1;j<100;++j)
		{
			plotXY.PushDataPoint(j, i, histXY[i][j]+1);
		}
		plotXY.PushDataBlockEnd();
	}
	plotX.Close();
	plotY.Close();
	plotXY.Close();
	plotX.Show();
	plotY.Show();
	plotXY.Show();
}

num_t RFIStatistics::FrequencySNR(Image2DCPtr image, Image2DCPtr model, Mask2DCPtr mask, unsigned channel)
{
	num_t sum = 0.0;
	size_t count = 0;
	for(unsigned x=0;x<image->Width();++x)
	{
		if(!mask->Value(x, channel))
		{
			num_t noise = fabsl(image->Value(x, channel) - model->Value(x, channel));
			num_t signal = fabsl(model->Value(x, channel));
			if(std::isfinite(signal) && std::isfinite(noise))
			{
				if(noise <= 1e-50) noise = 1e-50;
				num_t snr = logl(signal / noise);
				sum += snr;
	
				++count;
			}
		}
	}
	return expl(sum / count);
}
