#include "SmartBlock.h"
    
#define paddr(x) printf(#x ": %08x\n",(int)&x)
    
int main ()
{
  DMI::Debug::setLevel(2);

  try {
      
  SmartBlock block1(0x2000);
  SmartBlock * block2 = new SmartBlock(0x2000);
  
  BlockRef ref1( block1,DMI::NON_ANON|DMI::WRITE ),
           ref2( block2,DMI::ANON|DMI::WRITE ),
           ref3( block2,DMI::ANON|DMI::READONLY );
  paddr(ref1);
  paddr(ref2);
  paddr(ref3);
  
  BlockRef ref1a(ref1,DMI::COPYREF|DMI::WRITE);
  paddr(ref1a);
  BlockRef ref1b = ref1a.copy();
  paddr(ref1b);
  BlockRef ref1c(ref1b);
  paddr(ref1c);
  
  ref3.privatize();
  
  }
  catch( Debug::Error err ) 
  {
    cerr<<"\nCaught exception:\n"<<err.what()<<endl;
    return 1;
  }
  return 0;
}
    
    
    
