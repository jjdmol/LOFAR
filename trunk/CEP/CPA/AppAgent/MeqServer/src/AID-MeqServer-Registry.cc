    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
    #include <DMI/AtomicID.h>
    #include <DMI/TypeInfo.h>
    #include <DMI/DynamicTypeManager.h>
    #include <DMI/Packer.h>
    
#include "Sink.h"
BlockableObject * __construct_MeqSink (int n) { return n>0 ? new Meq::Sink [n] : new Meq::Sink; }
#include "Spigot.h"
BlockableObject * __construct_MeqSpigot (int n) { return n>0 ? new Meq::Spigot [n] : new Meq::Spigot; }
  
    int aidRegistry_MeqServer ()
    {
      static int res = 

        AtomicID::registerId(-1233,"node")+
        AtomicID::registerId(-1188,"name")+
        AtomicID::registerId(-1324,"nodeindex")+
        AtomicID::registerId(-1327,"meqserver")+
        AtomicID::registerId(-1332,"create")+
        AtomicID::registerId(-1335,"delete")+
        AtomicID::registerId(-1338,"get")+
        AtomicID::registerId(-1316,"set")+
        AtomicID::registerId(-1060,"state")+
        AtomicID::registerId(-1210,"request")+
        AtomicID::registerId(-1331,"resolve")+
        AtomicID::registerId(-1226,"child")+
        AtomicID::registerId(-1220,"children")+
        AtomicID::registerId(-1046,"list")+
        AtomicID::registerId(-1089,"app")+
        AtomicID::registerId(-1277,"command")+
        AtomicID::registerId(-1328,"args")+
        AtomicID::registerId(-1228,"result")+
        AtomicID::registerId(-1116,"data")+
        AtomicID::registerId(-1333,"processing")+
        AtomicID::registerId(-1084,"error")+
        AtomicID::registerId(-1045,"message")+
        AtomicID::registerId(-1164,"code")+
        AtomicID::registerId(-1372,"execute")+
        AtomicID::registerId(-1389,"clear")+
        AtomicID::registerId(-1375,"cache")+
        AtomicID::registerId(-1391,"save")+
        AtomicID::registerId(-1390,"load")+
        AtomicID::registerId(-1388,"forest")+
        AtomicID::registerId(-1392,"recursive")+
        AtomicID::registerId(-1055,"publish")+
        AtomicID::registerId(-1245,"results")+
        AtomicID::registerId(-1406,"enable")+
        AtomicID::registerId(-1420,"disable")+
        AtomicID::registerId(-1070,"event")+
        AtomicID::registerId(-1087,"id")+
        AtomicID::registerId(-1459,"silent")+
        AtomicID::registerId(-1386,"addstate")+
        AtomicID::registerId(-1126,"station")+
        AtomicID::registerId(-1051,"index")+
        AtomicID::registerId(-1280,"tile")+
        AtomicID::registerId(-1293,"format")+
        AtomicID::registerId(-1329,"vishandlernode")+
        AtomicID::registerId(-1177,"num")+
        AtomicID::registerId(-1147,"antenna")+
        AtomicID::registerId(-1092,"input")+
        AtomicID::registerId(-1282,"output")+
        AtomicID::registerId(-1334,"col")+
        AtomicID::registerId(-1153,"corr")+
        AtomicID::registerId(-1336,"next")+
        AtomicID::registerId(-1424,"read")+
        AtomicID::registerId(-1135,"flag")+
        AtomicID::registerId(-1298,"flags")+
        AtomicID::registerId(-1287,"mask")+
        AtomicID::registerId(-1150,"row")+
        AtomicID::registerId(-1481,"mandate")+
        AtomicID::registerId(-1482,"regular")+
        AtomicID::registerId(-1417,"grid")+
        AtomicID::registerId(-1337,"sink")+
        AtomicID::registerId(-1326,"meqsink")+
        TypeInfoReg::addToRegistry(-1326,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1326,__construct_MeqSink)+
        AtomicID::registerId(-1325,"spigot")+
        AtomicID::registerId(-1330,"meqspigot")+
        TypeInfoReg::addToRegistry(-1330,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1330,__construct_MeqSpigot)+
    0;
    return res;
  }
  
  int __dum_call_registries_for_MeqServer = aidRegistry_MeqServer();

