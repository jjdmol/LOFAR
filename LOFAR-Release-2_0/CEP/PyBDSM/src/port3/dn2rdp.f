      SUBROUTINE DN2RDP(IV, LIV, LV, N, RD, V)
C
C  ***  PRINT REGRESSION DIAGNOSTICS FOR MLPSL AND NL2S1 ***
C
      INTEGER LIV, LV, N
      INTEGER IV(LIV)
      DOUBLE PRECISION RD(N), V(LV)
C
C     ***  NOTE -- V IS PASSED FOR POSSIBLE _USE_ BY REVISED VERSIONS OF
C     ***  THIS ROUTINE.
C
      INTEGER PU
C
C  ***  IV AND V SUBSCRIPTS  ***
C
      INTEGER COVPRT, F, NEEDHD, PRUNIT, REGD
C
C/6
C     DATA COVPRT/14/, F/10/, NEEDHD/36/, PRUNIT/21/, REGD/67/
C/7
      PARAMETER (COVPRT=14, F=10, NEEDHD=36, PRUNIT=21, REGD=67)
C/
C
C+++++++++++++++++++++++++++++++  BODY  ++++++++++++++++++++++++++++++++
C
      PU = IV(PRUNIT)
      IF (PU .EQ. 0) GO TO 999
      IF (IV(COVPRT) .LT. 2) GO TO 999
      IF (IV(REGD) .LE. 0) GO TO 999
      IV(NEEDHD) = 1
      IF (V(F)) 10, 30, 10
 10   WRITE(PU,20) RD
 20   FORMAT(/70H REGRESSION DIAGNOSTIC = SQRT( G(I)**T * H(I)**-1 * G(I
     1) / ABS(F) ).../(6D12.3))
      GO TO 999
 30   WRITE(PU,40) RD
 40   FORMAT(/61H REGRESSION DIAGNOSTIC = SQRT( G(I)**T * H(I)**-1 * G(I
     1) ).../(6D12.3))
C
 999  RETURN
C  ***  LAST LINE OF DN2RDP FOLLOWS  ***
      END
