C$TEST LYMB
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE LYMB
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAM SYCE
C
C***********************************************************************
       INTEGER N, JEND, IREAD, I1MACH, I, JBEGIN, J, IWRITE
       INTEGER INTER(6), IEND, ITER, L, IFIX
       REAL C(20), SAVEC(36), B(6), SAVEB(6), R(6)
       REAL COND, R1MACH, BNORM, RNORM, ABS, ALOG10
       DOUBLE PRECISION D(6)
       N=5
C
C READ IN A SYMMETRIC MATRIX WHOSE UPPER TRIANGULAR
C PORTION IS STORED ONE ROW PER CARD. MAKE A
C COPY OF THE MATRIX SO THAT IT CAN BE USED LATER
C
        JEND=0
        IREAD=I1MACH(1)
        DO 20 I=1,N
           JBEGIN=JEND+1
           JEND=JBEGIN+N - I
           READ(IREAD,1)(C(J),J=JBEGIN,JEND)
  1        FORMAT(5F8.0)
           DO 10 J=JBEGIN,JEND
                SAVEC(J)=C(J)
  10       CONTINUE
  20    CONTINUE
C READ IN RIGHT HAND SIDE AND SAVE IT
        DO 30 I=1,N
           READ(IREAD,1)B(I)
           SAVEB(I)=B(I)
  30    CONTINUE
C
C  SOLVE AX = B USING SEPARATE CALLS TO SYCE AND SYFBS
C
       CALL SYCE(N,C,INTER,COND)
       CALL SYFBS(N,C,B,6,1,INTER)
       IWRITE=I1MACH(2)
       IF(COND.GE.1.0/R1MACH(4))WRITE(IWRITE,31)
  31   FORMAT(49H CONDITION NUMBER HIGH,ACCURATE SOLUTION UNLIKELY)
       WRITE(IWRITE,32) COND
  32   FORMAT(21H CONDITION NUMBER IS ,1PE16.8)
C      COMPUTE NORM OF SOLUTION
       BNORM=0.0
       WRITE(IWRITE,33)
  33   FORMAT(43H THE FIRST SOLUTION X, FROM SYCE AND SYFBS=)
       DO 40 I=1,N
          BNORM=BNORM+ABS(B(I))
  40      WRITE(IWRITE,41)B(I)
  41   FORMAT(1H ,F20.7)
C
C IEND IS THE UPPER BOUND ON THE NUMBER OF BITS PER WORD
C
       IEND=I1MACH(11)*IFIX(R1MACH(5)/ALOG10(2.0)+1.0)
C
C REFINE SOLUTION
C
       DO 90 ITER=1,IEND
C
C COMPUTE RESIDUAL R = B - AX, IN DOUBLE PRECISION
C
          DO 50 I=1,N
  50         D(I)=DBLE(SAVEB(I))
          L=1
          DO 70 I=1,N
             DO 60 J=I,N
                IF (I.NE.J) D(J)=D(J) - DBLE(SAVEC(L))*B(I)
                D(I) = D(I) - DBLE(SAVEC(L))*B(J)
  60         L=L+1
             R(I) = D(I)
  70      CONTINUE
C
C SOLVE A(DELTAX) =R
C
          CALL SYFBS(N,C,R,8,1,INTER)
C
C DETERMINE NORM OF CORRECTION AND ADD IN CORRECTION
C
          WRITE(IWRITE,71)ITER
  71      FORMAT(30H THE SOLUTION AFTER ITERATION ,I5)
          RNORM=0.0
          DO 80 I=1,N
                B(I) = B(I) + R(I)
                RNORM=RNORM+ABS(R(I))
                WRITE(IWRITE,41)B(I)
  80      CONTINUE
       IF(RNORM.LT.R1MACH(4)*BNORM) STOP
  90   CONTINUE
       WRITE(IWRITE,91)
  91   FORMAT(29H ITERATIVE IMPROVEMENT FAILED)
       STOP
       END
C
C DATA FOR THE EXAMPLE IN THE PORT SHEET...  (REMOVE THE C
C IN COLUMN 1 BEFORE FEEDING THIS DATA TO THE PROGRAM ABOVE.)
C$DATA
C    -4.      0.    -16.    -32.     28.
C     1.      5.     10.     -6.
C   -37.    -66.     64.
C   -85.     53.
C   -15.
C   448.
C  -111.
C  1029.
C  1207.
C  -719.
