#include <PL/Query/Expr.h>
#include <PL/Query/ColumnExprNode.h>
#include <PL/Collection.h>
#include <iostream>

using namespace LOFAR::PL::Query;
using namespace LOFAR::PL;
using namespace std;

int main()
{
  ColumnExprNode* ab = new ColumnExprNode("A","B");
  ColumnExprNode* ac = new ColumnExprNode("A","C");
  ColumnExprNode* xy = new ColumnExprNode("X","Y");
  ColumnExprNode* xz = new ColumnExprNode("X","Z");

  ab->addConstraint(Expr(new ColumnExprNode("A","B")) && 
                    Expr(new ColumnExprNode("A","C")));
  xy->addConstraint(Expr(new ColumnExprNode("X","Y")) &&
                    Expr(new ColumnExprNode("X","Z")));

  Expr eab(ab);
  Expr eac(ac);
  Expr exy(xy);
  Expr exz(xz);

  cout << (Expr("hello") != Expr("world")) << endl;
  cout << (!(Expr(5.0)+ -Expr(3.1)/4.7) == 2.0) << endl;
  cout << (!(Expr(5.0)+ -Expr(3.1)/4.7) == 2.0 && 
           (Expr("a")<=Expr("b") || Expr("b'\"s")>=Expr("c")) && 
           Expr(new ColumnExprNode("A","itsInt"))==42)
       << endl;

  Collection<Expr> c;
  c.add(2);
  c.add(3);
  c.add(4);

  cout << (Expr(4)+3).in(c) << endl;
  cout << (Expr(3)+4).notIn(c) << endl;
  cout << (Expr(5)-2).between(2,4) << endl;
  cout << (Expr(2)-5).notBetween(2,4) << endl;
  cout << endl;

  cout << exy.like("10*") << endl;
  cout << exy.like("10\\*") << endl;
  cout << exy.like("10\\\\*") << endl;
  cout << exy.like("10?") << endl;
  cout << exy.like("10\\?") << endl;
  cout << exy.like("10\\\\?") << endl;
  cout << exy.like("10a") << endl;
  cout << exy.like("10\\a") << endl;
  cout << exy.like("10\\\\a") << endl;
  cout << exy.like("10*\\") << endl;
  cout << exy.like("10\\*\\") << endl;
  cout << exy.like("10\\\\*\\") << endl;
  cout << exy.like("10_*") << endl;
  cout << exy.like("10\\\\*") << endl;
  cout << exy.like("10_??") << endl;
  cout << exy.like("10%*") << endl;
  cout << exy.like("10\\\\??") << endl;
  cout << exy.like("10*\\\\*") << endl;
  cout << exy.notLike("10*") << endl;
  cout << endl;

  cout << (eab + eac/exy - exz > 20) << endl;
  cout << (eab + eac < 10 || exy - exz > 20) << endl;
  cout << (eab == 10 || exy != 20 && eac < 30 || exz > 40) << endl;

  return 0;
}
