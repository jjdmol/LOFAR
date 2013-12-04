C$TEST PRSY
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE PRSY
C***********************************************************************
C
C  EXAMPLE OF USE OF THE PORT PROGRAM SPFNF
C
C***********************************************************************
       INTEGER N1, N2, N, MCP(500), MRP(500)
       INTEGER I1MACH, IREAD, IWRITE, TEMP, ISTAK(19000)
       REAL A1, A2, GROWTH
       INTEGER ISPAC,IW,MAXIW, IUL
       REAL EL1, EL2, TR, SASUM, RSTAK(19000), B(500)
       EXTERNAL QUEI, QUEA
       DOUBLE PRECISION DSTAK(9500)
       COMMON /CSTAK/ DSTAK
       COMMON /QUE/ A1, A2, N1, N2, N
       EQUIVALENCE(RSTAK(1), ISTAK(1), DSTAK(1))
       IREAD = I1MACH(1)
       IWRITE = I1MACH(2)
       CALL ISTKIN(19000, 2)
  10   READ(IREAD,11)N1, N2
  11   FORMAT(2I3)
       IF (N1. EQ. 0) STOP
       WRITE(IWRITE,12)N1, N2
  12   FORMAT(/4H N1=,I3,4H N2=,I3)
       N = (N1+1)*(N2+1)
C
C DETERMINE THE ORDERING
C
       CALL SPFOR(N, QUEI, MCP)
C
C GET THE WORK SPACE FROM THE STORAGE STACK
C
       MAXIW = ISTKQU(2)-3*N-50
       IW = ISTKGT(MAXIW, 2)
C
C DETERMINE THE SYMBOLIC FACTORIZATION
C
       DO 20 I=1,N
          MRP(I) = MCP(I)
  20   CONTINUE
       CALL SPFSF(N, MRP, MCP, QUEI, ISTAK(IW), MAXIW, ISIZE)
C
C DETERMINE THE ACTUAL AMOUNT OF SPACE USED, MODIFY THE
C INTEGER WORK SPACE AND ALLOCATE SPACE TO SAVE THE
C FACTORIZATION
C
       ISPAC = 2*N+1+ISIZE
       IW = ISTKMD(ISPAC,2)
       IUL= ISTKGT(ISIZE,3)
  30   READ(IREAD,31)A1, A2
  31   FORMAT(2F10.3)
       IF (A1.EQ.0.0)GO TO 50
       WRITE(IWRITE,32)A1, A2
  32   FORMAT(/4H A1=,F10.3,4H A2=,F10.3)
C
C COMPUTE THE NUMERICAL FACTORIZATION
C
       CALL SPFNF(N, MRP, MCP, QUEA, ISTAK(IW), RSTAK(IUL),
     1  GROWTH, 0.0)
       WRITE(IWRITE,33)GROWTH
  33   FORMAT(7H GROWTH,E25.7)
C
C GENERATE RIGHT HAND SIDE
C
       DO 40 I=1,N
          B(I) = 0.0
  40   CONTINUE
       B(N) = 1.0
C
C SOLVE THE PROBLEM
C
       CALL SPSOL(N, MRP, MCP, ISTAK(IW), RSTAK(IUL), B, N, 1)
C
C FIND PROBABILITY OF BEING LOST
C
       EL1 = B(N)
       EL2 = SASUM(N1+1,B(N2+1),N2+1)
       TEMP = (N2+1)*N1+1
       TR = SASUM(N2,B(TEMP),1)
       WRITE(IWRITE,41)EL1,EL2,TR
  41   FORMAT(6H L1 = ,E15.5,6H L2 = ,E15.5,7H I12 = ,E15.5)
       GO TO 30
C RELEASE STACK SPACE
  50   CALL ISTKRL(2)
       GO TO 10
       END
