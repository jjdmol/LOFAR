#include <GCF/GCF_Defines.h>

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



#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)
// property sets

// PropertySetA1: start

const TProperty propertiesSA1[] =
{
  {"F.P4", LPT_INTEGER, GCF_READWRITE_PROP, "10"},
  {"F.P5", LPT_CHAR, GCF_READWRITE_PROP, "20"},
  {"G.P6", LPT_DOUBLE, GCF_READWRITE_PROP, "12.1"},
  {"G.P7", LPT_STRING, GCF_READWRITE_PROP, "geit"},
};

const TPropertySet propertySetA1 = 
{
  "TTypeA", true, (sizeof(propertiesSA1)/sizeof(TProperty)), propertiesSA1
};

// PropertySetA1: end

// PropertySetB1: start

const TProperty propertiesSB1[] =
{
  {"P1", LPT_INTEGER, GCF_READWRITE_PROP, "11"},
  {"P2", LPT_CHAR, GCF_READWRITE_PROP, "25"},
  {"P3", LPT_DOUBLE, GCF_READWRITE_PROP, "33.3"},
};

const TPropertySet propertySetB1 = 
{
  "TTypeB", true, (sizeof(propertiesSB1)/sizeof(TProperty)), propertiesSB1
};

// PropertySetB1: end

// PropertySetB2: start

const TProperty propertiesSB2[] =
{
  {"P1", LPT_INTEGER, GCF_READWRITE_PROP, "12"},
  {"P2", LPT_CHAR, GCF_READWRITE_PROP, "26"},
  {"P3", LPT_DOUBLE, GCF_READWRITE_PROP, "34.3"},
};

const TPropertySet propertySetB2 = 
{
  "TTypeB", true, (sizeof(propertiesSB2)/sizeof(TProperty)), propertiesSB2
};

// PropertySetB2: end

// PropertySetB3: start

const TProperty propertiesSB3[] =
{
  {"P1", LPT_INTEGER, GCF_READWRITE_PROP, "13"},
  {"P2", LPT_CHAR, GCF_READWRITE_PROP, "27"},
  {"P3", LPT_DOUBLE, GCF_READWRITE_PROP, "35.3"},
};

const TPropertySet propertySetB3 = 
{
  "TTypeB", true, (sizeof(propertiesSB3)/sizeof(TProperty)), propertiesSB3
};

// PropertySetB3: end

// PropertySetB4: start


const TProperty propertiesSB4[] =
{
  {"P1", LPT_INTEGER, GCF_READWRITE_PROP, "14"},
  {"P2", LPT_CHAR, GCF_READWRITE_PROP, "28"},
  {"P3", LPT_DOUBLE, GCF_READWRITE_PROP, "36.3"},
};

const TPropertySet propertySetB4 = 
{
  "TTypeB", false, (sizeof(propertiesSB4)/sizeof(TProperty)), propertiesSB4
};

// PropertySetB4: end

// PropertySetC1: start

const TProperty propertiesSC1[] =
{
  {"P8", LPT_STRING, GCF_READWRITE_PROP, "aap"},
  {"P9", LPT_STRING, GCF_READWRITE_PROP, "noot"},
};

const TPropertySet propertySetC1 = 
{
  "TTypeC", true, (sizeof(propertiesSC1)/sizeof(TProperty)), propertiesSC1
};

// PropertySetC1: end

// PropertySetD1: start

const TProperty propertiesSD1[] =
{
  {"J.P00", LPT_DOUBLE, GCF_READWRITE_PROP, "10.00"},
  {"J.P01", LPT_DOUBLE, GCF_READWRITE_PROP, "10.01"},
  {"J.P02", LPT_DOUBLE, GCF_READWRITE_PROP, "10.02"},
  {"J.P03", LPT_DOUBLE, GCF_READWRITE_PROP, "10.03"},
  {"J.P04", LPT_DOUBLE, GCF_READWRITE_PROP, "10.04"},
  {"J.P05", LPT_DOUBLE, GCF_READWRITE_PROP, "10.05"},
  {"J.P06", LPT_DOUBLE, GCF_READWRITE_PROP, "10.06"},
  {"J.P07", LPT_DOUBLE, GCF_READWRITE_PROP, "10.07"},
  {"J.P08", LPT_DOUBLE, GCF_READWRITE_PROP, "10.08"},
  {"J.P09", LPT_DOUBLE, GCF_READWRITE_PROP, "10.09"},
  {"J.P10", LPT_DOUBLE, GCF_READWRITE_PROP, "10.00"},
  {"J.P11", LPT_DOUBLE, GCF_READWRITE_PROP, "10.11"},
  {"J.P12", LPT_DOUBLE, GCF_READWRITE_PROP, "10.12"},
  {"J.P13", LPT_DOUBLE, GCF_READWRITE_PROP, "10.13"},
  {"J.P14", LPT_DOUBLE, GCF_READWRITE_PROP, "10.14"},
  {"J.P15", LPT_DOUBLE, GCF_READWRITE_PROP, "10.15"},
  {"J.P16", LPT_DOUBLE, GCF_READWRITE_PROP, "10.16"},
  {"J.P17", LPT_DOUBLE, GCF_READWRITE_PROP, "10.17"},
  {"J.P18", LPT_DOUBLE, GCF_READWRITE_PROP, "10.18"},
  {"J.P19", LPT_DOUBLE, GCF_READWRITE_PROP, "10.19"},
  {"J.P20", LPT_DOUBLE, GCF_READWRITE_PROP, "10.20"},
  {"J.P21", LPT_DOUBLE, GCF_READWRITE_PROP, "10.21"},
  {"J.P22", LPT_DOUBLE, GCF_READWRITE_PROP, "10.22"},
  {"J.P23", LPT_DOUBLE, GCF_READWRITE_PROP, "10.23"},
  {"J.P24", LPT_DOUBLE, GCF_READWRITE_PROP, "10.24"},
  {"J.P25", LPT_DOUBLE, GCF_READWRITE_PROP, "10.25"},
  {"J.P26", LPT_DOUBLE, GCF_READWRITE_PROP, "10.26"},
  {"J.P27", LPT_DOUBLE, GCF_READWRITE_PROP, "10.27"},
  {"J.P28", LPT_DOUBLE, GCF_READWRITE_PROP, "10.28"},
  {"J.P29", LPT_DOUBLE, GCF_READWRITE_PROP, "10.29"},
  {"J.P30", LPT_DOUBLE, GCF_READWRITE_PROP, "10.30"},
  {"J.P31", LPT_DOUBLE, GCF_READWRITE_PROP, "10.31"},
  {"J.P32", LPT_DOUBLE, GCF_READWRITE_PROP, "10.32"},
  {"J.P33", LPT_DOUBLE, GCF_READWRITE_PROP, "10.33"},
  {"J.P34", LPT_DOUBLE, GCF_READWRITE_PROP, "10.34"},
  {"J.P35", LPT_DOUBLE, GCF_READWRITE_PROP, "10.35"},
  {"J.P36", LPT_DOUBLE, GCF_READWRITE_PROP, "10.36"},
  {"J.P37", LPT_DOUBLE, GCF_READWRITE_PROP, "10.37"},
  {"J.P38", LPT_DOUBLE, GCF_READWRITE_PROP, "10.38"},
  {"J.P39", LPT_DOUBLE, GCF_READWRITE_PROP, "10.39"},
  {"J.P40", LPT_DOUBLE, GCF_READWRITE_PROP, "10.40"},
  {"J.P41", LPT_DOUBLE, GCF_READWRITE_PROP, "10.41"},
  {"J.P42", LPT_DOUBLE, GCF_READWRITE_PROP, "10.42"},
  {"J.P43", LPT_DOUBLE, GCF_READWRITE_PROP, "10.43"},
  {"J.P44", LPT_DOUBLE, GCF_READWRITE_PROP, "10.44"},
  {"J.P45", LPT_DOUBLE, GCF_READWRITE_PROP, "10.45"},
  {"J.P46", LPT_DOUBLE, GCF_READWRITE_PROP, "10.46"},
  {"J.P47", LPT_DOUBLE, GCF_READWRITE_PROP, "10.47"},
  {"J.P48", LPT_DOUBLE, GCF_READWRITE_PROP, "10.48"},
  {"J.P49", LPT_DOUBLE, GCF_READWRITE_PROP, "10.49"},
  {"J.P50", LPT_DOUBLE, GCF_READWRITE_PROP, "10.50"},
  {"J.P51", LPT_DOUBLE, GCF_READWRITE_PROP, "10.51"},
  {"J.P52", LPT_DOUBLE, GCF_READWRITE_PROP, "10.52"},
  {"J.P53", LPT_DOUBLE, GCF_READWRITE_PROP, "10.53"},
  {"J.P54", LPT_DOUBLE, GCF_READWRITE_PROP, "10.54"},
  {"J.P55", LPT_DOUBLE, GCF_READWRITE_PROP, "10.55"},
  {"J.P56", LPT_DOUBLE, GCF_READWRITE_PROP, "10.56"},
  {"J.P57", LPT_DOUBLE, GCF_READWRITE_PROP, "10.57"},
  {"J.P58", LPT_DOUBLE, GCF_READWRITE_PROP, "10.58"},
  {"J.P59", LPT_DOUBLE, GCF_READWRITE_PROP, "10.59"},
  {"J.P60", LPT_DOUBLE, GCF_READWRITE_PROP, "10.60"},
  {"J.P61", LPT_DOUBLE, GCF_READWRITE_PROP, "10.61"},
  {"J.P62", LPT_DOUBLE, GCF_READWRITE_PROP, "10.62"},
  {"J.P63", LPT_DOUBLE, GCF_READWRITE_PROP, "10.63"},
  {"J.P64", LPT_DOUBLE, GCF_READWRITE_PROP, "10.64"},
  {"J.P65", LPT_DOUBLE, GCF_READWRITE_PROP, "10.65"},
  {"J.P66", LPT_DOUBLE, GCF_READWRITE_PROP, "10.66"},
  {"J.P67", LPT_DOUBLE, GCF_READWRITE_PROP, "10.67"},
  {"J.P68", LPT_DOUBLE, GCF_READWRITE_PROP, "10.68"},
  {"J.P69", LPT_DOUBLE, GCF_READWRITE_PROP, "10.69"},
  {"J.P70", LPT_DOUBLE, GCF_READWRITE_PROP, "10.70"},
  {"J.P71", LPT_DOUBLE, GCF_READWRITE_PROP, "10.71"},
  {"J.P72", LPT_DOUBLE, GCF_READWRITE_PROP, "10.72"},
  {"J.P73", LPT_DOUBLE, GCF_READWRITE_PROP, "10.73"},
  {"J.P74", LPT_DOUBLE, GCF_READWRITE_PROP, "10.74"},
  {"J.P75", LPT_DOUBLE, GCF_READWRITE_PROP, "10.75"},
  {"J.P76", LPT_DOUBLE, GCF_READWRITE_PROP, "10.76"},
  {"J.P77", LPT_DOUBLE, GCF_READWRITE_PROP, "10.77"},
  {"J.P78", LPT_DOUBLE, GCF_READWRITE_PROP, "10.78"},
  {"J.P79", LPT_DOUBLE, GCF_READWRITE_PROP, "10.79"},
  {"J.P80", LPT_DOUBLE, GCF_READWRITE_PROP, "10.80"},
  {"J.P81", LPT_DOUBLE, GCF_READWRITE_PROP, "10.81"},
  {"J.P82", LPT_DOUBLE, GCF_READWRITE_PROP, "10.82"},
  {"J.P83", LPT_DOUBLE, GCF_READWRITE_PROP, "10.83"},
  {"J.P84", LPT_DOUBLE, GCF_READWRITE_PROP, "10.84"},
  {"J.P85", LPT_DOUBLE, GCF_READWRITE_PROP, "10.85"},
  {"J.P86", LPT_DOUBLE, GCF_READWRITE_PROP, "10.86"},
  {"J.P87", LPT_DOUBLE, GCF_READWRITE_PROP, "10.87"},
  {"J.P88", LPT_DOUBLE, GCF_READWRITE_PROP, "10.88"},
  {"J.P89", LPT_DOUBLE, GCF_READWRITE_PROP, "10.89"},
  {"J.P90", LPT_DOUBLE, GCF_READWRITE_PROP, "10.90"},
  {"J.P91", LPT_DOUBLE, GCF_READWRITE_PROP, "10.91"},
  {"J.P92", LPT_DOUBLE, GCF_READWRITE_PROP, "10.92"},
  {"J.P93", LPT_DOUBLE, GCF_READWRITE_PROP, "10.93"},
  {"J.P94", LPT_DOUBLE, GCF_READWRITE_PROP, "10.94"},
  {"J.P95", LPT_DOUBLE, GCF_READWRITE_PROP, "10.95"},
  {"J.P96", LPT_DOUBLE, GCF_READWRITE_PROP, "10.96"},
  {"J.P97", LPT_DOUBLE, GCF_READWRITE_PROP, "10.97"},
  {"J.P98", LPT_DOUBLE, GCF_READWRITE_PROP, "10.98"},
  {"J.P99", LPT_DOUBLE, GCF_READWRITE_PROP, "10.99"},
};

const TPropertySet propertySetD1 = 
{
  "TTypeD", true, (sizeof(propertiesSD1)/sizeof(TProperty)), propertiesSD1
};

// PropertySetD1: end

// PropertySetE1: start

const TProperty propertiesSE1[] =
{
  {"P1", LPT_CHAR, GCF_READWRITE_PROP, "11"},
  {"P2", LPT_DOUBLE, GCF_READWRITE_PROP, "25"},
  {"P3", LPT_STRING, GCF_READWRITE_PROP, "33.3"},
  {"P4", LPT_DOUBLE, GCF_READWRITE_PROP, "11"},
  {"P5", LPT_INTEGER, GCF_READWRITE_PROP, "25"},
  {"P6", LPT_UNSIGNED, GCF_READWRITE_PROP, "33.3"},
};

const TPropertySet propertySetE1 = 
{
  "TTypeE", false, (sizeof(propertiesSE1)/sizeof(TProperty)), propertiesSE1
};

// PropertySetE1: end
