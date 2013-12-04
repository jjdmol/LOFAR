C$TEST PRMA
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE PRMA
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAM SPMSF
C
C***********************************************************************
       INTEGER MRP(32), MCP(32), INMCP(32)
       INTEGER IROW(33), JA(200), IWORK(500)
       IREAD = I1MACH(1)
       IWRITE = I1MACH(2)
       N = 32
       MAXIW = 500
C
C READ IN THE VECTORS DEFINING THE NONZERO BLOCKS
C
       READ(IREAD,11)(IROW(I),I=1,33)
  11   FORMAT(20I3)
       IEND=IROW(33) - 1
       READ(IREAD,11)(JA(I),I=1,IEND)
       WRITE(IWRITE,12)IEND
  12   FORMAT(29H NUMBER OF NONZEROS IN MATRIX,I5)
C
C SET UP THE PERMUTATION VECTORS TO REFLECT NO REORDERING
C
       DO 20 I=1, N
          MRP(I) = I
          MCP(I) = I
          INMCP(I) = I
  20   CONTINUE
C
C DETERMINE THE SYMBOLIC FACTORIZATION
C
       CALL SPMSF(N, MRP, INMCP, IROW, JA, IWORK, MAXIW, ISIZE)
       WRITE(IWRITE,21)ISIZE
  21   FORMAT(35H BLOCKS NEEDED WITHOUT PERMUTATIONS,I5)
C
C FIND AN ORDERING WHICH WOULD DECREASE FILL-IN AND RECOMPUTE THE
C SYMBOLIC FACTORIZATION
C
       CALL SPMOR(N, IROW, JA, MCP, INMCP)
       CALL SPMSF(N, MCP, INMCP, IROW, JA, IWORK, MAXIW, ISIZE)
       WRITE(IWRITE,22)ISIZE
  22   FORMAT(32H BLOCKS NEEDED WITH PERMUTATIONS,I5)
       STOP
       END
