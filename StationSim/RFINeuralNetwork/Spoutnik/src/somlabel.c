/* ce programme a pour but de donner un label aux formes presentees
   a la classification par Som ou Psom apres que tous les referents
   de la carte de Kohonen aient ete labellises

   EN ENTREE :
   Le fichier des numeros de referents affectes aux formes a classer
   numerotes a partir de 0 (somun_91d2.res) .
   Le fichier des referents labellises (somun_io3_omean2_91d2.cal.

   En sortie, le fichier contient 1 seule colonne : le numero de classe associee
   a chaque forme.    

 */

/*

   parametres du somlabel.c
   ------------------------

   fichiers d'entree  :
   res_file       = "../fic/provm_30_10d2.res";            neurone gagnant pour chaque forme vient de som.c.
   map_file       = "../fic/provm_30_10d2_io1_mean2.cal";  neurones indexes par CAH.

   fichiers de sortie :
   o_file         = "../fic/provm_30_10d2_io1.res";        numero de classe attribue a chaque forme.

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "somlabel.h"

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr, "\t%s [options]  <WinnerFile> <CAHIndexFile>  <OutClassFile>\n", CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<WinnerFile> ..... neurones gagnants pour chaque forme vient de som.\n");
  fprintf (stderr, "\t<CAHIndexFile> ... neurones indexes par CAH.\n");
  fprintf (stderr, "\t<OutClassFile> ... sortie : numero de classe attribue a chaque forme.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\n    pas d\'options pour SOMLABEL pour l\'instant\n");
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
    raise_error ("somlabel", "nombre de parametres invalide", ER_CRITICAL);
  }

  return 0;
}

int
main (int argc, char **argv)
{

/* -main_variable_list */

  file_t          mapFile;		       /* the map of the neurons with their label */
  file_t          resFile;		       /* the number of the neuron wich the sample is associated to */
  file_t          output;		       /* the output file : the class of the sample */

  char           *map_file;		       /*  FILENAME */
  char           *res_file;		       /*  FILENAME */
  char           *o_file;		       /*  FILENAME */

  mapDesc_t      *map = NULL;
  mapDesc_t      *res = NULL;

  long            nRef;			       /* Number of neurons in the map (weight vectors) */
  long            dim;			       /* Number of variables describing each vector    */
  long            nExempl;		       /* Number of samples to be classified            */
  vector_t        label;		       /* the neuron wich each sample is associated to  */
  long           *referent;		       /* the referent of each sample                   */
  long           *classe;		       /* the class we assign to the samples            */
  long            refOfSample, entreeSize;
  long            f;

  int             NbExpectedArgs = 3;
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
    raise_error ("somlabel", "invalid argument number", ER_CRITICAL);
  }

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| NEURONE GAGNANT POUR CHAQUE FORME |--------------------- */
  /*                -----------------------------------                       */
  res_file = strdup (argv[0]);

  /* --------------| NEURONES INDEXES PAR CAH |------------------------------ */
  /*                --------------------------                                */
  map_file = strdup (argv[1]);

  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| NUMERO DE CLASSE ATTRIBUE A CHAQUE FORME |-------------- */
  /*                ------------------------------------------                */
  o_file = strdup (argv[2]);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        SOMLABEL\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de neurone gagnant pour chaque forme (SOM) .... %s\n",
	   res_file);
  fprintf (stdout, "      Fichier de neurones indexes par CAH ................... %s\n",
	   map_file);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de numero de classe attribue a chaque forme ... %s\n",
	   o_file);
  /*
     fprintf(stdout, "\n      *******************************************************\n");
     fprintf(stdout, "      autres parametres :\n");
     fprintf(stdout, "      *******************************************************\n");
     fprintf(stdout, "      Nombre de xxx yyy ..................................... %ld\n", xxxyyy);
   */
  fprintf (stdout, "      *******************************************************\n\n");

/* -main_before_lib_call */
  /* Step 1: Open the input files */
  /* load the psom result file */
  /* printf ("newfile psom.res\n");   */
  NewFile (&resFile, res_file, FS_INPUT_MODE, "");
  LoadMap (&res, resFile);
  entreeSize = (res)->range;

  /* Load the map of weight vectors */
  /* printf ("newfile omean.map\n");   */
  NewFile (&mapFile, map_file, FS_INPUT_MODE, "");
  LoadMapLabel (&map, mapFile, &label);

  /* Initialisation */
  nRef = map->range;
  dim = (*(map->firstPoint))->dim;
  nExempl = res->range;
  referent = (long *) malloc (nExempl * sizeof (long));
  classe = (long *) malloc (nExempl * sizeof (long));

  /* Step 3: */
  /* Copy the number of the neuron associated to each sample */
  ForAllPoints (res, res->curPoint) {
    referent[(res->offset)] = (long) (*(res->curPoint))->values[0];
  }

  /* Associate the label of the neuron to the sample */
  for (f = 0; f < nExempl; f++) {
    refOfSample = referent[f];
    classe[f] = (long) (label->values[refOfSample]);
  }

  /* Step5: write the class for each sample */

  NewFile (&output, o_file, FS_OUTPUT_MODE, "");

  fprintf (GetFilePointer (output), "%s%ld%s\n", "# mapDim=1 mapSize={", entreeSize,
	   "} pointDim=1 mapMin={0.0} mapMax={0.0}");

  for (f = 0; f < nExempl; f++) {
    fprintf (GetFilePointer (output), "%ld", classe[f]);
    fprintf (GetFilePointer (output), FS_END_OF_RECORD);
  }

  /* free memory */
  FreeMap (&map);
  FreeMap (&res);
  FreeFile (&mapFile);
  FreeFile (&resFile);
  FreeFile (&output);
/*                    
   free(label);   */
  free (referent);
  free (classe);

  printf ("label termine\n\n");

  return (0);
}
