#include <GCFCommon/GCF_Defines.h>
#include <SAL/GCF_PValue.h>

//#define WAIT_FOR_INPUT

// property sets

// PropertySetA1: start

const TProperty propertiesSA1[] =
{
  {"F_P4", GCFPValue::INTEGER_VAL, 0, "10"},
  {"F_P5", GCFPValue::CHAR_VAL, 0, "20"},
  {"G_P6", GCFPValue::DOUBLE_VAL, 0, "12.1"},
  {"G_P7", GCFPValue::STRING_VAL, 0, "geit"},
};

const TPropertySet propertySetA1 = 
{
  4, "A_B", propertiesSA1
};

// PropertySetA1: end

// PropertySetB1: start

const TProperty propertiesSB1[] =
{
  {"P1", GCFPValue::INTEGER_VAL, 0, "11"},
  {"P2", GCFPValue::CHAR_VAL, 0, "25"},
  {"P3", GCFPValue::DOUBLE_VAL, 0, "33.3"},
};

const TPropertySet propertySetB1 = 
{
  3, "A_C", propertiesSB1
};

// PropertySetB1: end

// PropertySetB2: start

const TProperty propertiesSB2[] =
{
  {"P1", GCFPValue::INTEGER_VAL, 0, "12"},
  {"P2", GCFPValue::CHAR_VAL, 0, "26"},
  {"P3", GCFPValue::DOUBLE_VAL, 0, "34.3"},
};

const TPropertySet propertySetB2 = 
{
  3, "A_D", propertiesSB2
};

// PropertySetB2: end

// PropertySetB3: start

const TProperty propertiesSB3[] =
{
  {"P1", GCFPValue::INTEGER_VAL, 0, "13"},
  {"P2", GCFPValue::CHAR_VAL, 0, "27"},
  {"P3", GCFPValue::DOUBLE_VAL, 0, "35.3"},
};

const TPropertySet propertySetB3 = 
{
  3, "A_E", propertiesSB3
};

// PropertySetB3: end

// PropertySetB4: start


const TProperty propertiesSB4[] =
{
  {"P1", GCFPValue::INTEGER_VAL, 0, "14"},
  {"P2", GCFPValue::CHAR_VAL, 0, "28"},
  {"P3", GCFPValue::DOUBLE_VAL, 0, "36.3"},
};

const TPropertySet propertySetB4 = 
{
  3, "A_K", propertiesSB4
};

// PropertySetB4: end

// PropertySetC1: start

const TProperty propertiesSC1[] =
{
  {"P8", GCFPValue::STRING_VAL, 0, "aap"},
  {"P9", GCFPValue::STRING_VAL, 0, "noot"},
};

const TPropertySet propertySetC1 = 
{
  2, "A_H_I", propertiesSC1
};

// PropertySetC1: end

// PropertySetD1: start

const TProperty propertiesSD1[] =
{
  {"J_P00", GCFPValue::DOUBLE_VAL, 0, "10.00"},
  {"J_P01", GCFPValue::DOUBLE_VAL, 0, "10.01"},
  {"J_P02", GCFPValue::DOUBLE_VAL, 0, "10.02"},
  {"J_P03", GCFPValue::DOUBLE_VAL, 0, "10.03"},
  {"J_P04", GCFPValue::DOUBLE_VAL, 0, "10.04"},
  {"J_P05", GCFPValue::DOUBLE_VAL, 0, "10.05"},
  {"J_P06", GCFPValue::DOUBLE_VAL, 0, "10.06"},
  {"J_P07", GCFPValue::DOUBLE_VAL, 0, "10.07"},
  {"J_P08", GCFPValue::DOUBLE_VAL, 0, "10.08"},
  {"J_P09", GCFPValue::DOUBLE_VAL, 0, "10.09"},
  {"J_P10", GCFPValue::DOUBLE_VAL, 0, "10.00"},
  {"J_P11", GCFPValue::DOUBLE_VAL, 0, "10.11"},
  {"J_P12", GCFPValue::DOUBLE_VAL, 0, "10.12"},
  {"J_P13", GCFPValue::DOUBLE_VAL, 0, "10.13"},
  {"J_P14", GCFPValue::DOUBLE_VAL, 0, "10.14"},
  {"J_P15", GCFPValue::DOUBLE_VAL, 0, "10.15"},
  {"J_P16", GCFPValue::DOUBLE_VAL, 0, "10.16"},
  {"J_P17", GCFPValue::DOUBLE_VAL, 0, "10.17"},
  {"J_P18", GCFPValue::DOUBLE_VAL, 0, "10.18"},
  {"J_P19", GCFPValue::DOUBLE_VAL, 0, "10.19"},
  {"J_P20", GCFPValue::DOUBLE_VAL, 0, "10.20"},
  {"J_P21", GCFPValue::DOUBLE_VAL, 0, "10.21"},
  {"J_P22", GCFPValue::DOUBLE_VAL, 0, "10.22"},
  {"J_P23", GCFPValue::DOUBLE_VAL, 0, "10.23"},
  {"J_P24", GCFPValue::DOUBLE_VAL, 0, "10.24"},
  {"J_P25", GCFPValue::DOUBLE_VAL, 0, "10.25"},
  {"J_P26", GCFPValue::DOUBLE_VAL, 0, "10.26"},
  {"J_P27", GCFPValue::DOUBLE_VAL, 0, "10.27"},
  {"J_P28", GCFPValue::DOUBLE_VAL, 0, "10.28"},
  {"J_P29", GCFPValue::DOUBLE_VAL, 0, "10.29"},
  {"J_P30", GCFPValue::DOUBLE_VAL, 0, "10.30"},
  {"J_P31", GCFPValue::DOUBLE_VAL, 0, "10.31"},
  {"J_P32", GCFPValue::DOUBLE_VAL, 0, "10.32"},
  {"J_P33", GCFPValue::DOUBLE_VAL, 0, "10.33"},
  {"J_P34", GCFPValue::DOUBLE_VAL, 0, "10.34"},
  {"J_P35", GCFPValue::DOUBLE_VAL, 0, "10.35"},
  {"J_P36", GCFPValue::DOUBLE_VAL, 0, "10.36"},
  {"J_P37", GCFPValue::DOUBLE_VAL, 0, "10.37"},
  {"J_P38", GCFPValue::DOUBLE_VAL, 0, "10.38"},
  {"J_P39", GCFPValue::DOUBLE_VAL, 0, "10.39"},
  {"J_P40", GCFPValue::DOUBLE_VAL, 0, "10.40"},
  {"J_P41", GCFPValue::DOUBLE_VAL, 0, "10.41"},
  {"J_P42", GCFPValue::DOUBLE_VAL, 0, "10.42"},
  {"J_P43", GCFPValue::DOUBLE_VAL, 0, "10.43"},
  {"J_P44", GCFPValue::DOUBLE_VAL, 0, "10.44"},
  {"J_P45", GCFPValue::DOUBLE_VAL, 0, "10.45"},
  {"J_P46", GCFPValue::DOUBLE_VAL, 0, "10.46"},
  {"J_P47", GCFPValue::DOUBLE_VAL, 0, "10.47"},
  {"J_P48", GCFPValue::DOUBLE_VAL, 0, "10.48"},
  {"J_P49", GCFPValue::DOUBLE_VAL, 0, "10.49"},
  {"J_P50", GCFPValue::DOUBLE_VAL, 0, "10.50"},
  {"J_P51", GCFPValue::DOUBLE_VAL, 0, "10.51"},
  {"J_P52", GCFPValue::DOUBLE_VAL, 0, "10.52"},
  {"J_P53", GCFPValue::DOUBLE_VAL, 0, "10.53"},
  {"J_P54", GCFPValue::DOUBLE_VAL, 0, "10.54"},
  {"J_P55", GCFPValue::DOUBLE_VAL, 0, "10.55"},
  {"J_P56", GCFPValue::DOUBLE_VAL, 0, "10.56"},
  {"J_P57", GCFPValue::DOUBLE_VAL, 0, "10.57"},
  {"J_P58", GCFPValue::DOUBLE_VAL, 0, "10.58"},
  {"J_P59", GCFPValue::DOUBLE_VAL, 0, "10.59"},
  {"J_P60", GCFPValue::DOUBLE_VAL, 0, "10.60"},
  {"J_P61", GCFPValue::DOUBLE_VAL, 0, "10.61"},
  {"J_P62", GCFPValue::DOUBLE_VAL, 0, "10.62"},
  {"J_P63", GCFPValue::DOUBLE_VAL, 0, "10.63"},
  {"J_P64", GCFPValue::DOUBLE_VAL, 0, "10.64"},
  {"J_P65", GCFPValue::DOUBLE_VAL, 0, "10.65"},
  {"J_P66", GCFPValue::DOUBLE_VAL, 0, "10.66"},
  {"J_P67", GCFPValue::DOUBLE_VAL, 0, "10.67"},
  {"J_P68", GCFPValue::DOUBLE_VAL, 0, "10.68"},
  {"J_P69", GCFPValue::DOUBLE_VAL, 0, "10.69"},
  {"J_P70", GCFPValue::DOUBLE_VAL, 0, "10.70"},
  {"J_P71", GCFPValue::DOUBLE_VAL, 0, "10.71"},
  {"J_P72", GCFPValue::DOUBLE_VAL, 0, "10.72"},
  {"J_P73", GCFPValue::DOUBLE_VAL, 0, "10.73"},
  {"J_P74", GCFPValue::DOUBLE_VAL, 0, "10.74"},
  {"J_P75", GCFPValue::DOUBLE_VAL, 0, "10.75"},
  {"J_P76", GCFPValue::DOUBLE_VAL, 0, "10.76"},
  {"J_P77", GCFPValue::DOUBLE_VAL, 0, "10.77"},
  {"J_P78", GCFPValue::DOUBLE_VAL, 0, "10.78"},
  {"J_P79", GCFPValue::DOUBLE_VAL, 0, "10.79"},
  {"J_P80", GCFPValue::DOUBLE_VAL, 0, "10.80"},
  {"J_P81", GCFPValue::DOUBLE_VAL, 0, "10.81"},
  {"J_P82", GCFPValue::DOUBLE_VAL, 0, "10.82"},
  {"J_P83", GCFPValue::DOUBLE_VAL, 0, "10.83"},
  {"J_P84", GCFPValue::DOUBLE_VAL, 0, "10.84"},
  {"J_P85", GCFPValue::DOUBLE_VAL, 0, "10.85"},
  {"J_P86", GCFPValue::DOUBLE_VAL, 0, "10.86"},
  {"J_P87", GCFPValue::DOUBLE_VAL, 0, "10.87"},
  {"J_P88", GCFPValue::DOUBLE_VAL, 0, "10.88"},
  {"J_P89", GCFPValue::DOUBLE_VAL, 0, "10.89"},
  {"J_P90", GCFPValue::DOUBLE_VAL, 0, "10.90"},
  {"J_P91", GCFPValue::DOUBLE_VAL, 0, "10.91"},
  {"J_P92", GCFPValue::DOUBLE_VAL, 0, "10.92"},
  {"J_P93", GCFPValue::DOUBLE_VAL, 0, "10.93"},
  {"J_P94", GCFPValue::DOUBLE_VAL, 0, "10.94"},
  {"J_P95", GCFPValue::DOUBLE_VAL, 0, "10.95"},
  {"J_P96", GCFPValue::DOUBLE_VAL, 0, "10.96"},
  {"J_P97", GCFPValue::DOUBLE_VAL, 0, "10.97"},
  {"J_P98", GCFPValue::DOUBLE_VAL, 0, "10.98"},
  {"J_P99", GCFPValue::DOUBLE_VAL, 0, "10.99"},
};

const TPropertySet propertySetD1 = 
{
  100, "A_H", propertiesSD1
};

// PropertySetD1: end

// PropertySetE1: start

const TProperty propertiesSE1[] =
{
  {"P1", GCFPValue::CHAR_VAL, 0, "11"},
  {"P2", GCFPValue::DOUBLE_VAL, 0, "25"},
  {"P3", GCFPValue::STRING_VAL, 0, "33.3"},
  {"P4", GCFPValue::DOUBLE_VAL, 0, "11"},
  {"P5", GCFPValue::INTEGER_VAL, 0, "25"},
  {"P6", GCFPValue::UNSIGNED_VAL, 0, "33.3"},
};

const TPropertySet propertySetE1 = 
{
  6, "A_L", propertiesSE1
};

// PropertySetA1: end
