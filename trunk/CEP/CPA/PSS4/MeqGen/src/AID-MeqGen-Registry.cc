    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/src/build_aid_maps.pl
    #include <DMI/AtomicID.h>
    #include <DMI/TypeInfo.h>
    #include <DMI/DynamicTypeManager.h>
    #include <DMI/Packer.h>
    
#include "compare.h"
BlockableObject * __construct_Meqcompare (int n) { return n>0 ? new Meq::compare [n] : new Meq::compare; }
  
    int aidRegistry_MeqGen ()
    {
      static int res = 

        AtomicID::registerId(-1467,"Meqcompare")+
        TypeInfoReg::addToRegistry(-1467,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1467,__construct_Meqcompare)+
    0;
    return res;
  }
  
  int __dum_call_registries_for_MeqGen = aidRegistry_MeqGen();

