    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/src/build_aid_maps.pl
    #ifndef _TypeIter_VisDM_h
    #define _TypeIter_VisDM_h 1



#define DoForAllBinaryTypes_VisDM(Do,arg,separator) \
        

#define DoForAllDynamicTypes_VisDM(Do,arg,separator) \
        Do(ColumnarTableTile,arg) separator \
        Do(TableFormat,arg) separator \
        Do(VisCube,arg) separator \
        Do(VisCubeSet,arg) separator \
        Do(VisTile,arg)

#define DoForAllNumericTypes_VisDM(Do,arg,separator) \
        

#define DoForAllIntermediateTypes_VisDM(Do,arg,separator) \
        

#define DoForAllOtherTypes_VisDM(Do,arg,separator) \
        

#define DoForAllSpecialTypes_VisDM(Do,arg,separator) \
        
#endif
