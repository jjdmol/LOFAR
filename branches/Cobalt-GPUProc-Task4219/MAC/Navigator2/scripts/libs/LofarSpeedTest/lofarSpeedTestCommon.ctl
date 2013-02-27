const string STATION1 = "CS001:";
const string STATION2 = "CS010:";
const string STATION3 = "CS008:";
const string STATION4 = "CS016:";


void startScripts(dyn_string dbList,string testName)
{
  dyn_errClass err;

  for (int i=1; i<= dynlen(dbList);i++) {
    dpSetWait(dbList[i]+"lofarSpeedTest.result.testName:_original.._value", testName);
    err = getLastError();
    if (dynlen(err) > 0)
      errorDialog(err);
  }
}

