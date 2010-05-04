//# tJonesCMul3.cc: Unit test for the JonesCMul3 class.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <utils.h>

#include <BBSKernel/Expr/JonesCMul3.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

void testScalar(vector<bool> &results, double tol = 1e-8)
{
    const dcomplex perturbation(1e-6, 1e-6);

    bool ok = false;

    Axis::ShPtr freqAxis(new RegularAxis(60e6, 80e6, 1));
    Axis::ShPtr timeAxis(new RegularAxis(4e9, 5e9, 1));
    Grid grid(freqAxis, timeAxis);

    JonesExpr A(new JNodeStub(Matrix(dcomplex(1.0, 2.0)),
        Matrix(dcomplex(3.0, 4.0)),
        Matrix(dcomplex(5.0, 6.0)),
        Matrix(dcomplex(7.0, 8.0)),
        perturbation));
    JonesExpr B(new JNodeStub(Matrix(dcomplex(9.0, 10.0)),
        Matrix(dcomplex(11.0, 12.0)),
        Matrix(dcomplex(13.0, 14.0)),
        Matrix(dcomplex(15.0, 16.0)),
        perturbation));
    JonesExpr C(new JNodeStub(Matrix(dcomplex(17.0, 18.0)),
        Matrix(dcomplex(19.0, 20.0)),
        Matrix(dcomplex(21.0, 22.0)),
        Matrix(dcomplex(23.0, 24.0)),
        perturbation));

    JonesExpr cmul3(new JonesCMul3(A, B, C));

    JonesResult buf;
    const JonesResult &res = cmul3.getResultSynced(Request(grid), buf);
    
    // Check main value.
    ok = compare(res.getResult11().getValue(), dcomplex(3952.0, 5916.0), tol)
        && compare(res.getResult12().getValue(), dcomplex(4768.0, 7212.0), tol)
        && compare(res.getResult21().getValue(), dcomplex(11296.0, 13452.0),
            tol)
        && compare(res.getResult22().getValue(), dcomplex(13648.0, 16412.0),
            tol);

    results.push_back(ok);
    log("A * B * C', all scalars", ok);

    JonesExpr pvA(new JNodeStub(Matrix(dcomplex(1.0, 2.0) + perturbation),
        Matrix(dcomplex(3.0, 4.0) + perturbation),
        Matrix(dcomplex(5.0, 6.0) + perturbation),
        Matrix(dcomplex(7.0, 8.0) + perturbation),
        perturbation));
    JonesExpr pvB(new JNodeStub(Matrix(dcomplex(9.0, 10.0) + perturbation),
        Matrix(dcomplex(11.0, 12.0) + perturbation),
        Matrix(dcomplex(13.0, 14.0) + perturbation),
        Matrix(dcomplex(15.0, 16.0) + perturbation),
        perturbation));
    JonesExpr pvC(new JNodeStub(Matrix(dcomplex(17.0, 18.0) + perturbation),
        Matrix(dcomplex(19.0, 20.0) + perturbation),
        Matrix(dcomplex(21.0, 22.0) + perturbation),
        Matrix(dcomplex(23.0, 24.0) + perturbation),
        perturbation));

    JonesExpr pvCmul3(new JonesCMul3(pvA, pvB, pvC));
    
    JonesResult pvBuf;
    const JonesResult &pvResRhs = pvCmul3.getResultSynced(Request(grid), pvBuf);

    const JonesResult &pvResLhs =
        cmul3.getResultSynced(Request(grid, true), buf);
    
    // Check main value.
    ok = compare(pvResLhs.getResult11().getValue(),
        dcomplex(3952.0, 5916.0), tol)
        && compare(pvResLhs.getResult12().getValue(),
            dcomplex(4768.0, 7212.0), tol)
        && compare(pvResLhs.getResult21().getValue(),
            dcomplex(11296.0, 13452.0), tol)
        && compare(pvResLhs.getResult22().getValue(),
            dcomplex(13648.0, 16412.0), tol);

    // Check for a single pertubed value with key (1, 0).
    PValueKey key(1, 0);
    ok = ok
        && (pvResLhs.getResult11().getPerturbedValueCount() == 1)
        && pvResLhs.getResult11().isPerturbed(key)
        && (pvResLhs.getResult12().getPerturbedValueCount() == 1)
        && pvResLhs.getResult12().isPerturbed(key)
        && (pvResLhs.getResult21().getPerturbedValueCount() == 1)
        && pvResLhs.getResult21().isPerturbed(key)
        && (pvResLhs.getResult22().getPerturbedValueCount() == 1)
        && pvResLhs.getResult22().isPerturbed(key);
        
    // Check perturbed main value.
    ok = ok
        && compare(pvResRhs.getResult11().getValue(),
            dcomplex(3.952002348000264e+03, 5.916002644000273e+03), tol)
        && compare(pvResRhs.getResult12().getValue(),
            dcomplex(4.768002796000295e+03, 7.212003156000304e+03), tol)
        && compare(pvResRhs.getResult21().getValue(),
            dcomplex(1.129600334000030e+04, 1.345200363600030e+04), tol)
        && compare(pvResRhs.getResult22().getValue(),
            dcomplex(1.364800391600033e+04, 1.641200427600033e+04), tol);

    // Compare perturbed value to perturbed main value.
    ok = ok
        && compare(pvResLhs.getResult11().getPerturbedValue(key),
            pvResRhs.getResult11().getValue(), tol)
        && compare(pvResLhs.getResult12().getPerturbedValue(key),
            pvResRhs.getResult12().getValue(), tol)
        && compare(pvResLhs.getResult21().getPerturbedValue(key),
            pvResRhs.getResult21().getValue(), tol)
        && compare(pvResLhs.getResult22().getPerturbedValue(key),
            pvResRhs.getResult22().getValue(), tol);
    
    results.push_back(ok);
    log("A * B * C', all scalars, simulated PValue == PValue", ok);
}

void testSSE2(vector<bool> &results, double tol = 1e-8)
{
    const dcomplex perturbation(1e-6, 1e-6);

    bool ok = false;

    Axis::ShPtr freqAxis(new RegularAxis(60e6, 80e6, 4));
    Axis::ShPtr timeAxis(new RegularAxis(4e9, 5e9, 4));
    Grid grid(freqAxis, timeAxis);

    const dcomplex ans11[16] =
        {dcomplex(26.0160000000000053,41.1799999999999997),
        dcomplex(43.7120000000000033,67.5160000000000053),
        dcomplex(61.4080000000000013,93.8520000000000039),
        dcomplex(79.1039999999999850,120.1880000000000024),
        dcomplex(96.8000000000000114,146.5240000000000009),
        dcomplex(114.4960000000000235,172.8600000000000136),
        dcomplex(132.1920000000000073,199.1960000000000264),
        dcomplex(149.8880000000000052,225.5319999999999823),
        dcomplex(167.5840000000000032,251.8679999999999950),
        dcomplex(185.2800000000000296,278.2040000000000077),
        dcomplex(202.9759999999999991,304.5400000000000205),
        dcomplex(220.6720000000000255,330.8759999999999764),
        dcomplex(238.3680000000000234,357.2119999999999891),
        dcomplex(256.0639999999999645,383.5480000000000018),
        dcomplex(273.7599999999999909,409.8840000000000146),
        dcomplex(291.4560000000000173,436.2200000000000273)};

    const dcomplex ans12[16] =
        {dcomplex(26.7680000000000042,42.3799999999999955),
        dcomplex(44.9759999999999991,69.4839999999999947),
        dcomplex(63.1840000000000046,96.5880000000000081),
        dcomplex(81.3919999999999959,123.6919999999999931),
        dcomplex(99.5999999999999943,150.7959999999999923),
        dcomplex(117.8080000000000211,177.9000000000000057),
        dcomplex(136.0159999999999911,205.0039999999999907),
        dcomplex(154.2239999999999895,232.1080000000000041),
        dcomplex(172.4320000000000164,259.2119999999999891),
        dcomplex(190.6400000000000148,286.3159999999999741),
        dcomplex(208.8480000000000132,313.4200000000000159),
        dcomplex(227.0560000000000400,340.5240000000000009),
        dcomplex(245.2640000000000100,367.6280000000000427),
        dcomplex(263.4719999999999800,394.7319999999999709),
        dcomplex(281.6800000000000068,421.8360000000000127),
        dcomplex(299.8880000000000337,448.9399999999999977)};

    const dcomplex ans21[16] =
        {dcomplex(74.6400000000000148,93.8359999999999985),
        dcomplex(127.6640000000000157,155.2440000000000282),
        dcomplex(180.6879999999999882,216.6520000000000152),
        dcomplex(233.7120000000000459,278.0600000000000591),
        dcomplex(286.7360000000000468,339.4680000000000746),
        dcomplex(339.7599999999999909,400.8760000000000332),
        dcomplex(392.7839999999999918,462.2839999999999918),
        dcomplex(445.8080000000000496,523.6920000000000073),
        dcomplex(498.8319999999999936,585.0999999999999091),
        dcomplex(551.8559999999999945,646.5080000000000382),
        dcomplex(604.8800000000001091,707.9159999999999400),
        dcomplex(657.9040000000001100,769.3240000000000691),
        dcomplex(710.9279999999998836,830.7319999999999709),
        dcomplex(763.9519999999999982,892.1400000000001000),
        dcomplex(816.9759999999999991,953.5480000000000018),
        dcomplex(870.0000000000000000,1014.9560000000000173)};

    const dcomplex ans22[16] =
        {dcomplex(76.7999999999999972,96.5719999999999885),
        dcomplex(131.3600000000000136,159.7719999999999914),
        dcomplex(185.9200000000000159,222.9720000000000368),
        dcomplex(240.4800000000000466,286.1720000000000255),
        dcomplex(295.0400000000000773,349.3720000000000141),
        dcomplex(349.6000000000000227,412.5720000000000027),
        dcomplex(404.1599999999999682,475.7720000000000482),
        dcomplex(458.7200000000000273,538.9719999999999800),
        dcomplex(513.2799999999999727,602.1720000000000255),
        dcomplex(567.8400000000000318,665.3719999999999573),
        dcomplex(622.4000000000000909,728.5720000000000027),
        dcomplex(676.9600000000000364,791.7720000000001619),
        dcomplex(731.5199999999999818,854.9719999999999800),
        dcomplex(786.0799999999999272,918.1720000000000255),
        dcomplex(840.6399999999998727,981.3719999999998436),
        dcomplex(895.2000000000000455,1044.5720000000001164)};

    vector<dcomplex> B11(16);
    vector<dcomplex> B12(16);
    vector<dcomplex> B21(16);
    vector<dcomplex> B22(16);
    
    for(size_t i = 0; i < 16; ++i)
    {
        B11[i] = dcomplex(8 * (i + 1), 8 * (i + 1) + 1) / 1e3;
        B12[i] = dcomplex(8 * (i + 1) + 2, 8 * (i + 1) + 3) / 1e3;
        B21[i] = dcomplex(8 * (i + 1) + 4, 8 * (i + 1) + 5) / 1e3;
        B22[i] = dcomplex(8 * (i + 1) + 6, 8 * (i + 1) + 7) / 1e3;
    }

    JonesExpr A(new JNodeStub(Matrix(dcomplex(1.0, 2.0)),
        Matrix(dcomplex(3.0, 4.0)),
        Matrix(dcomplex(5.0, 6.0)),
        Matrix(dcomplex(7.0, 8.0)),
        perturbation));
    JonesExpr B(new JNodeStub(Matrix(&(B11[0]), 4, 4),
        Matrix(&(B12[0]), 4, 4),
        Matrix(&(B21[0]), 4, 4),
        Matrix(&(B22[0]), 4, 4),
        perturbation));
    JonesExpr C(new JNodeStub(Matrix(dcomplex(136.0, 137.0)),
        Matrix(dcomplex(138.0, 139.0)),
        Matrix(dcomplex(140.0, 141.0)),
        Matrix(dcomplex(142.0, 143.0)),
        perturbation));
    
    JonesExpr cmul3(new JonesCMul3(A, B, C));

    JonesResult buf;
    const JonesResult &res = cmul3.getResultSynced(Request(grid), buf);

    // Check main value.
    ok = compare(res.getResult11().getValue(), Matrix(ans11, 4, 4), tol)
        && compare(res.getResult12().getValue(), Matrix(ans12, 4, 4), tol)
        && compare(res.getResult21().getValue(), Matrix(ans21, 4, 4), tol)
        && compare(res.getResult22().getValue(), Matrix(ans22, 4, 4), tol);

    results.push_back(ok);
    log("A * B * C', A and C all scalar, B 4x4 array (tests SSE code)", ok);

    const dcomplex pvAns11[16] =
        {dcomplex(26.0182243451200854,41.1833054651200925),
        dcomplex(43.7142333051201533,67.5193144251201716),
        dcomplex(61.4102422651202033,93.8553233851202151),
        dcomplex(79.1062512251202890,120.1913323451203013),
        dcomplex(96.8022601851203319,146.5273413051203590),
        dcomplex(114.4982691451204175,172.8633502651204310),
        dcomplex(132.1942781051204747,199.1993592251204745),
        dcomplex(149.8902870651205319,225.5353681851205465),
        dcomplex(167.5862960251205891,251.8713771451206185),
        dcomplex(185.2823049851206179,278.2073861051206904),
        dcomplex(202.9783139451207603,304.5433950651207624),
        dcomplex(220.6743229051208175,330.8794040251207775),
        dcomplex(238.3703318651208747,357.2154129851208495),
        dcomplex(256.0663408251209603,383.5514219451209783),
        dcomplex(273.7623497851209891,409.8874309051210503),
        dcomplex(291.4583587451210747,436.2234398651211222)};

    const dcomplex pvAns12[16] =
        {dcomplex(26.7702886971520861,42.3834018491520936),
        dcomplex(44.9782979131521472,69.4874110651521733),
        dcomplex(63.1863071291522118,96.5914202811522244),
        dcomplex(81.3943163451522764,123.6954294971523041),
        dcomplex(99.6023255611523268,150.7994387131523695),
        dcomplex(117.8103347771524199,177.9034479291524349),
        dcomplex(136.0183439931524845,205.0074571451524719),
        dcomplex(154.2263532091525349,232.1114663611525089),
        dcomplex(172.4343624251525853,259.2154755771526311),
        dcomplex(190.6423716411526357,286.3194847931526397),
        dcomplex(208.8503808571527429,313.4234940091527619),
        dcomplex(227.0583900731527933,340.5275032251528273),
        dcomplex(245.2663992891528437,367.6315124411528359),
        dcomplex(263.4744085051528941,394.7355216571529581),
        dcomplex(281.6824177211530014,421.8395308731530235),
        dcomplex(299.8904269371530518,448.9435400891530890)};

    const dcomplex pvAns21[16] =
        {dcomplex(74.6466406971520939,93.8436898491520708),
        dcomplex(127.6706499131521753,155.2516990651521667),
        dcomplex(180.6946591291522282,216.6597082811522341),
        dcomplex(233.7186683451522811,278.0677174971522732),
        dcomplex(286.7426775611523908,339.4757267131523122),
        dcomplex(339.7666867771523584,400.8837359291524081),
        dcomplex(392.7906959931524398,462.2917451451525039),
        dcomplex(445.8147052091526348,523.6997543611525998),
        dcomplex(498.8387144251526024,585.1077635771525820),
        dcomplex(551.8627236411526837,646.5157727931526779),
        dcomplex(604.8867328571527651,707.9237820091527738),
        dcomplex(657.9107420731527327,769.3317912251527559),
        dcomplex(710.9347512891529277,830.7398004411527381),
        dcomplex(763.9587605051528953,892.1478096571529477),
        dcomplex(816.9827697211529767,953.5558188731528162),
        dcomplex(870.0067789371530580,1014.9638280891529121)};

    const dcomplex pvAns22[16] =
        {dcomplex(76.8068330491840925,96.5799142331840699),
        dcomplex(131.3668425211841679,159.7799237051841601),
        dcomplex(185.9268519931841865,222.9799331771842219),
        dcomplex(240.4868614651842904,286.1799426491842837),
        dcomplex(295.0468709371843943,349.3799521211842602),
        dcomplex(349.6068804091844413,412.5799615931843505),
        dcomplex(404.1668898811844883,475.7799710651844407),
        dcomplex(458.7268993531846490,538.9799805371845878),
        dcomplex(513.2869088251845824,602.1799900091845075),
        dcomplex(567.8469182971846294,665.3799994811846545),
        dcomplex(622.4069277691846764,728.5800089531846879),
        dcomplex(676.9669372411847235,791.7800184251848350),
        dcomplex(731.5269467131848842,854.9800278971847547),
        dcomplex(786.0869561851849312,918.1800373691849018),
        dcomplex(840.6469656571848645,981.3800468411848215),
        dcomplex(895.2069751291849116,1044.5800563131849685)};
    
    vector<dcomplex> pvB11(16);
    vector<dcomplex> pvB12(16);
    vector<dcomplex> pvB21(16);
    vector<dcomplex> pvB22(16);

    for(size_t i = 0; i < 16; ++i)
    {
        pvB11[i] = dcomplex(8 * (i + 1), 8 * (i + 1) + 1) / 1e3
            + perturbation; 
        pvB12[i] = dcomplex(8 * (i + 1) + 2, 8 * (i + 1) + 3) / 1e3
            + perturbation;
        pvB21[i] = dcomplex(8 * (i + 1) + 4, 8 * (i + 1) + 5) / 1e3
            + perturbation;
        pvB22[i] = dcomplex(8 * (i + 1) + 6, 8 * (i + 1) + 7) / 1e3
            + perturbation;
    }

    JonesExpr pvA(new JNodeStub(Matrix(dcomplex(1.0, 2.0) + perturbation),
        Matrix(dcomplex(3.0, 4.0) + perturbation),
        Matrix(dcomplex(5.0, 6.0) + perturbation),
        Matrix(dcomplex(7.0, 8.0) + perturbation),
        perturbation));
    JonesExpr pvB(new JNodeStub(Matrix(&(pvB11[0]), 4, 4),
        Matrix(&(pvB12[0]), 4, 4),
        Matrix(&(pvB21[0]), 4, 4),
        Matrix(&(pvB22[0]), 4, 4),
        perturbation));
    JonesExpr pvC(new JNodeStub(Matrix(dcomplex(136.0, 137.0) + perturbation),
        Matrix(dcomplex(138.0, 139.0) + perturbation),
        Matrix(dcomplex(140.0, 141.0) + perturbation),
        Matrix(dcomplex(142.0, 143.0) + perturbation),
        perturbation));

    JonesExpr pvCmul3(new JonesCMul3(pvA, pvB, pvC));
    
    JonesResult pvBuf;
    const JonesResult &pvResRhs = pvCmul3.getResultSynced(Request(grid), pvBuf);

    const JonesResult &pvResLhs =
        cmul3.getResultSynced(Request(grid, true), buf);
    
    // Check main value.
    ok = compare(pvResLhs.getResult11().getValue(), Matrix(ans11, 4, 4), tol)
        && compare(pvResLhs.getResult12().getValue(), Matrix(ans12, 4, 4), tol)
        && compare(pvResLhs.getResult21().getValue(), Matrix(ans21, 4, 4), tol)
        && compare(pvResLhs.getResult22().getValue(), Matrix(ans22, 4, 4), tol);

    // Check for a single pertubed value with key (1, 0).
    PValueKey key(1, 0);
    ok = ok
        && (pvResLhs.getResult11().getPerturbedValueCount() == 1)
        && pvResLhs.getResult11().isPerturbed(key)
        && (pvResLhs.getResult12().getPerturbedValueCount() == 1)
        && pvResLhs.getResult12().isPerturbed(key)
        && (pvResLhs.getResult21().getPerturbedValueCount() == 1)
        && pvResLhs.getResult21().isPerturbed(key)
        && (pvResLhs.getResult22().getPerturbedValueCount() == 1)
        && pvResLhs.getResult22().isPerturbed(key);
        
    // Check perturbed main value.
    ok = compare(pvResRhs.getResult11().getValue(), Matrix(pvAns11, 4, 4), tol)
        && compare(pvResRhs.getResult12().getValue(), Matrix(pvAns12, 4, 4),
            tol)
        && compare(pvResRhs.getResult21().getValue(), Matrix(pvAns21, 4, 4),
            tol)
        && compare(pvResRhs.getResult22().getValue(), Matrix(pvAns22, 4, 4),
            tol);

    // Compare perturbed value to perturbed main value.
    ok = ok
        && compare(pvResLhs.getResult11().getPerturbedValue(key),
            pvResRhs.getResult11().getValue(), tol)
        && compare(pvResLhs.getResult12().getPerturbedValue(key),
            pvResRhs.getResult12().getValue(), tol)
        && compare(pvResLhs.getResult21().getPerturbedValue(key),
            pvResRhs.getResult21().getValue(), tol)
        && compare(pvResLhs.getResult22().getPerturbedValue(key),
            pvResRhs.getResult22().getValue(), tol);
    
    results.push_back(ok);
    log("A * B * C', A and C all scalar, B 4x4 array (tests SSE code),"
        " simulated PValue == PValue", ok);
}

int main()
{
    INIT_LOGGER("tJonesCMul3");

    vector<bool> results;

    testScalar(results);
    testSSE2(results);

    bool ok = true;
    for(size_t i = 0; i < results.size(); ++i)
    {
        ok = ok && results[i];
    }
    
    return ok ? 0 : 1;
}

