#define HAVE_BLITZ
#include "Common/Lorrays.h"
#include "Common/Debug.h"


int main ()
{
  LoVec_double vec1(3);
  vec1 = 1,2,3;
  cout<<"vec1: "<<vec1<<endl;
  
  LoMat_double mat1(3,3);
  mat1 = 1,2,3,
         4,5,6,
         7,8,9;
  cout<<"mat1: "<<mat1<<endl;
  
  LoMat_double mat2(3,3);
  mat2  = mat1 + mat1;
  cout<<"mat2: "<<mat2<<endl;
  
  LoMatShape shape2(5,5);
  
  LoMat_double mat3(shape2);
  cout<<"mat3: "<<mat3<<endl;
  
  LoMatPos pos(3,3);
  cout<<"mat3(3,3): "<<mat3(pos)<<endl;
}
