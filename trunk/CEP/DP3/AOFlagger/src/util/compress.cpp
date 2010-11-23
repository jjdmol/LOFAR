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
#include <AOFlagger/rfi/thresholdtools.h>

#include <AOFlagger/util/compress.h>

void Compress::Initialize()
{
	if(!_isInitialized)
	{
		std::ofstream str("compress.bin");
		for(unsigned i=0;i<_data.ImageCount();++i)
		{
			Write(str, _data.GetImage(i), _data.GetSingleMask());
		}
		_isInitialized = true;
	}
}

void Compress::Deinitialize()
{
	if(_isInitialized)
	{
		//system("rm compress.bin");
	}
}

void Compress::Write(std::ofstream &stream, Image2DCPtr image, Mask2DCPtr mask)
{
	
	num_t
		max = ThresholdTools::MaxValue(image, mask),
		min = ThresholdTools::MinValue(image, mask);
	num_t normalizeFactor = (max - min) * ((2<<23) + ((2<<23)-1));
	for(unsigned y=0;y<image->Height();++y)
	{
		for(unsigned x=0;x<image->Width();++x)
		{
			if(!mask->Value(x, y))
			{
				int32_t value = (int32_t) round((image->Value(x, y) - min) * normalizeFactor);
				stream.write(reinterpret_cast<char*>(&value)+1, 3);
			}
		}
	}
}

unsigned long Compress::RawSize()
{
	Initialize();
	system("cp compress.bin compress.raw");
	return Size("compress.raw");
}

unsigned long Compress::FlacSize()
{
	Initialize();
	system("flac -f -8 --bps=24 --endian=little --channels=1 --sample-rate=128000 --sign=signed  -o compress.flac compress.bin");
	return Size("compress.flac");
}

unsigned long Compress::ZipSize()
{
	Initialize();
	system("zip -9 compress.zip compress.bin");
	return Size("compress.zip");
}

unsigned long Compress::Size(const std::string &file)
{
	system((std::string("du -sh ") + file).c_str());
	system((std::string("rm ") + file).c_str());
	return 0;
}

unsigned long Compress::GzSize()
{
	Initialize();
	system("gzip -9 -c compress.bin > compress.gz");
	return Size("compress.gz");
}

unsigned long Compress::Bz2Size()
{
	Initialize();
	system("bzip2 -9 -c compress.bin > compress.bz2");
	return Size("compress.bz2");
}
