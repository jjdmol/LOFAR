//#  WH_PPH.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APS/ParameterSet.h>
#include <WH_PPF.h>

#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>
#include <TFC_Interface/TFC_Config.h>

#include <Common/Timer.h>

#ifdef HAVE_BGL
#include <hummer_builtin.h>
#endif

#include <fftw.h>
#include <assert.h>
#include <complex.h>

using namespace LOFAR;


FIR::FIR()
{
  memset(itsDelayLine, 0, sizeof itsDelayLine);
}


const float FIR::weights[NR_SUB_CHANNELS][NR_TAPS] __attribute__((aligned(32))) = {
  {    22,   399,  -384,   179,    19,  -191,   320,  -380,
    32767,  -503,   377,  -226,    40,   166,  -377,   397 },
  {    22,   401,  -391,   193,    -3,  -157,   262,  -255,
    32766,  -625,   435,  -260,    62,   152,  -369,   395 },
  {    21,   402,  -399,   207,   -25,  -123,   204,  -130,
    32762,  -746,   491,  -293,    84,   138,  -362,   393 },
  {    20,   404,  -406,   221,   -47,   -88,   146,    -4,
    32757,  -866,   548,  -327,   105,   124,  -354,   391 },
  {    19,   406,  -414,   235,   -69,   -53,    87,   124,
    32750,  -985,   604,  -360,   126,   111,  -347,   389 },
  {    18,   408,  -421,   248,   -91,   -19,    28,   252,
    32742, -1103,   660,  -394,   148,    97,  -339,   387 },
  {    16,   410,  -429,   262,  -113,    16,   -31,   382,
    32732, -1219,   715,  -427,   169,    84,  -332,   385 },
  {    15,   412,  -436,   276,  -136,    51,   -90,   513,
    32720, -1335,   770,  -460,   190,    70,  -324,   383 },
  {    14,   413,  -443,   290,  -158,    87,  -150,   644,
    32706, -1449,   825,  -493,   211,    57,  -317,   381 },
  {    13,   415,  -451,   304,  -180,   122,  -210,   777,
    32690, -1562,   879,  -525,   232,    43,  -309,   379 },
  {    12,   417,  -458,   318,  -203,   158,  -270,   910,
    32673, -1675,   933,  -557,   253,    30,  -302,   377 },
  {    11,   419,  -465,   332,  -225,   193,  -331,  1045,
    32654, -1786,   987,  -590,   274,    16,  -294,   375 },
  {    11,   420,  -473,   346,  -248,   229,  -392,  1180,
    32634, -1896,  1040,  -622,   294,     3,  -287,   373 },
  {    11,   422,  -480,   360,  -270,   265,  -453,  1317,
    32612, -2004,  1093,  -653,   315,   -10,  -279,   370 },
  {    11,   424,  -487,   374,  -293,   300,  -514,  1454,
    32588, -2112,  1145,  -685,   335,   -24,  -272,   368 },
  {    12,   425,  -494,   388,  -315,   336,  -575,  1593,
    32562, -2218,  1197,  -716,   355,   -37,  -264,   366 },
  {    12,   427,  -502,   402,  -338,   372,  -637,  1732,
    32535, -2324,  1248,  -747,   375,   -50,  -257,   364 },
  {    13,   428,  -509,   416,  -361,   408,  -699,  1873,
    32506, -2428,  1299,  -778,   395,   -63,  -250,   362 },
  {    13,   430,  -516,   430,  -383,   445,  -761,  2014,
    32475, -2530,  1350,  -809,   415,   -76,  -242,   360 },
  {    14,   432,  -523,   444,  -406,   481,  -823,  2156,
    32443, -2632,  1400,  -839,   435,   -89,  -235,   358 },
  {    15,   433,  -530,   458,  -429,   517,  -885,  2299,
    32409, -2733,  1450,  -869,   455,  -102,  -227,   355 },
  {    15,   435,  -537,   472,  -452,   554,  -947,  2443,
    32373, -2832,  1499,  -899,   474,  -115,  -220,   353 },
  {    16,   436,  -544,   486,  -474,   590, -1010,  2588,
    32335, -2930,  1548,  -929,   494,  -127,  -212,   351 },
  {    16,   438,  -551,   499,  -497,   626, -1073,  2734,
    32296, -3027,  1597,  -958,   513,  -140,  -205,   349 },
  {    17,   439,  -558,   513,  -520,   663, -1136,  2881,
    32256, -3123,  1645,  -987,   532,  -153,  -198,   347 },
  {    17,   440,  -565,   527,  -543,   700, -1199,  3028,
    32213, -3218,  1692, -1016,   551,  -165,  -190,   344 },
  {    18,   442,  -572,   541,  -565,   736, -1262,  3177,
    32169, -3311,  1739, -1045,   570,  -178,  -183,   342 },
  {    18,   443,  -579,   555,  -588,   773, -1325,  3326,
    32123, -3403,  1786, -1073,   588,  -190,  -176,   340 },
  {    18,   444,  -586,   569,  -611,   809, -1388,  3476,
    32076, -3494,  1832, -1102,   607,  -203,  -168,   338 },
  {    19,   446,  -593,   582,  -634,   846, -1452,  3627,
    32027, -3584,  1877, -1129,   625,  -215,  -161,   335 },
  {    19,   447,  -599,   596,  -656,   883, -1515,  3779,
    31976, -3672,  1923, -1157,   643,  -227,  -154,   333 },
  {    20,   448,  -606,   610,  -679,   919, -1579,  3931,
    31924, -3760,  1967, -1184,   661,  -239,  -147,   331 },
  {    20,   449,  -613,   624,  -702,   956, -1642,  4084,
    31870, -3846,  2011, -1211,   679,  -251,  -139,   329 },
  {    21,   450,  -620,   637,  -725,   992, -1706,  4239,
    31814, -3931,  2055, -1238,   697,  -263,  -132,   326 },
  {    22,   452,  -626,   651,  -747,  1029, -1770,  4393,
    31757, -4014,  2098, -1265,   715,  -275,  -125,   324 },
  {    22,   453,  -633,   664,  -770,  1066, -1833,  4549,
    31698, -4097,  2141, -1291,   732,  -287,  -118,   322 },
  {    23,   454,  -639,   678,  -792,  1102, -1897,  4705,
    31638, -4178,  2183, -1317,   749,  -299,  -111,   320 },
  {    24,   455,  -646,   691,  -815,  1139, -1961,  4863,
    31576, -4258,  2224, -1342,   766,  -311,  -103,   317 },
  {    24,   456,  -652,   705,  -838,  1175, -2025,  5020,
    31512, -4336,  2265, -1368,   783,  -322,   -96,   315 },
  {    25,   457,  -659,   718,  -860,  1212, -2089,  5179,
    31447, -4414,  2306, -1393,   800,  -334,   -89,   313 },
  {    25,   458,  -665,   732,  -882,  1248, -2152,  5338,
    31380, -4490,  2346, -1418,   817,  -345,   -82,   310 },
  {    26,   459,  -671,   745,  -905,  1284, -2216,  5498,
    31311, -4565,  2385, -1442,   833,  -357,   -75,   308 },
  {    27,   460,  -678,   758,  -927,  1321, -2280,  5659,
    31241, -4639,  2424, -1466,   849,  -368,   -68,   306 },
  {    27,   460,  -684,   771,  -949,  1357, -2343,  5820,
    31170, -4711,  2462, -1490,   865,  -379,   -61,   303 },
  {    28,   461,  -690,   784,  -972,  1393, -2407,  5982,
    31097, -4782,  2500, -1514,   881,  -390,   -54,   301 },
  {    29,   462,  -696,   797,  -994,  1429, -2471,  6145,
    31022, -4852,  2537, -1537,   897,  -401,   -47,   299 },
  {    29,   463,  -702,   810, -1016,  1465, -2534,  6308,
    30946, -4921,  2574, -1560,   912,  -412,   -40,   297 },
  {    30,   463,  -708,   823, -1038,  1501, -2598,  6472,
    30868, -4989,  2610, -1582,   928,  -423,   -34,   294 },
  {    31,   464,  -714,   836, -1060,  1537, -2661,  6636,
    30789, -5055,  2646, -1605,   943,  -433,   -27,   292 },
  {    31,   465,  -720,   849, -1082,  1573, -2724,  6801,
    30708, -5120,  2681, -1627,   958,  -444,   -20,   290 },
  {    32,   465,  -726,   862, -1103,  1608, -2787,  6967,
    30625, -5184,  2715, -1648,   972,  -455,   -13,   287 },
  {    33,   466,  -731,   874, -1125,  1644, -2850,  7133,
    30542, -5246,  2749, -1669,   987,  -465,    -6,   285 },
  {    34,   467,  -737,   887, -1147,  1679, -2913,  7300,
    30456, -5307,  2783, -1690,  1001,  -475,     0,   283 },
  {    34,   467,  -743,   900, -1168,  1715, -2976,  7467,
    30369, -5367,  2815, -1711,  1016,  -486,     7,   280 },
  {    35,   468,  -748,   912, -1189,  1750, -3038,  7635,
    30281, -5426,  2848, -1731,  1030,  -496,    14,   278 },
  {    36,   468,  -754,   924, -1211,  1785, -3101,  7803,
    30191, -5484,  2879, -1751,  1043,  -506,    20,   276 },
  {    37,   468,  -759,   937, -1232,  1820, -3163,  7972,
    30100, -5540,  2910, -1771,  1057,  -516,    27,   273 },
  {    38,   469,  -765,   949, -1253,  1854, -3225,  8141,
    30007, -5595,  2941, -1790,  1070,  -525,    33,   271 },
  {    38,   469,  -770,   961, -1274,  1889, -3287,  8311,
    29913, -5649,  2971, -1809,  1084,  -535,    40,   269 },
  {    39,   469,  -775,   973, -1295,  1923, -3349,  8481,
    29817, -5701,  3000, -1828,  1097,  -545,    46,   267 },
  {    40,   470,  -781,   985, -1316,  1958, -3410,  8652,
    29720, -5753,  3029, -1846,  1110,  -554,    53,   264 },
  {    41,   470,  -786,   997, -1336,  1992, -3471,  8823,
    29622, -5803,  3057, -1864,  1122,  -564,    59,   262 },
  {    42,   470,  -791,  1008, -1357,  2026, -3532,  8995,
    29522, -5852,  3084, -1882,  1135,  -573,    65,   260 },
  {    43,   470,  -796,  1020, -1377,  2059, -3593,  9167,
    29420, -5899,  3111, -1899,  1147,  -582,    72,   257 },
  {    43,   470,  -801,  1032, -1397,  2093, -3654,  9339,
    29318, -5946,  3138, -1916,  1159,  -591,    78,   255 },
  {    44,   470,  -805,  1043, -1417,  2126, -3714,  9512,
    29214, -5991,  3163, -1933,  1171,  -600,    84,   253 },
  {    45,   470,  -810,  1054, -1437,  2160, -3774,  9685,
    29108, -6035,  3189, -1949,  1182,  -609,    90,   251 },
  {    46,   470,  -815,  1066, -1457,  2193, -3834,  9859,
    29001, -6077,  3213, -1965,  1194,  -618,    97,   248 },
  {    47,   470,  -819,  1077, -1477,  2225, -3893, 10033,
    28893, -6119,  3237, -1981,  1205,  -626,   103,   246 },
  {    48,   470,  -824,  1088, -1496,  2258, -3952, 10207,
    28784, -6159,  3261, -1996,  1216,  -635,   109,   244 },
  {    49,   470,  -828,  1099, -1516,  2290, -4011, 10381,
    28673, -6198,  3283, -2011,  1227,  -643,   115,   241 },
  {    50,   469,  -833,  1110, -1535,  2322, -4070, 10556,
    28560, -6236,  3306, -2025,  1237,  -652,   121,   239 },
  {    51,   469,  -837,  1120, -1554,  2354, -4128, 10731,
    28447, -6272,  3327, -2039,  1247,  -660,   127,   237 },
  {    52,   469,  -841,  1131, -1573,  2386, -4186, 10906,
    28332, -6308,  3348, -2053,  1258,  -668,   132,   235 },
  {    53,   468,  -845,  1141, -1592,  2417, -4243, 11082,
    28216, -6342,  3368, -2067,  1267,  -676,   138,   233 },
  {    54,   468,  -850,  1152, -1610,  2448, -4300, 11258,
    28099, -6375,  3388, -2080,  1277,  -684,   144,   230 },
  {    55,   468,  -854,  1162, -1629,  2479, -4357, 11434,
    27980, -6407,  3407, -2092,  1287,  -691,   150,   228 },
  {    56,   467,  -857,  1172, -1647,  2510, -4413, 11610,
    27860, -6437,  3426, -2105,  1296,  -699,   156,   226 },
  {    57,   467,  -861,  1182, -1665,  2540, -4469, 11787,
    27739, -6466,  3444, -2117,  1305,  -707,   161,   224 },
  {    58,   466,  -865,  1192, -1683,  2570, -4525, 11963,
    27616, -6495,  3461, -2128,  1314,  -714,   167,   221 },
  {    59,   465,  -869,  1202, -1700,  2600, -4580, 12140,
    27493, -6522,  3478, -2140,  1322,  -721,   172,   219 },
  {    60,   465,  -872,  1211, -1718,  2630, -4635, 12317,
    27368, -6547,  3494, -2151,  1331,  -728,   178,   217 },
  {    62,   464,  -876,  1221, -1735,  2659, -4689, 12494,
    27242, -6572,  3510, -2161,  1339,  -735,   183,   215 },
  {    63,   463,  -879,  1230, -1752,  2688, -4743, 12672,
    27115, -6595,  3525, -2171,  1347,  -742,   189,   213 },
  {    64,   462,  -882,  1239, -1769,  2717, -4796, 12849,
    26986, -6618,  3539, -2181,  1355,  -749,   194,   211 },
  {    65,   462,  -885,  1248, -1786,  2745, -4849, 13026,
    26856, -6639,  3553, -2191,  1362,  -756,   200,   208 },
  {    66,   461,  -888,  1257, -1802,  2773, -4901, 13204,
    26726, -6659,  3566, -2200,  1370,  -762,   205,   206 },
  {    67,   460,  -891,  1266, -1819,  2801, -4953, 13382,
    26594, -6677,  3579, -2209,  1377,  -769,   210,   204 },
  {    69,   459,  -894,  1274, -1835,  2828, -5005, 13559,
    26461, -6695,  3591, -2217,  1384,  -775,   215,   202 },
  {    70,   458,  -897,  1283, -1851,  2855, -5056, 13737,
    26327, -6711,  3602, -2225,  1390,  -781,   220,   200 },
  {    71,   457,  -900,  1291, -1866,  2882, -5106, 13915,
    26191, -6727,  3613, -2233,  1397,  -787,   226,   198 },
  {    72,   455,  -902,  1300, -1882,  2909, -5156, 14092,
    26055, -6741,  3623, -2240,  1403,  -793,   231,   196 },
  {    73,   454,  -905,  1308, -1897,  2935, -5205, 14270,
    25918, -6754,  3632, -2247,  1409,  -799,   236,   193 },
  {    75,   453,  -907,  1315, -1912,  2960, -5254, 14448,
    25779, -6766,  3641, -2254,  1415,  -805,   240,   191 },
  {    76,   452,  -910,  1323, -1927,  2986, -5302, 14625,
    25640, -6776,  3650, -2260,  1420,  -810,   245,   189 },
  {    77,   450,  -912,  1331, -1941,  3011, -5350, 14803,
    25499, -6786,  3658, -2266,  1426,  -816,   250,   187 },
  {    78,   449,  -914,  1338, -1956,  3036, -5397, 14980,
    25357, -6795,  3665, -2272,  1431,  -821,   255,   185 },
  {    80,   448,  -916,  1345, -1970,  3060, -5444, 15158,
    25215, -6802,  3671, -2277,  1436,  -826,   260,   183 },
  {    81,   446,  -918,  1353, -1983,  3084, -5490, 15335,
    25071, -6808,  3677, -2282,  1441,  -831,   264,   181 },
  {    82,   445,  -920,  1359, -1997,  3107, -5535, 15512,
    24927, -6814,  3683, -2286,  1445,  -836,   269,   179 },
  {    84,   443,  -922,  1366, -2010,  3131, -5580, 15690,
    24781, -6818,  3688, -2290,  1449,  -841,   274,   177 },
  {    85,   441,  -923,  1373, -2023,  3153, -5624, 15866,
    24634, -6821,  3692, -2294,  1453,  -846,   278,   175 },
  {    86,   440,  -925,  1379, -2036,  3176, -5667, 16043,
    24487, -6823,  3696, -2298,  1457,  -850,   283,   173 },
  {    88,   438,  -926,  1386, -2049,  3198, -5710, 16220,
    24338, -6824,  3699, -2301,  1461,  -855,   287,   171 },
  {    89,   436,  -927,  1392, -2061,  3219, -5752, 16396,
    24189, -6824,  3701, -2303,  1464,  -859,   291,   169 },
  {    91,   434,  -929,  1398, -2073,  3241, -5794, 16572,
    24039, -6822,  3703, -2306,  1467,  -863,   296,   167 },
  {    92,   432,  -930,  1403, -2085,  3261, -5835, 16748,
    23888, -6820,  3704, -2308,  1470,  -867,   300,   165 },
  {    93,   430,  -931,  1409, -2096,  3282, -5875, 16924,
    23736, -6817,  3705, -2310,  1473,  -871,   304,   163 },
  {    95,   428,  -931,  1414, -2107,  3302, -5914, 17100,
    23583, -6812,  3705, -2311,  1476,  -875,   308,   161 },
  {    96,   426,  -932,  1420, -2118,  3321, -5953, 17275,
    23429, -6807,  3705, -2312,  1478,  -879,   312,   159 },
  {    98,   424,  -933,  1425, -2129,  3340, -5991, 17450,
    23274, -6801,  3704, -2313,  1480,  -883,   316,   157 },
  {    99,   422,  -933,  1430, -2140,  3359, -6028, 17624,
    23119, -6793,  3703, -2313,  1482,  -886,   320,   155 },
  {   101,   420,  -934,  1434, -2150,  3377, -6065, 17799,
    22963, -6785,  3701, -2313,  1484,  -889,   324,   154 },
  {   102,   418,  -934,  1439, -2160,  3395, -6100, 17973,
    22806, -6775,  3698, -2312,  1485,  -893,   328,   152 },
  {   104,   415,  -934,  1443, -2169,  3412, -6135, 18146,
    22648, -6765,  3695, -2312,  1486,  -896,   332,   150 },
  {   105,   413,  -934,  1447, -2178,  3429, -6170, 18320,
    22489, -6753,  3691, -2311,  1488,  -899,   336,   148 },
  {   107,   410,  -934,  1451, -2187,  3445, -6203, 18492,
    22330, -6741,  3687, -2309,  1488,  -902,   340,   146 },
  {   108,   408,  -934,  1455, -2196,  3461, -6236, 18665,
    22170, -6728,  3682, -2307,  1489,  -904,   343,   144 },
  {   110,   405,  -934,  1459, -2204,  3477, -6268, 18837,
    22009, -6713,  3677, -2305,  1489,  -907,   347,   142 },
  {   112,   403,  -934,  1462, -2213,  3492, -6299, 19009,
    21848, -6698,  3671, -2303,  1490,  -910,   350,   141 },
  {   113,   400,  -933,  1465, -2220,  3506, -6329, 19180,
    21686, -6682,  3665, -2300,  1490,  -912,   354,   139 },
  {   115,   397,  -933,  1468, -2228,  3520, -6359, 19351,
    21523, -6665,  3658, -2297,  1489,  -914,   357,   137 },
  {   116,   395,  -932,  1471, -2235,  3534, -6388, 19521,
    21359, -6646,  3650, -2294,  1489,  -916,   361,   135 },
  {   118,   392,  -931,  1474, -2242,  3547, -6416, 19691,
    21195, -6627,  3642, -2290,  1488,  -918,   364,   133 },
  {   120,   389,  -930,  1476, -2249,  3560, -6443, 19860,
    21030, -6607,  3634, -2286,  1488,  -920,   367,   132 },
  {   121,   386,  -929,  1478, -2255,  3572, -6469, 20029,
    20865, -6587,  3625, -2282,  1487,  -922,   371,   130 },
  {   123,   383,  -928,  1480, -2261,  3583, -6494, 20197,
    20699, -6565,  3615, -2277,  1485,  -924,   374,   128 },
  {   125,   380,  -927,  1482, -2266,  3594, -6518, 20365,
    20532, -6542,  3605, -2272,  1484,  -925,   377,   126 },
  {   126,   377,  -925,  1484, -2272,  3605, -6542, 20532,
    20365, -6518,  3594, -2266,  1482,  -927,   380,   125 },
  {   128,   374,  -924,  1485, -2277,  3615, -6565, 20699,
    20197, -6494,  3583, -2261,  1480,  -928,   383,   123 },
  {   130,   371,  -922,  1487, -2282,  3625, -6587, 20865,
    20029, -6469,  3572, -2255,  1478,  -929,   386,   121 },
  {   132,   367,  -920,  1488, -2286,  3634, -6607, 21030,
    19860, -6443,  3560, -2249,  1476,  -930,   389,   120 },
  {   133,   364,  -918,  1488, -2290,  3642, -6627, 21195,
    19691, -6416,  3547, -2242,  1474,  -931,   392,   118 },
  {   135,   361,  -916,  1489, -2294,  3650, -6646, 21359,
    19521, -6388,  3534, -2235,  1471,  -932,   395,   116 },
  {   137,   357,  -914,  1489, -2297,  3658, -6665, 21523,
    19351, -6359,  3520, -2228,  1468,  -933,   397,   115 },
  {   139,   354,  -912,  1490, -2300,  3665, -6682, 21686,
    19180, -6329,  3506, -2220,  1465,  -933,   400,   113 },
  {   141,   350,  -910,  1490, -2303,  3671, -6698, 21848,
    19009, -6299,  3492, -2213,  1462,  -934,   403,   112 },
  {   142,   347,  -907,  1489, -2305,  3677, -6713, 22009,
    18837, -6268,  3477, -2204,  1459,  -934,   405,   110 },
  {   144,   343,  -904,  1489, -2307,  3682, -6728, 22170,
    18665, -6236,  3461, -2196,  1455,  -934,   408,   108 },
  {   146,   340,  -902,  1488, -2309,  3687, -6741, 22330,
    18492, -6203,  3445, -2187,  1451,  -934,   410,   107 },
  {   148,   336,  -899,  1488, -2311,  3691, -6753, 22489,
    18320, -6170,  3429, -2178,  1447,  -934,   413,   105 },
  {   150,   332,  -896,  1486, -2312,  3695, -6765, 22648,
    18146, -6135,  3412, -2169,  1443,  -934,   415,   104 },
  {   152,   328,  -893,  1485, -2312,  3698, -6775, 22806,
    17973, -6100,  3395, -2160,  1439,  -934,   418,   102 },
  {   154,   324,  -889,  1484, -2313,  3701, -6785, 22963,
    17799, -6065,  3377, -2150,  1434,  -934,   420,   101 },
  {   155,   320,  -886,  1482, -2313,  3703, -6793, 23119,
    17624, -6028,  3359, -2140,  1430,  -933,   422,    99 },
  {   157,   316,  -883,  1480, -2313,  3704, -6801, 23274,
    17450, -5991,  3340, -2129,  1425,  -933,   424,    98 },
  {   159,   312,  -879,  1478, -2312,  3705, -6807, 23429,
    17275, -5953,  3321, -2118,  1420,  -932,   426,    96 },
  {   161,   308,  -875,  1476, -2311,  3705, -6812, 23583,
    17100, -5914,  3302, -2107,  1414,  -931,   428,    95 },
  {   163,   304,  -871,  1473, -2310,  3705, -6817, 23736,
    16924, -5875,  3282, -2096,  1409,  -931,   430,    93 },
  {   165,   300,  -867,  1470, -2308,  3704, -6820, 23888,
    16748, -5835,  3261, -2085,  1403,  -930,   432,    92 },
  {   167,   296,  -863,  1467, -2306,  3703, -6822, 24039,
    16572, -5794,  3241, -2073,  1398,  -929,   434,    91 },
  {   169,   291,  -859,  1464, -2303,  3701, -6824, 24189,
    16396, -5752,  3219, -2061,  1392,  -927,   436,    89 },
  {   171,   287,  -855,  1461, -2301,  3699, -6824, 24338,
    16220, -5710,  3198, -2049,  1386,  -926,   438,    88 },
  {   173,   283,  -850,  1457, -2298,  3696, -6823, 24487,
    16043, -5667,  3176, -2036,  1379,  -925,   440,    86 },
  {   175,   278,  -846,  1453, -2294,  3692, -6821, 24634,
    15866, -5624,  3153, -2023,  1373,  -923,   441,    85 },
  {   177,   274,  -841,  1449, -2290,  3688, -6818, 24781,
    15690, -5580,  3131, -2010,  1366,  -922,   443,    84 },
  {   179,   269,  -836,  1445, -2286,  3683, -6814, 24927,
    15512, -5535,  3107, -1997,  1359,  -920,   445,    82 },
  {   181,   264,  -831,  1441, -2282,  3677, -6808, 25071,
    15335, -5490,  3084, -1983,  1353,  -918,   446,    81 },
  {   183,   260,  -826,  1436, -2277,  3671, -6802, 25215,
    15158, -5444,  3060, -1970,  1345,  -916,   448,    80 },
  {   185,   255,  -821,  1431, -2272,  3665, -6795, 25357,
    14980, -5397,  3036, -1956,  1338,  -914,   449,    78 },
  {   187,   250,  -816,  1426, -2266,  3658, -6786, 25499,
    14803, -5350,  3011, -1941,  1331,  -912,   450,    77 },
  {   189,   245,  -810,  1420, -2260,  3650, -6776, 25640,
    14625, -5302,  2986, -1927,  1323,  -910,   452,    76 },
  {   191,   240,  -805,  1415, -2254,  3641, -6766, 25779,
    14448, -5254,  2960, -1912,  1315,  -907,   453,    75 },
  {   193,   236,  -799,  1409, -2247,  3632, -6754, 25918,
    14270, -5205,  2935, -1897,  1308,  -905,   454,    73 },
  {   196,   231,  -793,  1403, -2240,  3623, -6741, 26055,
    14092, -5156,  2909, -1882,  1300,  -902,   455,    72 },
  {   198,   226,  -787,  1397, -2233,  3613, -6727, 26191,
    13915, -5106,  2882, -1866,  1291,  -900,   457,    71 },
  {   200,   220,  -781,  1390, -2225,  3602, -6711, 26327,
    13737, -5056,  2855, -1851,  1283,  -897,   458,    70 },
  {   202,   215,  -775,  1384, -2217,  3591, -6695, 26461,
    13559, -5005,  2828, -1835,  1274,  -894,   459,    69 },
  {   204,   210,  -769,  1377, -2209,  3579, -6677, 26594,
    13382, -4953,  2801, -1819,  1266,  -891,   460,    67 },
  {   206,   205,  -762,  1370, -2200,  3566, -6659, 26726,
    13204, -4901,  2773, -1802,  1257,  -888,   461,    66 },
  {   208,   200,  -756,  1362, -2191,  3553, -6639, 26856,
    13026, -4849,  2745, -1786,  1248,  -885,   462,    65 },
  {   211,   194,  -749,  1355, -2181,  3539, -6618, 26986,
    12849, -4796,  2717, -1769,  1239,  -882,   462,    64 },
  {   213,   189,  -742,  1347, -2171,  3525, -6595, 27115,
    12672, -4743,  2688, -1752,  1230,  -879,   463,    63 },
  {   215,   183,  -735,  1339, -2161,  3510, -6572, 27242,
    12494, -4689,  2659, -1735,  1221,  -876,   464,    62 },
  {   217,   178,  -728,  1331, -2151,  3494, -6547, 27368,
    12317, -4635,  2630, -1718,  1211,  -872,   465,    60 },
  {   219,   172,  -721,  1322, -2140,  3478, -6522, 27493,
    12140, -4580,  2600, -1700,  1202,  -869,   465,    59 },
  {   221,   167,  -714,  1314, -2128,  3461, -6495, 27616,
    11963, -4525,  2570, -1683,  1192,  -865,   466,    58 },
  {   224,   161,  -707,  1305, -2117,  3444, -6466, 27739,
    11787, -4469,  2540, -1665,  1182,  -861,   467,    57 },
  {   226,   156,  -699,  1296, -2105,  3426, -6437, 27860,
    11610, -4413,  2510, -1647,  1172,  -857,   467,    56 },
  {   228,   150,  -691,  1287, -2092,  3407, -6407, 27980,
    11434, -4357,  2479, -1629,  1162,  -854,   468,    55 },
  {   230,   144,  -684,  1277, -2080,  3388, -6375, 28099,
    11258, -4300,  2448, -1610,  1152,  -850,   468,    54 },
  {   233,   138,  -676,  1267, -2067,  3368, -6342, 28216,
    11082, -4243,  2417, -1592,  1141,  -845,   468,    53 },
  {   235,   132,  -668,  1258, -2053,  3348, -6308, 28332,
    10906, -4186,  2386, -1573,  1131,  -841,   469,    52 },
  {   237,   127,  -660,  1247, -2039,  3327, -6272, 28447,
    10731, -4128,  2354, -1554,  1120,  -837,   469,    51 },
  {   239,   121,  -652,  1237, -2025,  3306, -6236, 28560,
    10556, -4070,  2322, -1535,  1110,  -833,   469,    50 },
  {   241,   115,  -643,  1227, -2011,  3283, -6198, 28673,
    10381, -4011,  2290, -1516,  1099,  -828,   470,    49 },
  {   244,   109,  -635,  1216, -1996,  3261, -6159, 28784,
    10207, -3952,  2258, -1496,  1088,  -824,   470,    48 },
  {   246,   103,  -626,  1205, -1981,  3237, -6119, 28893,
    10033, -3893,  2225, -1477,  1077,  -819,   470,    47 },
  {   248,    97,  -618,  1194, -1965,  3213, -6077, 29001,
     9859, -3834,  2193, -1457,  1066,  -815,   470,    46 },
  {   251,    90,  -609,  1182, -1949,  3189, -6035, 29108,
     9685, -3774,  2160, -1437,  1054,  -810,   470,    45 },
  {   253,    84,  -600,  1171, -1933,  3163, -5991, 29214,
     9512, -3714,  2126, -1417,  1043,  -805,   470,    44 },
  {   255,    78,  -591,  1159, -1916,  3138, -5946, 29318,
     9339, -3654,  2093, -1397,  1032,  -801,   470,    43 },
  {   257,    72,  -582,  1147, -1899,  3111, -5899, 29420,
     9167, -3593,  2059, -1377,  1020,  -796,   470,    43 },
  {   260,    65,  -573,  1135, -1882,  3084, -5852, 29522,
     8995, -3532,  2026, -1357,  1008,  -791,   470,    42 },
  {   262,    59,  -564,  1122, -1864,  3057, -5803, 29622,
     8823, -3471,  1992, -1336,   997,  -786,   470,    41 },
  {   264,    53,  -554,  1110, -1846,  3029, -5753, 29720,
     8652, -3410,  1958, -1316,   985,  -781,   470,    40 },
  {   267,    46,  -545,  1097, -1828,  3000, -5701, 29817,
     8481, -3349,  1923, -1295,   973,  -775,   469,    39 },
  {   269,    40,  -535,  1084, -1809,  2971, -5649, 29913,
     8311, -3287,  1889, -1274,   961,  -770,   469,    38 },
  {   271,    33,  -525,  1070, -1790,  2941, -5595, 30007,
     8141, -3225,  1854, -1253,   949,  -765,   469,    38 },
  {   273,    27,  -516,  1057, -1771,  2910, -5540, 30100,
     7972, -3163,  1820, -1232,   937,  -759,   468,    37 },
  {   276,    20,  -506,  1043, -1751,  2879, -5484, 30191,
     7803, -3101,  1785, -1211,   924,  -754,   468,    36 },
  {   278,    14,  -496,  1030, -1731,  2848, -5426, 30281,
     7635, -3038,  1750, -1189,   912,  -748,   468,    35 },
  {   280,     7,  -486,  1016, -1711,  2815, -5367, 30369,
     7467, -2976,  1715, -1168,   900,  -743,   467,    34 },
  {   283,     0,  -475,  1001, -1690,  2783, -5307, 30456,
     7300, -2913,  1679, -1147,   887,  -737,   467,    34 },
  {   285,    -6,  -465,   987, -1669,  2749, -5246, 30542,
     7133, -2850,  1644, -1125,   874,  -731,   466,    33 },
  {   287,   -13,  -455,   972, -1648,  2715, -5184, 30625,
     6967, -2787,  1608, -1103,   862,  -726,   465,    32 },
  {   290,   -20,  -444,   958, -1627,  2681, -5120, 30708,
     6801, -2724,  1573, -1082,   849,  -720,   465,    31 },
  {   292,   -27,  -433,   943, -1605,  2646, -5055, 30789,
     6636, -2661,  1537, -1060,   836,  -714,   464,    31 },
  {   294,   -34,  -423,   928, -1582,  2610, -4989, 30868,
     6472, -2598,  1501, -1038,   823,  -708,   463,    30 },
  {   297,   -40,  -412,   912, -1560,  2574, -4921, 30946,
     6308, -2534,  1465, -1016,   810,  -702,   463,    29 },
  {   299,   -47,  -401,   897, -1537,  2537, -4852, 31022,
     6145, -2471,  1429,  -994,   797,  -696,   462,    29 },
  {   301,   -54,  -390,   881, -1514,  2500, -4782, 31097,
     5982, -2407,  1393,  -972,   784,  -690,   461,    28 },
  {   303,   -61,  -379,   865, -1490,  2462, -4711, 31170,
     5820, -2343,  1357,  -949,   771,  -684,   460,    27 },
  {   306,   -68,  -368,   849, -1466,  2424, -4639, 31241,
     5659, -2280,  1321,  -927,   758,  -678,   460,    27 },
  {   308,   -75,  -357,   833, -1442,  2385, -4565, 31311,
     5498, -2216,  1284,  -905,   745,  -671,   459,    26 },
  {   310,   -82,  -345,   817, -1418,  2346, -4490, 31380,
     5338, -2152,  1248,  -882,   732,  -665,   458,    25 },
  {   313,   -89,  -334,   800, -1393,  2306, -4414, 31447,
     5179, -2089,  1212,  -860,   718,  -659,   457,    25 },
  {   315,   -96,  -322,   783, -1368,  2265, -4336, 31512,
     5020, -2025,  1175,  -838,   705,  -652,   456,    24 },
  {   317,  -103,  -311,   766, -1342,  2224, -4258, 31576,
     4863, -1961,  1139,  -815,   691,  -646,   455,    24 },
  {   320,  -111,  -299,   749, -1317,  2183, -4178, 31638,
     4705, -1897,  1102,  -792,   678,  -639,   454,    23 },
  {   322,  -118,  -287,   732, -1291,  2141, -4097, 31698,
     4549, -1833,  1066,  -770,   664,  -633,   453,    22 },
  {   324,  -125,  -275,   715, -1265,  2098, -4014, 31757,
     4393, -1770,  1029,  -747,   651,  -626,   452,    22 },
  {   326,  -132,  -263,   697, -1238,  2055, -3931, 31814,
     4239, -1706,   992,  -725,   637,  -620,   450,    21 },
  {   329,  -139,  -251,   679, -1211,  2011, -3846, 31870,
     4084, -1642,   956,  -702,   624,  -613,   449,    20 },
  {   331,  -147,  -239,   661, -1184,  1967, -3760, 31924,
     3931, -1579,   919,  -679,   610,  -606,   448,    20 },
  {   333,  -154,  -227,   643, -1157,  1923, -3672, 31976,
     3779, -1515,   883,  -656,   596,  -599,   447,    19 },
  {   335,  -161,  -215,   625, -1129,  1877, -3584, 32027,
     3627, -1452,   846,  -634,   582,  -593,   446,    19 },
  {   338,  -168,  -203,   607, -1102,  1832, -3494, 32076,
     3476, -1388,   809,  -611,   569,  -586,   444,    18 },
  {   340,  -176,  -190,   588, -1073,  1786, -3403, 32123,
     3326, -1325,   773,  -588,   555,  -579,   443,    18 },
  {   342,  -183,  -178,   570, -1045,  1739, -3311, 32169,
     3177, -1262,   736,  -565,   541,  -572,   442,    18 },
  {   344,  -190,  -165,   551, -1016,  1692, -3218, 32213,
     3028, -1199,   700,  -543,   527,  -565,   440,    17 },
  {   347,  -198,  -153,   532,  -987,  1645, -3123, 32256,
     2881, -1136,   663,  -520,   513,  -558,   439,    17 },
  {   349,  -205,  -140,   513,  -958,  1597, -3027, 32296,
     2734, -1073,   626,  -497,   499,  -551,   438,    16 },
  {   351,  -212,  -127,   494,  -929,  1548, -2930, 32335,
     2588, -1010,   590,  -474,   486,  -544,   436,    16 },
  {   353,  -220,  -115,   474,  -899,  1499, -2832, 32373,
     2443,  -947,   554,  -452,   472,  -537,   435,    15 },
  {   355,  -227,  -102,   455,  -869,  1450, -2733, 32409,
     2299,  -885,   517,  -429,   458,  -530,   433,    15 },
  {   358,  -235,   -89,   435,  -839,  1400, -2632, 32443,
     2156,  -823,   481,  -406,   444,  -523,   432,    14 },
  {   360,  -242,   -76,   415,  -809,  1350, -2530, 32475,
     2014,  -761,   445,  -383,   430,  -516,   430,    13 },
  {   362,  -250,   -63,   395,  -778,  1299, -2428, 32506,
     1873,  -699,   408,  -361,   416,  -509,   428,    13 },
  {   364,  -257,   -50,   375,  -747,  1248, -2324, 32535,
     1732,  -637,   372,  -338,   402,  -502,   427,    12 },
  {   366,  -264,   -37,   355,  -716,  1197, -2218, 32562,
     1593,  -575,   336,  -315,   388,  -494,   425,    12 },
  {   368,  -272,   -24,   335,  -685,  1145, -2112, 32588,
     1454,  -514,   300,  -293,   374,  -487,   424,    11 },
  {   370,  -279,   -10,   315,  -653,  1093, -2004, 32612,
     1317,  -453,   265,  -270,   360,  -480,   422,    11 },
  {   373,  -287,     3,   294,  -622,  1040, -1896, 32634,
     1180,  -392,   229,  -248,   346,  -473,   420,    11 },
  {   375,  -294,    16,   274,  -590,   987, -1786, 32654,
     1045,  -331,   193,  -225,   332,  -465,   419,    11 },
  {   377,  -302,    30,   253,  -557,   933, -1675, 32673,
      910,  -270,   158,  -203,   318,  -458,   417,    12 },
  {   379,  -309,    43,   232,  -525,   879, -1562, 32690,
      777,  -210,   122,  -180,   304,  -451,   415,    13 },
  {   381,  -317,    57,   211,  -493,   825, -1449, 32706,
      644,  -150,    87,  -158,   290,  -443,   413,    14 },
  {   383,  -324,    70,   190,  -460,   770, -1335, 32720,
      513,   -90,    51,  -136,   276,  -436,   412,    15 },
  {   385,  -332,    84,   169,  -427,   715, -1219, 32732,
      382,   -31,    16,  -113,   262,  -429,   410,    16 },
  {   387,  -339,    97,   148,  -394,   660, -1103, 32742,
      252,    28,   -19,   -91,   248,  -421,   408,    18 },
  {   389,  -347,   111,   126,  -360,   604,  -985, 32750,
      124,    87,   -53,   -69,   235,  -414,   406,    19 },
  {   391,  -354,   124,   105,  -327,   548,  -866, 32757,
       -4,   146,   -88,   -47,   221,  -406,   404,    20 },
  {   393,  -362,   138,    84,  -293,   491,  -746, 32762,
     -130,   204,  -123,   -25,   207,  -399,   402,    21 },
  {   395,  -369,   152,    62,  -260,   435,  -625, 32766,
     -255,   262,  -157,    -3,   193,  -391,   401,    22 },
  {   397,  -377,   166,    40,  -226,   377,  -503, 32767,
     -380,   320,  -191,    19,   179,  -384,   399,    22 },
};


extern "C"
{
  void _fft_16(const fcomplex *in, int in_stride, fcomplex *out, int out_stride);
  void _prefetch(const i16complex *, int nr_samples);
  void _transpose_2(fcomplex *out, const fcomplex *in);
  void _transpose_3(fcomplex *out, const fcomplex *in, int nr_samples);
  void _filter(fcomplex delayLine[NR_TAPS], const float weights[NR_TAPS], const i16complex samples[], fcomplex *out, int nr_samples_div_16);
  void _zero_area(fcomplex *ptr, int nr_cache_lines);
}


WH_PPF::WH_PPF(const string& name, const short subBandID, const short max_element):
  WorkHolder(1, NR_CORRELATORS_PER_FILTER, name, "WH_Correlator"),
  itsSubBandID(subBandID),
  itsMaxElement(max_element)
{
  ACC::APS::ParameterSet myPS("TFlopCorrelator.cfg");

#if 1
  int NrTaps		     = myPS.getInt32("PPF.NrTaps");
  int NrStations	     = myPS.getInt32("PPF.NrStations");
  int NrStationSamples	     = myPS.getInt32("PPF.NrStationSamples");
  int NrPolarizations	     = myPS.getInt32("PPF.NrPolarizations");
  int NrSubChannels	     = myPS.getInt32("PPF.NrSubChannels");
  int NrCorrelatorsPerFilter = myPS.getInt32("PPF.NrCorrelatorsPerFilter");
  
  assert(NrTaps			== NR_TAPS);
  assert(NrStations		== NR_STATIONS);
  assert(NrStationSamples	== NR_STATION_SAMPLES);
  assert(NrPolarizations	== NR_POLARIZATIONS);
  assert(NrSubChannels		== NR_SUB_CHANNELS);
  assert(NrCorrelatorsPerFilter == NR_CORRELATORS_PER_FILTER);
  assert(itsMaxElement          <= MAX_STATIONS_PER_PPF);
#endif

  assert(NR_SAMPLES_PER_INTEGRATION % 16 == 0);

  getDataManager().addInDataHolder(0, new DH_PPF("input", itsSubBandID, myPS));
  
  for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
    getDataManager().addOutDataHolder(corr, new DH_CorrCube("output", itsSubBandID)); 
  }

  fftw_import_wisdom_from_string("(FFTW-2.1.5 (256 529 -1 0 1 1 1 352 0) (128 529 -1 0 1 1 0 2817 0) (64 529 -1 0 1 1 0 1409 0) (32 529 -1 0 1 1 0 705 0) (16 529 -1 0 1 1 0 353 0) (8 529 -1 0 1 1 0 177 0) (4 529 -1 0 1 1 0 89 0) (2 529 -1 0 1 1 0 45 0))");
  itsFFTWPlan = fftw_create_plan(NR_SUB_CHANNELS, FFTW_FORWARD, FFTW_USE_WISDOM);
}


WH_PPF::~WH_PPF()
{
  fftw_destroy_plan(itsFFTWPlan);
}


WorkHolder* WH_PPF::construct(const string& name, const short subBandID, const short max_element)
{
  return new WH_PPF(name, subBandID, max_element);
}


WH_PPF* WH_PPF::make(const string& name)
{
  return new WH_PPF(name, itsSubBandID, itsMaxElement);
}


void WH_PPF::preprocess()
{
}


#if defined __xlC__
inline __complex__ float to_fcomplex(i16complex z)
{
  return (float) z.real() + 1.0fi * z.imag();
}
#else
#define to_fcomplex(Z) (static_cast<fcomplex>(Z))
#endif



void WH_PPF::process()
{
  static NSTimer timer("WH_PPF::process()", true), fftTimer("FFT", true);
  static NSTimer inTimer("inTimer", true), prefetchTimer("prefetch", true);

  timer.start();

  typedef i16complex inputType[MAX_STATIONS_PER_PPF][NR_SAMPLES_PER_INTEGRATION][NR_SUB_CHANNELS][NR_POLARIZATIONS];

  inputType *input = (inputType *) static_cast<DH_PPF*>(getDataManager().getInHolder(0))->getBuffer();
  DH_CorrCube::BufferType *outputs[NR_CORRELATORS_PER_FILTER];
  static fcomplex fftInData[NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS][NR_SUB_CHANNELS] __attribute__((aligned(32)));
  static fcomplex fftOutData[2][NR_POLARIZATIONS][NR_SUB_CHANNELS] __attribute__((aligned(32)));
  static fcomplex tmp[4][NR_SAMPLES_PER_INTEGRATION] __attribute__((aligned(32)));

  for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
    outputs[corr] = (DH_CorrCube::BufferType *) (static_cast<DH_CorrCube*>(getDataManager().getOutHolder(corr))->getBuffer());
    assert((ptrdiff_t) outputs[corr] % 32 == 0);
  }

//   for (int stat = 0; stat < NR_STATIONS; stat ++) {
  for (int stat = 0; stat < itsMaxElement; stat ++) {

    inTimer.start();

    for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
      for (int chan = 0; chan < NR_SUB_CHANNELS; chan ++) {
#if 1
	for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time ++) {
	  fcomplex sample = to_fcomplex((*input)[stat][time][chan][pol]);
	  fftInData[time][pol][chan] = itsFIRs[stat][chan][pol].processNextSample(sample,(float*)FIR::weights[chan]);
	}
#else
	_filter(itsFIRs[stat][chan][pol].itsDelayLine, FIR::weights[chan], &(*input)[stat][0][chan][pol], tmp[chan & 3], NR_SAMPLES_PER_INTEGRATION / NR_TAPS);

	if ((chan & 3) == 3) {
	  _transpose_2(&fftInData[0][pol][chan - 3], &tmp[0][0]);
	}
#endif
      }
    }

    inTimer.stop();

    for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time += 2) {
      fftTimer.start();

      //_zero_area(fftOutData[0][0], sizeof fftOutData / 32);

      for (int t = 0; t < 2; t ++) {
	for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	  fftw_one(itsFFTWPlan, (fftw_complex *) fftInData[time + t][pol], (fftw_complex *) fftOutData[t][pol]);
	}
      }

      fftTimer.stop();

      for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
#if 1
	for (int chan = 0; chan < NR_CHANNELS_PER_CORRELATOR; chan ++) {
	  for (int t = 0; t < 2; t ++) {
	    for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	      (*outputs[corr])[chan][stat][time + t][pol] = fftOutData[t][pol][corr * NR_CHANNELS_PER_CORRELATOR + chan];
	    }
	  }
	}
#else
	_transpose_3(&(*outputs[corr])[0][stat][time][0], &fftOutData[0][0][corr * NR_CHANNELS_PER_CORRELATOR], NR_CHANNELS_PER_CORRELATOR);
#endif
      }
    }
  }

  timer.stop();
}


void WH_PPF::postprocess()
{
}


void WH_PPF::dump() const
{
}
