 /* programme de classification hierarchique en fonction du label 
    de chacun des referents.
  */

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>> 
   >>>> 	Main program for CAH
   >>>> 
   >>>>  Private: 
   >>>> 	main
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
#include "CAHclass.h"

/*
 * Variables Globales
 */

/* ------------------| critere de clustering |----------------------------- */
long            iopt = 1;

/* ------------------| flag de reecriture des fichiers existants |--------- */
int             flg_rewrite = 0;

/* ------------------| Number of Clusters in largest Partition INT |------- */
int             level_int = 0;


/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr,
	   "\t%s [options] <WhichClassifier> <NbOfClasses> <SomMapFile> <CardFile>  <OutIndexMapFile>\n",
	   CallName);

  fprintf (stderr, "\nor:\n");

  fprintf (stderr,
	   "\t%s [options] <WhichClassifier> <NbOfClasses> <SomMapFile> <CardFile> <VarFile>  <OutIndexMapFile>\n",
	   CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<WhichClassifier> .... 1 for SOM classiffier, 2 for PRSOM.\n");
  fprintf (stderr, "\t<NbOfClasses> ........ number of classes or number of clusters in largest partition.\n");
  fprintf (stderr, "\t<SomMapFile> ......... fichier de neurones partiellement issue de SOM/PRSOM.\n");
  fprintf (stderr, "\t<CardFile> ........... fichier de cardinalite des neurones (som).\n");
  fprintf (stderr, "\t<VarFile> ............ [Parameter ONLY for iopt=8] Fichier de variances (prsom).\n");
  fprintf (stderr,
	   "\t<OutIndexMapFile> .... sortie : neurones entierement indexes par CAH pour somlabel.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\t\t-iopt <ClusterIndex> :\n");
  fprintf (stderr, "\t\t                 to indicate the clustering index\n");
  fprintf (stderr, "\t\t                 used to compute distances. Default = 1.\n");
  fprintf (stderr, "\t\t                 Use 8 in case of 'prsom' input.\n");
  fprintf (stderr, "\t\t-rewrite :\n");
  fprintf (stderr, "\t\t                 to allows existing file rewriting for\n");
  fprintf (stderr, "\t\t                 output files. Default = %s.\n",
	   (flg_rewrite) ? ENABLED_MSG : DISABLED_MSG);
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
  if (!strcmp (Argument[0], "-iopt")) { /* -IOPT */
    iopt = OurAToL (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-rewrite")) { /* -REWRITE */
    flg_rewrite = 1;
    return 1;

  } else if (!strcmp (Argument[0], "-h")) {    /* HELP */
    print_call (Argument0);
    exit (1);

  } else {
    print_call (Argument0);
    raise_error (Argument0, "nombre de parametres invalide", ER_CRITICAL);
  }

  return 0;
}

/*-----------------------------------------------------------
|
|  Routine Name: main() - Hierarchical Clustering
|
|       Purpose: main program for CAH
|
|         Input:
|		char *i_file; {First Input data object}
|
|		char *icard_file; {File containing a cardinal associated to each class}
|		int   icard_flag; {TRUE if -icard specified}
|
|		int   iopt_list; {Clustering Criterion to be used}
|		char *iopt_label;
|
|        Output:
|		char *o_file; {Resulting output data object}
|
|       Returns:
|
|    Written By: Christophe Ambroise
|   Modified By: Dominique Frayssinet
|          Date: July 1, 1997
| Modifications:
|
------------------------------------------------------------*/

int
main (int argc, char *argv[])
{

/* -main_variable_list */

  file_t          mapFile;		       /* the input file : a spoutnik file describing a map */

/* the output file looks like the input file with a new column  */
/*  indicating the class label of each weight vector */
  file_t          cardFile;
  file_t          varFile;
  file_t          output;		       /* the output file  */

  char           *i_file;		       /* First Input data object FILENAME */
  char           *o_file;		       /* Resulting output data object FILENAME */
  char           *icard_file;		       /* File containing a cardinal associated to each class FILENAME */
  char           *ivar_file = NULL;	       /* File containing the variance of each class FILENAME */
  int             icard_flag;

  int             classifier;		       /* after som classifier =1 , after prsom clasifier = 2 */

  char            strCAH[512];

  mapDesc_t      *map = NULL;		       /* the map to be classified */
  mapDesc_t      *card = NULL;
  mapDesc_t      *var = NULL;

/* List of variables used for hierarchical clustering*/
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

  int            *classDuNeuro;		       /* list of the class of each neuron */

  int             i, j;			       /* Loop Index */

  char            BffMsg[BFFSIZE];
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

  NbArgs = 0;

  printf ("CAHclass appele\n");

  /*=======================================================*/
  /* A T T E N T I O N */
  /* prendre la variance et iopt = 8  pour les prsom */

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| DE QUEL CLASSIFFIEUR PROVIENT LES DONNEES |------------- */
  /*               |    1 for SOM classiffier, 2 for PRSOM     |              */
  /*                -------------------------------------------               */
  sscanf (argv[NbArgs++],"%d", &classifier);

  if ((classifier != 1) && (classifier != 2)) {
    sprintf(BffMsg,"\n *** %s: invalid classifier (%d). Gives only 1 or 2 (for SOM or PRSOM) ***\n\n",
	    argv0, classifier);
    raise_error (argv0, BffMsg, ER_CRITICAL);
  }

  /* --------------| NOMBRE DE CLASSES VOULUES |----------------------------- */
  /*                -------------------------------------------               */
  sscanf (argv[NbArgs++],"%d", &level_int);

  /* --------------| CELLULES PARTIELLEMENT INDEXES (SOMINDEX) |------------- */
  /*                -------------------------------------------               */
  i_file = strdup (argv[NbArgs++]);

  /* --------------| FICHIER DE CARDINALITE (SOM) |-------------------------- */
  /*                ------------------------------                            */
  icard_file = strdup (argv[NbArgs++]);


  /* --------------| FICHIER DE VARIANCES (PRSOM) |-------------------------- */
  /*                ------------------------------                            */
  if ((argc - NbArgs) > 1)
    ivar_file = strdup (argv[NbArgs++]);


  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| CELLULES INDEXES POUR SOMLABEL |------------------------ */
  /*                --------------------------------                          */
  o_file = strdup (argv[NbArgs++]);

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        CAHclass\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Quel classifieur? ........................................ %s\n",
	   (classifier == 1) ? "SOM" : ((classifier == 2) ? "PRSOM" : NONDEFINI_MSG));
  fprintf (stdout, "      Combien de classes? ...................................... %d\n", level_int);
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
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      autres parametres :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Indice de calcul de distances .............. %ld\n", iopt);
  fprintf (stdout, "      Flag permettant l'ecrassement d'un fichier\n");
  fprintf (stdout, "          de sortie deja existant ................ %s\n", 
	   (flg_rewrite) ? ENABLED_MSG : DISABLED_MSG);
  fprintf (stdout, "      *******************************************************\n\n");

  if ( (iopt == 8) & !(ivar_file) ) {
    sprintf(BffMsg,"\n *** %s: variances file not defined. If you gives iopt = 8 you must also specify a variances file <VarFile> ***\n\n",
	    argv0);
    raise_error (argv0, BffMsg, ER_CRITICAL);
  }

  /*=======================================================*/
  icard_flag = 1;

  /* printf ("NewFile icard_file\n");   */
  NewFile (&cardFile, icard_file, FS_INPUT_MODE, "");
  LoadMap (&card, cardFile);
  FreeFile (&cardFile);

  n = card->range;
  index = (long *) malloc (n * sizeof (long));

/* -main_before_lib_call */
  /* Step 1: Open the input files */

  /* printf ("NewFile i_file\n");  */
  NewFile (&mapFile, i_file, FS_INPUT_MODE, "");
  /* printf ("LoadMap i_file\n"); */
  LoadMap (&map, mapFile);
  FreeFile (&mapFile);

  if (iopt == 8) {
    /*printf ("NewFile ivar_file\n");  */
    NewFile (&varFile, ivar_file, FS_INPUT_MODE, "");
    LoadMap (&var, varFile);
    FreeFile (&varFile);
  }

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

  ClassRef (n, m, len, iopt, data, level_int, ia, ib, crit, membr, sigma, init);

  /*   for (indice = 0 ; indice < n-1 ; indice ++)
     { printf ("indice %d ia %ld ib %ld \n", indice , ia[indice], ib[indice]);
     i = ia[indice]; 
     j = ib[indice]; 
     printf (" index[ia] %d index[ib] %d \n", index[i], index[j]);
     } 
   */

  /* la numerotation des classes commence a un */
  classDuNeuro = (int *) malloc (n * sizeof (int));

  for (i = 0; i < n; i++)
    classDuNeuro[i] = 1;

  AttribClass (n, level_int, ia, ib, classDuNeuro);

  /* Step 5: write the results into output files        */
  NewFile (&output, o_file, ( flg_rewrite ) ? FS_OVERWRITE_MODE : FS_OUTPUT_MODE, "");

  fprintf (GetFilePointer (output), "%s", "classes ");
  fprintf (GetFilePointer (output), "%d", level_int);
  fprintf (GetFilePointer (output), FS_END_OF_FIELD);
  /* fprintf( GetFilePointer(output), "%s", "nb neurones ");
     fprintf( GetFilePointer(output), "%d", n);
     fprintf( GetFilePointer(output), FS_END_OF_FIELD ); */
  IndexToStr (map->size, strCAH);
  fprintf (GetFilePointer (output), "%s", "mapSize ");
  fprintf (GetFilePointer (output), "%s", strCAH);
  fprintf (GetFilePointer (output), FS_END_OF_RECORD);
  for (i = 0; i < n; i++) {
    fprintf (GetFilePointer (output), "%d", classDuNeuro[i]);
    fprintf (GetFilePointer (output), FS_END_OF_RECORD);
  }

  FreeFile (&output);

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
  printf ("CAHclass termine\n\n");

  return (0);
}
