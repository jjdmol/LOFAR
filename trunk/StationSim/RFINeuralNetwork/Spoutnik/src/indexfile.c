/*
   ce programme a pour but de convertir un fichier de probabilite
   stocke de facon matricielle en fichier indexe. Il stocke uniquement
   les valeurs differents de zero (ou superieurs a un seuil). Le
   format par ligne est le suivant :

   NbValDiffZero   INDEX1 VALEUR1  INDEX2 VALEUR2 ...

   ou :

   - NbValDiffZero et le nombre de valeurs differents de zero pour la
   ligne (le pattern) en cours.

   - INDEX* est l'indice de la colonne du valeur different de
   zero. Les indices sont deffinis dans l'intervalle [1 .. P] ou P est
   le nombre de colonnes (de cellules de la MAP)

   - VALEUR* est la valeur proprement de la variable stockee.

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "prsom.h"

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr, "\t%s [options]  <ActFile> <OutFile>\n",
	   CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<ActFile> ........ activation de la forme x dans les lois normales\n");
  fprintf (stderr, "\t                   des referents (produit par PRSOM sous forme matricielle.\n");
  fprintf (stderr, "\t<OutFile> ........ sortie : activation de la forme x sous formme indexee.\n");
  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\t\t(pas d'options pour l'instant)\n");
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
    raise_error ("indexfile", "nombre de parametres invalide", ER_CRITICAL);
  }

  return 0;
}

int
main (int argc, char **argv)
{

  /* -main_variable_list */

  char           *act_file;		       /*  FILENAME */
  char           *o_file;		       /*  FILENAME */

  char            record[FS_RECORD_SIZE];
  char           *field;
  int             i, icol, nb_elem;
  double          val;
  double          tval[BFFSIZE];
  int             ival[BFFSIZE];
  double          SeuilProbaMin = SEUILPROBAMIN;
  
  FILE           *fpIn;
  FILE           *fpOut;

  int             NbExpectedArgs = 2;
  char           *argv0 = argv[0];
  int             NbArgs;

/*  main  */

  ++argv;
  --argc;

  if (!argc) {
    argc = non_interactive_read_arguments (argv0, &argv);
  }

  while (argc && argv[0][0] == '-') {
    NbArgs = test_set_options (argv0, argv);
    argv += NbArgs;
    argc -= NbArgs;
  }

  if (argc < NbExpectedArgs) {
    print_call (argv0);
    raise_error ("indexfile", "invalid argument number", ER_CRITICAL);
  }

  /* ======================================================================== *
   *                     INITIALISATION  DES PARAMETRES                       *
   * ======================================================================== */

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| ACTIVATION DE LA FORME X (MATRICIELLE) |---------------- */
  /*                ----------------------------------------                  */
  act_file = strdup (argv[0]);


  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| ACTIVATION DE LA FORME X (INDEXEE) |-------------------- */
  /*                ------------------------------------                      */
  o_file = strdup (argv[1]);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        INDEXFILE\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier d'activation de la forme X (MAT) ... %s\n", act_file);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier d'activation de la forme X (INDEX) . %s\n", o_file);
  fprintf (stdout, "\n      *******************************************************\n");

  /* ======================================================================== */

  /* ======================================================================== */

  fpIn = fopen(act_file, "r");
  if (!fpIn)
    raise_error ("indexfile", "no input file ?", ER_CRITICAL);

  if ( fgets (record, FS_RECORD_SIZE, fpIn) == NULL)
    raise_error ("indexfile", "empty input file ?", ER_CRITICAL);

  if ( record[0] == '@')
    raise_error ("indexfile", "input file is allready indexed", ER_CRITICAL);

  if ( record[0] != '#')
    raise_error ("indexfile", "unknown type for input file", ER_CRITICAL);

  fpOut = fopen(o_file, "r");
  if (fpOut) {
    fclose(fpOut);
    raise_error ("indexfile", "output file exist. Erase it or give another name", ER_CRITICAL);
  }

  fpOut = fopen(o_file, "w");
  if (!fpOut)
    raise_error ("indexfile", "ostrange! cannot create output file exist", ER_CRITICAL);

  record[0] = '@';

  fputs (record, fpOut);
  fflush(fpOut);

  while (1) {

    if (fgets (record, FS_RECORD_SIZE, fpIn) == NULL)
      break;

    field = record;
    field += strspn (field, FS_WHITE_SPACES);
    if ( field[strlen(field)-1] == '\n')
      field[strlen(field)-1] = '\0';

    icol = 0;
    nb_elem = 0;
    while (strlen(field) > 0) {
      icol ++;

      sscanf (field, "%lf", &val);
      field += strspn (field, "+-.0123456789");
      field += strspn (field, FS_WHITE_SPACES);

      if ( val > SeuilProbaMin ) {
	tval[nb_elem] = val;
	ival[nb_elem] = icol;
	++nb_elem;
      }
    }
    fprintf(fpOut, "%d", nb_elem);

    for (i = 0; i < nb_elem; ++i)
      fprintf (fpOut, "%s%d %e", FS_END_OF_FIELD, ival[i], tval[i]);

    fprintf (fpOut, FS_END_OF_RECORD);
  }

  fclose(fpIn);
  fclose(fpOut);

  printf ("indexproba termine\n");

  return (0);
}
