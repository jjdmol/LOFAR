 /* destine a tourner sous Unix */

	/*-------------------------------------*
         |       #includes
        ---------------------------------------*/

#define NUMERO_ABSENT 999999

	/*-------------------------------------*
         |       routine definitions
        ---------------------------------------*/

/* -include_routines */

void            ClassRef (long n, long m, long len, long iopt, double *data, int level_int,
			  long *ia, long *ib, double *crit, double *membr, double *sigma,
			  long init);

int             Ioffset (long n, long i, long j);

int             NotDans_ia_ib (long i, long n, long *ia, long *ib);

double          DissNouvClas (long i2, long j2, long k, long n, double *membr, double *diss,
			      double diss_i2_j2, long iopt);

void            AttribClass (long n, int level_int, long *ia, long *ib, int *classDuNeuro);

void            Rechercher (long n, int c, long *ia, long *ib, long neurone, int *dispo,
			    int *classDuNeuro);
