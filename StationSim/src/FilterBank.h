/***************************************************************************
                          filterbank.h  -  description
                             -------------------
    begin                : Tue Oct 22 2002
    copyright            : (C) 2002 by Alex Gerdes
    email                : gerdes@astron.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATIONSIM_WH_FILTERBANK_H
#define STATIONSIM_WH_FILTERBANK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <blitz/array.h>
#ifdef BZ_NAMESPACES
using namespace blitz;
#endif

#include <complex.h>
#include <fftw.h>
#include <rfftw.h>

template<class Type>
class FilterBank
{
public:
  FilterBank(string CoefficientFile, int OverlapSamples = 0, int Real = 0);
  ~FilterBank();

  Array<complex<double>, 2> filter(Array<Type, 1> Input);

  int getItsNumberOfBands() {return itsNumberOfBands;}
  int getItsOrder() {return itsOrder;}

//private:
  int itsNumberOfBands;
  int itsOrder;
  int itsMatrixPosition;
  int itsOverlapSamples;
  int isReal;
  rfftwnd_plan fftplanreal;
  fftw_plan fftplancomplex;

  Array<Type, 2> itsFilterCoefficients;
  Array<Type, 2> itsReArrangedSignal;

  void ReArrange(Array<Type, 1> Input);
  void ReArrangeWithOverlap(Array<Type, 1> Input);
  Array<Type, 1> Convolve();
  Array<complex<double>, 2> FFT(Array<Type, 1> ConvolvedSignal);
};

#endif
