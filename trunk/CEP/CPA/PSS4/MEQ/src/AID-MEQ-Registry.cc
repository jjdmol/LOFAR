    // This file is generated automatically -- do not edit
    // Generated by /home/gvd/sim/LOFAR/autoconf_share/../DMI/src/build_aid_maps.pl
    #include <DMI/AtomicID.h>
    #include <DMI/TypeInfo.h>
    #include <DMI/DynamicTypeManager.h>
    #include <DMI/Packer.h>
    
#include "Add.h"
BlockableObject * __construct_MEQAdd (int n) { return n>0 ? new MEQ::Add [n] : new MEQ::Add; }
#include "Conj.h"
BlockableObject * __construct_MEQConj (int n) { return n>0 ? new MEQ::Conj [n] : new MEQ::Conj; }
#include "Cos.h"
BlockableObject * __construct_MEQCos (int n) { return n>0 ? new MEQ::Cos [n] : new MEQ::Cos; }
#include "Divide.h"
BlockableObject * __construct_MEQDivide (int n) { return n>0 ? new MEQ::Divide [n] : new MEQ::Divide; }
#include "Exp.h"
BlockableObject * __construct_MEQExp (int n) { return n>0 ? new MEQ::Exp [n] : new MEQ::Exp; }
#include "Function.h"
BlockableObject * __construct_MEQFunction (int n) { return n>0 ? new MEQ::Function [n] : new MEQ::Function; }
#include "Multiply.h"
BlockableObject * __construct_MEQMultiply (int n) { return n>0 ? new MEQ::Multiply [n] : new MEQ::Multiply; }
#include "Node.h"
BlockableObject * __construct_MEQNode (int n) { return n>0 ? new MEQ::Node [n] : new MEQ::Node; }
#include "ParmPolcStored.h"
BlockableObject * __construct_MEQParmPolcStored (int n) { return n>0 ? new MEQ::ParmPolcStored [n] : new MEQ::ParmPolcStored; }
#include "Sin.h"
BlockableObject * __construct_MEQSin (int n) { return n>0 ? new MEQ::Sin [n] : new MEQ::Sin; }
#include "Sqr.h"
BlockableObject * __construct_MEQSqr (int n) { return n>0 ? new MEQ::Sqr [n] : new MEQ::Sqr; }
#include "Sqrt.h"
BlockableObject * __construct_MEQSqrt (int n) { return n>0 ? new MEQ::Sqrt [n] : new MEQ::Sqrt; }
#include "Subtract.h"
BlockableObject * __construct_MEQSubtract (int n) { return n>0 ? new MEQ::Subtract [n] : new MEQ::Subtract; }
#include "ToComplex.h"
BlockableObject * __construct_MEQToComplex (int n) { return n>0 ? new MEQ::ToComplex [n] : new MEQ::ToComplex; }
  
    int aidRegistry_MEQ ()
    {
      static int res = 

        AtomicID::registerId(-1427,"MEQAdd")+
        TypeInfoReg::addToRegistry(-1427,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1427,__construct_MEQAdd)+
        AtomicID::registerId(-1144,"Domain")+
        AtomicID::registerId(-1439,"Nfreq")+
        AtomicID::registerId(-1438,"Times")+
        AtomicID::registerId(-1440,"TimeSteps")+
        AtomicID::registerId(-1442,"MEQConj")+
        TypeInfoReg::addToRegistry(-1442,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1442,__construct_MEQConj)+
        AtomicID::registerId(-1431,"MEQCos")+
        TypeInfoReg::addToRegistry(-1431,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1431,__construct_MEQCos)+
        AtomicID::registerId(-1429,"MEQDivide")+
        TypeInfoReg::addToRegistry(-1429,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1429,__construct_MEQDivide)+
        AtomicID::registerId(-1430,"MEQExp")+
        TypeInfoReg::addToRegistry(-1430,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1430,__construct_MEQExp)+
        AtomicID::registerId(-1447,"MEQFunction")+
        TypeInfoReg::addToRegistry(-1447,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1447,__construct_MEQFunction)+
        AtomicID::registerId(-1418,"MEQMultiply")+
        TypeInfoReg::addToRegistry(-1418,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1418,__construct_MEQMultiply)+
        AtomicID::registerId(-1424,"Node")+
        AtomicID::registerId(-1436,"Class")+
        AtomicID::registerId(-1163,"Name")+
        AtomicID::registerId(-1052,"State")+
        AtomicID::registerId(-1420,"Child")+
        AtomicID::registerId(-1445,"Children")+
        AtomicID::registerId(-1419,"Result")+
        AtomicID::registerId(-1425,"MEQNode")+
        TypeInfoReg::addToRegistry(-1425,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1425,__construct_MEQNode)+
        AtomicID::registerId(-1443,"Tablename")+
        AtomicID::registerId(-1448,"Default")+
        AtomicID::registerId(-1426,"MEQParmPolcStored")+
        TypeInfoReg::addToRegistry(-1426,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1426,__construct_MEQParmPolcStored)+
        AtomicID::registerId(-1433,"Cells")+
        AtomicID::registerId(-1441,"ReqId")+
        AtomicID::registerId(-1435,"CalcDeriv")+
        AtomicID::registerId(-1437,"Rider")+
        AtomicID::registerId(-1396,"Values")+
        AtomicID::registerId(-1434,"ParmValues")+
        AtomicID::registerId(-1446,"Spids")+
        AtomicID::registerId(-1428,"Perturbations")+
        AtomicID::registerId(-1423,"MEQSin")+
        TypeInfoReg::addToRegistry(-1423,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1423,__construct_MEQSin)+
        AtomicID::registerId(-1444,"MEQSqr")+
        TypeInfoReg::addToRegistry(-1444,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1444,__construct_MEQSqr)+
        AtomicID::registerId(-1432,"MEQSqrt")+
        TypeInfoReg::addToRegistry(-1432,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1432,__construct_MEQSqrt)+
        AtomicID::registerId(-1421,"MEQSubtract")+
        TypeInfoReg::addToRegistry(-1421,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1421,__construct_MEQSubtract)+
        AtomicID::registerId(-1422,"MEQToComplex")+
        TypeInfoReg::addToRegistry(-1422,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1422,__construct_MEQToComplex)+
    0;
    return res;
  }
  
  int __dum_call_registries_for_MEQ = aidRegistry_MEQ();

