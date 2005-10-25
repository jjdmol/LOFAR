%module meqcalibrater

class MeqCalibrater
{
 public:
 
  /*
   * initialise parameters
   * read ms
   * read skymodel
   * read mepDB
   * build MeqTree for skyModel
   *
   */
  MeqCalibrater(const String& ms,
		const String& meqModel = "WSRT",
		const String& skyModel = "SKY",
		const String& mepDB    = "MEP",
		Int spw = 0);
  
  void setTimeIntervalSize(Int secondsInterval); /* in seconds, 0 means all data */
  void resetTimeIterator();
  Bool nextTimeInterval();

  void clearSolvableParms(); /* clear solvable flag on all parms */
  void setSolvableParms(const String& parmPatterns, Bool isSolvable);

  void   predict(const String& modelColName);
  double solve(); /* returns fit, updates parameters */
  
  void saveParms();
  void saveData(const String& dataColName); /* save data, e.g. "MODEL" */
  void saveResidualData(const String& colAName, const String7 colBName,
			const String& residualColName); /* save A-B in residualColName */

  /* returns vector of parm records (name, value) pairs */
  GlishRecord getParms(const String& parmPatterns);
};
