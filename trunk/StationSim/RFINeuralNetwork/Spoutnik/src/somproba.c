/*
   ce programme a pour but de determiner les probabilites
   d'appartenance des formes aux differentes classes.

   Calcul de proba lors de la coloration de la carte : p(ci / ref) =
   Somme p(xi / ref) pour tous les xi appartenant a la classe ci on ne
   connait que les valeurs d'activation dans les fonctions de densite
   on ne peut pas faire la somme des fonctions de densite car x est
   continu.

   Calcul de proba apres la coloration de la carte :

   p(x et ci)
   p(ci/x) = ----------
   p(x)

   ni * fi(x)
   = --------------
   Somme nk * fk(x)  k pour tous les referents

   ni    = cardinalite du referent i
   fi(x) = activation de l'exemple x dans la loi normale du referent i

   Somme nc * fc(x) pour tous les referents c de couleur rouge
   p(rouge / x) = ----------------------------------------------------------
   Somme nk * fc(x)

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"
#include "somproba.h"

void            Tri (double *proba, double **triProba, long **triClass, long cl);

/*
 * Variables Globales
 */

/* ------------------| NUMBER OF TRUE CLASSES |---------------------------- */
long            nClass = 12;

/* ------------------| NUMBER OF POSSIBLE CHOICES |------------------------ */
/* Nombre de choix possibles, parmi les classes de plus forte probabilite */
long            nChoix = 3;

/* ------------------| Minimal probability to labeling |------------------------ */
/* Probabilite minimum pour qu'une classe puisse labelliser un neurone */
double          seuil = 0.15;

/*===========================================================================================*/

/*===========================================================================================*/
/* Tri du tableau des probabilites d'appartenance d'une forme a
   chacune des classes.  Il faut trier cette ligne par ordre de
   probabilite decroissante, de facon a avoir en tete les classes de
   plus forte appartenance au neurone.

   Nous redonnons la liste des probabilites d'appartenance triees en
   ordre decroissant dans le tableau a une dimension triProba.  La
   liste des classes associees a ces proba est donnee dans le tableau
   triClass.

   De facon a faire correspondre le tableau proba avec les numeros de
   classe, ohaque colonne correspond a la probabilite d'appartenance
   d'une classe i a ce referent.  la colonne 0 n'est pas
   utilisee. Elle correspond aux formes non labellisees.  C'est
   pourquoi la colonne tableau[0] n'est jamais consideree.
 */

void
Tri (double *proba, double **triProba, long **triClass, long cl)
{
  double         *tableau = proba;
  double         *tProba = *triProba;
  long           *tClass = *triClass;

  long            maxTri;		       /* indice maximum du poste ecrit dans le tableau de tri tProba */
  long            i, m, p;
  int             trouve;		       /* 0 = non trouve, 1 = trouve */

  tProba[0] = tableau[0];
  tClass[0] = 1;

  maxTri = 0;

  /* tri de la ligne par ordre decroissant */
  for (i = 1; i < cl; i++) {
    for (m = 0, trouve = 0; m <= maxTri && trouve == 0; m++) {
      if (tableau[i] > tProba[m]) {
	for (p = maxTri; p >= m; p--) {
	  tProba[p + 1] = tProba[p];
	  tClass[p + 1] = tClass[p];
	}
	tProba[m] = tableau[i];
	tClass[m] = i + 1;
	trouve = 1;
      }
    }
    maxTri++;
    if (trouve == 0) {
      /* non trouve donc plus petit que tous, il s'ecrit a la fn */
      tProba[maxTri] = tableau[i];
      tClass[maxTri] = i + 1;
    }
  }

  return;
}

/******************************************************************************
 * void print_call (char *CallName)                                           *
 ******************************************************************************/
void
print_call (char *CallName)
{

  fprintf (stderr, "\nCalling program \'%s\':\n", CallName);

  fprintf (stderr, "\t%s [options]  <MapFile> <CardFile> <ActFile> <ResFile> <OutFile>\n",
	   CallName);

  fprintf (stderr, "\nArguments:\n");
  fprintf (stderr, "\t<MapFile> ........ la carte des neurones labellises par la CAH.\n");
  fprintf (stderr, "\t<CardFile> ....... cardinalite des neurones.\n");
  fprintf (stderr, "\t<ActFile> ........ activation de la forme x dans les lois normales\n");
  fprintf (stderr, "\t                   des referents (produit par PRSOM).\n");
  fprintf (stderr, "\t<ResFile> ........ numero de la classe a laquelle est associe chaque\n");
  fprintf (stderr, "\t                   forme.\n");
  fprintf (stderr, "\t<OutFile> ........ sortie : les N-CHOICE classes de plus grande probabilite\n");
  fprintf (stderr, "\t                   pour chaque forme.\n");

  fprintf (stderr, "\nList of options:\n");
  fprintf (stderr, "\t\t-n-class <nClass> :\n");
  fprintf (stderr, "\t\t                 Number of true classes.\n");
  fprintf (stderr, "\t\t                 Default = %ld.\n", nClass);
  fprintf (stderr, "\t\t-n-choice <nChoice> :\n");
  fprintf (stderr, "\t\t                 number of choices for classes of greatest\n");
  fprintf (stderr, "\t\t                 probability. Default = %ld.\n", nChoix);
  fprintf (stderr, "\t\t-threshold <Treshold> :\n");
  fprintf (stderr, "\t\t                 Treshold of minimal probability for consider\n");
  fprintf (stderr, "\t\t                 a class. Default = %.1f.\n", seuil);
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

  } else if (!strcmp (Argument[0], "-n-class")) {	/* -N-CLASS */
    nClass = OurAToL (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-n-choice")) {	/* -N-CHOICE */
    nChoix = OurAToL (Argument[1]);
    return 2;

  } else if (!strcmp (Argument[0], "-threshold")) {	/* -THRESHOLD */
    seuil = OurAToF (Argument[1]);
    return 2;

  } else {
    print_call (Argument0);
    raise_error ("somproba", "nombre de parametres invalide", ER_CRITICAL);
  }

  return 0;
}

int
main (int argc, char **argv)
{

  /* -main_variable_list */

  file_t          mapFile;		       /* la carte des neurones labellises par la CAH */
  file_t          cardFile;		       /* cardinalite des neurones */
  file_t          actFile;		       /* activation de la forme x dans les lois
						  normales des referents */
  file_t          resFile;		       /* numero de la classe a laquelle est associe
						  chaque forme */
  file_t          output;		       /* en sortie : les N_CHOICE classes de plus grande
						  probabilite pour chaque forme */

  char           *map_file;		       /*  FILENAME */
  char           *card_file;		       /*  FILENAME */
  char           *act_file;		       /*  FILENAME */
  char           *res_file;		       /*  FILENAME */
  char           *o_file;		       /*  FILENAME */

  mapDesc_t      *map = NULL;		       /* the map to be indexed */
  mapDesc_t      *card = NULL;
  mapDesc_t      *act = NULL;
  mapDesc_t      *res = NULL;

  /* pour l'entete du fichier output */
  char            str[256];
  char            arg[256] = "# mapDim=";
  char            arg2[32] = "mapSize=";
  char            arg3[32] = "pointDim=";
  char            arg4[32] = "mapMin=";
  char            arg5[32] = "mapMax=";

  long            nRef;			       /* Number of neurons in the map (weight vectors) */
  long            nExempl;		       /* Number of samples to be classified */
  long            cl;			       /* Number of true classes */

  /* double ecart; */			       /* Pour chaque neurone, nous comparons les
						  probabilites d'affectation des differentes
						  classes affectees a ce neurone avec la classe
						  la plus representative du neurone. Si l'ecart
						  de probabilite est superieur au nombre
						  "ecart", la classe de probabilite moindre ne
						  sera pas retenue comme label.  */

  long           *cardi;		       /* cardinalite de chaque neurone */

  long           *resi;			       /* classe resultat pour chaque forme */

  double         *numerateur;		       /* tableau a 2 dimensions. Activation d'une
					          forme dans une classe */

  double         *proba;		       /* tableau a 2 dimensions. Probabilite qu'une
					          forme appartiene a une classe */

  double         *triProba;		       /* proba cumulees dans tableau triees pour un
					          referent passe en argument a la fonction Tri */

  long           *triClass;		       /* tableau des classes triees pour un referent
					          argument passe a la fonction Tri */

  vector_t        label;		       /* la classe de chaque neurone */

  long            x, c, n_card, classeDuRef, classe, rang, suivant, indTri;
  double          denominateur, fdens, probabilite;

  int             NbExpectedArgs = 5;
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
    raise_error ("somproba", "invalid argument number", ER_CRITICAL);
  }

  /* ======================================================================== *
   *                     INITIALISATION  DES PARAMETRES                       *
   * ======================================================================== */

  /* ************************************************************************ *
   *                          ENTREES AU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| CARTE DES NEURONES |------------------------------------ */
  /*                --------------------                                      */
  map_file = strdup (argv[0]);

  /* --------------| CARDINALITE DES NEURONES |------------------------------ */
  /*                --------------------------                                */
  card_file = strdup (argv[1]);

  /* --------------| ACTIVATION DE LA FORME X |------------------------------ */
  /*                --------------------                                      */
  act_file = strdup (argv[2]);

  /* --------------| NUMERO DE LA CLASSE ASSOCIE A FORME |------------------- */
  /*                -------------------------------------                     */
  res_file = strdup (argv[3]);

  /* ************************************************************************ *
   *                          SORTIES DU PROGRAMME                            *
   * ************************************************************************ */

  /* --------------| CLASSES DE PLUS GRANDE PROBABILITE |-------------------- */
  /*                ------------------------------------                      */
  o_file = strdup (argv[4]);

  /* ======================================================================== *
   *                         RAPPORT DES VARIABLES                            *
   * ======================================================================== */

  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "                        SOMPROBA\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      entrees :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier de la carte des neurones ........... %s\n", map_file);
  fprintf (stdout, "      Fichier de cardinalite des neurones ........ %s\n", card_file);
  fprintf (stdout, "      Fichier d'activation de la forme X ......... %s\n", act_file);
  fprintf (stdout, "      Fichier du numero de la classe associe ..... %s\n", res_file);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      sorties :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Fichier des classes de plus grande proba ... %s\n", o_file);
  fprintf (stdout, "\n      *******************************************************\n");
  fprintf (stdout, "      autres parametres :\n");
  fprintf (stdout, "      *******************************************************\n");
  fprintf (stdout, "      Nombre reel de classes ..................... %ld\n", nClass);
  fprintf (stdout, "      Nombre de choix possibles .................. %ld\n", nChoix);
  fprintf (stdout, "      Seuil de probabilite minimum pour etiquetage %f\n", seuil);

  /*============================================================================================*/

  /*=======================================================================*/
  /* ecart          = 0.60; */

  /* Les numeros de classe commencent a 1, aussi, nous avons besoin
     d'un espace supplementaire dans le tableau numerateur */
  cl = nClass + 1;

  /* -main_before_lib_call */
  /* Step 1: Open the input files */
  /* load the map of weight vectors */
  printf ("newfile omean\n");
  NewFile (&mapFile, map_file, FS_INPUT_MODE, "");
  LoadMapLabel (&map, mapFile, &label);
  FreeFile (&mapFile);

  /* Load cardinality file */
  printf ("newfile psom.card\n");
  NewFile (&cardFile, card_file, FS_INPUT_MODE, "");
  LoadMap (&card, cardFile);
  FreeFile (&cardFile);

  /* Load the psom activation file */
  printf ("newFile psom.act\n");
  NewFile (&actFile, act_file, FS_INPUT_MODE, "");
  LoadMapProba (&act, actFile);
  FreeFile (&actFile);

  /* Load the file of the number of the associated class file */
  printf ("newFile psom.res\n");
  NewFile (&resFile, res_file, FS_INPUT_MODE, "");
  LoadMap (&res, resFile);
  FreeFile (&resFile);

  printf ("output o_file\n");
  NewFile (&output, o_file, FS_OUTPUT_MODE, "");

  /* Initialisation */
  nRef = map->range;
  printf ("nRef %ld \n", nRef);
  nExempl = act->range;
  printf ("nExempl %ld \n", nExempl);

  /* ================================================================
     Allocation of complete Cardinality table (copy of the CARD file)
     ================================================================ */
  cardi = (long *) malloc (nRef * sizeof (long));
  /* fprintf(stdout, " cardi %08x\n", cardi); */
  if (cardi == (long *) NULL)
    raise_error ("somproba", "cardi: malloc failure", ER_CRITICAL);

  /* ================================================================
     Allocation of complete Assosiated Class table (copy of the RES
     file)
     ================================================================ */
  resi = (long *) malloc (nExempl * sizeof (long));
  /* fprintf(stdout, " resi %08x\n", resi); */
  if (resi == (long *) NULL)
    raise_error ("somproba", "resi: malloc failure", ER_CRITICAL);

  numerateur = (double *) malloc (cl * sizeof (double));
  /* fprintf(stdout, " numerateur %08x\n", numerateur); */
  if (numerateur == (double *) NULL)
    raise_error ("somproba", "numerateur: malloc failure", ER_CRITICAL);

  proba = (double *) malloc (cl * sizeof (double));
  /* fprintf(stdout, " proba %08x\n", proba); */
  if (proba == (double *) NULL)
    raise_error ("somproba", "proba: malloc failure", ER_CRITICAL);

  triProba = (double *) malloc (nClass * sizeof (double));
  /* fprintf(stdout, " triProba %08x\n", triProba); */
  if (triProba == (double *) NULL)
    raise_error ("somproba", "triProba: malloc failure", ER_CRITICAL);

  triClass = (long *) malloc (nClass * sizeof (long));
  /* fprintf(stdout, " triClass %08x\n", triClass); */
  if (triClass == (long *) NULL)
    raise_error ("somproba", "triClass: malloc failure", ER_CRITICAL);

  Show ("\n%s\n", "ecriture entete fichier de sortie");

  LongToStr (map->size->dim, str);
  strcat (arg, str);
  strcat (arg, " ");
  strcat (arg, arg2);
  IndexToStr (map->size, str);
  strcat (arg, str);
  strcat (arg, " ");
  strcat (arg, arg3);
  LongToStr (map->min->dim, str);
  strcat (arg, str);
  strcat (arg, " ");
  strcat (arg, arg4);
  VectorToStr (map->min, str);
  strcat (arg, str);
  strcat (arg, " ");
  strcat (arg, arg5);
  VectorToStr (map->max, str);
  strcat (arg, str);
  strcat (arg, " ");
  strcat (arg, GetHeader (output));
  SetHeader (output, arg);
  fprintf (GetFilePointer (output), "%s\n", GetHeader (output));

  Show ("\n%s\n", "tableau cardinalites");
  ForAllPoints (card, card->curPoint) {
    cardi[card->offset] = (*(card->curPoint))->values[0];
  }

  Show ("\n%s\n", "tableau de numero de classe associe");
  ForAllPoints (res, res->curPoint) {
    resi[res->offset] = (*(res->curPoint))->values[0];
  }

  /* BOUCLE SUR CHAQUE FORME */

  printf ("step3\n");
  /* Step 4: Calcul de proba pour chaque forme x
     1 - denominateur pour toutes les classes = Somme nc * fc(x)
     2 - pour chaque classe r
     numerateur[r] = Somme nr *  fr(x)
     3 - proba[r]   = numerateur / denominateur
     4 - classer les proba
     editer les 3 plus fortes
   */

  ForAllPoints (act, act->curPoint) {

    /* 1 - */

    denominateur = 0;
    for (classe = 0; classe < cl; classe++)
      numerateur[classe] = 0;

    rang = x * nRef;

    for (c = 0; c < nRef; c++) {
      n_card = cardi[c];
      fdens = (*(act->curPoint))->values[c];
      probabilite = n_card * fdens;
      denominateur += probabilite;

      /* 2 - */
      classeDuRef = (long) (label->values[c]);
      numerateur[classeDuRef-1] += probabilite;
    }

    /* 3 - */

    for (classe = 0; classe < cl; classe++)
      proba[classe] = numerateur[classe] / denominateur;

    /* 4 - */

    /* tri proba[classe]  */

    Tri (proba, &triProba, &triClass, cl);

    /* debug */
    /*
    printf (" proba telles quelles \n");
    for ( classe = 1 ; classe < cl ; classe++ )
      printf( "%f  ", proba[classe] );

    printf( "\n");

    printf (" proba triees \n");
    for ( indTri = 0 ; indTri < nClass ; indTri++ ) {
      printf( "cl %d ", triClass[indTri] );
      printf( "%f   ", triProba[indTri] );
    }
    
    printf( "\n");
    */

    /* fin debug */

    /*  Edition 3 classes de plus forte probabilite  */
    /*  0 is false; 1 is true                        */
    suivant = 1;
    for (indTri = 0; indTri < nChoix && suivant; indTri++) {
      if (triProba[indTri] > seuil) {
	fprintf (GetFilePointer (output), "%2ld ", triClass[indTri]);
	fprintf (GetFilePointer (output), "%8.6f ", triProba[indTri]);
      } else
	suivant = 0;
    }

    fprintf (GetFilePointer (output), FS_END_OF_RECORD);

    /*  affichage des probabilites triees  */
    /*
    printf ("f%8ld ", act->offset);

    for (indTri = 0; indTri < 4; indTri++) {
      printf ("c%2ld", triClass[indTri]);
      printf (" %f ", triProba[indTri]);
    }

    printf ("\n");
    */
  }

  /* free memory */

  /*  FreeMap(&map);
     FreeMap(&res);
     FreeMap(&act);
     FreeMap(&ind);

     free(index);
     free(referIndex);
     free(probaSelect);
     free(classSelect);

     FreeFile(&output);
     FreeFile(&outSam);
     FreeFile(&outLab);
   */

  printf ("indexproba termine\n");

  return (0);
}
