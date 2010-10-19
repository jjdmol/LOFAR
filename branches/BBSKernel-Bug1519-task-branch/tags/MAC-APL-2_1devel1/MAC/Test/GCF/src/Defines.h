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
    cout << "TESTS ABORTED due to an ERROR or terminated" << endl; \
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

// property sets

// PropertySetA1: start

const TPropertyConfig propertiesSA1[] =
{
  {"F.P4", GCF_READWRITE_PROP, "10"},
  {"F.P5", GCF_READWRITE_PROP, "20"},
  {"G.P6", GCF_READWRITE_PROP, "12.1"},
  {"G.P7", GCF_READWRITE_PROP, "geit"},
};

// PropertySetA1: end

// PropertySetB1: start

const TPropertyConfig propertiesSB1[] =
{
  {"P1", GCF_READWRITE_PROP, "11"},
  {"P2", GCF_READWRITE_PROP, "25"},
  {"P3", GCF_READWRITE_PROP, "33.3"},
};

// PropertySetB1: end

// PropertySetB2: start

const TPropertyConfig propertiesSB2[] =
{
  {"P1", GCF_READWRITE_PROP, "12"},
  {"P2", GCF_READWRITE_PROP, "26"},
  {"P3", GCF_READWRITE_PROP, "34.3"},
};

// PropertySetB2: end

// PropertySetB3: start

const TPropertyConfig propertiesSB3[] =
{
  {"P1", GCF_READWRITE_PROP, "13"},
  {"P2", GCF_READWRITE_PROP, "27"},
  {"P3", GCF_READWRITE_PROP, "35.3"},
};

// PropertySetB3: end

// PropertySetB4: start


const TPropertyConfig propertiesSB4[] =
{
  {"P1", GCF_READWRITE_PROP, "14"},
  {"P2", GCF_READWRITE_PROP, "28"},
  {"P3", GCF_READWRITE_PROP, "36.3"},
};

// PropertySetB4: end

// PropertySetC1: start

const TPropertyConfig propertiesSC1[] =
{
  {"P8", GCF_READWRITE_PROP, "aap"},
  {"P9", GCF_READWRITE_PROP, "noot"},
};
// PropertySetC1: end

// PropertySetD1: start

const TPropertyConfig propertiesSD1[] =
{
  {"J.P00", GCF_READWRITE_PROP, "10.00"},
  {"J.P01", GCF_READWRITE_PROP, "10.01"},
  {"J.P02", GCF_READWRITE_PROP, "10.02"},
  {"J.P03", GCF_READWRITE_PROP, "10.03"},
  {"J.P04", GCF_READWRITE_PROP, "10.04"},
  {"J.P05", GCF_READWRITE_PROP, "10.05"},
  {"J.P06", GCF_READWRITE_PROP, "10.06"},
  {"J.P07", GCF_READWRITE_PROP, "10.07"},
  {"J.P08", GCF_READWRITE_PROP, "10.08"},
  {"J.P09", GCF_READWRITE_PROP, "10.09"},
  {"J.P10", GCF_READWRITE_PROP, "10.00"},
  {"J.P11", GCF_READWRITE_PROP, "10.11"},
  {"J.P12", GCF_READWRITE_PROP, "10.12"},
  {"J.P13", GCF_READWRITE_PROP, "10.13"},
  {"J.P14", GCF_READWRITE_PROP, "10.14"},
  {"J.P15", GCF_READWRITE_PROP, "10.15"},
  {"J.P16", GCF_READWRITE_PROP, "10.16"},
  {"J.P17", GCF_READWRITE_PROP, "10.17"},
  {"J.P18", GCF_READWRITE_PROP, "10.18"},
  {"J.P19", GCF_READWRITE_PROP, "10.19"},
  {"J.P20", GCF_READWRITE_PROP, "10.20"},
  {"J.P21", GCF_READWRITE_PROP, "10.21"},
  {"J.P22", GCF_READWRITE_PROP, "10.22"},
  {"J.P23", GCF_READWRITE_PROP, "10.23"},
  {"J.P24", GCF_READWRITE_PROP, "10.24"},
  {"J.P25", GCF_READWRITE_PROP, "10.25"},
  {"J.P26", GCF_READWRITE_PROP, "10.26"},
  {"J.P27", GCF_READWRITE_PROP, "10.27"},
  {"J.P28", GCF_READWRITE_PROP, "10.28"},
  {"J.P29", GCF_READWRITE_PROP, "10.29"},
  {"J.P30", GCF_READWRITE_PROP, "10.30"},
  {"J.P31", GCF_READWRITE_PROP, "10.31"},
  {"J.P32", GCF_READWRITE_PROP, "10.32"},
  {"J.P33", GCF_READWRITE_PROP, "10.33"},
  {"J.P34", GCF_READWRITE_PROP, "10.34"},
  {"J.P35", GCF_READWRITE_PROP, "10.35"},
  {"J.P36", GCF_READWRITE_PROP, "10.36"},
  {"J.P37", GCF_READWRITE_PROP, "10.37"},
  {"J.P38", GCF_READWRITE_PROP, "10.38"},
  {"J.P39", GCF_READWRITE_PROP, "10.39"},
  {"J.P40", GCF_READWRITE_PROP, "10.40"},
  {"J.P41", GCF_READWRITE_PROP, "10.41"},
  {"J.P42", GCF_READWRITE_PROP, "10.42"},
  {"J.P43", GCF_READWRITE_PROP, "10.43"},
  {"J.P44", GCF_READWRITE_PROP, "10.44"},
  {"J.P45", GCF_READWRITE_PROP, "10.45"},
  {"J.P46", GCF_READWRITE_PROP, "10.46"},
  {"J.P47", GCF_READWRITE_PROP, "10.47"},
  {"J.P48", GCF_READWRITE_PROP, "10.48"},
  {"J.P49", GCF_READWRITE_PROP, "10.49"},
  {"J.P50", GCF_READWRITE_PROP, "10.50"},
  {"J.P51", GCF_READWRITE_PROP, "10.51"},
  {"J.P52", GCF_READWRITE_PROP, "10.52"},
  {"J.P53", GCF_READWRITE_PROP, "10.53"},
  {"J.P54", GCF_READWRITE_PROP, "10.54"},
  {"J.P55", GCF_READWRITE_PROP, "10.55"},
  {"J.P56", GCF_READWRITE_PROP, "10.56"},
  {"J.P57", GCF_READWRITE_PROP, "10.57"},
  {"J.P58", GCF_READWRITE_PROP, "10.58"},
  {"J.P59", GCF_READWRITE_PROP, "10.59"},
  {"J.P60", GCF_READWRITE_PROP, "10.60"},
  {"J.P61", GCF_READWRITE_PROP, "10.61"},
  {"J.P62", GCF_READWRITE_PROP, "10.62"},
  {"J.P63", GCF_READWRITE_PROP, "10.63"},
  {"J.P64", GCF_READWRITE_PROP, "10.64"},
  {"J.P65", GCF_READWRITE_PROP, "10.65"},
  {"J.P66", GCF_READWRITE_PROP, "10.66"},
  {"J.P67", GCF_READWRITE_PROP, "10.67"},
  {"J.P68", GCF_READWRITE_PROP, "10.68"},
  {"J.P69", GCF_READWRITE_PROP, "10.69"},
  {"J.P70", GCF_READWRITE_PROP, "10.70"},
  {"J.P71", GCF_READWRITE_PROP, "10.71"},
  {"J.P72", GCF_READWRITE_PROP, "10.72"},
  {"J.P73", GCF_READWRITE_PROP, "10.73"},
  {"J.P74", GCF_READWRITE_PROP, "10.74"},
  {"J.P75", GCF_READWRITE_PROP, "10.75"},
  {"J.P76", GCF_READWRITE_PROP, "10.76"},
  {"J.P77", GCF_READWRITE_PROP, "10.77"},
  {"J.P78", GCF_READWRITE_PROP, "10.78"},
  {"J.P79", GCF_READWRITE_PROP, "10.79"},
  {"J.P80", GCF_READWRITE_PROP, "10.80"},
  {"J.P81", GCF_READWRITE_PROP, "10.81"},
  {"J.P82", GCF_READWRITE_PROP, "10.82"},
  {"J.P83", GCF_READWRITE_PROP, "10.83"},
  {"J.P84", GCF_READWRITE_PROP, "10.84"},
  {"J.P85", GCF_READWRITE_PROP, "10.85"},
  {"J.P86", GCF_READWRITE_PROP, "10.86"},
  {"J.P87", GCF_READWRITE_PROP, "10.87"},
  {"J.P88", GCF_READWRITE_PROP, "10.88"},
  {"J.P89", GCF_READWRITE_PROP, "10.89"},
  {"J.P90", GCF_READWRITE_PROP, "10.90"},
  {"J.P91", GCF_READWRITE_PROP, "10.91"},
  {"J.P92", GCF_READWRITE_PROP, "10.92"},
  {"J.P93", GCF_READWRITE_PROP, "10.93"},
  {"J.P94", GCF_READWRITE_PROP, "10.94"},
  {"J.P95", GCF_READWRITE_PROP, "10.95"},
  {"J.P96", GCF_READWRITE_PROP, "10.96"},
  {"J.P97", GCF_READWRITE_PROP, "10.97"},
  {"J.P98", GCF_READWRITE_PROP, "10.98"},
  {"J.P99", GCF_READWRITE_PROP, "10.99"},
};

// PropertySetD1: end

// PropertySetE1: start

const TPropertyConfig propertiesSE1[] =
{
  {"P1", GCF_READWRITE_PROP, "11"},
  {"P2", GCF_READWRITE_PROP, "25"},
  {"P3", GCF_READWRITE_PROP, "33.3"},
  {"P4", GCF_READWRITE_PROP, "11"},
  {"P5", GCF_READWRITE_PROP, "25"},
  {"P6", GCF_READWRITE_PROP, "33.3"},
};

// PropertySetE1: end
