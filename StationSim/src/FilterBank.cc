/***************************************************************************
                          filterbank.cpp  -  description
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

#include <stdio.h>             // for sprintf
#include <fstream.h>
#include <filterbank.h>

template<class Type>
FilterBank<Type>::FilterBank(string CoefficientFile, int OverlapSamples, int Real)
{
  // Open a stream to the file where the filter coefficients are located
  ifstream CoeffFile(CoefficientFile.c_str(), ifstream::in);

  // Read the filter coefficients and place them in a matrix
  CoeffFile >> itsFilterCoefficients;

   // Initialize the member variables
  itsNumberOfBands = itsFilterCoefficients.rows();
  itsOrder = itsFilterCoefficients.cols();
  itsMatrixPosition = 0;
  itsReArrangedSignal.resize(itsNumberOfBands, itsOrder);
  itsOverlapSamples = OverlapSamples;
  isReal = Real;

  // Initialize FFTW
  fftplancomplex = fftw_create_plan(itsNumberOfBands, FFTW_FORWARD, FFTW_ESTIMATE);
  fftplanreal = rfftwnd_create_plan(1, &itsNumberOfBands, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
}


template<class Type>
FilterBank<Type>::~FilterBank()
{
  fftw_destroy_plan(fftplancomplex);
  rfftwnd_destroy_plan(fftplanreal);
}


template<class Type>
Array<complex<double>, 2> FilterBank<Type>::filter(Array<Type, 1> Input)
{
  Array<complex<double>, 2> FilterBankOutput(itsNumberOfBands, 1);

  if (itsOverlapSamples > 0)
  {
    // Rearrange the input so that it can be easily convolved with the filtercoefficients
    ReArrangeWithOverlap(Input);
  }
  else
  {
    ReArrange(Input);
  }

  // Convolve the rearrangedsignals with the filtercoefficients
  Array<Type, 1> ConvolvedSignal = Convolve();

  // Do a FFT n the convolved signal and return the output
  FilterBankOutput = FFT(ConvolvedSignal);

  if (isReal == 0)
  {
    // Convert the output of the FFT into the appropriate format:
    // 2. Invert the order of the output. [1 2 3 4] -(fftw)-> [4 3 2 1] -> [2 1 4 3]
    //FilterBankOutput.reverseSelf(firstDim);
    Array<complex<double>, 2> Temp(itsNumberOfBands / 2, 1);
    Temp = FilterBankOutput(Range(0, itsNumberOfBands / 2 ), Range(0));
    FilterBankOutput(Range(0, itsNumberOfBands / 2 - 1), Range(0)) = FilterBankOutput(Range(itsNumberOfBands / 2, itsNumberOfBands - 1), Range(0));
    FilterBankOutput(Range(itsNumberOfBands / 2, itsNumberOfBands - 1), Range(0)) = Temp;
  }

  return FilterBankOutput;
}


template<class Type>
void FilterBank<Type>::ReArrange(Array<Type, 1> Input) // This function might be not necessary instead use ReArrange with overlap with overlap = 0
{
  // The input must be number of bands long!
  for (int i = 0; i < itsNumberOfBands; ++i)
  {
    itsReArrangedSignal(i, itsMatrixPosition) = Input(i);
  }

  itsMatrixPosition = ++itsMatrixPosition % itsOrder;
}


template<class Type>
void FilterBank<Type>::ReArrangeWithOverlap(Array<Type, 1> Input)
{
  // The input must be number of bands - number of overlap samples) long
  int PreviousMatrixPosition = (itsMatrixPosition == 0) ? itsOrder - 1 : itsMatrixPosition - 1;

  for (int i = 0; i < itsOverlapSamples; ++i)
  {
    itsReArrangedSignal(i, itsMatrixPosition) = itsReArrangedSignal(itsNumberOfBands - itsOverlapSamples + i,
                                                                    PreviousMatrixPosition);
  }

  for (int i = itsOverlapSamples; i < itsNumberOfBands; ++i)
  {
    itsReArrangedSignal(i, itsMatrixPosition) = Input(i - itsOverlapSamples);
  }
  itsMatrixPosition = ++itsMatrixPosition % itsOrder;
}


template<class Type>
Array<Type, 1> FilterBank<Type>::Convolve()
{
  Array<Type, 1> ConvolvedSignal(itsNumberOfBands);
  ConvolvedSignal(Range(Range::all())) = 0;

  for (int b = 0; b < itsNumberOfBands; ++b)
  {
    // MatrixPosition points to the last addition to the rearranged signal
    int i = 0;
    for (int o = itsMatrixPosition; o < itsOrder; ++o)
    {
      ConvolvedSignal(b) += itsReArrangedSignal(b, o) * itsFilterCoefficients(b, i++);
    }
    for (int o = 0; o < itsMatrixPosition; ++o)
    {
      ConvolvedSignal(b) += itsReArrangedSignal(b, o) * itsFilterCoefficients(b, i++);
    }
  }
  return ConvolvedSignal;
}


template<>
Array<complex<double>, 2> FilterBank< complex<double> >::FFT(Array<complex<double>, 1> ConvolvedSignal)
{
  Array<complex<double>, 2> FilterBankOutput(itsNumberOfBands, 1);

  fftw_one(fftplancomplex, (fftw_complex*)ConvolvedSignal.data(), (fftw_complex*)FilterBankOutput.data());

  FilterBankOutput /= itsNumberOfBands;

  return FilterBankOutput;
}

template<>
Array<complex<double>, 2> FilterBank<double>::FFT(Array<double, 1> ConvolvedSignal)
{
  Array<complex<double>, 2> FilterBankOutput(itsNumberOfBands, 1);

  rfftwnd_one_real_to_complex(fftplanreal, ConvolvedSignal.data(), (fftw_complex*)FilterBankOutput.data());

  FilterBankOutput /= itsNumberOfBands;

  // mirror the first part of FFT to second part
  FilterBankOutput(Range(itsNumberOfBands / 2 + 1, itsNumberOfBands - 1), 0) =
              (FilterBankOutput(Range(1, itsNumberOfBands / 2 - 1), 0)).reverse(0);

  return FilterBankOutput;
}

template class FilterBank< complex<double> >;
template class FilterBank< double >;