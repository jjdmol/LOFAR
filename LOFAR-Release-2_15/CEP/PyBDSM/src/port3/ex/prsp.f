C$TEST PRSP
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE PRSP
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAM SPMLU
C
C***********************************************************************
      INTEGER IA(401), JA(2500), ISTAK(18500), I1MACH, IWRITE, N
      INTEGER MRP(400), MCP(400), ITEMP, INMCP(400)
      INTEGER ILAPSZ, IT, IT2, IT3, IT4, IT5, IT6, I, NUMBER
      INTEGER NUM, IPOINT, NP1
      REAL RSTAK(18500), A(2500), GROWTH
      DOUBLE PRECISION DSTAK(9250)
      COMMON /CSTAK/DSTAK
      EQUIVALENCE(ISTAK(1), RSTAK(1), DSTAK(1))
      IWRITE = I1MACH(2)
      CALL ISTKIN(18500,2)
      CALL ENTSRC(NEW,1)
      DO 40 K = 9,19,5
         N = (K+1)*(K+1)
         CALL SETUP(K, N, IA, JA, A)
         NUMBER = IA(N+1)-1
         WRITE(IWRITE,11)N,NUMBER
  11     FORMAT(5H N = ,I4,22H NUMBER OF NONZEROS = ,I7)
C
C ORDER THE ROWS AND COLUMNS OF THE MATRIX
C TO DECREASE FILL-IN
C
         CALL SPMOR(N, IA, JA, MRP, INMCP)
C ALLOCATE THE AVAILABLE SPACE FOR THE WORK SPACE IN SPMSF
C BUT MAKE SURE THERE IS EMOUGH FOR SPMSF'S ALLOCATIONS
C
         MAXIW = ISTKQU(2)-3*N-5
         IW = ISTKGT(MAXIW,2)
C
C TIME THE SYMBOLIC FACTORIZATION
C
         IT = ILAPSZ(0)
         CALL SPMSF(N, MRP, INMCP, IA, JA, ISTAK(IW), MAXIW, ISIZE)
         IT2= ILAPSZ(0)-IT
         WRITE(IWRITE, 12)ISIZE
  12     FORMAT(37H NUMBER OF NONZEROS IN DECOMPOSITION=,I5)
         WRITE(IWRITE,13)IT2
  13     FORMAT(23H ELAPSED TIME FOR SPMSF,I7)
C
C MODIFY THE WORK STACK TO REFLECT THE AMOUNT NEEDED BY
C SPMSF AND ALLOCATE SPACE FOR THE NUMERICAL FACTORIZATION
C
         ISPAC= 2*N+2+ISIZE
         IW = ISTKMD(ISPAC,2)
         IUL = ISTKGT(ISIZE, 3)
C
C COMPUTE THE TIME NEEDED TO INSERT THE NUMERICAL ELEMENTS
C IN THEIR PROPER PLACES
C
         IT3 = ILAPSZ(0)
         DO 20 I=1, N
            MCP(I) = MRP(I)
            IR = MRP(I)
            NUM = IA(IR+1)-IA(IR)
            IPOINT = IA(IR)
            CALL SPMIN(N, INMCP, ISTAK(IW), I, A(IPOINT),
     1        JA(IPOINT), NUM, I, RSTAK(IUL))
  20     CONTINUE
         IT4 = ILAPSZ(0)-IT3
         WRITE(IWRITE,21)IT4
  21     FORMAT(23H ELAPSED TIME FOR SPMIN,I7)
C
C TIME THE SUBROUTINE WHICH COMPUTES THE NUMERICAL
C FACTORIZATION
C
         IT5 =ILAPSZ(0)
         CALL SPMNF(N, ISTAK(IW), RSTAK(IUL), 0.0, GROWTH)
         IT6 =ILAPSZ(0)-IT5
         WRITE(IWRITE, 22)IT6
  22     FORMAT(23H ELAPSED TIME FOR SPMNF,I7)
         IT6 = IT2 + IT4 +IT6
         WRITE(6,23)IT6
  23     FORMAT(26H ELAPSED TIME FOR SF-IN-NF,I7)
         NP1 = N+1
C
C REDO THE FACTORIZATION WITH SPMLU AND TIME IT
C
         CALL MOVEFR(NUMBER,A,RSTAK(IUL))
         CALL MOVEFI(NUMBER,JA,ISTAK(IW))
         IL = ISTKGT(N+1,2)
         IT5 =ILAPSZ(0)
         CALL SPMLU(N, MRP, MCP, IA, ISTAK(IW), RSTAK(IUL), ISPAC,
     1   ISTAK(IL), 0.0, 0.0, ISIZE, GROWTH)
         IT6 = ILAPSZ(0)-IT5
         WRITE(IWRITE, 31)IT6
  31     FORMAT(23H ELAPSED TIME FOR SPMLU, I7)
         CALL ISTKRL(3)
  40  CONTINUE
      STOP
      END
