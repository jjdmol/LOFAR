 /* programme de classification hierarchique en fonction du label 
    de chacun des referents.
    Apres la classification par Psom, grace au fichier de labellisation
    fourni par les geologues, certains referents ont pu etre labellises
    par vote majoritaire.
    Les referents non labellises recevront le label du referent  
    le plus proche dans la classification hierarchique.
    La classification hierarchique est amenagee de facon a ce que 
    2 referents ne puissent pas etre regroupes dans la meme classe
    si ils ont deja un label different.                            */

/*
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>> 
   >>>>         Main program for CAH
   >>>> 
   >>>>  Private: 
   >>>>         main
   >>>> 
   >>>>   Static: 
   >>>>   Public: 
   >>>> 
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "CAH.h"

/*
 * Variables Globales
 */

/* ------------------| TOPOLOGIE DE LA CARTE |----------------------------- */
char           *topol = "hexa";

/* ------------------| critere de clustering |----------------------------- */
long            iopt = 1;

/*-----------------------------------------------------------
|
|  Routine Name: main() - Hierarchical Clustering
|
|       Purpose: main program for CAH
|
|         Input:
|		char *i_file; {First Input data object}
|
|
|		int   iopt_list; {Clustering Criterion to be used}
|		char *iopt_label;
|
|		char *o_file; {Resulting output data object}
|
|		char *icard_file; {File containing a cardinal associated to each class}
|		int   icard_flag; {TRUE if -icard specified}
|
|        Output:
|       Returns:
|
|    Written By: Christophe Ambroise
|   Modified By: Dominique Frayssinet
|          Date: July 1, 1997
| Modifications:
|
------------------------------------------------------------*/

/*
   parametres du CAH.c
   ------------------------

   fichiers d'entree  :
   i_file      = "../fic/pvm_7_13d2_mean1.cal";      neurones partiellement indexes par somindex.c
   icard_file  = "../fic/pvm_7_13d2_card.res";       cardinalite des neurones (som.c)
   ivar_file   = NULL;                               variance des neurones (prsom.c)  NULL si som.c

   fichiers de sortie :
   o_file      = "../fic/pvm_7_13d2_io8_mean2.cal";  neurones entierement indexes par CAH pour
                                                     somlabel.c
   sam_file    = "../fic/pvm_7_13d2_io8_sam2.cal";   neurones entierement indexes pour sammon

   iopt        = 1;     indice de calcul des distances (voir CAHfunc.c)

 */

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr,
	   "\t%s [options] <SomIndexFile> <CardFile> <OutIndexMapFile> <OutIndexSaMapFile>\n",
	   CallName);
  fprintf (stderr,
	   "or after 'prsom':\n\t%s [options] <SomIndexFile> <CardFile> <VarFile> <OutIndexMapFile> <OutIndexSaMapFile>\n",
	   CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<SomIndexFile> ....... neurones partiellement indexes par somindex.\n");
  fprintf (stderr, "\t<CardFile> ........... cardinalite des neurones (som).\n");
  fprintf (stderr, "\t<VarFile> ............ variance des neurones (uniquement si prsom).\n");
  fprintf (stderr,
	   "\t<OutIndexMapFile> .... sortie : neurones entierement indexes par CAH pour somlabel.\n");
  fprintf (stderr,
	   "\t<OutIndexSaMapFile> .. sortie : neurones entierement indexes pour sammon.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\t\t-topol <MapTopology> :\n");
  fprintf (stderr, "\t\t                 to indicate the topology of the\n");
  fprintf (stderr, "\t\t                 map {'hexa', 'rect'}. Default = 'hexa'.\n");
  fprintf (stderr, "\t\t-iopt <ClusterIndex> :\n");
  fprintf (stderr, "\t\t                 to indicate the clustering index\n");
  fprintf (stderr, "\t\t                 used to compute distances. Default = 1.\n");
  fprintf (stderr, "\t\t                 Use 8 in case of 'prsom' input.\n");
}

/***************************************************************************\
 *  test_set_options (Argument0, Appel, Argument)                           *
\***************************************************************************/
int
test_set_options (
#ifndef NO_PROTOTYPES
		   char *Argument0, char *Argument[]
#else
		   Argument0, Argument
#endif
  )
#ifdef NO_PROTOTYPES
     char           *Argument0;
     char           *Argument[];
#endif
{
  if (!strcmp (Argument[0], "-topol")) {       /* -TOPOL */
    topol = strdup (Argument[1]);
    if (strcmp (topol, "rect") && strcmp (topol, "hexa"))
      raise_error (Argument0, "bad topology type. Only 'rect' or 'hexa'.", ER_CRITICAL);
    return 2;

  } else if (!strcmp (Argument[0], "-iopt")) { /* -IOPT */
    iopt = OurAToL (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-h")) {    /* HELP */
    print_call (Argument0);
    exit (1);

  } else {
    char Bff[BFFSIZE];
    sprintf(Bff,"option <%s> invalide", Argument[0]);
    print_call (Argument0);
    raise_error (Argument0, Bff, ER_CRITICAL);
  }

  return 0;
}

int
main (int argc, char **argv)
{

/* -main_variable_list */

  file_t          mapFile;		       /* the input file : a spoutnik file describing a map */

/* the output file looks like the input file with a new column  */
/*  indicating the class label of each weight vector */
  file_t          cardFile;
  file_t          varFile;
  file_t          output;		       /* the output file  */
  file_t          outsam;		       /* the output file for sammon */

  char           *i_file;		       /* First Input data object FILENAME */
  char           *o_file;		       /* Resulting output data object FILENAME */
  char           *sam_file;		       /* Resulting output data object FILENAME */
  char           *icard_file;		       /* File containing a cardinal associated to each class FILENAME */
  char           *ivar_file = (char *) NULL;   /* File containing the variance of each class FILENAME */
  int             icard_flag;

/* int             level_int; *//* Number of Clusters in largest Partition INT */

  mapDesc_t      *map = NULL;		       /* the map to be classified */
  mapDesc_t      *card = NULL;
  mapDesc_t      *var = NULL;

/* long           *label; *//* Vector of labels */

/* List of variables used for hierarchical clustering */
  long            n;			       /* Number of input vectors (weight vectors) */
  long            m;			       /* Number of variables describing each vector */
  long            len;			       /* len = n.n-1/2 is the number of usefull elements in diss */
  double         *data;			       /* input data matrix */
  long           *index;		       /* the real class we know for some of the n input vectors */
  long           *ia;			       /* history of agglomerations: index of first class */
  long           *ib;			       /* history of agglomerations: index of second class */
  double         *crit;			       /* history of agglomerations : minimum distance used */
  double         *membr;		       /* cluster cardinalities */
  double         *sigma;		       /* variance of each class */
  int             init;			       /* if init=1 membr is used for computing the first diss table */

  int             i, j, indice, plusProche;    /*Loop Index */

  int             NbExpectedArgs = 4;
  char           *argv0 = argv[0];
  int             NbArgs;

/*  main  */

  ++argv;
  --argc;
  while (argc && argv[0][0] == '-') {
    NbArgs = test_set_options (argv0, argv);
    argv += NbArgs;
    argc -= NbArgs;
  }

  if (argc < NbExpectedArgs) {
    print_call (argv0);
    raise_error (argv0, "invalid argument number", ER_CRITICAL);
  }

  /* ======================================================================== *
   *                     INITIALISATION  DES PARAMETRES                       *
   * ======================================================================== */

  /*
   * A T T E N T I O N
   * prendre la variance et iopt = 8  pour les som
   */
  icard_flag = 1;
  NbArgs = 0;

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| CELLULES PARTIELLEMENT INDEXES (SOMINDEX) |------------- */
  /*                -------------------------------------------               */
  i_file = strdup (argv[NbArgs++]);

  /* --------------| FICHIER DE CARDINALITE (SOM) |-------------------------- */
  /*                ------------------------------                            */
  icard_file = strdup (argv[NbArgs++]);

  if (argc == 5) {
    /* appel apres PRSOM */
    /* --------------| VARIANCE DES CELLULES (PRSOM) |----------------------- */
    /*                -------------------------------                         */
    ivar_file = strdup (argv[NbArgs++]);
  }

  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| CELLULES INDEXES POUR SOMLABEL |------------------------ */
  /*                --------------------------------                          */
  o_file = strdup (argv[NbArgs++]);

  /* --------------| CELLULES INDEXES POUR SAMMON |-------------------------- */
  /*                ------------------------------                            */
  sam_file = strdup (argv[NbArgs++]);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        CAH\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de neurones partiellement indexes par somindex ... %s\n",
	   i_file);
  fprintf (stdout, "      Fichier de cardinalite des neurones (som.c) .............. %s\n",
	   icard_file);
  fprintf (stdout, "      Fichier de variances des neurones (prsom.c) .............. %s\n",
	   (ivar_file) ? ivar_file : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de neurones entierement indexes pour somlabel .... %s\n",
	   o_file);
  fprintf (stdout, "      Fichier de neurones entierement indexes pour sammon ...... %s\n",
	   sam_file);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      autres parametres :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Topologie de la carte ...................... %s\n", topol);
  fprintf (stdout, "      Indice de calcul de distances .............. %ld\n", iopt);
  fprintf (stdout, "      *******************************************************\n\n");

  printf ("NewFile icard_file\n");
  NewFile (&cardFile, icard_file, FS_INPUT_MODE, "");
  LoadMap (&card, cardFile);
  FreeFile (&cardFile);

  n = card->range;
  index = (long *) malloc (n * sizeof (long));

  /* -main_before_lib_call */
  /* Step 1: Open the input files */

  printf ("NewFile i_file\n");
  NewFile (&mapFile, i_file, FS_INPUT_MODE, "");
  /* printf ("LoadMapIndex i_file\n"); */
  LoadMapIndex (&map, mapFile, &index);
  printf ("apres LoadMapIndex i_file\n");
  printf ("map->size->dim %ld\n", map->size->dim);
  printf ("map->size->values[0] %ld\n", map->size->values[0]);
  printf ("map->size->values[1] %ld\n", map->size->values[1]);
  FreeFile (&mapFile);

  if (iopt == 8) {
    printf ("NewFile ivar_file\n");
    NewFile (&varFile, ivar_file, FS_INPUT_MODE, "");
    LoadMap (&var, varFile);
    FreeFile (&varFile);
  }
  printf ("initialisation\n");
  /* Step 2: Initialisation */
  /* Initialisation */
  m = (*(map->firstPoint))->dim;
  len = (n * (n - 1)) / 2;
  data = (double *) malloc (m * n * sizeof (double));
  ia = (long *) malloc ((n - 1) * sizeof (long));
  ib = (long *) malloc ((n - 1) * sizeof (long));
  crit = (double *) malloc ((n - 1) * sizeof (double));
  membr = (double *) malloc (n * sizeof (double));
  sigma = (double *) malloc (n * sizeof (double));

  init = (icard_flag);

  /* Step 3: read the input file and save the information into tables */
  /* and variables */
  ForAllPoints (card, card->curPoint) {
    membr[(card->offset)] = (*(card->curPoint))->values[0];
  }

  if (iopt == 8) {
    printf ("avec variance\n");
    ForAllPoints (var, var->curPoint) {
      sigma[(var->offset)] = (*(var->curPoint))->values[0];
    }
  }
  /* Copy the map to DATA */
  ForAllPoints (map, map->curPoint) {
    for (j = 0; j < m; j++) {
      data[(map->offset) * m + j] = (*(map->curPoint))->values[j];
    }
  }

  ClassRef (n, m, len, iopt, data, index, ia, ib, crit, membr, sigma, init);

/* si 2 referents non labellises sont regroupes ensembles, aucun des 2 n'est labellise.
   Lorsque cette nouvelle classe sera regroupee avec un referent deja labellise,
   seul le plus petit referent du premier regroupement sera labellise.
   Un balayage des tables ia et ib permet de redonner un label a ces referents oublies.  */

  for (i = 0; i < n; i++)
    if (index[i] == 0 && membr[i] != 0) {
      for (indice = 0; index[i] == 0 && indice < n; indice++) {
	if (ia[indice] == i) {
	  plusProche = ib[indice];
	  index[i] = index[plusProche];
	}
	if (ib[indice] == i) {
	  plusProche = ia[indice];
	  index[i] = index[plusProche];
	}
      }
    }
/*
   for (indice = 0 ; indice < n-1 ; indice ++)
   { printf ("indice %d ia %ld ib %ld ", indice , ia[indice], ib[indice]);
   i = ia[indice]; 
   j = ib[indice]; 
   printf (" index[ia] %d index[ib] %d \n", index[i], index[j]);
   } 
 */
/*
   for ( i=0; i < n ; i++)
   printf ("index[%d] %d membr %f\n", i, index[i], membr[i]); 
 */

  /* Step 5: write the results into output files        */
  /* printf ("NewFile o_file\n");                       */
  NewFile (&output, o_file, FS_OUTPUT_MODE, "");
  SaveIndexMap (map, index, &output, membr);
  FreeFile (&output);

  /* printf ("NewFile sam_file\n");                     */
  NewFile (&outsam, sam_file, FS_OUTPUT_MODE, "");
  SaveIndexSaMap (map, index, &outsam, topol, membr);
  FreeFile (&outsam);

  /* write some information about the distance min used for agglomeration */

  printf ("\n--N = %ld\t", n);
  printf ("--M = %ld\n", m);

  /* free memory */
/*    FreeMap(&map);
   FreeMap(&card);
   FreeMap(&var);
   free(data);
   free(ia);
   free(ib);
   free(crit);
   free(membr);
   free(sigma);
 */
  printf ("CAH termine\n\n");

  return (0);
}
