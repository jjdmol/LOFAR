/* ce programme a pour but d'indexer les referents de la carte de Kohonen
   produite par l'algorithme psom de Philippe Daigremont.

   Le fichier .res cree par Prsom contient autant de lignes que de formes a classer.
   L'unique colonne du fichier contient le numero de referent auquel se rattache la forme.

   Le fichier labelA contient lui aussi autant de lignes que de formes a classer.
   Il possede 1 colonne pour la profondeur, 5 colonnes pour les mesures de
   diagraphie, 1 colonne pour la classe attribuee a la forme par les nuees 
   dynamiques, et une colonne pour la classe vraie quand elle est connue.

   Si une forme est labellisee, le programme recherche a quel le referent est associee
   la forme. Ainsi, on peut comptabiliser pour chaque referent les classes vraies 
   correspondantes. A partir de la, L'affectation d'une classe vraie (index) a un 
   referent se fait par vote majoritaire.

 */

/*

   parametres du somindex.c
   ------------------------

   fichiers d'entree (les 2 premiers sont crees par le som) :
   map_file       = "../fic/provm_30_10d2_mean.map";   neurones de la carte  
   res_file       = "../fic/pr-ovm_30_10d2.res";       neurone gagnant de chaque exemple
   ind_file       = "../fic/labelA_4.dat";             fichier de labels

   fichiers de sortie :
   o_file         = "../fic/provm_30_10d2_mean1.cal";  neurones indexes pour CAH
   o_samm         = "../fic/provm_30_10d2_sam1.cal";   neurones indexes pour Sammon
   o_labe         = "../fic/provm_30_10d2_label.cal";  fichier d'analyse pour le developpeur

   nClass         = 12;                                nombre de classes voulues
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "somindex.h"

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr,
	   "\t%s [options] <NbClass> <MapFile> <WinNeuronFile> <LabelsFile> <FilePrefix>\n",
	   CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<NbClass> ......... neurones de la carte.\n");
  fprintf (stderr, "\t<WinNeuronFile> ... neurone gagnant de chaque exemple.\n");
  fprintf (stderr, "\t<LabelsFile> ...... fichier de labels.\n");
  fprintf (stderr, "\t<FilePrefix> ...... prefix des fichiers de sortie qui seront :\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_INDEX_NCAH.res .... fichier neurones indexes pour CAH.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_INDEX_NSam.res .... fichier neurones indexes pour Sammon.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_INDEX_Ana.dat ..... fichier d'analyse pour le developpeur.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\n    pas d\'options pour SOMINDEX pour l\'instant\n");
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
  if (!strcmp (Argument[0], "-h")) {	       /* HELP */
    print_call (Argument0);
    exit (1);

  } else {
    print_call (Argument0);
    raise_error ("somindex", "nombre de parametres invalide", ER_CRITICAL);
  }

  return 0;
}

int
main (int argc, char **argv)
{

/* -main_variable_list */

  char            BffFile[BFFSIZE];
  char           *FileNamePrefix = NULL;

  file_t          mapFile;		       /* the map of the neurons (vectors of weight) */
  file_t          resFile;		       /* the number of the neuron wich the sample is associated to */
  file_t          indFile;		       /* the true classes that we know */
  file_t          output;		       /* the output file : the neurons and their index */
  file_t          outSam;		       /* map for Sammon representation  */
  file_t          outLab;		       /* number of the labels presents for each neuron  */

  char           *map_file = NULL;	       /*  FILENAME */
  char           *res_file = NULL;	       /*  FILENAME */
  char           *ind_file = NULL;	       /*  FILENAME */
  char           *o_file = NULL;	       /*  FILENAME */
  char           *o_samm = NULL;	       /*  FILENAME */
  char           *o_labe = NULL;	       /*  FILENAME */

  mapDesc_t      *map = NULL;		       /* the map to be indexed */
  mapDesc_t      *res = NULL;
  mapDesc_t      *ind = NULL;

  long            nRef;			       /* Number of neurons in the map (weight vectors) */
  long            nExempl;		       /* Number of samples to be classified */
  long            nClass;		       /* Number of true classes */
  long           *index;		       /* the real class we know for some of each sample */
  long           *referent;		       /* the referent of each sample */
  long           *referIndex;		       /* two dimension array. Number of each index for each weight vector */
  long           *DBGclassIndex;

  long           *indexDuRefer;		       /* Index of each weight vector, at the end of program */
  long            cl, r, i, f, max, indexMax;
  char            topol[10] = "rect";

/* char topol[10]   = "hexa"; *//* topology type used in the map */

  int             NbExpectedArgs = 5;
  char           *argv0 = argv[0];
  int             NbArgs;

  ++argv;
  --argc;
  while (argc && argv[0][0] == '-') {
    NbArgs = test_set_options (argv0, argv);
    argv += NbArgs;
    argc -= NbArgs;
  }

  if (argc < NbExpectedArgs) {
    print_call (argv0);
    raise_error ("somindex", "invalid argument number", ER_CRITICAL);
  }

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| NOMBRE DE CLASSES A VOULUES |--------------------------- */
  /*                -----------------------------                             */
  nClass = OurAToL (argv[0]);

  /* --------------| FICHIER CARTE DES POIDS ISSUE DE SOM |------------------ */
  /*                --------------------------------------                    */
  map_file = strdup (argv[1]);

  /* --------------| FICHIER NEURONE GAGNANT POUR CHAQUE FORME PRESENTEE |--- */
  /*                -----------------------------------------------------     */
  res_file = strdup (argv[2]);
  /* --------------| FICHIER DE LABELS |------------------------------------- */
  /*                -------------------                                       */
  ind_file = strdup (argv[3]);

  /* --------------| PREFIX DES FICHIERS |----------------------------------- */
  /*                ---------------------                                     */
  FileNamePrefix = strdup (argv[4]);

  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| FICHIER NEURONES INDEXES POUR CAH |--------------------- */
  /*                -----------------------------------                       */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_INDEX_NCAH.res");
  o_file = strdup (BffFile);

  /* --------------| FICHIER NEURONES INDEXES POUR SAMMON |------------------ */
  /*                --------------------------------------                    */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_INDEX_NSam.res");
  o_samm = strdup (BffFile);

  /* --------------| FICHIER D'ANALYSE POUR LE DEVELOPPEUR |----------------- */
  /*                ---------------------------------------                   */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_INDEX_Ana.dat");
  o_labe = strdup (BffFile);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        SOMINDEX\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier carte des poids issue de SOM ....... %s\n",
	   (map_file) ? map_file : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier neurone gagnant par forme .......... %s\n",
	   (res_file) ? res_file : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier des labels ......................... %s\n",
	   (ind_file) ? ind_file : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier neurones indexes pour CAH .......... %s\n",
	   (o_file) ? o_file : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier neurones indexes pour SAMMON ....... %s\n",
	   (o_samm) ? o_samm : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier d'analyse pour le developpeur ...... %s\n",
	   (o_labe) ? o_labe : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      autres parametres :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Nombre de classes voulues .................. %ld\n", nClass);
  fprintf (stdout, "      *******************************************************\n\n");

/* The numbers of the true classes begin at 1, so we need one more space
   in the referIndex array */
  cl = nClass + 1;

/* -main_before_lib_call */
  /* Step 1: Open the input files */
  /* load the map of weight vectors */
/*    printf ("newfile omean\n"); */
  NewFile (&mapFile, map_file, FS_INPUT_MODE, "");
  LoadMap (&map, mapFile);

/*    printf ("newfile psom.res\n");  */
  /* Load the psom result file */
  NewFile (&resFile, res_file, FS_INPUT_MODE, "");
  LoadMap (&res, resFile);

  /* Initialisation */
  /* le second indice du tabeau referIndex va de 1 a nClass, donc un poste de plus 
     dans le tableau referIndex */
  nRef = map->range;
  nExempl = res->range;
  index = (long *) malloc (nExempl * sizeof (long));
  referent = (long *) malloc (nExempl * sizeof (long));
  referIndex = (long *) malloc (nRef * cl * sizeof (long));
  indexDuRefer = (long *) malloc (nRef * sizeof (long));

  DBGclassIndex = (long *) malloc (cl * sizeof (long));

  /* Load the index file */
  /*printf ("newfile label\n");  */
  NewFile (&indFile, ind_file, FS_INPUT_MODE, "");
  LoadMapIndex (&ind, indFile, &index);

  /* Step 2: open output file */
  /* printf ("output o_file\n"); */
  NewFile (&output, o_file, FS_OUTPUT_MODE, "");
  NewFile (&outSam, o_samm, FS_OUTPUT_MODE, "");
  NewFile (&outLab, o_labe, FS_OUTPUT_MODE, "");

  /* printf ("step3\n"); */
  /* Step 3: */
  /* Copy the number of the neuron associated to each sample */
  ForAllPoints (res, res->curPoint) {
    referent[(res->offset)] = (*(res->curPoint))->values[0];
  }

  /* r = referent affecte a la forme f                                    */
  /* i = classe vraie connue pour f ou zero si la classe n'est pas connue */
  /* referIndex = tableau  1 ligne par referent, 1 colonne par classe     */
  /* 1 case (r,i) du tableau = nombre d'exemples labelles dans la         */
  /* classe i pour le referent r                                          */
  /* printf ("step4\n");  */
  /* Step 4: compute the number of each index associated to each neuron */
  for (r = 0; r < nRef; r++)
    for (i = 0; i < cl; i++)
      referIndex[i + r * cl] = 0;

  for (i = 0; i < cl; i++)
    DBGclassIndex[i] = 0;

  for (f = 0; f < nExempl; f++) {
    /* if ( index[f] != 0 ) { */
    r = referent[f];
    i = index[f];
    referIndex[i + r * cl]++;
    DBGclassIndex[i] ++;
    /*       printf ("f %d index[f] %d r %d \n", f, index[f], r ); */
    /*  } */
  }

  for (i = 0; i < cl; i++)
    fprintf(stdout,"nombre de points class %ld = %ld\n", i, DBGclassIndex[i]);

  /* choose the index of the neurons when we can (vote majoritaire) */
  for (r = 0; r < nRef; r++) {
    max = 0;
    indexMax = 0;
    /*  printf ("referent %d " , r);   */
    for (i = 1; i < cl; i++) {		       /* printf ("%ld ", referIndex[r,i]);   */
      if (referIndex[i + r * cl] > max) {
	max = referIndex[i + r * cl];
	indexMax = i;
      }
    }

    /*   printf ("\n");   */
    indexDuRefer[r] = indexMax;
    /*    printf ("refer %ld max %ld indexMax %ld\n", r, max, indexMax);     */
  }

  /* printf ("step5\n");  */
  /* Step5: write the map of weight vectors with the index if the neuron is indexed */
  SaveIndexMap (map, indexDuRefer, &output, NULL);
  SaveIndexSaMap (map, indexDuRefer, &outSam, topol, NULL);

  /* edit the number of each label for each neurons */
  for (r = 0; r < nRef; r++) {
    fprintf (GetFilePointer (outLab), " referent %ld", r);
    for (i = 1; i < cl; i++) {
      if (referIndex[i + r * cl] != 0)
	fprintf (GetFilePointer (outLab), " %ld lab %ld ", referIndex[i + r * cl], i);
    }
    if (referIndex[r * cl] != 0)
      fprintf (GetFilePointer (outLab), " %ld non labellises ", referIndex[r * cl]);

    fprintf (GetFilePointer (outLab), "\n");
  }

/*    printf ("\naffichage tableau index\n"); 
   for ( i=0; i < nExempl ; i++)
   if (index[i] != 0)
   printf ("i%d %d  ", i, index[i]); 
 */

/*    printf ("\n--N = %ld\t", n);
   printf ("--M = %ld\t", m);
 */

  /* free memory */
  FreeMap (&map);
  FreeFile (&mapFile);
  FreeMap (&res);
  FreeFile (&resFile);
  FreeMap (&ind);
  FreeFile (&indFile);
  FreeFile (&output);
  free (index);
  free (referent);
  free (referIndex);

  printf ("index termine\n\n");

  return (0);
}
