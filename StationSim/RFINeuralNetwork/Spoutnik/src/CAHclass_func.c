/***********************  Contents  ****************************************
* fonctions de classification hierarchiques en C
* issues de  F. Murtagh, ESA/ESO/STECF, Garching, February 1986.
* Sample driver program, VAX-11 Fortran; **********************************
* HC: O(n^2) time, O(n^2) space hierarchical clustering,  *******
* HCASS: determine cluster-memberships, Fortran 77. *********************** 
* HCDEN: draw upper part of dendrogram, VAX-11 Fortran. *******************
* Sample data set: last 36 lines. *****************************************
***************************************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
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
#------------------------------------------------------------*/
/* #include <values.h> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "CAHclass.h"

void
ClassRef (long n, long m, long len, long iopt, double *data, int level_int, long *ia, long *ib,
	  double *crit, double *membr, double *sigma, long init)
{
  int             i;			       /* numero du referent */
  int             j;			       /* numero d'une autre classe */
  int             k;			       /* indice pour examiner toutes les classes */
  int             d;			       /* numero de la dimension consideree */
  int             jj = -1;			       /* classe la + semblable a la classe en cours d'etude */
  int             im = -1;			       /* plus petit numero des 2 classes les + semblables en cours de boucle */
  int             jm = -1;			       /* plus grand numero des 2 classes les + semblables en cours de boucle */
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
  int             nbClass;
  int             cptCardZero = 0;
  int             ind_ajout;

  printf ("dans ClassRef\n");

  diss = (double *) malloc (len * sizeof (double));
  nn = (long *) malloc (n * sizeof (long));
  disnn = (double *) malloc (n * sizeof (double));
  flag = (int *) malloc (n * sizeof (int));

  printf ("n %ld m %ld len %ld iopt %ld init %ld\n", n, m, len, iopt, init);

/*  Initialisations + comptage des cardinalites a zeros  */

  if (init == 0)
    for (i = 0; i < n; i++) {
      membr[i] = 1;
      flag[i] = 1;
  } else
    for (i = 0; i < n; i++)
      if (membr[i] == 0) {
	flag[i] = 0;
	cptCardZero++;
      } else
	flag[i] = 1;

  for (i = 0; i < n - 1; i++) {
    ia[i] = NUMERO_ABSENT;
    ib[i] = NUMERO_ABSENT;
  }

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
	}
    }

/* nombre de classes au depart = nombre de neurones de cardinalite > 0 */
  nbClass = n - cptCardZero;
  printf ("cptCardZero %d\n", cptCardZero);

  for (boucle = 0; boucle < n - 1 && nbClass > level_int; boucle++) {

/*  Carry out an agglomeration - first create list of NNs  */
/* pour chaque classe i de 1 a n-2, calcul de la + petite dissimilarite dissn[i]
                                        et de la classe la plus semblable  nn[i]  */
    for (i = 0; i < n - 1; i++)
      if (flag[i]) {
	dmin = MAXSCALAR;
	for (j = i + 1; j < n; j++)
	  if (flag[j]) {
	    ind = Ioffset (n, i, j);
	    if (diss[ind] < dmin) {
	      dmin = diss[ind];
	      jj = j;
	    }
	  }
	if (jj < 0)
	  raise_error ("CAHclass_func", "ClassRef: indice 'jj' negatif", ER_FAIL);
	nn[i] = jj;
	disnn[i] = dmin;
      }

/* Next, determine least diss. using list of NNs */
/* recherche des 2 classes les plus semblables */
    dmin = MAXSCALAR;
    for (i = 0; i < n - 1; i++)
      if (flag[i] && disnn[i] < dmin) {
	j = nn[i];
	dmin = disnn[i];
	im = i;
	jm = nn[i];
      }
    if ((im < 0) || (jm < 0))
      raise_error ("CAHclass_func", "ClassRef: un indice 'im' ou 'jm' est negatif", ER_FAIL);
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

/*  Update dissimilarities from new cluster   */
    dmin = MAXSCALAR;
    ind3 = Ioffset (n, i2, j2);
    diss_i2_j2 = diss[ind3];

    for (k = 0; k < n; k++)
      if (flag[k] && k != i2) {
	/*  calcul nouvelle diss[ind]  */
	nouvDiss = DissNouvClas (i2, j2, k, n, membr, diss, diss_i2_j2, iopt);

	if (i2 < k && nouvDiss < dmin) {
	  dmin = nouvDiss;
	  jj = k;
	}
      }

    nbClass--;
    membr[i2] += membr[j2];

  }					       /* fin de la boucle sur l'indice boucle */

/* ramassage des neurones seuls dans leur classe,
   c'est-a-dire ceux qui n'ont pas ete agglomeres dans une autre classe */

  ind_ajout = boucle;
  for (i = 0; i < n; i++)
    if (flag[i] && NotDans_ia_ib (i, n, ia, ib)) {
      ia[ind_ajout] = i;
      ind_ajout++;
    }

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

int
NotDans_ia_ib (long i, long n, long *ia, long *ib)
{
  int             ind;

  /* balayage des tables ia et ib pour trouver le neurone i */
  for (ind = 0; ind < n - 1; ind++)
    if (ia[ind] == i || ib[ind] == i)
      return 0;

  /* le neurone i n'est pas trouve, donc retour vrai */
  return 1;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                                                                             */
/* cette fonction rassemble les neurones par classe                            */
/* ia = tableau chaque neurone dans ia a recu le neurone correspondant dans ib */
/* neuroClasses  = tableau des numeros de classe pour chaque neurone           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

void
AttribClass (long n, int level_int, long *ia, long *ib, int *classDuNeuro)
{
  int             i;			       /* balayage du tableau dispo */
  int             c;			       /* numero de classe */
  int             plusLoin;		       /* booleen */

  int            *dispo;		       /* taableau de booleens indiquant si 
						  le neurone correspondant est disponible
						  --> 0 : deja classe
						  --> 1 : disponible */
  long            neurone = -1;		       /* numero de neurone */
  long            ibCorrespondant = -1;	       /* numero de neurone */

/* int nb_classes = 0;
   int nb_attribues = 0; */

  printf ("\ndans AttribClass nombre de classes voulues %d\n", level_int);

  dispo = (int *) malloc (n * sizeof (int));

  for (i = 0; i < n; i++)
    dispo[i] = 1;

/* la numerotation des classes commence a un */
  for (c = 1; c <= level_int; c++) {
    plusLoin = 1;
    /* recherche du 1er neurone de ia disponible pour une nouvelle classe */
    for (i = 0; i < n - 1 && plusLoin; i++) {
      neurone = ia[i];
      if (dispo[neurone]) {
	neurone = ia[i];
	ibCorrespondant = ib[i];
	plusLoin = 0;
      }
    }

    if (neurone < 0)
      raise_error ("CAHclass_func", "AttribClass: indice 'neurone' negatif", ER_FAIL);

    /* le neurone traite n'est plus disponible, il est de classe c */
    dispo[neurone] = 0;
    classDuNeuro[neurone] = c;

    if (ibCorrespondant < 0)
      raise_error ("CAHclass_func", "AttribClass: indice 'ibCorrespondant' negatif", ER_FAIL);

    if (ibCorrespondant != NUMERO_ABSENT) {
      /* le neurone correspondant dans la liste ib 
         n'est plus disponible et doit etre classe c */
      dispo[ibCorrespondant] = 0;
      classDuNeuro[ibCorrespondant] = c;

      /* il faut rechercher les neurones faisant partie de la meme classe */
      dispo[neurone] = 0;
      classDuNeuro[neurone] = c;
      Rechercher (n, c, ia, ib, neurone, dispo, classDuNeuro);

    }

  }

/* Pour debuguer : Controle du nombre de neurones classes */
/*
   for ( i = 0 ; i < n ; i++)
       { if ( dispo[i] == 0 )
              nb_classes++;
         if ( classDuNeuro[i] > 0 && classDuNeuro[i] <= level_int )
              nb_attribues++;
       }
*/

}

void
Rechercher (long n, int c, long *ia, long *ib, long neurone, int *dispo, int *classDuNeuro)
{
/* parcourir les tableaux ia et ib jusqu'a la fin des tableaux ou bien
   jusqu'a ce l'on trouve le neurone traite dans le tableau ib.
   En effet, si ce neurone est dans ib, ceci signifie qu'il est agglomere
   dans un autre neurone, et qu'on ne le retrouvera pas plus loin. */

  int             i;			       /* balayage des tableaux ia et ib */
  int             absent_ib = 1;	       /* booleen indiquant si le neurone traite n'a pas encore ete 

					          rencontre dans le tableau ib */
  long            suivant;		       /* numero de neurone */
  long            ibCorres;		       /* numero de neurone */
  long            iaCorres;		       /* numero de neurone */

/* si un neurone = 999999, c'est le correspondant ib d'un neurone de ia
   qui est seul dans sa classe, donc il est inexistant.
*/

  for (i = 0; i < n - 1 && absent_ib; i++) {
    iaCorres = ia[i];
    ibCorres = ib[i];
    if (ia[i] == neurone && dispo[ibCorres]) {
      suivant = ibCorres;
      dispo[suivant] = 0;
      classDuNeuro[suivant] = c;
      Rechercher (n, c, ia, ib, suivant, dispo, classDuNeuro);
    }

    if (ib[i] == neurone && dispo[iaCorres]) {
      suivant = iaCorres;
      absent_ib = 0;
      dispo[suivant] = 0;
      classDuNeuro[suivant] = c;
      Rechercher (n, c, ia, ib, suivant, dispo, classDuNeuro);
    }
  }
}
