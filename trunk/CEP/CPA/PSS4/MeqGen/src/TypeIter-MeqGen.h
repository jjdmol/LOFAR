    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
    #ifndef _TypeIter_MeqGen_h
    #define _TypeIter_MeqGen_h 1



#define DoForAllBinaryTypes_MeqGen(Do,arg,separator) \
        

#define DoForAllDynamicTypes_MeqGen(Do,arg,separator) \
        Do(meqFlagger,arg) separator \
        Do(meqCopy,arg) separator \
        Do(meqCompare,arg) separator \
        Do(meqDFT_GVD,arg) separator \
        Do(meqShiftPhaseCentre,arg)

#define DoForAllNumericTypes_MeqGen(Do,arg,separator) \
        

#define DoForAllIntermediateTypes_MeqGen(Do,arg,separator) \
        

#define DoForAllOtherTypes_MeqGen(Do,arg,separator) \
        

#define DoForAllSpecialTypes_MeqGen(Do,arg,separator) \
        
#endif
