	/*-------------------------------------*
         |       routine definitions
        ---------------------------------------*/

/* -include_routines */
/*void  HC (long *, long *, long *, long *,
   double *,long *,long *,double *,
   double *,long *,double *,
   char  *, double *, long *);  */

void            ClassRef (long n, long m, long len, long iopt, double *data, long *index,
			  long *ia, long *ib, double *crit, double *membr, double *sigma,
			  long init);

int             Ioffset (long n, long i, long j);

double          DissNouvClas (long i2, long j2, long k, long n, double *membr, double *diss,
			      double diss_i2_j2, long iopt);

void            print_call (char *CallName);

int             test_set_options (char *Argument0, char *Argument[]);
