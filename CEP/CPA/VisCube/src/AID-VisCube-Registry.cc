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
static AtomicID::Register aid_reg_UVData(-1226,"UVData");
static AtomicID::Register aid_reg_UVSet(-1213,"UVSet");
static AtomicID::Register aid_reg_Row(-1224,"Row");
static AtomicID::Register aid_reg_Raw(-1159,"Raw");
static AtomicID::Register aid_reg_Sorted(-1198,"Sorted");
static AtomicID::Register aid_reg_Unsorted(-1170,"Unsorted");
static AtomicID::Register aid_reg_Time(-1218,"Time");
static AtomicID::Register aid_reg_Timeslot(-1185,"Timeslot");
static AtomicID::Register aid_reg_Channel(-1225,"Channel");
static AtomicID::Register aid_reg_Num(-1140,"Num");
static AtomicID::Register aid_reg_Control(-1141,"Control");
static AtomicID::Register aid_reg_MS(-1120,"MS");
static AtomicID::Register aid_reg_Integrate(-1187,"Integrate");
static AtomicID::Register aid_reg_Flag(-1207,"Flag");
static AtomicID::Register aid_reg_Exposure(-1190,"Exposure");
static AtomicID::Register aid_reg_Receptor(-1200,"Receptor");
static AtomicID::Register aid_reg_Antenna(-1194,"Antenna");
static AtomicID::Register aid_reg_IFR(-1220,"IFR");
static AtomicID::Register aid_reg_SPW(-1158,"SPW");
static AtomicID::Register aid_reg_Field(-1162,"Field");
static AtomicID::Register aid_reg_UVW(-1204,"UVW");
static AtomicID::Register aid_reg_Data(-1107,"Data");
static AtomicID::Register aid_reg_Integrated(-1165,"Integrated");
static AtomicID::Register aid_reg_Point(-1173,"Point");
static AtomicID::Register aid_reg_Source(-1212,"Source");
static AtomicID::Register aid_reg_Segment(-1179,"Segment");
static AtomicID::Register aid_reg_Name(-1163,"Name");
static AtomicID::Register aid_reg_Header(-1133,"Header");
static AtomicID::Register aid_reg_Footer(-1202,"Footer");
static AtomicID::Register aid_reg_Patch(-1178,"Patch");
static AtomicID::Register aid_reg_XX(-1155,"XX");
static AtomicID::Register aid_reg_YY(-1168,"YY");
static AtomicID::Register aid_reg_XY(-1156,"XY");
static AtomicID::Register aid_reg_YX(-1166,"YX");
static AtomicID::Register aid_reg_Chunk(-1152,"Chunk");
static AtomicID::Register aid_reg_Indexing(-1169,"Indexing");
static AtomicID::Register aid_reg_Index(-1045,"Index");
static AtomicID::Register aid_reg_Subtable(-1210,"Subtable");
static AtomicID::Register aid_reg_Type(-1098,"Type");
static AtomicID::Register aid_reg_Station(-1167,"Station");
static AtomicID::Register aid_reg_Mount(-1161,"Mount");
static AtomicID::Register aid_reg_Pos(-1164,"Pos");
static AtomicID::Register aid_reg_Offset(-1227,"Offset");
static AtomicID::Register aid_reg_Dish(-1189,"Dish");
static AtomicID::Register aid_reg_Diameter(-1176,"Diameter");
static AtomicID::Register aid_reg_Feed(-1196,"Feed");
static AtomicID::Register aid_reg_Interval(-1177,"Interval");
static AtomicID::Register aid_reg_Polarization(-1215,"Polarization");
static AtomicID::Register aid_reg_Response(-1172,"Response");
static AtomicID::Register aid_reg_Angle(-1182,"Angle");
static AtomicID::Register aid_reg_Ref(-1201,"Ref");
static AtomicID::Register aid_reg_Width(-1186,"Width");
static AtomicID::Register aid_reg_Bandwidth(-1199,"Bandwidth");
static AtomicID::Register aid_reg_Effective(-1206,"Effective");
static AtomicID::Register aid_reg_Resolution(-1175,"Resolution");
static AtomicID::Register aid_reg_Total(-1219,"Total");
static AtomicID::Register aid_reg_Net(-1157,"Net");
static AtomicID::Register aid_reg_Sideband(-1174,"Sideband");
static AtomicID::Register aid_reg_IF(-1228,"IF");
static AtomicID::Register aid_reg_Conv(-1181,"Conv");
static AtomicID::Register aid_reg_Chain(-1203,"Chain");
static AtomicID::Register aid_reg_Group(-1184,"Group");
static AtomicID::Register aid_reg_Desc(-1214,"Desc");
static AtomicID::Register aid_reg_Code(-1222,"Code");
static AtomicID::Register aid_reg_Poly(-1154,"Poly");
static AtomicID::Register aid_reg_Delay(-1217,"Delay");
static AtomicID::Register aid_reg_Dir(-1209,"Dir");
static AtomicID::Register aid_reg_Phase(-1193,"Phase");
static AtomicID::Register aid_reg_Pointing(-1192,"Pointing");
static AtomicID::Register aid_reg_Lines(-1180,"Lines");
static AtomicID::Register aid_reg_Calibration(-1208,"Calibration");
static AtomicID::Register aid_reg_Proper(-1216,"Proper");
static AtomicID::Register aid_reg_Motion(-1211,"Motion");
static AtomicID::Register aid_reg_Sigma(-1197,"Sigma");
static AtomicID::Register aid_reg_Weight(-1223,"Weight");
static AtomicID::Register aid_reg_Origin(-1183,"Origin");
static AtomicID::Register aid_reg_Target(-1205,"Target");
static AtomicID::Register aid_reg_Tracking(-1195,"Tracking");
static AtomicID::Register aid_reg_Beam(-1221,"Beam");
static AtomicID::Register aid_reg_Product(-1153,"Product");
static AtomicID::Register aid_reg_Meas(-1188,"Meas");
static AtomicID::Register aid_reg_Centroid(-1171,"Centroid");
static AtomicID::Register aid_reg_AIPSPP(-1191,"AIPSPP");
static AtomicID::Register aid_reg_Ignore(-1160,"Ignore");
static AtomicID::Register aid_reg_VDSID(-1121,"VDSID");

