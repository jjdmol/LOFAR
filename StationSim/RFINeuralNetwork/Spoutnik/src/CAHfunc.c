
/***********************  Contents  ****************************************
* fonctions de classification hierarchiques en C
* issues de  F. Murtagh, ESA/ESO/STECF, Garching, February 1986.
* Sample driver program, VAX-11 Fortran; **********************************
* HC: O(n^2) time, O(n^2) space hierarchical clustering,  *******
* HCASS: determine cluster-memberships, Fortran 77. *********************** 
* HCDEN: draw upper part of dendrogram, VAX-11 Fortran. *******************
* Sample data set: last 36 lines. *****************************************
***************************************************************************/

/* #++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
   #                                                            #
   #  HIERARCHICAL CLUSTERING using (user-specified) criterion. #
   #                                                            #
   #  Parameters:                                               #
   #                                                            #
   #  DATA(N,M)         input data matrix,                      #
   #  DISS(LEN)         dissimilarities in lower half diagonal  #
   #                    storage; LEN = N.N-1/2,                 #
   #  IOPT              clustering criterion to be used,        #
   #  IA, IB, CRIT      history of agglomerations; dimensions   #
   #                    N, first N-1 locations only used,       #
   #  MEMBR, NN, DISNN  vectors of length N, used to store      # 
   #                    cluster cardinalities, current nearest  #
   #                    neighbour, and the dissimilarity assoc. #
   #                    with the latter.                        #
   #  FLAG              boolean indicator of agglomerable obj./ #
   #                    clusters.                               #
   #                                                            #
   #  F. Murtagh, ESA/ESO/STECF, Garching, February 1986.       #
   #                                                            #
   #------------------------------------------------------------# */
/* #include <values.h> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "CAH.h"

void
ClassRef (long n, long m, long len, long iopt, double *data, long *index, long *ia, long *ib,
	  double *crit, double *membr, double *sigma, long init)
{
  int             i;			       /* numero du referent */
  int             j;			       /* numero d'une autre classe */
  int             k;			       /* indice pour examiner toutes les classes */
  int             d;			       /* numero de la dimension consideree */
  int             jj = -1;		       /* classe la + semblable a la classe en cours d'etude */
  int             im = -1;		       /* plus petit numero des 2 classes les + semblables en cours de boucle */
  int             jm = -1;		       /* plus grand numero des 2 classes les + semblables en cours de boucle */
  int             i2;			       /* plus petit numero des 2 classes a agglomerer */
  int             j2;			       /* plus grand numero des 2 classes a agglomerer */

  double         *diss;			       /* dissimilarities in lower half diagonal storage   */
  int             ind;			       /* deplacement (indice) dans la liste des disimilarites   */
  long           *nn;			       /* current nearest  neighbour                       */
  double         *disnn;		       /* dissimilarity assoc. with the nearest neighbors  */
  int            *flag;			       /* boolean indicator of agglomerable obj. clusters  */

  /* 1 vrai = classe a prendre en compte, peut etre regroupee */
  /* 0 faux = classe a ne plus considerer, fondue dans une autre classe */
  double          dmin;			       /* the smallest dissimilaritie   */
  int             ind3;			       /* index sur la table des dissimilarites */
  double          diss_i2_j2;		       /* dissimilarite entre 2 classes a regrouper */
  double          nouvDiss;		       /* dissimilarite entre 1 regroupement et 1 autre classe */
  double          denominateur;		       /* dissimilarite entre 1 regroupement et 1 autre classe */
  int             boucle;
  int             indexAbs;
  int             ok;

  diss = (double *) malloc (len * sizeof (double));
  nn = (long *) malloc (n * sizeof (long));
  disnn = (double *) malloc (n * sizeof (double));
  flag = (int *) malloc (n * sizeof (int));

  printf ("\n n %ld m %ld len %ld iopt %ld init %ld\n", n, m, len, iopt, init);

/*  Initialisations   */

  if (init == 0)
    for (i = 0; i < n; i++) {
      membr[i] = 1;
      flag[i] = 1;
  } else
    for (i = 0; i < n; i++)
      if (membr[i] == 0)
	flag[i] = 0;
      else
	flag[i] = 1;

/*                         

   j| 0  1  2  3  4  5  6  7  8  9 10 11
   i   | 
   -----|------------------------------------          
   0   | #  1  2  3  4  5  6  7  8  9 10 11           # non calcule
   1   | #  # 12 13 14 15 16 17 18 19 20 21 
   2   | #  #  # 22 23 24 25 26 27 28 29 30
   3   | #  #  #  # 31 32 33 34 35 36 37 38 
   4   | #  #  #  #  # 39 40 41 42 43 44 45
   5   | #  #  #  #  #  # 46 47 48 49 50 51
   6   | #  #  #  #  #  #  # 52 53 54 55 56  
   7   | #  #  #  #  #  #  #  # 57 58 59 60 
   8   | #  #  #  #  #  #  #  #  # 61 62 63   
   9   | #  #  #  #  #  #  #  #  #  # 64 65 
   10   | #  #  #  #  #  #  #  #  #  #  # 66  
   11   | #  #  #  #  #  #  #  #  #  #  #  #

   tableau des indices de la table des dissimilarites 
   pour 12 classes de depart
 */

/*  Construct dissimilarity matrix  */
  for (i = 0; i < n - 1; i++)
    if (flag[i]) {
      /*     if (i < 2 )
         printf ("i %d ",i); */
      for (j = i + 1; j < n; j++)
	if (flag[j]) {
	  ind = Ioffset (n, i, j);
	  diss[ind] = 0;
	  for (d = 0; d < m; d++)
	    diss[ind] = diss[ind] + pow ((data[d + (i * m)] - data[d + (j * m)]), 2);

	  /* iopt = 1 : case of the min. var. method where merging criteria are
	     defined in terms of variances  rather than distances.)  */
	  if (iopt == 1)
	    diss[ind] = (diss[ind] * membr[i] * membr[j]) / (membr[i] + membr[j]);

	  /* iopt = 8 : case of the min var. method using the variance of the class */
	  if (iopt == 8) {
	    denominateur = (membr[i] + membr[j]) * sigma[i] * sigma[j];
	    diss[ind] = (diss[ind] * membr[i] * membr[j]) / denominateur;
	  }
	  /*if (i < 2 )
	     printf ("%dd%1.9f ", j, diss[ind]); */
	}
      /*if (i < 2 )
         printf ("\n"); */
    }
/* Looking for the index wich are not present */
  indexAbs = 0;
  for (i = 0; i < n - 1; i++)
    if (flag[i] && index[i] == 0)
      indexAbs++;
  printf ("nombre de referents non labellises avant la CAH : %d\n", indexAbs);

  for (boucle = 0; boucle < n - 1 && indexAbs > 0; boucle++)
/*  for ( boucle = 0 ; indexAbs > 0 ; boucle++ )  */
  {
/*printf ("boucle : %d indexAbs : %d ",boucle, indexAbs);
   if (boucle == n) 
   boucle = 0;
 */
/*  Carry out an agglomeration - first create list of NNs  */
/* pour chaque classe i de 1 a n-2, calcul de la + petite dissimilarite dissn[i]
   et de la classe la plus semblable  nn[i]  */
    for (i = 0; i < n - 1; i++)
      if (flag[i]) {
	ok = 1;
	dmin = MAXSCALAR;
	for (j = i + 1; j < n; j++)
	  if (flag[j]
	      && (index[i] == index[j] || index[i] == 0 || index[j] == 0)) {
	    ind = Ioffset (n, i, j);
	    if (diss[ind] < dmin) {
	      dmin = diss[ind];
	      jj = j;
	      ok = 0;
	    }
	  }

	if (jj < 0)
	  raise_error ("ClassRef", "jj variable is not initialized", ER_FAIL);

	nn[i] = jj;
	disnn[i] = dmin;
	/*  if (boucle == 0 )
	   if (ok==0)
	   printf ("i %d +p %d dmin %1.9f\n", i , jj, dmin);
	   else
	   printf ("tous les j flag 1"); 
	 */
      }
/* Next, determine least diss. using list of NNs */
/* recherche des 2 classes les plus semblables, dans le but de les regrouper
   si 2 classes ont un index different, elles ne sont pas regroupees */
    dmin = MAXSCALAR;
    for (i = 0; i < n - 1; i++)
      if (flag[i] && disnn[i] < dmin) {
	j = nn[i];
	if (index[i] == index[j] || index[i] == 0 || index[j] == 0) {
	  dmin = disnn[i];
	  im = i;
	  jm = nn[i];
	}
      }
    if (im < 0)
      raise_error ("ClassRef", "im variable is not initialized", ER_FAIL);
    if (jm < 0)
      raise_error ("ClassRef", "jm variable is not initialized", ER_FAIL);
    /* This allows an agglomeration to be carried out */
    if (im < jm) {
      i2 = im;
      j2 = jm;
    } else {
      i2 = jm;
      j2 = im;
    }
    ia[boucle] = i2;
    ib[boucle] = j2;
    crit[boucle] = dmin;
    flag[j2] = 0;
    if (index[i2] == 0 && index[j2] != 0) {
      index[i2] = index[j2];
      indexAbs--;
    }
    if (index[j2] == 0 && index[i2] != 0) {
      index[j2] = index[i2];
      indexAbs--;
    }
/*  Update dissimilarities from new cluster   */
    dmin = MAXSCALAR;
    ind3 = Ioffset (n, i2, j2);
    diss_i2_j2 = diss[ind3];

    jj = -1;
    for (k = 0; k < n; k++)
      if (flag[k] && k != i2) {
	/*  calcul nouvelle diss[ind]  */
	nouvDiss = DissNouvClas (i2, j2, k, n, membr, diss, diss_i2_j2, iopt);

	if (i2 < k && nouvDiss < dmin) {
	  dmin = nouvDiss;
	  jj = k;
	}
      }
    if (jj < 0)
      raise_error ("ClassRef", "jj variable is not initialized (2)", ER_FAIL);
    membr[i2] += membr[j2];
    disnn[i2] = dmin;
    nn[i2] = jj;

  }					       /* fin de la boucle sur l'indice boucle */

  printf ("boucle %d\n", boucle);
  printf ("indexAbs %d\n", indexAbs);

  /* free memory */
  free (diss);
  free (nn);
  free (disnn);
  free (flag);

  return;
}

double
DissNouvClas (long i2, long j2, long k, long n, double *membr, double *diss, double diss_i2_j2,
	      long iopt)
{
  /*Calcul de la dissimilarite entre la nouvelle classe et une autre classe */
  double          m_i2_j2_k;
  int             ind1, ind2;

  m_i2_j2_k = membr[i2] + membr[j2] + membr[k];

  if (i2 < k)
    ind1 = Ioffset (n, i2, k);
  else
    ind1 = Ioffset (n, k, i2);

  if (j2 < k)
    ind2 = Ioffset (n, j2, k);
  else
    ind2 = Ioffset (n, k, j2);

  switch (iopt) {
  case 1:
    /*  WARD'S MINIMUM VARIANCE METHOD - IOPT=1.            */
    diss[ind1] =
      (membr[i2] + membr[k]) * diss[ind1] + (membr[j2] + membr[k]) * diss[ind2] -
      membr[k] * diss_i2_j2;
    diss[ind1] = diss[ind1] / m_i2_j2_k;
    break;

  case 8:
    /*  WARD'S MINIMUM VARIANCE METHOD using the variance of the class - IOPT=8.            */
    diss[ind1] =
      (membr[i2] + membr[k]) * diss[ind1] + (membr[j2] + membr[k]) * diss[ind2] -
      membr[k] * diss_i2_j2;
    diss[ind1] = diss[ind1] / m_i2_j2_k;
    break;

  case 2:
    /*  SINGLE LINK METHOD - IOPT=2.                        */
    /*  on choisit le + petit de diss[ind1] et diss[ind2]   */
    if (diss[ind2] < diss[ind1])
      diss[ind1] = diss[ind2];
    break;

  case 3:
    /*  COMPLETE LINK METHOD - IOPT=3.                      */
    /*  on choisit le + grand de diss[ind1] et diss[ind2]   */
    if (diss[ind2] > diss[ind1])
      diss[ind1] = diss[ind2];
    break;

  case 4:
    /*  AVERAGE LINK (OR GROUP AVERAGE) METHOD - IOPT=4     */
    diss[ind1] = (membr[i2] * diss[ind1] + membr[j2] * diss[ind2]) / (membr[i2] + membr[j2]);
    break;

  case 5:
    /*  MCQUITTY'S METHOD - IOPT=5                          */
    diss[ind1] = 0.5 * diss[ind1] + 0.5 * diss[ind2];
    break;

  case 6:
    /*  MEDIAN (GOWER'S) METHOD - IOPT=6                    */
    diss[ind1] = 0.5 * diss[ind1] + 0.5 * diss[ind2] - 0.25 * diss_i2_j2;
    break;

  case 7:
    /*  CENTROID METHOD - IOPT=7                            */
    diss[ind1] =
      (membr[i2] * diss[ind1] + membr[j2] * diss[ind2] -
       membr[i2] * membr[j2] * diss_i2_j2 / (membr[i2] + membr[j2])
      ) / (membr[i2] + membr[j2]);
    break;

  default:
    diss[ind1] =
      (membr[i2] + membr[k]) * diss[ind1] + (membr[j2] + membr[k]) * diss[ind2] -
      membr[k] * diss_i2_j2;
    diss[ind1] = diss[ind1] / m_i2_j2_k;
    break;

  }
  return (diss[ind1]);
}

int
Ioffset (long n, long i, long j)
{
  /*Map row I and column J of upper half diagonal symmetric matrix onto vector */

  return (j + i * n - (i + 1) * (i + 2) / 2);

}
