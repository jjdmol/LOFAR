      #ifndef TID_VisCube_h
      #define TID_VisCube_h 1

      // This file is generated automatically -- do not edit
      // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
      #include "DMI/TypeId.h"

      // should be called somewhere in order to link in the registry
      int aidRegistry_VisCube ();

#ifndef _defined_id_TpColumnarTableTile
#define _defined_id_TpColumnarTableTile 1
const TypeId TpColumnarTableTile(-1168);          // from /home/oms/LOFAR/CEP/CPA/VisCube/src/ColumnarTableTile.h:9
const int TpColumnarTableTile_int = -1168;
class ColumnarTableTile;
            template<>
            class DMIBaseTypeTraits<ColumnarTableTile> : public TypeTraits<ColumnarTableTile>
            {
              public:
              enum { isContainable = true };
              enum { typeId = TpColumnarTableTile_int };
              enum { TypeCategory = TypeCategories::DYNAMIC };
              enum { ParamByRef = true, ReturnByRef = true };
              typedef const ColumnarTableTile & ContainerReturnType;
              typedef const ColumnarTableTile & ContainerParamType;
            };
#endif
#ifndef _defined_id_TpTableFormat
#define _defined_id_TpTableFormat 1
const TypeId TpTableFormat(-1127);                // from /home/oms/LOFAR/CEP/CPA/VisCube/src/TableFormat.h:9
const int TpTableFormat_int = -1127;
class TableFormat;
            template<>
            class DMIBaseTypeTraits<TableFormat> : public TypeTraits<TableFormat>
            {
              public:
              enum { isContainable = true };
              enum { typeId = TpTableFormat_int };
              enum { TypeCategory = TypeCategories::DYNAMIC };
              enum { ParamByRef = true, ReturnByRef = true };
              typedef const TableFormat & ContainerReturnType;
              typedef const TableFormat & ContainerParamType;
            };
#endif
#ifndef _defined_id_TpVisCube
#define _defined_id_TpVisCube 1
const TypeId TpVisCube(-1197);                    // from /home/oms/LOFAR/CEP/CPA/VisCube/src/VisCube.h:33
const int TpVisCube_int = -1197;
class VisCube;
            template<>
            class DMIBaseTypeTraits<VisCube> : public TypeTraits<VisCube>
            {
              public:
              enum { isContainable = true };
              enum { typeId = TpVisCube_int };
              enum { TypeCategory = TypeCategories::DYNAMIC };
              enum { ParamByRef = true, ReturnByRef = true };
              typedef const VisCube & ContainerReturnType;
              typedef const VisCube & ContainerParamType;
            };
#endif
#ifndef _defined_id_TpVisCubeSet
#define _defined_id_TpVisCubeSet 1
const TypeId TpVisCubeSet(-1180);                 // from /home/oms/LOFAR/CEP/CPA/VisCube/src/VisCubeSet.h:12
const int TpVisCubeSet_int = -1180;
class VisCubeSet;
            template<>
            class DMIBaseTypeTraits<VisCubeSet> : public TypeTraits<VisCubeSet>
            {
              public:
              enum { isContainable = true };
              enum { typeId = TpVisCubeSet_int };
              enum { TypeCategory = TypeCategories::DYNAMIC };
              enum { ParamByRef = true, ReturnByRef = true };
              typedef const VisCubeSet & ContainerReturnType;
              typedef const VisCubeSet & ContainerParamType;
            };
#endif
#ifndef _defined_id_TpVisTile
#define _defined_id_TpVisTile 1
const TypeId TpVisTile(-1181);                    // from /home/oms/LOFAR/CEP/CPA/VisCube/src/VisTile.h:47
const int TpVisTile_int = -1181;
class VisTile;
            template<>
            class DMIBaseTypeTraits<VisTile> : public TypeTraits<VisTile>
            {
              public:
              enum { isContainable = true };
              enum { typeId = TpVisTile_int };
              enum { TypeCategory = TypeCategories::DYNAMIC };
              enum { ParamByRef = true, ReturnByRef = true };
              typedef const VisTile & ContainerReturnType;
              typedef const VisTile & ContainerParamType;
            };
#endif


#endif
