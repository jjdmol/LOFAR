    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
    #ifndef _TypeIter_MeqNodes_h
    #define _TypeIter_MeqNodes_h 1



#define DoForAllOtherTypes_MeqNodes(Do,arg,separator) \
        

#define DoForAllBinaryTypes_MeqNodes(Do,arg,separator) \
        

#define DoForAllSpecialTypes_MeqNodes(Do,arg,separator) \
        

#define DoForAllIntermediateTypes_MeqNodes(Do,arg,separator) \
        

#define DoForAllDynamicTypes_MeqNodes(Do,arg,separator) \
        Do(Meq::Constant,arg) separator \
        Do(Meq::Parm,arg) separator \
        Do(Meq::Freq,arg) separator \
        Do(Meq::Time,arg) separator \
        Do(Meq::Selector,arg) separator \
        Do(Meq::Composer,arg) separator \
        Do(Meq::Add,arg) separator \
        Do(Meq::Subtract,arg) separator \
        Do(Meq::Multiply,arg) separator \
        Do(Meq::Divide,arg) separator \
        Do(Meq::Sin,arg) separator \
        Do(Meq::Cos,arg) separator \
        Do(Meq::Exp,arg) separator \
        Do(Meq::Pow,arg) separator \
        Do(Meq::Sqr,arg) separator \
        Do(Meq::Sqrt,arg) separator \
        Do(Meq::Conj,arg) separator \
        Do(Meq::ToComplex,arg) separator \
        Do(Meq::Polar,arg) separator \
        Do(Meq::UVW,arg) separator \
        Do(Meq::LMN,arg) separator \
        Do(Meq::StatPointSourceDFT,arg) separator \
        Do(Meq::PointSourceDFT,arg) separator \
        Do(Meq::Condeq,arg) separator \
        Do(Meq::Solver,arg) separator \
        Do(Meq::MergeFlags,arg) separator \
        Do(Meq::ReqSeq,arg)

#define DoForAllNumericTypes_MeqNodes(Do,arg,separator) \
        
#endif
