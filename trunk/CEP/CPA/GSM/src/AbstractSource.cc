//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//


#include <sstream>

#include <GSM/AbstractSource.h>

#include <MNS/MeqParmPolc.h>
#include <MNS/MeqMatrix.h>

#include <aips/aips.h>
#include <aips/Measures.h>
#include <aips/Measures/MDirection.h>

#include <aips/Tables/Table.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/Matrix.h>

using namespace GSM;

static const int   MaxSourceTypeName = 11;
static const char* SourceTypeNames[MaxSourceTypeName+1] = 
  {
    "None",
    "Point",
    "Gauss", 
    "Shapelet", 
    "Wavelet", 
    "Pixon", 
    "Image",
    "",
    "",
    "",
    "Compound", 
    "Other"
  };




//================>>>  AbstractSource::AbstractSource  <<<================

AbstractSource::AbstractSource(SourceType          type,
                               double              startTime,
                               double              endTime,
                               double              startFreq,
                               double              endFreq,
                               double              ra,
                               double              dec,
                               unsigned int        catNumber,
                               const std::string&  name)
  : itsStokesNames(4),
    itsCatalogNumber(catNumber),
    itsName(name),
    itsSourceType(type)
{
  itsStokesNames[0] = "I";
  itsStokesNames[1] = "Q";
  itsStokesNames[2] = "U";
  itsStokesNames[3] = "V";
  MeqDomain Domain(startTime, endTime, startFreq, endFreq);
  std::vector<MeqPolc> RaPolcs(1);
  std::vector<MeqPolc> DecPolcs(1);

  MeqMatrix RaMatrix(ra);
  MeqMatrix DecMatrix(dec);

  RaPolcs [0].setCoeff(RaMatrix);
  RaPolcs [0].setDomain(Domain);
  RaPolcs [0].setPerturbation(1e-7, false);
  DecPolcs[0].setCoeff(DecMatrix);
  DecPolcs[0].setDomain(Domain);
  DecPolcs[0].setPerturbation(1e-7, false);
  
  std::string ParmName(createParmName());

  itsRa  = new MeqParmPolc(ParmName+"RA");
  itsRa->setPolcs(RaPolcs);

  itsDec = new MeqParmPolc(ParmName+"DEC");
  itsDec->setPolcs(DecPolcs);
}





//=============>>>  AbstractSource::~AbstractSource  <<<=============

AbstractSource::~AbstractSource()
{
  delete itsRa;
  delete itsDec;
}




//============>>>  AbstractSource::getCatalogNumber  <<<============

unsigned int AbstractSource::getCatalogNumber() const
{
  return itsCatalogNumber;
}



//================>>>  AbstractSource::getName  <<<================

std::string AbstractSource::getName() const
{
  return itsName;
}




//================>>>  AbstractSource::getType  <<<================

GSM::SourceType AbstractSource::getType() const
{
  return itsSourceType;
}





//===============>>>  AbstractSource::getNumberOfParameters  <<<===============

unsigned int AbstractSource::getNumberOfParameters() const
{
  return 2;
}



//===============>>>  AbstractSource::getParameters  <<<===============

unsigned int AbstractSource::getParameters(std::vector<MeqParmPolc* > &parameters)
{
  unsigned int added=0;
  
  parameters[added] = itsRa;
  added++;

  parameters[added] = itsDec;
  added++;
  
  return added;
}




//===============>>>  AbstractSource::getParameters  <<<===============

unsigned int AbstractSource::getParameters(std::vector<const MeqParmPolc* > &parameters) const
{
  unsigned int added=0;
  
  parameters[added] = itsRa;
  added++;

  parameters[added] = itsDec;
  added++;
  
  return added;
}




//===============>>>  AbstractSource::setParameters  <<<===============

unsigned int AbstractSource::setParameters(const std::vector<MeqParmPolc* > &parameters)
{
  unsigned int         read=0;
  std::vector<MeqPolc> Polcs;

  Polcs = parameters[read]->getPolcs();
  itsRa->setPolcs(Polcs);
  read++;

  Polcs = parameters[read]->getPolcs();
  itsDec->setPolcs(Polcs);
  read++;

  return read;
}




//===============>>>  AbstractSource::createParmName  <<<===============

std::string AbstractSource::createParmName() const
{
  std::ostringstream out;

  out << "GSM." << getCatalogNumber() << ".";

  return out.str();
}




//================>>>  AbstractSource::getType  <<<================

MDirection AbstractSource::getPosition(double time,
                                       double frequency,
                                       Stokes stokes) const
{
  double ra  = 0;
  double dec = 0;

  return MDirection(Quantity(ra, "deg"),
                    Quantity(dec,"deg"),
                    MDirection::Ref( MDirection::J2000));
}







//===============>>>  AbstractSource::getPositionExpressions  <<<===============

void AbstractSource::getPositionExpressions(MeqExpr* &ra,
                                            MeqExpr* &dec)
{
  ra  = itsRa;
  dec = itsDec;
}





//===============>>>  AbstractSource::store  <<<===============

MeqDomain AbstractSource::store(Table&      table,
                                unsigned int row) const
{
  std::vector<MeqPolc> Polcs;

  writeNiceAscii(std::cout);
  ScalarColumn<int>         Number  (table, "NUMBER");
  ScalarColumn<String>      Name    (table, "NAME");
  ScalarColumn<int>         Type    (table, "TYPE");
  ArrayColumn<double>       RaParms (table, "RAPARMS");
  ArrayColumn<double>       DecParms(table, "DECPARMS");
  ArrayColumn<double>       TDomain (table, "TDOMAIN");
  ArrayColumn<double>       FDomain (table, "FDOMAIN");

  Number.put(row, itsCatalogNumber);
  Name.put  (row, itsName);
  Type.put  (row, int(itsSourceType));

  Polcs = itsRa->getPolcs();
  RaParms.put (row, Polcs[0].getCoeff().getDoubleMatrix());

  Polcs = itsDec->getPolcs();
  DecParms.put(row, Polcs[0].getCoeff().getDoubleMatrix());

  MeqDomain Domain = Polcs[0].domain();

  Vector<double> TVector(2);
  TVector(0) = Domain.startX();
  TVector(1) = Domain.endX();
  TDomain.put(row, TVector);
  
  Vector<double> FVector(2);
  FVector(0) = Domain.startY();
  FVector(1) = Domain.endY();
  FDomain.put(row, FVector);

  return Domain;
}




//===============>>>  AbstractSource::load  <<<===============

MeqDomain AbstractSource::load(const Table& table,
                          unsigned int row)
{
  ROScalarColumn<int>    Number  (table, "NUMBER");
  ROScalarColumn<String> Name    (table, "NAME");
  ROScalarColumn<int>    Type    (table, "TYPE");
  ROArrayColumn<double>  RaParms (table, "RAPARMS");
  ROArrayColumn<double>  DecParms(table, "DECPARMS");
  ROArrayColumn<double>  TDomain (table, "TDOMAIN");
  ROArrayColumn<double>  FDomain (table, "FDOMAIN");


  Vector<double> TVector;
  Vector<double> FVector;

  TDomain.get(row, TVector);
  FDomain.get(row, FVector);

  MeqDomain Domain(TVector(0), TVector(1),
                   FVector(0), FVector(1));

  std::vector<MeqPolc> RaPolcs(1);
  std::vector<MeqPolc> DecPolcs(1);

  Matrix<double> coef(1,1);
  RaParms.get (row, coef, true);
  RaPolcs[0].setDomain(Domain);
  RaPolcs[0].setCoeff(coef);
  itsRa->setPolcs(RaPolcs);

  DecParms.get(row, coef, true);
  DecPolcs[0].setCoeff(coef);
  DecPolcs[0].setDomain(Domain);
  itsDec->setPolcs(DecPolcs);

  
  int temp;
  Number.get(row, temp);
  itsCatalogNumber = (unsigned int)temp;

  String    NameStr;
  Name.get  (row, NameStr);
  itsName = NameStr;

  Type.get  (row, temp);
  itsSourceType = SourceType(temp);
  
  return Domain;
}




//===============>>>  AbstractSource::writeNiceAscii  <<<===============

std::ostream& AbstractSource::writeNiceAscii(std::ostream& out) const
{
  using std::endl;

  out << "================================" << endl;
  out << "Cat : ";
  out.width(10);
  out << itsCatalogNumber << ", \"" << itsName << "\"" << endl;

  out << "Type: " << (int(itsSourceType) > MaxSourceTypeName? "Unknown" : SourceTypeNames[itsSourceType]) << endl;

  out << "Parms:" << endl;

  std::vector<const MeqParmPolc*> ParmList(getNumberOfParameters());
  getParameters(ParmList);
  out << "Number of parameters: " << getNumberOfParameters() << endl;

  for(unsigned int i = 0; i < ParmList.size(); i++) {
    out << i <<"/" << ParmList.size()<<": " <<ParmList[i] << ": " << std::flush;
    std::string Name(ParmList[i]->getName());
    out << Name << endl << std::flush;
  }

  out << "itsRa: " << itsRa << endl;
  std::vector<MeqPolc> Polcs = itsRa->getPolcs();

  MeqMatrix mat(Polcs[0].getCoeff());
  
  Matrix<double> coef(mat.getDoubleMatrix());
  out << "Ra  : " << endl;
  for(unsigned int t = 0; t < coef.nrow(); t++) {
    for(unsigned int f = 0; f < coef.ncolumn(); f++) {
      out.width(10);
      out << coef(t,f);
    }
    out << endl;
  }
  out << "Dec : " << endl;
  Polcs = itsDec->getPolcs();
  coef = Polcs[0].getCoeff().getDoubleMatrix();
  for(unsigned int t = 0; t < coef.nrow(); t++) {
    for(unsigned int f = 0; f < coef.ncolumn(); f++) {
      out.width(10);
      out << coef(t,f);
    }
    out << endl;
  }
  
  return out;
}
