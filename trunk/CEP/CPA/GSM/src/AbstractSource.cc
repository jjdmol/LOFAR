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


#include <GSM/AbstractSource.h>

#include <MNS/MesParmPolc.h>
#include <MNS/MnsMatrix.h>

#include <aips/aips.h>
#include <aips/Measures.h>
#include <aips/Measures/MDirection.h>

#include <aips/Tables/Table.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
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
                               double              ra,
                               double              dec,
                               unsigned int        catNumber,
                               const std::string&  name)
  : itsCatalogNumber(catNumber),
    itsName(name),
    itsSourceType(type)
{
  MnsMatrix RaMatrix(ra);
  MnsMatrix DecMatrix(dec);

  itsRA  = new MesParmPolc("Ra", RaMatrix);
  itsDec = new MesParmPolc("Dec", DecMatrix);
}





//=============>>>  AbstractSource::~AbstractSource  <<<=============

AbstractSource::~AbstractSource()
{
  delete itsRA;
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

unsigned int AbstractSource::getParameters(std::vector<MesParm* > &parameters)
{
  unsigned int added=0;
  
  parameters[added] = itsRA;
  added++;

  parameters[added] = itsDec;
  added++;
  
  return added;
}




//===============>>>  AbstractSource::setParameters  <<<===============

unsigned int AbstractSource::setParameters(const std::vector<MesParm* > &parameters)
{
  unsigned int read=0;
  
  *itsRA = *parameters[read];
  read++;

  *itsDec = *parameters[read];
  read++;
  
  return read;
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

void AbstractSource::getPositionExpressions(MesExpr* ra,
                                            MesExpr* dec)
{
  ra  = itsRA;
  dec = itsDec;
}





//===============>>>  AbstractSource::store  <<<===============

void AbstractSource::store(Table&      table,
                           unsigned int row) const
{
  writeNiceAscii(std::cout);
  ScalarColumn<int>         Number  (table, "NUMBER");
  ScalarColumn<String>      Name    (table, "NAME");
  ScalarColumn<int>         Type    (table, "TYPE");
  ArrayColumn<double>       RAParms (table, "RAPARMS");
  ArrayColumn<double>       DecParms(table, "DECPARMS");

  Number.put(row, itsCatalogNumber);
  Name.put  (row, itsName);
  Type.put  (row, int(itsSourceType));
  RAParms.put (row, itsRA->getCoeff().getDoubleMatrix());
  DecParms.put(row, itsDec->getCoeff().getDoubleMatrix());
}




//===============>>>  AbstractSource::load  <<<===============

void AbstractSource::load(const Table& table,
                          unsigned int row)
{
  ROScalarColumn<int>    Number  (table, "NUMBER");
  ROScalarColumn<String> Name    (table, "NAME");
  ROScalarColumn<int>    Type    (table, "TYPE");
  ROArrayColumn<double>  RAParms (table, "RAPARMS");
  ROArrayColumn<double>  DecParms(table, "DECPARMS");

  int temp;
  Number.get(row, temp);
  itsCatalogNumber = (unsigned int)temp;

  String    NameStr;
  Name.get  (row, NameStr);
  itsName = NameStr;

  Type.get  (row, temp);
  itsSourceType = SourceType(temp);

  Matrix<double> coef(1,1);
  RAParms.get (row, coef, true);
  itsRA->setCoeff(coef);

  DecParms.get(row, coef, true);
  itsDec->setCoeff(coef);
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

  out << "itsRA: " << itsRA << endl;
  MnsMatrix mat(itsRA->getCoeff());
  
  Matrix<double> coef(mat.getDoubleMatrix());
  out << "RA  : " << endl;
  for(unsigned int t = 0; t < coef.nrow(); t++) {
    for(unsigned int f = 0; f < coef.ncolumn(); f++) {
      out.width(10);
      out << coef(t,f);
    }
    out << endl;
  }
  out << "Dec : " << endl;
  coef = itsDec->getCoeff().getDoubleMatrix();
  for(unsigned int t = 0; t < coef.nrow(); t++) {
    for(unsigned int f = 0; f < coef.ncolumn(); f++) {
      out.width(10);
      out << coef(t,f);
    }
    out << endl;
  }
  
  return out;
}
