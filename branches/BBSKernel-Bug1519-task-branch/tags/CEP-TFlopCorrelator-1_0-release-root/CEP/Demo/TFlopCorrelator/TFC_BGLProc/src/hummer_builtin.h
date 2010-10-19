/*
 * builtin interface to hummer^2 instructions for TOBEY
 */

extern "builtin" {

/* float, double loads */
double _Complex __lfps (float * addr);
double _Complex __lfxs (float * addr);
double _Complex __lfpd (double * addr);
double _Complex __lfxd (double * addr);

/* float, double stores */
void __stfps (float * addr, double _Complex);
void __stfxs (float * addr, double _Complex);
void __stfpd (double * addr, double _Complex);
void __stfxd (double * addr, double _Complex);
void __stfpiw (int * addr, double _Complex);

/* parallel operations */
double _Complex __fpadd (double _Complex, double _Complex);
double _Complex __fpsub (double _Complex, double _Complex);
double _Complex __fpmul (double _Complex, double _Complex);
double _Complex __fxmul (double _Complex, double _Complex);
double _Complex __fxpmul (double _Complex, double);
double _Complex __fxsmul (double _Complex, double);

/* multiply add */
double _Complex __fpmadd (double _Complex b, double _Complex a, double _Complex c);
double _Complex __fpnmadd (double _Complex b, double _Complex a, double _Complex c);
double _Complex __fpmsub (double _Complex b, double _Complex a, double _Complex c);
double _Complex __fpnmsub (double _Complex b, double _Complex a, double _Complex c);

double _Complex __fxmadd (double _Complex b, double _Complex a, double _Complex c);
double _Complex __fxnmadd (double _Complex b, double _Complex a, double _Complex c);
double _Complex __fxmsub (double _Complex b, double _Complex a, double _Complex c);
double _Complex __fxnmsub (double _Complex b, double _Complex a, double _Complex c);

double _Complex __fxcpmadd (double _Complex b, double _Complex a, double c);
double _Complex __fxcsmadd (double _Complex b, double _Complex a, double c);
double _Complex __fxcpnmadd (double _Complex b, double _Complex a, double c);
double _Complex __fxcsnmadd (double _Complex b, double _Complex a, double c);
double _Complex __fxcpmsub (double _Complex b, double _Complex a, double c);
double _Complex __fxcsmsub (double _Complex b, double _Complex a, double c);
double _Complex __fxcpnmsub (double _Complex b, double _Complex a, double c);
double _Complex __fxcsnmsub (double _Complex b, double _Complex a, double c);

double _Complex __fxcpnpma (double _Complex b, double _Complex a, double c);
double _Complex __fxcsnpma (double _Complex b, double _Complex a, double c);
double _Complex __fxcpnsma (double _Complex b, double _Complex a, double c);
double _Complex __fxcsnsma (double _Complex b, double _Complex a, double c);

double _Complex __fxcxnpma (double _Complex b, double _Complex a, double c);
double _Complex __fxcxnsma (double _Complex b, double _Complex a, double c);
double _Complex __fxcxma (double _Complex b, double _Complex a, double c);
double _Complex __fxcxnms (double _Complex b, double _Complex a, double c);

double _Complex __fpsel (double _Complex a, double _Complex b, double _Complex c);
double _Complex __fpctiw (double _Complex a);
double _Complex __fpctiwz (double _Complex a);
double _Complex __fprsp (double _Complex a);
double _Complex __fpneg (double _Complex a);
double _Complex __fpabs (double _Complex a);
double _Complex __fpnabs (double _Complex a);
double _Complex __fxmr (double _Complex a);

/* estimates */
double _Complex __fpre (double _Complex);
double _Complex __fprsqrte (double _Complex);

/* alignment: like prefetchByStream */
void __alignx (int, const void *);

} // extern
