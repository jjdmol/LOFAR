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
#include <aips/Glish/GlishArray.h>
#include <aips/Glish/GlishValue.h>
#include <aips/Arrays/Vector.h>

#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/Utilities/Regex.h>
#include <aips/Exceptions/Error.h>

#include <MNS/MeqParm.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqRequest.h>
#include <aips/Arrays/Matrix.h>

MeqCalibrater::MeqCalibrater(const String& msName,
			     const String& meqModel,
			     const String& skyModel,
			     const Int     spw)
  :
  itsMs(msName),
  itsMEP(meqModel + ".MEP"),
  itsTimeIteration(10),
  itsFitValue(10.0)
{
  cout << "MeqCalibrater constructor (";
  cout << "'" << msName   << "', ";
  cout << "'" << meqModel << "', ";
  cout << "'" << skyModel << "', ";
  cout << spw << ")" << endl;

  //
  // create the MeqTree corresponding to the meqModel name
  // 

  // tree = ...

#if 1
  // 
  // Initialize some parmpolcs for testing purposes.
  //
  MeqStoredParmPolc* p0 = new MeqStoredParmPolc("a.c.d.e", &itsMEP);
  MeqStoredParmPolc* p1 = new MeqStoredParmPolc("a.b.c.d.e", &itsMEP);
  MeqStoredParmPolc* p2 = new MeqStoredParmPolc("f.g.h.i.j", &itsMEP);
  MeqStoredParmPolc* p3 = new MeqStoredParmPolc("f.g.h.i.j.0", &itsMEP);
  MeqStoredParmPolc* p4 = new MeqStoredParmPolc("f.g.h.i.j.1", &itsMEP);
  MeqStoredParmPolc* p5 = new MeqStoredParmPolc("f.g.h.i.j.2", &itsMEP);
  MeqStoredParmPolc* p6 = new MeqStoredParmPolc("f.g.h.i.j.3", &itsMEP);
  MeqStoredParmPolc* p7 = new MeqStoredParmPolc("f.g.h.i.j.4", &itsMEP);
  MeqStoredParmPolc* p8 = new MeqStoredParmPolc("f.g.h.i.j.5", &itsMEP);
  MeqStoredParmPolc* p9 = new MeqStoredParmPolc("f.g.h.i.j.6", &itsMEP);
  MeqStoredParmPolc* p10 = new MeqStoredParmPolc("f.g.h.i.j.7", &itsMEP);
  MeqStoredParmPolc* p11 = new MeqStoredParmPolc("f.g.h.i.j.8", &itsMEP);
  MeqStoredParmPolc* p12 = new MeqStoredParmPolc("f.g.h.i.j.9", &itsMEP);

  // keep compiler happy
  p0  = p0 ;
  p1  = p1 ;
  p2  = p2 ;
  p3  = p3 ;
  p4  = p4 ;
  p5  = p5 ;
  p6  = p6 ;
  p7  = p7 ;
  p8  = p8 ;
  p9  = p9 ;
  p10 = p10;
  p11 = p11;
  p12 = p12;
#endif
}

void MeqCalibrater::initParms(MeqDomain& /* theDomain */)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
#if 0
      (*iter)->initDomain(theDomain, 0); // what should the spidIndex be?
#endif
    }
  }
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

  //
  // set the domain to the beginning
  //
  initParms(itsDomain);

#if 1
  // Dummy implementation
  itsTimeIteration = 10;
  itsFitValue = 10.0;
#endif
}

Bool MeqCalibrater::nextTimeInterval()
{
  Bool returnval;

  //
  // Go to next time domain, update domain
  //

  // domain = ...

#if 1
  // Dummy implementation
  cout << "nextTimeInterval" << endl;

  if (0 == --itsTimeIteration) returnval = False;
  else returnval = True;

  itsFitValue = 10.0;
#endif

  return returnval;
}

void MeqCalibrater::clearSolvableParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "clearSolvableParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      cout << "clearSolvable: " << (*iter)->getName() << endl;
      (*iter)->setSolvable(false);
    }
  }
}

void MeqCalibrater::setSolvableParms(Vector<String>& parmPatterns, Bool isSolvable)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "setSolvableParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    for (int i=0; i < (int)parmPatterns.nelements(); i++)
    {
      Regex pattern(Regex::fromPattern(parmPatterns[i]));

      if (*iter)
      {
	if (String((*iter)->getName()).matches(pattern))
	{
	  cout << "setSolvable: " << (*iter)->getName() << endl;
	  (*iter)->setSolvable(isSolvable);
	}
      }
    }
  }

  cout << "isSolvable = " << isSolvable << endl;
}

void MeqCalibrater::predict(const String& modelColName)
{
  cout << "predict('" << modelColName << "')" << endl;


}

Double MeqCalibrater::solve()
{
  cout << "solve" << endl;

  // invoke solver for the current domain

  // update the paramters

#if 1
  // Dummy implementation
  itsFitValue /= 2.0;
#endif

  return itsFitValue;
}

void MeqCalibrater::saveParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "saveParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      cout << "saveParm: " << (*iter)->getName() << endl;
      (*iter)->save();
    }
  }
}

void MeqCalibrater::saveData(const String& dataColName)
{
  cout << "saveData('" << dataColName << "')" << endl;
}

void MeqCalibrater::saveResidualData(const String& colAName,
				     const String& colBName,
				     const String& residualColName)
{
  if (colAName == residualColName)
  {
    throw(AipsError("residualcolname can not be the same as colaname"));
  }
  if (colBName == residualColName)
  {
    throw(AipsError("residualcolname can not be the same as colbname"));
  }

  cout << "saveResidualData('" << colAName << "', '" << colBName << "', ";
  cout << "'" << residualColName << "')" << endl;
}

void addParm(MeqParm* parm, GlishRecord* rec)
{
  GlishRecord parmRec;

#if 0

  //
  // In order to fill in the result for the current domain
  // we need some specification of the current domain.
  // The domain should be set in the nextTimeInterval method.
  //

  MeqRequest mr(MeqDomain(0,1,0,1), 1, 1);
  MeqMatrix mm = parm->getResult(mr).getValue();	    
  if (mm.isDouble()) rec->add("result", mm.getDoubleMatrix());
  else               rec->add("result", mm.getDComplexMatrix());
#endif

  parmRec.add("parmid", Int(parm->getParmId()));
  
  rec->add(parm->getName(), parmRec);
}

GlishRecord MeqCalibrater::getParms(Vector<String>& parmPatterns,
				    Vector<String>& excludePatterns)
{
  GlishRecord rec;
  bool parmDone = false;
  vector<MeqParm*> parmVector;

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "getParms: " << endl;


  //
  // Find all parms matching the parmPatterns
  //
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    parmDone = false;

    for (int i=0; i < (int)parmPatterns.nelements(); i++)
    {
      Regex pattern(Regex::fromPattern(parmPatterns[i]));

      if (*iter && !parmDone)
      {
	if (String((*iter)->getName()).matches(pattern))
	{
	  parmDone = true;
	  parmVector.push_back(*iter);
	}
      }
    }
  }

  //
  // Make record of parms to return, but exclude parms
  // matching the excludePatterns
  //
  for (vector<MeqParm*>::iterator iter = parmVector.begin();
       iter != parmVector.end();
       iter++)
  {
    if (0 == excludePatterns.nelements())
    {
      if (*iter) addParm((*iter), &rec);
    }
    else
    {
      for (int i=0; i < (int)excludePatterns.nelements(); i++)
      {
	Regex pattern(Regex::fromPattern(excludePatterns[i]));
	
	if (*iter)
	{
	  if (! String((*iter)->getName()).matches(pattern))
	  {
	    addParm((*iter), &rec);
	  }
	}
      }
    }
  }

  return rec;
}

GlishRecord MeqCalibrater::getSolveDomain()
{
  GlishRecord rec;

  rec.add("offsetx", itsDomain.offsetX());
  rec.add("scalex",  itsDomain.scaleX());
  rec.add("offsety", itsDomain.offsetY());
  rec.add("scaley",  itsDomain.scaleY());

  return rec;
}

String MeqCalibrater::className() const
{
  return "meqcalibrater";
}

Vector<String> MeqCalibrater::methods() const
{
  Vector<String> method(12);

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
  method(11) = "getsolvedomain";

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
      Parameter<Vector<String> > excludePatterns(inputRecord, "excludepatterns",
						 ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getParms(parmPatterns(), excludePatterns());
    }
    break;

  case 11: // getsolvedomain
    {
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getSolveDomain();
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
      Parameter<String>   msName(inputRecord, "msname",   ParameterSet::In);
      Parameter<String> meqModel(inputRecord, "meqmodel", ParameterSet::In);
      Parameter<String> skyModel(inputRecord, "skymodel", ParameterSet::In);
      Parameter<Int>         spw(inputRecord, "spw",      ParameterSet::In);

      if (runConstructor)
	{
	  newObject = new MeqCalibrater(msName(), meqModel(),
					skyModel(), spw());
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
