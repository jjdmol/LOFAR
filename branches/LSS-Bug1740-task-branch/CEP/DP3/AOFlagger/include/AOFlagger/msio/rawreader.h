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
#ifndef RAWREADER_H
#define RAWREADER_H

#include <cstring>
#include <fstream>
#include <string>

#include <AOFlagger/util/aologger.h>

class RawReader
{
	public:
		explicit RawReader(const std::string &filename) :
		_blockHeaderSize(512),
		_blockPostfixSize(8),
		_beamCount(1),
		_subbandCount(1),
		_channelsPerSubbandCount(1),
		_samplesPerBlockCount(12208),
		_filename(filename)
		{
			AOLogger::Debug << "RawReader for " << filename << " constructed.\n";
		}
		
		size_t TimestepCount()
		{
			std::ifstream stream(_filename.c_str());
			stream.seekg(0, std::ios_base::end);
			std::streampos fileSize = stream.tellg();
			unsigned long blockSize = BlockSize();
			return (fileSize / blockSize) * _samplesPerBlockCount;
		}
		
		void Read(size_t startIndex, size_t endIndex, float *dest, size_t beamIndex, size_t subbandIndex, size_t channelIndex)
		{
			AOLogger::Debug << "Reading " << startIndex << " to " << endIndex << " (total: " << TimestepCount() << ")\n";
			
			std::ifstream stream(_filename.c_str());
			
			size_t startBlock = startIndex / _samplesPerBlockCount;
			stream.seekg(startBlock * BlockSize(), std::ios_base::beg);
			
			RawBlock block(*this);
			block.read(stream);
			
			size_t startSampleInFirstBlock = startIndex - startBlock * _samplesPerBlockCount;
			
			// TODO check if endIndex > what we copy here
			memcpy(dest, block.SamplePtr(beamIndex, subbandIndex, channelIndex, startSampleInFirstBlock), sizeof(float) * (_samplesPerBlockCount - startSampleInFirstBlock));
			
			size_t samplesCopied = _samplesPerBlockCount - startSampleInFirstBlock;
			
			size_t currentIndex = (startBlock + 1) * _samplesPerBlockCount;
			while(currentIndex < endIndex)
			{
				//AOLogger::Debug << currentIndex << '\n';
				block.read(stream);
				if(currentIndex + _samplesPerBlockCount > endIndex)
				{
					// Whole block won't fit
					memcpy(&dest[samplesCopied], block.SamplePtr(beamIndex, subbandIndex, channelIndex, 0), sizeof(float) * (endIndex - currentIndex));
					samplesCopied += (endIndex - currentIndex);
				}
				else {
					// Block fits
					memcpy(&dest[samplesCopied], block.SamplePtr(beamIndex, subbandIndex, channelIndex, 0), sizeof(float) * _samplesPerBlockCount);
					samplesCopied += _samplesPerBlockCount;
				}
				
				currentIndex += _samplesPerBlockCount;
			}
		}
		
		const std::string &Filename() const { return _filename; }
		
		size_t BlockSize() const
		{
			return _blockHeaderSize + _blockPostfixSize +
				_beamCount * _subbandCount * _channelsPerSubbandCount * _samplesPerBlockCount * sizeof(float);
		}
		
	private:
		const unsigned _blockHeaderSize;
		const unsigned _blockPostfixSize;
		const unsigned _beamCount;
		const unsigned _subbandCount;
		const unsigned _channelsPerSubbandCount;
		const unsigned _samplesPerBlockCount;
		const std::string _filename;
		
		void readBlock();

		class RawBlock
		{
			public:
				RawBlock(RawReader &reader) :
				_reader(reader)
				{
					_header = new unsigned char[reader._blockHeaderSize];
					_data = new float[reader._beamCount * reader._subbandCount * reader._channelsPerSubbandCount * reader._samplesPerBlockCount];
					_postFix = new unsigned char[reader._blockPostfixSize];
				}
				
				~RawBlock()
				{
					delete[] _header;
					delete[] _data;
					delete[] _postFix;
				}
				
				void read(std::istream &stream)
				{
					size_t length = _reader._beamCount * _reader._subbandCount * _reader._channelsPerSubbandCount * _reader._samplesPerBlockCount;
					stream.read(reinterpret_cast<char*>(_header), _reader._blockHeaderSize);
					stream.read(reinterpret_cast<char*>(_data), length * sizeof(float));
					stream.read(reinterpret_cast<char*>(_postFix), _reader._blockPostfixSize);
					
					for(size_t i=0;i<length;++i)
					{
						_data[i] = swapfloat(_data[i]);
					}
				}
				
				float *SamplePtr(size_t beamIndex, size_t subbandIndex, size_t channelIndex, size_t sampleIndex)
				{
					size_t dataIndex =
						sampleIndex +
						channelIndex * _reader._samplesPerBlockCount +
						subbandIndex * _reader._samplesPerBlockCount * _reader._channelsPerSubbandCount +
						beamIndex * _reader._samplesPerBlockCount * _reader._channelsPerSubbandCount * _reader._subbandCount;
					return &_data[dataIndex];
				}
				
				float swapfloat(float input)
				{
					union { char valueChar[4]; float valueFloat; } a, b;
					a.valueFloat = input;
					b.valueChar[3] = a.valueChar[0];
					b.valueChar[2] = a.valueChar[1];
					b.valueChar[1] = a.valueChar[2];
					b.valueChar[0] = a.valueChar[3];
					return b.valueFloat;
				}
				
			private:
				RawReader &_reader;
				unsigned char *_header;
				float *_data;
				unsigned char *_postFix;
		};
};

#endif
