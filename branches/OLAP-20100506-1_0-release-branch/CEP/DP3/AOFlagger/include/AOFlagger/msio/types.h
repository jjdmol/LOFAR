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

#ifndef MSIO_TYPES
#define MSIO_TYPES

class AntennaInfo;
class BandInfo;
class FieldInfo;
class TimeFrequencyData;
class TimeFrequencyImager;
class FitsFile;

typedef double num_t;

enum DataKind { ObservedData, CorrectedData, ResidualData, ModelData, WeightData };

enum PolarisationType { SinglePolarisation, DipolePolarisation, AutoDipolePolarisation, CrossDipolePolarisation, StokesIPolarisation, XXPolarisation, XYPolarisation, YXPolarisation, YYPolarisation };


#define sqrtn(X) sqrt(X)
#define expn(X) exp(X)
#define logn(X) log(X)
#define sinn(X) sin(X)
#define cosn(X) cos(X)
#define tann(X) tan(X)

#endif // MSIO_TYPES
