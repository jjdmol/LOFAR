// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "volume.h"

Volume::Volume()
{
}

Volume::~Volume()
{
}

Volume::Volume(const Volume & vol)
{
}

Volume & Volume::operator= (const Volume & vol)
{
	volume = vol.getVolume();
	return *this;
}

const int Volume::getVolumeLeft() const
{
	return (volume) & 0xff;
}

const int Volume::getVolumeRight() const
{
	return (volume >> 8) & 0xff;
}

const volume_t Volume::getVolume() const
{
	return (volume & 0xffff);
}

void Volume::setVolumeLeft(const int vol)
{
	volume |= ((vol) & 0xff);
}

void Volume::setVolumeRight(const int vol)
{
	volume |= ((vol << 8) & 0xff);
}

void Volume::setVolume(const volume_t vol)
{
	volume = vol;
}

