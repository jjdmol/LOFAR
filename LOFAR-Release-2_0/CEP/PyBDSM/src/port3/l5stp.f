      SUBROUTINE  L5STP(NPTS, MESH, FN, QK, DELK, M, N, P, Q)
      INTEGER NPTS
      INTEGER M, N
      REAL MESH(NPTS), FN(NPTS), QK(NPTS), DELK, P(1), Q(1)
      COMMON /CSTAK/ DSTAK
      DOUBLE PRECISION DSTAK(500)
      INTEGER APTR, XPTR, ISTKGT, ISTAK(1000)
      INTEGER BC, BX, C, G, IW, LIW, LW, MM, NN, W
      REAL WS(500)
      EQUIVALENCE (DSTAK(1), ISTAK(1))
      EQUIVALENCE (DSTAK(1), WS(1))
C   THIS ROUTINE ALLOCATES STORAGE SO THAT
C    L9STP CAN DEFINE THE LINEAR PROGRAMMING SUBPROBLEM OF
C   THE DIFFERENTIAL CORRECTION ALGORITHM AND CALL A GENERAL
C   PURPOSE LINEAR PROGRAMMING PACKAGE.
C   INPUT...
C   NPTS   - THE NUMBER OF MESH POINTS.
C   MESH   - THE ARRAY OF MESH POINTS.
C   FN     - THE ARRAY OF FUNCTION VALUES.
C   QK     - THE ARRAY OF CURRENT DENOMINATOR VALUES.
C   DELK   - THE CURRENT MINIMAX ERROR.
C   M      - THE DEGREE OF THE NUMERATOR POLYNOMIAL.
C   N      - THE DEGREE OF THE DENOMINATOR POLYNOMIAL.
C   P      - THE CURRENT NUMERATOR POLYNOMIAL.
C   Q      - THE CURRENT DENOMINATOR POLYNOMIAL.
C   OUTPUT...
C   P      - THE ARRAY OF COEFFICIENTS FOR THE NUMERATOR POLYNOMIAL.
C   Q      - THE ARRAY OF COEFFICIENTS FOR THE DENOMINATOR POLYNOMIAL.
C   ERROR STATES (ASTERISK INDICATES FATAL)...
C   1* - INVALID DEGREE
C   2* - TOO FEW MESH POINTS
C   3* - NONPOSITIVE DELK
C   4  - NO IMPROVEMENT IN THE LP SUBPROBLEM
C
C *** BODY ***
C
      CALL ENTER(1)
C/6S
C     IF (M .LT. 0 .OR. N .LT. 0) CALL SETERR(
C    1   23H L5STP - INVALID DEGREE, 23, 1, 2)
C     IF (NPTS .LT. M+N+2) CALL SETERR(28H L5STP - TOO FEW MESH POINTS,
C    1    28, 2, 2)
C/7S
      IF (M .LT. 0 .OR. N .LT. 0) CALL SETERR(
     1   ' L5STP - INVALID DEGREE', 23, 1, 2)
      IF (NPTS .LT. M+N+2) CALL SETERR(' L5STP - TOO FEW MESH POINTS',
     1    28, 2, 2)
C/
      MM = 2 * NPTS
      NN = M + N + 3
      LIW = MM + NN + 7
      LW = NN*(3*NN+17)/2 + MM + 2
      G = ISTKGT(NN, 3)
      C = ISTKGT(NN*MM, 3)
      BC = ISTKGT(2*MM, 3)
      BX = ISTKGT(2*NN, 3)
      W = ISTKGT(LW, 3)
      IW = ISTKGT(LIW, 2)
      APTR = ISTKGT(3*NPTS+1, 3)
      XPTR = ISTKGT(NN, 3)
      CALL  L9STP(NPTS, MESH, FN, QK, DELK, M, N, P, Q, WS(APTR),
     1            WS(BC), WS(BX), WS(C), WS(G), ISTAK(IW), LIW, LW,
     2            MM, NN, WS(W), WS(XPTR))
      CALL LEAVE
      RETURN
C *** LAST LINE OF  L5STP FOLLOWS ***
      END
