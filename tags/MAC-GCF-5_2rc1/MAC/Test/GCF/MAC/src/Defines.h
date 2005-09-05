#include <GCF/GCF_Defines.h>

//#define DISTRIBUTED

#ifdef DISTRIBUTED
#define REMOTESYS1 "CCU1:"
#define REMOTESYS2 "LCU1:"
#else
#define REMOTESYS1
#define REMOTESYS2
#endif

#define NEXT_TEST(_test_, _descr_) \
  { \
    setCurSubTest(#_test_, _descr_); \
    TRAN(Application::test##_test_); \
  }

#define FINISH \
  { \
    reportSubTest(); \
    TRAN(Application::finished); \
  }

#define ABORT_TESTS \
  { \
    cerr << "TESTS ABORTED due to an ERROR or terminated" << endl; \
    FINISH; \
  }

#define FAIL_AND_ABORT(_txt_) \
  { \
    FAIL(_txt_);  \
    ABORT_TESTS; \
  }

#define TESTC_ABORT_ON_FAIL(cond) \
  if (!TESTC(cond)) \
  { \
    ABORT_TESTS; \
    break; \
  }

#define TESTC_DESCR_ABORT_ON_FAIL(cond, _descr_) \
  if (!TESTC_DESCR(cond, _descr_)) \
  { \
    ABORT_TESTS; \
    break; \
  }

namespace LOFAR
{
 namespace GCF
 {
using namespace Common;
  namespace Test
  {
// property sets

// PropertySetA1: start

PROPERTYCONFIGLIST_BEGIN(propertiesSA1)
  PROPERTYCONFIGLIST_ITEM("F.P4", GCF_READWRITE_PROP, "10")
  PROPERTYCONFIGLIST_ITEM("F.P5", GCF_READWRITE_PROP, "20")
  PROPERTYCONFIGLIST_ITEM("G.P6", GCF_READWRITE_PROP, "12.1")
  PROPERTYCONFIGLIST_ITEM("G.P7", GCF_READWRITE_PROP, "geit")
PROPERTYCONFIGLIST_END
// PropertySetA1: end

// PropertySetB1: start

PROPERTYCONFIGLIST_BEGIN(propertiesSB1)
  PROPERTYCONFIGLIST_ITEM("P1", GCF_READWRITE_PROP, "11")
  PROPERTYCONFIGLIST_ITEM("P2", GCF_READWRITE_PROP, "25")
  PROPERTYCONFIGLIST_ITEM("P3", GCF_READWRITE_PROP, "33.3")
PROPERTYCONFIGLIST_END

// PropertySetB1: end

// PropertySetB2: start

PROPERTYCONFIGLIST_BEGIN(propertiesSB2)
  PROPERTYCONFIGLIST_ITEM("P1", GCF_READWRITE_PROP, "12")
  PROPERTYCONFIGLIST_ITEM("P2", GCF_READWRITE_PROP, "26")
  PROPERTYCONFIGLIST_ITEM("P3", GCF_READWRITE_PROP, "34.3")
PROPERTYCONFIGLIST_END

// PropertySetB2: end

// PropertySetB3: start

PROPERTYCONFIGLIST_BEGIN(propertiesSB3)
  PROPERTYCONFIGLIST_ITEM("P1", GCF_READWRITE_PROP, "13")
  PROPERTYCONFIGLIST_ITEM("P2", GCF_READWRITE_PROP, "27")
  PROPERTYCONFIGLIST_ITEM("P3", GCF_READWRITE_PROP, "35.3")
PROPERTYCONFIGLIST_END

// PropertySetB3: end

// PropertySetB4: start


PROPERTYCONFIGLIST_BEGIN(propertiesSB4)
  PROPERTYCONFIGLIST_ITEM("P1", GCF_READWRITE_PROP, "14")
  PROPERTYCONFIGLIST_ITEM("P2", GCF_READWRITE_PROP, "28")
  PROPERTYCONFIGLIST_ITEM("P3", GCF_READWRITE_PROP, "36.3")
PROPERTYCONFIGLIST_END

// PropertySetB4: end

// PropertySetC1: start

PROPERTYCONFIGLIST_BEGIN(propertiesSC1)
  PROPERTYCONFIGLIST_ITEM("P8", GCF_READWRITE_PROP, "aap")
  PROPERTYCONFIGLIST_ITEM("P9", GCF_READWRITE_PROP, "noot")
PROPERTYCONFIGLIST_END
// PropertySetC1: end

// PropertySetD1: start

PROPERTYCONFIGLIST_BEGIN(propertiesSD1)
  PROPERTYCONFIGLIST_ITEM("J.P00", GCF_READWRITE_PROP, "10.00")
  PROPERTYCONFIGLIST_ITEM("J.P01", GCF_READWRITE_PROP, "10.01")
  PROPERTYCONFIGLIST_ITEM("J.P02", GCF_READWRITE_PROP, "10.02")
  PROPERTYCONFIGLIST_ITEM("J.P03", GCF_READWRITE_PROP, "10.03")
  PROPERTYCONFIGLIST_ITEM("J.P04", GCF_READWRITE_PROP, "10.04")
  PROPERTYCONFIGLIST_ITEM("J.P05", GCF_READWRITE_PROP, "10.05")
  PROPERTYCONFIGLIST_ITEM("J.P06", GCF_READWRITE_PROP, "10.06")
  PROPERTYCONFIGLIST_ITEM("J.P07", GCF_READWRITE_PROP, "10.07")
  PROPERTYCONFIGLIST_ITEM("J.P08", GCF_READWRITE_PROP, "10.08")
  PROPERTYCONFIGLIST_ITEM("J.P09", GCF_READWRITE_PROP, "10.09")
  PROPERTYCONFIGLIST_ITEM("J.P10", GCF_READWRITE_PROP, "10.00")
  PROPERTYCONFIGLIST_ITEM("J.P11", GCF_READWRITE_PROP, "10.11")
  PROPERTYCONFIGLIST_ITEM("J.P12", GCF_READWRITE_PROP, "10.12")
  PROPERTYCONFIGLIST_ITEM("J.P13", GCF_READWRITE_PROP, "10.13")
  PROPERTYCONFIGLIST_ITEM("J.P14", GCF_READWRITE_PROP, "10.14")
  PROPERTYCONFIGLIST_ITEM("J.P15", GCF_READWRITE_PROP, "10.15")
  PROPERTYCONFIGLIST_ITEM("J.P16", GCF_READWRITE_PROP, "10.16")
  PROPERTYCONFIGLIST_ITEM("J.P17", GCF_READWRITE_PROP, "10.17")
  PROPERTYCONFIGLIST_ITEM("J.P18", GCF_READWRITE_PROP, "10.18")
  PROPERTYCONFIGLIST_ITEM("J.P19", GCF_READWRITE_PROP, "10.19")
  PROPERTYCONFIGLIST_ITEM("J.P20", GCF_READWRITE_PROP, "10.20")
  PROPERTYCONFIGLIST_ITEM("J.P21", GCF_READWRITE_PROP, "10.21")
  PROPERTYCONFIGLIST_ITEM("J.P22", GCF_READWRITE_PROP, "10.22")
  PROPERTYCONFIGLIST_ITEM("J.P23", GCF_READWRITE_PROP, "10.23")
  PROPERTYCONFIGLIST_ITEM("J.P24", GCF_READWRITE_PROP, "10.24")
  PROPERTYCONFIGLIST_ITEM("J.P25", GCF_READWRITE_PROP, "10.25")
  PROPERTYCONFIGLIST_ITEM("J.P26", GCF_READWRITE_PROP, "10.26")
  PROPERTYCONFIGLIST_ITEM("J.P27", GCF_READWRITE_PROP, "10.27")
  PROPERTYCONFIGLIST_ITEM("J.P28", GCF_READWRITE_PROP, "10.28")
  PROPERTYCONFIGLIST_ITEM("J.P29", GCF_READWRITE_PROP, "10.29")
  PROPERTYCONFIGLIST_ITEM("J.P30", GCF_READWRITE_PROP, "10.30")
  PROPERTYCONFIGLIST_ITEM("J.P31", GCF_READWRITE_PROP, "10.31")
  PROPERTYCONFIGLIST_ITEM("J.P32", GCF_READWRITE_PROP, "10.32")
  PROPERTYCONFIGLIST_ITEM("J.P33", GCF_READWRITE_PROP, "10.33")
  PROPERTYCONFIGLIST_ITEM("J.P34", GCF_READWRITE_PROP, "10.34")
  PROPERTYCONFIGLIST_ITEM("J.P35", GCF_READWRITE_PROP, "10.35")
  PROPERTYCONFIGLIST_ITEM("J.P36", GCF_READWRITE_PROP, "10.36")
  PROPERTYCONFIGLIST_ITEM("J.P37", GCF_READWRITE_PROP, "10.37")
  PROPERTYCONFIGLIST_ITEM("J.P38", GCF_READWRITE_PROP, "10.38")
  PROPERTYCONFIGLIST_ITEM("J.P39", GCF_READWRITE_PROP, "10.39")
  PROPERTYCONFIGLIST_ITEM("J.P40", GCF_READWRITE_PROP, "10.40")
  PROPERTYCONFIGLIST_ITEM("J.P41", GCF_READWRITE_PROP, "10.41")
  PROPERTYCONFIGLIST_ITEM("J.P42", GCF_READWRITE_PROP, "10.42")
  PROPERTYCONFIGLIST_ITEM("J.P43", GCF_READWRITE_PROP, "10.43")
  PROPERTYCONFIGLIST_ITEM("J.P44", GCF_READWRITE_PROP, "10.44")
  PROPERTYCONFIGLIST_ITEM("J.P45", GCF_READWRITE_PROP, "10.45")
  PROPERTYCONFIGLIST_ITEM("J.P46", GCF_READWRITE_PROP, "10.46")
  PROPERTYCONFIGLIST_ITEM("J.P47", GCF_READWRITE_PROP, "10.47")
  PROPERTYCONFIGLIST_ITEM("J.P48", GCF_READWRITE_PROP, "10.48")
  PROPERTYCONFIGLIST_ITEM("J.P49", GCF_READWRITE_PROP, "10.49")
  PROPERTYCONFIGLIST_ITEM("J.P50", GCF_READWRITE_PROP, "10.50")
  PROPERTYCONFIGLIST_ITEM("J.P51", GCF_READWRITE_PROP, "10.51")
  PROPERTYCONFIGLIST_ITEM("J.P52", GCF_READWRITE_PROP, "10.52")
  PROPERTYCONFIGLIST_ITEM("J.P53", GCF_READWRITE_PROP, "10.53")
  PROPERTYCONFIGLIST_ITEM("J.P54", GCF_READWRITE_PROP, "10.54")
  PROPERTYCONFIGLIST_ITEM("J.P55", GCF_READWRITE_PROP, "10.55")
  PROPERTYCONFIGLIST_ITEM("J.P56", GCF_READWRITE_PROP, "10.56")
  PROPERTYCONFIGLIST_ITEM("J.P57", GCF_READWRITE_PROP, "10.57")
  PROPERTYCONFIGLIST_ITEM("J.P58", GCF_READWRITE_PROP, "10.58")
  PROPERTYCONFIGLIST_ITEM("J.P59", GCF_READWRITE_PROP, "10.59")
  PROPERTYCONFIGLIST_ITEM("J.P60", GCF_READWRITE_PROP, "10.60")
  PROPERTYCONFIGLIST_ITEM("J.P61", GCF_READWRITE_PROP, "10.61")
  PROPERTYCONFIGLIST_ITEM("J.P62", GCF_READWRITE_PROP, "10.62")
  PROPERTYCONFIGLIST_ITEM("J.P63", GCF_READWRITE_PROP, "10.63")
  PROPERTYCONFIGLIST_ITEM("J.P64", GCF_READWRITE_PROP, "10.64")
  PROPERTYCONFIGLIST_ITEM("J.P65", GCF_READWRITE_PROP, "10.65")
  PROPERTYCONFIGLIST_ITEM("J.P66", GCF_READWRITE_PROP, "10.66")
  PROPERTYCONFIGLIST_ITEM("J.P67", GCF_READWRITE_PROP, "10.67")
  PROPERTYCONFIGLIST_ITEM("J.P68", GCF_READWRITE_PROP, "10.68")
  PROPERTYCONFIGLIST_ITEM("J.P69", GCF_READWRITE_PROP, "10.69")
  PROPERTYCONFIGLIST_ITEM("J.P70", GCF_READWRITE_PROP, "10.70")
  PROPERTYCONFIGLIST_ITEM("J.P71", GCF_READWRITE_PROP, "10.71")
  PROPERTYCONFIGLIST_ITEM("J.P72", GCF_READWRITE_PROP, "10.72")
  PROPERTYCONFIGLIST_ITEM("J.P73", GCF_READWRITE_PROP, "10.73")
  PROPERTYCONFIGLIST_ITEM("J.P74", GCF_READWRITE_PROP, "10.74")
  PROPERTYCONFIGLIST_ITEM("J.P75", GCF_READWRITE_PROP, "10.75")
  PROPERTYCONFIGLIST_ITEM("J.P76", GCF_READWRITE_PROP, "10.76")
  PROPERTYCONFIGLIST_ITEM("J.P77", GCF_READWRITE_PROP, "10.77")
  PROPERTYCONFIGLIST_ITEM("J.P78", GCF_READWRITE_PROP, "10.78")
  PROPERTYCONFIGLIST_ITEM("J.P79", GCF_READWRITE_PROP, "10.79")
  PROPERTYCONFIGLIST_ITEM("J.P80", GCF_READWRITE_PROP, "10.80")
  PROPERTYCONFIGLIST_ITEM("J.P81", GCF_READWRITE_PROP, "10.81")
  PROPERTYCONFIGLIST_ITEM("J.P82", GCF_READWRITE_PROP, "10.82")
  PROPERTYCONFIGLIST_ITEM("J.P83", GCF_READWRITE_PROP, "10.83")
  PROPERTYCONFIGLIST_ITEM("J.P84", GCF_READWRITE_PROP, "10.84")
  PROPERTYCONFIGLIST_ITEM("J.P85", GCF_READWRITE_PROP, "10.85")
  PROPERTYCONFIGLIST_ITEM("J.P86", GCF_READWRITE_PROP, "10.86")
  PROPERTYCONFIGLIST_ITEM("J.P87", GCF_READWRITE_PROP, "10.87")
  PROPERTYCONFIGLIST_ITEM("J.P88", GCF_READWRITE_PROP, "10.88")
  PROPERTYCONFIGLIST_ITEM("J.P89", GCF_READWRITE_PROP, "10.89")
  PROPERTYCONFIGLIST_ITEM("J.P90", GCF_READWRITE_PROP, "10.90")
  PROPERTYCONFIGLIST_ITEM("J.P91", GCF_READWRITE_PROP, "10.91")
  PROPERTYCONFIGLIST_ITEM("J.P92", GCF_READWRITE_PROP, "10.92")
  PROPERTYCONFIGLIST_ITEM("J.P93", GCF_READWRITE_PROP, "10.93")
  PROPERTYCONFIGLIST_ITEM("J.P94", GCF_READWRITE_PROP, "10.94")
  PROPERTYCONFIGLIST_ITEM("J.P95", GCF_READWRITE_PROP, "10.95")
  PROPERTYCONFIGLIST_ITEM("J.P96", GCF_READWRITE_PROP, "10.96")
  PROPERTYCONFIGLIST_ITEM("J.P97", GCF_READWRITE_PROP, "10.97")
  PROPERTYCONFIGLIST_ITEM("J.P98", GCF_READWRITE_PROP, "10.98")
  PROPERTYCONFIGLIST_ITEM("J.P99", GCF_READWRITE_PROP, "10.99")
PROPERTYCONFIGLIST_END

// PropertySetD1: end

// PropertySetE1: start

PROPERTYCONFIGLIST_BEGIN(propertiesSE1)
  PROPERTYCONFIGLIST_ITEM("P1", GCF_READWRITE_PROP, "11")
  PROPERTYCONFIGLIST_ITEM("P2", GCF_READWRITE_PROP, "25")
  PROPERTYCONFIGLIST_ITEM("P3", GCF_READWRITE_PROP, "33.3")
  PROPERTYCONFIGLIST_ITEM("P4", GCF_READWRITE_PROP, "11")
  PROPERTYCONFIGLIST_ITEM("P5", GCF_READWRITE_PROP, "25")
  PROPERTYCONFIGLIST_ITEM("P6", GCF_READWRITE_PROP, "33.3")
PROPERTYCONFIGLIST_END

// PropertySetE1: end
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
