/*
   * Programme destine a tourner sous Unix

   Dans cette version de Psom, l'algorithme recoit une carte deja deployee par som.
   Les moyennes sont figees. Seules les variances sont modifiees.
   (ajout de l'indicateur psom->ComputeMoyenne).
   Cet algorythme doit effectuer peu de cycles, avec une distance de lissage reduite.
   La mise a jour des variances est limitee aux cellules dont l'eloignement est
   inferieur ou egal a la distance de lissage.  
   (modif dans spoutnik CyclePsom
   if (d <= 3 * som->smoothDistance) devient if (d <= som->smoothDistance)     

   modification du LoadPsom de facon a ne charger que la carte des poids tout en 
   conservant l'initialisation automatique des cartes de variations et d'activation 
 */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>> 
   >>>>         Main program for klearn_psom
   >>>> 
   >>>>  Private: 
   >>>>         main
   >>>> 
   >>>>   Static: 
   >>>>   Public: 
   >>>> 
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

/*------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "prsom.h"

#define BFFSIZE 1024
#define NONDEFINI "NON DEFINI"

/*
 * Variables Globales
 */

/* ------------------| FICHIER TEST |---------------------------------------------- */
char           *testSetFileName = NULL;

/* ------------------| inputDevFileName |------------------------------------------ */
/* carte des variances a reprendre                                                  */
char           *inputDevFileName = NULL;

/* ------------------| DIMENSION DE LA CARTE |------------------------------------- */
long            mapDim = 2;

/* ------------------| TAILLE DE LA CARTE |---------------------------------------- */
char           *mapsize_string = "{10,10}";

/* ------------------| mapmin and mapmax |----------------------------------------- */
char           *mapmin_string = NULL;
char           *mapmax_string = NULL;
int             mapmin_string_ok = 0;
int             mapmax_string_ok = 0;
double          mapmin_value = -1.0;
double          mapmax_value = 1.0;
vector_t        mapMin = NULL;
vector_t        mapMax = NULL;

/* ------------------| mapmax_string |--------------------------------------------- */

/* ------------------| NOMBRE DE CYCLES MAXIMUM DEMANDE |-------------------------- */
/* Pour le calcul de la variance */
long            nbCycleVar  = 50;

/* Pour le calcul de la moyenne */
long            nbCycleMean = 200;

/* ------------------| FREQUENCE DES TESTS |--------------------------------------- */
/* Pour le calcul de la variance */
long            freqTestVar = 1;

/* Pour le calcul de la moyenne */
long            freqTestMean = 10;

/* ------------------| SMOOTHDISTANCE Max & Min |---------------------------------- */
/* smoothdistance value gere le voisinage pour le calcul de la
   variance. Pas de valeur minimum, la valeur reste constante */
scalar_t        smoothDValVar = 3;

/* Pour le calcul de la moyenne */
scalar_t        smoothDMaxMean = 1;
scalar_t        smoothDMinMean = 0.1;

/* -----------------------------| seuil du voisinage |----------------------------- */
/* au-dela de cette distance le voisinage ne compte plus                            */
scalar_t        ThresholdNeighborVar  = SomThresholdNeighborhood;
scalar_t        ThresholdNeighborMean = SomThresholdNeighborhood;

/* ------------| INDICATEUR POUR SAUVEGARDER LES POIDS SI BEST RMS |--------------- */
int             traceBRmsOnOff = OFF;
char           *BRmsFileName = NULL;
int             refreshBRmsFrequency = 0;

/* ------------| INDICATEUR POUR SAUVEGARDER LES POIDS SI BEST CONTRAST |---------- */
int             traceBContrastOnOff = OFF;
char           *BContrastFileName = NULL;
int             refreshBContrastFrequency = 0;

/* ------------| FREQUENCE POUR SAUVEGARDER A TOUTES LES ITERATIONS DE TEST |------ */
int             refreshAllIterFrequency = 0;
char           *AllIterFileName = NULL;

/* ------------| FLAG POUR PERMETTE LA REECRITURE SUR SORTIES EXISTANTES |--------- */
int             overwriteOutFilesOk = OFF;

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr, "\t%s [options] <LearnFile> <MapFile> <FilePrefix>\n", CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<LearnFile> ....... fichier de formes a classer.\n");
  fprintf (stderr, "\t<MapFile> ......... fichier des poids issue de SOM.\n");
  fprintf (stderr, "\t<FilePrefix> ...... prefix des fichiers de sortie qui seront :\n");
  fprintf (stderr, "\t     - <FilePrefix>_PRSOM_Wei.map ........ poids de la carte.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_PRSOM_WeiCard.map .... poids + cardinalite pour Sammon.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_PRSOM_WeiVar.map ..... poids + variance pour Sammon.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_PRSOM_Card.res ....... cardinalite chaque neurone.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_PRSOM_Var.res ........ variance de chaque neurone.\n");
  fprintf (stderr, "\t     - <FilePrefix>_PRSOM_WFEP.res ....... neurone gagnant par forme.\n");
  fprintf (stderr, "\t     - <FilePrefix>_PRSOM_Acti.res ....... activation de chaque exemple\n");
  fprintf (stderr, "\t       dans la fonction densite de chaque neurone.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\t\t-test-file <TestSetFileName> :\n");
  fprintf (stderr, "\t\t                 to indicate the name of the\n");
  fprintf (stderr, "\t\t                 file to test. Default = NULL.\n");
  fprintf (stderr, "\t\t-var-file <VariancesFileName> :\n");
  fprintf (stderr, "\t\t                 to indicate the name of the variances\n");
  fprintf (stderr, "\t\t                 map file to test. Default = NULL.\n");
  fprintf (stderr, "\t\t-map-size <MapDim> <MapSize1> .. <MapSizeN> :\n");
  fprintf (stderr, "\t\t                 Map dimension and size.\n");
  fprintf (stderr, "\t\t                 Default = 2 dimensions, 10x10\n");
  fprintf (stderr, "\t\t-map-min-val <MapMinValue> :\n");
  fprintf (stderr, "\t\t                 Real value with min map value. This will be\n");
  fprintf (stderr, "\t\t                 the same for all dimensions of input data.\n");
  fprintf (stderr, "\t\t                 Default = %f\n", mapmin_value);
  fprintf (stderr, "\t\t-map-min <MapMinString> :\n");
  fprintf (stderr, "\t\t                 string with min map value. Give as many\n");
  fprintf (stderr, "\t\t                 values as dimensions of input data.\n");
  fprintf (stderr, "\t\t                 Default is related to <MapMinValue> default value.\n");
  fprintf (stderr, "\t\t-map-max-val <MapMaxValue> :\n");
  fprintf (stderr, "\t\t                 Real value with max map value. This will be\n");
  fprintf (stderr, "\t\t                 the same for all dimensions of input data.\n");
  fprintf (stderr, "\t\t                 Default = %f\n", mapmax_value);
  fprintf (stderr, "\t\t-map-max <MapMaxString> :\n");
  fprintf (stderr, "\t\t                 string with max map value. Give as many\n");
  fprintf (stderr, "\t\t                 values as dimensions of input data\n");
  fprintf (stderr, "\t\t                 Default is related to <MapMaxValue> default value.\n");
  fprintf (stderr, "\t\t-cycles-var <NbCyclesVar> <FrequenceTestsVar> :\n");
  fprintf (stderr, "\t\t                 Number of learning cycles and test frequency (in cycles)\n");
  fprintf (stderr, "\t\t                 for Variance computation. Default = [%ld, %ld].\n", nbCycleVar, freqTestVar);
  fprintf (stderr, "\t\t-smooth-dist-var <SmoothDistanceValVar> :\n");
  fprintf (stderr, "\t\t                 Value of smooth distance for Variance\n");
  fprintf (stderr, "\t\t                 computation. Default = %.1f.\n", smoothDValVar);
  fprintf (stderr, "\t\t-cycles-mean <NbCyclesMean> <FrequenceTestsMean> :\n");
  fprintf (stderr, "\t\t                 Number of learning cycles and test frequency (in cycles)\n");
  fprintf (stderr, "\t\t                 for Mean computation. Default = [%ld, %ld].\n", nbCycleMean, freqTestMean);
  fprintf (stderr, "\t\t-smooth-dist-mean <SmoothDistanceMeanMaximum> <SmoothDistanceMeanMinimum> :\n");
  fprintf (stderr, "\t\t                 Maximal and minimal value of smooth\n");
  fprintf (stderr, "\t\t                 distance for Mean computation. Default = [%.1f, %.1f].\n", smoothDMaxMean, smoothDMinMean);
  fprintf (stderr, "\t\t-threshold-neigh-var <SomThresholdNeighborVar> :\n");
  fprintf (stderr, "\t\t-threshold-neigh-mean <SomThresholdNeighborMean> :\n");
  fprintf (stderr, "\t\t                 indicates, for Variance and MEAN cycles,\n");
  fprintf (stderr, "\t\t                 the threshold, farther than that,\n");
  fprintf (stderr, "\t\t                 neighborhood doesnt counts.\n");
  fprintf (stderr, "\t\t                 Default <SomThresholdNeighborVar> = %f\n", ThresholdNeighborVar);
  fprintf (stderr, "\t\t                 Default <SomThresholdNeighborMean> = %f\n", ThresholdNeighborMean);
  fprintf (stderr, "\t\t-best-contrast <Frequency> :\n");
  fprintf (stderr, "\t\t                 Activates flag to allow to save weights if\n");
  fprintf (stderr, "\t\t                 best contrast. Default = %d.\n", refreshBContrastFrequency);
  fprintf (stderr, "\t\t-best-rms <Frequency> :\n");
  fprintf (stderr, "\t\t                 Activates flag to allow to save weights if\n");
  fprintf (stderr, "\t\t                 best RMS. Default = %d.\n", refreshBRmsFrequency);
  fprintf (stderr, "\t\t-trace-map <Frequency> :\n");
  fprintf (stderr, "\t\t                 Activates flag to allow to save weights at\n");
  fprintf (stderr, "\t\t                 each <Frequency> iterations. Default = %d.\n", refreshAllIterFrequency);
  fprintf (stderr, "\t\t-overwrite :\n");
  fprintf (stderr, "\t\t                 Flag to allow overwriting all output files if already\n");
  fprintf (stderr, "\t\t                 exists. Default = %s.\n", (overwriteOutFilesOk == OFF) ? "OFF" : "ON");
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
  if (!strcmp (Argument[0], "-test-file")) {   /* -TEST-FILE */
    testSetFileName = strdup (Argument[1]);
    if (testSetFileName == (char *) NULL)
      raise_error ("prsom", "strdup test-file: malloc failure", ER_CRITICAL);
    return 2;

  } else if (!strcmp (Argument[0], "-var-file")) {	/* -VAR-FILE */
    inputDevFileName = strdup (Argument[1]);
    if (inputDevFileName == (char *) NULL)
      raise_error ("prsom", "strdup var-file: malloc failure", ER_CRITICAL);
    return 2;

  } else if (!strcmp (Argument[0], "-map-size")) {	/* -MAP-SIZE */
    register int    i, nret = 1;
    int             vdim;
    char            TokenBff[1024], Bff[256];

    mapDim = OurAToL (Argument[nret++]);
    vdim = OurAToL (Argument[nret++]);
    sprintf (TokenBff, "{%d", vdim);
    for (i = 1; i < mapDim; ++i) {
      vdim = OurAToL (Argument[nret++]);
      sprintf (Bff, ",%d", vdim);
      strcat (TokenBff, Bff);
    }
    strcat (TokenBff, "}");

    mapsize_string = strdup (TokenBff);
    if (mapsize_string == (char *) NULL)
      raise_error ("prsom", "strdup map-size: malloc failure", ER_FAIL);

    return nret;

  } else if (!strcmp (Argument[0], "-map-min-val")) {	/* -MAP-MIN-VAL */
    mapmin_value = OurAToF (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-map-max-val")) {	/* -MAP-MAX-VAL */
    mapmax_value = OurAToF (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-map-min")) {	/* -MAP-MIN */
    mapmin_string = strdup (Argument[1]);
    if (mapmin_string == (char *) NULL)
      raise_error ("prsom", "strdup map-min: malloc failure", ER_FAIL);
    mapmin_string_ok = 1;
    return 2;

  } else if (!strcmp (Argument[0], "-map-max")) {	/* -MAP-MAX */
    mapmax_string = strdup (Argument[1]);
    if (mapmax_string == (char *) NULL)
      raise_error ("prsom", "strdup map-max: malloc failure", ER_FAIL);
    mapmax_string_ok = 1;
    return 2;

  } else if (!strcmp (Argument[0], "-cycles-var")) {	/* -CYCLES-VAR */
    nbCycleVar = OurAToL (Argument[1]);
    freqTestVar = OurAToL (Argument[2]);
    return 3;

  } else if (!strcmp (Argument[0], "-smooth-dist-var")) { /* -SMOOTH-DIST-VAR */
    smoothDValVar = (scalar_t) OurAToF (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-cycles-mean")) { /* -CYCLES-MEAN */
    nbCycleMean = OurAToL (Argument[1]);
    freqTestMean = OurAToL (Argument[2]);
    return 3;

  } else if (!strcmp (Argument[0], "-smooth-dist-mean")) { /* -SMOOTH-DIST-MEAN */
    smoothDMaxMean = (scalar_t) OurAToF (Argument[1]);
    smoothDMinMean = (scalar_t) OurAToF (Argument[2]);
    return 3;

  } else if (!strcmp (Argument[0], "-threshold-neigh-var")) {  /* -THRESHOLD-NEIGH-VAR */
    ThresholdNeighborVar = (scalar_t) OurAToF (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-threshold-neigh-mean")) {  /* -THRESHOLD-NEIGH-MEAN */
    ThresholdNeighborMean = (scalar_t) OurAToF (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-best-contrast")) { /* BEST-CONTRAST */
    refreshBContrastFrequency = OurAToL (Argument[1]);
    traceBContrastOnOff = ON;
    return 2;

  } else if (!strcmp (Argument[0], "-best-rms")) {      /* BEST-RMS */
    refreshBRmsFrequency = OurAToL (Argument[1]);
    traceBRmsOnOff = ON;
    return 2;

  } else if (!strcmp (Argument[0], "-trace-map")) {      /* TRACE-MAP */
    refreshAllIterFrequency = OurAToL (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-overwrite")) {    /* OVERWRITE */
    overwriteOutFilesOk = ON;
    return 1;

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
  register int    i;			       /* Loop Index */
  int             NotStable;
  char            BffFile[BFFSIZE];
  char           *FileNamePrefix = NULL;
  char           *inputMapFileName;	       /* carte des poids issue du Som       */
  char           *learnSetFileName;	       /* formes a classer                   */
  char           *outputMapFileName;	       /* poids de la carte                  */
  char           *outputMapCardName;	       /* poids + cardinalites pour sammon   */
  char           *outputMapDeviName;	       /* poids + variances    pour sammon   */
  char           *outputDevFileName;	       /* variance associee a chaque neurone */
  char           *outputCardFileName;	       /* cardinalite uniquement             */
  char           *outputWinFileName;	       /* neurone gagnant pour chaque forme presentee */
  char           *outputProbaName;	       /* probabilite d'appartenance a la loi normale

					          de chaque referent pour chaque forme presentee */

  long            pointDim;
  long            entreeSize;

  mapindex_t      mapSize = NULL;

  vector_t        minmax = NULL;

  psom_t          net = NULL;
  set_t           tmpSet = NULL;
  mapDesc_t      *card;

  file_t          netFile = NULL;
  file_t          devFile = NULL;
  file_t          learnFile = NULL;
  file_t          testFile = NULL;
  file_t          cardFile = NULL;
  file_t          resFile = NULL;
  file_t          BContrastFile = NULL;
  file_t          BRmsFile = NULL;
  file_t          AllIterFile = NULL;

  double          LastRms;
  double          OldRms = 0;
  double          OldContrast = 0;

  double          BestContrast;
  double          BestRMS;
  double          CurrentContrast;
  double          CurrentRMS;

  double          init_clock;
  double          used_time;

  int             NbExpectedArgs = 3;
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
    raise_error (argv0, "invalid argument number", ER_CRITICAL);
  }

  /* ======================================================================== *
   *                     INITIALISATION  DES PARAMETRES                       *
   * ======================================================================== */

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| FORMES A CLASSER |-------------------------------------- */
  /*                ------------------                                        */
  learnSetFileName = strdup (argv[0]);

  /* --------------| CARTE DES POIDS ISSUE DE SOM |-------------------------- */
  /*                ------------------------------                            */
  inputMapFileName = strdup (argv[1]);

  /* --------------| PREFIX DES FICHIERS |----------------------------------- */
  /*                ---------------------                                     */
  FileNamePrefix = strdup (argv[2]);

  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| POIDS DE LA CARTE |------------------------------------- */
  /*                -------------------                                       */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_Wei.map");
  outputMapFileName = strdup (BffFile);

  /* --------------| POIDS + CARDINALITE POUR SAMMON |----------------------- */
  /*                ---------------------------------                         */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_WeiCard.map");
  outputMapCardName = strdup (BffFile);

  /* --------------| POIDS + VARIANCE POUR SAMMON |-------------------------- */
  /*                ------------------------------                            */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_WeiVar.map");
  outputMapDeviName = strdup (BffFile);

  /* --------------| CARDINALITE CHAQUE NEURONE |---------------------------- */
  /*                ----------------------------                              */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_Card.res");
  outputCardFileName = strdup (BffFile);

  /* --------------| VARIANCE DE CHAQUE NEURONE |---------------------------- */
  /*                ----------------------------                              */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_Var.res");
  outputDevFileName = strdup (BffFile);

  /* --------------| NEURONE GAGNANT POUR CHAQUE FORME PRESENTEE |----------- */
  /*                ---------------------------------------------             */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_WFEP.res");
  outputWinFileName = strdup (BffFile);

  /* --------------| ACTIVATION DE CHAQUE EXEMPLE DANS LA FONCTION |--------- *
   *               | DENSITE DE CHAQUE NEURONE                     |          */
  /*                -----------------------------------------------           */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_Acti.res");
  outputProbaName = strdup (BffFile);

  /* ************************************************************************ *
   *                          SORTIES OPTIONNELLES                            *
   * ************************************************************************ */

  /* --------------| POIDS SI BEST CONTRAST |-------------------------------- */
  /*                ------------------------                                  */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_BCTR_Wei.map");
  BContrastFileName = strdup (BffFile);

  /* --------------| POIDS SI BEST RMS |------------------------------------- */
  /*                -------------------                                       */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_BRMS_Wei.map");
  BRmsFileName = strdup (BffFile);

  /* --------------| POIDS SI TRACE MAP |------------------------------------ */
  /*                --------------------                                      */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_PRSOM_Wei_it");
  AllIterFileName = strdup (BffFile);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        PRSOM\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de formes a classer ................ %s\n",
	   (learnSetFileName) ? learnSetFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier des poids issue de SOM ............. %s\n",
	   (inputMapFileName) ? inputMapFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier test ............................... %s\n",
	   (testSetFileName) ? testSetFileName : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier poids de la carte .................. %s\n",
	   (outputMapFileName) ? outputMapFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier poids + cardinalite pour sammon .... %s\n",
	   (outputMapCardName) ? outputMapCardName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier poids + variance pour sammon ....... %s\n",
	   (outputMapDeviName) ? outputMapDeviName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier cardinalite par neurone ............ %s\n",
	   (outputCardFileName) ? outputCardFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier variance par neurone ............... %s\n",
	   (outputDevFileName) ? outputDevFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier neurone gagnant .................... %s\n",
	   (outputWinFileName) ? outputWinFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier fonction densite (Probabilite) ..... %s\n",
	   (outputProbaName) ? outputProbaName : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      autres parametres :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Dimension de la carte ...................... %ld\n", mapDim);
  fprintf (stdout, "      Taille de la carte ......................... %s\n", mapsize_string);
  if (mapmin_string_ok)
    fprintf (stdout, "      Map min string ............................. %s\n", mapmin_string);
  else
    fprintf (stdout, "      Map min value .............................. %f\n", mapmin_value);
  if (mapmax_string_ok)
    fprintf (stdout, "      Map max string ............................. %s\n", mapmax_string);
  else
    fprintf (stdout, "      Map max value .............................. %f\n", mapmax_value);
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      *            Cycle de calcul des variances            *\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Nombre de cycles maximum ................... %ld\n", nbCycleVar);
  fprintf (stdout, "      Frequence Des Tests (nb. de cycles) ........ %ld\n", freqTestVar);
  fprintf (stdout, "      smoothDVal ................................. %f\n", smoothDValVar);
  fprintf (stdout, "      Seuil de voisinage ......................... %f\n", ThresholdNeighborVar);
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      * Cycle de mise a jour des variances et des moyennes  *\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Nombre de cycles maximum ................... %ld\n", nbCycleMean);
  fprintf (stdout, "      Frequence Des Tests (nb. de cycles) ........ %ld\n", freqTestMean);
  fprintf (stdout, "      smoothDMax ................................. %f\n", smoothDMaxMean);
  fprintf (stdout, "      smoothDMin ................................. %f\n", smoothDMinMean);
  fprintf (stdout, "      Seuil de voisinage ......................... %f\n", ThresholdNeighborMean);
  fprintf (stdout, "      Sauvegarder des poids si best contrast ..... %s\n",
	   (traceBContrastOnOff == OFF) ? "OFF" : "ON");
  if (traceBContrastOnOff) {
    fprintf (stdout, "      Fichier poids si best contrast ............. %s\n",
	     (BContrastFileName) ? BContrastFileName : NONDEFINI_MSG);
    fprintf (stdout, "      Frequence de test/sauvegarde best contrast . %d\n",
	     refreshBContrastFrequency);
  }
  fprintf (stdout, "      Sauvegarder des poids si best RMS .......... %s\n",
	   (traceBRmsOnOff == OFF) ? "OFF" : "ON");
  if (traceBRmsOnOff) {
    fprintf (stdout, "      Fichier poids si best RMS .................. %s\n",
	     (BRmsFileName) ? BRmsFileName : NONDEFINI_MSG);
    fprintf (stdout, "      Frequence de test/sauvegarde de best RMS ... %d\n",
	     refreshBRmsFrequency);
  }
  fprintf (stdout, "      Sauvegarder des poids a chaque iteration ... %s\n",
	   (refreshAllIterFrequency == 0) ? "OFF" : "ON");
  if (refreshAllIterFrequency) {
    fprintf (stdout, "      Fichier des poids chaque iteration ......... %s_XXXXXX.map\n",
	     (AllIterFileName) ? AllIterFileName : NONDEFINI_MSG);
    fprintf (stdout, "      Frequence de sauvegarde des poids .......... %d\n",
	     refreshAllIterFrequency);
  }
  fprintf (stdout, "      Forcer reecriture si fichiers existants .... %s\n",
	   (overwriteOutFilesOk == OFF) ? "OFF" : "ON");
  fprintf (stdout, "      *******************************************************\n\n");

/*============================================================================================*/

  /* ********************************************************************** *
   * Verification de l'existence des fichiers d'entree et de
   * l'inexistence des fichiers de sortie.
   * ********************************************************************** */

  /* ****************************** *
   * Verification des E N T R E E S
   * ****************************** */

  /* Loading the learning set */
  NewFile (&learnFile, learnSetFileName, FS_TEST_INPUT_MODE, "");

  /* Initialisation of the Map */
  if (inputMapFileName != NULL)
    NewFile (&netFile, inputMapFileName, FS_TEST_INPUT_MODE, "");

  if (testSetFileName != NULL)
    NewFile (&testFile, testSetFileName, FS_TEST_INPUT_MODE, "");

  /* ****************************** *
   * Verification des S O R T I E S
   * ****************************** */

  if (overwriteOutFilesOk) {
    fprintf (stdout, "\n\n      *******************************************************\n");
    fprintf (stdout, "      ***           OVERWRITE OUTPUT ENABLED              ***\n");
    fprintf (stdout, "      *******************************************************\n\n");
  } else {
    /* Save the resulting maps */
    NewFile (&netFile, outputMapFileName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition des variances  */
    NewFile (&devFile, outputDevFileName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition des cardinalites de chaque referent  */
    NewFile (&cardFile, outputCardFileName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition des poids + cardinalite de chaque referent pour Sammon  */
    NewFile (&cardFile, outputMapCardName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition des poids + variance de chaque referent pour Sammon  */
    NewFile (&cardFile, outputMapDeviName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition de la classe gagnante pour chaque element du set de test */
    NewFile (&netFile, outputWinFileName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition des probabilite d'appartenance de chaque forme aux lois
       normales representee par chaque referent */
    NewFile (&resFile, outputProbaName, FS_TEST_OUTPUT_MODE, "");
  }

/*=======================================================================*/
/* attention au flag VarieSmoothD */

/*************/
/* LEARN_SOM */
/*************/
  NewIndex (&mapSize, mapDim);
  StrToIndex (mapsize_string, mapSize);

  /* Loading the learning set */
  /*-----------------------*/
  NewFile (&learnFile, learnSetFileName, FS_INPUT_MODE, "");
  LoadSet (&tmpSet, learnFile);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      learnFile--file Name ....................... %s\n", learnFile->fileName);
  fprintf (stdout, "      learnFile--header .......................... %s\n", learnFile->header);
  FreeFile (&learnFile);

  /* Automatic setting of the dimension */
  /*------------------------------------------------------*/
  pointDim = (tmpSet)->dim;
  entreeSize = (tmpSet)->size;

  fprintf (stdout, "      Dimension des entrees ...................... %ld\n", pointDim);
  fprintf (stdout, "      Dimension de la carte ...................... %ld\n", entreeSize);
  fprintf (stdout, "      *******************************************************\n\n");

  /* Automatic setting of map min and max (used in NewSom) */
  /*------------------------------------------------------*/
  NewVector (&mapMin, pointDim);
  if (mapmin_string_ok) {
    int n_elem = StrToVector(mapmin_string,mapMin);
    int v_size = mapMin->dim;
    if (n_elem) {
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MAPMIN_STRING string and expected vector size: (%d < %d)",
	      n_elem - 1, v_size);
      raise_error (argv0, Bff, ER_CRITICAL);
    }
  } else
    for (i = 0; i < pointDim; i++)
      SetVectorValue (mapMin, i, mapmin_value);

  NewVector (&mapMax, pointDim);
  if (mapmax_string_ok) {
    int n_elem = StrToVector(mapmax_string,mapMax);
    int v_size = mapMax->dim;
    if (n_elem) {
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MAPMAX_STRING string and expected vector size: (%d < %d)",
	      n_elem - 1, v_size);
      raise_error (argv0, Bff, ER_CRITICAL);
    }
  } else
    for (i = 0; i < pointDim; i++)
      SetVectorValue (mapMax, i, mapmax_value);

/* Initialization of the Map */
/*---------------------------*/
  printf ("\n--initialisation de la map--\n");
  if (inputMapFileName != NULL) {
    NewFile (&netFile, inputMapFileName, FS_INPUT_MODE, "");
    /* NewFile( &devFile, inputDevFileName, FS_INPUT_MODE, "" ); */
    Load2Psom (&(net), netFile, devFile);      /* Loading the maps : mean and dev */
    FreeFile (&netFile);
    /* FreeFile( &devFile ); */
  } else {
    if ((pointDim == 0)
	|| (mapSize == NULL)
	|| (mapMin == NULL)
	|| (mapMax == NULL)
      )
      raise_error ("spoutnik", "Some map dimension not specified", ER_FAIL);
    else {
      NewPsom (&net, mapSize, mapMin, mapMax, pointDim);
      /*     
         NewFile( &NetFile, outputMapFileName, FS_OUTPUT_MODE, "" );
         SaveMap( net->map, netFile );
         FreeFile( &netFile );
       */
    }
  }

  printf ("\n--apres initialisation de la map--\n");

  /* Setting all the parameter of the learning and testing */
  /*-------------------------------------------------------*/

  if (net == NULL)
    raise_error ("spoutnik", "/data/psomamb/bin", ER_FAIL);
  else {
    net->som->learnSet = tmpSet;	       /* Specifying the learn set */
    tmpSet = NULL;

    NewSet (&(net->som->learnSetInfo), net->som->learnSet->size, SI_SIZE);
    net->som->learnSetInfo->stat->values[SET_BESTRMS] = MAXSCALAR;
    net->som->learnSetInfo->stat->values[SET_BESTCONTRAST] = MAXSCALAR;

    if (testSetFileName == NULL) {	       /*if no test set is specified, the learn set is used */
      net->som->testSet = (net->som->learnSet);
      net->som->testSetInfo = (net->som->learnSetInfo);
    } else {
      NewFile (&testFile, testSetFileName, FS_INPUT_MODE, "");
      LoadSet (&(net->som->testSet), testFile);
      FreeFile (&testFile);

      NewSet (&(net->som->testSetInfo), net->som->testSet->size, SI_SIZE);
      net->som->testSetInfo->stat->values[SET_BESTRMS] = MAXSCALAR;
      net->som->testSetInfo->stat->values[SET_BESTCONTRAST] = MAXSCALAR;
    }

  }
  Show ("\n%s\n", "Learning");
  /*-----------------------------*/
  /* CYCLES DE L'ALGORITHME PSOM  */
  /*-----------------------------*/

  /*      
     NewFile( &rmsFile, rmsFileName, FS_OUTPUT_MODE, "" );
     fprintf( GetFilePointer(rmsFile), "#\n" );
     FreeFile( &rmsFile );

   */

  /* initialisation carte des variances, carte d'activation et matrice des distances */
  InitPsom (&(net), net->som->learnSet);       /* Init the map using the learning set */
  Show ("\n%s\n", "apres InitPsom");

  /* pour voir l'initialisation des moyennes et variances */
  /* Show("\n%s\n","test initialisation");           
     NewFile( &netFile, outputMapFileName, FS_OUTPUT_MODE, "" );
     SaveMap( net->som->map, netFile );
     FreeFile( &netFile );
     
     NewFile( &devFile, outputDevFileName, FS_OUTPUT_MODE, "" );
     SaveMap( net->dev, devFile );
     FreeFile( &devFile );
     
     exit(3);
  */
  
  /* CALCUL DES VARIANCES
   * apres deploiement de la carte par som,
   * calcul des variances pour chaque referent, 
   * les moyennes sont figees, la distance de voisinage est fixe
   */
  /*===========================================*/
  /* CALCUL DES VARIANCES UNIQUEMENT           */
  /*===========================================*/
  net->ComputeVariance             = ON;
  net->ComputeMoyenne              = OFF;
  net->VarieSmoothD                = OFF;
  net->som->cycle                  = 0;
  net->som->nbCycle                = nbCycleVar;
  net->som->freqTest               = freqTestVar;
  net->som->smoothDistanceMax      = smoothDValVar;
  net->som->smoothDistanceMin      = 0;
  net->som->threshold_neighborhood = ThresholdNeighborVar;

  init_clock  = (double) clock ();

  OldRms      = 0;
  OldContrast = 0;

  NotStable   = 1;

  printf ("\n--CALCUL DES VARIANCES--\n");
  while (((net->som)->cycle < (net->som)->nbCycle) && NotStable) {
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* APPRENTISSAGE   */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    CyclePsom (net);

    /*It is stable when nothing is changing */
    NotStable = (((net->som->testSetInfo->stat->values[SET_RMS] != OldRms)
		  || (net->som->testSetInfo->stat->values[SET_CONTRAST] != OldContrast)));

    OldRms = net->som->testSetInfo->stat->values[SET_RMS];
    OldContrast = net->som->testSetInfo->stat->values[SET_CONTRAST];

    (net->som)->cycle++;
  }

  used_time = (((double) clock ()) - init_clock) / CLOCKS_PER_SEC;
  printf (" Cycle(%ld). Temps d'execution = %.6f s.\n", (net->som)->cycle, used_time);

  /* MISE AU POINT DES MOYENNES des referents, avec un voisinage tres
     petit et un pas d'apprentissage tres petit aussi les moyennes
     sont mises a jour, les variances aussi, le voisinage diminue */

  /*===========================================*/
  /* MISE A JOUR DES VARIANCES ET DES MOYENNES */
  /*===========================================*/
  net->ComputeVariance             = ON;
  net->ComputeMoyenne              = ON;
  net->VarieSmoothD                = ON;
  net->som->cycle                  = 0;
  net->som->nbCycle                = nbCycleMean;
  net->som->freqTest               = freqTestMean;
  net->som->smoothDistanceMax      = smoothDMaxMean;
  net->som->smoothDistanceMin      = smoothDMinMean;
  net->som->threshold_neighborhood = ThresholdNeighborMean;
  /*===========================================*/

  OldRms       = 0;
  OldContrast  = 0;

  BestContrast = MAXSCALAR;
  BestRMS      = MAXSCALAR;

  NotStable    = 1;

  /* sauvegarde du premier Map */
  if (refreshAllIterFrequency) {
    sprintf (BffFile, "%s%08d.map", AllIterFileName, 0);
    NewFile (&AllIterFile, BffFile, FS_OVERWRITE_MODE, "");
    SaveMap ((net->som)->map, &AllIterFile);
    FreeFile (&AllIterFile);
  }

  printf ("\n--CALCUL DES MOYENNES--\n");
  while (((net->som)->cycle < (net->som)->nbCycle) && NotStable) {
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* APPRENTISSAGE   */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    CyclePsom (net);

    /*It is stable when nothing is changing */
    NotStable = (((net->som->testSetInfo->stat->values[SET_RMS] != OldRms)
		  || (net->som->testSetInfo->stat->values[SET_CONTRAST] != OldContrast)));

    OldRms      = net->som->testSetInfo->stat->values[SET_RMS];
    OldContrast = net->som->testSetInfo->stat->values[SET_CONTRAST];

    /* ----- Meilleure RMS ----- */
    CurrentRMS  = (net->som)->testSetInfo->stat->values[SET_BESTRMS];

    /* --- Meilleur Contrast --- */
    CurrentContrast = (net->som)->testSetInfo->stat->values[SET_BESTCONTRAST];

    if ((refreshAllIterFrequency) &&
	(!(((net->som)->cycle + 1) % refreshAllIterFrequency))) {
      sprintf (BffFile, "%s%08ld.map", AllIterFileName, (net->som)->cycle + 1);
      NewFile (&AllIterFile, BffFile, FS_OVERWRITE_MODE, "");
      SaveMap ((net->som)->map, &AllIterFile);
      FreeFile (&AllIterFile);
    }

    if ((traceBContrastOnOff) &&
	(!(((int) (net->som)->cycle + 1) % refreshBContrastFrequency)) &&
	(CurrentContrast < BestContrast)) {
      fprintf (stdout, " *** iteration %ld ==> best contrast = %f\n", (net->som)->cycle, CurrentContrast);
      NewFile (&BContrastFile, BContrastFileName, FS_OVERWRITE_MODE, "");
      SaveMap ((net->som)->map, &BContrastFile);
      FreeFile (&BContrastFile);
    }

    if ((traceBRmsOnOff) &&
	(!(((int) (net->som)->cycle + 1) % refreshBRmsFrequency)) &&
	(CurrentRMS < BestRMS)) {
      fprintf (stdout, " *** iteration %ld ==> best RMS = %f\n", (net->som)->cycle, CurrentRMS);
      NewFile (&BRmsFile, BRmsFileName, FS_OVERWRITE_MODE, "");
      SaveMap ((net->som)->map, &BRmsFile);
      FreeFile (&BRmsFile);
      }

    (net->som)->cycle++;
  }

  used_time = 1000 * (clock () - init_clock) / CLOCKS_PER_SEC;
  printf (" Cycle(%ld). Temps d'execution = %.6f ms.\n", (net->som)->cycle, used_time);

/* SELECTION DU GAGNANT sans mise a jour de la variance, pour etablir les probabilites */

  /*===========================================*/
  /* SELECTION DU GAGNANT                       */
  /*===========================================*/
  net->ComputeVariance             = OFF;
  net->ComputeMoyenne              = OFF;
  net->VarieSmoothD                = OFF;
  net->som->cycle                  = 0;
  net->som->nbCycle                = 1;
  net->som->smoothDistanceMax      = 1.1;
  net->som->smoothDistanceMin      = 1;
  /*===========================================*/

  printf ("\n--SELECTION DU GAGNANT--\n");
  while ((net->som)->cycle < (net->som)->nbCycle) {
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* APPRENTISSAGE   */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    CyclePsom (net);

    (net->som)->cycle++;
  }

  /* sauvegarde de la carte */
  LastRms = net->som->testSetInfo->stat->values[SET_RMS];

  /* Save the resulting maps */
  /* edition des poids (moyennes) */
  Show ("\n%s\n", "New outputMap");
  NewFile (&netFile, outputMapFileName, FS_OVERWRITE_MODE, "");
  SaveMap (net->som->map, &netFile);
  FreeFile (&netFile);

  /* edition des variances  */
  Show ("\n%s\n", "New outputDev");
  NewFile (&devFile, outputDevFileName, FS_OVERWRITE_MODE, "");
  SaveMap (net->dev, &devFile);
  FreeFile (&devFile);

  /* edition des cardinalites de chaque referent  */
  Show ("\n%s\n", "New outputCard");
  NewFile (&cardFile, outputCardFileName, FS_OVERWRITE_MODE, "");
  NewVector (&minmax, 1);
  NewMap (&card, net->som->map->size, minmax, minmax, 1);
  tmpSet = (net->som->testSetInfo);
  ForAllElements (tmpSet, tmpSet->curElement) {
    card->curPoint = card->firstPoint + (long) (*(tmpSet->curElement))->values[SI_WINNEROFFSET];
    ((*(card->curPoint))->values[0]) += 1.0;
  }
  SaveMap (card, &cardFile);
  FreeFile (&cardFile);

  /* edition des poids + cardinalite de chaque referent pour Sammon  */
  Show ("\n%s\n", "New outputMapCard");
  NewFile (&cardFile, outputMapCardName, FS_OVERWRITE_MODE, "");

  /* fprintf( GetFilePointer(cardFile), "%s\n", "5 rect 7 13 gaussian" ); */
  fprintf (GetFilePointer (cardFile), "%ld %s %ld %ld %s\n", pointDim, "rect",
	   net->som->map->size->values[0], net->som->map->size->values[1], "gaussian");

  card->curPoint = (card)->firstPoint;
  (card)->offset = 0;
  ForAllPoints (net->som->map, net->som->map->curPoint) {
    for (i = 0; i < net->som->map->min->dim; i++) {
      fprintf (GetFilePointer (cardFile), "%f", (*(net->som->map->curPoint))->values[i]);
      fprintf (GetFilePointer (cardFile), FS_END_OF_FIELD);
    }

    fprintf (GetFilePointer (cardFile), "%1.0f", (*(card->curPoint))->values[0]);
    fprintf (GetFilePointer (cardFile), FS_END_OF_RECORD);

    ((card)->offset)++;
    (card->curPoint)++;
  }

  FreeFile (&cardFile);

/* edition des poids + variance de chaque referent pour Sammon  */
  Show ("\n%s\n", "New outputMapCard");
  NewFile (&cardFile, outputMapDeviName, FS_OVERWRITE_MODE, "");

  fprintf (GetFilePointer (cardFile), "%s\n", "5 rect 7 13 gaussian");

  net->dev->curPoint = (net->dev)->firstPoint;
  (net->dev)->offset = 0;
  ForAllPoints (net->som->map, net->som->map->curPoint) {
    for (i = 0; i < net->som->map->min->dim; i++) {
      fprintf (GetFilePointer (cardFile), "%f", (*(net->som->map->curPoint))->values[i]);
      fprintf (GetFilePointer (cardFile), FS_END_OF_FIELD);
    }

    fprintf (GetFilePointer (cardFile), "%1.2f", (*(net->dev->curPoint))->values[0]);
    fprintf (GetFilePointer (cardFile), FS_END_OF_RECORD);

    ((net->dev)->offset)++;
    (net->dev->curPoint)++;
  }

  FreeFile (&cardFile);

/* edition de la classe gagnante pour chaque element du set de test */
  Show ("\n%s\n", "New output WinFile");
  NewFile (&netFile, outputWinFileName, FS_OVERWRITE_MODE, "");

  fprintf (GetFilePointer (netFile), "%s%ld%s\n", "# mapDim=1 mapSize={", entreeSize,
	   "} pointDim=1 mapMin={0.0} mapMax={0.0}");

  ForAllElements (tmpSet, tmpSet->curElement) {
    fprintf (GetFilePointer (netFile), "%1.0f",
	     (*(tmpSet->curElement))->values[SI_WINNEROFFSET]);
    fprintf (GetFilePointer (netFile), FS_END_OF_RECORD);
  }
  FreeFile (&netFile);

  /* Save the resulting maps */

  /* edition des probabilite d'appartenance de chaque forme aux lois normales
     representee par chaque referent */
  Show ("\n%s\n", "New outputMap");
  NewFile (&resFile, outputProbaName, FS_OVERWRITE_MODE, "");
  SaveProbability (net, resFile);
  FreeFile (&resFile);

  Show ("\n%s", "----- Meilleure RMS -----");
  Show ("\n%f", net->som->testSetInfo->stat->values[SET_BESTRMS]);
  Show ("\n%s", "--- Meilleur Contrast ---");
  Show ("\n%f", net->som->testSetInfo->stat->values[SET_BESTCONTRAST]);
  Show ("\n%s\n", "-------------------------");

  return (0);
}
