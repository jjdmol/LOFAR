#include <PL/Query.h>
#include <PL/Query/Expr.h>
#include <Common/Debug.h>
#include <string>
#include <iostream>

using namespace std;
using namespace LOFAR::PL;

int main(int argc, const char* argv[])
{
  Debug::initLevels(argc, argv);

  string      s("string");
  const char* c("const char *");
  Query::Expr e("Query::Expr");

  cout << "QueryObject(s).getSql() = " << QueryObject(s).getSql() << endl;
  cout << "QueryObject(c).getSql() = " << QueryObject(c).getSql() << endl;
  cout << "QueryObject(e).getSql() = " << QueryObject(e).getSql() << endl;

  cout << "QueryObject(\"\").getSql() = " 
       << QueryObject("").getSql() << endl;
  cout << "QueryObject(Query::Expr()) = " 
       << QueryObject(Query::Expr()).getSql() << endl;
}
