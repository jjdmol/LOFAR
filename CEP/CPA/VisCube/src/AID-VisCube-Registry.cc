    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/src/build_aid_maps.pl
    #include "DMI/AtomicID.h"
    #include "DMI/TypeInfo.h"
    #include "DMI/DynamicTypeManager.h"
    #include "DMI/Packer.h"

    int aidRegistry_VisCube ()
    {
      return 1;
    }

static AtomicID::Register aid_reg_ColumnarTableTile(-1037,"ColumnarTableTile");
#include "ColumnarTableTile.h"
static TypeInfoReg::Register ti_reg_ColumnarTableTile(-1037,TypeInfo(TypeInfo::DYNAMIC,0));
BlockableObject * __construct_ColumnarTableTile (int n) { return n>0 ? new ColumnarTableTile [n] : new ColumnarTableTile; }
static DynamicTypeManager::Register dtm_reg_ColumnarTableTile(-1037,__construct_ColumnarTableTile);
static AtomicID::Register aid_reg_TableFormat(-1036,"TableFormat");
#include "TableFormat.h"
static TypeInfoReg::Register ti_reg_TableFormat(-1036,TypeInfo(TypeInfo::DYNAMIC,0));
BlockableObject * __construct_TableFormat (int n) { return n>0 ? new TableFormat [n] : new TableFormat; }
static DynamicTypeManager::Register dtm_reg_TableFormat(-1036,__construct_TableFormat);
static AtomicID::Register aid_reg_VisCube(-1038,"VisCube");
#include "VisCube.h"
static TypeInfoReg::Register ti_reg_VisCube(-1038,TypeInfo(TypeInfo::DYNAMIC,0));
BlockableObject * __construct_VisCube (int n) { return n>0 ? new VisCube [n] : new VisCube; }
static DynamicTypeManager::Register dtm_reg_VisCube(-1038,__construct_VisCube);
static AtomicID::Register aid_reg_Corr(-1111,"Corr");
static AtomicID::Register aid_reg_Freq(-1110,"Freq");
static AtomicID::Register aid_reg_VisCubeSet(-1034,"VisCubeSet");
#include "VisCubeSet.h"
static TypeInfoReg::Register ti_reg_VisCubeSet(-1034,TypeInfo(TypeInfo::DYNAMIC,0));
BlockableObject * __construct_VisCubeSet (int n) { return n>0 ? new VisCubeSet [n] : new VisCubeSet; }
static DynamicTypeManager::Register dtm_reg_VisCubeSet(-1034,__construct_VisCubeSet);
static AtomicID::Register aid_reg_VisTile(-1035,"VisTile");
#include "VisTile.h"
static TypeInfoReg::Register ti_reg_VisTile(-1035,TypeInfo(TypeInfo::DYNAMIC,0));
BlockableObject * __construct_VisTile (int n) { return n>0 ? new VisTile [n] : new VisTile; }
static DynamicTypeManager::Register dtm_reg_VisTile(-1035,__construct_VisTile);

