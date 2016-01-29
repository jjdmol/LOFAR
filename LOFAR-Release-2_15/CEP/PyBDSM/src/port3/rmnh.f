      SUBROUTINE  RMNH(D, FX, G, H, IV, LH, LIV, LV, N, V, X)
C
C  ***  CARRY OUT   MNH (UNCONSTRAINED MINIMIZATION) ITERATIONS, USING
C  ***  HESSIAN MATRIX PROVIDED BY THE CALLER.
C
C  ***  PARAMETER DECLARATIONS  ***
C
      INTEGER LH, LIV, LV, N
      INTEGER IV(LIV)
      REAL D(N), FX, G(N), H(LH), V(LV), X(N)
C
C--------------------------  PARAMETER USAGE  --------------------------
C
C D.... SCALE VECTOR.
C FX... FUNCTION VALUE.
C G.... GRADIENT VECTOR.
C H.... LOWER TRIANGLE OF THE HESSIAN, STORED ROWWISE.
C IV... INTEGER VALUE ARRAY.
C LH... LENGTH OF H = P*(P+1)/2.
C LIV.. LENGTH OF IV (AT LEAST 60).
C LV... LENGTH OF V (AT LEAST 78 + N*(N+21)/2).
C N.... NUMBER OF VARIABLES (COMPONENTS IN X AND G).
C V.... FLOATING-POINT VALUE ARRAY.
C X.... PARAMETER VECTOR.
C
C  ***  DISCUSSION  ***
C
C        PARAMETERS IV, N, V, AND X ARE THE SAME AS THE CORRESPONDING
C     ONES TO   MNH (WHICH SEE), EXCEPT THAT V CAN BE SHORTER (SINCE
C     THE PART OF V THAT   MNH USES FOR STORING G AND H IS NOT NEEDED).
C     MOREOVER, COMPARED WITH   MNH, IV(1) MAY HAVE THE TWO ADDITIONAL
C     OUTPUT VALUES 1 AND 2, WHICH ARE EXPLAINED BELOW, AS IS THE USE
C     OF IV(TOOBIG) AND IV(NFGCAL).  THE VALUE IV(G), WHICH IS AN
C     OUTPUT VALUE FROM   MNH, IS NOT REFERENCED BY  RMNH OR THE
C     SUBROUTINES IT CALLS.
C
C IV(1) = 1 MEANS THE CALLER SHOULD SET FX TO F(X), THE FUNCTION VALUE
C             AT X, AND CALL  RMNH AGAIN, HAVING CHANGED NONE OF THE
C             OTHER PARAMETERS.  AN EXCEPTION OCCURS IF F(X) CANNOT BE
C             COMPUTED (E.G. IF OVERFLOW WOULD OCCUR), WHICH MAY HAPPEN
C             BECAUSE OF AN OVERSIZED STEP.  IN THIS CASE THE CALLER
C             SHOULD SET IV(TOOBIG) = IV(2) TO 1, WHICH WILL CAUSE
C              RMNH TO IGNORE FX AND TRY A SMALLER STEP.  THE PARA-
C             METER NF THAT   MNH PASSES TO CALCF (FOR POSSIBLE _USE_ BY
C             CALCGH) IS A COPY OF IV(NFCALL) = IV(6).
C IV(1) = 2 MEANS THE CALLER SHOULD SET G TO G(X), THE GRADIENT OF F AT
C             X, AND H TO THE LOWER TRIANGLE OF H(X), THE HESSIAN OF F
C             AT X, AND CALL  RMNH AGAIN, HAVING CHANGED NONE OF THE
C             OTHER PARAMETERS EXCEPT PERHAPS THE SCALE VECTOR D.
C                  THE PARAMETER NF THAT   MNH PASSES TO CALCG IS
C             IV(NFGCAL) = IV(7).  IF G(X) AND H(X) CANNOT BE EVALUATED,
C             THEN THE CALLER MAY SET IV(TOOBIG) TO 0, IN WHICH CASE
C              RMNH WILL RETURN WITH IV(1) = 65.
C                  NOTE --  RMNH OVERWRITES H WITH THE LOWER TRIANGLE
C             OF  DIAG(D)**-1 * H(X) * DIAG(D)**-1.
C.
C  ***  GENERAL  ***
C
C     CODED BY DAVID M. GAY (WINTER 1980).  REVISED SEPT. 1982.
C     THIS SUBROUTINE WAS WRITTEN IN CONNECTION WITH RESEARCH SUPPORTED
C     IN PART BY THE NATIONAL SCIENCE FOUNDATION UNDER GRANTS
C     MCS-7600324 AND MCS-7906671.
C
C        (SEE   MNG AND   MNH FOR REFERENCES.)
C
C+++++++++++++++++++++++++++  DECLARATIONS  ++++++++++++++++++++++++++++
C
C  ***  LOCAL VARIABLES  ***
C
      INTEGER DG1, DUMMY, I, J, K, L, LSTGST, NN1O2, RSTRST, STEP1,
     1        TEMP1, W1, X01
      REAL T
C
C     ***  CONSTANTS  ***
C
      REAL ONE, ONEP2, ZERO
C
C  ***  NO INTRINSIC FUNCTIONS  ***
C
C  ***  EXTERNAL FUNCTIONS AND SUBROUTINES  ***
C
      LOGICAL STOPX
      REAL  D7TPR,  RLDST,  V2NRM
      EXTERNAL A7SST, IVSET,  D7TPR, D7DUP, G7QTS, ITSUM, PARCK,
     1          RLDST,  S7LVM, STOPX, V2AXY, V7CPY,  V7SCP,  V2NRM
C
C A7SST.... ASSESSES CANDIDATE STEP.
C IVSET.... PROVIDES DEFAULT IV AND V INPUT VALUES.
C  D7TPR... RETURNS INNER PRODUCT OF TWO VECTORS.
C D7DUP.... UPDATES SCALE VECTOR D.
C G7QTS.... COMPUTES OPTIMALLY LOCALLY CONSTRAINED STEP.
C ITSUM.... PRINTS ITERATION SUMMARY AND INFO ON INITIAL AND FINAL X.
C PARCK.... CHECKS VALIDITY OF INPUT IV AND V VALUES.
C  RLDST... COMPUTES V(RELDX) = RELATIVE STEP SIZE.
C  S7LVM... MULTIPLIES SYMMETRIC MATRIX TIMES VECTOR, GIVEN THE LOWER
C             TRIANGLE OF THE MATRIX.
C STOPX.... RETURNS .TRUE. IF THE BREAK KEY HAS BEEN PRESSED.
C V2AXY.... COMPUTES SCALAR TIMES ONE VECTOR PLUS ANOTHER.
C V7CPY.... COPIES ONE VECTOR TO ANOTHER.
C  V7SCP... SETS ALL ELEMENTS OF A VECTOR TO A SCALAR.
C  V2NRM... RETURNS THE 2-NORM OF A VECTOR.
C
C  ***  SUBSCRIPTS FOR IV AND V  ***
C
      INTEGER CNVCOD, DG, DGNORM, DINIT, DSTNRM, DTINIT, DTOL,
     1        DTYPE, D0INIT, F, F0, FDIF, GTSTEP, INCFAC, IRC, KAGQT,
     2        LMAT, LMAX0, LMAXS, MODE, MODEL, MXFCAL, MXITER, NEXTV,
     3        NFCALL, NFGCAL, NGCALL, NITER, PHMXFC, PREDUC, RADFAC,
     4        RADINC, RADIUS, RAD0, RELDX, RESTOR, STEP, STGLIM, STLSTG,
     5        STPPAR, TOOBIG, TUNER4, TUNER5, VNEED, W, XIRC, X0
C
C  ***  IV SUBSCRIPT VALUES  ***
C
C/6
C     DATA CNVCOD/55/, DG/37/, DTOL/59/, DTYPE/16/, IRC/29/, KAGQT/33/,
C    1     LMAT/42/, MODE/35/, MODEL/5/, MXFCAL/17/, MXITER/18/,
C    2     NEXTV/47/, NFCALL/6/, NFGCAL/7/, NGCALL/30/, NITER/31/,
C    3     RADINC/8/, RESTOR/9/, STEP/40/, STGLIM/11/, STLSTG/41/,
C    4     TOOBIG/2/, VNEED/4/, W/34/, XIRC/13/, X0/43/
C/7
      PARAMETER (CNVCOD=55, DG=37, DTOL=59, DTYPE=16, IRC=29, KAGQT=33,
     1           LMAT=42, MODE=35, MODEL=5, MXFCAL=17, MXITER=18,
     2           NEXTV=47, NFCALL=6, NFGCAL=7, NGCALL=30, NITER=31,
     3           RADINC=8, RESTOR=9, STEP=40, STGLIM=11, STLSTG=41,
     4           TOOBIG=2, VNEED=4, W=34, XIRC=13, X0=43)
C/
C
C  ***  V SUBSCRIPT VALUES  ***
C
C/6
C     DATA DGNORM/1/, DINIT/38/, DSTNRM/2/, DTINIT/39/, D0INIT/40/,
C    1     F/10/, F0/13/, FDIF/11/, GTSTEP/4/, INCFAC/23/, LMAX0/35/,
C    2     LMAXS/36/, PHMXFC/21/, PREDUC/7/, RADFAC/16/, RADIUS/8/,
C    3     RAD0/9/, RELDX/17/, STPPAR/5/, TUNER4/29/, TUNER5/30/
C/7
      PARAMETER (DGNORM=1, DINIT=38, DSTNRM=2, DTINIT=39, D0INIT=40,
     1           F=10, F0=13, FDIF=11, GTSTEP=4, INCFAC=23, LMAX0=35,
     2           LMAXS=36, PHMXFC=21, PREDUC=7, RADFAC=16, RADIUS=8,
     3           RAD0=9, RELDX=17, STPPAR=5, TUNER4=29, TUNER5=30)
C/
C
C/6
C     DATA ONE/1.E+0/, ONEP2/1.2E+0/, ZERO/0.E+0/
C/7
      PARAMETER (ONE=1.E+0, ONEP2=1.2E+0, ZERO=0.E+0)
C/
C
C+++++++++++++++++++++++++++++++  BODY  ++++++++++++++++++++++++++++++++
C
      I = IV(1)
      IF (I .EQ. 1) GO TO 30
      IF (I .EQ. 2) GO TO 40
C
C  ***  CHECK VALIDITY OF IV AND V INPUT VALUES  ***
C
      IF (IV(1) .EQ. 0) CALL IVSET(2, IV, LIV, LV, V)
      IF (IV(1) .EQ. 12 .OR. IV(1) .EQ. 13)
     1     IV(VNEED) = IV(VNEED) + N*(N+21)/2 + 7
      CALL PARCK(2, D, IV, LIV, LV, N, V)
      I = IV(1) - 2
      IF (I .GT. 12) GO TO 999
      NN1O2 = N * (N + 1) / 2
      IF (LH .GE. NN1O2) GO TO (220,220,220,220,220,220,160,120,160,
     1                          10,10,20), I
         IV(1) = 66
         GO TO 400
C
C  ***  STORAGE ALLOCATION  ***
C
 10   IV(DTOL) = IV(LMAT) + NN1O2
      IV(X0) = IV(DTOL) + 2*N
      IV(STEP) = IV(X0) + N
      IV(STLSTG) = IV(STEP) + N
      IV(DG) = IV(STLSTG) + N
      IV(W) = IV(DG) + N
      IV(NEXTV) = IV(W) + 4*N + 7
      IF (IV(1) .NE. 13) GO TO 20
         IV(1) = 14
         GO TO 999
C
C  ***  INITIALIZATION  ***
C
 20   IV(NITER) = 0
      IV(NFCALL) = 1
      IV(NGCALL) = 1
      IV(NFGCAL) = 1
      IV(MODE) = -1
      IV(MODEL) = 1
      IV(STGLIM) = 1
      IV(TOOBIG) = 0
      IV(CNVCOD) = 0
      IV(RADINC) = 0
      V(RAD0) = ZERO
      V(STPPAR) = ZERO
      IF (V(DINIT) .GE. ZERO) CALL  V7SCP(N, D, V(DINIT))
      K = IV(DTOL)
      IF (V(DTINIT) .GT. ZERO) CALL  V7SCP(N, V(K), V(DTINIT))
      K = K + N
      IF (V(D0INIT) .GT. ZERO) CALL  V7SCP(N, V(K), V(D0INIT))
      IV(1) = 1
      GO TO 999
C
 30   V(F) = FX
      IF (IV(MODE) .GE. 0) GO TO 220
      V(F0) = FX
      IV(1) = 2
      IF (IV(TOOBIG) .EQ. 0) GO TO 999
         IV(1) = 63
         GO TO 400
C
C  ***  MAKE SURE GRADIENT COULD BE COMPUTED  ***
C
 40   IF (IV(TOOBIG) .EQ. 0) GO TO 50
         IV(1) = 65
         GO TO 400
C
C  ***  UPDATE THE SCALE VECTOR D  ***
C
 50   DG1 = IV(DG)
      IF (IV(DTYPE) .LE. 0) GO TO 70
      K = DG1
      J = 0
      DO 60 I = 1, N
         J = J + I
         V(K) = H(J)
         K = K + 1
 60      CONTINUE
      CALL D7DUP(D, V(DG1), IV, LIV, LV, N, V)
C
C  ***  COMPUTE SCALED GRADIENT AND ITS NORM  ***
C
 70   DG1 = IV(DG)
      K = DG1
      DO 80 I = 1, N
         V(K) = G(I) / D(I)
         K = K + 1
 80      CONTINUE
      V(DGNORM) =  V2NRM(N, V(DG1))
C
C  ***  COMPUTE SCALED HESSIAN  ***
C
      K = 1
      DO 100 I = 1, N
         T = ONE / D(I)
         DO 90 J = 1, I
              H(K) = T * H(K) / D(J)
              K = K + 1
 90           CONTINUE
 100     CONTINUE
C
      IF (IV(CNVCOD) .NE. 0) GO TO 390
      IF (IV(MODE) .EQ. 0) GO TO 350
C
C  ***  ALLOW FIRST STEP TO HAVE SCALED 2-NORM AT MOST V(LMAX0)  ***
C
      V(RADIUS) = V(LMAX0) / (ONE + V(PHMXFC))
C
      IV(MODE) = 0
C
C
C-----------------------------  MAIN LOOP  -----------------------------
C
C
C  ***  PRINT ITERATION SUMMARY, CHECK ITERATION LIMIT  ***
C
 110  CALL ITSUM(D, G, IV, LIV, LV, N, V, X)
 120  K = IV(NITER)
      IF (K .LT. IV(MXITER)) GO TO 130
         IV(1) = 10
         GO TO 400
C
 130  IV(NITER) = K + 1
C
C  ***  INITIALIZE FOR START OF NEXT ITERATION  ***
C
      DG1 = IV(DG)
      X01 = IV(X0)
      V(F0) = V(F)
      IV(IRC) = 4
      IV(KAGQT) = -1
C
C     ***  COPY X TO X0  ***
C
      CALL V7CPY(N, V(X01), X)
C
C  ***  UPDATE RADIUS  ***
C
      IF (K .EQ. 0) GO TO 150
      STEP1 = IV(STEP)
      K = STEP1
      DO 140 I = 1, N
         V(K) = D(I) * V(K)
         K = K + 1
 140     CONTINUE
      V(RADIUS) = V(RADFAC) *  V2NRM(N, V(STEP1))
C
C  ***  CHECK STOPX AND FUNCTION EVALUATION LIMIT  ***
C
 150  IF (.NOT. STOPX(DUMMY)) GO TO 170
         IV(1) = 11
         GO TO 180
C
C     ***  COME HERE WHEN RESTARTING AFTER FUNC. EVAL. LIMIT OR STOPX.
C
 160  IF (V(F) .GE. V(F0)) GO TO 170
         V(RADFAC) = ONE
         K = IV(NITER)
         GO TO 130
C
 170  IF (IV(NFCALL) .LT. IV(MXFCAL)) GO TO 190
         IV(1) = 9
 180     IF (V(F) .GE. V(F0)) GO TO 400
C
C        ***  IN CASE OF STOPX OR FUNCTION EVALUATION LIMIT WITH
C        ***  IMPROVED V(F), EVALUATE THE GRADIENT AT X.
C
              IV(CNVCOD) = IV(1)
              GO TO 340
C
C. . . . . . . . . . . . .  COMPUTE CANDIDATE STEP  . . . . . . . . . .
C
 190  STEP1 = IV(STEP)
      DG1 = IV(DG)
      L = IV(LMAT)
      W1 = IV(W)
      CALL G7QTS(D, V(DG1), H, IV(KAGQT), V(L), N, V(STEP1), V, V(W1))
      IF (IV(IRC) .NE. 6) GO TO 200
         IF (IV(RESTOR) .NE. 2) GO TO 220
         RSTRST = 2
         GO TO 230
C
C  ***  CHECK WHETHER EVALUATING F(X0 + STEP) LOOKS WORTHWHILE  ***
C
 200  IV(TOOBIG) = 0
      IF (V(DSTNRM) .LE. ZERO) GO TO 220
      IF (IV(IRC) .NE. 5) GO TO 210
      IF (V(RADFAC) .LE. ONE) GO TO 210
      IF (V(PREDUC) .GT. ONEP2 * V(FDIF)) GO TO 210
         IF (IV(RESTOR) .NE. 2) GO TO 220
         RSTRST = 0
         GO TO 230
C
C  ***  COMPUTE F(X0 + STEP)  ***
C
 210  X01 = IV(X0)
      STEP1 = IV(STEP)
      CALL V2AXY(N, X, ONE, V(STEP1), V(X01))
      IV(NFCALL) = IV(NFCALL) + 1
      IV(1) = 1
      GO TO 999
C
C. . . . . . . . . . . . .  ASSESS CANDIDATE STEP  . . . . . . . . . . .
C
 220  RSTRST = 3
 230  X01 = IV(X0)
      V(RELDX) =  RLDST(N, D, X, V(X01))
      CALL A7SST(IV, LIV, LV, V)
      STEP1 = IV(STEP)
      LSTGST = IV(STLSTG)
      I = IV(RESTOR) + 1
      GO TO (270, 240, 250, 260), I
 240  CALL V7CPY(N, X, V(X01))
      GO TO 270
 250   CALL V7CPY(N, V(LSTGST), V(STEP1))
       GO TO 270
 260     CALL V7CPY(N, V(STEP1), V(LSTGST))
         CALL V2AXY(N, X, ONE, V(STEP1), V(X01))
         V(RELDX) =  RLDST(N, D, X, V(X01))
         IV(RESTOR) = RSTRST
C
 270  K = IV(IRC)
      GO TO (280,310,310,310,280,290,300,300,300,300,300,300,380,350), K
C
C     ***  RECOMPUTE STEP WITH NEW RADIUS  ***
C
 280     V(RADIUS) = V(RADFAC) * V(DSTNRM)
         GO TO 150
C
C  ***  COMPUTE STEP OF LENGTH V(LMAXS) FOR SINGULAR CONVERGENCE TEST.
C
 290  V(RADIUS) = V(LMAXS)
      GO TO 190
C
C  ***  CONVERGENCE OR FALSE CONVERGENCE  ***
C
 300  IV(CNVCOD) = K - 4
      IF (V(F) .GE. V(F0)) GO TO 390
         IF (IV(XIRC) .EQ. 14) GO TO 390
              IV(XIRC) = 14
C
C. . . . . . . . . . . .  PROCESS ACCEPTABLE STEP  . . . . . . . . . . .
C
 310  IF (IV(IRC) .NE. 3) GO TO 340
         TEMP1 = LSTGST
C
C     ***  PREPARE FOR GRADIENT TESTS  ***
C     ***  SET  TEMP1 = HESSIAN * STEP + G(X0)
C     ***             = DIAG(D) * (H * STEP + G(X0))
C
C        _USE_ X0 VECTOR AS TEMPORARY.
         K = X01
         DO 320 I = 1, N
              V(K) = D(I) * V(STEP1)
              K = K + 1
              STEP1 = STEP1 + 1
 320          CONTINUE
         CALL  S7LVM(N, V(TEMP1), H, V(X01))
         DO 330 I = 1, N
              V(TEMP1) = D(I) * V(TEMP1) + G(I)
              TEMP1 = TEMP1 + 1
 330          CONTINUE
C
C  ***  COMPUTE GRADIENT AND HESSIAN  ***
C
 340  IV(NGCALL) = IV(NGCALL) + 1
      IV(TOOBIG) = 0
      IV(1) = 2
      GO TO 999
C
 350  IV(1) = 2
      IF (IV(IRC) .NE. 3) GO TO 110
C
C  ***  SET V(RADFAC) BY GRADIENT TESTS  ***
C
      TEMP1 = IV(STLSTG)
      STEP1 = IV(STEP)
C
C     ***  SET  TEMP1 = DIAG(D)**-1 * (HESSIAN*STEP + (G(X0)-G(X)))  ***
C
      K = TEMP1
      DO 360 I = 1, N
         V(K) = (V(K) - G(I)) / D(I)
         K = K + 1
 360     CONTINUE
C
C     ***  DO GRADIENT TESTS  ***
C
      IF ( V2NRM(N, V(TEMP1)) .LE. V(DGNORM) * V(TUNER4)) GO TO 370
           IF ( D7TPR(N, G, V(STEP1))
     1               .GE. V(GTSTEP) * V(TUNER5))  GO TO 110
 370            V(RADFAC) = V(INCFAC)
                GO TO 110
C
C. . . . . . . . . . . . . .  MISC. DETAILS  . . . . . . . . . . . . . .
C
C  ***  BAD PARAMETERS TO ASSESS  ***
C
 380  IV(1) = 64
      GO TO 400
C
C  ***  PRINT SUMMARY OF FINAL ITERATION AND OTHER REQUESTED ITEMS  ***
C
 390  IV(1) = IV(CNVCOD)
      IV(CNVCOD) = 0
 400  CALL ITSUM(D, G, IV, LIV, LV, N, V, X)
C
 999  RETURN
C
C  ***  LAST CARD OF  RMNH FOLLOWS  ***
      END
