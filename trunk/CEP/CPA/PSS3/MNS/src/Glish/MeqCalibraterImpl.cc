
#include <MeqCalibraterImpl.h>

#include <trial/Tasking/Parameter.h>
#include <trial/Tasking/MethodResult.h>
#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>
#include <aips/Arrays/Vector.h>

MeqCalibrater::MeqCalibrater(const String& /*msName*/,
			     const String& /*meqModel*/,
			     const String& /*skyModel*/,
			     const String& /*mepDB*/,
			     Int /*spw*/)
{
}

MeqCalibrater::~MeqCalibrater()
{
}

void MeqCalibrater::setTimeIntervalSize(Int /*secondsInterval*/)
{
}

void MeqCalibrater::resetTimeIterator()
{
}

GlishRecord MeqCalibrater::nextTimeInterval()
{
  GlishRecord rec;

  return rec;
}

void MeqCalibrater::clearSolvableParms()
{
}

void MeqCalibrater::setSolvableParms(const String& /*parmPatters*/, Bool /*isSolvable*/)
{
}

void MeqCalibrater::predict(const String& /*modelColName*/)
{
}

GlishRecord MeqCalibrater::solve()
{
  GlishRecord rec;

  return rec;
}

void MeqCalibrater::saveParms()
{
}

void MeqCalibrater::saveData(const String& /*dataColName*/)
{
}

void MeqCalibrater::saveResidualData(const String& /*colAName*/,
				     const String& /*colBName*/,
				     const String& /*residualColName*/)
{
}

GlishRecord MeqCalibrater::getParms(const String& /*parmPatterns*/)
{
  GlishRecord rec;

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
				      ParameterSet& /*inputRecord*/,
				      Bool /*runMethod*/)
{
  switch (which) 
  {
  case 0:
    {
    }
    break;
  case 1:
    {
    }
    break;
  case 2:
    {
    }
    break;
  case 3:
    {
    }
    break;
  case 4:
    {
    }
    break;
  case 5:
    {
    }
    break;
  case 6:
    {
    }
    break;
  case 7:
    {
    }
    break;
  case 8:
    {
    }
    break;
  case 9:
    {
    }
    break;
  case 10:
    {
    }
    break;
  case 11:
    {
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
