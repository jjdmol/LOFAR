C$TEST LNAB
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE LNAB
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAM LSTSQ
C
C***********************************************************************
      REAL X(10,2), Y(10), C(2), XX(10,2), YY(10)
C
C  SET THE FIRST COLUMN OF THE X ARRAY TO THE ACTUAL X,
C  AND THE SECOND COLUMN TO 1.0
C
      DO 10  K=1,6
      X(K,1) = FLOAT(K)
 10   X(K,2) = 1.
C
C  SET THE VALUES OF THE RIGHT-HAND SIDE, Y
C
      Y(1) =  .3
      Y(2) =  .95
      Y(3) = 2.6
      Y(4) = 2.5
      Y(5) = 2.3
      Y(6) = 3.95
C
C  SINCE LSTSQ WRITES OVER THE ARRAYS X AND Y,
C  SAVE THEM FOR LATER DEMONSTRATION COMPUTATION.
C
      DO 15 K=1,6
      YY(K) = Y(K)
      DO 15 J=1,2
 15   XX(K,J)=X(K,J)
C
      CALL LSTSQ (10,2,6,2,X,Y,1,C)
C
      IWRITE = I1MACH(2)
      WRITE(IWRITE,97) C(1), C(2)
 97   FORMAT (8H0C(1) = ,E16.8, 11H    C(2) = ,E16.8)
C
C  COMPUTE THE SUM OF THE SQUARES OF THE ERROR
C  USING BRUTE FORCE.
C
      ERR = 0.
      DO 20 J=1,6
      ADD = (C(1)*XX(J,1)+C(2)-YY(J))**2
 20   ERR = ERR + ADD
C
      WRITE(IWRITE,98) ERR
 98   FORMAT(35H0LEAST-SQUARES ERROR (VERSION 1) = ,E16.8)
C
C  COMPUTE THE LEAST-SQUARES ERROR USING THE PROGRAM SOLUTION.
C
      ERR = 0.
      DO 30 L=3,6
      ERR = ERR + Y(L)*Y(L)
 30   CONTINUE
C
      WRITE(IWRITE,99) ERR
 99   FORMAT(35H0LEAST-SQUARES ERROR (VERSION 2) = ,E16.8)
C
      STOP
      END
