      SUBROUTINE A7SST(IV, LIV, LV, V)
C
C  ***  ASSESS CANDIDATE STEP (***SOL VERSION 2.3)  ***
C
      INTEGER LIV, LV
      INTEGER IV(LIV)
      REAL V(LV)
C
C  ***  PURPOSE  ***
C
C        THIS SUBROUTINE IS CALLED BY AN UNCONSTRAINED MINIMIZATION
C     ROUTINE TO ASSESS THE NEXT CANDIDATE STEP.  IT MAY RECOMMEND ONE
C     OF SEVERAL COURSES OF ACTION, SUCH AS ACCEPTING THE STEP, RECOM-
C     PUTING IT USING THE SAME OR A NEW QUADRATIC MODEL, OR HALTING DUE
C     TO CONVERGENCE OR FALSE CONVERGENCE.  SEE THE RETURN CODE LISTING
C     BELOW.
C
C--------------------------  PARAMETER USAGE  --------------------------
C
C  IV (I/O) INTEGER PARAMETER AND SCRATCH VECTOR -- SEE DESCRIPTION
C             BELOW OF IV VALUES REFERENCED.
C LIV (IN)  LENGTH OF IV ARRAY.
C  LV (IN)  LENGTH OF V ARRAY.
C   V (I/O) REAL PARAMETER AND SCRATCH VECTOR -- SEE DESCRIPTION
C             BELOW OF V VALUES REFERENCED.
C
C  ***  IV VALUES REFERENCED  ***
C
C    IV(IRC) (I/O) ON INPUT FOR THE FIRST STEP TRIED IN A NEW ITERATION,
C             IV(IRC) SHOULD BE SET TO 3 OR 4 (THE VALUE TO WHICH IT IS
C             SET WHEN STEP IS DEFINITELY TO BE ACCEPTED).  ON INPUT
C             AFTER STEP HAS BEEN RECOMPUTED, IV(IRC) SHOULD BE
C             UNCHANGED SINCE THE PREVIOUS RETURN OF A7SST.
C                ON OUTPUT, IV(IRC) IS A RETURN CODE HAVING ONE OF THE
C             FOLLOWING VALUES...
C                  1 = SWITCH MODELS OR TRY SMALLER STEP.
C                  2 = SWITCH MODELS OR ACCEPT STEP.
C                  3 = ACCEPT STEP AND DETERMINE V(RADFAC) BY GRADIENT
C                       TESTS.
C                  4 = ACCEPT STEP, V(RADFAC) HAS BEEN DETERMINED.
C                  5 = RECOMPUTE STEP (USING THE SAME MODEL).
C                  6 = RECOMPUTE STEP WITH RADIUS = V(LMAXS) BUT DO NOT
C                       EVALUATE THE OBJECTIVE FUNCTION.
C                  7 = X-CONVERGENCE (SEE V(XCTOL)).
C                  8 = RELATIVE FUNCTION CONVERGENCE (SEE V(RFCTOL)).
C                  9 = BOTH X- AND RELATIVE FUNCTION CONVERGENCE.
C                 10 = ABSOLUTE FUNCTION CONVERGENCE (SEE V(AFCTOL)).
C                 11 = SINGULAR CONVERGENCE (SEE V(LMAXS)).
C                 12 = FALSE CONVERGENCE (SEE V(XFTOL)).
C                 13 = IV(IRC) WAS OUT OF RANGE ON INPUT.
C             RETURN CODE I HAS PRECEDENCE OVER I+1 FOR I = 9, 10, 11.
C IV(MLSTGD) (I/O) SAVED VALUE OF IV(MODEL).
C  IV(MODEL) (I/O) ON INPUT, IV(MODEL) SHOULD BE AN INTEGER IDENTIFYING
C             THE CURRENT QUADRATIC MODEL OF THE OBJECTIVE FUNCTION.
C             IF A PREVIOUS STEP YIELDED A BETTER FUNCTION REDUCTION,
C             THEN IV(MODEL) WILL BE SET TO IV(MLSTGD) ON OUTPUT.
C IV(NFCALL) (IN)  INVOCATION COUNT FOR THE OBJECTIVE FUNCTION.
C IV(NFGCAL) (I/O) VALUE OF IV(NFCALL) AT STEP THAT GAVE THE BIGGEST
C             FUNCTION REDUCTION THIS ITERATION.  IV(NFGCAL) REMAINS
C             UNCHANGED UNTIL A FUNCTION REDUCTION IS OBTAINED.
C IV(RADINC) (I/O) THE NUMBER OF RADIUS INCREASES (OR MINUS THE NUMBER
C             OF DECREASES) SO FAR THIS ITERATION.
C IV(RESTOR) (OUT) SET TO 1 IF V(F) HAS BEEN RESTORED AND X SHOULD BE
C             RESTORED TO ITS INITIAL VALUE, TO 2 IF X SHOULD BE SAVED,
C             TO 3 IF X SHOULD BE RESTORED FROM THE SAVED VALUE, AND TO
C             0 OTHERWISE.
C  IV(STAGE) (I/O) COUNT OF THE NUMBER OF MODELS TRIED SO FAR IN THE
C             CURRENT ITERATION.
C IV(STGLIM) (IN)  MAXIMUM NUMBER OF MODELS TO CONSIDER.
C IV(SWITCH) (OUT) SET TO 0 UNLESS A NEW MODEL IS BEING TRIED AND IT
C             GIVES A SMALLER FUNCTION VALUE THAN THE PREVIOUS MODEL,
C             IN WHICH CASE A7SST SETS IV(SWITCH) = 1.
C IV(TOOBIG) (I/O)  IS NONZERO ON INPUT IF STEP WAS TOO BIG (E.G., IF
C             IT WOULD CAUSE OVERFLOW).  IT IS SET TO 0 ON RETURN.
C   IV(XIRC) (I/O) VALUE THAT IV(IRC) WOULD HAVE IN THE ABSENCE OF
C             CONVERGENCE, FALSE CONVERGENCE, AND OVERSIZED STEPS.
C
C  ***  V VALUES REFERENCED  ***
C
C V(AFCTOL) (IN)  ABSOLUTE FUNCTION CONVERGENCE TOLERANCE.  IF THE
C             ABSOLUTE VALUE OF THE CURRENT FUNCTION VALUE V(F) IS LESS
C             THAN V(AFCTOL) AND A7SST DOES NOT RETURN WITH
C             IV(IRC) = 11, THEN A7SST RETURNS WITH IV(IRC) = 10.
C V(DECFAC) (IN)  FACTOR BY WHICH TO DECREASE RADIUS WHEN IV(TOOBIG) IS
C             NONZERO.
C V(DSTNRM) (IN)  THE 2-NORM OF D*STEP.
C V(DSTSAV) (I/O) VALUE OF V(DSTNRM) ON SAVED STEP.
C   V(DST0) (IN)  THE 2-NORM OF D TIMES THE NEWTON STEP (WHEN DEFINED,
C             I.E., FOR V(NREDUC) .GE. 0).
C      V(F) (I/O) ON BOTH INPUT AND OUTPUT, V(F) IS THE OBJECTIVE FUNC-
C             TION VALUE AT X.  IF X IS RESTORED TO A PREVIOUS VALUE,
C             THEN V(F) IS RESTORED TO THE CORRESPONDING VALUE.
C   V(FDIF) (OUT) THE FUNCTION REDUCTION V(F0) - V(F) (FOR THE OUTPUT
C             VALUE OF V(F) IF AN EARLIER STEP GAVE A BIGGER FUNCTION
C             DECREASE, AND FOR THE INPUT VALUE OF V(F) OTHERWISE).
C V(FLSTGD) (I/O) SAVED VALUE OF V(F).
C     V(F0) (IN)  OBJECTIVE FUNCTION VALUE AT START OF ITERATION.
C V(GTSLST) (I/O) VALUE OF V(GTSTEP) ON SAVED STEP.
C V(GTSTEP) (IN)  INNER PRODUCT BETWEEN STEP AND GRADIENT.
C V(INCFAC) (IN)  MINIMUM FACTOR BY WHICH TO INCREASE RADIUS.
C  V(LMAXS) (IN)  MAXIMUM REASONABLE STEP SIZE (AND INITIAL STEP BOUND).
C             IF THE ACTUAL FUNCTION DECREASE IS NO MORE THAN TWICE
C             WHAT WAS PREDICTED, IF A RETURN WITH IV(IRC) = 7, 8, OR 9
C             DOES NOT OCCUR, IF V(DSTNRM) .GT. V(LMAXS) OR THE CURRENT
C             STEP IS A NEWTON STEP, AND IF
C             V(PREDUC) .LE. V(SCTOL) * ABS(V(F0)), THEN A7SST RETURNS
C             WITH IV(IRC) = 11.  IF SO DOING APPEARS WORTHWHILE, THEN
C             A7SST REPEATS THIS TEST (DISALLOWING A FULL NEWTON STEP)
C             WITH V(PREDUC) COMPUTED FOR A STEP OF LENGTH V(LMAXS)
C             (BY A RETURN WITH IV(IRC) = 6).
C V(NREDUC) (I/O)  FUNCTION REDUCTION PREDICTED BY QUADRATIC MODEL FOR
C             NEWTON STEP.  IF A7SST IS CALLED WITH IV(IRC) = 6, I.E.,
C             IF V(PREDUC) HAS BEEN COMPUTED WITH RADIUS = V(LMAXS) FOR
C             _USE_ IN THE SINGULAR CONVERGENCE TEST, THEN V(NREDUC) IS
C             SET TO -V(PREDUC) BEFORE THE LATTER IS RESTORED.
C V(PLSTGD) (I/O) VALUE OF V(PREDUC) ON SAVED STEP.
C V(PREDUC) (I/O) FUNCTION REDUCTION PREDICTED BY QUADRATIC MODEL FOR
C             CURRENT STEP.
C V(RADFAC) (OUT) FACTOR TO BE USED IN DETERMINING THE NEW RADIUS,
C             WHICH SHOULD BE V(RADFAC)*DST, WHERE  DST  IS EITHER THE
C             OUTPUT VALUE OF V(DSTNRM) OR THE 2-NORM OF
C             DIAG(NEWD)*STEP  FOR THE OUTPUT VALUE OF STEP AND THE
C             UPDATED VERSION, NEWD, OF THE SCALE VECTOR D.  FOR
C             IV(IRC) = 3, V(RADFAC) = 1.0 IS RETURNED.
C V(RDFCMN) (IN)  MINIMUM VALUE FOR V(RADFAC) IN TERMS OF THE INPUT
C             VALUE OF V(DSTNRM) -- SUGGESTED VALUE = 0.1.
C V(RDFCMX) (IN)  MAXIMUM VALUE FOR V(RADFAC) -- SUGGESTED VALUE = 4.0.
C  V(RELDX) (IN) SCALED RELATIVE CHANGE IN X CAUSED BY STEP, COMPUTED
C             (E.G.) BY FUNCTION   RLDST  AS
C                 MAX (D(I)*ABS(X(I)-X0(I)), 1 .LE. I .LE. P) /
C                    MAX (D(I)*(ABS(X(I))+ABS(X0(I))), 1 .LE. I .LE. P).
C V(RFCTOL) (IN)  RELATIVE FUNCTION CONVERGENCE TOLERANCE.  IF THE
C             ACTUAL FUNCTION REDUCTION IS AT MOST TWICE WHAT WAS PRE-
C             DICTED AND  V(NREDUC) .LE. V(RFCTOL)*ABS(V(F0)),  THEN
C             A7SST RETURNS WITH IV(IRC) = 8 OR 9.
C  V(SCTOL) (IN)  SINGULAR CONVERGENCE TOLERANCE -- SEE V(LMAXS).
C V(STPPAR) (IN)  MARQUARDT PARAMETER -- 0 MEANS FULL NEWTON STEP.
C V(TUNER1) (IN)  TUNING CONSTANT USED TO DECIDE IF THE FUNCTION
C             REDUCTION WAS MUCH LESS THAN EXPECTED.  SUGGESTED
C             VALUE = 0.1.
C V(TUNER2) (IN)  TUNING CONSTANT USED TO DECIDE IF THE FUNCTION
C             REDUCTION WAS LARGE ENOUGH TO ACCEPT STEP.  SUGGESTED
C             VALUE = 10**-4.
C V(TUNER3) (IN)  TUNING CONSTANT USED TO DECIDE IF THE RADIUS
C             SHOULD BE INCREASED.  SUGGESTED VALUE = 0.75.
C  V(XCTOL) (IN)  X-CONVERGENCE CRITERION.  IF STEP IS A NEWTON STEP
C             (V(STPPAR) = 0) HAVING V(RELDX) .LE. V(XCTOL) AND GIVING
C             AT MOST TWICE THE PREDICTED FUNCTION DECREASE, THEN
C             A7SST RETURNS IV(IRC) = 7 OR 9.
C  V(XFTOL) (IN)  FALSE CONVERGENCE TOLERANCE.  IF STEP GAVE NO OR ONLY
C             A SMALL FUNCTION DECREASE AND V(RELDX) .LE. V(XFTOL),
C             THEN A7SST RETURNS WITH IV(IRC) = 12.
C
C-------------------------------  NOTES  -------------------------------
C
C  ***  APPLICATION AND USAGE RESTRICTIONS  ***
C
C        THIS ROUTINE IS CALLED AS PART OF THE NL2SOL (NONLINEAR
C     LEAST-SQUARES) PACKAGE.  IT MAY BE USED IN ANY UNCONSTRAINED
C     MINIMIZATION SOLVER THAT USES DOGLEG, GOLDFELD-QUANDT-TROTTER,
C     OR LEVENBERG-MARQUARDT STEPS.
C
C  ***  ALGORITHM NOTES  ***
C
C        SEE (1) FOR FURTHER DISCUSSION OF THE ASSESSING AND MODEL
C     SWITCHING STRATEGIES.  WHILE NL2SOL CONSIDERS ONLY TWO MODELS,
C     A7SST IS DESIGNED TO HANDLE ANY NUMBER OF MODELS.
C
C  ***  USAGE NOTES  ***
C
C        ON THE FIRST CALL OF AN ITERATION, ONLY THE I/O VARIABLES
C     STEP, X, IV(IRC), IV(MODEL), V(F), V(DSTNRM), V(GTSTEP), AND
C     V(PREDUC) NEED HAVE BEEN INITIALIZED.  BETWEEN CALLS, NO I/O
C     VALUES EXCEPT STEP, X, IV(MODEL), V(F) AND THE STOPPING TOLER-
C     ANCES SHOULD BE CHANGED.
C        AFTER A RETURN FOR CONVERGENCE OR FALSE CONVERGENCE, ONE CAN
C     CHANGE THE STOPPING TOLERANCES AND CALL A7SST AGAIN, IN WHICH
C     CASE THE STOPPING TESTS WILL BE REPEATED.
C
C  ***  REFERENCES  ***
C
C     (1) DENNIS, J.E., JR., GAY, D.M., AND WELSCH, R.E. (1981),
C        AN ADAPTIVE NONLINEAR LEAST-SQUARES ALGORITHM,
C        ACM TRANS. MATH. SOFTWARE, VOL. 7, NO. 3.
C
C     (2) POWELL, M.J.D. (1970)  A FORTRAN SUBROUTINE FOR SOLVING
C        SYSTEMS OF NONLINEAR ALGEBRAIC EQUATIONS, IN NUMERICAL
C        METHODS FOR NONLINEAR ALGEBRAIC EQUATIONS, EDITED BY
C        P. RABINOWITZ, GORDON AND BREACH, LONDON.
C
C  ***  HISTORY  ***
C
C        JOHN DENNIS DESIGNED MUCH OF THIS ROUTINE, STARTING WITH
C     IDEAS IN (2). ROY WELSCH SUGGESTED THE MODEL SWITCHING STRATEGY.
C        DAVID GAY AND STEPHEN PETERS CAST THIS SUBROUTINE INTO A MORE
C     PORTABLE FORM (WINTER 1977), AND DAVID GAY CAST IT INTO ITS
C     PRESENT FORM (FALL 1978), WITH MINOR CHANGES TO THE SINGULAR
C     CONVERGENCE TEST IN MAY, 1984 (TO DEAL WITH FULL NEWTON STEPS).
C
C  ***  GENERAL  ***
C
C     THIS SUBROUTINE WAS WRITTEN IN CONNECTION WITH RESEARCH
C     SUPPORTED BY THE NATIONAL SCIENCE FOUNDATION UNDER GRANTS
C     MCS-7600324, DCR75-10143, 76-14311DSS, MCS76-11989, AND
C     MCS-7906671.
C
C------------------------  EXTERNAL QUANTITIES  ------------------------
C
C  ***  NO EXTERNAL FUNCTIONS AND SUBROUTINES  ***
C
C--------------------------  LOCAL VARIABLES  --------------------------
C
      LOGICAL GOODX
      INTEGER I, NFC
      REAL EMAX, EMAXS, GTS, RFAC1, XMAX
      REAL HALF, ONE, ONEP2, TWO, ZERO
C
C  ***  SUBSCRIPTS FOR IV AND V  ***
C
      INTEGER AFCTOL, DECFAC, DSTNRM, DSTSAV, DST0, F, FDIF, FLSTGD, F0,
     1        GTSLST, GTSTEP, INCFAC, IRC, LMAXS, MLSTGD, MODEL, NFCALL,
     2        NFGCAL, NREDUC, PLSTGD, PREDUC, RADFAC, RADINC, RDFCMN,
     3        RDFCMX, RELDX, RESTOR, RFCTOL, SCTOL, STAGE, STGLIM,
     4        STPPAR, SWITCH, TOOBIG, TUNER1, TUNER2, TUNER3, XCTOL,
     5        XFTOL, XIRC
C
C  ***  DATA INITIALIZATIONS  ***
C
C/6
C     DATA HALF/0.5E+0/, ONE/1.E+0/, ONEP2/1.2E+0/, TWO/2.E+0/,
C    1     ZERO/0.E+0/
C/7
      PARAMETER (HALF=0.5E+0, ONE=1.E+0, ONEP2=1.2E+0, TWO=2.E+0,
     1           ZERO=0.E+0)
C/
C
C/6
C     DATA IRC/29/, MLSTGD/32/, MODEL/5/, NFCALL/6/, NFGCAL/7/,
C    1     RADINC/8/, RESTOR/9/, STAGE/10/, STGLIM/11/, SWITCH/12/,
C    2     TOOBIG/2/, XIRC/13/
C/7
      PARAMETER (IRC=29, MLSTGD=32, MODEL=5, NFCALL=6, NFGCAL=7,
     1           RADINC=8, RESTOR=9, STAGE=10, STGLIM=11, SWITCH=12,
     2           TOOBIG=2, XIRC=13)
C/
C/6
C     DATA AFCTOL/31/, DECFAC/22/, DSTNRM/2/, DST0/3/, DSTSAV/18/,
C    1     F/10/, FDIF/11/, FLSTGD/12/, F0/13/, GTSLST/14/, GTSTEP/4/,
C    2     INCFAC/23/, LMAXS/36/, NREDUC/6/, PLSTGD/15/, PREDUC/7/,
C    3     RADFAC/16/, RDFCMN/24/, RDFCMX/25/, RELDX/17/, RFCTOL/32/,
C    4     SCTOL/37/, STPPAR/5/, TUNER1/26/, TUNER2/27/, TUNER3/28/,
C    5     XCTOL/33/, XFTOL/34/
C/7
      PARAMETER (AFCTOL=31, DECFAC=22, DSTNRM=2, DST0=3, DSTSAV=18,
     1           F=10, FDIF=11, FLSTGD=12, F0=13, GTSLST=14, GTSTEP=4,
     2           INCFAC=23, LMAXS=36, NREDUC=6, PLSTGD=15, PREDUC=7,
     3           RADFAC=16, RDFCMN=24, RDFCMX=25, RELDX=17, RFCTOL=32,
     4           SCTOL=37, STPPAR=5, TUNER1=26, TUNER2=27, TUNER3=28,
     5           XCTOL=33, XFTOL=34)
C/
C
C+++++++++++++++++++++++++++++++  BODY  ++++++++++++++++++++++++++++++++
C
      NFC = IV(NFCALL)
      IV(SWITCH) = 0
      IV(RESTOR) = 0
      RFAC1 = ONE
      GOODX = .TRUE.
      I = IV(IRC)
      IF (I .GE. 1 .AND. I .LE. 12)
     1             GO TO (20,30,10,10,40,280,220,220,220,220,220,170), I
         IV(IRC) = 13
         GO TO 999
C
C  ***  INITIALIZE FOR NEW ITERATION  ***
C
 10   IV(STAGE) = 1
      IV(RADINC) = 0
      V(FLSTGD) = V(F0)
      IF (IV(TOOBIG) .EQ. 0) GO TO 110
         IV(STAGE) = -1
         IV(XIRC) = I
         GO TO 60
C
C  ***  STEP WAS RECOMPUTED WITH NEW MODEL OR SMALLER RADIUS  ***
C  ***  FIRST DECIDE WHICH  ***
C
 20   IF (IV(MODEL) .NE. IV(MLSTGD)) GO TO 30
C        ***  OLD MODEL RETAINED, SMALLER RADIUS TRIED  ***
C        ***  DO NOT CONSIDER ANY MORE NEW MODELS THIS ITERATION  ***
         IV(STAGE) = IV(STGLIM)
         IV(RADINC) = -1
         GO TO 110
C
C  ***  A NEW MODEL IS BEING TRIED.  DECIDE WHETHER TO KEEP IT.  ***
C
 30   IV(STAGE) = IV(STAGE) + 1
C
C     ***  NOW WE ADD THE POSSIBILITY THAT STEP WAS RECOMPUTED WITH  ***
C     ***  THE SAME MODEL, PERHAPS BECAUSE OF AN OVERSIZED STEP.     ***
C
 40   IF (IV(STAGE) .GT. 0) GO TO 50
C
C        ***  STEP WAS RECOMPUTED BECAUSE IT WAS TOO BIG.  ***
C
         IF (IV(TOOBIG) .NE. 0) GO TO 60
C
C        ***  RESTORE IV(STAGE) AND PICK UP WHERE WE LEFT OFF.  ***
C
         IV(STAGE) = -IV(STAGE)
         I = IV(XIRC)
         GO TO (20, 30, 110, 110, 70), I
C
 50   IF (IV(TOOBIG) .EQ. 0) GO TO 70
C
C  ***  HANDLE OVERSIZE STEP  ***
C
      IV(TOOBIG) = 0
      IF (IV(RADINC) .GT. 0) GO TO 80
         IV(STAGE) = -IV(STAGE)
         IV(XIRC) = IV(IRC)
C
 60      IV(TOOBIG) = 0
         V(RADFAC) = V(DECFAC)
         IV(RADINC) = IV(RADINC) - 1
         IV(IRC) = 5
         IV(RESTOR) = 1
         V(F) = V(FLSTGD)
         GO TO 999
C
 70   IF (V(F) .LT. V(FLSTGD)) GO TO 110
C
C     *** THE NEW STEP IS A LOSER.  RESTORE OLD MODEL.  ***
C
      IF (IV(MODEL) .EQ. IV(MLSTGD)) GO TO 80
         IV(MODEL) = IV(MLSTGD)
         IV(SWITCH) = 1
C
C     ***  RESTORE STEP, ETC. ONLY IF A PREVIOUS STEP DECREASED V(F).
C
 80   IF (V(FLSTGD) .GE. V(F0)) GO TO 110
         IF (IV(STAGE) .LT. IV(STGLIM)) THEN
            GOODX = .FALSE.
         ELSE IF (NFC .LT. IV(NFGCAL) + IV(STGLIM) + 2) THEN
            GOODX = .FALSE.
         ELSE IF (IV(SWITCH) .NE. 0) THEN
            GOODX = .FALSE.
            ENDIF
         IV(RESTOR) = 3
         V(F) = V(FLSTGD)
         V(PREDUC) = V(PLSTGD)
         V(GTSTEP) = V(GTSLST)
         IF (IV(SWITCH) .EQ. 0) RFAC1 = V(DSTNRM) / V(DSTSAV)
         V(DSTNRM) = V(DSTSAV)
         IF (GOODX) THEN
C
C     ***  ACCEPT PREVIOUS SLIGHTLY REDUCING STEP ***
C
            V(FDIF) = V(F0) - V(F)
            IV(IRC) = 4
            V(RADFAC) = RFAC1
            GO TO 999
            ENDIF
         NFC = IV(NFGCAL)
C
 110  V(FDIF) = V(F0) - V(F)
      IF (V(FDIF) .GT. V(TUNER2) * V(PREDUC)) GO TO 140
      IF (IV(RADINC) .GT. 0) GO TO 140
C
C        ***  NO (OR ONLY A TRIVIAL) FUNCTION DECREASE
C        ***  -- SO TRY NEW MODEL OR SMALLER RADIUS
C
         IF (V(F) .LT. V(F0)) GO TO 120
              IV(MLSTGD) = IV(MODEL)
              V(FLSTGD) = V(F)
              V(F) = V(F0)
              IV(RESTOR) = 1
              GO TO 130
 120     IV(NFGCAL) = NFC
 130     IV(IRC) = 1
         IF (IV(STAGE) .LT. IV(STGLIM)) GO TO 160
              IV(IRC) = 5
              IV(RADINC) = IV(RADINC) - 1
              GO TO 160
C
C  ***  NONTRIVIAL FUNCTION DECREASE ACHIEVED  ***
C
 140  IV(NFGCAL) = NFC
      RFAC1 = ONE
      V(DSTSAV) = V(DSTNRM)
      IF (V(FDIF) .GT. V(PREDUC)*V(TUNER1)) GO TO 190
C
C  ***  DECREASE WAS MUCH LESS THAN PREDICTED -- EITHER CHANGE MODELS
C  ***  OR ACCEPT STEP WITH DECREASED RADIUS.
C
      IF (IV(STAGE) .GE. IV(STGLIM)) GO TO 150
C        ***  CONSIDER SWITCHING MODELS  ***
         IV(IRC) = 2
         GO TO 160
C
C     ***  ACCEPT STEP WITH DECREASED RADIUS  ***
C
 150  IV(IRC) = 4
C
C  ***  SET V(RADFAC) TO FLETCHER*S DECREASE FACTOR  ***
C
 160  IV(XIRC) = IV(IRC)
      EMAX = V(GTSTEP) + V(FDIF)
      V(RADFAC) = HALF * RFAC1
      IF (EMAX .LT. V(GTSTEP)) V(RADFAC) = RFAC1 * AMAX1(V(RDFCMN),
     1                                           HALF * V(GTSTEP)/EMAX)
C
C  ***  DO FALSE CONVERGENCE TEST  ***
C
 170  IF (V(RELDX) .LE. V(XFTOL)) GO TO 180
         IV(IRC) = IV(XIRC)
         IF (V(F) .LT. V(F0)) GO TO 200
              GO TO 230
C
 180  IV(IRC) = 12
      GO TO 240
C
C  ***  HANDLE GOOD FUNCTION DECREASE  ***
C
 190  IF (V(FDIF) .LT. (-V(TUNER3) * V(GTSTEP))) GO TO 210
C
C     ***  INCREASING RADIUS LOOKS WORTHWHILE.  SEE IF WE JUST
C     ***  RECOMPUTED STEP WITH A DECREASED RADIUS OR RESTORED STEP
C     ***  AFTER RECOMPUTING IT WITH A LARGER RADIUS.
C
      IF (IV(RADINC) .LT. 0) GO TO 210
      IF (IV(RESTOR) .EQ. 1) GO TO 210
      IF (IV(RESTOR) .EQ. 3) GO TO 210
C
C        ***  WE DID NOT.  TRY A LONGER STEP UNLESS THIS WAS A NEWTON
C        ***  STEP.
C
         V(RADFAC) = V(RDFCMX)
         GTS = V(GTSTEP)
         IF (V(FDIF) .LT. (HALF/V(RADFAC) - ONE) * GTS)
     1            V(RADFAC) = AMAX1(V(INCFAC), HALF*GTS/(GTS + V(FDIF)))
         IV(IRC) = 4
         IF (V(STPPAR) .EQ. ZERO) GO TO 230
         IF (V(DST0) .GE. ZERO .AND. (V(DST0) .LT. TWO*V(DSTNRM)
     1             .OR. V(NREDUC) .LT. ONEP2*V(FDIF)))  GO TO 230
C             ***  STEP WAS NOT A NEWTON STEP.  RECOMPUTE IT WITH
C             ***  A LARGER RADIUS.
              IV(IRC) = 5
              IV(RADINC) = IV(RADINC) + 1
C
C  ***  SAVE VALUES CORRESPONDING TO GOOD STEP  ***
C
 200  V(FLSTGD) = V(F)
      IV(MLSTGD) = IV(MODEL)
      IF (IV(RESTOR) .EQ. 0) IV(RESTOR) = 2
      V(DSTSAV) = V(DSTNRM)
      IV(NFGCAL) = NFC
      V(PLSTGD) = V(PREDUC)
      V(GTSLST) = V(GTSTEP)
      GO TO 230
C
C  ***  ACCEPT STEP WITH RADIUS UNCHANGED  ***
C
 210  V(RADFAC) = ONE
      IV(IRC) = 3
      GO TO 230
C
C  ***  COME HERE FOR A RESTART AFTER CONVERGENCE  ***
C
 220  IV(IRC) = IV(XIRC)
      IF (V(DSTSAV) .GE. ZERO) GO TO 240
         IV(IRC) = 12
         GO TO 240
C
C  ***  PERFORM CONVERGENCE TESTS  ***
C
 230  IV(XIRC) = IV(IRC)
 240  IF (IV(RESTOR) .EQ. 1 .AND. V(FLSTGD) .LT. V(F0)) IV(RESTOR) = 3
      IF ( ABS(V(F)) .LT. V(AFCTOL)) IV(IRC) = 10
      IF (HALF * V(FDIF) .GT. V(PREDUC)) GO TO 999
      EMAX = V(RFCTOL) *  ABS(V(F0))
      EMAXS = V(SCTOL) *  ABS(V(F0))
      IF (V(PREDUC) .LE. EMAXS .AND. (V(DSTNRM) .GT. V(LMAXS) .OR.
     1     V(STPPAR) .EQ. ZERO)) IV(IRC) = 11
      IF (V(DST0) .LT. ZERO) GO TO 250
      I = 0
      IF ((V(NREDUC) .GT. ZERO .AND. V(NREDUC) .LE. EMAX) .OR.
     1    (V(NREDUC) .EQ. ZERO. AND. V(PREDUC) .EQ. ZERO))  I = 2
      IF (V(STPPAR) .EQ. ZERO .AND. V(RELDX) .LE. V(XCTOL)
     1                        .AND. GOODX)                  I = I + 1
      IF (I .GT. 0) IV(IRC) = I + 6
C
C  ***  CONSIDER RECOMPUTING STEP OF LENGTH V(LMAXS) FOR SINGULAR
C  ***  CONVERGENCE TEST.
C
 250  IF (IV(IRC) .GT. 5 .AND. IV(IRC) .NE. 12) GO TO 999
      IF (V(STPPAR) .EQ. ZERO) GO TO 999
      IF (V(DSTNRM) .GT. V(LMAXS)) GO TO 260
         IF (V(PREDUC) .GE. EMAXS) GO TO 999
              IF (V(DST0) .LE. ZERO) GO TO 270
                   IF (HALF * V(DST0) .LE. V(LMAXS)) GO TO 999
                        GO TO 270
 260  IF (HALF * V(DSTNRM) .LE. V(LMAXS)) GO TO 999
      XMAX = V(LMAXS) / V(DSTNRM)
      IF (XMAX * (TWO - XMAX) * V(PREDUC) .GE. EMAXS) GO TO 999
 270  IF (V(NREDUC) .LT. ZERO) GO TO 290
C
C  ***  RECOMPUTE V(PREDUC) FOR _USE_ IN SINGULAR CONVERGENCE TEST  ***
C
      V(GTSLST) = V(GTSTEP)
      V(DSTSAV) = V(DSTNRM)
      IF (IV(IRC) .EQ. 12) V(DSTSAV) = -V(DSTSAV)
      V(PLSTGD) = V(PREDUC)
      I = IV(RESTOR)
      IV(RESTOR) = 2
      IF (I .EQ. 3) IV(RESTOR) = 0
      IV(IRC) = 6
      GO TO 999
C
C  ***  PERFORM SINGULAR CONVERGENCE TEST WITH RECOMPUTED V(PREDUC)  ***
C
 280  V(GTSTEP) = V(GTSLST)
      V(DSTNRM) =  ABS(V(DSTSAV))
      IV(IRC) = IV(XIRC)
      IF (V(DSTSAV) .LE. ZERO) IV(IRC) = 12
      V(NREDUC) = -V(PREDUC)
      V(PREDUC) = V(PLSTGD)
      IV(RESTOR) = 3
 290  IF (-V(NREDUC) .LE. V(SCTOL) *  ABS(V(F0))) IV(IRC) = 11
C
 999  RETURN
C
C  ***  LAST LINE OF A7SST FOLLOWS  ***
      END
