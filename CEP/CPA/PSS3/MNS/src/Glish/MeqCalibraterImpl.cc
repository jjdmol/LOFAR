//# MeqCalibraterImpl.cc: Implementation of the MeqCalibrater DO
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <MeqCalibraterImpl.h>

#include <trial/Tasking/Parameter.h>
#include <trial/Tasking/MethodResult.h>
#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>
#include <aips/Arrays/Vector.h>

#include <aips/MeasurementSets/MeasurementSet.h>

MeqCalibrater::MeqCalibrater(const String& msName,
			     const String& meqModel,
			     const String& skyModel,
			     const String& mepDB,
			     const Int     spw)
  :
  // ms(msName),
  timeIteration(10)
{
  cout << "MeqCalibrater constructor (";
  cout << "'" << msName   << "', ";
  cout << "'" << meqModel << "', ";
  cout << "'" << skyModel << "', ";
  cout << "'" << mepDB    << "', ";
  cout << spw << ")" << endl;
}

MeqCalibrater::~MeqCalibrater()
{
  cout << "MeqCalibrater destructor" << endl;
}

void MeqCalibrater::setTimeIntervalSize(Int secondsInterval)
{
  cout << "setTimeIntervalSize = " << secondsInterval << endl;
}

void MeqCalibrater::resetTimeIterator()
{
  cout << "resetTimeIterator" << endl;

  timeIteration = 10;
}

Bool MeqCalibrater::nextTimeInterval()
{
  Bool returnval;

  cout << "nextTimeInterval" << endl;

  if (0 == --timeIteration) returnval = False;
  else returnval = True;

  return returnval;
}

void MeqCalibrater::clearSolvableParms()
{
  cout << "clearSolvableParms" << endl;
}

void MeqCalibrater::setSolvableParms(Vector<String>& parmPatterns, Bool isSolvable)
{
  cout << "setSolvableParms" << endl;

  for (int i=0; i < (int)parmPatterns.nelements(); i++)
  {
    cout << parmPatterns[i] << endl;
  }

  cout << "isSolvable = " << isSolvable << endl;
}

void MeqCalibrater::predict(const String& modelColName)
{
  cout << "predict('" << modelColName << "')" << endl;
}

Double MeqCalibrater::solve()
{
  static Double returnval = 10.0;

  cout << "solve" << endl;

  returnval /= 2.0;

  return returnval;
}

void MeqCalibrater::saveParms()
{
  cout << "saveParms" << endl;
}

void MeqCalibrater::saveData(const String& dataColName)
{
  cout << "saveData('" << dataColName << "')" << endl;
}

void MeqCalibrater::saveResidualData(const String& colAName,
				     const String& colBName,
				     const String& residualColName)
{
  cout << "saveResidualData('" << colAName << "', '" << colBName << "', ";
  cout << "'" << residualColName << "')" << endl;
}

GlishRecord MeqCalibrater::getParms(Vector<String>& parmPatterns)
{
  GlishRecord rec;

  cout << "getParms: " << endl;
  for (int i=0; i < (int)parmPatterns.nelements(); i++)
  {
    cout << parmPatterns[i] << endl;
  }

  return rec;
}

String MeqCalibrater::className() const
{
  return "meqcalibrater";
}

Vector<String> MeqCalibrater::methods() const
{
  Vector<String> method(11);

  method(0)  = "settimeintervalsize";
  method(1)  = "resettimeiterator";
  method(2)  = "nexttimeinterval";
  method(3)  = "clearsolvableparms";
  method(4)  = "setsolvableparms";
  method(5)  = "predict";
  method(6)  = "solve";
  method(7)  = "saveparms";
  method(8)  = "savedata";
  method(9)  = "saveresidualdata";
  method(10) = "getparms";

  return method;
}

Vector<String> MeqCalibrater::noTraceMethods() const
{
  return methods();
}

MethodResult MeqCalibrater::runMethod(uInt which,
				      ParameterSet& inputRecord,
				      Bool runMethod)
{
  switch (which) 
  {
  case 0: // settimeintervalsize
    {
      Parameter<Int> secondsInterval(inputRecord, "secondsinterval",
				     ParameterSet::In);

      if (runMethod) setTimeIntervalSize(secondsInterval());
    }
    break;

  case 1: // resettimeiterator
    {
      if (runMethod) resetTimeIterator();
    }
    break;

  case 2: // nexttimeinterval
    {
      Parameter<Bool> returnval(inputRecord, "returnval",
				ParameterSet::Out);

      if (runMethod) returnval() = nextTimeInterval();
    }
    break;

  case 3: // clearsolvableparms
    {
      if (runMethod) clearSolvableParms();
    }
    break;

  case 4: // setsolvableparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					     ParameterSet::In);
      Parameter<Bool> isSolvable(inputRecord, "issolvable",
				 ParameterSet::In);

      if (runMethod) setSolvableParms(parmPatterns(), isSolvable());
    }
    break;

  case 5: // predict
    {
      Parameter<String> modelColName(inputRecord, "modelcolname",
				     ParameterSet::In);

      if (runMethod) predict(modelColName());
    }
    break;

  case 6: // solve
    {
      Parameter<Double> returnval(inputRecord, "returnval",
				  ParameterSet::Out);

      if (runMethod) returnval() = solve();
    }
    break;

  case 7: // saveparms
    {
      if (runMethod) saveParms();
    }
    break;

  case 8: // savedata
    {
      Parameter<String> dataColName(inputRecord, "datacolname",
				    ParameterSet::In);

      if (runMethod) saveData(dataColName());
    }
    break;

  case 9: // saveresidualdata
    {
      Parameter<String> colAName(inputRecord, "colaname",
				 ParameterSet::In);
      Parameter<String> colBName(inputRecord, "colbname",
				 ParameterSet::In);
      Parameter<String> residualColName(inputRecord, "residualcolname",
					ParameterSet::In);

      if (runMethod) saveResidualData(colAName(),
				      colBName(),
				      residualColName());
    }
    break;

  case 10: // getparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					      ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getParms(parmPatterns());
    }
    break;

  default:
    return error("No such method");
  }

  return ok();
}

MethodResult MeqCalibraterFactory::make(ApplicationObject*& newObject,
					const String& whichConstructor,
					ParameterSet& inputRecord,
					Bool runConstructor)
{
  MethodResult retval;
  newObject = 0;

  if (whichConstructor == "meqcalibrater")
    {
      Parameter<String> msName(inputRecord, "msname", ParameterSet::In);
      Parameter<String> meqModel(inputRecord, "meqmodel", ParameterSet::In);
      Parameter<String> skyModel(inputRecord, "skymodel", ParameterSet::In);
      Parameter<String> mepDB(inputRecord, "mepdb", ParameterSet::In);
      Parameter<Int>    spw(inputRecord, "spw", ParameterSet::In);

      if (runConstructor)
	{
	  newObject = new MeqCalibrater(msName(), meqModel(), skyModel(), mepDB(), spw());
	}
    }
  else
    {
      retval = String("Unknown constructor") + whichConstructor;
    }

  if (retval.ok() && runConstructor && !newObject)
    {
      retval = "Memory allocation error";
    }

  return retval;
}
