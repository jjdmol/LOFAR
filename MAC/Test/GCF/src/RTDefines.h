#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>

//#define WAIT_FOR_INPUT

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)
// property sets

// echoPingPSET: start

const TProperty echoPingPSETprops[] =
{
  {"maxSeqNr", GCFPValue::LPT_INTEGER, GCF_WRITABLE_PROP, "100"},
  {"seqNr", GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP, "0"},
};

const TPropertySet echoPingPSET = 
{
  2, "B_RT", echoPingPSETprops
};

// echoPingPSET: end

