      SUBROUTINE  N2CVP(IV, LIV, LV, P, V)
C
C  ***  PRINT COVARIANCE MATRIX FOR   RN2G  ***
C
      INTEGER LIV, LV, P
      INTEGER IV(LIV)
      REAL V(LV)
C
C  ***  LOCAL VARIABLES  ***
C
      INTEGER COV1, I, II, I1, J, PU
      REAL T
C
C     ***  IV SUBSCRIPTS  ***
C
      INTEGER COVMAT, COVPRT, COVREQ, NEEDHD, NFCOV, NGCOV, PRUNIT,
     1        RCOND, REGD, STATPR
C
C/6
C     DATA COVMAT/26/, COVPRT/14/, COVREQ/15/, NEEDHD/36/, NFCOV/52/,
C    1     NGCOV/53/, PRUNIT/21/, RCOND/53/, REGD/67/, STATPR/23/
C/7
      PARAMETER (COVMAT=26, COVPRT=14, COVREQ=15, NEEDHD=36, NFCOV=52,
     1           NGCOV=53, PRUNIT=21, RCOND=53, REGD=67, STATPR=23)
C/
C  ***  BODY  ***
C
      IF (IV(1) .GT. 8) GO TO 999
      PU = IV(PRUNIT)
      IF (PU .EQ. 0) GO TO 999
      IF (IV(STATPR) .EQ. 0) GO TO 30
         IF (IV(NFCOV) .GT. 0) WRITE(PU,10) IV(NFCOV)
 10      FORMAT(/1X,I4,50H EXTRA FUNC. EVALS FOR COVARIANCE AND DIAGNOST
     1ICS.)
         IF (IV(NGCOV) .GT. 0) WRITE(PU,20) IV(NGCOV)
 20      FORMAT(1X,I4,50H EXTRA GRAD. EVALS FOR COVARIANCE AND DIAGNOSTI
     1CS.)
C
 30   IF (IV(COVPRT) .LE. 0) GO TO 999
      COV1 = IV(COVMAT)
      IF (IV(REGD) .LE. 0 .AND. COV1 .LE. 0) GO TO 70
      IV(NEEDHD) = 1
      T = V(RCOND)**2
      IF (IABS(IV(COVREQ)) .GT. 2) GO TO 50
C
      WRITE(PU,40) T
 40   FORMAT(/47H RECIPROCAL CONDITION OF F.D. HESSIAN = AT MOST,E10.2)
      GO TO 70
C
 50   WRITE(PU,60) T
 60   FORMAT(/44H RECIPROCAL CONDITION OF (J**T)*J = AT LEAST,E10.2)
C
 70   IF (MOD(IV(COVPRT),2) .EQ. 0) GO TO 999
      IV(NEEDHD) = 1
      IF (COV1) 80,110,130
 80   IF (-1 .EQ. COV1) WRITE(PU,90)
 90   FORMAT(/43H ++++++ INDEFINITE COVARIANCE MATRIX ++++++)
      IF (-2 .EQ. COV1) WRITE(PU,100)
 100  FORMAT(/52H ++++++ OVERSIZE STEPS IN COMPUTING COVARIANCE +++++)
      GO TO 999
C
 110  WRITE(PU,120)
 120  FORMAT(/45H ++++++ COVARIANCE MATRIX NOT COMPUTED ++++++)
      GO TO 999
C
 130  I = IABS(IV(COVREQ))
      IF (I .LE. 1) WRITE(PU,140)
 140  FORMAT(/48H COVARIANCE = SCALE * H**-1 * (J**T * J) * H**-1/
     1       23H WHERE H = F.D. HESSIAN/)
      IF (I .EQ. 2) WRITE(PU,150)
 150  FORMAT(/56H COVARIANCE = H**-1, WHERE H = FINITE-DIFFERENCE HESSIA
     1N/)
      IF (I .GT. 2) WRITE(PU,160)
 160  FORMAT(/30H COVARIANCE = SCALE * J**T * J/)
      II = COV1 - 1
      DO 170 I = 1, P
         I1 = II + 1
         II = II + I
         WRITE(PU,180) I, (V(J), J = I1, II)
 170     CONTINUE
 180  FORMAT(4H ROW,I3,2X,5E12.3/(9X,5E12.3))
C
 999  RETURN
C  ***  LAST CARD OF  N2CVP FOLLOWS  ***
      END
