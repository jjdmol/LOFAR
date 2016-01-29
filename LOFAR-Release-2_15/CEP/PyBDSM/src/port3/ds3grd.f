      SUBROUTINE DS3GRD(ALPHA, B, D, ETA0, FX, G, IRC, P, W, X)
C
C  ***  COMPUTE FINITE DIFFERENCE GRADIENT BY STWEART*S SCHEME  ***
C
C     ***  PARAMETERS  ***
C
      INTEGER IRC, P
      DOUBLE PRECISION ALPHA(P), B(2,P), D(P), ETA0, FX, G(P), W(6),
     1                 X(P)
C
C.......................................................................
C
C     ***  PURPOSE  ***
C
C        THIS SUBROUTINE USES AN EMBELLISHED FORM OF THE FINITE-DIFFER-
C     ENCE SCHEME PROPOSED BY STEWART (REF. 1) TO APPROXIMATE THE
C     GRADIENT OF THE FUNCTION F(X), WHOSE VALUES ARE SUPPLIED BY
C     REVERSE COMMUNICATION.
C
C     ***  PARAMETER DESCRIPTION  ***
C
C  ALPHA IN  (APPROXIMATE) DIAGONAL ELEMENTS OF THE HESSIAN OF F(X).
C      B IN  ARRAY OF SIMPLE LOWER AND UPPER BOUNDS ON X.  X MUST
C             SATISFY B(1,I) .LE. X(I) .LE. B(2,I), I = 1(1)P.
C             FOR ALL I WITH B(1,I) .GE. B(2,I), DS3GRD SIMPLY
C             SETS G(I) TO 0.
C      D IN  SCALE VECTOR SUCH THAT D(I)*X(I), I = 1,...,P, ARE IN
C             COMPARABLE UNITS.
C   ETA0 IN  ESTIMATED BOUND ON RELATIVE ERROR IN THE FUNCTION VALUE...
C             (TRUE VALUE) = (COMPUTED VALUE)*(1+E),   WHERE
C             ABS(E) .LE. ETA0.
C     FX I/O ON INPUT,  FX  MUST BE THE COMPUTED VALUE OF F(X).  ON
C             OUTPUT WITH IRC = 0, FX HAS BEEN RESTORED TO ITS ORIGINAL
C             VALUE, THE ONE IT HAD WHEN DS3GRD WAS LAST CALLED WITH
C             IRC = 0.
C      G I/O ON INPUT WITH IRC = 0, G SHOULD CONTAIN AN APPROXIMATION
C             TO THE GRADIENT OF F NEAR X, E.G., THE GRADIENT AT THE
C             PREVIOUS ITERATE.  WHEN DS3GRD RETURNS WITH IRC = 0, G IS
C             THE DESIRED FINITE-DIFFERENCE APPROXIMATION TO THE
C             GRADIENT AT X.
C    IRC I/O INPUT/RETURN CODE... BEFORE THE VERY FIRST CALL ON DS3GRD,
C             THE CALLER MUST SET IRC TO 0.  WHENEVER DS3GRD RETURNS A
C             NONZERO VALUE (OF AT MOST P) FOR IRC, IT HAS PERTURBED
C             SOME COMPONENT OF X... THE CALLER SHOULD EVALUATE F(X)
C             AND CALL DS3GRD AGAIN WITH FX = F(X).  IF B PREVENTS
C             ESTIMATING G(I) I.E., IF THERE IS AN I WITH
C             B(1,I) .LT. B(2,I) BUT WITH B(1,I) SO CLOSE TO B(2,I)
C             THAT THE FINITE-DIFFERENCING STEPS CANNOT BE CHOSEN,
C             THEN DS3GRD RETURNS WITH IRC .GT. P.
C      P IN  THE NUMBER OF VARIABLES (COMPONENTS OF X) ON WHICH F
C             DEPENDS.
C      X I/O ON INPUT WITH IRC = 0, X IS THE POINT AT WHICH THE
C             GRADIENT OF F IS DESIRED.  ON OUTPUT WITH IRC NONZERO, X
C             IS THE POINT AT WHICH F SHOULD BE EVALUATED.  ON OUTPUT
C             WITH IRC = 0, X HAS BEEN RESTORED TO ITS ORIGINAL VALUE
C             (THE ONE IT HAD WHEN DS3GRD WAS LAST CALLED WITH IRC = 0)
C             AND G CONTAINS THE DESIRED GRADIENT APPROXIMATION.
C      W I/O WORK VECTOR OF LENGTH 6 IN WHICH DS3GRD SAVES CERTAIN
C             QUANTITIES WHILE THE CALLER IS EVALUATING F(X) AT A
C             PERTURBED X.
C
C     ***  APPLICATION AND USAGE RESTRICTIONS  ***
C
C        THIS ROUTINE IS INTENDED FOR _USE_ WITH QUASI-NEWTON ROUTINES
C     FOR UNCONSTRAINED MINIMIZATION (IN WHICH CASE  ALPHA  COMES FROM
C     THE DIAGONAL OF THE QUASI-NEWTON HESSIAN APPROXIMATION).
C
C     ***  ALGORITHM NOTES  ***
C
C        THIS CODE DEPARTS FROM THE SCHEME PROPOSED BY STEWART (REF. 1)
C     IN ITS GUARDING AGAINST OVERLY LARGE OR SMALL STEP SIZES AND ITS
C     HANDLING OF SPECIAL CASES (SUCH AS ZERO COMPONENTS OF ALPHA OR G).
C
C     ***  REFERENCES  ***
C
C 1. STEWART, G.W. (1967), A MODIFICATION OF DAVIDON*S MINIMIZATION
C        METHOD TO ACCEPT DIFFERENCE APPROXIMATIONS OF DERIVATIVES,
C        J. ASSOC. COMPUT. MACH. 14, PP. 72-83.
C
C     ***  HISTORY  ***
C
C     DESIGNED AND CODED BY DAVID M. GAY (SUMMER 1977/SUMMER 1980).
C
C     ***  GENERAL  ***
C
C        THIS ROUTINE WAS PREPARED IN CONNECTION WITH WORK SUPPORTED BY
C     THE NATIONAL SCIENCE FOUNDATION UNDER GRANTS MCS76-00324 AND
C     MCS-7906671.
C
C.......................................................................
C
C     *****  EXTERNAL FUNCTION  *****
C
      DOUBLE PRECISION DR7MDC
      EXTERNAL DR7MDC
C DR7MDC... RETURNS MACHINE-DEPENDENT CONSTANTS.
C
C     ***** INTRINSIC FUNCTIONS *****
C/+
      DOUBLE PRECISION DSQRT
C/
C     ***** LOCAL VARIABLES *****
C
      LOGICAL HIT
      INTEGER FH, FX0, HSAVE, I, XISAVE
      DOUBLE PRECISION AAI, AFX, AFXETA, AGI, ALPHAI, AXI, AXIBAR,
     1                 DISCON, ETA, GI, H, HMIN, XI, XIH
      DOUBLE PRECISION C2000, FOUR, HMAX0, HMIN0, H0, MACHEP, ONE, P002,
     1                 THREE, TWO, ZERO
C
C/6
C     DATA C2000/2.0D+3/, FOUR/4.0D+0/, HMAX0/0.02D+0/, HMIN0/5.0D+1/,
C    1     ONE/1.0D+0/, P002/0.002D+0/, THREE/3.0D+0/,
C    2     TWO/2.0D+0/, ZERO/0.0D+0/
C/7
      PARAMETER (C2000=2.0D+3, FOUR=4.0D+0, HMAX0=0.02D+0, HMIN0=5.0D+1,
     1     ONE=1.0D+0, P002=0.002D+0, THREE=3.0D+0,
     2     TWO=2.0D+0, ZERO=0.0D+0)
C/
C/6
C     DATA FH/3/, FX0/4/, HSAVE/5/, XISAVE/6/
C/7
      PARAMETER (FH=3, FX0=4, HSAVE=5, XISAVE=6)
C/
C
C---------------------------------  BODY  ------------------------------
C
      IF (IRC) 80, 10, 210
C
C     ***  FRESH START -- GET MACHINE-DEPENDENT CONSTANTS  ***
C
C     STORE MACHEP IN W(1) AND H0 IN W(2), WHERE MACHEP IS THE UNIT
C     ROUNDOFF (THE SMALLEST POSITIVE NUMBER SUCH THAT
C     1 + MACHEP .GT. 1  AND  1 - MACHEP .LT. 1),  AND  H0 IS THE
C     SQUARE-ROOT OF MACHEP.
C
 10   W(1) = DR7MDC(3)
      W(2) = DSQRT(W(1))
C
      W(FX0) = FX
C
C     ***  INCREMENT  I  AND START COMPUTING  G(I)  ***
C
 20   I = IABS(IRC) + 1
      IF (I .GT. P) GO TO 220
         IRC = I
         IF (B(1,I) .LT. B(2,I)) GO TO 30
            G(I) = ZERO
            GO TO 20
 30      AFX = DABS(W(FX0))
         MACHEP = W(1)
         H0 = W(2)
         HMIN = HMIN0 * MACHEP
         XI = X(I)
         W(XISAVE) = XI
         AXI = DABS(XI)
         AXIBAR = DMAX1(AXI, ONE/D(I))
         GI = G(I)
         AGI = DABS(GI)
         ETA = DABS(ETA0)
         IF (AFX .GT. ZERO) ETA = DMAX1(ETA, AGI*AXI*MACHEP/AFX)
         ALPHAI = ALPHA(I)
         IF (ALPHAI .EQ. ZERO) GO TO 130
         IF (GI .EQ. ZERO .OR. FX .EQ. ZERO) GO TO 140
         AFXETA = AFX*ETA
         AAI = DABS(ALPHAI)
C
C        *** COMPUTE H = STEWART*S FORWARD-DIFFERENCE STEP SIZE.
C
         IF (GI**2 .LE. AFXETA*AAI) GO TO 40
              H = TWO*DSQRT(AFXETA/AAI)
              H = H*(ONE - AAI*H/(THREE*AAI*H + FOUR*AGI))
              GO TO 50
C40      H = TWO*(AFXETA*AGI/(AAI**2))**(ONE/THREE)
 40      H = TWO * (AFXETA*AGI)**(ONE/THREE) * AAI**(-TWO/THREE)
         H = H*(ONE - TWO*AGI/(THREE*AAI*H + FOUR*AGI))
C
C        ***  ENSURE THAT  H  IS NOT INSIGNIFICANTLY SMALL  ***
C
 50      H = DMAX1(H, HMIN*AXIBAR)
C
C        *** _USE_ FORWARD DIFFERENCE IF BOUND ON TRUNCATION ERROR IS AT
C        *** MOST 10**-3.
C
         IF (AAI*H .LE. P002*AGI) GO TO 120
C
C        *** COMPUTE H = STEWART*S STEP FOR CENTRAL DIFFERENCE.
C
         DISCON = C2000*AFXETA
         H = DISCON/(AGI + DSQRT(GI**2 + AAI*DISCON))
C
C        ***  ENSURE THAT  H  IS NEITHER TOO SMALL NOR TOO BIG  ***
C
         H = DMAX1(H, HMIN*AXIBAR)
         IF (H .GE. HMAX0*AXIBAR) H = AXIBAR * H0**(TWO/THREE)
C
C        ***  COMPUTE CENTRAL DIFFERENCE  ***
C
         XIH = XI + H
         IF (XI - H .LT. B(1,I)) GO TO 60
         IRC = -I
         IF (XIH .LE. B(2,I)) GO TO 200
            H = -H
            XIH = XI + H
            IF (XI + TWO*H .LT. B(1,I)) GO TO 190
            GO TO 70
 60      IF (XI + TWO*H .GT. B(2,I)) GO TO 190
C        *** MUST DO OFF-SIDE CENTRAL DIFFERENCE ***
 70      IRC = -(I + P)
         GO TO 200
C
 80      I = -IRC
         IF (I .LE. P) GO TO 100
         I = I - P
         IF (I .GT. P) GO TO 90
         W(FH) = FX
         H = TWO * W(HSAVE)
         XIH = W(XISAVE) + H
         IRC = IRC - P
         GO TO 200
C
C    *** FINISH OFF-SIDE CENTRAL DIFFERENCE ***
C
 90      I = I - P
         G(I) = (FOUR*W(FH) - FX - THREE*W(FX0)) / W(HSAVE)
         IRC = I
         X(I) = W(XISAVE)
         GO TO 20
C
 100     H = -W(HSAVE)
         IF (H .GT. ZERO) GO TO 110
         W(FH) = FX
         XIH = W(XISAVE) + H
         GO TO 200
C
 110     G(I) = (W(FH) - FX) / (TWO * H)
         X(I) = W(XISAVE)
         GO TO 20
C
C     ***  COMPUTE FORWARD DIFFERENCES IN VARIOUS CASES  ***
C
 120     IF (H .GE. HMAX0*AXIBAR) H = H0 * AXIBAR
         IF (ALPHAI*GI .LT. ZERO) H = -H
         GO TO 150
 130     H = AXIBAR
         GO TO 150
 140     H = H0 * AXIBAR
C
 150     HIT = .FALSE.
 160     XIH = XI + H
         IF (H .GT. ZERO) GO TO 170
            IF (XIH .GE. B(1,I)) GO TO 200
            GO TO 180
 170     IF (XIH .LE. B(2,I)) GO TO 200
 180        IF (HIT) GO TO 190
            HIT = .TRUE.
            H = -H
            GO TO 160
C
C        *** ERROR RETURN...
 190     IRC = I + P
         GO TO 230
C
C        *** RETURN FOR NEW FUNCTION VALUE...
 200     X(I) = XIH
         W(HSAVE) = H
         GO TO 999
C
C     ***  COMPUTE ACTUAL FORWARD DIFFERENCE  ***
C
 210     G(IRC) = (FX - W(FX0)) / W(HSAVE)
         X(IRC) = W(XISAVE)
         GO TO 20
C
C  ***  RESTORE FX AND INDICATE THAT G HAS BEEN COMPUTED  ***
C
 220  IRC = 0
 230  FX = W(FX0)
C
 999  RETURN
C  ***  LAST LINE OF DS3GRD FOLLOWS  ***
      END
