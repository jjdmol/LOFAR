/*
    programme destine a tourner sous Unix
  */
/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>> 
   >>>> 	Main program for klearn_som
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

#include "outils.h"
#include "spoutnik.h"

#define TRUE 1

/*
 * Variables Globales
 */

/* ------------| FLAG POUR PERMETTE LA REECRITURE SUR SORTIES EXISTANTES |--------- */
int             overwriteOutFilesOk = OFF;


/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr, "\t%s [options] <TestSetFile> <MapFile><FilePrefix>\n", CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<TestFile> ........ fichier de formes a tester.\n");
  fprintf (stderr, "\t<MapFile> ......... fichier des poids issue de SOM ou PRSOM.\n");
  fprintf (stderr, "\t<FilePrefix> ...... prefix des fichiers de sortie qui seront :\n");
  fprintf (stderr, "\t  - <FilePrefix>_TSOM_Wei.map ........ poids de la carte.\n");
  fprintf (stderr, "\t  - <FilePrefix>_TSOM_WeiCard.map .... poids + cardinalite pour Sammon.\n");
  fprintf (stderr, "\t  - <FilePrefix>_TSOM_WFEP.res ....... neurone gagnant par forme.\n");
  fprintf (stderr, "\t  - <FilePrefix>_TSOM_Card.res ....... cardinalite chaque neurone.\n");

  fprintf (stderr, "\nList of options:\n");
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
  if (!strcmp (Argument[0], "-overwrite")) {    /* OVERWRITE */
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

int main(
	 int  argc,
	 char **argv)
{
  /* -main_variable_list */
  char            BffFile[BFFSIZE];
  char           *FileNamePrefix     = NULL;
  char           *inputMapFileName   = NULL;  
  char           *testSetFileName    = NULL;
  char           *outputMapFileName  = NULL;  /* poids de la carte */
  char           *outputMapCardName  = NULL;  /* poids + cardinalite pour sammon  */
  char           *outputWinFileName  = NULL;  /* neurone gagnant pour chaque forme presentee */
  char           *outputCardFileName = NULL;  /* cardinalite uniquement pour CAH  */    

  long            pointDim       = 0;
  long            entreeSize;

  vector_t        minmax         = NULL;

  som_t           net            = NULL;
  set_t           tmpSet         = NULL;
  mapDesc_t      *card;


  file_t          netFile        = NULL;
  file_t          testFile       = NULL;
  file_t          cardFile       = NULL;


  /* file_t          rmsFile;
  char           *rmsFileName    = NULL;
  */

  int             i; /* Loop Index */
  int             NbExpectedArgs = 3;
  char           *argv0          = argv[0];
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
  /* --------------| FORMES A TESTER  |-------------------------------------- */
  /*                ------------------                                        */
  testSetFileName = strdup (argv[0]);

  /* --------------| CARTE DES POIDS ISSUE DE SOM OU PRSOM |----------------- */
  /*                ---------------------------------------                   */
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
  strcat (BffFile, "_TSOM_Wei.map");
  outputMapFileName = strdup (BffFile);

  /* --------------| POIDS + CARDINALITE POUR SAMMON |----------------------- */
  /*                ---------------------------------                         */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_TSOM_WeiCard.map");
  outputMapCardName = strdup (BffFile);

  /* --------------| NEURONE GAGNANT POUR CHAQUE FORME PRESENTEE |----------- */
  /*                ---------------------------------------------             */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_TSOM_WFEP.res");
  outputWinFileName = strdup (BffFile);

  /* --------------| CARDINALITE CHAQUE NEURONE |---------------------------- */
  /*                ----------------------------                              */
  strcpy (BffFile, FileNamePrefix);
  strcat (BffFile, "_TSOM_Card.res");
  outputCardFileName = strdup (BffFile);



  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        TEST_SOM\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de formes a tester ................. %s\n",
	   (testSetFileName) ? testSetFileName : NONDEFINI_MSG);
  fprintf (stdout, "      Fichier des poids issue de SOM ............. %s\n",
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
  fprintf (stdout, "      Fichier cardinalite par neurone ............ %s\n",
	   (outputCardFileName) ? outputCardFileName : NONDEFINI_MSG);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      autres parametres :\n");
  fprintf (stdout, "      *******************************************************\n");
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

  /* Loading the test set */
  NewFile (&testFile, testSetFileName, FS_TEST_INPUT_MODE, "");

  /* Loading the Map */
  NewFile (&netFile, inputMapFileName, FS_TEST_INPUT_MODE, "");

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
    
    /* edition des poids + cardinalite de chaque referent pour Sammon  */
    NewFile (&cardFile, outputMapCardName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition de la classe gagnante pour chaque element du set de test */
    NewFile (&netFile, outputWinFileName, FS_TEST_OUTPUT_MODE, "");
    
    /* edition des cardinalites de chaque referent  */
    NewFile (&cardFile, outputCardFileName, FS_TEST_OUTPUT_MODE, "");
  }


  /*************/
  /* LEARN_SOM */
  /*************/
  /* Loading the learning set */
  /*-----------------------*/
  printf ("\n--avant Newfile testSet--\n");
  NewFile( &testFile, testSetFileName, FS_INPUT_MODE, "" );
  LoadSet( &tmpSet, testFile );
  FreeFile( &testFile );

  /* Automatic setting of the dimension */
  pointDim=(tmpSet)->dim;
  entreeSize = (tmpSet)->size;
  printf ("\n--dimension pointDim--%ld\n", pointDim);

 

  /* Initialization of the Map */
  /*---------------------------*/
  printf ("\n--avant Newfile inputMap--\n");
  NewFile( &netFile, inputMapFileName, FS_INPUT_MODE, "" );
  LoadSom( &(net), netFile ); /* Reading the map*/
  FreeFile( &netFile );

  printf ("\n--apres la map--\n");
  /* edition de la map juste initialisee  */

      
  /*    NewFile( &netFile, outputMapFileName, FS_OUTPUT_MODE, "" );
	SaveMap( net->map, netFile );
	FreeFile( &netFile );
	exit (3); 
  */


  /* Setting all the parameter of the learning and testing */
  /*-------------------------------------------------------*/
     
  if (net == NULL)
    raise_error("spoutnik","/usr/local_nn/khoros220/neurosat/bin",ER_FAIL);
  else {
    net->nbCycle = 1;
    net->cycle = 0;
    net->learningRateMax = 0;
    net->learningRateMin = 0; 
    net->smoothDistanceMax = 0;
    net->smoothDistanceMin = 0;
    
    net->learnSet=tmpSet;       /* Specifying the learn set */
    tmpSet=NULL;                
    
    
    net->learnSetInfo = NULL;
    /*   net->learnSetInfo->stat->values[SET_BESTRMS] = MAXSCALAR;
	 net->learnSetInfo->stat->values[SET_BESTCONTRAST] = MAXSCALAR;
    */   
    
    NewFile( &testFile, testSetFileName, FS_INPUT_MODE, "" );
    LoadSet( &(net->testSet), testFile );
    FreeFile( &testFile );
    
    
    NewSet( &(net->testSetInfo), net->testSet->size, SI_SIZE );
    net->testSetInfo->stat->values[SET_BESTRMS] = MAXSCALAR;
    net->testSetInfo->stat->values[SET_BESTCONTRAST] = MAXSCALAR;
    
  }
  Show("\n%s\n","Learning");
  /*-----------------------------*/
  /* CYCLES DE L'ALGORITHME SOM  */
  /*-----------------------------*/

  /*      
	  NewFile( &rmsFile, rmsFileName, FS_OUTPUT_MODE, "" );
	  fprintf( GetFilePointer(rmsFile), "#\n" );
	  FreeFile( &rmsFile );

  */

 
  /*~~~~~~~~~~~~~~~*/
  /*    T E S T    */
  /*~~~~~~~~~~~~~~~*/

  ForAllElements( net->testSet, net->testSet->curElement) {
      net->info = *(net->testSetInfo->firstElement+net->testSet->index);
      net->inputVector = *(net->testSet->curElement);

      ClassSom( net );
    }
  SomStat( net );


  /* edition de la carte topologique */
  Show("\n%s\n","New outputMap");
  NewFile( &netFile, outputMapFileName, FS_OUTPUT_MODE, "" );
  SaveMap( net->map, &netFile );
  FreeFile( &netFile );


  /* edition de la cardinalite uniquement pour chaque referent  */
  Show("\n%s\n","New outputCard");
  NewFile( &cardFile,outputCardFileName, FS_OUTPUT_MODE, "" );
  NewVector(&minmax,1);
  NewMap( &card, net->map->size,minmax,minmax,1);
  tmpSet = (net->testSetInfo);
  ForAllElements( tmpSet, tmpSet->curElement ) {
    card->curPoint = card->firstPoint +
      (long) (*(tmpSet->curElement))->values[SI_WINNEROFFSET];
    ((*(card->curPoint))->values[0])+=1.0;
  }
  SaveMap(card,&cardFile);
  FreeFile( &cardFile );


  /* edition des poids + cardinalite de chaque referent pour Sammon  */
  NewFile( &cardFile,outputMapCardName, FS_OUTPUT_MODE, "" );
            
  fprintf( GetFilePointer(cardFile), "%s\n", "5 rect 7 13 gaussian" );
 
  card->curPoint=(card)->firstPoint;
  (card)->offset=0;
  ForAllPoints( net->map, net->map->curPoint ) {
    for ( i=0 ; i < net->map->min->dim ; i++ ) {
      fprintf( GetFilePointer(cardFile), "%f", (*(net->map->curPoint))->values[i] );
      fprintf( GetFilePointer(cardFile), FS_END_OF_FIELD );
    }
    
    fprintf( GetFilePointer(cardFile), "%1.0f", (*(card->curPoint))->values[0] );
    fprintf( GetFilePointer(cardFile), FS_END_OF_RECORD );
    
    ((card)->offset)++; 
    (card->curPoint)++; 
  }

  FreeFile( &cardFile );


  /* edition de la classe gagnante pour chaque element du set de test */
  Show("\n%s\n","New outputWinFileCard");
  NewFile( &netFile, outputWinFileName, FS_OUTPUT_MODE, "" );

  fprintf( GetFilePointer(netFile), "%s%ld%s\n", "# mapDim=1 mapSize={",entreeSize,"} pointDim=1 mapMin={0.0} mapMax={0.0}" );

  ForAllElements( net->testSetInfo, net->testSetInfo->curElement ) {
    fprintf( GetFilePointer(netFile), "%1.0f", (*(net->testSetInfo->curElement))->values[SI_WINNEROFFSET] );
    fprintf( GetFilePointer(netFile), FS_END_OF_RECORD );
  }
  
  FreeFile( &netFile );

  return (0);
}
