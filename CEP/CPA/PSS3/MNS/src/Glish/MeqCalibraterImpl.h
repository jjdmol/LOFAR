

#ifndef MEQ_CALIBRATER_H
#define MEQ_CALIBRATER_H

#include <aips/aips.h>
#include <trial/Tasking/ApplicationObject.h>
#include <aips/Utilities/String.h>

class MeqCalibrater : public ApplicationObject
{

 public:
  // MeqCalibrater specific methods
  MeqCalibrater(const String& msName,
		const String& meqModel,
		const String& skyModel,
		const String& mepDB,
		Int spw);
  ~MeqCalibrater();

  void setTimeIntervalSize(Int secInterval);
  void resetTimeIterator();
  GlishRecord nextTimeInterval();

  void clearSolvableParms();
  void setSolvableParms(const String& parmPatterns, Bool isSolvable);

  void        predict(const String& modelColName);
  GlishRecord solve(); /* returns double fit, updates parameters */

  void saveParms();
  void saveData(const String& dataColName); /* save data, e.g. "MODEL" */
  void saveResidualData(const String& colAName, const String& colBName,
			const String& residualColName); /* save A-B in residualColName */

  GlishRecord getParms(const String& parmPatterns);

  // standard methods
  virtual String         className() const;
  virtual Vector<String> methods() const;
  virtual Vector<String> noTraceMethods() const;

  virtual MethodResult runMethod(uInt which,
				 ParameterSet& inputRecord,
				 Bool runMethod);

 private:

  MeqCalibrater(const MeqCalibrater& other);            // no copy constructor
  MeqCalibrater& operator=(const MeqCalibrater& other); // no assignment operator

  
};

class MeqCalibraterFactory : public ApplicationObjectFactory
{
  virtual MethodResult make(ApplicationObject*& newObject,
			    const String& whichConstructor,
			    ParameterSet& inputRecord,
			    Bool runConstructor);
};

#endif
