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


#include <GSM/PointSource.h>

#include <MNS/MeqParmPolc.h>
#include <MNS/MeqMatrix.h>

#include <aips/aips.h>
#include <aips/Tables/ScalarColumn.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Arrays/Matrix.h>

using namespace GSM;


//====================>>>  PointSource::PointSource  <<<====================

PointSource::PointSource(double                     startTime,
                         double                     endTime,
                         double                     startFreq,
                         double                     endFreq,
                         double                     ra,
                         double                     dec,
                         unsigned int               catNumber,
                         const std::string&         name,
                         const std::vector<double>& flux)
  : AbstractSource(POINT, startTime, endTime, startFreq, endFreq,
                   ra, dec, catNumber, name)
{
  std::string ParmName(createParmName());
  MeqDomain   Domain(startTime, endTime, startFreq, endFreq);
  std::vector<MeqPolc> Polcs(1);

  for(unsigned int i = 0; i < NUMBER_OF_POLARIZATIONS;i++) {
    MeqMatrix  matrix(flux[i]);
    Polcs[0].setCoeff(matrix);
    Polcs[0].setDomain(Domain);
    itsFlux[i] =  new MeqParmPolc(ParmName + itsStokesNames[i]);
    dynamic_cast<MeqParmPolc*>(itsFlux[i])->setPolcs(Polcs);
  }
}





//====================>>>  PointSource::~PointSource  <<<====================

PointSource::~PointSource()
{
  for(unsigned int i = 0; i < NUMBER_OF_POLARIZATIONS;i++) {
    delete itsFlux[i];
  }
}





//====================>>>  PointSource:load:  <<<====================

MeqDomain PointSource::load(const Table& table,
                            unsigned int row)
{
  MeqDomain            Domain(AbstractSource::load(table, row));
  std::vector<MeqPolc> Polcs(1);
 
  ROArrayColumn<double> IParms(table, "IPARMS");
  ROArrayColumn<double> QParms(table, "QPARMS");
  ROArrayColumn<double> UParms(table, "UPARMS");
  ROArrayColumn<double> VParms(table, "VPARMS");
  
  Matrix<double> coef(1,1);
  IParms.get(row, coef);
  Polcs[0].setCoeff(coef);
  Polcs[0].setDomain(Domain);
  dynamic_cast<MeqParmPolc*>(itsFlux[I])->setPolcs(Polcs);

  QParms.get(row, coef);
  Polcs[0].setCoeff(coef);
  Polcs[0].setDomain(Domain);
  dynamic_cast<MeqParmPolc*>(itsFlux[Q])->setPolcs(Polcs);

  UParms.get(row, coef);
  Polcs[0].setCoeff(coef);
  Polcs[0].setDomain(Domain);
  dynamic_cast<MeqParmPolc*>(itsFlux[U])->setPolcs(Polcs);

  VParms.get(row, coef);
  Polcs[0].setCoeff(coef);
  Polcs[0].setDomain(Domain);
  dynamic_cast<MeqParmPolc*>(itsFlux[V])->setPolcs(Polcs);
  
  return Domain;
}




//====================>>>  PointSource::store  <<<====================

MeqDomain PointSource::store(Table&       table,
                        unsigned int row)
{
  MeqDomain            Domain(AbstractSource::store(table, row));
  std::vector<MeqPolc> Polcs(1);
  
  ArrayColumn<double> IParms(table, "IPARMS");
  ArrayColumn<double> QParms(table, "QPARMS");
  ArrayColumn<double> UParms(table, "UPARMS");
  ArrayColumn<double> VParms(table, "VPARMS");
  
  Polcs = dynamic_cast<MeqParmPolc*>(itsFlux[I])->getPolcs();
  IParms.put(row, Polcs[0].getCoeff().getDoubleMatrix());

  Polcs = dynamic_cast<MeqParmPolc*>(itsFlux[Q])->getPolcs();
  QParms.put(row, Polcs[0].getCoeff().getDoubleMatrix());

  Polcs = dynamic_cast<MeqParmPolc*>(itsFlux[U])->getPolcs();
  UParms.put(row, Polcs[0].getCoeff().getDoubleMatrix());

  Polcs = dynamic_cast<MeqParmPolc*>(itsFlux[V])->getPolcs();
  VParms.put(row, Polcs[0].getCoeff().getDoubleMatrix());

  return Domain;
}





//====================>>>  PointSource::getParameters  <<<====================

unsigned int PointSource::getParameters(std::vector<MeqParm*> &parameters)
{

  unsigned int i = AbstractSource::getParameters(parameters);
  
  for(unsigned int j = 0; j < NUMBER_OF_POLARIZATIONS; j++) {
    parameters[i+j] = itsFlux[j];
    i++;
  }
  
  return i;
}





//====================>>>  PointSource::getParameters  <<<====================

unsigned int PointSource::getParameters(std::vector<const MeqParm*> &parameters) const
{

  unsigned int i = AbstractSource::getParameters(parameters);
  
  for(unsigned int j = 0; j < NUMBER_OF_POLARIZATIONS; j++) {
    parameters[i+j] = itsFlux[j];
  }
  i += NUMBER_OF_POLARIZATIONS;
  return i;
}





//====================>>>  PointSource::setParameters  <<<====================

unsigned int PointSource::setParameters(const std::vector<MeqParm*> &parameters)
{
  unsigned int         i = AbstractSource::setParameters(parameters);
  std::vector<MeqPolc> Polcs;
  
  for(unsigned int j = 0; j < NUMBER_OF_POLARIZATIONS; j++) {
    Polcs = dynamic_cast<MeqParmPolc*>(parameters[i+j])->getPolcs();
    dynamic_cast<MeqParmPolc*>(itsFlux[j])->setPolcs(Polcs);
  }
  i += NUMBER_OF_POLARIZATIONS;
  return i;
}





//===============>>>  PointSource::getNumberOfParameters  <<<===============

unsigned int PointSource::getNumberOfParameters() const
{
  return AbstractSource::getNumberOfParameters()+ NUMBER_OF_POLARIZATIONS;
}





//===============>>>  PointSource::getFluxExpressions  <<<===============

void PointSource::getFluxExpressions(std::vector<MeqExpr*> &flux)
{
  for(unsigned int i = 0; i < NUMBER_OF_POLARIZATIONS; i++) {
    flux[i] = itsFlux[i];
  }
}
