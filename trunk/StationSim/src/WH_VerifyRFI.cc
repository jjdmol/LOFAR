//#  WH_VerifyRFI.cc:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

#include <stdio.h>             // for sprintf
#include <stdlib.h>            // for atof

#include <StationSim/WH_VerifyRFI.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>
#include <aips/Arrays/ArrayMath.h>

#include <aips/OS/RegularFile.h>
#include <aips/IO/RegularFileIO.h>

WH_VerifyRFI::WH_VerifyRFI (const string& name,
			  unsigned int nout, unsigned int nrcu,
			  unsigned int nsub1)
: WorkHolder    (1, nout, name, "WH_VerifyRFI"),
  itsInHolder   ("in", nrcu, nsub1),
  itsOutHolders (0),
  itsNrcu       (nrcu),
  itsNsub1      (nsub1),
  itsVerifyCount(0),
  itsCount      (-11)
{
  if (nout > 0) 
    {
      itsOutHolders = new DH_SampleC* [nout];
    }

  char str[8];
  for (unsigned int i=0; i<nout; i++) 
    {
      sprintf (str, "%d", i);
      itsOutHolders[i] = new DH_SampleC (string("out_") + str, nrcu, nsub1);
    }

  osVerficationOutput.open("/home/alex/rfi_verification.out", ios::out);
}

WH_VerifyRFI::~WH_VerifyRFI()
{
  osVerficationOutput.close();

  for (int i=0; i<getOutputs(); i++) 
    {
      delete itsOutHolders[i];
    }
  delete [] itsOutHolders;
}

WorkHolder* WH_VerifyRFI::construct (const string& name,
				    int ninput, int noutput,
				    const ParamBlock& params)
{
  Assert (ninput == 2);
  return new WH_VerifyRFI (name, noutput,
			  params.getInt ("nrcu", 10),
			  params.getInt ("nsub1", 10));
}

WH_VerifyRFI* WH_VerifyRFI::make (const string& name) const
{
  return new WH_VerifyRFI (name, getOutputs(),
			 itsNrcu, itsNsub1);
}

void WH_VerifyRFI::preprocess()
{
  char number[100];
  int nsamples = 0;
  FILE *fin;
  fin = fopen("/home/alex/gerdes/cancel_matlab.out", "r");

  while (fscanf(fin, "%s", number) != EOF)
    {
      nsamples++;
    }

  MatrixVerifySamples.resize((nsamples / itsNsub1), itsNsub1);
  fseek(fin, 0, SEEK_SET); //move pointer to begin of the file again

  for (int n = 0; n < nsamples / itsNsub1; n++)
    {
      for (int s = 0; s < itsNsub1; ++s)
	{
	  fscanf(fin, "%s", number);
	  MatrixVerifySamples(n, s) = atof(number);
	}
    }

  //  cout << MatrixVerifySamples << endl;
  fclose(fin);
}

void WH_VerifyRFI::process()
{
  DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();
  Matrix<DH_SampleC::BufferType> InputMatrix(IPosition(2, itsNrcu, itsNsub1), bufin, SHARE);
  Matrix<double> PowerMatrix = real(InputMatrix * conj(InputMatrix)); // Calculate the power

  if (itsCount >= 0)
    {
      for (int sub = 0; sub < itsNsub1; sub++)
	{
	  osVerficationOutput << PowerMatrix(0, sub) << ' ';
	}
      osVerficationOutput << endl;
    }
  
//   if (itsCount >= 0)
//   {
//     for (int sub = 0; sub < itsNsub1; sub++) 
//       {
// 	if (sub == 20)
// 	  cout << MatrixVerifySamples(itsCount, sub) << ' ' << PowerMatrix(0, sub) << endl; //DEBUG
	
// 	if (MatrixVerifySamples(itsCount, sub) <= PowerMatrix(0, sub) * 0.9
// 	    || MatrixVerifySamples(itsCount, sub) >= PowerMatrix(0, sub) * 1.1)
// 	  // check within a 1 %
// 	  {
// 	    itsVerifyCount++;
// 	  }
//       }
//     cout << endl;
//   }

   itsCount++;

  //      cout << itsVerifyCount << endl << endl;
}


void WH_VerifyRFI::dump() const
{
  cout << "WH_VerifyRFI " << getName() << " Buffers:" << endl;
  cout << itsVerifyCount << endl;
}


DataHolder* WH_VerifyRFI::getInHolder (int channel)
{
  AssertStr (channel == 0, "input channel too high");
  return &itsInHolder;
}

DH_SampleC* WH_VerifyRFI::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}
