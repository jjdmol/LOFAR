 /*
  * programme destine a tourner sous Unix
  *
  * >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
  >>>> 
  >>>>  Main program for klearn_som
  >>>> 
  >>>>  Private: 
  >>>>  main
  >>>> 
  >>>>   Static: 
  >>>>   Public: 
  >>>> 
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "som.h"

/*
 * Variables Globales
 */

/* ------------------------------|     FICHIER TEST      |------------------------- */
char           *testSetFileName = NULL;	       /* fichier test */

/* ----------| NULL SI NOUVELLE CARTE, SINON NOM DE LA CARTE A REPRENDRE |--------- */
char           *inputMapFileName = NULL;

/* -----------------------------| DIMENSION DE LA CARTE   |------------------------ */
long            mapDim = 2;

/* ------------------------------|   TAILLE DE LA CARTE  |------------------------- */
char           *mapsize_string = "{10,10}";

/* ------------------| mapmin and mapmax |--------------------------------- */
char           *mapmin_string = NULL;
char           *mapmax_string = NULL;
int             mapmin_string_ok = 0;
int             mapmax_string_ok = 0;
double          mapmin_value = -1.0;
double          mapmax_value = 1.0;
vector_t        mapMin = NULL;
vector_t        mapMax = NULL;

/* ------------------------------|  TEMPERATURE MAXIMUM  |------------------------- */
scalar_t        smoothDistanceMax = 5;

/* ------------------------------|  TEMPERATURE MINIMUM  |------------------------- */
scalar_t        smoothDistanceMin = 1;

/* -----------------------------| PAS D'APPRENTISSAGE MAX |------------------------ */
scalar_t        learningRateMax = 0.1;

/* -----------------------------| PAS D'APPRENTISSAGE MIN |------------------------ */
scalar_t        learningRateMin = 0.01;

/* -----------------------------| seuil du voisinage |----------------------------- */
/* au-dela de cette distance le voisinage ne compte plus                            */
scalar_t        ThresholdNeighborhood = SomThresholdNeighborhood;

/* ------------------------|  NOMBRE DE CYCLES MAXIMUM DEMANDE  |------------------ */
long            nbCycle = 2000;

/* -----------------------------| NUMERO DU CYCLE DEPART  |------------------------ */
long            cycle = 0;

/* -------------------------------| FREQUENCE DES TESTS |-------------------------- */
long            freqTest = 200;

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

/* ------------| FLAG POUR CALCUL DE TEMPERATURE EN MODE STOCHASTIQUE |------------ */
int             temperatureStochasticOk = OFF;

/* ------------| FLAG POUR APPRENTISSAGE EN MODE STOCHASTIQUE |-------------------- */
int             apprentissageBatchOk = OFF;

/* ------------| FLAG POUR PERMETTE LA REECRITURE SUR SORTIES EXISTANTES |--------- */
int             overwriteOutFilesOk = OFF;

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr, "\t%s [options] <LearnFile> <FilePrefix>\n", CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<LearnFile> ....... fichier de formes a classer.\n");
  fprintf (stderr, "\t<FilePrefix> ...... prefix des fichiers de sortie qui seront :\n");
  fprintf (stderr, "\t     - <FilePrefix>_SOM_Wei.map ........ poids de la carte.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_SOM_WeiCard.map .... poids + cardinalite pour Sammon.\n");
  fprintf (stderr, "\t     - <FilePrefix>_SOM_WFEP.res ....... neurone gagnant par forme.\n");
  fprintf (stderr,
	   "\t     - <FilePrefix>_SOM_Card.res ....... cardinalite de chaque neurone.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\t\t-test-file <TestSetFileName> :\n");
  fprintf (stderr, "\t\t                 to indicate the name of the\n");
  fprintf (stderr, "\t\t                 file to test. Default = NULL.\n");
  fprintf (stderr, "\t\t-old-map <InputMapFileName> :\n");
  fprintf (stderr, "\t\t                 to indicate the old map file\n");
  fprintf (stderr, "\t\t                 name to read. Default = NULL.\n");
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
  fprintf (stderr, "\t\t-temp <TemperatureMaximum> <TemperatureMinimum> :\n");
  fprintf (stderr, "\t\t                 Maximal (start) and minimal (end) temperature\n");
  fprintf (stderr, "\t\t                 or smooth values. Default = [5.0, 1.0].\n");
  fprintf (stderr, "\t\t-learn-rate <PasDApprentissageMaximum> <PasDApprentissageMinimum> :\n");
  fprintf (stderr, "\t\t                 Maximal (start) and minimal (end) learning step\n");
  fprintf (stderr, "\t\t                 values. Default = [0.10, 0.01].\n");
  fprintf (stderr, "\t\t-threshold-neigh <SomThresholdNeighborhood> :\n");
  fprintf (stderr, "\t\t                 indicates the threshold, farther than that,\n");
  fprintf (stderr, "\t\t                 neighborhood doesnt counts.\n");
  fprintf (stderr, "\t\t                 Default = %f\n", ThresholdNeighborhood);
  fprintf (stderr, "\t\t-cycles <NdCycles> <FrequenceTests> :\n");
  fprintf (stderr, "\t\t                 Number of learning cycles and\n");
  fprintf (stderr, "\t\t                 test frequency (in cycles). Default = [2000, 100].\n");
  fprintf (stderr, "\t\t-start-cycle <CycleDepart> :\n");
  fprintf (stderr, "\t\t                 Start cycle.\n");
  fprintf (stderr, "\t\t                 Default = 0.\n");
  fprintf (stderr, "\t\t-best-contrast <Frequency> :\n");
  fprintf (stderr, "\t\t                 Activates flag to allow to save weights if\n");
  fprintf (stderr, "\t\t                 best contrast. Default = %d.\n", refreshBContrastFrequency);
  fprintf (stderr, "\t\t-best-rms <Frequency> :\n");
  fprintf (stderr, "\t\t                 Activates flag to allow to save weights if\n");
  fprintf (stderr, "\t\t                 best RMS. Default = %d.\n", refreshBRmsFrequency);
  fprintf (stderr, "\t\t-trace-map <Frequency> :\n");
  fprintf (stderr, "\t\t                 Activates flag to allow to save weights at\n");
  fprintf (stderr, "\t\t                 each <Frequency> iterations. Default = %d.\n", refreshAllIterFrequency);
  fprintf (stderr, "\t\t-temp-stochastic :\n");
  fprintf (stderr, "\t\t                 Flag to compute learning rate and smooth distance in\n");
  fprintf (stderr, "\t\t                 stochastic mode (i.e. at each iteration and not each\n");
  fprintf (stderr, "\t\t                 cycle.) Default = %s.\n", (temperatureStochasticOk == OFF) ? "OFF" : "ON");
  fprintf (stderr, "\t\t-app-batch :\n");
  fprintf (stderr, "\t\t                 Flag to learn in batch mode (i.e. weights are adjusted\n");
  fprintf (stderr, "\t\t                 at the end of one cycle, not each itaretion\n");
  fprintf (stderr, "\t\t                 Default = %s.\n", (apprentissageBatchOk == OFF) ? "OFF" : "ON");
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
      raise_error ("som", "strdup test-file: malloc failure", ER_CRITICAL);
    return 2;

  } else if (!strcmp (Argument[0], "-old-map")) {	/* -OLD-MAP */
    inputMapFileName = strdup (Argument[1]);
    if (inputMapFileName == (char *) NULL)
      raise_error ("som", "strdup old-map: malloc failure", ER_CRITICAL);
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
      raise_error ("som", "strdup map-size: malloc failure", ER_FAIL);

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

  } else if (!strcmp (Argument[0], "-temp")) { /* -TEMP */
    smoothDistanceMax = (scalar_t) OurAToF (Argument[1]);
    smoothDistanceMin = (scalar_t) OurAToF (Argument[2]);
    return 3;

  } else if (!strcmp (Argument[0], "-learn-rate")) {	/* -LEARN-RATE */
    learningRateMax = (scalar_t) OurAToF (Argument[1]);
    learningRateMin = (scalar_t) OurAToF (Argument[2]);
    return 3;

  } else if (!strcmp (Argument[0], "-threshold-neigh")) {	/* -THRESHOLD-NEIGH */
    ThresholdNeighborhood = (scalar_t) OurAToF (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-cycles")) {	/* -CYCLES */
    nbCycle = OurAToL (Argument[1]);
    freqTest = OurAToL (Argument[2]);
    return 3;

  } else if (!strcmp (Argument[0], "-start-cycle")) {	/* START-CYCLE */
    cycle = OurAToL (Argument[1]);
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

  } else if (!strcmp (Argument[0], "-temp-stochastic") ||
	     !strcmp (Argument[0], "-stochastic")) {    /* TEMP-STOCHASTIC or STOCHASTIC */
    temperatureStochasticOk = ON;
    return 1;

  } else if (!strcmp (Argument[0], "-app-batch")) {    /* APP-BATCH */
    raise_error ("som", "option non implementee 'apprentissage BATCH'", ER_FAIL);
    apprentissageBatchOk = ON;
    return 1;

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
  /*====================================================================================*/
  /*                                    -main_variable_list                             */
  /*====================================================================================*/
  register int    i;			       /* Loop Index */
  int             IsStable;
  char            BffFile[BFFSIZE];
  char           *FileNamePrefix = NULL;
  char           *learnSetFileName = NULL;
  char           *outputMapFileName = NULL;
  char           *outputMapCardName = NULL;
  char           *outputWinFileName = NULL;
  char           *outputCardFileName = NULL;
  long            pointDim = 0;
  long            entreeSize;
  int             numero = 0;
  mapindex_t      mapSize = NULL;
  vector_t        minmax = NULL;
  som_t           net = NULL;
  set_t           tmpSet = NULL;
  mapDesc_t      *card;
  file_t          netFileIn = NULL;
  file_t          netFileOut = NULL;
  file_t          netFileWin = NULL;
  file_t          learnFile = NULL;
  file_t          testFile = NULL;
  file_t          cardFile = NULL;
  file_t          BContrastFile = NULL;
  file_t          BRmsFile = NULL;
  file_t          AllIterFile = NULL;

  double          OldRms = 0;
  double          OldContrast = 0;

  double          BestContrast;
  double          BestRMS;
  double          CurrentContrast;
  double          CurrentRMS;

  double          init_clock;
  double          used_time;

  int             NbExpectedArgs = 2;
  char           *argv0 = argv[0];
  int             NbArgs;

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

  /* --------------| FICHIER APPRENTISSAGE |--------------------------------- */
  /*                -----------------------                                   */
  learnSetFileName = strdup (argv[0]);

  /* --------------| PREFIX DES FICHIERS |----------------------------------- */
  /*                ---------------------                                     */
  FileNamePrefix = strdup (argv[1]);

  NewIndex (&mapSize, mapDim);
  StrToIndex (mapsize_string, mapSize);

  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| POIDS DE LA CARTE |------------------------------------- */
  /*                -------------------                                       */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_Wei.map");
  outputMapFileName = strdup (BffFile);

  /* --------------| POIDS + CARDINALITE POUR SAMMON |----------------------- */
  /*                ---------------------------------                         */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_WeiCard.map");
  outputMapCardName = strdup (BffFile);

  /* --------------| NEURONE GAGNANT POUR CHAQUE FORME PRESENTEE |----------- */
  /*                ---------------------------------------------             */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_WFEP.res");
  outputWinFileName = strdup (BffFile);

  /* --------------| CARDINALITE DE CHAQUE NEURONE |------------------------- */
  /*                -------------------------------                           */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_Card.res");
  outputCardFileName = strdup (BffFile);

  /* ************************************************************************ *
   *                          SORTIES OPTIONNELLES                            *
   * ************************************************************************ */

  /* --------------| POIDS SI BEST CONTRAST |-------------------------------- */
  /*                ------------------------                                  */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_BCTR_Wei.map");
  BContrastFileName = strdup (BffFile);

  /* --------------| POIDS SI BEST RMS |------------------------------------- */
  /*                -------------------                                       */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_BRMS_Wei.map");
  BRmsFileName = strdup (BffFile);

  /* --------------| POIDS SI TRACE MAP |------------------------------------ */
  /*                --------------------                                      */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_SOM_Wei_it");
  AllIterFileName = strdup (BffFile);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        SOM\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier apprentissage ...................... %s\n",
	   (learnSetFileName) ? learnSetFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier test ............................... %s\n",
	   (testSetFileName) ? testSetFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Ancienne carte ............................. %s\n",
	   (inputMapFileName) ? inputMapFileName : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier poids de la carte .................. %s\n",
	   (outputMapFileName) ? outputMapFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier poids + cardinalite pour sammon .... %s\n",
	   (outputMapCardName) ? outputMapCardName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier neurone gagnant .................... %s\n",
	   (outputWinFileName) ? outputWinFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier cardinalite de chaque neurone ...... %s\n",
	   (outputCardFileName) ? outputCardFileName : NONDEFINI_MSG);
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
  fprintf (stdout, "      Temperature maximum ........................ %f\n",
	   smoothDistanceMax);
  fprintf (stdout, "      Temperature minimum ........................ %f\n",
	   smoothDistanceMin);
  fprintf (stdout, "      Taux d'apprentissage maximum ............... %f\n", learningRateMax);
  fprintf (stdout, "      Taux d'apprentissage minimum ............... %f\n", learningRateMin);
  fprintf (stdout, "      Seuil de voisinage ......................... %f\n", ThresholdNeighborhood);
  fprintf (stdout, "      Nombre de cycles maximum ................... %ld\n", nbCycle);
  fprintf (stdout, "      Numero du cycle depart ..................... %ld\n", cycle);
  fprintf (stdout, "      Frequence Des Tests (cycles) ............... %ld\n", freqTest);
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
  fprintf (stdout, "      Calculer la temperature en mode stochastic . %s\n",
	   (temperatureStochasticOk == OFF) ? "OFF" : "ON");
  fprintf (stdout, "      Apprentissage en mode batch ................ %s\n",
	   (apprentissageBatchOk == OFF) ? "OFF" : "ON");
  fprintf (stdout, "      Forcer reecriture si fichiers existants .... %s\n",
	   (overwriteOutFilesOk == OFF) ? "OFF" : "ON");
  fprintf (stdout, "      *******************************************************\n\n");


  /* ********************************************************************** *
   * Verification de l'existence des fichiers d'entree et de
   * l'inexistence des fichiers de sortie.
   * ********************************************************************** */

  /* ****************************** *
   * Verification des E N T R E E S
   * ****************************** */

  /* fichier des donnees */
  NewFile (&learnFile, learnSetFileName, FS_TEST_INPUT_MODE, "");

  /* Initialization of the Map */
  if (inputMapFileName != NULL)
    NewFile (&netFileIn, inputMapFileName, FS_TEST_INPUT_MODE, "");

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
    /* fichier de cardinalite */
    NewFile (&cardFile, outputCardFileName, FS_TEST_OUTPUT_MODE, "");

    /* Poids + cardinalite de chaque referent pour Sammon  */
    NewFile (&cardFile, outputMapCardName, FS_TEST_OUTPUT_MODE, "");

    /*  Classe gagnante pour chaque element du set de test */
    NewFile (&netFileWin, outputWinFileName, FS_TEST_OUTPUT_MODE, "");
  }

  /*====================================================================================*/
  /*                                       LEARN_SOM                                    */
  /*====================================================================================*/
  /* Loading the learning set */
  /*--------------------------*/
  NewFile (&learnFile, learnSetFileName, FS_INPUT_MODE, "");
  LoadSet (&tmpSet, learnFile);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      learnFile--file Name ....................... %s\n", learnFile->fileName);
  fprintf (stdout, "      learnFile--header .......................... %s\n", learnFile->header);
  FreeFile (&learnFile);

  /* Automatic setting of the dimension */
  pointDim = (tmpSet)->dim;
  entreeSize = (tmpSet)->size;

  fprintf (stdout, "      Dimension des entrees ...................... %ld\n", pointDim);
  fprintf (stdout, "      Dimension de la carte ...................... %ld\n", entreeSize);
  fprintf (stdout, "      *******************************************************\n\n");

  /* Automatic setting of map min and max (used in NewSom) */
  /*-------------------------------------------------------*/
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
  if (inputMapFileName != NULL) {
    NewFile (&netFileIn, inputMapFileName, FS_INPUT_MODE, "");
    LoadSom (&(net), netFileIn);	       /* Reading the map */
    FreeFile (&netFileIn);

  } else {
    if ((pointDim == 0) || (mapSize == NULL) || (mapMin == NULL) || (mapMax == NULL))
      raise_error ("som", "Some map dimension not specified", ER_FAIL);

    else {
      NewSom (&net, mapSize, mapMin, mapMax, pointDim);
      Scale5Map (net->map, pointDim);
      Show ("\n%s\n", "apres scale5map");
      /* NewFile (&netFileOut, outputMapFileName, FS_OUTPUT_MODE, "");
         SaveMap (net->map, netFileOut);
         Show ("\n%s\n", "apres savemap");
         FreeFile (&netFileOut); */
    }
  }

  /* edition de la map juste initialisee */

  /* NewFile (&netFileOut, outputMapFileName, FS_OUTPUT_MODE, "");
  SaveMap (net->map, &netFileOut);
  FreeFile (&netFileOut); */

  /* Setting all the parameter of the learning and testing */
  /*-------------------------------------------------------*/

  if (net == NULL)
    raise_error ("som", "net equal NULL", ER_FAIL);

  else {
    net->nbCycle                = nbCycle;
    net->freqTest               = freqTest;
    net->cycle                  = cycle;
    net->learningRateMax        = learningRateMax;
    net->learningRateMin        = learningRateMin;
    net->smoothDistanceMax      = smoothDistanceMax;
    net->smoothDistanceMin      = smoothDistanceMin;
    net->tempStochastic         = temperatureStochasticOk;
    net->appBatch               = apprentissageBatchOk;
    net->threshold_neighborhood = ThresholdNeighborhood;

    net->learnSet               = tmpSet;       /* Specifying the learn set */
    tmpSet = NULL;

    NewSet (&(net->learnSetInfo), net->learnSet->size, SI_SIZE);
    net->learnSetInfo->stat->values[SET_BESTRMS] = MAXSCALAR;
    net->learnSetInfo->stat->values[SET_BESTCONTRAST] = MAXSCALAR;

    if (testSetFileName == NULL) {	       /*if no test set is specified, the learn set is used */
      net->testSet = (net->learnSet);
      net->testSetInfo = (net->learnSetInfo);
    } else {
      NewFile (&testFile, testSetFileName, FS_INPUT_MODE, "");
      LoadSet (&(net->testSet), testFile);
      FreeFile (&testFile);

      NewSet (&(net->testSetInfo), net->testSet->size, SI_SIZE);
      net->testSetInfo->stat->values[SET_BESTRMS] = MAXSCALAR;
      net->testSetInfo->stat->values[SET_BESTCONTRAST] = MAXSCALAR;
    }

  }
  Show ("\n%s\n", "Learning");
  /*-----------------------------*/
  /* CYCLES DE L'ALGORITHME SOM  */
  /*-----------------------------*/

  /* initialisation de facon aleatoire autour du centre de gravite */
/* InitSom(&net,net->learnSet);  *//* Init the map using the learning set */

  /* edition de la map juste initialisee  */

  init_clock = (double) clock ();

  OldRms       = 0;
  OldContrast  = 0;

  BestContrast = MAXSCALAR;
  BestRMS      = MAXSCALAR;

  IsStable     = 0;

  /* Sauvegarde du permier Map */
  if (refreshAllIterFrequency) {
    sprintf (BffFile, "%s%08d.map", AllIterFileName, 0);
    NewFile (&AllIterFile, BffFile, FS_OVERWRITE_MODE, "");
    SaveMap (net->map, &AllIterFile);
    FreeFile (&AllIterFile);
  }

  ForAllCycles (net) {

    if (IsStable)
      break;

    /*~~~~~~~~~~~~~~~ */
    /* APPRENTISSAGE */
    /*~~~~~~~~~~~~~~~ */
    numero++;

    CycleSom (net);

    /*It is stable when nothing is changing */
    IsStable = (((net->testSetInfo->stat->values[SET_RMS] == OldRms) &&
		 (net->testSetInfo->stat->values[SET_CONTRAST] == OldContrast)));

    OldRms = net->testSetInfo->stat->values[SET_RMS];
    OldContrast = net->testSetInfo->stat->values[SET_CONTRAST];

    /* ----- Meilleure RMS ----- */
    CurrentRMS = net->testSetInfo->stat->values[SET_BESTRMS];

    /* --- Meilleur Contrast --- */
    CurrentContrast = net->testSetInfo->stat->values[SET_BESTCONTRAST];

    if ((refreshAllIterFrequency) &&
	(!((net->cycle + 1) % refreshAllIterFrequency))) {
      sprintf (BffFile, "%s%08ld.map", AllIterFileName, net->cycle + 1 );
      NewFile (&AllIterFile, BffFile, FS_OVERWRITE_MODE, "");
      SaveMap (net->map, &AllIterFile);
      FreeFile (&AllIterFile);
    }

    if ((traceBContrastOnOff) &&
	(!(((int) net->cycle + 1) % refreshBContrastFrequency)) &&
	(CurrentContrast < BestContrast)) {
      fprintf (stdout, " *** iteration %ld ==> best contrast = %f\n", net->cycle, CurrentContrast);
      NewFile (&BContrastFile, BContrastFileName, FS_OVERWRITE_MODE, "");
      SaveMap (net->map, &BContrastFile);
      FreeFile (&BContrastFile);
    }

    if ((traceBRmsOnOff) &&
	(!(((int) net->cycle + 1) % refreshBRmsFrequency)) &&
	(CurrentRMS < BestRMS)) {
      fprintf (stdout, " *** iteration %ld ==> best RMS = %f\n", net->cycle, CurrentRMS);
      NewFile (&BRmsFile, BRmsFileName, FS_OVERWRITE_MODE, "");
      SaveMap (net->map, &BRmsFile);
      FreeFile (&BRmsFile);
      }
  }

  used_time = (((double) clock ()) - init_clock) / CLOCKS_PER_SEC;
  printf (" Cycle(%d). Temps d'execution = %.6f s.\n", numero, used_time);

  /* Save the resulting map */
  NewFile (&netFileOut, outputMapFileName, FS_OVERWRITE_MODE, "");
  SaveMap (net->map, &netFileOut);
  FreeFile (&netFileOut);

  /* Ajout D Frayssinet */

  /* edition de la cardinalite uniquement pour chaque referent  */
  Show ("\n%s\n", "New outputCard");
  NewFile (&cardFile, outputCardFileName, FS_OVERWRITE_MODE, "");
  NewVector (&minmax, 1);
  NewMap (&card, net->map->size, minmax, minmax, 1);
  tmpSet = (net->testSetInfo);
  ForAllElements (tmpSet, tmpSet->curElement) {
    card->curPoint = card->firstPoint + (long) (*(tmpSet->curElement))->values[SI_WINNEROFFSET];
    ((*(card->curPoint))->values[0]) += 1.0;
  }
  SaveMap (card, &cardFile);
  FreeFile (&cardFile);

  /* edition des poids + cardinalite de chaque referent pour Sammon  */
  Show ("\n%s\n", "New outputMapCard");
  if (net->map->size->dim == 2) {
    NewFile (&cardFile, outputMapCardName, FS_OVERWRITE_MODE, "");

    /*   fprintf( GetFilePointer(cardFile), "%s\n", "8 rect 7 13 gaussian" ); */
    fprintf (GetFilePointer (cardFile), "%ld %s %ld %ld %s\n", pointDim, "rect",
	     net->map->size->values[1], net->map->size->values[0], "gaussian");

    card->curPoint = (card)->firstPoint;
    (card)->offset = 0;
    ForAllPoints (net->map, net->map->curPoint) {
      for (i = 0; i < net->map->min->dim; i++) {
	fprintf (GetFilePointer (cardFile), "%f", (*(net->map->curPoint))->values[i]);
	fprintf (GetFilePointer (cardFile), FS_END_OF_FIELD);
      }

      fprintf (GetFilePointer (cardFile), "%1.0f", (*(card->curPoint))->values[0]);
      fprintf (GetFilePointer (cardFile), FS_END_OF_RECORD);

      ((card)->offset)++;
      (card->curPoint)++;
    }

    FreeFile (&cardFile);
  }

  /* edition de la classe gagnante pour chaque element du set de test */
  NewFile (&netFileWin, outputWinFileName, FS_OVERWRITE_MODE, "");

  fprintf (GetFilePointer (netFileWin), "%s%ld%s\n", "# mapDim=1 mapSize={", entreeSize,
	   "} pointDim=1 mapMin={0.0} mapMax={0.0}");

  ForAllElements (net->learnSetInfo, net->learnSetInfo->curElement) {
    fprintf (GetFilePointer (netFileWin), "%1.0f",
	     (*(net->learnSetInfo->curElement))->values[SI_WINNEROFFSET]);
    fprintf (GetFilePointer (netFileWin), FS_END_OF_RECORD);
  }

  FreeFile (&netFileWin);

  Show ("\n%s", "----- Meilleure RMS -----");
  Show ("\n%f", net->testSetInfo->stat->values[SET_BESTRMS]);
  Show ("\n%s", "--- Meilleur Contrast ---");
  Show ("\n%f", net->testSetInfo->stat->values[SET_BESTCONTRAST]);
  Show ("\n%s\n", "-------------------------");

  return (0);
}
