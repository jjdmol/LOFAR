#include <DFTServer/mpidft.h>

int main()
{
  double re1=1;
  double im1=2;
  double re2=3;
  double im2=4;
  double re,im;
  for (int i=0; i<100000000; i++) {
    re = mulre(re1,im1,re2,im2);
    im = mulim(re1,im1,re2,im2);

    //    mulreim(re1,im1,re2,im2,re,im);
  }
}
