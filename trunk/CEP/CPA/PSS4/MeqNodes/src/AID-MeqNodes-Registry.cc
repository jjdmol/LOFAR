    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
    #include <DMI/AtomicID.h>
    #include <DMI/TypeInfo.h>
    #include <DMI/DynamicTypeManager.h>
    #include <DMI/Packer.h>
    
#include "Constant.h"
BlockableObject * __construct_MeqConstant (int n) { return n>0 ? new Meq::Constant [n] : new Meq::Constant; }
#include "Parm.h"
BlockableObject * __construct_MeqParm (int n) { return n>0 ? new Meq::Parm [n] : new Meq::Parm; }
#include "Freq.h"
BlockableObject * __construct_MeqFreq (int n) { return n>0 ? new Meq::Freq [n] : new Meq::Freq; }
#include "Time.h"
BlockableObject * __construct_MeqTime (int n) { return n>0 ? new Meq::Time [n] : new Meq::Time; }
#include "Selector.h"
BlockableObject * __construct_MeqSelector (int n) { return n>0 ? new Meq::Selector [n] : new Meq::Selector; }
#include "Composer.h"
BlockableObject * __construct_MeqComposer (int n) { return n>0 ? new Meq::Composer [n] : new Meq::Composer; }
#include "Add.h"
BlockableObject * __construct_MeqAdd (int n) { return n>0 ? new Meq::Add [n] : new Meq::Add; }
#include "Subtract.h"
BlockableObject * __construct_MeqSubtract (int n) { return n>0 ? new Meq::Subtract [n] : new Meq::Subtract; }
#include "Multiply.h"
BlockableObject * __construct_MeqMultiply (int n) { return n>0 ? new Meq::Multiply [n] : new Meq::Multiply; }
#include "Divide.h"
BlockableObject * __construct_MeqDivide (int n) { return n>0 ? new Meq::Divide [n] : new Meq::Divide; }
#include "Sin.h"
BlockableObject * __construct_MeqSin (int n) { return n>0 ? new Meq::Sin [n] : new Meq::Sin; }
#include "Cos.h"
BlockableObject * __construct_MeqCos (int n) { return n>0 ? new Meq::Cos [n] : new Meq::Cos; }
#include "Exp.h"
BlockableObject * __construct_MeqExp (int n) { return n>0 ? new Meq::Exp [n] : new Meq::Exp; }
#include "Pow.h"
BlockableObject * __construct_MeqPow (int n) { return n>0 ? new Meq::Pow [n] : new Meq::Pow; }
#include "Sqr.h"
BlockableObject * __construct_MeqSqr (int n) { return n>0 ? new Meq::Sqr [n] : new Meq::Sqr; }
#include "Sqrt.h"
BlockableObject * __construct_MeqSqrt (int n) { return n>0 ? new Meq::Sqrt [n] : new Meq::Sqrt; }
#include "Conj.h"
BlockableObject * __construct_MeqConj (int n) { return n>0 ? new Meq::Conj [n] : new Meq::Conj; }
#include "ToComplex.h"
BlockableObject * __construct_MeqToComplex (int n) { return n>0 ? new Meq::ToComplex [n] : new Meq::ToComplex; }
#include "UVW.h"
BlockableObject * __construct_MeqUVW (int n) { return n>0 ? new Meq::UVW [n] : new Meq::UVW; }
#include "LMN.h"
BlockableObject * __construct_MeqLMN (int n) { return n>0 ? new Meq::LMN [n] : new Meq::LMN; }
#include "StatPointSourceDFT.h"
BlockableObject * __construct_MeqStatPointSourceDFT (int n) { return n>0 ? new Meq::StatPointSourceDFT [n] : new Meq::StatPointSourceDFT; }
#include "PointSourceDFT.h"
BlockableObject * __construct_MeqPointSourceDFT (int n) { return n>0 ? new Meq::PointSourceDFT [n] : new Meq::PointSourceDFT; }
#include "ModRes.h"
BlockableObject * __construct_MeqModRes (int n) { return n>0 ? new Meq::ModRes [n] : new Meq::ModRes; }
#include "Condeq.h"
BlockableObject * __construct_MeqCondeq (int n) { return n>0 ? new Meq::Condeq [n] : new Meq::Condeq; }
#include "Solver.h"
BlockableObject * __construct_MeqSolver (int n) { return n>0 ? new Meq::Solver [n] : new Meq::Solver; }
#include "ZeroFlagger.h"
BlockableObject * __construct_MeqZeroFlagger (int n) { return n>0 ? new Meq::ZeroFlagger [n] : new Meq::ZeroFlagger; }
#include "MergeFlags.h"
BlockableObject * __construct_MeqMergeFlags (int n) { return n>0 ? new Meq::MergeFlags [n] : new Meq::MergeFlags; }
#include "Resampler.h"
BlockableObject * __construct_MeqResampler (int n) { return n>0 ? new Meq::Resampler [n] : new Meq::Resampler; }
#include "ReqSeq.h"
BlockableObject * __construct_MeqReqSeq (int n) { return n>0 ? new Meq::ReqSeq [n] : new Meq::ReqSeq; }
  
    int aidRegistry_MeqNodes ()
    {
      static int res = 

        AtomicID::registerId(-1387,"meqconstant")+
        TypeInfoReg::addToRegistry(-1387,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1387,__construct_MeqConstant)+
        AtomicID::registerId(-1244,"meqparm")+
        TypeInfoReg::addToRegistry(-1244,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1244,__construct_MeqParm)+
        AtomicID::registerId(-1219,"meqfreq")+
        TypeInfoReg::addToRegistry(-1219,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1219,__construct_MeqFreq)+
        AtomicID::registerId(-1225,"meqtime")+
        TypeInfoReg::addToRegistry(-1225,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1225,__construct_MeqTime)+
        AtomicID::registerId(-1255,"meqselector")+
        TypeInfoReg::addToRegistry(-1255,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1255,__construct_MeqSelector)+
        AtomicID::registerId(-1241,"meqcomposer")+
        TypeInfoReg::addToRegistry(-1241,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1241,__construct_MeqComposer)+
        AtomicID::registerId(-1236,"meqadd")+
        TypeInfoReg::addToRegistry(-1236,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1236,__construct_MeqAdd)+
        AtomicID::registerId(-1232,"meqsubtract")+
        TypeInfoReg::addToRegistry(-1232,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1232,__construct_MeqSubtract)+
        AtomicID::registerId(-1227,"meqmultiply")+
        TypeInfoReg::addToRegistry(-1227,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1227,__construct_MeqMultiply)+
        AtomicID::registerId(-1223,"meqdivide")+
        TypeInfoReg::addToRegistry(-1223,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1223,__construct_MeqDivide)+
        AtomicID::registerId(-1224,"meqsin")+
        TypeInfoReg::addToRegistry(-1224,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1224,__construct_MeqSin)+
        AtomicID::registerId(-1243,"meqcos")+
        TypeInfoReg::addToRegistry(-1243,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1243,__construct_MeqCos)+
        AtomicID::registerId(-1240,"meqexp")+
        TypeInfoReg::addToRegistry(-1240,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1240,__construct_MeqExp)+
        AtomicID::registerId(-1214,"meqpow")+
        TypeInfoReg::addToRegistry(-1214,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1214,__construct_MeqPow)+
        AtomicID::registerId(-1249,"meqsqr")+
        TypeInfoReg::addToRegistry(-1249,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1249,__construct_MeqSqr)+
        AtomicID::registerId(-1251,"meqsqrt")+
        TypeInfoReg::addToRegistry(-1251,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1251,__construct_MeqSqrt)+
        AtomicID::registerId(-1212,"meqconj")+
        TypeInfoReg::addToRegistry(-1212,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1212,__construct_MeqConj)+
        AtomicID::registerId(-1217,"meqtocomplex")+
        TypeInfoReg::addToRegistry(-1217,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1217,__construct_MeqToComplex)+
        AtomicID::registerId(-1239,"mequvw")+
        TypeInfoReg::addToRegistry(-1239,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1239,__construct_MeqUVW)+
        AtomicID::registerId(-1380,"ra")+
        AtomicID::registerId(-1379,"dec")+
        AtomicID::registerId(-1030,"x")+
        AtomicID::registerId(-1031,"y")+
        AtomicID::registerId(-1032,"z")+
        AtomicID::registerId(-1467,"meqlmn")+
        TypeInfoReg::addToRegistry(-1467,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1467,__construct_MeqLMN)+
        AtomicID::registerId(-1468,"meqstatpointsourcedft")+
        TypeInfoReg::addToRegistry(-1468,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1468,__construct_MeqStatPointSourceDFT)+
        AtomicID::registerId(-1476,"lmn")+
        AtomicID::registerId(-1158,"uvw")+
        AtomicID::registerId(-1469,"meqpointsourcedft")+
        TypeInfoReg::addToRegistry(-1469,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1469,__construct_MeqPointSourceDFT)+
        AtomicID::registerId(-1474,"st")+
        AtomicID::registerId(-1475,"dft")+
        AtomicID::registerId(-1020,"n")+
        AtomicID::registerId(-1447,"meqmodres")+
        TypeInfoReg::addToRegistry(-1447,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1447,__construct_MeqModRes)+
        AtomicID::registerId(-1448,"factor")+
        AtomicID::registerId(-1177,"num")+
        AtomicID::registerId(-1247,"cells")+
        AtomicID::registerId(-1365,"meqcondeq")+
        TypeInfoReg::addToRegistry(-1365,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1365,__construct_MeqCondeq)+
        AtomicID::registerId(-1367,"meqsolver")+
        TypeInfoReg::addToRegistry(-1367,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1367,__construct_MeqSolver)+
        AtomicID::registerId(-1433,"meqzeroflagger")+
        TypeInfoReg::addToRegistry(-1433,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1433,__construct_MeqZeroFlagger)+
        AtomicID::registerId(-1435,"oper")+
        AtomicID::registerId(-1135,"flag")+
        AtomicID::registerId(-1428,"bit")+
        AtomicID::registerId(-1434,"eq")+
        AtomicID::registerId(-1429,"ne")+
        AtomicID::registerId(-1436,"lt")+
        AtomicID::registerId(-1432,"gt")+
        AtomicID::registerId(-1430,"le")+
        AtomicID::registerId(-1431,"ge")+
        AtomicID::registerId(-1437,"meqmergeflags")+
        TypeInfoReg::addToRegistry(-1437,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1437,__construct_MeqMergeFlags)+
        AtomicID::registerId(-1287,"mask")+
        AtomicID::registerId(-1439,"meqresampler")+
        TypeInfoReg::addToRegistry(-1439,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1439,__construct_MeqResampler)+
        AtomicID::registerId(-1172,"integrate")+
        AtomicID::registerId(-1440,"density")+
        AtomicID::registerId(-1478,"meqreqseq")+
        TypeInfoReg::addToRegistry(-1478,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1478,__construct_MeqReqSeq)+
        AtomicID::registerId(-1477,"only")+
    0;
    return res;
  }
  
  int __dum_call_registries_for_MeqNodes = aidRegistry_MeqNodes();

